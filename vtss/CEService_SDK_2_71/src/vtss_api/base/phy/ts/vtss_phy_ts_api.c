/*

 Vitesse API software.

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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PHY

// Avoid "vtss_api.h not used in module vtss_phy_ts_api.c"
/*lint --e{766} */

#include "vtss_api.h"
#include "../../ail/vtss_state.h"
#include "vtss_phy_ts_api.h"
#include "vtss_phy_ts.h"
#ifdef VTSS_ARCH_DAYTONA
#include "../../daytona/vtss_daytona_basics.h"
#endif

#if defined(VTSS_FEATURE_MACSEC)
#include "vtss_macsec_api.h"
#endif /* VTSS_FEATURE_MACSEC */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
#include "../phy_1g/vtss_phy_init_scripts.h"
#endif

#if defined(VTSS_IOADDR)
#undef VTSS_IOADDR
#endif

#if defined(VTSS_IOREG)
#undef VTSS_IOREG
#endif

#if defined(VTSS_IOREG_IX)
#undef VTSS_IOREG_IX
#endif

#include "vtss_phy_ts_regs_common.h"
#include "vtss_phy_ts_regs.h"

/*lint -sem( vtss_callout_lock, thread_lock ) */
/*lint -sem( vtss_callout_unlock, thread_unlock ) */

#ifdef VTSS_CHIP_10G_PHY
#define VTSS_PHY_TS_10G_DATA_CLK_CTRL_REG ((u16)0x03)
#define VTSS_PHY_TS_10G_DATA_CLK_MMD_ADDR ((u16)0x1E)
#define VTSS_PHY_TS_10G_DATA_CLK_SEL_MASK ((u16)0x1)
#define VTSS_PHY_TS_10G_FRC_DIS ((u16)0x280)
#endif

#ifdef VTSS_CHIP_CU_PHY
#define VTSS_PHY_TS_1G_BIU_ADDR_REG   ((u16)0x10)

#define VTSS_PHY_TS_1G_ADDR_EXT_REG   ((u16)0x1f)
#define VTSS_PHY_TS_1G_ADDR_MIIM_REG  ((u16)0x17)
#define VTSS_PHY_TS_1G_FIFO_DEPTH_REG ((u16)0x14)

/* Register to read/write the Least significant 2 bytes of the CSR data
 */
#define VTSS_PHY_TS_1G_CSR_DATA_LOWER  ((u16)0x11)
/* Register to read/write the Most significant 2 bytes of the CSR data
 */
#define VTSS_PHY_TS_1G_CSR_DATA_UPPER  ((u16)0x12)

#define VTSS_PHY_TS_1G_BIU_ADDR_REG_EXE_CMD    ((u16)0x8000)

#define VTSS_PHY_TS_1G_BIU_ADDR_REG_READ_CMD   ((u16)0x4000)
#define VTSS_PHY_TS_1G_BIU_ADDR_REG_WRITE_CMD  ((u16)0x0000)

#define VTSS_PHY_TS_INGR_DF_DEPTH              0xe
#define VTSS_PHY_TS_EGR_DF_DEPTH               0xe

#define VTSS_PHY_TS_1G_REG_READ_MAX_CNT         ((u16)0x8)
#endif /* VTSS_CHIP_CU_PHY */

#define VTSS_PHY_TS_READ_CSR(p, b, a, v) vtss_phy_ts_read_csr(vtss_state, p, b, a, v)

#define VTSS_PHY_TS_WRITE_CSR(p, b, a, v) vtss_phy_ts_write_csr(vtss_state, p , b, a, v)


#define VTSS_PHY_TS_TIME_INTERVAL_ADJUST_16(ti) \
                      ((ti & 0xffff0000) >> 16)

#define VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32(ti) \
                      (ti & 0xffffffffLL)
#define VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32_NS(ti) \
                      ((ti & 0xffffffff0000LL) >> 16)


#define  VTSS_PHY_TS_CLR_BITS(value, mask)   (value & ~mask)


#define VTSS_PHY_TS_EVENT_MASK              0x7f

#define VTSS_PHY_TS_ASSERT(x) if((x)) { return VTSS_RC_ERROR;}

#define VTSS_PHY_TS_ANA_BLK_ID_ING(x) VTSS_PHY_TS_ANA_BLK_ID_ING_##x
#define VTSS_PHY_TS_ANA_BLK_ID_EGR(x) VTSS_PHY_TS_ANA_BLK_ID_EGR_##x
#define VTSS_PHY_TS_PROC_BLK_ID(x)    VTSS_PHY_TS_PROC_BLK_ID_##x

#define  BPC    8               /* Bits per (unsigned) char */

/* The functions below operate on an array of u32 words of data.
            These data are stored according to the CPU endianness.
                             The endianness affects the mapping from a given bit to corresponding byte offset. */
#ifdef VTSS_OS_BIG_ENDIAN
#define BYTE_OFFSET(offset) ((offset/BPC) + 3 - 2*((offset/BPC)%4))
#else
#define BYTE_OFFSET(offset) (offset/BPC)
#endif /* VTSS_OS_BIG_ENDIAN */

extern long long int llabs(long long int);

#ifdef VTSS_CHIP_10G_PHY
vtss_rc vtss_phy_10g_id_get_priv(vtss_state_t *vtss_state,
                                 const vtss_port_no_t   port_no,
                                 vtss_phy_10g_id_t *const phy_id);
#endif

static vtss_rc vtss_phy_ts_base_port_get_priv(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no,
                                              vtss_port_no_t    *const base_port_no);

typedef enum {
    VTSS_PHY_TS_ANA_BLK_ID_ING_0, /* Order taken from 1G PHY */
    VTSS_PHY_TS_ANA_BLK_ID_EGR_0,
    VTSS_PHY_TS_ANA_BLK_ID_ING_1,
    VTSS_PHY_TS_ANA_BLK_ID_EGR_1,
    VTSS_PHY_TS_ANA_BLK_ID_ING_2,
    VTSS_PHY_TS_ANA_BLK_ID_EGR_2,
    VTSS_PHY_TS_PROC_BLK_ID_0,
    VTSS_PHY_TS_PROC_BLK_ID_1,
    VTSS_PHY_TS_MAX_BLK_ID
} vtss_phy_ts_blk_id_t;

#ifdef VTSS_ARCH_DAYTONA
enum {
    VTSS_DEV_TYPE_8492 = 0x8492,
    VTSS_DEV_TYPE_8494 = 0x8494,
};
#endif

typedef struct vtss_phy_ts_target_map {
    u16  dev_id;
    u16  mmd_addr;
    u16  mdio_address[VTSS_PHY_TS_MAX_BLK_ID];
} vtss_phy_ts_biu_addr_map_t;

typedef enum {
    VTSS_PHY_TS_ING_LATENCY_SET,
    VTSS_PHY_TS_EGR_LATENCY_SET,
    VTSS_PHY_TS_PATH_DELAY_SET,
    VTSS_PHY_TS_DELAY_ASYM_SET,
    VTSS_PHY_TS_RATE_ADJ_SET,
    VTSS_PHY_TS_PORT_ENA_SET,
    VTSS_PHY_TS_PORT_EVT_MASK_SET,
    VTSS_PHY_TS_PORT_OPER_MODE_CHANGE_SET,
    VTSS_PHY_TS_PPS_CONF_SET,
    VTSS_PHY_TS_ALT_CLK_SET,
    VTSS_PHY_TS_SERTOD_SET,
    VTSS_PHY_TS_LOAD_PULSE_DLY_SET,
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    VTSS_PHY_TS_ING_DELAY_COMP_SET,
    VTSS_PHY_TS_EGR_DELAY_COMP_SET,
#endif

} vtss_phy_ts_proc_conf_t;

typedef enum {
#ifdef VTSS_CHIP_10G_PHY
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8487,
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8488,
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8490,
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8574,
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8572,
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_ARCH_DAYTONA
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8492,
    VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8494,
#endif /* VTSS_ARCH_DAYTONA */
} vtss_phy_ts_chip_conf_idx_t;

static vtss_phy_ts_biu_addr_map_t vtss_phy_ts_biu_addr_map [] = {
#ifdef VTSS_CHIP_10G_PHY
    {
        .dev_id = VTSS_PHY_TYPE_8487,
        .mmd_addr = 30,
        .mdio_address = {
            [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = 0xA000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = 0xA800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = 0xB000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = 0xB800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = 0xC000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = 0xC800,
            [VTSS_PHY_TS_PROC_BLK_ID_0]    = 0x9000,
            [VTSS_PHY_TS_PROC_BLK_ID_1]    = 0x9100
        },
    },
    {
        .dev_id = VTSS_PHY_TYPE_8488,
        .mmd_addr = 30,
        .mdio_address = {
            [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = 0xA000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = 0xA800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = 0xB000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = 0xB800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = 0xC000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = 0xC800,
            [VTSS_PHY_TS_PROC_BLK_ID_0]    = 0x9000,
            [VTSS_PHY_TS_PROC_BLK_ID_1]    = 0x9100
        },
    },
    {
        .dev_id = VTSS_PHY_TYPE_8490,   /* Same is applicable for 8489 and 8490 */
        .mmd_addr = 30,
        .mdio_address = {
            [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = 0xA000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = 0xA800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = 0xB000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = 0xB800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = 0xC000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = 0xC800,
            [VTSS_PHY_TS_PROC_BLK_ID_0]    = 0x9000,
            [VTSS_PHY_TS_PROC_BLK_ID_1]    = 0x9200
        },
    },

#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    {
        .dev_id = VTSS_PHY_TYPE_8574,
        .mmd_addr = 0, /* Not Valid for Tesla */
        /* This Address mapping is not supported for 1G
         */
        .mdio_address = {
            [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = 0xA000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = 0xA800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = 0xB000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = 0xB800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = 0xC000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = 0xC800,
            [VTSS_PHY_TS_PROC_BLK_ID_0]    = 0x9000,
            [VTSS_PHY_TS_PROC_BLK_ID_1]    = 0x9100
        },
    },
    {
        .dev_id = VTSS_PHY_TYPE_8572,
        .mmd_addr = 0, /* Not Valid for Tesla */
        /* This Address mapping is not supported for 1G
         */
        .mdio_address = {
            [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = 0xA000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = 0xA800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = 0xB000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = 0xB800,
            [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = 0xC000,
            [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = 0xC800,
            [VTSS_PHY_TS_PROC_BLK_ID_0]    = 0x9000,
            [VTSS_PHY_TS_PROC_BLK_ID_1]    = 0x9100
        },
    },

#endif
#ifdef VTSS_ARCH_DAYTONA
    {
        .dev_id = VTSS_DEV_TYPE_8492,
        .mmd_addr = 0, /* Not Valid for Daytona */
    },
    {
        .dev_id = VTSS_DEV_TYPE_8494,
        .mmd_addr = 0, /* Not Valid for Daytona */
    },
#endif
};

#ifdef VTSS_ARCH_DAYTONA
u32 daytona_blk_id_map[VTSS_PHY_TS_MAX_BLK_ID] = {
    [VTSS_PHY_TS_ANA_BLK_ID_ING_0] = DAYTONA_BLOCK_IP_1588_ANA0_INGR,
    [VTSS_PHY_TS_ANA_BLK_ID_EGR_0] = DAYTONA_BLOCK_IP_1588_ANA0_EGR,
    [VTSS_PHY_TS_ANA_BLK_ID_ING_1] = DAYTONA_BLOCK_IP_1588_ANA1_INGR,
    [VTSS_PHY_TS_ANA_BLK_ID_EGR_1] = DAYTONA_BLOCK_IP_1588_ANA1_EGR,
    [VTSS_PHY_TS_ANA_BLK_ID_ING_2] = DAYTONA_BLOCK_IP_1588_ANA2_INGR,
    [VTSS_PHY_TS_ANA_BLK_ID_EGR_2] = DAYTONA_BLOCK_IP_1588_ANA2_EGR,
    [VTSS_PHY_TS_PROC_BLK_ID_0]    = DAYTONA_BLOCK_IP_1588,
    [VTSS_PHY_TS_PROC_BLK_ID_1]    = DAYTONA_BLOCK_IP_1588
};
#endif

/* Ts Phy Modes */
typedef enum {
    VTSS_PHY_TS_OPER_MODE_LAN = 0, /* LAN Mode     */
    VTSS_PHY_TS_OPER_MODE_WAN = 1, /* WAN Mode     */
    VTSS_PHY_TS_OPER_MODE_1G =  2, /* 1G Mode     */
    VTSS_PHY_TS_OPER_MODE_INV = 3  /* Invalid Mode */
} vtss_phy_ts_oper_mode_t;

#define  VTSS_PHY_TS_LATENCY_IDX_MODE_MAX VTSS_PHY_TS_OPER_MODE_INV

#ifdef VTSS_CHIP_10G_PHY
static  u32 latency_ingr_10g[VTSS_PHY_TS_LATENCY_IDX_MODE_MAX][VTSS_PHY_TS_CLOCK_FREQ_MAX] = {
    {   /* LAN Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 112,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 109,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 107, /* To Be determined */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 106,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 100, /* To Be determined */
    }, { /* WAN Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 155,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 152,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 150, /* To Be determined */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 149,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 143 /* To Be determined */
    }, { /* 1G Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 0 /* TBD */
    }
};
static   u32 latency_egr_10g[VTSS_PHY_TS_LATENCY_IDX_MODE_MAX][VTSS_PHY_TS_CLOCK_FREQ_MAX] = {
    {   /* LAN Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 200,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 202,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 204, /* To Be determined */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 206,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 212, /* To Be determined */
    }, { /* WAN Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 207,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 210,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 211, /* To Be determined */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 213,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 219 /* To Be determined */
    }, { /* 1G Mode */
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 0, /* TBD */
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 0 /* TBD */
    }
};
#endif /* VTSS_CHIP_10G_PHY */

#ifdef VTSS_CHIP_CU_PHY
/* Only for LAN mode */
static  u32 latency_ingr_1g[VTSS_PHY_TS_CLOCK_FREQ_MAX - 1 /* don't support 500MHz */] = {
    [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 12,
    [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 10,
    [VTSS_PHY_TS_CLOCK_FREQ_200M]   =  8,
    [VTSS_PHY_TS_CLOCK_FREQ_250M]   =  6,
};
static  u32 latency_egr_1g[VTSS_PHY_TS_CLOCK_FREQ_MAX - 1 /* don't support 500MHz */] = {
    [VTSS_PHY_TS_CLOCK_FREQ_125M]   =  0,
    [VTSS_PHY_TS_CLOCK_FREQ_15625M] =  2,
    [VTSS_PHY_TS_CLOCK_FREQ_200M]   =  5,
    [VTSS_PHY_TS_CLOCK_FREQ_250M]   =  6,
};
#endif /* VTSS_CHIP_CU_PHY */

#ifdef VTSS_ARCH_DAYTONA
static  u32 latency_ingr_daytona[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEE_W]          = 178,
    [VTSS_CONFIG_MODE_PEE_MW]         = 178,
    [VTSS_CONFIG_MODE_PEE_P]          = 117,
    [VTSS_CONFIG_MODE_PEE_MP]         = 117,
    [VTSS_CONFIG_MODE_PEO_MWA]        = 10415,
    [VTSS_CONFIG_MODE_PEO_WA]         = 10412,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 10417,
    [VTSS_CONFIG_MODE_PEO_WS]         = 10416,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 9758,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 9761,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 9756,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 9803,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 9784,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 9764,
};
static  u32 latency_egr_daytona[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEE_W]          = 275,
    [VTSS_CONFIG_MODE_PEE_MW]         = 275,
    [VTSS_CONFIG_MODE_PEE_P]          = 240,
    [VTSS_CONFIG_MODE_PEE_MP]         = 240,
    [VTSS_CONFIG_MODE_PEO_MWA]        = 368,
    [VTSS_CONFIG_MODE_PEO_WA]         = 365,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 370,
    [VTSS_CONFIG_MODE_PEO_WS]         = 369,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 101,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 104,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 99,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 105,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 127,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 107,
};

#ifdef VTSS_FEATURE_OTN
static  u32 latency_ingr_daytona_i4fec[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEE_W]          = 178,
    [VTSS_CONFIG_MODE_PEE_MW]         = 178,
    [VTSS_CONFIG_MODE_PEE_P]          = 117,
    [VTSS_CONFIG_MODE_PEE_MP]         = 117,
    [VTSS_CONFIG_MODE_PEO_MWA]        = 36609,
    [VTSS_CONFIG_MODE_PEO_WA]         = 36603,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 36609,
    [VTSS_CONFIG_MODE_PEO_WS]         = 36611,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 35040,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 35044,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 35039,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 35020,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 35173,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 35047,
};

static  u32 latency_egr_daytona_i4fec[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEE_W]          = 275,
    [VTSS_CONFIG_MODE_PEE_MW]         = 275,
    [VTSS_CONFIG_MODE_PEE_P]          = 240,
    [VTSS_CONFIG_MODE_PEE_MP]         = 240,
    [VTSS_CONFIG_MODE_PEO_MWA]        = 26562,
    [VTSS_CONFIG_MODE_PEO_WA]         = 26556,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 26562,
    [VTSS_CONFIG_MODE_PEO_WS]         = 26564,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 25383,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 25387,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 25382,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 25322,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 25516,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 25390,
};
/* Actual latency values expected
static  u32 latency_ingr_daytona_i7fec[VTSS_CONFIG_MODE_MAX] = {
            [VTSS_CONFIG_MODE_PEO_MWA]        = 73547,
            [VTSS_CONFIG_MODE_PEO_WA]         = 73552,
            [VTSS_CONFIG_MODE_PEO_MWS]        = 73553,
            [VTSS_CONFIG_MODE_PEO_WS]         = 73548,
            [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 70697,
            [VTSS_CONFIG_MODE_PEO_P2E_20]     = 70701,
            [VTSS_CONFIG_MODE_PEO_P2E_100]    = 70698,
            [VTSS_CONFIG_MODE_PEO_P1E_100]    = 70999,
            [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 70749,
            [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 70744,
    };

static  u32 latency_egr_daytona_i7fec[VTSS_CONFIG_MODE_MAX] = {
            [VTSS_CONFIG_MODE_PEO_MWA]        = 63500,
            [VTSS_CONFIG_MODE_PEO_WA]         = 63505,
            [VTSS_CONFIG_MODE_PEO_MWS]        = 63506,
            [VTSS_CONFIG_MODE_PEO_WS]         = 63501,
            [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 61040,
            [VTSS_CONFIG_MODE_PEO_P2E_20]     = 61044,
            [VTSS_CONFIG_MODE_PEO_P2E_100]    = 61041,
            [VTSS_CONFIG_MODE_PEO_P1E_100]    = 61301,
            [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 61092,
            [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 61087,
    };*/

static  u32 latency_ingr_daytona_i7fec[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEO_MWA]        = 64947,
    [VTSS_CONFIG_MODE_PEO_WA]         = 64952,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 64953,
    [VTSS_CONFIG_MODE_PEO_WS]         = 64948,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 62097,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 62101,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 62098,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 62399,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 62149,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 62144,
};

static  u32 latency_egr_daytona_i7fec[VTSS_CONFIG_MODE_MAX] = {
    [VTSS_CONFIG_MODE_PEO_MWA]        = 54900,
    [VTSS_CONFIG_MODE_PEO_WA]         = 54905,
    [VTSS_CONFIG_MODE_PEO_MWS]        = 54906,
    [VTSS_CONFIG_MODE_PEO_WS]         = 54901,
    [VTSS_CONFIG_MODE_PEO_MP2E_20]    = 52440,
    [VTSS_CONFIG_MODE_PEO_P2E_20]     = 52444,
    [VTSS_CONFIG_MODE_PEO_P2E_100]    = 52441,
    [VTSS_CONFIG_MODE_PEO_P1E_100]    = 52701,
    [VTSS_CONFIG_MODE_TEO_PMP_1E]     = 52492,
    [VTSS_CONFIG_MODE_TEO_PMP_2E]     = 52487,
};

#endif /* VTSS_FEATURE_OTN */

static vtss_rc vtss_daytona_chip_id_get_priv(vtss_state_t *vtss_state,
                                             vtss_chip_id_t *const chip_id);
#endif

static vtss_rc vtss_phy_ts_rev_get_priv(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no,
                                        u16              *const revision);

#ifdef VTSS_CHIP_CU_PHY
vtss_rc vtss_phy_id_get_priv(vtss_state_t *vtss_state,
                             const vtss_port_no_t   port_no,
                             vtss_phy_type_t *phy_id);
#endif

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
static vtss_rc vtss_phy_ts_channel_id_get_priv(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no,
                                               u16     *const channel_id);
static vtss_rc vtss_phy_ts_spi_pause_priv(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no);
static vtss_rc vtss_phy_ts_spi_unpause_priv(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no);

#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
/* pause_priv is used for private function where no need of vtss_exit */
#define VTSS_PHY_TS_SPI_PAUSE_PRIV(p) \
{ \
    vtss_rc err; \
    if (vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8574 || vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8572) { \
        if ((err = vtss_phy_ts_spi_pause_priv(vtss_state, (p))) != VTSS_RC_OK) { \
            VTSS_E("SPI pause fail!, port %u", (p)); \
            return err; \
        } \
    } \
}

#define VTSS_PHY_TS_SPI_PAUSE(p) \
{ \
    vtss_rc err; \
    if (vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8574 || vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8572) { \
        if ((err = vtss_phy_ts_spi_pause_priv(vtss_state, (p))) != VTSS_RC_OK) { \
            VTSS_E("SPI pause fail!, port %u", (p)); \
            VTSS_EXIT(); \
            return err; \
        } \
    } \
}
#else
#define VTSS_PHY_TS_SPI_PAUSE_PRIV(p) {}
#define VTSS_PHY_TS_SPI_PAUSE(p) {}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
/* unpause_priv is used for private function where no need of vtss_exit */
#define VTSS_PHY_TS_SPI_UNPAUSE_PRIV(p) \
{ \
    vtss_rc err; \
    if (vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8574 || vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8572) { \
        if ((err = vtss_phy_ts_spi_unpause_priv(vtss_state, (p))) != VTSS_RC_OK) { \
            VTSS_E("SPI Un-pause fail!, port %u", (p)); \
            return err; \
        } \
    } \
}

#define VTSS_PHY_TS_SPI_UNPAUSE(p) \
{ \
    vtss_rc err; \
    if (vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8574 || vtss_state->phy_ts_port_conf[(p)].new_spi_conf.phy_type == VTSS_PHY_TYPE_8572) { \
        if ((err = vtss_phy_ts_spi_unpause_priv(vtss_state, (p))) != VTSS_RC_OK) { \
            VTSS_E("SPI Un-pause fail!, port %u", (p)); \
            VTSS_EXIT(); \
            return err; \
        } \
    } \
}
#else
#define VTSS_PHY_TS_SPI_UNPAUSE_PRIV(p) {}
#define VTSS_PHY_TS_SPI_UNPAUSE(p) {}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
#define VTSS_PHY_TS_SPI_PAUSE_COLD(p) \
{ \
    if (!vtss_state->warm_start_cur)  VTSS_PHY_TS_SPI_PAUSE(p) \
}
#else
#define VTSS_PHY_TS_SPI_PAUSE_COLD(p) {}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */



#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
#define VTSS_PHY_TS_SPI_UNPAUSE_COLD(p) \
{ \
    if (!vtss_state->warm_start_cur)  VTSS_PHY_TS_SPI_UNPAUSE(p) \
}
#else
#define VTSS_PHY_TS_SPI_UNPAUSE_COLD(p) {}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */

/* ================================================================= *
 *  Local functions
 * ================================================================= */
static vtss_rc vtss_phy_ts_register_access_type_get(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    u16 *const phy_type,
    BOOL *const clause45,
    vtss_phy_ts_oper_mode_t *oper_mode)
{
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_port_state_t  *ps_10g = &vtss_state->phy_10g_state[port_no];
    vtss_phy_10g_id_t          phy_id_10g;
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_port_state_t      *ps_1g = &vtss_state->phy_state[port_no];
    vtss_phy_type_t            phy_id_1g;
#endif
#ifdef VTSS_ARCH_DAYTONA
    vtss_chip_id_t             chip_id;
#endif

#ifdef VTSS_CHIP_10G_PHY
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get_priv(vtss_state, port_no, &phy_id_10g) == VTSS_RC_OK) {
        *phy_type = ps_10g->type;
        *clause45 = TRUE;
        if (ps_10g->mode.oper_mode == VTSS_PHY_LAN_MODE) {
            *oper_mode = VTSS_PHY_TS_OPER_MODE_LAN;
        } else if (ps_10g->mode.oper_mode == VTSS_PHY_WAN_MODE) {
            *oper_mode = VTSS_PHY_TS_OPER_MODE_WAN;
        } else if (ps_10g->mode.oper_mode == VTSS_PHY_1G_MODE) {
            *oper_mode = VTSS_PHY_TS_OPER_MODE_1G;
        } else {
            *oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
        }
        return VTSS_RC_OK;
    }
#endif

#ifdef VTSS_CHIP_CU_PHY
    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
    if (vtss_phy_id_get_priv(vtss_state, port_no, &phy_id_1g) == VTSS_RC_OK) {
        *phy_type = ps_1g->type.part_number;
        *clause45 = FALSE;
        *oper_mode = VTSS_PHY_TS_OPER_MODE_LAN;
        return VTSS_RC_OK;
    }
#endif

#ifdef VTSS_ARCH_DAYTONA
    if (vtss_daytona_chip_id_get_priv(vtss_state, &chip_id) == VTSS_RC_OK) {
        /* 1588 block is present in port 2 and 3 i.e. line side */
        if (port_no == 2 || port_no == 3) {
            *phy_type = chip_id.part_number;
            *clause45 = FALSE;
            *oper_mode = VTSS_PHY_TS_OPER_MODE_LAN; /*TODO: may need to change if mode reqd for Daytona */
            return VTSS_RC_OK;
        }
    }
#endif

    return VTSS_RC_ERROR;
}

static vtss_rc vtss_phy_ts_biu_address_map_get(
    const u16 dev_id,
    vtss_phy_ts_biu_addr_map_t **biu_addr_map_ptr)
{
    vtss_rc rc = VTSS_RC_ERROR;

    switch (dev_id) {
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8487];
        rc = VTSS_RC_OK;
        break;
    case VTSS_PHY_TYPE_8488:
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8488];
        rc = VTSS_RC_OK;
        break;
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490:
    case VTSS_PHY_TYPE_8491:
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8490];
        rc = VTSS_RC_OK;
        break;
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586:
        /* BIU address map is same for all 1G phys*/
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8574];
        rc = VTSS_RC_OK;
        break;
    case VTSS_PHY_TYPE_8572:
        /* BIU address map is same for all 1G phys*/
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8572];
        rc = VTSS_RC_OK;
        break;
#endif
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8492];
        rc = VTSS_RC_OK;
        break;
    case VTSS_DEV_TYPE_8494:
        *biu_addr_map_ptr = &vtss_phy_ts_biu_addr_map[VTSS_PHY_TS_CHIP_CONF_IDX_TYPE_8494];
        rc = VTSS_RC_OK;
        break;
#endif
    default:
        VTSS_E("unknown phy type %d", dev_id);
        break;
    }
    if (*biu_addr_map_ptr == NULL) {
        VTSS_E("unknown biu_addr_map_ptr");
        rc = VTSS_RC_ERROR;
    }
    return rc;
}

static vtss_rc vtss_phy_ts_read_csr(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no,
                                    const vtss_phy_ts_blk_id_t blk_id,
                                    const u16 csr_address,
                                    u32 *const value)
{
    u16                        phy_type = VTSS_PHY_TYPE_NONE;
    BOOL                       clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_port_no_t             cfg_port = port_no;
    vtss_phy_ts_blk_id_t       actual_blk_id = 0;
    vtss_phy_ts_biu_addr_map_t *biu_addr_map_ptr = NULL;
    vtss_port_no_t             base_port_no = 0;

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, cfg_port, &phy_type, &clause45, &oper_mode));
    VTSS_RC(vtss_phy_ts_biu_address_map_get(phy_type, &biu_addr_map_ptr));
    actual_blk_id = blk_id;
    if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done != FALSE) {
        /* Function Access */
        /* Input Port (cfg_port) = 25, 26
         * Block Id = Ingress Engine0, Egress Engine0
         *            Ingress Engine1, Egress Engine1
         *            Ingress Engine2, Egress Engine2
         *            Processor0
         */
        base_port_no = vtss_state->phy_ts_port_conf[cfg_port].base_port;
        if (base_port_no != cfg_port) {
            cfg_port = base_port_no;
            if (blk_id == VTSS_PHY_TS_PROC_BLK_ID(0)) {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(1);
            }
        }
    }

    switch (phy_type) {
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8491: {
        u16                 reg_value;
        vtss_mmd_read_t     mmd_read_func = vtss_state->init_conf.mmd_read;
        vtss_mmd_read_inc_t mmd_read_inc_func = vtss_state->init_conf.mmd_read_inc;
        u16                 reg_values[2];
        u16                 mmd_addr = 0;
        u16                 reg_addr = 0;

        if (!clause45) {
            return VTSS_RC_ERROR;
        }

        if ((blk_id == VTSS_PHY_TS_PROC_BLK_ID(0)) || (blk_id == VTSS_PHY_TS_PROC_BLK_ID(1))) {
            reg_value = 0;
            VTSS_RC(mmd_read_func(vtss_state, cfg_port, VTSS_PHY_TS_10G_DATA_CLK_MMD_ADDR,
                                  VTSS_PHY_TS_10G_DATA_CLK_CTRL_REG, &reg_value));
            if (reg_value & VTSS_PHY_TS_10G_DATA_CLK_SEL_MASK) {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(1);
            } else {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(0);
            }
        }
        /* If the port_no is base_port_no this call can also
         * be from CLI context actual_blk_id = blk_id
         */
        mmd_addr = biu_addr_map_ptr->mmd_addr;
        /* [15..0] bits Address */
        reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
        VTSS_RC(mmd_read_inc_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_values, 2));
        *value = reg_values[0] + (((u32)reg_values[1]) << 16);
        break;
    }
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490: {
        vtss_mmd_read_inc_t mmd_read_inc_func = vtss_state->init_conf.mmd_read_inc;
        u16                 reg_values[2];
        u16                 mmd_addr = 0;
        u16                 reg_addr = 0;
        if (!clause45) {
            return VTSS_RC_ERROR;
        }

        /* If the port_no is base_port_no this call can also
         * be from CLI context actual_blk_id = blk_id
         */
        mmd_addr = biu_addr_map_ptr->mmd_addr;
        /* [15..0] bits Address */
        reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
        VTSS_RC(mmd_read_inc_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_values, 2));
        *value = reg_values[0] + (((u32)reg_values[1]) << 16);
        break;
    }
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586: {
        u16                reg_value;
        u16                max_read;
        u16                reg_value_upper, reg_value_lower;
        vtss_miim_read_t   miim_read_func = vtss_state->init_conf.miim_read;
        vtss_miim_write_t  miim_write_func = vtss_state->init_conf.miim_write;

        /* Assume it is 1G or clause 22 Register Access */
        /* Command Execution : Enable the Extended Page Register
         * Register : 16, Bit:15    = 1: Execute the command
         *                            0: Command being Executed
         *                Bit:14    = 1: Execute a Read on CSR regisers
         *                            0: Execute a write on CSR registers
         *                Bit:13:11 = Target Block Id
         *                            000 - Analyzer 0 Ingress
         *                            001 - Analyzer 0 Egress
         *                            010 - Analyzer 1 Ingress
         *                            011 - Analyzer 1 Egress
         *                            100 - Analyzer 2 Ingress
         *                            101 - Analyzer 2 Egress
         *                            110 - Processor 0
         *                            111 - Processor 1
         *                Bit:10:0  = CSR register address [10:0]
         */
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
        if (vtss_state->phy_state[port_no].dce_port_init_done == TRUE) {
            if ((blk_id == 6 || blk_id == 7) && ((csr_address >= 0x10) && (csr_address <= 0x18) )) {
                break;
            }
        }
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
        /* 1588 - Page Selection */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_ADDR_EXT_REG, 0x1588));

        reg_value = (VTSS_PHY_TS_1G_BIU_ADDR_REG_EXE_CMD  |
                     VTSS_PHY_TS_1G_BIU_ADDR_REG_READ_CMD |
                     (actual_blk_id << 11) | csr_address);
        /* Write the reg_value which contains the read/write operation, target
         * Id and the CSR register address to register 16
         */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_BIU_ADDR_REG, reg_value));

        reg_value = 0;
        max_read = 0;
        do {
            max_read++;
            VTSS_RC(miim_read_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_BIU_ADDR_REG, &reg_value));
        } while ((!(reg_value & VTSS_PHY_TS_1G_BIU_ADDR_REG_EXE_CMD)) && (max_read < VTSS_PHY_TS_1G_REG_READ_MAX_CNT));
        /* Read the upper word (upper 16 bits) from register 18
         */
        VTSS_RC(miim_read_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_UPPER, &reg_value_upper));
        /* Read the lower word (lower 16 bits) from register 17
         */
        VTSS_RC(miim_read_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_LOWER, &reg_value_lower));
        /* Restore standard page
         */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_ADDR_EXT_REG, VTSS_PHY_PAGE_STANDARD));

        *value = ((reg_value_upper << 16) | reg_value_lower);
        VTSS_D("Read CSR: port %u, blk_id %d, adr %x, value %x", port_no, blk_id, csr_address, *value);
        break;
    }
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494: {
        u32 target, daytona_val;
        /* 1588 IP is present on line side i.e. valid for port 2 & 3 */
        if (port_no == 0 || port_no == 1) {
            return VTSS_RC_ERROR;
        }
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, daytona_blk_id_map[actual_blk_id], &target));
        DAYTONA_RD(target + ((4 * csr_address) & 0xFFFF), &daytona_val);
        *value = daytona_val;
        break;
    }
#endif /* VTSS_ARCH_DAYTONA */
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_write_csr(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no,
                                     const vtss_phy_ts_blk_id_t blk_id,
                                     const u16 csr_address,
                                     const u32 *const value)
{
    //VTSS_I("Write CSR: port %u, blk_id %d, adr %x, value %x", port_no, blk_id, csr_address, *value);
    u16                        phy_type = VTSS_PHY_TYPE_NONE;
    BOOL                       clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_port_no_t             cfg_port = port_no;
    vtss_phy_ts_blk_id_t       actual_blk_id = 0;
    vtss_phy_ts_biu_addr_map_t *biu_addr_map_ptr = NULL;
    vtss_port_no_t             base_port_no = 0;

#if defined(VTSS_CHIP_10G_PHY) || defined(VTSS_CHIP_CU_PHY)
    u16                        reg_value_upper, reg_value_lower;
    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    reg_value_lower = (*value & 0xffff);
    reg_value_upper = (*value >> 16);
#endif

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, cfg_port, &phy_type, &clause45, &oper_mode));
    VTSS_RC(vtss_phy_ts_biu_address_map_get(phy_type, &biu_addr_map_ptr));
    actual_blk_id = blk_id;
    if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done != FALSE) {
        /* Function Access */
        /* Input Port (cfg_port) = 25, 26
         * Block Id = Ingress Engine0, Egress Engine0
         *            Ingress Engine1, Egress Engine1
         *            Ingress Engine2, Egress Engine2
         *            Processor0
         */
        base_port_no = vtss_state->phy_ts_port_conf[cfg_port].base_port;
        if (base_port_no != cfg_port) {
            cfg_port = base_port_no;
            if (blk_id == VTSS_PHY_TS_PROC_BLK_ID(0)) {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(1);
            }
        }
    }

    switch (phy_type) {
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8491: {
        vtss_mmd_write_t  mmd_write_func = vtss_state->init_conf.mmd_write;
        u16               mmd_addr = 0;
        u16               reg_addr = 0;

        if (!clause45) {
            return VTSS_RC_ERROR;
        }
        base_port_no = vtss_state->phy_ts_port_conf[cfg_port].base_port;
        if (base_port_no != cfg_port) {
            cfg_port = base_port_no;
        }
        if ((blk_id != VTSS_PHY_TS_PROC_BLK_ID(0)) && (blk_id != VTSS_PHY_TS_PROC_BLK_ID(1))) {
            /* MMD Address */
            mmd_addr = biu_addr_map_ptr->mmd_addr;
            /* [15..0] bits Address */
            reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
            /* Write the Upper 2 Bytes */
            VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, (reg_addr | 1), reg_value_upper));
            /* Write the Lower 2 Bytes */
            VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_value_lower));
        } else if (((blk_id == VTSS_PHY_TS_PROC_BLK_ID(0)) || (blk_id == VTSS_PHY_TS_PROC_BLK_ID(1)))) {
            if (vtss_state->phy_ts_port_conf[base_port_no].xaui_sel_8487 & VTSS_PHY_TS_8487_XAUI_SEL_0) {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(0);
                /* MMD Address */
                mmd_addr = biu_addr_map_ptr->mmd_addr;
                /* [15..0] bits Address */
                reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
                /* Write the Upper 2 Bytes */
                VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, (reg_addr | 1), reg_value_upper));
                /* Write the Lower 2 Bytes */
                VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_value_lower));
            }
            if (vtss_state->phy_ts_port_conf[base_port_no].xaui_sel_8487 & VTSS_PHY_TS_8487_XAUI_SEL_1) {
                actual_blk_id = VTSS_PHY_TS_PROC_BLK_ID(1);
                /* MMD Address */
                mmd_addr = biu_addr_map_ptr->mmd_addr;
                /* [15..0] bits Address */
                reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
                /* Write the Upper 2 Bytes */
                VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, (reg_addr | 1), reg_value_upper));
                /* Write the Lower 2 Bytes */
                VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_value_lower));
            }
        }
        break;
    }
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490: {
        vtss_mmd_write_t  mmd_write_func = vtss_state->init_conf.mmd_write;
        u16               mmd_addr = 0;
        u16               reg_addr = 0;

        if (!clause45) {
            return VTSS_RC_ERROR;
        }
        /* If the port_no is base_port_no this call can also
         * be from CLI context actual_blk_id = blk_id
         */
        /* MMD Address */
        mmd_addr = biu_addr_map_ptr->mmd_addr;
        /* [15..0] bits Address */
        reg_addr = biu_addr_map_ptr->mdio_address[actual_blk_id] | (csr_address << 1);
        /* Write the Upper 2 Bytes */
        VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, (reg_addr | 1), reg_value_upper));
        /* Write the Lower 2 Bytes */
        VTSS_RC(mmd_write_func(vtss_state, cfg_port, mmd_addr, reg_addr, reg_value_lower));
        break;
    }
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586: {
        u16                max_read;
        u16                reg_value;
        u16                base_reg_value;
        BOOL               base_reg_update;
        vtss_miim_read_t   miim_read_func = vtss_state->init_conf.miim_read;
        vtss_miim_write_t  miim_write_func = vtss_state->init_conf.miim_write;

        /* Assume it is 1G :: Clause 22 Register Access */
        /* Command Execution : Enable the Extended Page Register
         * Register : 16, Bit:15    = 1: Execute the command
         *                            0: Command being Executed
         *                Bit:14    = 1: Execute a Read on CSR regisers
         *                            0: Execute a write on CSR registers
         *                Bit:13:11 = Target Block Id
         *                            000 - Analyzer 0 Ingress
         *                            001 - Analyzer 0 Egress
         *                            010 - Analyzer 1 Ingress
         *                            011 - Analyzer 1 Egress
         *                            100 - Analyzer 2 Ingress
         *                            101 - Analyzer 2 Egress
         *                            110 - Processor 0
         *                            111 - Processor 1
         *                Bit:10:0  = CSR register address [10:0]
         */
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
        if (vtss_state->phy_state[port_no].dce_port_init_done == TRUE) {
            if ((blk_id == 6 || blk_id == 7) && ((csr_address >= 0x10) && (csr_address <= 0x18) )) {
                break;
            }
        }
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
        base_reg_value = 0;
        base_reg_update = FALSE;
        /* Read basepage Reg-18 */
        VTSS_RC(miim_read_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_UPPER, &base_reg_value));

        /* 1588 - Page Selection */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_ADDR_EXT_REG, 0x1588));

        /* Write the upper word data (upper 16 bits) to register 18 */
        if ((blk_id == 6 || blk_id == 7) && (csr_address == 0x10 ||
                                             csr_address == 0x2e || csr_address == 0x4e ||
                                             csr_address == 0x2d || csr_address == 0x4d)) {
            if (!reg_value_upper) {
                VTSS_D("Ignore upper value for blk_id %d, csr_address %x", blk_id, csr_address);
            } else {
                base_reg_update = TRUE;
                VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_UPPER, reg_value_upper));
            }
        } else {
            VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_UPPER, reg_value_upper));
        }
        /* Write the lower word data (lower 16 bits) to register 17 */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_LOWER, reg_value_lower));

        reg_value = (VTSS_PHY_TS_1G_BIU_ADDR_REG_EXE_CMD  |
                     VTSS_PHY_TS_1G_BIU_ADDR_REG_WRITE_CMD |
                     (actual_blk_id << 11) | csr_address);
        /* Write the reg_value which contains the read/write operation, target
         * Id and the CSR register address to register 16.
         */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_BIU_ADDR_REG, reg_value));

        reg_value = 0;
        max_read  = 0;
        do {
            max_read++;
            VTSS_RC(miim_read_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_BIU_ADDR_REG, &reg_value));
        } while ((!(reg_value & VTSS_PHY_TS_1G_BIU_ADDR_REG_EXE_CMD)) && (max_read < VTSS_PHY_TS_1G_REG_READ_MAX_CNT));

        /* Restore standard page */
        VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_ADDR_EXT_REG, VTSS_PHY_PAGE_STANDARD));
        if (base_reg_update != FALSE) {
            VTSS_RC(miim_write_func(vtss_state, cfg_port, VTSS_PHY_TS_1G_CSR_DATA_UPPER, base_reg_value));
        }
        break;
    }
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494: {
        u32  target;
        /* 1588 IP is present on line side i.e. valid for port 2 & 3 */
        if (port_no == 0 || port_no == 1) {
            return VTSS_RC_ERROR;
        }
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, daytona_blk_id_map[actual_blk_id], &target));
        DAYTONA_WR(target + ((4 * csr_address) & 0xFFFF), *value);
        break;
    }
#endif /* VTSS_ARCH_DAYTONA */
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_phy_1588_csr_reg_write(const vtss_inst_t inst,
                                    const vtss_port_no_t port_no,
                                    const u32 blk_id,
                                    const u16 csr_address,
                                    const u32 *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_port_no_t cfg_port = port_no;
    u16  phy_type = 0;
    BOOL clause45 = FALSE;
    vtss_phy_ts_oper_mode_t oper_mode = VTSS_PHY_TS_OPER_MODE_INV;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode)) != VTSS_RC_OK) {
            break;
        }

#ifdef VTSS_CHIP_10G_PHY
        if ((phy_type == VTSS_PHY_TYPE_8487) || (phy_type == VTSS_PHY_TYPE_8488) ||
            (phy_type == VTSS_PHY_TYPE_8489) || (phy_type == VTSS_PHY_TYPE_8490) ||
            (phy_type == VTSS_PHY_TYPE_8491)) {
            if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &cfg_port)) != VTSS_RC_OK) {
                break;
            }
        }
#endif
#ifdef VTSS_CHIP_CU_PHY
        if ((phy_type == VTSS_PHY_TYPE_8574) ||
            (phy_type == VTSS_PHY_TYPE_8572) ||
            (phy_type == VTSS_PHY_TYPE_8575) ||
            (phy_type == VTSS_PHY_TYPE_8582) ||
            (phy_type == VTSS_PHY_TYPE_8584) ||
            (phy_type == VTSS_PHY_TYPE_8586)) {
            if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &cfg_port)) != VTSS_RC_OK) {
                break;
            }
        }
#endif

        rc = VTSS_PHY_TS_WRITE_CSR(cfg_port, blk_id, csr_address, value);
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_1588_csr_reg_read(const vtss_inst_t inst,
                                   const vtss_port_no_t port_no,
                                   const u32 blk_id,
                                   const u16 csr_address,
                                   u32 *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_port_no_t cfg_port = port_no;
    u16  phy_type = 0;
    BOOL clause45 = FALSE;
    vtss_phy_ts_oper_mode_t oper_mode = VTSS_PHY_TS_OPER_MODE_INV;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode)) != VTSS_RC_OK) {
            break;
        }

#ifdef VTSS_CHIP_10G_PHY
        if ((phy_type == VTSS_PHY_TYPE_8487) || (phy_type == VTSS_PHY_TYPE_8488) ||
            (phy_type == VTSS_PHY_TYPE_8489) || (phy_type == VTSS_PHY_TYPE_8490) ||
            (phy_type == VTSS_PHY_TYPE_8491)) {
            if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &cfg_port)) != VTSS_RC_OK) {
                break;
            }
        }
#endif
#ifdef VTSS_CHIP_CU_PHY
        if ((phy_type == VTSS_PHY_TYPE_8574) ||
            (phy_type == VTSS_PHY_TYPE_8572) ||
            (phy_type == VTSS_PHY_TYPE_8575) ||
            (phy_type == VTSS_PHY_TYPE_8582) ||
            (phy_type == VTSS_PHY_TYPE_8584) ||
            (phy_type == VTSS_PHY_TYPE_8586)) {
            if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &cfg_port)) != VTSS_RC_OK) {
                break;
            }
        }
#endif
        rc = VTSS_PHY_TS_READ_CSR(cfg_port, blk_id, csr_address, value);
    } while (0);
    VTSS_EXIT();

    return rc;
}

static vtss_rc vtss_phy_ts_ana_blk_id_get(vtss_phy_ts_engine_t eng_id,
                                          BOOL                 ingress,
                                          vtss_phy_ts_blk_id_t *blk_id)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (eng_id) {
    case VTSS_PHY_TS_PTP_ENGINE_ID_0:
        *blk_id = (ingress ? VTSS_PHY_TS_ANA_BLK_ID_ING(0) : VTSS_PHY_TS_ANA_BLK_ID_EGR(0));
        break;
    case VTSS_PHY_TS_PTP_ENGINE_ID_1:
        *blk_id = (ingress ? VTSS_PHY_TS_ANA_BLK_ID_ING(1) :
                   VTSS_PHY_TS_ANA_BLK_ID_EGR(1));
        break;
    case VTSS_PHY_TS_OAM_ENGINE_ID_2A:
    case VTSS_PHY_TS_OAM_ENGINE_ID_2B:
        *blk_id = (ingress ? VTSS_PHY_TS_ANA_BLK_ID_ING(2) :
                   VTSS_PHY_TS_ANA_BLK_ID_EGR(2));
        break;
    default:
        /* should never reach here */
        *blk_id = VTSS_PHY_TS_ANA_BLK_ID_ING(0);
        VTSS_E("Invalid engine id (%d), ingress %d", eng_id, ingress);
        rc = VTSS_RC_ERROR;
        break;
    }
    return rc;
}

#ifdef VTSS_CHIP_10G_PHY
vtss_rc vtss_phy_10g_id_get_priv(vtss_state_t *vtss_state,
                                 const vtss_port_no_t   port_no,
                                 vtss_phy_10g_id_t *const phy_id)
{
    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_10G_NONE) {
        return VTSS_RC_ERROR;
    }
    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
        phy_id->part_number = 0x8486;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) {
        phy_id->part_number = 0x8484;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488) {
        phy_id->part_number = 0x8488;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) {
        phy_id->part_number = 0x8487;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8489) {
        phy_id->part_number = 0x8489;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8490) {
        phy_id->part_number = 0x8490;
    } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8491) {
        phy_id->part_number = 0x8491;
    }

    phy_id->revision = vtss_state->phy_10g_state[port_no].revision;
    phy_id->channel_id = vtss_state->phy_10g_state[port_no].channel_id;
    phy_id->phy_api_base_no = vtss_state->phy_10g_state[port_no].phy_api_base_no;
    phy_id->type = vtss_state->phy_10g_state[port_no].type;
    phy_id->family = vtss_state->phy_10g_state[port_no].family;

    return VTSS_RC_OK;
}
#endif /* VTSS_CHIP_10G_PHY */

#ifdef VTSS_CHIP_CU_PHY
vtss_rc vtss_phy_id_get_priv(vtss_state_t *vtss_state,
                             const vtss_port_no_t   port_no,
                             vtss_phy_type_t *phy_id)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    if (ps->type.part_number == VTSS_PHY_TYPE_NONE) {
        rc = VTSS_RC_ERROR;
    } else {
        *phy_id = ps->type;
        VTSS_N("channel_id:%d, port_no:%d, phy_api_base_no:%d", phy_id->channel_id, port_no, phy_id->phy_api_base_no);
    }

    return rc;
}
#endif /* VTSS_CHIP_CU_PHY */

#ifdef VTSS_ARCH_DAYTONA
static vtss_rc vtss_daytona_chip_id_get_priv(vtss_state_t *vtss_state,
                                             vtss_chip_id_t *const chip_id)
{
    vtss_rc rc = VTSS_RC_OK;

    if ((vtss_state->misc.chip_id_get != NULL) &&
        ((rc = vtss_state->misc.chip_id_get(vtss_state, chip_id)) == VTSS_RC_OK)) {
        if ((chip_id->part_number == VTSS_DEV_TYPE_8492) || (chip_id->part_number == VTSS_DEV_TYPE_8494)) {
            return VTSS_RC_OK;
        }
    }
    return rc;
}
#endif

static vtss_rc vtss_phy_ts_base_port_get_priv(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no,
                                              vtss_port_no_t     *const base_port_no)
{
#ifdef VTSS_CHIP_10G_PHY
    vtss_phy_10g_id_t phy_id_10g;
#endif
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_type_t   phy_id_1g;
#endif
#ifdef VTSS_ARCH_DAYTONA
    vtss_chip_id_t chip_id;
#endif

#ifdef VTSS_CHIP_10G_PHY
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get_priv(vtss_state, port_no, &phy_id_10g) == VTSS_RC_OK) {
        /* Get base port_no for 10G */
        *base_port_no = phy_id_10g.phy_api_base_no;
        return VTSS_RC_OK;
    }
#endif
#ifdef VTSS_CHIP_CU_PHY
    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
    if (vtss_phy_id_get_priv(vtss_state, port_no, &phy_id_1g) == VTSS_RC_OK) {
        *base_port_no = phy_id_1g.phy_api_base_no;
        return VTSS_RC_OK;
    }
#endif

#ifdef VTSS_ARCH_DAYTONA
    if (vtss_daytona_chip_id_get_priv(vtss_state, &chip_id) == VTSS_RC_OK) {
        /* Daytona and Teledega(2 die and each has separate inst) 1588 block is
           present in line side. So port 2 and 3 out of which 2 is base port */
        if (port_no == 2 || port_no == 3) {
            *base_port_no = 2;
            return VTSS_RC_OK;
        }
    }
#endif
    VTSS_E("Invalid port (%d)", (u32)port_no);
    return VTSS_RC_ERROR;
}

static vtss_rc vtss_phy_ts_block_init(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u16                        phy_type = VTSS_PHY_TYPE_NONE;
    BOOL                       clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_port_no_t             base_port_no;

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode));
    VTSS_RC(vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no));

    switch (phy_type) {
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490:
    case VTSS_PHY_TYPE_8491:
        /* Fall Through */
    {
        u16               mmd_addr = (u16)0x1E;
        u16               reg_value = 0;
        u16               reg_addr = 0x001A;
        vtss_mmd_write_t  mmd_write_func = vtss_state->init_conf.mmd_write;
        vtss_mmd_read_t   mmd_read_func = vtss_state->init_conf.mmd_read;

        if (!clause45) {
            return VTSS_RC_ERROR;
        }
        VTSS_RC(mmd_read_func(vtss_state, base_port_no, mmd_addr, reg_addr, &reg_value));
        if (reg_value == 0xff01) {
            /* Already Enabled */
            return VTSS_RC_OK;
        } else {
            reg_value = 0xff01;
        }
        VTSS_RC(mmd_write_func(vtss_state, base_port_no, mmd_addr, reg_addr, reg_value));
        /* Note :: This is specific to the 8487
         * Configure the Load/save pin as input in the GPIO_1
         */
        reg_addr = 0x0102;
        VTSS_RC(mmd_read_func(vtss_state, base_port_no, mmd_addr, reg_addr, &reg_value));
        if (reg_value == 0x8100) {
            /* Already Enabled */
            return VTSS_RC_OK;
        } else {
            reg_value = 0x8100;
        }
        VTSS_RC(mmd_write_func(vtss_state, base_port_no, mmd_addr, reg_addr, reg_value));
        break;
    }
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586:
        /* initial setup of extended register 29 and 30 in 1588 extended page */
        /* 1588 - Page Selection */
        VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, base_port_no, VTSS_PHY_TS_1G_ADDR_EXT_REG, 0x1588));
        /* Write the lower word data to register 29 */
        VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, base_port_no, 29, 0x7ae0));
        /* Write the upper word data to register 30 */
        VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, base_port_no, 30, 0xb71c));
        break;
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494: {
        u32 target, value;
        /* Configure PCS_CFG to activate the 1588 data path */
        VTSS_RC(VTSS_FUNC_COLD(cil.pcs_10gbase_r_config_set, port_no));

        /* Configure the GPIO Pin Mux for 1PPS Output; pin 2 for 1PPS output
           for channel 0 and pin 10 for channel 1 */
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_DEVCPU_GCB, &target));
        /* offset for VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX is 0xe4 */
        DAYTONA_RD(target + ((4 * 0xe4) & 0xFFFF), &value);
        value |= ((port_no == 2) ? 0x01 : 0x02); /* bit 0 -> 1PPS_SEL_0, bit 1 -> 1PPS_SEL_1 */
        DAYTONA_WR(target + ((4 * 0xe4) & 0xFFFF), value);
        /* GPIO Pin Direction for 1PPS Out: output */
        VTSS_RC(vtss_state->misc.gpio_mode(vtss_state, 0, ((port_no == 2) ? 2 : 10), VTSS_GPIO_OUT));
        /* GPIO Pin Direction for 1PPS IN i.e. Load/Save: input */
        VTSS_RC(vtss_state->misc.gpio_mode(vtss_state, 0, 1, VTSS_GPIO_IN));
        break;
    }
#endif /* VTSS_ARCH_DAYTONA */
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
static vtss_rc vtss_phy_dce_port_init(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32  value = 0;
    u16  phy_type = 0;
    BOOL clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    u32  mii_protocol = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(1);  /* initialize to reserved value */

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode));

    value = 0x5000;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0), 0x10, &value));
    value = 0x1448;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0), 0x17, &value));

    value = VTSS_F_PTP_INGR_IP_1588_DF_INGR_DF_CTRL_INGR_DF_DEPTH(VTSS_PHY_TS_INGR_DF_DEPTH);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_DF_INGR_DF_CTRL, &value));

    value = VTSS_F_PTP_EGR_IP_1588_DF_EGR_DF_CTRL_EGR_DF_DEPTH(VTSS_PHY_TS_EGR_DF_DEPTH);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_DF_EGR_DF_CTRL, &value));

    /* Enable the interface control register */
    value = 0;
    /* Bit 6 :: CLK_ENA = 1, CLK_DIS = 0,   Init:CLK_ENA
     * Bit 2 :: BYPASS_DIS  = 0, BYPASS_ENA = 1, Init:BYPASS_ENA
     * Bit 1:0 :: MII protocol : 0 XGMII-64
     */

    mii_protocol =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(2);
    value = mii_protocol;
    value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_CLK_ENA;
    value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &value));
    value = 0x0;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0), 0x17, &value));
    vtss_state->phy_state[port_no].dce_port_init_done = TRUE;

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */

static vtss_rc vtss_phy_ts_port_init(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no,
                                     const vtss_phy_ts_init_conf_t *const conf)
{
    u32  value = 0, ingr_latency = 0, egr_latency = 0;
    u32  eg_cfg;
#ifdef VTSS_CHIP_10G_PHY
    u32  eg_stall_cfg = 0;
#endif /* VTSS_CHIP_10G_PHY */
    u32  stall_latency;
    u16  phy_type = 0;
    BOOL clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    u32  mii_protocol = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(1);  /* initialize to reserved value */
    u32  packet_mode  = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(0); /* XGMII-64 */

    u8  ltc_seq_a[] = {
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 8,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 6,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 5,
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 4,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 2,
    };
    u32 ltc_seq_e[] = {
        [VTSS_PHY_TS_CLOCK_FREQ_125M]   = 0L,
        [VTSS_PHY_TS_CLOCK_FREQ_15625M] = 400000L,
        [VTSS_PHY_TS_CLOCK_FREQ_200M]   = 0L,
        [VTSS_PHY_TS_CLOCK_FREQ_250M]   = 0L,
        [VTSS_PHY_TS_CLOCK_FREQ_500M]   = 0L,
    };
#ifdef VTSS_ARCH_DAYTONA
    vtss_port_no_t base_port_no;

    VTSS_RC(vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no));
#endif

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode));

    /* select the LTC clock source */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    /* set the clock source from the vtss_state
     */
    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_LTC_LTC_CTRL_LTC_CLK_SEL);
    value |= VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_CLK_SEL(conf->clk_src);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    /* get the LTC sequence configuration */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_SEQUENCE, &value));
    /* set the clock source from the vtss_state
     */
    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_LTC_LTC_SEQUENCE_LTC_SEQUENCE_A);
    value |= VTSS_F_PTP_IP_1588_LTC_LTC_SEQUENCE_LTC_SEQUENCE_A(ltc_seq_a[conf->clk_freq]);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_SEQUENCE, &value));
    /* get the sequence error
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_SEQ, &value));
    /* set the sequence error configuration
     */
    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_LTC_LTC_SEQ_LTC_SEQ_E);
    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_LTC_SEQ_LTC_SEQ_ADD_SUB);
    if (ltc_seq_e[conf->clk_freq]) {
        value |= VTSS_F_PTP_IP_1588_LTC_LTC_SEQ_LTC_SEQ_ADD_SUB;
    }
    value |= VTSS_F_PTP_IP_1588_LTC_LTC_SEQ_LTC_SEQ_E(ltc_seq_e[conf->clk_freq]);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_SEQ, &value));

#ifdef VTSS_CHIP_10G_PHY
    /* Enable the n-phase sampler */
    /* Need to enable n-phase sampler for applicable PHYs based on the subtype */
    if ((phy_type == VTSS_PHY_TYPE_8489) || (phy_type == VTSS_PHY_TYPE_8490) ||
        (phy_type == VTSS_PHY_TYPE_8491)) {
        u16                        reg_value;
        vtss_mmd_read_t            mmd_read_func;
        vtss_mmd_write_t           mmd_write_func;
        vtss_port_no_t base_port_no;
        mmd_read_func  = vtss_state->init_conf.mmd_read;
        mmd_write_func  = vtss_state->init_conf.mmd_write;

        VTSS_RC(vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no));
        VTSS_RC(mmd_read_func(vtss_state, base_port_no, VTSS_PHY_TS_10G_DATA_CLK_MMD_ADDR,
                              VTSS_PHY_TS_10G_FRC_DIS, &reg_value));
        reg_value = VTSS_PHY_TS_CLR_BITS(reg_value, 0x2);
        VTSS_RC(mmd_write_func(vtss_state, base_port_no, VTSS_PHY_TS_10G_DATA_CLK_MMD_ADDR,
                               VTSS_PHY_TS_10G_FRC_DIS, reg_value));

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_ACC_BYPASS);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_ACC_BYPASS);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_ACC_BYPASS);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_INGR_SOF_ACC_BYPASS);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LOAD_SAVE_ACC_BYPASS);
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_ERR;

        /* Enable the n-phase sampler */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));

        /* Trigger the re-calibration after LTC_SEQUENCE setting */
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_TRIG;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));

        /* Clear the recalibration, ERR and DONE bits */
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_TRIG);
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_ERR;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_DONE;
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_ERR;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));

        /* Trigger the re-calibration again */
        value |= VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_TRIG;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
        /* Clear the recalibration */
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_TRIG);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value));
    }
#endif

    /* set the ingress and egress delay FIFO depth register */
    switch (phy_type) {
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586:
        value = VTSS_F_PTP_INGR_IP_1588_DF_INGR_DF_CTRL_INGR_DF_DEPTH(VTSS_PHY_TS_INGR_DF_DEPTH);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_DF_INGR_DF_CTRL, &value));

        value = VTSS_F_PTP_EGR_IP_1588_DF_EGR_DF_CTRL_EGR_DF_DEPTH(VTSS_PHY_TS_EGR_DF_DEPTH);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_DF_EGR_DF_CTRL, &value));
        break;
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490:
    case VTSS_PHY_TYPE_8491:
        value = VTSS_F_PTP_INGR_IP_1588_DF_INGR_DF_CTRL_INGR_DF_DEPTH(0xF);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_DF_INGR_DF_CTRL, &value));

        value = VTSS_F_PTP_EGR_IP_1588_DF_EGR_DF_CTRL_EGR_DF_DEPTH(0xF);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_DF_EGR_DF_CTRL, &value));
        break;
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494:
        value = VTSS_F_PTP_INGR_IP_1588_DF_INGR_DF_CTRL_INGR_DF_DEPTH(VTSS_PHY_TS_INGR_DF_DEPTH);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_DF_INGR_DF_CTRL, &value));

        value = VTSS_F_PTP_EGR_IP_1588_DF_EGR_DF_CTRL_EGR_DF_DEPTH(VTSS_PHY_TS_EGR_DF_DEPTH);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_DF_EGR_DF_CTRL, &value));
        break;
#endif /* VTSS_ARCH_DAYTONA */
    default:
        /* Never reach here as phy_type check at the beginning! */
        break;
    }

    /* FIFO mode register */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));

    /* Set the serial FIFO mode */
    if (conf->tx_fifo_mode == VTSS_PHY_TS_FIFO_MODE_SPI) {
        value |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA;
    } else {
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA);
    }

#ifdef VTSS_ARCH_DAYTONA
    if ((phy_type == VTSS_DEV_TYPE_8492) || (phy_type == VTSS_DEV_TYPE_8494)) {
        /* Set the frequency of SI_CLK to 40MHz */
        u32 base_si_cfg = 0;
        if (base_port_no != port_no) {
            /* SPI CLK is driven by channel-0 */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &base_si_cfg));
            base_si_cfg = VTSS_PHY_TS_CLR_BITS(base_si_cfg, VTSS_M_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_LO_CYCS);
            base_si_cfg |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_LO_CYCS(0x04);
            base_si_cfg = VTSS_PHY_TS_CLR_BITS(base_si_cfg, VTSS_M_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_HI_CYCS);
            base_si_cfg |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_HI_CYCS(0x04);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                          VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &base_si_cfg));
        }

        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_LO_CYCS);
        value |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_LO_CYCS(0x04);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_HI_CYCS);
        value |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_SI_CLK_HI_CYCS(0x04);
    }
#endif
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));

#ifdef VTSS_ARCH_DAYTONA
    if ((phy_type == VTSS_DEV_TYPE_8492) || (phy_type == VTSS_DEV_TYPE_8494)) {
        u32 target;
        /* Configure GPIO Pin to use for SPI: GPIO pin 0, 8 and 9 is used for SPI */
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_DEVCPU_GCB, &target));
        value = 0;
        /* offset for VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX is 0xe4 */
        DAYTONA_RD(target + ((4 * 0xe4) & 0xFFFF), &value);
        value |= 0x04; /* IP1588_SI_SEL is bit 2 */
        DAYTONA_WR(target + ((4 * 0xe4) & 0xFFFF), value);

        /* GPIO Pin Direction: pin 0, 8 & 9 to be in output mode */
        VTSS_RC(vtss_state->misc.gpio_mode(vtss_state, 0, 0, VTSS_GPIO_OUT));
        VTSS_RC(vtss_state->misc.gpio_mode(vtss_state, 0, 8, VTSS_GPIO_OUT));
        VTSS_RC(vtss_state->misc.gpio_mode(vtss_state, 0, 9, VTSS_GPIO_OUT));
    }
#endif

    /* Set the Rx timestamp position */
    if (conf->rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
        /* ingress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
        /* No preamble modification */
        value &= (~VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_REDUCE_PREAMBLE);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
#if 0
        /* preamble shrink not supported in egress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
        value &= (~VTSS_F_PTP_EGR_IP_1588_RW_EGR_RW_CTRL_EGR_RW_REDUCE_PREAMBLE);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
#endif
    } else {
        /* ingress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
        /* Reduce preamble by 4 bytes */
        value |= VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_REDUCE_PREAMBLE;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
#if 0
        /* preamble shrink not supported in egress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
        value |= VTSS_F_PTP_EGR_IP_1588_RW_EGR_RW_CTRL_EGR_RW_REDUCE_PREAMBLE;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
#endif
    }
    if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
        /* ingress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
        /* Byte offset of the bit to be modified */
        value |= VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_FLAG_BIT(7);
        /*Value to be written to the bit "1"*/
        value |= VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_FLAG_VAL;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
        /* Egress */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
        /* Byte offset of the bit to be modified */
        value |= VTSS_F_PTP_EGR_IP_1588_RW_EGR_RW_CTRL_EGR_RW_FLAG_BIT(7);
        /*Value to be written to the bit "0"*/
        value &= ~VTSS_F_PTP_EGR_IP_1588_RW_EGR_RW_CTRL_EGR_RW_FLAG_VAL;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_RW_EGR_RW_CTRL, &value));
    }

    /* Time Stamp length configuration
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    /* Enable the Ingress timestamp length configuration
     */
    if (conf->rx_ts_len == VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT) {
        value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_FRACT_NS_MODE;
    } else {
        value  = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_FRACT_NS_MODE);
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
    /* Enable the Egress timestamp length configuration
     */
    if (conf->rx_ts_len == VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT) {
        value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_FRACT_NS_MODE;
    } else {
        value  = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_FRACT_NS_MODE);
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
    if (conf->auto_clear_ls) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));
        value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_AUTO_CLR;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));
    }
    /* ingress prediction block enable */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_PREDICTOR_IG_CFG, &value));
    value |= VTSS_F_PTP_INGR_PREDICTOR_IG_CFG_IG_ENABLE;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_PREDICTOR_IG_CFG, &value));

    if (oper_mode == VTSS_PHY_TS_OPER_MODE_WAN) {
        value = VTSS_F_PTP_INGR_PREDICTOR_IG_PMA_TPMA(0x337);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_PREDICTOR_IG_PMA, &value));
    }

    /* egress prediction block enable */
    eg_cfg = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_PREDICTOR_EG_CFG, &eg_cfg));
    eg_cfg |= VTSS_F_PTP_EGR_PREDICTOR_EG_CFG_EG_ENABLE;
    switch (phy_type) {
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586:
        ingr_latency = latency_ingr_1g[conf->clk_freq];
        egr_latency  = latency_egr_1g[conf->clk_freq];
        mii_protocol =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(2);
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            mii_protocol =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(0);
            if (vtss_state->phy_ts_port_conf[port_no].macsec_ena == TRUE) {
                packet_mode  =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(4);
            } else {
                packet_mode  =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(2);
            }
        }
        break;
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490:
    /* Fall Through */
    case VTSS_PHY_TYPE_8491:
        eg_cfg |= VTSS_F_PTP_EGR_PREDICTOR_EG_CFG_WAF(0xc);
        eg_cfg |= VTSS_F_PTP_EGR_PREDICTOR_EG_CFG_PAF(0xc);
        /* TODO program WAF2, PAF2, WAF3, PAF3 */
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            eg_stall_cfg = 0;
            eg_stall_cfg |=  VTSS_F_PTP_EGR_STALL_PREDICTOR_EG_STALL_CFG_WAF3(0);
            eg_stall_cfg |=  VTSS_F_PTP_EGR_STALL_PREDICTOR_EG_STALL_CFG_PAF3(0);
            eg_stall_cfg |=  VTSS_F_PTP_EGR_STALL_PREDICTOR_EG_STALL_CFG_WAF2(0);
            eg_stall_cfg |=  VTSS_F_PTP_EGR_STALL_PREDICTOR_EG_STALL_CFG_PAF2(0);
        }
        ingr_latency = latency_ingr_10g[oper_mode][conf->clk_freq];
        egr_latency  = latency_egr_10g[oper_mode][conf->clk_freq];
        mii_protocol = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(0);
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            if (vtss_state->phy_ts_port_conf[port_no].macsec_ena == TRUE) {
                packet_mode  =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(4);
            } else {
                packet_mode  =  VTSS_F_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(0);
            }
        }
        break;
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494: {
        vtss_config_mode_t  channel_mode;

        VTSS_RC(daytona_port_2_mode(vtss_state, port_no, &channel_mode));
#ifdef VTSS_FEATURE_OTN
        switch (vtss_state->otn_state[port_no].och_fec.type) {
        case VTSS_OTN_OCH_FEC_NONE:
        case VTSS_OTN_OCH_FEC_RS:
            ingr_latency = latency_ingr_daytona[channel_mode];
            egr_latency  = latency_egr_daytona[channel_mode];
            break;
        case VTSS_OTN_OCH_FEC_I4:
            ingr_latency = latency_ingr_daytona_i4fec[channel_mode];
            egr_latency  = latency_egr_daytona_i4fec[channel_mode];
            break;
        case VTSS_OTN_OCH_FEC_I7:
            ingr_latency = latency_ingr_daytona_i7fec[channel_mode];
            egr_latency  = latency_egr_daytona_i7fec[channel_mode];
            break;
        default:
            VTSS_E("Latency values for %d type FEC not supported", vtss_state->otn_state[port_no].och_fec.type);
            break;
        }
#else
        ingr_latency = latency_ingr_daytona[channel_mode];
        egr_latency  = latency_egr_daytona[channel_mode];
#endif /* VTSS_FEATURE_OTN */
        mii_protocol = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_MII_PROTOCOL(0);
        break;
    }
#endif /* VTSS_ARCH_DAYTONA */
    default:
        /* Never reach here as phy_type check at the beginning! */
        break;
    }

    /* write egress prediction block enable */
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_PREDICTOR_EG_CFG, &eg_cfg));

    if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
#ifdef VTSS_CHIP_10G_PHY
        /* TODO program the predictor stall latency */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_STALL_PREDICTOR_EG_STALL_CFG, &eg_stall_cfg));
#endif /* VTSS_CHIP_10G_PHY */
        /* TODO program the egress stall latency */
        stall_latency = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_CFG_STAT_EGR_CFG_STALL_LATENCY, &stall_latency));
    }

    /* Load the default ingress latency in the ingress local latency register
     */
    value = ingr_latency;
    value = VTSS_F_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY_INGR_LOCAL_LATENCY(value);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY, &value));
    /* Load the Values in the ingress time stamp block
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    /* Load the default egress latency in the egress local latency register
     */
    value = egr_latency;
    value = VTSS_F_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY_EGR_LOCAL_LATENCY(value);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY, &value));
    /* Load the Values in the egress time stamp block
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
    value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));

    /* Set the Egress timestamp FIFO configuration and status register
     * Set timestamp bytes to 16 as default, it can be changed later
     * The TS FIFO popsout the data everytime we read the FIFO,
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR, &value));

    /* Default BIT::17 = 0, full 10 byte timestamp is stored,
     * Signature length is set to 16 bytes; Reset the FIFO
     * Generate the timestamp interrrupt for each timestamp update
     */
    value = VTSS_PHY_TS_CLR_BITS(value,
                                 VTSS_M_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_SIGNAT_BYTES);
    value |= VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_SIGNAT_BYTES(0x10);

    value |= VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_FIFO_RESET;
    value = VTSS_PHY_TS_CLR_BITS(value,
                                 VTSS_M_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_THRESH);
    value |= VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_THRESH(7);

    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR, &value));
    /* clear the reset*/
    value =  VTSS_PHY_TS_CLR_BITS(value,
                                  VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_FIFO_RESET);

    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR, &value));

    /* Enable the interface control register */
    value = 0;
    /* Bit 6 :: CLK_ENA = 1, CLK_DIS = 0,   Init:CLK_ENA
     * Bit 2 :: BYPASS_DIS  = 0, BYPASS_ENA = 1, Init:BYPASS_ENA
     * Bit 1:0 :: MII protocol : 0 XGMII-64
     */

    value = mii_protocol;
    value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_CLK_ENA;
    /* Below setting bypasses complete 1588 block for Gen1 devices
     * VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS
     */
    value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &value));
    value = packet_mode;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_TOP_CFG_STAT_MODE_CTL, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_analyzer_init_priv(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no)
{
    vtss_phy_ts_blk_id_t blk_id;
    vtss_phy_ts_engine_t eng_id;
    u32  value, i, dir;
    BOOL ingress;

    /* engine 0 and 1 reset: ingress and egress */
    for (dir = 0; dir < 2; dir++) {
        if (dir == 0) {
            ingress = TRUE;
        } else {
            ingress = FALSE;
        }
        for (eng_id = VTSS_PHY_TS_PTP_ENGINE_ID_0;
             eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1; eng_id++) {
            VTSS_RC(vtss_phy_ts_ana_blk_id_get(eng_id, ingress, &blk_id));

            /* ETH1 reset */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
            value = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG_ETH1_VLAN_TPID_CFG(0x88A8);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG, &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH, &value));
            for (i = 0; i < 8; i++) {
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
                value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
            }

            /* ETH2 reset */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
            value = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG_ETH2_VLAN_TPID_CFG(0x88A8);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG, &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH, &value));
            for (i = 0; i < 8; i++) {
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
                value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(0xFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
            }

            /* MPLS reset */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
            for (i = 0; i < 8; i++) {
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(0xFFFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(0xFFFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(0xFFFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(0xFFFFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
            }

            /* IP1 reset */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_FLOW_OFFSET(0x0C);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_MODE, &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_WIDTH(0x02);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
            if (ingress == FALSE) {
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG, &value));
            }
            for (i = 0; i < 8; i++) {
                value = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
            }

            /* IP2 reset */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));
            value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_MODE_IP2_FLOW_OFFSET(0x0C);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_MODE, &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_UPPER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_LOWER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_UPPER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_LOWER, &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_OFFSET_2, &value));
            value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_WIDTH(0x02);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG, &value));
            if (ingress == FALSE) {
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_FRAME_SIG_CFG, &value));
            }
            for (i = 0; i < 8; i++) {
                value = VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER(i), &value));
            }

            /* PTP/OAM reset */
            for (i = 0; i < 5; i++) {
                value = VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(0x03);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(i), &value));
                value = VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(0xFF);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(i), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(i), &value));
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_IP_CHKSUM_CTL_PTP_IP_CKSUM_SEL, &value));
        }

        /* Egress signature builder */
        if (ingress == FALSE) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
            value = VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_4(0x04);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_3(0x03);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_2(0x02);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_1(0x01);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0, &value));
            value = VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_9(0x09);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_8(0x08);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_7(0x07);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_6(0x06);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_5(0x05);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1, &value));
            value = VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_14(0x0E);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_13(0x0D);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_12(0x0C);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_11(0x0B);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_10(0x0A);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2, &value));
            value = VTSS_F_ANA_FRAME_SIG_CFG_FSB_MAP_REG_3_FSB_MAP_15(0x0F);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_3, &value));
        }
    }

    /* OAM engine reset: ingress and egress */
    for (dir = 0; dir < 2; dir++) {
        if (dir == 0) {
            ingress = TRUE;
        } else {
            ingress = FALSE;
        }
        VTSS_RC(vtss_phy_ts_ana_blk_id_get(VTSS_PHY_TS_OAM_ENGINE_ID_2A, ingress, &blk_id));

        /* ETH1 reset */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
        value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A_ETH1_VLAN_TPID_CFG_A(0x88A8);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A, &value));

        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
        value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B_ETH1_VLAN_TPID_CFG_B(0x88A8);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B, &value));

        for (i = 0; i < 8; i++) {
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(0x03);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
        }

        /* ETH2 reset */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        value = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A_ETH2_VLAN_TPID_CFG_A(0x88A8);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A, &value));

        for (i = 0; i < 8; i++) {
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(0x03);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(0xFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
        }

        /* MPLS reset */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        for (i = 0; i < 8; i++) {
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(0x03);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(0xFFFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(0xFFFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(0xFFFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(0xFFFFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
        }

        /* PTP/OAM reset */
        for (i = 0; i < 5; i++) {
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x03);
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(0x03);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(i), &value));
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(0xFF);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(i), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(i), &value));
        }
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            /* Egress signature builder */
            if (ingress == FALSE) {
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_CFG, &value));
                value =  VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_4(0x04);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_3(0x03);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_2(0x02);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0_FSB_MAP_1(0x01);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0, &value));
                value = VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_9(0x09);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_8(0x08);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_7(0x07);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_6(0x06);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1_FSB_MAP_5(0x05);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1, &value));
                value = VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_14(0x0E);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_13(0x0D);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_12(0x0C);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_11(0x0B);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2_FSB_MAP_10(0x0A);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2, &value));
                value = VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_3_FSB_MAP_15(0x0F);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_3, &value));
            }
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc warm_wr_masked(vtss_state_t *vtss_state, const vtss_port_no_t port_no, u32 blok, u32 addr, u32 value, u32 mask, const char *function, const u16 line)
{
    u32 val;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blok, addr, &val));
    if (vtss_state->sync_calling_private) { /* Warm start sync - check for changes */
        if ((val ^ value) & mask) { /* There is a change */
            VTSS_E("Warm start synch. field changed: port:%u blok:%d Register:0x%X  Mask:0x%X  from value:0x%X  to value:0x%X  by function:%s, line:%d",
                   port_no + 1, blok, addr, mask, val, value, function, line);
            val = (val & ~mask) | (value & mask);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blok, addr, &val));
        }
    } else {
        val = (val & ~mask) | (value & mask);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blok, addr, &val));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_set_priv(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no,
                                        const vtss_phy_ts_proc_conf_t conf)
{
    vtss_rc      rc = VTSS_RC_OK;
    u32          w_mask, mask, value = 0, ingr_latency = 0, egr_latency = 0;
    vtss_timeinterval_t ti;
    BOOL         enable = FALSE;
    vtss_phy_ts_event_t event = 0;
    u16  phy_type = 0;
    BOOL clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;


    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode));

    switch (phy_type) {
#ifdef VTSS_CHIP_CU_PHY
    case VTSS_PHY_TYPE_8574:
    case VTSS_PHY_TYPE_8572:
    case VTSS_PHY_TYPE_8575:
    case VTSS_PHY_TYPE_8582:
    case VTSS_PHY_TYPE_8584:
    case VTSS_PHY_TYPE_8586:
        ingr_latency = latency_ingr_1g[vtss_state->phy_ts_port_conf[port_no].clk_freq];
        egr_latency  = latency_egr_1g[vtss_state->phy_ts_port_conf[port_no].clk_freq];
        break;
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_CHIP_10G_PHY
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8488:
    case VTSS_PHY_TYPE_8489:
    case VTSS_PHY_TYPE_8490:
    case VTSS_PHY_TYPE_8491:
        ingr_latency = latency_ingr_10g[oper_mode][vtss_state->phy_ts_port_conf[port_no].clk_freq];
        egr_latency  = latency_egr_10g[oper_mode][vtss_state->phy_ts_port_conf[port_no].clk_freq];
        break;
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_ARCH_DAYTONA
    case VTSS_DEV_TYPE_8492:
    /* Fall Through */
    case VTSS_DEV_TYPE_8494: {
        vtss_config_mode_t  channel_mode;

        VTSS_RC(daytona_port_2_mode(vtss_state, port_no, &channel_mode));
#ifdef VTSS_FEATURE_OTN
        switch (vtss_state->otn_state[port_no].och_fec.type) {
        case VTSS_OTN_OCH_FEC_NONE:
        case VTSS_OTN_OCH_FEC_RS:
            ingr_latency = latency_ingr_daytona[channel_mode];
            egr_latency  = latency_egr_daytona[channel_mode];
            break;
        case VTSS_OTN_OCH_FEC_I4:
            ingr_latency = latency_ingr_daytona_i4fec[channel_mode];
            egr_latency  = latency_egr_daytona_i4fec[channel_mode];
            break;
        case VTSS_OTN_OCH_FEC_I7:
            ingr_latency = latency_ingr_daytona_i7fec[channel_mode];
            egr_latency = latency_egr_daytona_i7fec[channel_mode];
            break;
        default:
            VTSS_E("Latency values for %d type FEC not supported", vtss_state->otn_state[port_no].och_fec.type);
            break;
        }
#else
        ingr_latency = latency_ingr_daytona[channel_mode];
        egr_latency  = latency_egr_daytona[channel_mode];
#endif
        break;
    }
#endif /* VTSS_ARCH_DAYTONA */
    default:
        /* Never reach here as phy_type check at the beginning! */
        break;
    }

    switch (conf) {
    case VTSS_PHY_TS_ING_LATENCY_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].ingress_latency;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_16(ti);
        value += ingr_latency;
        value = VTSS_F_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY_INGR_LOCAL_LATENCY(value);
        /* update the ingress latency in the ingr local latency register
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY, &value));
        /* Load the Values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        break;
    case VTSS_PHY_TS_EGR_LATENCY_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].egress_latency;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_16(ti);
        value += egr_latency;
        value = VTSS_F_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY_EGR_LOCAL_LATENCY(value);
        /* update the egress latency in the egr local latency register
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY, &value));
        /* Load the Values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        break;
    case VTSS_PHY_TS_PATH_DELAY_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].path_delay;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32_NS(ti);
        /* update the path delay in both ingress and egress directions
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_PATH_DELAY, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_PATH_DELAY, &value));
        /* Load the ingress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        /* Load the egress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        break;
    case VTSS_PHY_TS_DELAY_ASYM_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].delay_asym;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32(ti);
        /* update the path delay in both ingress and egress directions
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_DELAY_ASYMMETRY, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_DELAY_ASYMMETRY, &value));
        /* Load the ingress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        /* Load the egress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        break;
    case VTSS_PHY_TS_RATE_ADJ_SET: {
        u64 val = 0;
        ti = vtss_state->phy_ts_port_conf[port_no].rate_adj;
        value = 0;
        if (ti != 0) {
            /* one Mia*2**16/|adj| */
            val = VTSS_DIV64(1000000000ULL * 0x10000ULL,  (u64)VTSS_LLABS(ti));
            if (val > 1000000000ULL) {
                val = 1000000000ULL;
            }
            value =
                VTSS_F_PTP_IP_1588_LTC_LTC_AUTO_ADJUST_LTC_AUTO_ADJUST_NS(val) |
                ((ti > 0) ?
                 VTSS_F_PTP_IP_1588_LTC_LTC_AUTO_ADJUST_LTC_AUTO_ADD_SUB_1NS(1) :
                 VTSS_F_PTP_IP_1588_LTC_LTC_AUTO_ADJUST_LTC_AUTO_ADD_SUB_1NS(2)) ;
        }
        /* update the ppb value in nano seconds to the auto adjust reg */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_LTC_AUTO_ADJUST, &value));
        /* read the auto adjust update value register */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
        /* The auto adjust update value is set to 0 after write operation
         * No need to clear the bit before | operation
         */
        value = value |
                VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_AUTO_ADJUST_UPDATE;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
        break;
    }
    case VTSS_PHY_TS_PORT_ENA_SET:
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &value));
        enable = vtss_state->phy_ts_port_conf[port_no].port_ena;
        if (enable) {
            /* ENABLE :: disable the bypass mode in the timestamp processor */
            /* For Gen1 VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS */
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS);
        } else {
            /* DISABLE :: enable the bypass mode in the timestamp processor */
            value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS;
        }
        /* update the interface control register */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &value));
        break;
    case VTSS_PHY_TS_PORT_EVT_MASK_SET:
        event = vtss_state->phy_ts_port_conf[port_no].event_mask;
        /* ingress interrupt mask register */
        mask = 0;
        if (event & VTSS_PHY_TS_INGR_ENGINE_ERR) {
            mask |= VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_ANALYZER_ERROR_MASK;
        }
        if (event & VTSS_PHY_TS_INGR_RW_PREAM_ERR) {
            mask |= VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_PREAMBLE_ERR_MASK;
        }
        if (event & VTSS_PHY_TS_INGR_RW_FCS_ERR) {
            mask |= VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_FCS_ERR_MASK;
        }

        vtss_state->phy_ts_port_conf[port_no].ingr_reg_mask = mask;
        w_mask = (VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_ANALYZER_ERROR_MASK | VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_PREAMBLE_ERR_MASK | VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_FCS_ERR_MASK);
        VTSS_RC(warm_wr_masked(vtss_state, port_no, VTSS_PHY_TS_PROC_BLK_ID(0), VTSS_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK, mask, w_mask, __FUNCTION__,  __LINE__));

        /* egress interrupt mask register */
        mask = 0;
        if (event & VTSS_PHY_TS_EGR_ENGINE_ERR) {
            mask |= VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_ANALYZER_ERROR_MASK;
        }
        if (event & VTSS_PHY_TS_EGR_RW_FCS_ERR) {
            mask |= VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_RW_FCS_ERR_MASK;
        }
        if (event & VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED) {
            mask |= VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_LOADED_MASK;
        }
        if (event & VTSS_PHY_TS_EGR_FIFO_OVERFLOW) {
            mask |= VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_OVERFLOW_MASK;
        }

        vtss_state->phy_ts_port_conf[port_no].egr_reg_mask = mask;
        w_mask = (VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_ANALYZER_ERROR_MASK | VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_RW_FCS_ERR_MASK | VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_LOADED_MASK | VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_OVERFLOW_MASK);
        VTSS_RC(warm_wr_masked(vtss_state, port_no, VTSS_PHY_TS_PROC_BLK_ID(0), VTSS_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK, mask, w_mask, __FUNCTION__,  __LINE__));

        break;
    case VTSS_PHY_TS_PORT_OPER_MODE_CHANGE_SET:
        /* set the correct Ingress Predictor value for WAN and LAN modes */
        value = 0x00;
        if (oper_mode == VTSS_PHY_TS_OPER_MODE_WAN) {
            value = VTSS_F_PTP_INGR_PREDICTOR_IG_PMA_TPMA(0x337);
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_PREDICTOR_IG_PMA, &value));

        break;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    case VTSS_PHY_TS_ING_DELAY_COMP_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].ingress_delay_comp;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32(ti);
        /* update the asymmetry in ingress direction
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_DELAY_ASYMMETRY, &value));
        /* Load the ingress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
        break;

    case VTSS_PHY_TS_EGR_DELAY_COMP_SET:
        ti = vtss_state->phy_ts_port_conf[port_no].egress_delay_comp;
        value = VTSS_PHY_TS_TIME_INTERVAL_ADJUST_32(ti);
        /* update asymmetry in egress direction
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_DELAY_ASYMMETRY, &value));
        /* Load the egress delay values in the time stamp block
         */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
        break;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
    case VTSS_PHY_TS_PPS_CONF_SET:
        /* TODO :Configure PPS_GEN_CNT value an integer multiple of LTC time period.*/
        value = vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_offset;
        value = VTSS_F_PTP_IP_1588_LTC_CFG_LTC_PPS_GEN_PPS_GEN_CNT(value);
        /* Write PPS_GEN_CNT value to the register
         */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_LTC_PPS_GEN, &value));
        value = vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_width_adj;
        if (value != 0) {
            value = VTSS_F_PTP_IP_1588_LTC_LTC_1PPS_WIDTH_ADJ_LTC_1PPS_WIDTH_ADJ(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                          VTSS_PTP_IP_1588_LTC_LTC_1PPS_WIDTH_ADJ, &value));
        }
        break;

    case VTSS_PHY_TS_ALT_CLK_SET:
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));

        if (vtss_state->phy_ts_port_conf[port_no].alt_clock_mode.pps_ls_lpbk) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_PPS_LOAD_SAVE_LPBK_EN;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_PPS_LOAD_SAVE_LPBK_EN);
        }

        if (vtss_state->phy_ts_port_conf[port_no].alt_clock_mode.ls_lpbk) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_LPBK_EN;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_LPBK_EN);
        }

        if (vtss_state->phy_ts_port_conf[port_no].alt_clock_mode.ls_pps_lpbk) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_PPS_LPBK_EN;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_PPS_LPBK_EN);
        }

        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));
        break;

    case VTSS_PHY_TS_SERTOD_SET:
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));

        if (vtss_state->phy_ts_port_conf[port_no].sertod_conf.ip_enable) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_SER_TOD_INPUT_EN;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_SER_TOD_INPUT_EN);
        }
        if (vtss_state->phy_ts_port_conf[port_no].sertod_conf.op_enable) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_SER_TOD_OUTPUT_EN;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_SER_TOD_OUTPUT_EN);
        }
        if (vtss_state->phy_ts_port_conf[port_no].sertod_conf.ls_inv) {
            value |=  VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_INV;
        } else {
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_SAVE_INV);
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));

        value = vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_width_adj;
        value = VTSS_F_PTP_IP_1588_LTC_LTC_1PPS_WIDTH_ADJ_LTC_1PPS_WIDTH_ADJ(value);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_LTC_1PPS_WIDTH_ADJ, &value));
        break;
    case VTSS_PHY_TS_LOAD_PULSE_DLY_SET:
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_PULSE_DLY);

        value |= VTSS_F_PTP_IP_1588_LTC_CFG_SER_TOD_INTF_LOAD_PULSE_DLY(vtss_state->phy_ts_port_conf[port_no].load_pulse_delay);

        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_SER_TOD_INTF, &value));
        break;

    default:
        rc = VTSS_RC_ERROR;
        break;
    }

    return rc;
}

static vtss_rc vtss_phy_ts_csr_ptptime_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    const vtss_phy_timestamp_t  *const ts)
{
    u32          value = 0;

    /* Write the timestamp in the phy*/
    value = ts->seconds.high;
    value = VTSS_F_PTP_IP_1588_LTC_LTC_LOAD_SEC_H_LTC_LOAD_SEC_H(value);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_LOAD_SEC_H, &value));
    value = ts->seconds.low;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_LOAD_SEC_L, &value));
    value = ts->nanoseconds;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_LOAD_NS, &value));
    /* Signal the PHY to Save the Timestamp from the LTC save registers
     * Read the LTC_CTRL, update the Load operation, write the regsiter
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    value = value | VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_LOAD_ENA;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_ptptime_get_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    vtss_phy_timestamp_t  *const ts)
{
    u32          value = 0;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));

    if ((vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) &&
        (vtss_state->phy_ts_port_conf[port_no].auto_clear_ls == TRUE)) {
        if (value & VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_SAVE_ENA) {
            return VTSS_RC_ERROR;
        }
    } else {
        /* clear the save enable */
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_SAVE_ENA);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    }

    /* Update the vtss_phy_timestamp_t struct */
    /* Read the timestamp from the phy*/
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_SAVED_SEC_H, &value));
    value = VTSS_X_PTP_IP_1588_LTC_LTC_SAVED_SEC_H_LTC_SAVED_SEC_H(value);
    ts->seconds.high = value;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_SAVED_SEC_L, &value));

    ts->seconds.low = value;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_SAVED_NS, &value));
    ts->nanoseconds = value;

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_load_ptptime_get_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    vtss_phy_timestamp_t  *const ts)
{
    u32          value = 0;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_LOAD_SEC_H, &value));
    ts->seconds.high = value;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_LOAD_SEC_L, &value));
    ts->seconds.low = value;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_LOAD_NS, &value));
    ts->nanoseconds = value;
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_alt_clock_saved_get_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    BOOL *const valid, u32 *const saved)
{
    u32          value = 0;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_CFG_LTC_PPS_RI, &value));

    *valid = (value & VTSS_F_PTP_IP_1588_LTC_CFG_LTC_PPS_RI_PPS_RI_VALID) ? TRUE : FALSE;
    *saved = VTSS_X_PTP_IP_1588_LTC_CFG_LTC_PPS_RI_PPS_RI_TIME(value);
    /* Clear PPS_RI_VALID */
    if (*valid) {
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_LTC_CFG_LTC_PPS_RI, &value));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_ptptime_arm_priv(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no)
{
    u32 value;

    /* Signal the PHY to load the Timestamp to LTC load register
     * Read the LTC_CTRL, update the Save operation, write the regsiter
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));

    value = value | VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_SAVE_ENA;

    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_ptptime_set_done_priv(vtss_state_t *vtss_state,
                                                     const vtss_port_no_t port_no)
{
    u32 value = 0;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    /* clear the load enable
     */
    value = VTSS_PHY_TS_CLR_BITS(value,
                                 VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_LOAD_ENA);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_event_poll_priv(vtss_state_t *vtss_state,
                                               const vtss_port_no_t  port_no,
                                               vtss_phy_ts_event_t   *const status)
{
    u32 mask, pending;

    *status = 0;
    /* read the sticky bits */
    //VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
    //                              VTSS_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK, &mask));
    mask = vtss_state->phy_ts_port_conf[port_no].ingr_reg_mask;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_STATUS, &pending));
    pending &= mask;    /* Only event on enabled sources */
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_STATUS, &pending));

    /* ingress interrupt mask register */
    if (pending & VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_ANALYZER_ERROR_MASK) {
        *status |= VTSS_PHY_TS_INGR_ENGINE_ERR;
    }
    if (pending & VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_PREAMBLE_ERR_MASK) {
        *status |= VTSS_PHY_TS_INGR_RW_PREAM_ERR;
    }
    if (pending & VTSS_F_PTP_INGR_IP_1588_CFG_STAT_INGR_INT_MASK_INGR_RW_FCS_ERR_MASK) {
        *status |= VTSS_PHY_TS_INGR_RW_FCS_ERR;
    }

    //VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
    //                                VTSS_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK, &mask));
    mask = vtss_state->phy_ts_port_conf[port_no].egr_reg_mask;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_STATUS, &pending));
    pending &= mask;    /* Only event on enabled sources */
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_STATUS, &pending));

    /* egress interrupt mask register */
    if (pending & VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_ANALYZER_ERROR_MASK) {
        *status |= VTSS_PHY_TS_EGR_ENGINE_ERR;
    }
    if (pending & VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_RW_FCS_ERR_MASK) {
        *status |= VTSS_PHY_TS_EGR_RW_FCS_ERR;
    }
    if (pending & VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_LOADED_MASK) {
        *status |= VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED;
    }
    if (pending & VTSS_F_PTP_EGR_IP_1588_CFG_STAT_EGR_INT_MASK_EGR_TS_OVERFLOW_MASK) {
        *status |= VTSS_PHY_TS_EGR_FIFO_OVERFLOW;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_stats_get_priv(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no,
                                          vtss_phy_ts_stats_t   *const statistics)
{
    u32          value = 0;


    memset(statistics, 0 , sizeof(vtss_phy_ts_stats_t));

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_RW_INGR_RW_PREAMBLE_ERR_CNT, &value));
    statistics->ingr_pream_shrink_err = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_RW_EGR_RW_PREAMBLE_ERR_CNT, &value));
    statistics->egr_pream_shrink_err = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_RW_INGR_RW_FCS_ERR_CNT, &value));
    statistics->ingr_fcs_err = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_RW_EGR_RW_FCS_ERR_CNT, &value));
    statistics->egr_fcs_err = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_RW_INGR_RW_MODFRM_CNT, &value));
    statistics->ingr_frm_mod_cnt = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_RW_EGR_RW_MODFRM_CNT, &value));
    statistics->egr_frm_mod_cnt = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_TX_CNT, &value));
    statistics->ts_fifo_tx_cnt = value;
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_DROP_CNT, &value));
    statistics->ts_fifo_drop_cnt = value;
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_correction_overflow_get_priv(vtss_state_t *vtss_state,
                                                        const vtss_port_no_t port_no,
                                                        BOOL  *const ingr_overflow,
                                                        BOOL  *const egr_overflow)
{
    u32          value = 0;

    *ingr_overflow = TRUE;
    *egr_overflow  = TRUE;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_STAT, &value));

    if (value & VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_STAT_INGR_CF_TOO_BIG_STICKY) {
        *ingr_overflow = FALSE;
    }

    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_STAT, &value));

    if (value & VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_STAT_EGR_CF_TOO_BIG_STICKY) {
        *egr_overflow = FALSE;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_csr_adj_1ns_set_priv(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                BOOL incr)
{
    u32          value = 0;

    /* read the auto adjust update value register */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    /* The auto adjust update value is set to 0 after write operation
     * No need to clear the bit before | operation
     */
    value |= VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_ADD_SUB_1NS_REQ;
    if (incr) {
        value |= VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_ADD_SUB_1NS;
    } else {
        value = VTSS_PHY_TS_CLR_BITS(value,
                                     VTSS_F_PTP_IP_1588_LTC_LTC_CTRL_LTC_ADD_SUB_1NS);
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_timeofday_offset_set_priv(vtss_state_t *vtss_state,
                                                     const vtss_port_no_t port_no,
                                                     i32 offset)
{
    u32          value = 0;

    //VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
    //            VTSS_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_STAT, &value));

    value |= abs(offset);
    value |= VTSS_F_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_LTC_OFFSET_ADJ;

    if (offset < 0) {
        value |= VTSS_F_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_LTC_ADD_SUB_OFFSET;
    }

    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ, &value));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ongoing_adjustment_priv(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t        port_no,
                                                   vtss_phy_ts_todadj_status_t *const ongoing_adjustment)
{
    u32          value = 0;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_STAT, &value));

    if (value & VTSS_F_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_STAT_LTC_OFFSET_LOAD_ERR) {
        *ongoing_adjustment = VTSS_PHY_TS_TODADJ_FAIL;
    } else if (value & VTSS_F_PTP_IP_1588_LTC_CFG_LTC_OFFSET_ADJ_STAT_LTC_OFFSET_DONE) {
        *ongoing_adjustment = VTSS_PHY_TS_TODADJ_DONE;
    } else {
        *ongoing_adjustment = VTSS_PHY_TS_TODADJ_INPROGRESS;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id);

static vtss_rc vtss_phy_ts_ip2_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id);

static vtss_rc vtss_phy_ts_eth1_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id);

static vtss_rc vtss_phy_ts_eth2_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id);

#define VTSS_PHY_TS_SIG_TIME_STAMP_LEN     10
#define VTSS_PHY_TS_SIG_SOURCE_PORT_ID_LEN 10
#define VTSS_PHY_TS_SIG_SEQUENCE_ID_LEN    2
#define VTSS_PHY_TS_SIG_DEST_IP_LEN        4
#define VTSS_PHY_TS_SIG_SRC_IP_LEN         4
#define VTSS_PHY_TS_SIG_DEST_MAC_LEN       6

static vtss_rc vtss_phy_ts_signature_set_priv(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no)
{
    u32 value = 0, pos = 0;
    i32 byte_ct = 0;
    u8 sig_sel[16];
    vtss_phy_ts_blk_id_t blk_id;
    vtss_phy_ts_engine_t engine_id, last_engine;
    vtss_phy_ts_encap_t  encap_type;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;

    /* Configure the signature bytes in each of the 3 engines */
    /* If the engine is not able to generate the signature than Ignore the sig_mask
     * E.G : getting the Source / Destination IP address from the Packet in case of
     * Engine 2A/2B
     */
    /* All these signature bits are based on the PTP header fields */
    /* Algorithm: 1. Go through all egress analyzers configuration and get
     *                the engine Id corresponding to the PTP engine.
     *            2. Based on the Signature mask bits select the bytes in
     *                the PTP header to extract the signature.
     */
    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask;
    memset(&sig_sel[0], 0, 16);


    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) {
        /* sequene_id is 2 Bytes long, it starts from 30th byte in PTP header
         * [0 - based byte count]
         * use 0, 1
         */
        sig_sel[pos++] = 0;
        sig_sel[pos++] = 1;
    }

    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) {
        /* source_port_id is 10 Bytes long, it starts from 20th byte in PTP header
         * [0 - based byte count]
         * use 2,3,4,5,6,7,8,9,10,11
         */
        for (byte_ct = 0; byte_ct < 10; byte_ct++) {
            /* Select Byte starting is 2 and ending is 11 */
            sig_sel[pos + byte_ct] = 2 + byte_ct;
        }
        pos += VTSS_PHY_TS_SIG_SOURCE_PORT_ID_LEN;
    }

    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) {
        /* domain_num is the 4th byte in PTP header [0 - based byte count]
         * use 25
         */
        sig_sel[pos++] = 25;

    }

    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) {
        /* msg_type is 1 nibble in 0 byte in PTP header [0 - based byte count]
         * use 26
         */
        sig_sel[pos++] = 26;
    }

    if (sig_mask & (VTSS_PHY_TS_FIFO_SIG_DEST_IP | VTSS_PHY_TS_FIFO_SIG_SRC_IP)) {
        /* configure both the IP comparators for all the three engines */
        /* read the auto adjust update value register */
        value = 0;
        /* Irrespective of the engine configuration just configure signature
         * bytes for both the IP comparators and the offset is always 12 bytes
         * that is the location in the ip header where the source and
         * destination IP address are stored, since 8 bytes are taken from the
         * starting byte of offset in the ip header for determination of the
         * signature
         */
        /* Signature bytes are always taken from the egress analyzer
         */
        /* IP comparater block is only present in engine-1 and engine-2
         */
        for (engine_id = VTSS_PHY_TS_PTP_ENGINE_ID_0; engine_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1;
             engine_id++) {
            encap_type = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].encap_type;
            VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, FALSE, &blk_id));
            if (vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].eng_used == TRUE) {

                if ((encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP)) {
                    VTSS_RC(vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, engine_id, blk_id));
                } else if (encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP) {
                    VTSS_RC(vtss_phy_ts_ip2_sig_mask_set_priv(vtss_state, port_no, engine_id, blk_id));
                }
            }
        }
        /* If source IP is selected for signature then use next position storing
         * the IP address */
        /* Note :: no need to check for the position because the length of the
         * signature is taken care before calling this function itself
         * All the checks that have been done are to satisfy LINT
         */
        if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
            /* Dest_IP is 4 Bytes long, it starts from 16th byte in IP header
             * [0 - based byte count]
             * use 28,29,30,31
             */
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 28;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 29;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 30;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 31;
        }

        if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
            /* src_IP is 4 Bytes long, it starts from 12th byte in IP header
             * [0 - based byte count]
             * use 32,33,34,35
             */
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 32;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 33;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 34;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 35;
        }
    }

    if (sig_mask & (VTSS_PHY_TS_FIFO_SIG_DEST_MAC)) {
        /* configure both the ETH comparators */
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            last_engine = VTSS_PHY_TS_OAM_ENGINE_ID_2B;
        } else {
            last_engine = VTSS_PHY_TS_PTP_ENGINE_ID_1;
        }

        for (engine_id = VTSS_PHY_TS_PTP_ENGINE_ID_0; engine_id <= last_engine;
             engine_id++) {
            encap_type = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].encap_type;
            VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, FALSE, &blk_id));
            if (vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].eng_used == TRUE) {

                if ((encap_type == VTSS_PHY_TS_ENCAP_ETH_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP) ||
                    (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP)) {
                    VTSS_RC(vtss_phy_ts_eth1_sig_mask_set_priv(vtss_state, port_no, engine_id, blk_id));
                } else if (((encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_PTP)      ||
                            (encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP)   ||
                            (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP) ||
                            (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP)) &&
                           (engine_id != VTSS_PHY_TS_OAM_ENGINE_ID_2B)) {
                    VTSS_RC(vtss_phy_ts_eth2_sig_mask_set_priv(vtss_state, port_no, engine_id, blk_id));
                }
            }
        }
        /* If Destination MAC is selected for signature then use next position storing
         * the MAC address */
        /* Note :: no need to check for the position because the length of the
         * signature is taken care before calling this function itself
         * All the checks that have been done are to satisfy LINT
         */
        if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
            /* Dest_MAC is 6 Bytes long
             * [0 - based byte count]
             * use 30,31,32,33,34,35
             */
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 35;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 34;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 33;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 32;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 31;
            VTSS_PHY_TS_ASSERT(pos > 15);
            sig_sel[pos++] = 30;
        }
    }

    /* configure the signature selection
     * This signature selection is only valid for Engine-1 and Engine-2
     */
    for (engine_id = VTSS_PHY_TS_PTP_ENGINE_ID_0; engine_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1;
         engine_id++) {
        encap_type = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].encap_type;
        VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, FALSE, &blk_id));

        if ((vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].eng_used == TRUE) &&
            ((encap_type == VTSS_PHY_TS_ENCAP_ETH_OAM) ||
             (encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_OAM) ||
             (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) ||
             (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM))) {
            continue;
        }
        value = 0;
        pos = 0;
        for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
            value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
        }
        pos += 5;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_0, &value));

        value = 0;
        for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
            value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
        }
        pos += 5;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_1, &value));

        value = 0;
        for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
            value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
        }
        pos += 5;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_2, &value));

        value = 0;
        value = (sig_sel[pos++] & 0x3f);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_MAP_REG_3, &value));
    }

    if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
        for (engine_id =  VTSS_PHY_TS_OAM_ENGINE_ID_2A; engine_id <= VTSS_PHY_TS_OAM_ENGINE_ID_2B;
             engine_id++) {
            encap_type = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].encap_type;
            VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, FALSE, &blk_id));

            if ((vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].eng_used == TRUE) &&
                (encap_type != VTSS_PHY_TS_ENCAP_ETH_OAM) &&
                (encap_type != VTSS_PHY_TS_ENCAP_ETH_ETH_OAM) &&
                (encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) &&
                (encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM)) {
                value = 0;
                pos = 0;
                for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
                    value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
                }
                pos += 5;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                              VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_0, &value));

                value = 0;
                for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
                    value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
                }
                pos += 5;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                              VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_1, &value));

                value = 0;
                for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
                    value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
                }
                pos += 5;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                              VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_2, &value));

                value = 0;
                for (byte_ct = 4; byte_ct >= 0; byte_ct--) {
                    value = (value << 6) | (sig_sel[pos + byte_ct] & 0x3f);
                }
                pos += 5;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                              VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_MAP_REG_3, &value));
            }
        }
    }

    return VTSS_RC_OK;
}

/* to avoid lint warnings for the VTSS_EXIT() ; VTSS_ENTER when calling out from the API */
/*lint -e{454, 455, 456} */

#define VTSS_PHY_TS_EXTRACT_BYTE(value,pos) ((value & ((u32)0xff << pos)) >> pos)

/*
 * TS FIFO service algorithm: 2R TSFIFO_0 with optimization as par TIMM algorithm
 */
static vtss_rc vtss_phy_ts_fifo_empty_priv(const vtss_inst_t inst,
                                           vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32   value = 0;
    u32   loop_cnt = 5;
    u32   depth = 0;
    u8    sig[26], id_cnt = 0;
    BOOL  entry_found = FALSE;
    vtss_rc rc = VTSS_RC_OK;
    u32    pos = 0;
    vtss_phy_timestamp_t        ts;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;
    vtss_phy_ts_fifo_sig_t      signature;
    vtss_phy_ts_fifo_status_t   status = VTSS_PHY_TS_FIFO_SUCCESS;
    u32   val_1st = 0, val_2nd = 0;

    vtss_phy_ts_fifo_read cb;
    void *cx;
    if (vtss_state->ts_fifo_cb == NULL) {
        return VTSS_RC_ERROR;
    }
    cb = vtss_state->ts_fifo_cb;
    cx = vtss_state->cntxt;
    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask;

    /* Step 1:: Loop reading the TSFIFO_0 register, until TS_EMPTY bit = 0 */
    do {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0, &val_1st));
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0, &val_2nd));

        if ((!(val_1st & VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TS_EMPTY)) &&
            (!(val_2nd & VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TS_EMPTY))) {
            /* Entries found */
            entry_found = TRUE;
            break;
        }
        loop_cnt--;
    } while (loop_cnt > 0);

    if (entry_found) {
        do {
            value = 0;
            pos = 0;
            /* Step 2:: Read the TSFIFO_0 register again to get valid timestamp[15:0] data and valid flags[2:0] data */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0, &value));

            if (value & VTSS_F_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TS_EMPTY) {
                break;
            }

            if (VTSS_X_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TS_FLAGS(value) != 7) {
                /* Partial time stamps are invalid, empty the FIFO */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_1, &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_2, &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_3, &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_4, &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_5, &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                             VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_6, &value));
                break;
            }
            memset(&signature, 0, sizeof(vtss_phy_ts_fifo_sig_t));
            memset(&ts, 0, sizeof(vtss_phy_timestamp_t));

            /* we only support 26 Byte Timestamp [16 Signature Bytes + 10 Ts Bytes]
             */

            sig[1] = (VTSS_X_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TSFIFO_0(value) & 0xff00) >> 8;
            sig[0] = VTSS_X_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_0_EGR_TSFIFO_0(value) & 0xff;
            /* Step 3:: Read the TSFIFO_1 to TSFIFO_6 registers to get valid timestamp[207:16] data;
                        must always read the TSFIFO_6 register and it must be read last */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_1, &value));
            sig[2] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[3] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[4] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[5] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_2, &value));
            sig[6] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[7] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[8] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[9] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_3, &value));
            sig[10] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[11] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[12] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[13] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_4, &value));
            sig[14] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[15] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[16] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[17] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_5, &value));
            sig[18] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[19] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[20] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[21] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            value = 0;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_6, &value));
            sig[22] = VTSS_PHY_TS_EXTRACT_BYTE(value, 0);
            sig[23] = VTSS_PHY_TS_EXTRACT_BYTE(value, 8);
            sig[24] = VTSS_PHY_TS_EXTRACT_BYTE(value, 16);
            sig[25] = VTSS_PHY_TS_EXTRACT_BYTE(value, 24);

            /* Step 4:: Read the TSFIFO_CSR register and check the value of TS_FIFO_LEVEL */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                         VTSS_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR, &value));

            depth = VTSS_X_PTP_EGR_IP_1588_TSFIFO_EGR_TSFIFO_CSR_EGR_TS_LEVEL(value);
            signature.sig_mask = sig_mask;
            ts.nanoseconds = (sig[3] << 24) | (sig[2] << 16) | (sig[1] << 8)  | sig[0];
            ts.seconds.low = (sig[7] << 24) | (sig[6] << 16) | (sig[5] << 8)  | sig[4];
            ts.seconds.high = (sig[9] << 8) | sig[8];

            pos += VTSS_PHY_TS_SIG_TIME_STAMP_LEN; /* 10 Byte Timestamp length */

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) {
                VTSS_PHY_TS_ASSERT((pos + VTSS_PHY_TS_SIG_SEQUENCE_ID_LEN) > 26); /* LINT */
                signature.sequence_id = (sig[pos + 1] << 8) | sig[pos];
                pos += VTSS_PHY_TS_SIG_SEQUENCE_ID_LEN;
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) {
                VTSS_PHY_TS_ASSERT((pos + VTSS_PHY_TS_SIG_SOURCE_PORT_ID_LEN) > 26); /* LINT */
                for (id_cnt = 0; id_cnt <= 9; id_cnt++) { /* 0 - 9, Total 10 Byte Source Port ID */
                    signature.src_port_identity[id_cnt] = sig[pos + (9 - id_cnt)];
                }
                pos += VTSS_PHY_TS_SIG_SOURCE_PORT_ID_LEN;
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) {
                VTSS_PHY_TS_ASSERT((pos + 1) > 26); /* LINT */
                signature.domain_num = sig[pos++];
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) {
                /* message_type field is only the lower nibble
                 */
                VTSS_PHY_TS_ASSERT((pos + 1) > 26); /* LINT */
                signature.msg_type = sig[pos++] & 0x0f;
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
                VTSS_PHY_TS_ASSERT((pos + VTSS_PHY_TS_SIG_DEST_IP_LEN) > 26); /* LINT */
                signature.dest_ip = (sig[pos + 3] << 24) | (sig[pos + 2] << 16) | (sig[pos + 1] << 8) | sig[pos];
                pos += VTSS_PHY_TS_SIG_DEST_IP_LEN;
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
                VTSS_PHY_TS_ASSERT((pos + VTSS_PHY_TS_SIG_SRC_IP_LEN) > 26); /* LINT */
                signature.src_ip = (sig[pos + 3] << 24) | (sig[pos + 2] << 16) | (sig[pos + 1] << 8) | sig[pos];
                pos += VTSS_PHY_TS_SIG_SRC_IP_LEN;
            }

            if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
                VTSS_PHY_TS_ASSERT((pos + VTSS_PHY_TS_SIG_DEST_MAC_LEN) > 26); /* LINT */
                for (loop_cnt = 0; loop_cnt < VTSS_PHY_TS_SIG_DEST_MAC_LEN; loop_cnt++) {
                    signature.dest_mac[loop_cnt] = sig[pos + loop_cnt];
                }
                pos += VTSS_PHY_TS_SIG_DEST_MAC_LEN;
            }

            VTSS_D("Time Stamp :: ");
            VTSS_D("    Seconds High :: %x ", ts.seconds.high);
            VTSS_D("    Seconds  Low :: %x ", (unsigned int)ts.seconds.low);
            VTSS_D("    Nano Seconds :: %x ", (unsigned int)ts.nanoseconds);
            VTSS_D("Signature  :: ");
            VTSS_D("    Message Type  :: %x ", signature.msg_type);
            VTSS_D("    Domain Number :: %x ", signature.domain_num);
            VTSS_D("    Source portId :: %x::%x::%x::%x::%x::%x::%x::%x::%x::%x", signature.src_port_identity[0], signature.src_port_identity[1],
                   signature.src_port_identity[2], signature.src_port_identity[3], signature.src_port_identity[4],
                   signature.src_port_identity[5], signature.src_port_identity[6], signature.src_port_identity[7],
                   signature.src_port_identity[8], signature.src_port_identity[9]);
            VTSS_D("    SequenceId    :: %x", signature.sequence_id);
            VTSS_D("    Source IP     :: %x", (unsigned int)signature.src_ip);
            VTSS_D("    Destination IP:: %x", (unsigned int)signature.dest_ip);
            VTSS_D("    Destination MAC :: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", signature.dest_mac[0],
                   signature.dest_mac[1],
                   signature.dest_mac[2],
                   signature.dest_mac[3],
                   signature.dest_mac[4],
                   signature.dest_mac[5]);

            status = VTSS_PHY_TS_FIFO_SUCCESS;
            /* avoid using vtss_state while outside the API lock, as the API may be called from an other thread */
            VTSS_EXIT();
            /* call out of the API */
            cb(inst, port_no, &ts, &signature, cx, status);
            VTSS_ENTER();
        } while (depth > 1);  /* Step 4a:: If TS_FIFO_LEVEL > 1, go back and repeat steps 2 through 4 */

        /* Step 4b:: If TS_FIFO_LEVEL = 1, finished handling the TS_FIFO */
    }
    return rc;
}

/* ================================================================= *
 *  Public functions
 * ================================================================= */
vtss_rc vtss_phy_ts_pps_conf_set(const vtss_inst_t inst,
                                 const vtss_port_no_t  port_no,
                                 const vtss_phy_ts_pps_conf_t *const phy_pps_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(phy_pps_conf == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("PPS configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].pps_conf = *phy_pps_conf;
            if (vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_offset > 0x3B9ACA00) {
                vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_offset = 0x3B9ACA00;
                VTSS_I("PPS offset value is more than allowed value setting to 0x3B9ACA00, port %u", port_no);
            }
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PPS_CONF_SET)) != VTSS_RC_OK) {
                VTSS_E("PPS Configuration set fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_pps_conf_get(const vtss_inst_t inst,
                                 const vtss_port_no_t  port_no,
                                 vtss_phy_ts_pps_conf_t *const phy_pps_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(phy_pps_conf == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("PPS configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            *phy_pps_conf = vtss_state->phy_ts_port_conf[port_no].pps_conf;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_alt_clock_mode_set(const vtss_inst_t inst,
                                       const vtss_port_no_t        port_no,
                                       const vtss_phy_ts_alt_clock_mode_t *const phy_alt_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(phy_alt_clock_mode == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Alternate Clock configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].alt_clock_mode = *phy_alt_clock_mode;
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_ALT_CLK_SET)) != VTSS_RC_OK) {
                VTSS_E("Latency set fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_alt_clock_mode_get(const vtss_inst_t inst,
                                       const vtss_port_no_t        port_no,
                                       vtss_phy_ts_alt_clock_mode_t *const phy_alt_clock_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(phy_alt_clock_mode == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Alternate Clock configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            *phy_alt_clock_mode = vtss_state->phy_ts_port_conf[port_no].alt_clock_mode;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_alt_clock_saved_get(const vtss_inst_t inst,
                                        const vtss_port_no_t        port_no,
                                        u32 *const saved)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    BOOL valid = FALSE;

    VTSS_PHY_TS_ASSERT(saved == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Alternate Clock configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (((rc = vtss_phy_ts_alt_clock_saved_get_priv(vtss_state, port_no, &valid, saved)) !=  VTSS_RC_OK) ||
                (valid == FALSE)) {
                VTSS_D("Alternate clock time get fail, port %u", port_no);
                rc = VTSS_RC_ERROR;
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_sertod_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               const vtss_phy_ts_sertod_conf_t *const sertod_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(sertod_conf == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Serial ToD not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].sertod_conf = *sertod_conf;
            /* Set the PPS width adjutment to 1uS for serial ToD */
            vtss_state->phy_ts_port_conf[port_no].pps_conf.pps_width_adj = 1000;
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_SERTOD_SET)) != VTSS_RC_OK) {
                VTSS_E("Serial ToD conf set fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_sertod_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_phy_ts_sertod_conf_t *const sertod_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(sertod_conf == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Serial ToD not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            *sertod_conf = vtss_state->phy_ts_port_conf[port_no].sertod_conf;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_loadpulse_delay_set(const vtss_inst_t inst,
                                        const vtss_port_no_t port_no,
                                        const u16 *const delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(delay == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Load Pulse Delay configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].load_pulse_delay = *delay;
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_LOAD_PULSE_DLY_SET)) != VTSS_RC_OK) {
                VTSS_E("Serial ToD conf set fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_loadpulse_delay_get(const vtss_inst_t inst,
                                        const vtss_port_no_t port_no,
                                        u16 *const delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(delay == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Load Pulse Delay configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            *delay = vtss_state->phy_ts_port_conf[port_no].load_pulse_delay;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}



vtss_rc vtss_phy_ts_ingress_latency_set(const vtss_inst_t     inst,
                                        const vtss_port_no_t  port_no,
                                        const vtss_timeinterval_t    *const latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(latency == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].ingress_latency = *latency;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_ING_LATENCY_SET)) != VTSS_RC_OK) {
                VTSS_E("Latency set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ingress_latency_get(const vtss_inst_t     inst,
                                        const vtss_port_no_t  port_no,
                                        vtss_timeinterval_t          *const latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(latency == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *latency = vtss_state->phy_ts_port_conf[port_no].ingress_latency;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_egress_latency_set(const vtss_inst_t     inst,
                                       const vtss_port_no_t  port_no,
                                       const vtss_timeinterval_t    *const latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(latency == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].egress_latency = *latency;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_EGR_LATENCY_SET)) != VTSS_RC_OK) {
                VTSS_E("Latency set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_egress_latency_get(const vtss_inst_t     inst,
                                       const vtss_port_no_t  port_no,
                                       vtss_timeinterval_t          *const latency)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(latency == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *latency = vtss_state->phy_ts_port_conf[port_no].egress_latency;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_path_delay_set(const vtss_inst_t     inst,
                                   const vtss_port_no_t  port_no,
                                   const vtss_timeinterval_t    *const path_delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(path_delay == NULL);

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].path_delay = *path_delay;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PATH_DELAY_SET)) != VTSS_RC_OK) {
                VTSS_E("Path_delay set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_path_delay_get(const vtss_inst_t     inst,
                                   const vtss_port_no_t  port_no,
                                   vtss_timeinterval_t          *const path_delay)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(path_delay == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *path_delay = vtss_state->phy_ts_port_conf[port_no].path_delay;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_delay_asymmetry_set(const vtss_inst_t    inst,
                                        const vtss_port_no_t port_no,
                                        const vtss_timeinterval_t   *const delay_asym)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(delay_asym == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].delay_asym = *delay_asym;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_DELAY_ASYM_SET)) != VTSS_RC_OK) {
                VTSS_E("Latency set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_ts_delay_asymmetry_get(const vtss_inst_t    inst,
                                        const vtss_port_no_t port_no,
                                        vtss_timeinterval_t         *const delay_asym)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(delay_asym == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *delay_asym = vtss_state->phy_ts_port_conf[port_no].delay_asym;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ptptime_set(const vtss_inst_t           inst,
                                const vtss_port_no_t        port_no,
                                const vtss_phy_timestamp_t  *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(ts == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_ptptime_set_priv(vtss_state, port_no, ts)) != VTSS_RC_OK) {
                VTSS_E("PTP time set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);

        }
    } while (0);

    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_ts_ptptime_arm(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_ptptime_arm_priv(vtss_state, port_no)) != VTSS_RC_OK) {
                VTSS_E("PTP time arm fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ptptime_set_done(const vtss_inst_t     inst,
                                     const vtss_port_no_t  port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_ptptime_set_done_priv(vtss_state, port_no)) != VTSS_RC_OK) {
                VTSS_E("PTP time set done fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_ts_ptptime_get(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no,
                                vtss_phy_timestamp_t  *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(ts == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_ptptime_get_priv(vtss_state, port_no, ts)) !=  VTSS_RC_OK) {
                VTSS_D("PTP time get fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_load_ptptime_get(const vtss_inst_t     inst,
                                     const vtss_port_no_t  port_no,
                                     vtss_phy_timestamp_t  *const ts)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(ts == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_load_ptptime_get_priv(vtss_state, port_no, ts)) !=  VTSS_RC_OK) {
                VTSS_E("PTP time get fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_clock_rateadj_set(const vtss_inst_t              inst,
                                      const vtss_port_no_t           port_no,
                                      const vtss_phy_ts_scaled_ppb_t *const adj)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(adj == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].rate_adj = *adj;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_RATE_ADJ_SET)) != VTSS_RC_OK) {
                VTSS_E("Rate Adj fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_clock_rateadj_get(const vtss_inst_t         inst,
                                      const vtss_port_no_t      port_no,
                                      vtss_phy_ts_scaled_ppb_t  *const adj)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(adj == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *adj = vtss_state->phy_ts_port_conf[port_no].rate_adj;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ptptime_adj1ns(const vtss_inst_t     inst,
                                   const vtss_port_no_t  port_no,
                                   const BOOL            incr)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_adj_1ns_set_priv(vtss_state, port_no, incr)) !=  VTSS_RC_OK) {
                VTSS_E("1ns adj fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_timeofday_offset_set(const vtss_inst_t     inst,
                                         const vtss_port_no_t  port_no,
                                         const i32   offset)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Tod offset configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if ((rc = vtss_phy_ts_timeofday_offset_set_priv(vtss_state, port_no, offset)) !=  VTSS_RC_OK) {
                VTSS_E("LTC offset set fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ongoing_adjustment(const vtss_inst_t           inst,
                                       const vtss_port_no_t        port_no,
                                       vtss_phy_ts_todadj_status_t *const ongoing_adjustment)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                VTSS_E("Tod offset configuration not supported on 1588 Gen1, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if ((rc = vtss_phy_ts_ongoing_adjustment_priv(vtss_state, port_no, ongoing_adjustment)) !=  VTSS_RC_OK) {
                VTSS_E("LTC adjustment status get fail, port %u", port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;

}



/* Note :: The application should take care of passing the correct signature mask
 * to both the base port number and also the alternate port in case of 8488-15.
 */
vtss_rc vtss_phy_ts_fifo_sig_set(const vtss_inst_t                 inst,
                                 const vtss_port_no_t              port_no,
                                 const vtss_phy_ts_fifo_sig_mask_t sig_mask)
{
    vtss_state_t   *vtss_state;
    vtss_rc        rc;
    u8             len = 0;
    vtss_port_no_t base_port_no = 0;

    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) {
        len += 1;    /* PTP Msg Type = 1Byte */
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) {
        len += 1;    /* PTP Dom Nm = 1Byte */
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) {
        len += 10;    /* SRC Port Identity = 10 Bytes */
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) {
        len += 2;    /* Sequence Number = 2 Bytes*/
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
        len += 4;    /* Dest IP = 4 Bytes */
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
        len += 4;    /* Src IP = 4 Bytes */
    }
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
        len += 6;    /* Dest MAC = 6 Bytes */
    }

    if (((sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) || (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP)) &&
        (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC)) {
        return VTSS_RC_ERROR;
    }

    if (len > 16) {
        return VTSS_RC_ERROR;
    }

    /* inst check done inside this func */
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].sig_mask = sig_mask;
            vtss_state->phy_ts_port_conf[base_port_no].sig_mask = sig_mask;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            /* configure the analyzer to extract the signature bytes from the packet
             */
            /* set the signature timestamp bytes based on the signature mask config */
            if ((rc = vtss_phy_ts_signature_set_priv(vtss_state, base_port_no)) != VTSS_RC_OK) {
                VTSS_E("Signature set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_fifo_sig_get(const vtss_inst_t            inst,
                                 const vtss_port_no_t         port_no,
                                 vtss_phy_ts_fifo_sig_mask_t  *const sig_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(sig_mask == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_fifo_empty(const vtss_inst_t       inst,
                               const vtss_port_no_t    port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode != VTSS_PHY_TS_FIFO_MODE_NORMAL) {
                rc = VTSS_RC_ERROR;
                break;
            }
            rc = vtss_phy_ts_fifo_empty_priv(inst, vtss_state, port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_fifo_read_install(const vtss_inst_t      inst,
                                      vtss_phy_ts_fifo_read  rd_cb,
                                      void                   *cntxt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->ts_fifo_cb = rd_cb;
        vtss_state->cntxt = cntxt;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_fifo_read_cb_get(const vtss_inst_t      inst,
                                     vtss_phy_ts_fifo_read  *rd_cb,
                                     void                   **cntxt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *rd_cb = vtss_state->ts_fifo_cb;
        *cntxt = vtss_state->cntxt;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_nphase_status_get(const vtss_inst_t     inst,
                                      const vtss_port_no_t  port_no,
                                      vtss_phy_ts_nphase_sampler_t sampler,
                                      vtss_phy_ts_nphase_status_t *const status)
{
    vtss_rc rc = VTSS_RC_OK;
    u32 value;
    VTSS_D("Entered %s", __FUNCTION__);
    if ((rc = vtss_phy_1588_csr_reg_read(inst, port_no, VTSS_PHY_TS_PROC_BLK_ID(0), VTSS_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS, &value)) == VTSS_RC_OK && status ) {
        switch (sampler) {
        case VTSS_PHY_TS_NPHASE_PPS_O:
            status->CALIB_ERR = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_ERR) ? TRUE : FALSE ;
            status->CALIB_DONE = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_O_CALIB_DONE) ? TRUE : FALSE ;
            break;
        case VTSS_PHY_TS_NPHASE_PPS_RI:
            status->CALIB_ERR = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_ERR) ? TRUE : FALSE ;
            status->CALIB_DONE = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_PPS_RI_CALIB_DONE) ? TRUE : FALSE ;
            break;
        case VTSS_PHY_TS_NPHASE_EGR_SOF:
            status->CALIB_ERR = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_ERR) ? TRUE : FALSE ;
            status->CALIB_DONE = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_EGR_SOF_CALIB_DONE) ? TRUE : FALSE ;
            break;
        case VTSS_PHY_TS_NPHASE_ING_SOF:
            status->CALIB_ERR =  (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_ERR) ? TRUE : FALSE ;
            status->CALIB_DONE =  (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_IGR_SOF_CALIB_DONE) ? TRUE : FALSE ;
            break;
        case VTSS_PHY_TS_NPHASE_LS:
            status->CALIB_ERR = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_ERR) ? TRUE : FALSE ;
            status->CALIB_DONE = (value & VTSS_F_PTP_IP_1588_ACC_CFG_ACC_CFG_STATUS_LS_CALIB_DONE) ? TRUE : FALSE ;
            break;
        default:
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_D("Exited %s", __FUNCTION__);
    return rc;
}

#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
vtss_rc vtss_phy_ts_ingress_delay_comp_set(const vtss_inst_t     inst,
                                           const vtss_port_no_t  port_no,
                                           const vtss_timeinterval_t    *const delay_comp)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_OK;

    VTSS_PHY_TS_ASSERT(delay_comp == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].ingress_delay_comp = *delay_comp;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_ING_DELAY_COMP_SET)) != VTSS_RC_OK) {
                VTSS_E("Ingress Delay compensation set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ingress_delay_comp_get(const vtss_inst_t     inst,
                                           const vtss_port_no_t  port_no,
                                           vtss_timeinterval_t          *const delay_comp)
{

    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_OK;

    VTSS_PHY_TS_ASSERT(delay_comp == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *delay_comp = vtss_state->phy_ts_port_conf[port_no].ingress_delay_comp;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_egress_delay_comp_set(const vtss_inst_t     inst,
                                          const vtss_port_no_t  port_no,
                                          const vtss_timeinterval_t    *const delay_comp)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_OK;

    VTSS_PHY_TS_ASSERT(delay_comp == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].egress_delay_comp = *delay_comp;
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            if ((rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_EGR_DELAY_COMP_SET)) != VTSS_RC_OK) {
                VTSS_E("Egress Delay Compensation set fail, port %u", port_no);
                /* don't break, needs to unpause */
            }
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_egress_delay_comp_get(const vtss_inst_t     inst,
                                          const vtss_port_no_t  port_no,
                                          vtss_timeinterval_t          *const delay_comp)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_OK;

    VTSS_PHY_TS_ASSERT(delay_comp == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *delay_comp = vtss_state->phy_ts_port_conf[port_no].egress_delay_comp;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */

/* ================================================================= *
 *  Engine functions
 * ================================================================= */
static vtss_phy_ts_eth_conf_t vtss_phy_ts_def_outer_eth_conf_for_pbb = {
    .comm_opt = {
        .pbb_en = TRUE,
        .tpid   = 0x88A8,
    },

    .flow_opt = {
        [0] = {
            .flow_en           = TRUE,
            .addr_match_mode   = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST |
            VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST,
            .addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR,
            .vlan_check        = TRUE,
            .num_tag           = 2,
            .outer_tag_type    = VTSS_PHY_TS_TAG_TYPE_B,
            .inner_tag_type    = VTSS_PHY_TS_TAG_TYPE_I,
        },
    },
};

static vtss_phy_ts_eth_conf_t vtss_phy_ts_def_outer_eth_conf = {
    .comm_opt = {
        .pbb_en = FALSE,
        .tpid   = 0x88A8,
    },

    .flow_opt = {
        [0] = {
            .flow_en           = TRUE,
            .addr_match_mode   = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST |
            VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST,
            .addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR,
            .vlan_check        = TRUE,
            .num_tag           = 0,
        },
    },
};

static vtss_phy_ts_eth_conf_t vtss_phy_ts_def_inner_eth_conf_for_ptp = {
    .comm_opt = {
        .pbb_en = FALSE,
        .tpid   = 0x88A8,
    },

    .flow_opt = {
        [0] = {
            .flow_en           = TRUE,
            .addr_match_mode   = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT,
            .addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR,
            .mac_addr          = {0x01, 0x1B, 0x19, 0x00, 0x00, 0x00},
            .vlan_check        = TRUE,
            .num_tag           = 0,
        },
    },
};

static vtss_phy_ts_eth_conf_t vtss_phy_ts_def_inner_eth_conf_for_oam = {
    .comm_opt = {
        .pbb_en = FALSE,
        .tpid   = 0x88A8,
    },

    .flow_opt = {
        [0] = {
            .flow_en           = TRUE,
            .addr_match_mode   = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT,
            .addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR,
            .mac_addr          = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x31},
            .vlan_check        = TRUE,
            .num_tag           = 0,
        },
    },
};

static vtss_phy_ts_mpls_conf_t vtss_phy_ts_def_mpls_conf = {
    .flow_opt = {
        [0] = {
            .flow_en         = TRUE,
            .stack_depth     = VTSS_PHY_TS_MPLS_STACK_DEPTH_1,
            .stack_ref_point = VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP,
        },
    },
};

static vtss_phy_ts_ach_conf_t vtss_phy_ts_def_ach_conf_for_ptp = {
    .comm_opt = {
        .version = {
            .value = 0,
            .mask  = 0x0F,
        },
        .channel_type = {
            .value = 0x0001,  /* MCC channel type */
            .mask  = 0xFFFF,
        },
        .proto_id = {
            .value = 0,
            .mask  = 0,
        },

    },
};

static vtss_phy_ts_ach_conf_t vtss_phy_ts_def_ach_conf_for_oam = {
    .comm_opt = {
        .version = {
            .value = 0,
            .mask  = 0x0F,
        },
        .channel_type = {
            .value = 0x8902, /* Proposed value for Y.1731 OAM */
            .mask  = 0xFFFF,
        },
        .proto_id = {
            .value = 0,
            .mask  = 0,
        },

    },
};

static vtss_phy_ts_ip_conf_t vtss_phy_ts_def_outer_ip_conf = {
    .comm_opt = {
        .ip_mode    = VTSS_PHY_TS_IP_VER_4,
    },

    .flow_opt = {
        [0] = {
            .flow_en    = TRUE,
            .match_mode = VTSS_PHY_TS_IP_MATCH_DEST,
            .ip_addr    = {
                .ipv4 = {
                    .addr = 0x00,
                    .mask = 0x00,
                },
            },
        },
    },
};

static vtss_phy_ts_ip_conf_t vtss_phy_ts_def_inner_ip_conf = {
    .comm_opt = {
        .ip_mode    = VTSS_PHY_TS_IP_VER_4,
        .sport_val  = 0,
        .sport_mask = 0,
        .dport_val  = 319,
        .dport_mask = 0xFFFF,
    },

    .flow_opt = {
        [0] = {
            .flow_en    = TRUE,
            .match_mode = VTSS_PHY_TS_IP_MATCH_DEST,
            .ip_addr    = {
                .ipv4 = {
                    .addr = 0xE0000181,
                    .mask = 0xFFFFFFFF,
                },
            },
        },
    },
};

static vtss_phy_ts_ptp_engine_action_t vtss_phy_ts_def_ptp_action = {
    .enable      = TRUE,

    .ptp_conf    = {
        .range_en = FALSE,
        .domain   = {
            .value = {
                .val  = 0,
                .mask = 0,
            },
        },
    },

    .clk_mode    = VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP,
    .delaym_type = VTSS_PHY_TS_PTP_DELAYM_E2E,
};

static vtss_phy_ts_oam_engine_action_t vtss_phy_ts_def_oam_action = {
    .enable      = TRUE,
    .y1731_en    = TRUE,

    .oam_conf = {
        .y1731_oam_conf = {
            .range_en = FALSE,
            .delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM,
            .meg_level   = {
                .value = {
                    .val  = 0,
                    .mask = 0,
                },
            },
        },
    },
};

/*
 * parameter to be passed into default config set for each engine
 */
typedef struct {
    const vtss_port_no_t          port_no;
    const vtss_port_no_t          base_port_no;
    const vtss_phy_ts_blk_id_t    blk_id;
    const vtss_phy_ts_engine_t    eng_id;
    const vtss_phy_ts_encap_t     encap_type;
    const u8                      flow_st_index;
    const u8                      flow_end_index;
} vtss_ts_engine_parm_t;

static vtss_rc vtss_phy_ts_eth1_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t        *eng_parm,
    vtss_phy_ts_eth_conf_t       *eth1_conf,
    const vtss_phy_ts_eth_conf_t *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, temp, i;


    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* Tag mode config */
        value = (def_conf->comm_opt.pbb_en ?
                 VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE_ETH1_PBB_ENA : 0);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE, &value));
        /* TPID config */
        value = def_conf->comm_opt.tpid;
        value = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG_ETH1_VLAN_TPID_CFG(value);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG, &value));

        for (i = flow_st_index; i <= flow_end_index; i++) {
            /* port map config */
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            /* config MATCH_MODE as per default value in DSH, remember to set
               VLAN_VERIFY_ENA to false */
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &value));
            /* set ADDR_MATCH_1 and ADDR_MATCH_2 as per DSH */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            value = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        if (def_conf->flow_opt[0].flow_en) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(flow_st_index), &value));
            value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(flow_st_index), &value));
        }
        /* addr match mode, match select and part of MAC */
        temp = def_conf->flow_opt[0].addr_match_mode;
        value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(temp);
        temp = def_conf->flow_opt[0].addr_match_select;
        value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(temp);
        temp = (def_conf->flow_opt[0].mac_addr[4] << 8) | def_conf->flow_opt[0].mac_addr[5];
        value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(temp);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(flow_st_index), &value));
        /* MAC addr first 4 bytes */
        value = def_conf->flow_opt[0].mac_addr[0] << 24 |
                def_conf->flow_opt[0].mac_addr[1] << 16 |
                def_conf->flow_opt[0].mac_addr[2] << 8 |
                def_conf->flow_opt[0].mac_addr[3];
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(flow_st_index), &value));

        /* num tag, vlan check */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(flow_st_index), &value));
        if (def_conf->flow_opt[0].vlan_check) {
            value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
        } else {
            value &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
        }
        temp = def_conf->flow_opt[0].num_tag;
        temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(flow_st_index), &value));
        /* other param like tag type, vlan tag etc we can set from eth1_conf_priv */
    } else {
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* Tag mode config */
            value = (def_conf->comm_opt.pbb_en ?
                     VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A_ETH1_PBB_ENA_A : 0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A, &value));
            /* TPID config */
            value = def_conf->comm_opt.tpid;
            value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A_ETH1_VLAN_TPID_CFG_A(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A, &value));
        } else {
            /* Tag mode config */
            value = (def_conf->comm_opt.pbb_en ?
                     VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B_ETH1_PBB_ENA_B : 0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B, &value));
            /* TPID config */
            value = def_conf->comm_opt.tpid;
            value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B_ETH1_VLAN_TPID_CFG_B(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B, &value));
        }

        for (i = flow_st_index; i <= flow_end_index; i++) {
            /* port map config */
            temp = ((port_no == base_port_no) ? 0x01 : 0x02);
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(temp);
            /* select Group A/B */
            if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
                value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_NXT_PROT_GRP_SEL;
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));

            /* config MATCH_MODE as per default value in DSH, remember to set
               VLAN_VERIFY_ENA to false */
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &value));
            /* set ADDR_MATCH_1 and ADDR_MATCH_2 as per DSH */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            value = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        if (def_conf->flow_opt[0].flow_en) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(flow_st_index), &value));
            value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(flow_st_index), &value));
        }
        /* addr match mode, match select and part of MAC */
        temp = def_conf->flow_opt[0].addr_match_mode;
        value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(temp);
        temp = def_conf->flow_opt[0].addr_match_select;
        value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(temp);
        temp = (def_conf->flow_opt[0].mac_addr[4] << 8) | def_conf->flow_opt[0].mac_addr[5];
        value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(temp);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(flow_st_index), &value));
        /* MAC addr first 4 bytes */
        value = def_conf->flow_opt[0].mac_addr[0] << 24 |
                def_conf->flow_opt[0].mac_addr[1] << 16 |
                def_conf->flow_opt[0].mac_addr[2] << 8 |
                def_conf->flow_opt[0].mac_addr[3];
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(flow_st_index), &value));

        /* num tag, vlan check */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(flow_st_index), &value));
        if (def_conf->flow_opt[0].vlan_check) {
            value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
        } else {
            value &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
        }

        temp = def_conf->flow_opt[0].num_tag;
        temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(flow_st_index), &value));
        /* other param like tag type, vlan tag etc we can set from eth1_conf_priv */

    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t        *eng_parm,
    vtss_phy_ts_eth_conf_t       *eth2_conf,
    const vtss_phy_ts_eth_conf_t *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, temp, i;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* No Tag mode in Eth2 */
        /* TPID config */
        value = def_conf->comm_opt.tpid;
        value = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG_ETH2_VLAN_TPID_CFG(value);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG, &value));

        for (i = flow_st_index; i <= flow_end_index; i++) {
            /* port map config */
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            /* config MATCH_MODE as per default value in DSH, remember to set
               VLAN_VERIFY_ENA to false */
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &value));
            /* set ADDR_MATCH_1 and ADDR_MATCH_2 as per DSH */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
            value = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        if (def_conf->flow_opt[0].flow_en) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(flow_st_index), &value));
            value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(flow_st_index), &value));
        }
        /* addr match mode, match select and part of MAC */
        temp = def_conf->flow_opt[0].addr_match_mode;
        value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(temp);
        temp = def_conf->flow_opt[0].addr_match_select;
        value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT(temp);
        temp = (def_conf->flow_opt[0].mac_addr[4] << 8) | def_conf->flow_opt[0].mac_addr[5];
        value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2(temp);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(flow_st_index), &value));
        /* MAC addr first 4 bytes */
        value = def_conf->flow_opt[0].mac_addr[0] << 24 |
                def_conf->flow_opt[0].mac_addr[1] << 16 |
                def_conf->flow_opt[0].mac_addr[2] << 8 |
                def_conf->flow_opt[0].mac_addr[3];
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(flow_st_index), &value));

        /* num tag, vlan check */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(flow_st_index), &value));
        if (def_conf->flow_opt[0].vlan_check) {
            value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
        } else {
            value &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
        }

        temp = def_conf->flow_opt[0].num_tag;
        temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(flow_st_index), &value));
        /* other param like tag type, vlan tag etc we can set from eth2_conf_priv */
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* No Tag mode in Eth2 */
        /* TPID config */
        value = def_conf->comm_opt.tpid;
        value = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A_ETH2_VLAN_TPID_CFG_A(value);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A, &value));

        for (i = flow_st_index; i <= flow_end_index; i++) {
            /* port map config */
            temp = ((port_no == base_port_no) ? 0x01 : 0x02);
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            /* config MATCH_MODE as per default value in DSH, remember to set
               VLAN_VERIFY_ENA to false */
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &value));
            /* set ADDR_MATCH_1 and ADDR_MATCH_2 as per DSH */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
            value = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        if (def_conf->flow_opt[0].flow_en) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(flow_st_index), &value));
            value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(flow_st_index), &value));
        }
        /* addr match mode, match select and part of MAC */
        temp = def_conf->flow_opt[0].addr_match_mode;
        value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(temp);
        temp = def_conf->flow_opt[0].addr_match_select;
        value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT(temp);
        temp = (def_conf->flow_opt[0].mac_addr[4] << 8) | def_conf->flow_opt[0].mac_addr[5];
        value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2(temp);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(flow_st_index), &value));
        /* MAC addr first 4 bytes */
        value = def_conf->flow_opt[0].mac_addr[0] << 24 |
                def_conf->flow_opt[0].mac_addr[1] << 16 |
                def_conf->flow_opt[0].mac_addr[2] << 8 |
                def_conf->flow_opt[0].mac_addr[3];
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(flow_st_index), &value));

        /* num tag, vlan check */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(flow_st_index), &value));
        if (def_conf->flow_opt[0].vlan_check) {
            value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
        } else {
            value &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
        }

        temp = def_conf->flow_opt[0].num_tag;
        temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(flow_st_index), &value));
        /* other param like tag type, vlan tag etc we can set from eth2_conf_priv */
    } else {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mpls_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t           *eng_parm,
    vtss_phy_ts_mpls_conf_t         *const mpls_conf,
    const vtss_phy_ts_mpls_conf_t   *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    const vtss_phy_ts_encap_t  encap_type = eng_parm->encap_type;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, temp, i;
    BOOL cw_present;

    /* CW is present only for EoMPLS PW i.e. for non-ACH control word */
    cw_present = ((encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP ||
                   encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP ||
                   encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) ?
                  TRUE : FALSE);

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        if (cw_present) {
            value |= VTSS_F_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_CTL_WORD;
        } else {
            value &= ~VTSS_F_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_CTL_WORD;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));

        for (i = flow_st_index; i <= flow_end_index; i++) {
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(flow_st_index), &value));
        if (def_conf->flow_opt[0].flow_en) {
            value |=  VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
        }
        /* stack depth */
        temp = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(def_conf->flow_opt[0].stack_depth);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH) | temp;
        /* stack ref point */
        if (def_conf->flow_opt[0].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_END) {
            value |=  VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(flow_st_index), &value));
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        if (cw_present) {
            value |= VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_CTL_WORD_A;
        } else {
            value &= ~VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_CTL_WORD_A;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));

        for (i = flow_st_index; i <= flow_end_index; i++) {
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }

        /* set default flow config */
        /* flow enable */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(flow_st_index), &value));
        if (def_conf->flow_opt[0].flow_en) {
            value |=  VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
        }
        /* stack depth */
        temp = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(def_conf->flow_opt[0].stack_depth);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH) | temp;
        /* stack ref point */
        if (def_conf->flow_opt[0].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_END) {
            value |=  VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(flow_st_index), &value));

    } else {
        return VTSS_RC_ERROR;
    }


    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ach_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_ach_conf_t         *const ach_conf,
    const vtss_phy_ts_ach_conf_t   *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_encap_t  encap_type = eng_parm->encap_type;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, mask, temp, i;
    BOOL is_gen2 = vtss_state->phy_ts_port_conf[port_no].is_gen2;
    BOOL is_eng2a = (eng_parm->eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? TRUE : FALSE;

    if (is_gen2 == TRUE && is_eng2a == TRUE ) {
        /* MATCH_2_UPPER: fields are 4-bit nibble, version and channel type */
        value = 0x10000000;
        mask = 0xF0FF0000; /* check the reserved 1 byte */
        if (def_conf->comm_opt.version.mask) {
            value |= ((def_conf->comm_opt.version.value & 0x0F) << 24);
            mask |= ((def_conf->comm_opt.version.mask & 0x0F) << 24);
        }
        if (def_conf->comm_opt.channel_type.mask) {
            value |= def_conf->comm_opt.channel_type.value;
            mask |= def_conf->comm_opt.channel_type.mask;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MATCH_UPPER_A, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MASK_UPPER_A, &mask));

        /* MATCH_2_LOWER: protocol ID only for PTP, OAM does not have protocol ID */
        value = 0;
        mask = 0;
        if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
            if (def_conf->comm_opt.proto_id.mask) {
                value |= (def_conf->comm_opt.proto_id.value << 16);
                value |= (def_conf->comm_opt.proto_id.mask << 16);
            }
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MATCH_LOWER_A, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MASK_LOWER_A, &mask));

        if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
            value = VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_NXT_PROTOCOL_A(6);
        } else {
            value = VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_NXT_PROTOCOL_A(4);
        }
        value |= VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_CTL_WORD_A;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A , &value));
    } else {
        /* IP mode */
        value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_MODE(3); /* 128-bit other protocol match */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_MODE, &value));

        /* IP1_PROT_MATCH_1: not reqd for ACH */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));

        /* MATCH_2_UPPER: fields are 4-bit nibble, version and channel type */
        value = 0x10000000;
        mask = 0xF0FF0000; /* check the reserved 1 byte */
        if (def_conf->comm_opt.version.mask) {
            value |= ((def_conf->comm_opt.version.value & 0x0F) << 24);
            mask |= ((def_conf->comm_opt.version.mask & 0x0F) << 24);
        }
        if (def_conf->comm_opt.channel_type.mask) {
            value |= def_conf->comm_opt.channel_type.value;
            mask |= def_conf->comm_opt.channel_type.mask;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &mask));

        /* MATCH_2_LOWER: protocol ID only for PTP, OAM does not have protocol ID */
        value = 0;
        mask = 0;
        if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
            if (def_conf->comm_opt.proto_id.mask) {
                value |= (def_conf->comm_opt.proto_id.value << 16);
                value |= (def_conf->comm_opt.proto_id.mask << 16);
            }
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &mask));

        /* offset from header: start from ACH header */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
        /* checksum */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));

        /* port map config: only one flow enable is reqd for ACH, set the first one */
        for (i = flow_st_index; i <= flow_end_index; i++) {
            value = (i == flow_st_index) ? VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA : 0;
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            value |= VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(temp);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            /* no otther flow conf reqd */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
        }

        /* IP Checksum Block Select: not reqd */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_IP_CHKSUM_CTL_PTP_IP_CKSUM_SEL, &value));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_different_offset_set(
    vtss_state_t *vtss_state,
    const vtss_port_no_t        port_no,
    const vtss_phy_ts_blk_id_t  blk_id,
    const u8                    ip_mode,
    BOOL                        double_ip)
{
    u32 value, temp;

    switch (ip_mode) {
    case VTSS_PHY_TS_IP_VER_4:
        /* IP mode */
        value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_MODE(VTSS_PHY_TS_IP_VER_4);
        value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_FLOW_OFFSET(0x0C);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_MODE, &value));
        if (double_ip) {
            /* IP1_PROT_MATCH_1 */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_OFFSET_1(9);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MASK_1(0xFF);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MATCH_1(4);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));

            /* No need of PROT_OFFSET_2 */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
            /* checksum: no UDP for IP-IP */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        } else {
            /* IP1_PROT_MATCH_1 */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_OFFSET_1(9);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MASK_1(0xFF);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MATCH_1(17);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));

            /* UDP port number offset from IP header */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2_IP1_PROT_OFFSET_2(20);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
            /* checksum */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
            temp = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_OFFSET(26);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_OFFSET) | temp;
            temp = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_WIDTH(2);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_WIDTH) | temp;
            value &= ~VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_UPDATE_ENA;
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_CLEAR_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        }
        break;
    case VTSS_PHY_TS_IP_VER_6:
        /* IP mode */
        value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_MODE(VTSS_PHY_TS_IP_VER_6);
        value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_MODE_IP1_FLOW_OFFSET(8);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_MODE, &value));
        if (double_ip) {
            /* IP1_PROT_MATCH_1 */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_OFFSET_1(6);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MASK_1(0xFF);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MATCH_1(41);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));

            /* No need of PROT_OFFSET_2 */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
            /* checksum: no UDP for IP-IP */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        } else {
            /* IP1_PROT_MATCH_1 */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_OFFSET_1(6);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MASK_1(0xFF);
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1_IP1_PROT_MATCH_1(17);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_1, &value));

            /* UDP port number offset from IP header */
            value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2_IP1_PROT_OFFSET_2(40);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_OFFSET_2, &value));
            /* checksum */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
            temp = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_OFFSET(46);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_OFFSET) | temp;
            temp = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_WIDTH(2);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_WIDTH) | temp;
            value &= ~VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_UPDATE_ENA;
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG_IP1_UDP_CHKSUM_CLEAR_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        }
        break;
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    BOOL                           double_ip,
    vtss_phy_ts_ip_conf_t          *const ip1_conf,
    const vtss_phy_ts_ip_conf_t    *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, temp, i;

    if (double_ip) {
        /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
        VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, def_conf->comm_opt.ip_mode, TRUE));

        /* set MATCH_2_UPPER, MATCH_2_LOWER, and Mask */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));

        /* port map config */
        for (i = flow_st_index; i <= flow_end_index; i++) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(temp);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK) | temp;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        /* set default flow config */
        /* flow enable */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(0), &value));
        if (def_conf->flow_opt[0].flow_en) {
            value |= VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
        } else {
            value &= ~VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
        }
        temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(def_conf->flow_opt[0].match_mode);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(0), &value));
        /* IP address */
        value = def_conf->flow_opt[0].ip_addr.ipv4.addr;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(0), &value));
        value = def_conf->flow_opt[0].ip_addr.ipv4.mask;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(0), &value));

        /* IP Checksum Block Select: should be from IP2 */
        value = 1;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_IP_CHKSUM_CTL_PTP_IP_CKSUM_SEL, &value));
    } else {
        /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
        VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, def_conf->comm_opt.ip_mode, FALSE));
        /* Src and dest port number */
        value = def_conf->comm_opt.dport_val;  /* dest port, src port 0 */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
        value = 0xFFFF; /* dest port mask, src port ignore */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
        /* MATCH_2_LOWER not used */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));

        /* port map config */
        for (i = 0; i <= flow_end_index; i++) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
            temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(temp);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK) | temp;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        /* set default flow config */
        /* flow enable */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(0), &value));
        if (def_conf->flow_opt[0].flow_en) {
            value |= VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
        } else {
            value &= ~VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
        }
        temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(def_conf->flow_opt[0].match_mode);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(0), &value));
        /* IP address */
        value = def_conf->flow_opt[0].ip_addr.ipv4.addr;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(0), &value));
        value = def_conf->flow_opt[0].ip_addr.ipv4.mask;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(0), &value));

        /* IP Checksum Block Select: should be from IP1 */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_IP_CHKSUM_CTL_PTP_IP_CKSUM_SEL, &value));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_different_offset_set(
    vtss_state_t *vtss_state,
    const vtss_port_no_t        port_no,
    const vtss_phy_ts_blk_id_t  blk_id,
    const u8                    ip_mode)
{
    u32 value, temp;

    switch (ip_mode) {
    case VTSS_PHY_TS_IP_VER_4:
        /* IP mode */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_MODE_IP2_MODE(VTSS_PHY_TS_IP_VER_4);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_MODE_IP2_FLOW_OFFSET(0x0C);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_MODE, &value));
        /* IP2_PROT_MATCH_1 */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_OFFSET_1(9);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_MASK_1(0xFF);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_MATCH_1(0x11);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1, &value));

        /* UDP port number offset from IP header */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_OFFSET_2_IP2_PROT_OFFSET_2(20);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_OFFSET_2, &value));
        /* checksum */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG, &value));
        temp = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_OFFSET(26);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_OFFSET) | temp;
        temp = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_WIDTH(2);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_WIDTH) | temp;
        value &= ~VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_UPDATE_ENA;
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_CLEAR_ENA;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG, &value));
        break;
    case VTSS_PHY_TS_IP_VER_6:
        /* IP mode */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_MODE_IP2_MODE(VTSS_PHY_TS_IP_VER_6);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_MODE_IP2_FLOW_OFFSET(8);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_MODE, &value));
        /* IP2_PROT_MATCH_1 */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_OFFSET_1(6);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_MASK_1(0xFF);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1_IP2_PROT_MATCH_1(0x11);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_1, &value));

        /* UDP port number offset from IP header */
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_PROT_OFFSET_2_IP2_PROT_OFFSET_2(40);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_OFFSET_2, &value));
        /* checksum */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG, &value));
        temp = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_OFFSET(46);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_OFFSET) | temp;
        temp = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_WIDTH(2);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_WIDTH) | temp;
        value &= ~VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_UPDATE_ENA;
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG_IP2_UDP_CHKSUM_CLEAR_ENA;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_UDP_CHKSUM_CFG, &value));
        break;
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_def_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_ip_conf_t          *const ip2_conf,
    const vtss_phy_ts_ip_conf_t    *const def_conf)
{
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_port_no_t       base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const u8                   flow_st_index = eng_parm->flow_st_index;
    const u8                   flow_end_index = eng_parm->flow_end_index;
    u32 value, temp, i;

    /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
    VTSS_RC(vtss_phy_ts_ip2_different_offset_set(vtss_state, port_no, blk_id, def_conf->comm_opt.ip_mode));

    /* Src and dest port number */
    value = def_conf->comm_opt.dport_val;  /* dest port, src port 0 */
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_UPPER, &value));
    value = 0xFFFF; /* dest port mask, src port ignore */
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_UPPER, &value));
    /* MATCH_2_LOWER not used */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_LOWER, &value));
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_LOWER, &value));

    /* port map config */
    for (i = flow_st_index; i <= flow_end_index; i++) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
        temp = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
        temp = VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
    }
    /* set default flow config */
    /* flow enable */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(0), &value));
    if (def_conf->flow_opt[0].flow_en) {
        value |= VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_ENA;
    } else {
        value &= ~VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_ENA;
    }
    temp = VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_MATCH_MODE(def_conf->flow_opt[0].match_mode);
    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_MATCH_MODE) | temp;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(0), &value));
    /* IP address */
    value = def_conf->flow_opt[0].ip_addr.ipv4.addr;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(0), &value));
    value = def_conf->flow_opt[0].ip_addr.ipv4.mask;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(0), &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth1_flow_conf_set_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, i;
    BOOL bool1, bool2, bool3;
    const vtss_phy_ts_eth_conf_t *old_eth_conf;
    const vtss_phy_ts_eth_conf_t *new_eth_conf;

    u32 match_mode_val      = 0;
    u32 tag_rng_i_tag_lower = 0, tag_rng_i_tag_upper = 0;
    u32 tag2_i_tag_lower    = 0, tag2_i_tag_upper    = 0;
    u32 tag1_lower          = 0, tag1_upper          = 0;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        old_eth_conf = &eng_conf->flow_conf.flow_conf.oam.eth1_opt;
        new_eth_conf = &new_flow_conf->flow_conf.oam.eth1_opt;
    } else {
        old_eth_conf = &eng_conf->flow_conf.flow_conf.ptp.eth1_opt;
        new_eth_conf = &new_flow_conf->flow_conf.ptp.eth1_opt;
    }


    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* PTP engine Eth1 config */
        /* Tag mode config */
        if (old_eth_conf->comm_opt.pbb_en != new_eth_conf->comm_opt.pbb_en) {
            value = (new_eth_conf->comm_opt.pbb_en ?
                     VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE_ETH1_PBB_ENA : 0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE, &value));
        }
        /* TPID config */
        if (old_eth_conf->comm_opt.tpid != new_eth_conf->comm_opt.tpid) {
            value = new_eth_conf->comm_opt.tpid;
            value = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG_ETH1_VLAN_TPID_CFG(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG, &value));
        }
        /* etype match */
        if (old_eth_conf->comm_opt.etype != new_eth_conf->comm_opt.etype) {
            value = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH_ETH1_ETYPE_MATCH(new_eth_conf->comm_opt.etype);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH, &value));
        }

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_eth_conf->flow_opt[i].flow_en != old_eth_conf->flow_opt[i].flow_en);
            if (bool1 || bool2) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_eth_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
                    } else {
                        value &=  ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            }

            /* addr match mode, match select and last 2 bytes of MAC */
            bool1 = (old_eth_conf->flow_opt[i].addr_match_mode != new_eth_conf->flow_opt[i].addr_match_mode);
            bool2 = (old_eth_conf->flow_opt[i].addr_match_select != new_eth_conf->flow_opt[i].addr_match_select);
            bool3 = (old_eth_conf->flow_opt[i].mac_addr[5] != new_eth_conf->flow_opt[i].mac_addr[5]) ||
                    (old_eth_conf->flow_opt[i].mac_addr[4] != new_eth_conf->flow_opt[i].mac_addr[4]);

            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(new_eth_conf->flow_opt[i].addr_match_mode);
                    value =  VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE) | temp;
                }
                if (bool2) {
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(new_eth_conf->flow_opt[i].addr_match_select);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT) | temp;
                }
                if (bool3) {
                    temp = (new_eth_conf->flow_opt[i].mac_addr[4] << 8) | new_eth_conf->flow_opt[i].mac_addr[5];
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(temp);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
            }
            /* first 4 bytes of MAC */
            bool1 = memcmp(old_eth_conf->flow_opt[i].mac_addr, new_eth_conf->flow_opt[i].mac_addr, 4);
            if (bool1) {
                value = new_eth_conf->flow_opt[i].mac_addr[0] << 24 |
                        new_eth_conf->flow_opt[i].mac_addr[1] << 16 |
                        new_eth_conf->flow_opt[i].mac_addr[2] << 8 |
                        new_eth_conf->flow_opt[i].mac_addr[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            }

            /* ETH1_MATCH_MODE, VLAN_TAG_RANGE_I_TAG, VLAN_TAG1 and VLAN_TAG2_I_TAG
               has different config based on combination of number of tag, tag_type
               and tag range. So it's better to set these reg everytime flow config
               is set. This will be set only for enabled flow */
            /* ETH1_MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
            temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(new_eth_conf->flow_opt[i].num_tag);
            match_mode_val = VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS) | temp;
            if (new_eth_conf->flow_opt[i].vlan_check) {
                match_mode_val |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
            } else {
                match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
            }

            if (new_eth_conf->flow_opt[i].flow_en == FALSE) {
                match_mode_val = 0;
                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            } else if (new_eth_conf->comm_opt.pbb_en) { /* flow en and pbb en */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;
                /* tag2_type is always I-tag: set the bit, DSH is not correct! */
                match_mode_val |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                /* tag1_type is always B-tag: clear the bit, DSH is not correct! */
                match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;

                /* TAG_RANGE_I_TAG and TAG2_I_TAG contains I-tag */
                tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.i_tag.val & 0xFFF;  /* lower 12-bits */
                tag_rng_i_tag_upper = (new_eth_conf->flow_opt[i].inner_tag.i_tag.val & 0xFFF000) >> 12; /* upper 12-bits */
                tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.i_tag.mask & 0xFFF; /* lower 12-bits */
                tag2_i_tag_upper = temp = (new_eth_conf->flow_opt[i].inner_tag.i_tag.mask & 0xFFF000) >> 12; /* upper 12-bits */
                /* VLAN_TAG1 */
                if (new_eth_conf->flow_opt[i].num_tag == 2) {
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag1_lower = 0;
                    tag1_upper = 0;
                }
            } else if (new_eth_conf->flow_opt[i].num_tag == 2) { /* flow en, pbb dis and num-tag = 2 */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(new_eth_conf->flow_opt[i].tag_range_mode);
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                }

                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].outer_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].outer_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                } else if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = 0;
                    tag2_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }

            } else if (new_eth_conf->flow_opt[i].num_tag == 1) { /* flow en, pbb dis and num-tag = 1 */
                /* ETH1_MATCH_MODE: it uses tag1 */
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(1); /* Tag1 range check */
                } else {
                    temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                }
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;

                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }
            } else { /* flow en, pbb dis and num-tag = 0 */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;

                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            }

            /* All the reg are updated, now it's time to write them */
            /* MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
            /* TAG_RANGE_I_TAG */
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_LOWER(tag_rng_i_tag_lower);
            value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(tag_rng_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
            /* TAG1 */
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MATCH(tag1_lower);
            value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(tag1_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
            /* TAG2_I_TAG */
            value = VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MATCH(tag2_i_tag_lower);
            value |= VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(tag2_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));

        }
    } else {
        /* OAM engine Eth1 config */
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* Tag mode config */
            if (old_eth_conf->comm_opt.pbb_en != new_eth_conf->comm_opt.pbb_en) {
                value = (new_eth_conf->comm_opt.pbb_en ?
                         VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A_ETH1_PBB_ENA_A : 0);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A, &value));
            }
            /* TPID config */
            if (old_eth_conf->comm_opt.tpid != new_eth_conf->comm_opt.tpid) {
                value = new_eth_conf->comm_opt.tpid;
                value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A_ETH1_VLAN_TPID_CFG_A(value);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A, &value));
            }
            /* etype match */
            if (old_eth_conf->comm_opt.etype != new_eth_conf->comm_opt.etype) {
                value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A_ETH1_ETYPE_MATCH_A(new_eth_conf->comm_opt.etype);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A, &value));
            }
        } else {
            /* Tag mode config */
            if (old_eth_conf->comm_opt.pbb_en != new_eth_conf->comm_opt.pbb_en) {
                value = (new_eth_conf->comm_opt.pbb_en ?
                         VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B_ETH1_PBB_ENA_B : 0);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B, &value));
            }
            /* TPID config */
            if (old_eth_conf->comm_opt.tpid != new_eth_conf->comm_opt.tpid) {
                value = new_eth_conf->comm_opt.tpid;
                value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B_ETH1_VLAN_TPID_CFG_B(value);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B, &value));
            }
            /* etype match */
            if (old_eth_conf->comm_opt.etype != new_eth_conf->comm_opt.etype) {
                value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B_ETH1_ETYPE_MATCH_B(new_eth_conf->comm_opt.etype);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B, &value));
            }
        }

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_eth_conf->flow_opt[i].flow_en != old_eth_conf->flow_opt[i].flow_en);
            if (bool1 || bool2) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_eth_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
                    } else {
                        value &=  ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            }

            /* addr match mode, match select and last 2 bytes of MAC */
            bool1 = (old_eth_conf->flow_opt[i].addr_match_mode != new_eth_conf->flow_opt[i].addr_match_mode);
            bool2 = (old_eth_conf->flow_opt[i].addr_match_select != new_eth_conf->flow_opt[i].addr_match_select);
            bool3 = (old_eth_conf->flow_opt[i].mac_addr[5] != new_eth_conf->flow_opt[i].mac_addr[5]) ||
                    (old_eth_conf->flow_opt[i].mac_addr[4] != new_eth_conf->flow_opt[i].mac_addr[4]);

            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(new_eth_conf->flow_opt[i].addr_match_mode);
                    value =  VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE) | temp;
                }
                if (bool2) {
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(new_eth_conf->flow_opt[i].addr_match_select);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT) | temp;
                }
                if (bool3) {
                    temp = (new_eth_conf->flow_opt[i].mac_addr[4] << 8) | new_eth_conf->flow_opt[i].mac_addr[5];
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(temp);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
            }
            /* first 4 bytes of MAC */
            bool1 = memcmp(old_eth_conf->flow_opt[i].mac_addr, new_eth_conf->flow_opt[i].mac_addr, 4);
            if (bool1) {
                value = new_eth_conf->flow_opt[i].mac_addr[0] << 24 |
                        new_eth_conf->flow_opt[i].mac_addr[1] << 16 |
                        new_eth_conf->flow_opt[i].mac_addr[2] << 8 |
                        new_eth_conf->flow_opt[i].mac_addr[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            }

            /* ETH1_MATCH_MODE, VLAN_TAG_RANGE_I_TAG, VLAN_TAG1 and VLAN_TAG2_I_TAG
               has different config based on combination of number of tag, tag_type
               and tag range. So it's better to set these reg everytime flow config
               is set. This will be set only for enabled flow */
            /* ETH1_MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
            temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(new_eth_conf->flow_opt[i].num_tag);
            match_mode_val = VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS) | temp;
            if (new_eth_conf->flow_opt[i].vlan_check) {
                match_mode_val |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
            } else {
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA;
            }

            if (new_eth_conf->flow_opt[i].flow_en == FALSE) {
                match_mode_val = 0;
                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            } else if (new_eth_conf->comm_opt.pbb_en) { /* flow en and pbb en */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag2_type is always I-tag: set the bit, DSH is not correct! */
                match_mode_val |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                /* tag1_type is always B-tag: clear the bit, DSH is not correct! */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;

                /* TAG_RANGE_I_TAG and TAG2_I_TAG contains I-tag */
                tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.i_tag.val & 0xFFF;  /* lower 12-bits */
                tag_rng_i_tag_upper = (new_eth_conf->flow_opt[i].inner_tag.i_tag.val & 0xFFF000) >> 12; /* upper 12-bits */
                tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.i_tag.mask & 0xFFF; /* lower 12-bits */
                tag2_i_tag_upper = temp = (new_eth_conf->flow_opt[i].inner_tag.i_tag.mask & 0xFFF000) >> 12; /* upper 12-bits */
                /* VLAN_TAG1 */
                if (new_eth_conf->flow_opt[i].num_tag == 2) {
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag1_lower = 0;
                    tag1_upper = 0;
                }
            } else if (new_eth_conf->flow_opt[i].num_tag == 2) { /* flow en, pbb dis and num-tag = 2 */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(new_eth_conf->flow_opt[i].tag_range_mode);
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;
                }

                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].outer_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].outer_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                } else if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = 0;
                    tag2_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }

            } else if (new_eth_conf->flow_opt[i].num_tag == 1) { /* flow en, pbb dis and num-tag = 1 */
                /* ETH1_MATCH_MODE: it uses tag1 */
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(1); /* Tag1 range check */
                } else {
                    temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                }
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;

                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }
            } else { /* flow en, pbb dis and num-tag = 0 */
                /* ETH1_MATCH_MODE */
                temp = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(0); /* no range check */
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE;
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE;

                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            }

            /* All the reg are updated, now it's time to write them */
            /* MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
            /* TAG_RANGE_I_TAG */
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_LOWER(tag_rng_i_tag_lower);
            value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(tag_rng_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
            /* TAG1 */
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MATCH(tag1_lower);
            value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(tag1_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
            /* TAG2_I_TAG */
            value = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MATCH(tag2_i_tag_lower);
            value |= VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(tag2_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth1_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 i;
    const vtss_phy_ts_eth_conf_t *new_eth_conf;
    const vtss_port_no_t         port_no = eng_parm->port_no;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        new_eth_conf = &new_flow_conf->flow_conf.oam.eth1_opt;
    } else {
        new_eth_conf = &new_flow_conf->flow_conf.ptp.eth1_opt;
    }

    /* For PBB enabled, certain restriction on num of tag and range checking */
    if (new_eth_conf->comm_opt.pbb_en) {
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            if (new_eth_conf->flow_opt[i].flow_en == FALSE) {
                continue;
            }
            if (new_eth_conf->flow_opt[i].num_tag == 0) {
                VTSS_E("Port: %u engine: %u:: PBB requires at least I-tag", (u32)port_no, (u32)eng_id);
                return VTSS_RC_ERROR;
            }
            if (new_eth_conf->flow_opt[i].tag_range_mode != VTSS_PHY_TS_TAG_RANGE_NONE) {
                VTSS_E("Port: %u engine: %u:: Tag range not supported when PBB enabled", (u32)port_no, (u32)eng_id);
                return VTSS_RC_ERROR;
            }
            /* For single tag, inner_tag is always I-tag, for double tag, inner is
               I-tag, outer is B-tag always */
            if (new_eth_conf->flow_opt[i].inner_tag_type != VTSS_PHY_TS_TAG_TYPE_I ||
                (new_eth_conf->flow_opt[i].num_tag == 2 &&
                 new_eth_conf->flow_opt[i].outer_tag_type != VTSS_PHY_TS_TAG_TYPE_B)) {
                VTSS_E("Port: %u engine: %u:: Wrong tag type for PBB", (u32)port_no, (u32)eng_id);
                return VTSS_RC_ERROR;
            }
        }
    }

    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_eth1_flow_conf_set_priv(vtss_state, eng_parm, eng_conf, new_flow_conf)));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_flow_conf_set_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, i;
    BOOL bool1, bool2, bool3;
    const vtss_phy_ts_eth_conf_t *old_eth_conf;
    const vtss_phy_ts_eth_conf_t *new_eth_conf;

    u32 match_mode_val      = 0;
    u32 tag_rng_i_tag_lower = 0, tag_rng_i_tag_upper = 0;
    u32 tag2_i_tag_lower    = 0, tag2_i_tag_upper    = 0;
    u32 tag1_lower          = 0, tag1_upper          = 0;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) {
        old_eth_conf = &eng_conf->flow_conf.flow_conf.oam.eth2_opt;
        new_eth_conf = &new_flow_conf->flow_conf.oam.eth2_opt;
    } else {
        old_eth_conf = &eng_conf->flow_conf.flow_conf.ptp.eth2_opt;
        new_eth_conf = &new_flow_conf->flow_conf.ptp.eth2_opt;
    }

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* PTP engine Eth2 config */
        /* TPID config */
        if (old_eth_conf->comm_opt.tpid != new_eth_conf->comm_opt.tpid) {
            value = new_eth_conf->comm_opt.tpid;
            value = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG_ETH2_VLAN_TPID_CFG(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG, &value));
        }
        /* etype match */
        if (old_eth_conf->comm_opt.etype != new_eth_conf->comm_opt.etype) {
            value = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH_ETH2_ETYPE_MATCH(new_eth_conf->comm_opt.etype);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH, &value));
        }

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_eth_conf->flow_opt[i].flow_en != old_eth_conf->flow_opt[i].flow_en);
            if (bool1 || bool2) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_eth_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
                    } else {
                        value &=  ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            }

            /* addr match mode, match select and last 2 bytes of MAC */
            bool1 = (old_eth_conf->flow_opt[i].addr_match_mode != new_eth_conf->flow_opt[i].addr_match_mode);
            bool2 = (old_eth_conf->flow_opt[i].addr_match_select != new_eth_conf->flow_opt[i].addr_match_select);
            bool3 = (old_eth_conf->flow_opt[i].mac_addr[5] != new_eth_conf->flow_opt[i].mac_addr[5]) ||
                    (old_eth_conf->flow_opt[i].mac_addr[4] != new_eth_conf->flow_opt[i].mac_addr[4]);

            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(new_eth_conf->flow_opt[i].addr_match_mode);
                    value =  VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE) | temp;
                }
                if (bool2) {
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT(new_eth_conf->flow_opt[i].addr_match_select);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT) | temp;
                }
                if (bool3) {
                    temp = (new_eth_conf->flow_opt[i].mac_addr[4] << 8) | new_eth_conf->flow_opt[i].mac_addr[5];
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2(temp);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
            }
            /* first 4 bytes of MAC */
            bool1 = memcmp(old_eth_conf->flow_opt[i].mac_addr, new_eth_conf->flow_opt[i].mac_addr, 4);
            if (bool1) {
                value = new_eth_conf->flow_opt[i].mac_addr[0] << 24 |
                        new_eth_conf->flow_opt[i].mac_addr[1] << 16 |
                        new_eth_conf->flow_opt[i].mac_addr[2] << 8 |
                        new_eth_conf->flow_opt[i].mac_addr[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
            }

            /* ETH2_MATCH_MODE, VLAN_TAG_RANGE_I_TAG, VLAN_TAG1 and VLAN_TAG2_I_TAG
               has different config based on combination of number of tag, tag_type
               and tag range. So it's better to set these reg everytime flow config
               is set. This will be set only for enabled flow */
            /* ETH2_MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
            temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(new_eth_conf->flow_opt[i].num_tag);
            match_mode_val = VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS) | temp;
            if (new_eth_conf->flow_opt[i].vlan_check) {
                match_mode_val |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
            } else {
                match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
            }

            if (new_eth_conf->flow_opt[i].flow_en == FALSE) {
                match_mode_val = 0;
                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            } else if (new_eth_conf->flow_opt[i].num_tag == 2) { /* flow en and num-tag = 2 */
                /* ETH2_MATCH_MODE */
                temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(new_eth_conf->flow_opt[i].tag_range_mode);
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
                }

                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].outer_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].outer_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                } else if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = 0;
                    tag2_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }

            } else if (new_eth_conf->flow_opt[i].num_tag == 1) { /* flow en and num-tag = 1 */
                /* ETH2_MATCH_MODE: it uses tag1 */
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(1); /* Tag1 range check */
                } else {
                    temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(0); /* no range check */
                }
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;

                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }
            } else { /* flow en and num-tag = 0 */
                /* ETH2_MATCH_MODE */
                temp = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(0); /* no range check */
                match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE) | temp;

                /* tag1_type */
                match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;

                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            }

            /* All the reg are updated, now it's time to write them */
            /* MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
            /* TAG_RANGE_I_TAG */
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_LOWER(tag_rng_i_tag_lower);
            value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(tag_rng_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
            /* TAG1 */
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MATCH(tag1_lower);
            value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(tag1_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
            /* TAG2_I_TAG */
            value = VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MATCH(tag2_i_tag_lower);
            value |= VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(tag2_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
        }
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* OAM engine Eth2 config */
        /* TPID config */
        if (old_eth_conf->comm_opt.tpid != new_eth_conf->comm_opt.tpid) {
            value = new_eth_conf->comm_opt.tpid;
            value = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A_ETH2_VLAN_TPID_CFG_A(value);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A, &value));
        }
        /* etype match */
        if (old_eth_conf->comm_opt.etype != new_eth_conf->comm_opt.etype) {
            value = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A_ETH2_ETYPE_MATCH_A(new_eth_conf->comm_opt.etype);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A, &value));
        }

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_eth_conf->flow_opt[i].flow_en != old_eth_conf->flow_opt[i].flow_en);
            if (bool1 || bool2) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_eth_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
                    } else {
                        value &=  ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            }

            /* addr match mode, match select and last 2 bytes of MAC */
            bool1 = (old_eth_conf->flow_opt[i].addr_match_mode != new_eth_conf->flow_opt[i].addr_match_mode);
            bool2 = (old_eth_conf->flow_opt[i].addr_match_select != new_eth_conf->flow_opt[i].addr_match_select);
            bool3 = (old_eth_conf->flow_opt[i].mac_addr[5] != new_eth_conf->flow_opt[i].mac_addr[5]) ||
                    (old_eth_conf->flow_opt[i].mac_addr[4] != new_eth_conf->flow_opt[i].mac_addr[4]);

            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(new_eth_conf->flow_opt[i].addr_match_mode);
                    value =  VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE) | temp;
                }
                if (bool2) {
                    temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT(new_eth_conf->flow_opt[i].addr_match_select);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT) | temp;
                }
                if (bool3) {
                    temp = (new_eth_conf->flow_opt[i].mac_addr[4] << 8) | new_eth_conf->flow_opt[i].mac_addr[5];
                    temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2(temp);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
            }
            /* first 4 bytes of MAC */
            bool1 = memcmp(old_eth_conf->flow_opt[i].mac_addr, new_eth_conf->flow_opt[i].mac_addr, 4);
            if (bool1) {
                value = new_eth_conf->flow_opt[i].mac_addr[0] << 24 |
                        new_eth_conf->flow_opt[i].mac_addr[1] << 16 |
                        new_eth_conf->flow_opt[i].mac_addr[2] << 8 |
                        new_eth_conf->flow_opt[i].mac_addr[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));
            }

            /* ETH2_MATCH_MODE, VLAN_TAG_RANGE_I_TAG, VLAN_TAG1 and VLAN_TAG2_I_TAG
               has different config based on combination of number of tag, tag_type
               and tag range. So it's better to set these reg everytime flow config
               is set. This will be set only for enabled flow */
            /* ETH2_MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
            temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(new_eth_conf->flow_opt[i].tag_range_mode);
            match_mode_val =  VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE) | temp;
            temp = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(new_eth_conf->flow_opt[i].num_tag);
            match_mode_val = VTSS_PHY_TS_CLR_BITS(match_mode_val, VTSS_M_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS) | temp;
            if (new_eth_conf->flow_opt[i].vlan_check) {
                match_mode_val |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
            } else {
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA;
            }

            if (new_eth_conf->flow_opt[i].flow_en == FALSE) {
                match_mode_val = 0;
                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            } else if (new_eth_conf->flow_opt[i].num_tag == 2) { /* flow en and num-tag = 2 */
                /* ETH2_MATCH_MODE */
                /* tag1_type */
                if (new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].outer_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;
                }

                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].outer_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].outer_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                } else if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = 0;
                    tag2_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].outer_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].outer_tag.value.mask & 0xFFF; /* 12-bits */
                    tag2_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag2_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }

            } else if (new_eth_conf->flow_opt[i].num_tag == 1) { /* flow en and num-tag = 1 */
                /* ETH2_MATCH_MODE: it uses tag1 */
                /* tag1_type */
                if (new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_S ||
                    new_eth_conf->flow_opt[i].inner_tag_type == VTSS_PHY_TS_TAG_TYPE_B) {
                    match_mode_val |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                } else {
                    match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                }
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;

                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
                if (new_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    tag_rng_i_tag_lower = new_eth_conf->flow_opt[i].inner_tag.range.lower & 0xFFF; /* 12-bits */
                    tag_rng_i_tag_upper = new_eth_conf->flow_opt[i].inner_tag.range.upper & 0xFFF; /* 12-bits */
                    tag1_lower = 0;
                    tag1_upper = 0;
                } else {
                    tag_rng_i_tag_lower = 0;
                    tag_rng_i_tag_upper = 0;
                    tag1_lower = new_eth_conf->flow_opt[i].inner_tag.value.val & 0xFFF; /* 12-bits */
                    tag1_upper = new_eth_conf->flow_opt[i].inner_tag.value.mask & 0xFFF; /* 12-bits */
                }
            } else { /* flow en and num-tag = 0 */
                /* ETH2_MATCH_MODE */
                /* tag1_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE;
                /* tag2_type */
                match_mode_val &= ~VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE;

                tag_rng_i_tag_lower = 0;
                tag_rng_i_tag_upper = 0;
                tag1_lower = 0;
                tag1_upper = 0;
                tag2_i_tag_lower = 0;
                tag2_i_tag_upper = 0;
            }

            /* All the reg are updated, now it's time to write them */
            /* MATCH_MODE */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
            /* TAG_RANGE_I_TAG */
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_LOWER(tag_rng_i_tag_lower);
            value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(tag_rng_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
            /* TAG1 */
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MATCH(tag1_lower);
            value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(tag1_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
            /* TAG2_I_TAG */
            value = VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MATCH(tag2_i_tag_lower);
            value |= VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(tag2_i_tag_upper);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{

    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        return VTSS_RC_ERROR;
    }

    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_eth2_flow_conf_set_priv(vtss_state, eng_parm, eng_conf, new_flow_conf)));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mpls_flow_conf_set_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, i;
    BOOL bool1, bool2, bool3, bool4;
    const vtss_phy_ts_mpls_conf_t *old_mpls_conf;
    const vtss_phy_ts_mpls_conf_t *new_mpls_conf;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    BOOL cw_present = FALSE;
    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        old_mpls_conf = &eng_conf->flow_conf.flow_conf.oam.mpls_opt;
        new_mpls_conf = &new_flow_conf->flow_conf.oam.mpls_opt;
    } else {
        old_mpls_conf = &eng_conf->flow_conf.flow_conf.ptp.mpls_opt;
        new_mpls_conf = &new_flow_conf->flow_conf.ptp.mpls_opt;
    }
    if ((eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) ||
        (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP) ||
        (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP)) {
        cw_present = new_mpls_conf->comm_opt.cw_en;
    }

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* MPLS common conf */
        if (old_mpls_conf->comm_opt.cw_en != new_mpls_conf->comm_opt.cw_en) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
            if (cw_present) {
                value |= VTSS_F_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_CTL_WORD;
            } else {
                value &= ~VTSS_F_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_CTL_WORD;
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        }
        /* PTP engine MPLS config */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map, flow enable, stack depth and stack ref point config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_mpls_conf->flow_opt[i].flow_en != old_mpls_conf->flow_opt[i].flow_en);
            bool3 = (new_mpls_conf->flow_opt[i].stack_depth != old_mpls_conf->flow_opt[i].stack_depth);
            bool4 = (new_mpls_conf->flow_opt[i].stack_ref_point != old_mpls_conf->flow_opt[i].stack_ref_point);
            if (bool1 || bool2 || bool3 || bool4) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_mpls_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
                    } else {
                        value &=  ~VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
                    }
                }
                if (bool3) {
                    temp = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(new_mpls_conf->flow_opt[i].stack_depth);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH) | temp;
                }
                if (bool4) {
                    if (new_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_END) {
                        value |=  VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
                    } else {
                        value &=  ~VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            }

            /* configure stack levels based on ref point */
            if (new_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                /* top-of-stack referenced */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.top.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.top.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                /* 1st after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                /* 2nd after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                /* 3rd after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
            } else {
                /* end-of-stack referenced */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                /* 1st before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                /* 2nd before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                /* 3rd before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper;
                value = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
            }
        }
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        if (old_mpls_conf->comm_opt.cw_en != new_mpls_conf->comm_opt.cw_en) {
            /* MPLS common conf */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
            if (cw_present) {
                value |= VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_CTL_WORD_A;
            } else {
                value &= ~VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_CTL_WORD_A;
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        }

        /* OAM engine MPLS config */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map, flow enable, stack depth and stack ref point config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_mpls_conf->flow_opt[i].flow_en != old_mpls_conf->flow_opt[i].flow_en);
            bool3 = (new_mpls_conf->flow_opt[i].stack_depth != old_mpls_conf->flow_opt[i].stack_depth);
            bool4 = (new_mpls_conf->flow_opt[i].stack_ref_point != old_mpls_conf->flow_opt[i].stack_ref_point);
            if (bool1 || bool2 || bool3 || bool4) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_mpls_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
                    } else {
                        value &=  ~VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
                    }
                }
                if (bool3) {
                    temp = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(new_mpls_conf->flow_opt[i].stack_depth);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH) | temp;
                }
                if (bool4) {
                    if (new_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_END) {
                        value |=  VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
                    } else {
                        value &=  ~VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT;
                    }
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            }

            /* configure stack levels based on ref point */
            if (new_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                /* top-of-stack referenced */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.top.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.top.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                /* 1st after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                /* 2nd after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                /* 3rd after top */
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
            } else {
                /* end-of-stack referenced */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                /* 1st before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                /* 2nd before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                /* 3rd before end */
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                temp = new_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper;
                value = VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(temp);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mpls_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    const vtss_phy_ts_mpls_conf_t *new_mpls_conf;

    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ||
        eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        new_mpls_conf = &new_flow_conf->flow_conf.oam.mpls_opt;
    } else {
        new_mpls_conf = &new_flow_conf->flow_conf.ptp.mpls_opt;
    }

    if ((eng_conf->encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM) &&
        (eng_conf->encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP) &&
        (eng_conf->encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP) &&
        (new_mpls_conf->comm_opt.cw_en)) {
        VTSS_E("Port: %u engine: %u:: Wrong control word configuration for encaptype : %d", (u32)port_no, (u32)eng_id, eng_conf->encap_type);
        return VTSS_RC_ERROR;
    }
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B)  {
        return VTSS_RC_ERROR;
    }

    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_mpls_flow_conf_set_priv(vtss_state, eng_parm, eng_conf, new_flow_conf)));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ach_flow_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, frm_reg;
    BOOL bool1, bool2, is_gen2, is_eng2a, is_ach;
    u8 i;
    const vtss_phy_ts_ach_conf_t  *old_ach_conf;
    const vtss_phy_ts_ach_conf_t  *new_ach_conf;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    is_gen2 = vtss_state->phy_ts_port_conf[eng_parm->port_no].is_gen2;
    is_eng2a = eng_parm->eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A ? TRUE : FALSE;
    is_ach = is_gen2 && is_eng2a ? TRUE : FALSE;
    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        old_ach_conf = &eng_conf->flow_conf.flow_conf.oam.ach_opt;
        new_ach_conf = &new_flow_conf->flow_conf.oam.ach_opt;
    } else {
        old_ach_conf = &eng_conf->flow_conf.flow_conf.ptp.ach_opt;
        new_ach_conf = &new_flow_conf->flow_conf.ptp.ach_opt;
    }

    bool1 = (old_ach_conf->comm_opt.version.value != new_ach_conf->comm_opt.version.value);
    bool2 = (old_ach_conf->comm_opt.channel_type.value != new_ach_conf->comm_opt.channel_type.value);
    if (bool1 || bool2) {
        if (is_ach == TRUE) {
            frm_reg = VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MATCH_UPPER_A ;
        } else {
            frm_reg = VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER;
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, frm_reg, &value));
        if (bool1) {
            value &= (~(0x0F << 24));
            value |= ((new_ach_conf->comm_opt.version.value & 0x0F) << 24);
        }
        if (bool2) {
            value &= 0xFFFF0000;
            value |= new_ach_conf->comm_opt.channel_type.value;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, frm_reg, &value));
    }
    bool1 = (old_ach_conf->comm_opt.version.mask != new_ach_conf->comm_opt.version.mask);
    bool2 = (old_ach_conf->comm_opt.channel_type.mask != new_ach_conf->comm_opt.channel_type.mask);
    if (bool1 || bool2) {
        if (is_ach == TRUE) {
            frm_reg = VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_MASK_UPPER_A;
        } else {
            frm_reg = VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER;
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, frm_reg, &value));
        if (bool1) {
            value &= (~(0x0F << 24));
            value |= ((new_ach_conf->comm_opt.version.mask & 0x0F) << 24);
        }
        if (bool2) {
            value &= 0xFFFF0000;
            value |= new_ach_conf->comm_opt.channel_type.mask;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, frm_reg, &value));
    }

    if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
        if (old_ach_conf->comm_opt.proto_id.value != new_ach_conf->comm_opt.proto_id.value) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
            value &= 0x0000FFFF;
            value |= (new_ach_conf->comm_opt.proto_id.value << 16);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
        }
        if (old_ach_conf->comm_opt.proto_id.mask != new_ach_conf->comm_opt.proto_id.mask) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));
            value &= 0x0000FFFF;
            value |= (new_ach_conf->comm_opt.proto_id.mask << 16);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));
        }
    }

    if (is_gen2 != TRUE || is_eng2a != TRUE) {
        /* change the channel map flow_st_index which is the only index enabled for ACH */
        i = eng_conf->flow_st_index;
        if (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(new_flow_conf->channel_map[i]);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK) | temp;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
    } else if (is_eng2a == TRUE) {
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A, &value));
        if (eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
            value = VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_NXT_PROTOCOL_A(6);
        } else {
            value = VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_NXT_PROTOCOL_A(4);
        }
        value |= VTSS_F_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A_ACH_CTL_WORD_A;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A, &value));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ach_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{

    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;
    if (vtss_state->phy_ts_port_conf[eng_parm->port_no].is_gen2 == TRUE) {
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
            return VTSS_RC_ERROR;
        }
    } else if ((eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ||
               (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B)) {
        return VTSS_RC_ERROR;
    }
    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_ach_flow_conf_priv(vtss_state, eng_parm, eng_conf, new_flow_conf)));

    return VTSS_RC_OK;
}

typedef enum {
    VTSS_PHY_TS_NEXT_COMP_NONE    = 0,
    VTSS_PHY_TS_NEXT_COMP_ETH2    = 1,
    VTSS_PHY_TS_NEXT_COMP_IP1     = 2,
    VTSS_PHY_TS_NEXT_COMP_IP2     = 3,
    VTSS_PHY_TS_NEXT_COMP_MPLS    = 4,
    VTSS_PHY_TS_NEXT_COMP_PTP_OAM = 5,
} vtss_phy_ts_next_comp_t;

static vtss_rc vtss_phy_ts_ip1_next_comp_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_next_comp_t     next_comp,
    const u8                          hdr_length)
{
    u32 value;

    if (next_comp == VTSS_PHY_TS_NEXT_COMP_NONE) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL) | VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(hdr_length);
    } else {
        value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(hdr_length);
        value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_COMPARATOR(next_comp);
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_flow_conf_set_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    BOOL                                 double_ip,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, i;
    BOOL bool1, bool2, bool3;
    const vtss_phy_ts_ip_conf_t  *old_ip_conf;
    const vtss_phy_ts_ip_conf_t  *new_ip_conf;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;

    old_ip_conf = &eng_conf->flow_conf.flow_conf.ptp.ip1_opt;
    new_ip_conf = &new_flow_conf->flow_conf.ptp.ip1_opt;

    if (double_ip) {
        if (old_ip_conf->comm_opt.ip_mode != new_ip_conf->comm_opt.ip_mode) {
            if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                /* IP1 next protocol i.e. number of bytes in this header */
                VTSS_RC(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 20)); /* default IPv4 header length */

                /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
                VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_4, TRUE));
            } else if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_6) {
                /* IP1 next protocol i.e. number of bytes in this header */
                VTSS_RC(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 40)); /* default IPv6 header length */
                /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
                VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_6, TRUE));
            }
        }

        /* Src and dest port number is not valid for IP1 i.e. IP over IP,
           so PROT_MATCH_2 no need to config, already set to default from
           def conf */

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable, match mode config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_ip_conf->flow_opt[i].flow_en != old_ip_conf->flow_opt[i].flow_en);
            bool3 = (new_ip_conf->flow_opt[i].match_mode != old_ip_conf->flow_opt[i].match_mode);
            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_ip_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
                    } else {
                        value &=  ~VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
                    }
                }
                if (bool3) {
                    temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(new_ip_conf->flow_opt[i].match_mode);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            }

            /* IP address */
            bool1 = memcmp(&old_ip_conf->flow_opt[i].ip_addr, &new_ip_conf->flow_opt[i].ip_addr, sizeof(old_ip_conf->flow_opt[i].ip_addr));
            if (bool1) {
                if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv4.addr;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv4.mask;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                    /* clear the other mask register */
                    value = 0;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                } else {
                    /* Upper 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                    /* Upper mid 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                    /* Lower mid 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                    /* Lower 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                }
            }
        }
    } else {
        if (old_ip_conf->comm_opt.ip_mode != new_ip_conf->comm_opt.ip_mode) {
            if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                /* IP1 next protocol i.e. number of bytes in this header */
                VTSS_RC(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 28)); /* IPv4+UDP header length */
                /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
                VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_4, FALSE));
            } else if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_6) {
                /* IP1 next protocol i.e. number of bytes in this header */
                VTSS_RC(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 48)); /* IPv6+UDP header length */
                /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
                VTSS_RC(vtss_phy_ts_ip1_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_6, FALSE));
            }
        }

        /* Src and dest port number */
        bool1 = (old_ip_conf->comm_opt.sport_val != new_ip_conf->comm_opt.sport_val);
        bool2 = (old_ip_conf->comm_opt.dport_val != new_ip_conf->comm_opt.dport_val);
        if (bool1 || bool2) {
            value = new_ip_conf->comm_opt.sport_val << 16 | new_ip_conf->comm_opt.dport_val;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
        }
        bool1 = (old_ip_conf->comm_opt.sport_mask != new_ip_conf->comm_opt.sport_mask);
        bool2 = (old_ip_conf->comm_opt.dport_mask != new_ip_conf->comm_opt.dport_mask);
        if (bool1 || bool2) {
            value = new_ip_conf->comm_opt.sport_mask << 16 | new_ip_conf->comm_opt.dport_mask;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
        }
        /* UDP port number offset from IP header: taken care in def conf */

        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            /* channel map and flow enable, match mode config */
            bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
            bool2 = (new_ip_conf->flow_opt[i].flow_en != old_ip_conf->flow_opt[i].flow_en);
            bool3 = (new_ip_conf->flow_opt[i].match_mode != old_ip_conf->flow_opt[i].match_mode);
            if (bool1 || bool2 || bool3) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
                if (bool1) {
                    temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK) | temp;
                }
                if (bool2) {
                    if (new_ip_conf->flow_opt[i].flow_en) {
                        value |=  VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
                    } else {
                        value &=  ~VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
                    }
                }
                if (bool3) {
                    temp = VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(new_ip_conf->flow_opt[i].match_mode);
                    value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE) | temp;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            }

            /* IP address */
            bool1 = memcmp(&old_ip_conf->flow_opt[i].ip_addr, &new_ip_conf->flow_opt[i].ip_addr, sizeof(old_ip_conf->flow_opt[i].ip_addr));
            if (bool1) {
                if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv4.addr;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv4.mask;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                    /* clear the other mask register */
                    value = 0;
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                } else {
                    /* Upper 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                    /* Upper mid 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                    /* Lower mid 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                    /* Lower 32-bit of ipv6 address */
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
                    value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0];
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                }
            }
        }
    }

    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_ts_ip1_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    BOOL                                 double_ip,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    const vtss_phy_ts_ip_conf_t  *new_ip_conf;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    new_ip_conf = &new_flow_conf->flow_conf.ptp.ip1_opt;
    if ((new_ip_conf->comm_opt.ip_mode != VTSS_PHY_TS_IP_VER_4) &&
        (new_ip_conf->comm_opt.ip_mode != VTSS_PHY_TS_IP_VER_6)) {
        return VTSS_RC_ERROR;
    }

    if ((eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ||
        (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B)) {
        return VTSS_RC_ERROR;
    }
    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_ip1_flow_conf_set_priv(vtss_state, eng_parm, double_ip, eng_conf, new_flow_conf)));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_next_comp_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_next_comp_t     next_comp,
    const u8                          hdr_length)
{
    u32 value;

    if (next_comp == VTSS_PHY_TS_NEXT_COMP_NONE) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_PROTOCOL) | VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_PROTOCOL(hdr_length);
    } else {
        value = VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_PROTOCOL(hdr_length);
        value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_COMPARATOR(next_comp);
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_flow_conf_set_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    u32 value, temp, i;
    BOOL bool1, bool2, bool3;
    const vtss_phy_ts_ip_conf_t  *old_ip_conf;
    const vtss_phy_ts_ip_conf_t  *new_ip_conf;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;

    old_ip_conf = &eng_conf->flow_conf.flow_conf.ptp.ip2_opt;
    new_ip_conf = &new_flow_conf->flow_conf.ptp.ip2_opt;

    if (old_ip_conf->comm_opt.ip_mode != new_ip_conf->comm_opt.ip_mode) {
        if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
            /* IP2 next protocol i.e. number of bytes in this header */
            VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_ip2_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 28))); /* IPv4+UDP header length */
            /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
            VTSS_RC(vtss_phy_ts_ip2_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_4));
        } else if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_6) {
            /* IP2 next protocol i.e. number of bytes in this header */
            VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_ip2_next_comp_set_priv(vtss_state, port_no, blk_id, VTSS_PHY_TS_NEXT_COMP_NONE, 48))); /* IPv6+UDP header length */
            /* set IP mode, IP1_PROT_MATCH_1, PROT_OFFSET_2, checksum */
            VTSS_RC(vtss_phy_ts_ip2_different_offset_set(vtss_state, port_no, blk_id, VTSS_PHY_TS_IP_VER_6));
        } else {
            return VTSS_RC_ERROR; /* not supported other protocol */
        }
    }

    /* Src and dest port number */
    bool1 = (old_ip_conf->comm_opt.sport_val != new_ip_conf->comm_opt.sport_val);
    bool2 = (old_ip_conf->comm_opt.dport_val != new_ip_conf->comm_opt.dport_val);
    if (bool1 || bool2) {
        value = new_ip_conf->comm_opt.sport_val << 16 | new_ip_conf->comm_opt.dport_val;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_UPPER, &value));
    }
    bool1 = (old_ip_conf->comm_opt.sport_mask != new_ip_conf->comm_opt.sport_mask);
    bool2 = (old_ip_conf->comm_opt.dport_mask != new_ip_conf->comm_opt.dport_mask);
    if (bool1 || bool2) {
        value = new_ip_conf->comm_opt.sport_mask << 16 | new_ip_conf->comm_opt.dport_mask;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_UPPER, &value));
    }
    /* UDP port number offset from IP header: taken care in def conf */

    for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
        /* channel map and flow enable, match mode config */
        bool1 = (new_flow_conf->channel_map[i] != eng_conf->flow_conf.channel_map[i]);
        bool2 = (new_ip_conf->flow_opt[i].flow_en != old_ip_conf->flow_opt[i].flow_en);
        bool3 = (new_ip_conf->flow_opt[i].match_mode != old_ip_conf->flow_opt[i].match_mode);
        if (bool1 || bool2 || bool3) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
            if (bool1) {
                temp = VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK(new_flow_conf->channel_map[i]);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK) | temp;
            }
            if (bool2) {
                if (new_ip_conf->flow_opt[i].flow_en) {
                    value |=  VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_ENA;
                } else {
                    value &=  ~VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_ENA;
                }
            }
            if (bool3) {
                temp = VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_MATCH_MODE(new_ip_conf->flow_opt[i].match_mode);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_MATCH_MODE) | temp;
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
        }

        /* IP address */
        bool1 = memcmp(&old_ip_conf->flow_opt[i].ip_addr, &new_ip_conf->flow_opt[i].ip_addr, sizeof(old_ip_conf->flow_opt[i].ip_addr));
        if (bool1) {
            if (new_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                value = new_ip_conf->flow_opt[i].ip_addr.ipv4.addr;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(i), &value));
                value = new_ip_conf->flow_opt[i].ip_addr.ipv4.mask;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(i), &value));
                /* clear the other mask register */
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER_MID(i), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER(i), &value));
            } else {
                /* Upper 32-bit of ipv6 address */
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(i), &value));
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(i), &value));
                /* Upper mid 32-bit of ipv6 address */
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER_MID(i), &value));
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER_MID(i), &value));
                /* Lower mid 32-bit of ipv6 address */
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER_MID(i), &value));
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER_MID(i), &value));
                /* Lower 32-bit of ipv6 address */
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER(i), &value));
                value = new_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0];
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER(i), &value));
            }
        }
    }
    return VTSS_RC_OK;


}

static vtss_rc vtss_phy_ts_ip2_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t                *eng_parm,
    const vtss_phy_ts_eng_conf_t         *const eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    const vtss_phy_ts_ip_conf_t  *new_ip_conf;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    new_ip_conf = &new_flow_conf->flow_conf.ptp.ip1_opt;
    if ((new_ip_conf->comm_opt.ip_mode != VTSS_PHY_TS_IP_VER_4) &&
        (new_ip_conf->comm_opt.ip_mode != VTSS_PHY_TS_IP_VER_6)) {
        return VTSS_RC_ERROR;
    }

    if ((eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ||
        (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B)) {
        return VTSS_RC_ERROR;
    }
    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_ip2_flow_conf_set_priv(vtss_state, eng_parm, eng_conf, new_flow_conf)));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ptp_def_conf_priv(
    vtss_state_t *vtss_state,
    BOOL                  ingress,
    vtss_ts_engine_parm_t *eng_parm);

static vtss_rc vtss_phy_ts_oam_def_conf_priv(
    vtss_state_t *vtss_state,
    BOOL                  ingress,
    vtss_ts_engine_parm_t *eng_parm);

static vtss_rc vtss_phy_ts_ptp_ts_all_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t *eng_parm);

#define VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)  (((eng_id) >= VTSS_PHY_TS_PTP_ENGINE_ID_0) && ((eng_id) <= VTSS_PHY_TS_OAM_ENGINE_ID_2B))

static vtss_rc vtss_phy_ts_eth1_next_comp_etype_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_engine_t        eng_id,
    const vtss_phy_ts_next_comp_t     next_comp,
    const u16                         etype)
{
    u32 value, temp;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* set next comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
        temp = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
        /* etype match */
        value = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH_ETH1_ETYPE_MATCH(etype);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH, &value));
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* set next comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
        temp = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_NXT_COMPARATOR_A(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_NXT_COMPARATOR_A) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
        /* etype match */
        value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A_ETH1_ETYPE_MATCH_A(etype);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A, &value));
    } else {
        /* set next comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
        temp = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
        /* etype match */
        value = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B_ETH1_ETYPE_MATCH_B(etype);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B, &value));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_next_comp_etype_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_engine_t        eng_id,
    const vtss_phy_ts_next_comp_t     next_comp,
    const u16                         etype)
{
    u32 value, temp;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* set eth2 next comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        temp = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_NXT_COMPARATOR(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_NXT_COMPARATOR) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        /* etype match */
        value = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH_ETH2_ETYPE_MATCH(etype);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH, &value));
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* set next comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        temp = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A_ETH2_NXT_COMPARATOR_A(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A_ETH2_NXT_COMPARATOR_A) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        /* etype match */
        value = VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A_ETH2_ETYPE_MATCH_A(etype);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A, &value));
    } else {
        /* eth2 can not be used for engine 2B */
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mpls_next_comp_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_engine_t        eng_id,
    const vtss_phy_ts_next_comp_t     next_comp)
{
    u32 value, temp;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        temp = VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_NXT_COMPARATOR(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        temp = VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A(next_comp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
    } else {
        /* MPLS can not be used for engine 2B */
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ach_next_comp_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_blk_id_t        blk_id,
    const vtss_phy_ts_encap_t         encap_type,
    const vtss_phy_ts_next_comp_t     next_comp)
{
    u32 value, temp;

    /* set ACH next protocol: length of ACH */
    temp = 4; /* default ACH header length */
    if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
        temp = 6; /* 4 bytes G-ACH + 2 bytes Protocol ID */
    } else if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        temp = 4; /* 4 bytes G-ACH */
    }
    value = VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(temp);
    value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_COMPARATOR(next_comp);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id)
{
    u32 value;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL;

    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask ;
    flow_conf = &vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].flow_conf;
    if (sig_mask & (VTSS_PHY_TS_FIFO_SIG_DEST_IP | VTSS_PHY_TS_FIFO_SIG_SRC_IP)) {
        /* select the offset */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                     VTSS_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG, &value));
        value = VTSS_PHY_TS_CLR_BITS(value,
                                     VTSS_M_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET);
        if (flow_conf->flow_conf.ptp.ip1_opt.comm_opt.ip_mode
            == VTSS_PHY_TS_IP_VER_4) {
            value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(12);
        } else if (flow_conf->flow_conf.ptp.ip1_opt.comm_opt.ip_mode
                   == VTSS_PHY_TS_IP_VER_6) {
            /* Gen2 */
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
                    VTSS_E("Invalid Signature Selection :: For IPV6 frames only"
                           " Source IP can be selected, engine_id : %d", engine_id);
                }
                value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(20);
            } else {
                if ((sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) && (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP)) {
                    VTSS_E("For IPv6 frames, either source IP or destination IP can be selected"
                           " but not both, engine_id : %d", engine_id);
                }
                if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
                    value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(32);
                } else if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
                    value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(20);
                }
            }
        }

        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG, &value));
        /* select the IP1 comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                     VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
        value = VTSS_PHY_TS_CLR_BITS(value,
                                     VTSS_M_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
        value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(2) ;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id)
{
    u32 value;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL;

    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask ;
    flow_conf = &vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[engine_id].flow_conf;
    if (vtss_state->phy_ts_port_conf[port_no].sig_mask &
        (VTSS_PHY_TS_FIFO_SIG_DEST_IP | VTSS_PHY_TS_FIFO_SIG_SRC_IP)) {
        /* select the offset */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                     VTSS_ANA_IP2_NXT_PROTOCOL_IP2_FRAME_SIG_CFG, &value));
        value = VTSS_PHY_TS_CLR_BITS(value,
                                     VTSS_M_ANA_IP2_NXT_PROTOCOL_IP2_FRAME_SIG_CFG_IP2_FRAME_SIG_OFFSET);
        if (flow_conf->flow_conf.ptp.ip2_opt.comm_opt.ip_mode
            == VTSS_PHY_TS_IP_VER_4) {
            value |= VTSS_F_ANA_IP2_NXT_PROTOCOL_IP2_FRAME_SIG_CFG_IP2_FRAME_SIG_OFFSET(12) ;
        } else if (flow_conf->flow_conf.ptp.ip2_opt.comm_opt.ip_mode
                   == VTSS_PHY_TS_IP_VER_6) {
            /* Gen2 */
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
                    VTSS_E("Invalid Signature Selection :: For IPV6 frames only"
                           " Source IP can be selected, engine_id : %d", engine_id);
                }
                value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(20);
            } else {
                if ((sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) && (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP)) {
                    VTSS_E("For IPv6 frames, either source IP or destination IP can be selected"
                           " but not both, engine_id : %d", engine_id);
                }
                if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
                    value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(32);
                } else if (sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
                    value |= VTSS_F_ANA_IP1_NXT_PROTOCOL_IP1_FRAME_SIG_CFG_IP1_FRAME_SIG_OFFSET(20);
                }
            }
        }

        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_IP2_NXT_PROTOCOL_IP2_FRAME_SIG_CFG, &value));
        /* select the IP2 comparator */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                     VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
        value = VTSS_PHY_TS_CLR_BITS(value,
                                     VTSS_M_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
        value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(3);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                      VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth1_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id)
{
    u32 value;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;

    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask ;
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
        if (engine_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* select the offset */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_FRAME_SIG_OFFSET);
            value |= VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_FRAME_SIG_OFFSET(0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));

            /* select the ETH1 comparator in Frame Signature builder mode config register*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(0) ;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
        } else if (engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A ||
                   engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
                if (engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                    /*Select the offset */
                    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                                 VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
                    value = VTSS_PHY_TS_CLR_BITS(value,
                                                 VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_FRAME_SIG_OFFSET_A);
                    value |= VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_FRAME_SIG_OFFSET_A(0);
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                                  VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
                } else {
                    /*Select the offset */
                    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                                 VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
                    value = VTSS_PHY_TS_CLR_BITS(value,
                                                 VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_FRAME_SIG_OFFSET_B);
                    value |= VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_FRAME_SIG_OFFSET_B(0);
                    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                                  VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
                }

                /* select the ETH1 comparator in Frame Signature builder mode config register*/
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                             VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_CFG, &value));
                value = VTSS_PHY_TS_CLR_BITS(value,
                                             VTSS_M_ANA_OAM_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
                value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(0) ;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                              VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_CFG, &value));
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_sig_mask_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t              port_no,
    const vtss_phy_ts_engine_t        engine_id,
    const vtss_phy_ts_blk_id_t        blk_id)
{
    u32 value;
    vtss_phy_ts_fifo_sig_mask_t sig_mask;

    sig_mask = vtss_state->phy_ts_port_conf[port_no].sig_mask ;
    if (sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
        if (engine_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* select the offset */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_FRAME_SIG_OFFSET);
            value |= VTSS_F_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_FRAME_SIG_OFFSET(0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));

            /* select the ETH2 comparator in Frame Signature builder mode config register*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
            value |= VTSS_F_ANA_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(1) ;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_FRAME_SIG_CFG_FSB_CFG, &value));
        } else {
            /*Select the offset */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A_ETH2_FRAME_SIG_OFFSET_A);
            value |= VTSS_F_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A_ETH2_FRAME_SIG_OFFSET_A(0);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));

            /* select the ETH2 comparator in Frame Signature builder mode config register*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id,
                                         VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_CFG, &value));
            value = VTSS_PHY_TS_CLR_BITS(value,
                                         VTSS_M_ANA_OAM_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL);
            value |= VTSS_F_ANA_OAM_FRAME_SIG_CFG_FSB_CFG_FSB_ADR_SEL(1) ;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id,
                                          VTSS_ANA_OAM_FRAME_SIG_CFG_FSB_CFG, &value));
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_action_flow_init (
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id)
{
    u32 value;
    i32 flow_index;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* flow disable */
        for (flow_index = 0; flow_index < 6; flow_index++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
            /* clear other action registers */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
        }
    } else {
        /* flow disable */
        for (flow_index = 0; flow_index < 6; flow_index++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
            /* clear other action registers */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
        }
    }
    return VTSS_RC_OK;
}
static vtss_rc vtss_phy_ts_flow_match_mode_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const vtss_phy_ts_engine_flow_match_t flow_match_mode,
    const BOOL                  ingress);

static vtss_rc vtss_phy_ts_engine_init_priv(
    vtss_state_t *vtss_state,
    const BOOL        ingress,
    vtss_ts_engine_parm_t                 *eng_parm,
    const vtss_phy_ts_engine_flow_match_t flow_match_mode)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_phy_ts_eng_conf_t *base_port_eng_conf, *alt_eng_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf;
    vtss_phy_ts_engine_action_t    *action_conf;
    u8 i;
    const vtss_port_no_t        port_no = eng_parm->port_no;
    const vtss_port_no_t        base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t  blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t  eng_id = eng_parm->eng_id;
    const vtss_phy_ts_encap_t   encap_type = eng_parm->encap_type;
    const u8                    flow_st_index = eng_parm->flow_st_index;
    const u8                    flow_end_index = eng_parm->flow_end_index;
    vtss_phy_ts_engine_flow_match_t match_mode1 = 0;
    BOOL is_gen2 = vtss_state->phy_ts_port_conf[port_no].is_gen2;
    BOOL is_eng2a = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? TRUE : FALSE;
    BOOL is_eng_used = FALSE;
    vtss_phy_ts_eth_conf_t      ts_any_eth_conf;

    VTSS_N("Port: %u engine: %u:: Init", (u32)port_no, (u32)eng_id);
    if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
        VTSS_E("ts_init not done for port %u", port_no);
        return VTSS_RC_ERROR;
    }
    if ((flow_end_index > 7) || (flow_end_index < flow_st_index)) {
        VTSS_E("invalid flow indexes (%d, %d) for port %u", flow_st_index, flow_end_index, port_no);
        return VTSS_RC_ERROR;
    }

    /* In Gen-1, PTP application not allowed in OAM engine because of no TSFIFO support.
       ACH support for OAM is also not possible in OAM engine, since ACH uses
       IP1 comparator */
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A ||
        eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        if (encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_PTP ||
            encap_type == VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP ||
            encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP ||
            encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP ||
            encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP ||
            encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
            if (is_gen2 == TRUE && is_eng2a == TRUE) {
                if (encap_type != VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
                    VTSS_E("Invalid encap type (%d)for eng_id (%d)", encap_type, eng_id);
                    return VTSS_RC_ERROR;
                }
            } else {
                VTSS_E("Invalid encap type (%d)for eng_id (%d)", encap_type, eng_id);
                return VTSS_RC_ERROR;
            }
        }

        if (is_gen2 != TRUE) {
            if (encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP ||
                encap_type == VTSS_PHY_TS_ENCAP_ETH_PTP ||
                encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP ||
                encap_type == VTSS_PHY_TS_ENCAP_ETH_ETH_PTP ||
                encap_type == VTSS_PHY_TS_ENCAP_ANY) {
                VTSS_E("Invalid encap type (%d)for eng_id (%d)", encap_type, eng_id);
                return VTSS_RC_ERROR;
            }
        }

        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
            do {
                if (encap_type == VTSS_PHY_TS_ENCAP_ETH_OAM) {
                    break;
                }

                if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
                    if (encap_type == VTSS_PHY_TS_ENCAP_ETH_PTP ||
                        encap_type == VTSS_PHY_TS_ENCAP_ANY) {
                        break;
                    }
                }

                VTSS_E("Invalid encap type (%d)for engine 2B", encap_type);
                return VTSS_RC_ERROR;
            } while (0);
            if (ingress == TRUE) {
                /*Direction is Ingress */
                is_eng_used = vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].eng_used;
                if (encap_type != vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].encap_type && is_eng_used == TRUE) {
                    /* 2A and 2B have different encapsulation */
                    match_mode1 = vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].flow_match_mode;

                    if (flow_match_mode != VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                        if (match_mode1 == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                            VTSS_E("Invalid configuration on Engine C as 2A group is configured with strict");
                            return VTSS_RC_ERROR;
                        }
                    } else {
                        VTSS_E("Invalid configuration on Engine C as 2A group is configured with strict");
                        return VTSS_RC_ERROR;
                    }
                }
            } else {
                /* Direction is Egress */
                is_eng_used = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].eng_used;
                if (encap_type != vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].encap_type && is_eng_used == TRUE) {
                    /* 2A and 2B have different encapsulation */
                    match_mode1 = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A].flow_match_mode;
                    if (flow_match_mode != VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                        if (match_mode1 == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                            VTSS_E("Invalid configuration on Engine C as 2A group is configured with strict");
                            return VTSS_RC_ERROR;
                        }
                    } else {
                        VTSS_E("Invalid configuration on Engine C as 2A group is configured with strict");
                        return VTSS_RC_ERROR;
                    }
                }
            }
        } else {
            if (ingress == TRUE) {
                /*Direction is Ingress */
                is_eng_used = vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].eng_used;
                if (encap_type != vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].encap_type && is_eng_used == TRUE) {
                    /* 2A and 2B have different encapsulation */
                    match_mode1 = vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].flow_match_mode;
                    if (flow_match_mode != VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {                 /* if Match mode of the current engine is not strict */
                        if (match_mode1 == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                            VTSS_E("Invalid configuration on Engine C as 2B group is configured with strict");
                            return VTSS_RC_ERROR;
                        }
                    } else {
                        VTSS_E("Invalid configuration on Engine C as 2B group is configured with strict");
                        return VTSS_RC_ERROR;
                    }
                }
            } else {
                /* Direction is Egress */
                is_eng_used = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].eng_used;
                if (encap_type != vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].encap_type && is_eng_used == TRUE) {
                    /* 2A and 2B have different encapsulation */
                    match_mode1 = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B].flow_match_mode;
                    if (flow_match_mode != VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                        if (match_mode1 == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
                            VTSS_E("Invalid configuration on Engine C as 2B group is configured with strict");
                            return VTSS_RC_ERROR;
                        }
                    } else {
                        VTSS_E("Invalid configuration on Engine C as 2B group is configured with strict");
                        return VTSS_RC_ERROR;
                    }
                }
            }/* end,Direction is Egress */
        }
    }

    if (ingress) {
        base_port_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
    } else {
        base_port_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
    }
    if (base_port_eng_conf->eng_used) {
        VTSS_E("eng (%d) already used for port %u", eng_id, port_no);
        return VTSS_RC_ERROR;
    }

    /* Make sure flow is not overlap between engine 2A and 2B */
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A ||
        eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        if (ingress) {
            if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B];
            } else {
                alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A];
            }
        } else {
            if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B];
            } else {
                alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A];
            }
        }

        if (alt_eng_conf->eng_used) {
            if ((flow_end_index < alt_eng_conf->flow_st_index) ||
                (flow_st_index > alt_eng_conf->flow_end_index)) {
                rc = VTSS_RC_OK;
            } else {
                VTSS_E("Invalid flow_index (end_index %d, st_index %d)", flow_end_index, flow_st_index);
                return VTSS_RC_ERROR;
            }
            /* In Gen-1 Phys, flow match should be same as other engine */
            if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
                if (flow_match_mode != alt_eng_conf->flow_match_mode) {
                    VTSS_I("port: %u:: flow match mode is not consistent with engine ID %d",
                           (u32)port_no, (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?
                           VTSS_PHY_TS_OAM_ENGINE_ID_2B : eng_id);
                }
            }
        } else {
            /* This is the place where the alt_eng is not enabled and the current
                       * engine is also not setup so clear the PTP/OAM Comparator flows here */
            VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_action_flow_init(vtss_state, port_no, blk_id, eng_id)));
        }
    } else {
        /* This is the place where the application has called for ENGINE_0 or ENGINE_1
                * so clear the PTP/OAM comparator flows here */
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_action_flow_init(vtss_state, port_no, blk_id, eng_id)));
    }

    /* In gen-1 Phys, flow match should be same in ingress and egress for same engine ID */
    if (vtss_state->phy_ts_port_conf[port_no].is_gen2 != TRUE) {
        if (ingress) {
            alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
        } else {
            alt_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
        }
        if (alt_eng_conf->eng_used && flow_match_mode != alt_eng_conf->flow_match_mode) {
            VTSS_I("port: %u:: flow match mode is not consistent in ingress and egress engine ID %d", (u32)port_no, eng_id);
        }
    }

    base_port_eng_conf->eng_used = TRUE;
    base_port_eng_conf->encap_type = encap_type;
    base_port_eng_conf->flow_match_mode = flow_match_mode;
    base_port_eng_conf->flow_st_index = flow_st_index;
    base_port_eng_conf->flow_end_index = flow_end_index;

    /* set the engine flow match mode */
    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_flow_match_mode_set_priv(vtss_state, port_no, blk_id, eng_id, flow_match_mode, ingress)));

    flow_conf = &base_port_eng_conf->flow_conf;
    action_conf = &base_port_eng_conf->action_conf;
    /* reset flow_conf and action_conf */
    memset(flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
    memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));

    /* By default all the flows are associated with init port */
    for (i = flow_st_index; i <= flow_end_index; i++) {
        flow_conf->channel_map[i] = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
    }

    switch (encap_type) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        /* PTP allows only in PTP engine */
        /* next comp: PTP */

        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x88F7;

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));
        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x0800;

        /* set IP next comparator: PTP */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_def_conf_priv(vtss_state, eng_parm, FALSE,
                                                        &flow_conf->flow_conf.ptp.ip1_opt,
                                                        &vtss_phy_ts_def_inner_ip_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ip1_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_inner_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        /* signature mask config */
        if (ingress == FALSE) {
            rc = VTSS_RC_COLD(vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id));
        }
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x0800;

        /* set IP1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_IP2, 20));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_def_conf_priv(vtss_state, eng_parm, TRUE,
                                                        &flow_conf->flow_conf.ptp.ip1_opt,
                                                        &vtss_phy_ts_def_outer_ip_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }

        memset(&flow_conf->flow_conf.ptp.ip1_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_outer_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        /* set IP2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip2_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip2_def_conf_priv(vtss_state, eng_parm,
                                                        &flow_conf->flow_conf.ptp.ip2_opt,
                                                        &vtss_phy_ts_def_inner_ip_conf));

        /* signature mask config */
        if (rc == VTSS_RC_OK && ingress == FALSE) {
            rc = VTSS_RC_COLD(vtss_phy_ts_ip2_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id));
        }
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ip2_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip2_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip2_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_inner_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        /* No OAM engine for this encap type */
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7));
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf_for_pbb));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));

        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt));

        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x88E7;

        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth2_opt.comm_opt.etype = 0x88F7;

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf_for_pbb));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x88E7;

        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth2_opt.comm_opt.etype = 0x0800;

        /* set IP1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_def_conf_priv(vtss_state, eng_parm, FALSE,
                                                        &flow_conf->flow_conf.ptp.ip1_opt,
                                                        &vtss_phy_ts_def_inner_ip_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ip1_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_inner_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        if (rc != VTSS_RC_OK) {
            break;
        }

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                              eng_id, VTSS_PHY_TS_NEXT_COMP_IP1));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.mpls_opt.comm_opt.cw_en = FALSE;
        /* set IP1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_def_conf_priv(vtss_state, eng_parm, FALSE,
                                                        &flow_conf->flow_conf.ptp.ip1_opt,
                                                        &vtss_phy_ts_def_inner_ip_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ip1_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_inner_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        if (rc != VTSS_RC_OK) {
            break;
        }

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                              eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.mpls_opt.comm_opt.cw_en = TRUE;
        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth2_opt.comm_opt.etype = 0x88F7;

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                              eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.mpls_opt.comm_opt.cw_en = TRUE;
        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth2_opt.comm_opt.etype = 0x0800;

        /* set IP1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                             VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ip1_def_conf_priv(vtss_state, eng_parm, FALSE,
                                                        &flow_conf->flow_conf.ptp.ip1_opt,
                                                        &vtss_phy_ts_def_inner_ip_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ip1_opt, 0, sizeof(vtss_phy_ts_ip_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.comm_opt, &vtss_phy_ts_def_inner_ip_conf.comm_opt,
               sizeof(vtss_phy_ts_def_inner_ip_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.ip1_opt.flow_opt[flow_st_index], &vtss_phy_ts_def_inner_ip_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_ip_conf.flow_opt[0]));
        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        if (rc != VTSS_RC_OK) {
            break;
        }

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_ptp.flow_opt[0]));
        flow_conf->flow_conf.ptp.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        if (is_gen2 == TRUE && eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                                  eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM));
        } else {
            rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                                  eng_id, VTSS_PHY_TS_NEXT_COMP_IP1));
        }
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.ptp.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.ptp.mpls_opt.comm_opt.cw_en = FALSE;
        /* set ACH next comparator */
        if (!(is_gen2 == TRUE && eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A)) {
            rc = VTSS_RC_COLD(vtss_phy_ts_ach_next_comp_set_priv(vtss_state, port_no, blk_id, encap_type,
                                                                 VTSS_PHY_TS_NEXT_COMP_PTP_OAM));
        }
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set ACH conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ach_def_conf_priv(vtss_state, eng_parm,
                                                        &flow_conf->flow_conf.ptp.ach_opt,
                                                        &vtss_phy_ts_def_ach_conf_for_ptp));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.ptp.ach_opt, 0, sizeof(vtss_phy_ts_ach_conf_t));
        memcpy(&flow_conf->flow_conf.ptp.ach_opt.comm_opt, &vtss_phy_ts_def_ach_conf_for_ptp.comm_opt,
               sizeof(vtss_phy_ts_def_ach_conf_for_ptp.comm_opt));
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = TRUE;
        memcpy(&action_conf->action.ptp_conf[0],
               &vtss_phy_ts_def_ptp_action, sizeof(vtss_phy_ts_ptp_engine_action_t));

        rc = vtss_phy_ts_ptp_def_conf_priv(vtss_state, ingress, eng_parm);


        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_oam));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0]));
        /* etype for OAM : 0x8902 */
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x8902;

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = FALSE;
        memcpy(&action_conf->action.oam_conf[0],
               &vtss_phy_ts_def_oam_action, sizeof(vtss_phy_ts_oam_engine_action_t));

        rc = vtss_phy_ts_oam_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_OAM:
        /* Note :: ETH_ETH_OAM encapsulation is not valid for ENGINE_2B  */
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf_for_pbb));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf_for_pbb.flow_opt[0]));
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x88E7; /* PBB encap TPID */

        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_oam));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0]));
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x8902; /* etype for OAM : 0x8902 */

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = FALSE;
        memcpy(&action_conf->action.oam_conf[0],
               &vtss_phy_ts_def_oam_action, sizeof(vtss_phy_ts_oam_engine_action_t));

        rc = vtss_phy_ts_oam_def_conf_priv(vtss_state, ingress, eng_parm);

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth1_opt,
                                                         &vtss_phy_ts_def_outer_eth_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.comm_opt,
               &vtss_phy_ts_def_outer_eth_conf.comm_opt,
               sizeof(vtss_phy_ts_def_outer_eth_conf.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_outer_eth_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_outer_eth_conf.flow_opt[0]));
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                              eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.oam.mpls_opt.comm_opt.cw_en = TRUE;

        /* set eth2 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth2_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth2_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_oam));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth2_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth2_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth2_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0]));
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x8902; /* etype for OAM : 0x8902 */

        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = FALSE;
        memcpy(&action_conf->action.oam_conf[0],
               &vtss_phy_ts_def_oam_action, sizeof(vtss_phy_ts_oam_engine_action_t));

        rc = vtss_phy_ts_oam_def_conf_priv(vtss_state, ingress, eng_parm);

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        /* set eth1 next comparator */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847));
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth1 conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.eth1_opt,
                                                         &vtss_phy_ts_def_inner_eth_conf_for_oam));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.eth1_opt, 0, sizeof(vtss_phy_ts_eth_conf_t));
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.comm_opt,
               &vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt,
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.comm_opt));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.eth1_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0],
               sizeof(vtss_phy_ts_def_inner_eth_conf_for_oam.flow_opt[0]));
        flow_conf->flow_conf.oam.eth1_opt.comm_opt.etype = 0x8847; /* MPLS unicast */

        /* set MPLS next comparator */
        if (is_gen2 == TRUE && is_eng2a == TRUE) {
            rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                                  eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM));
        } else {
            rc = VTSS_RC_COLD(vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                                  eng_id, VTSS_PHY_TS_NEXT_COMP_IP1));
        }
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set MPLS conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_mpls_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.oam.mpls_opt,
                                                         &vtss_phy_ts_def_mpls_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.mpls_opt, 0, sizeof(vtss_phy_ts_mpls_conf_t));
        /* copy flow_opt at flow_st_index as it is the start index for the engine */
        memcpy(&flow_conf->flow_conf.oam.mpls_opt.flow_opt[flow_st_index],
               &vtss_phy_ts_def_mpls_conf.flow_opt[0],
               sizeof(vtss_phy_ts_def_mpls_conf.flow_opt[0]));
        flow_conf->flow_conf.oam.mpls_opt.comm_opt.cw_en = FALSE;
        if (is_eng2a != TRUE ) {
            /* set ACH next comparator */
            rc = VTSS_RC_COLD(vtss_phy_ts_ach_next_comp_set_priv(vtss_state, port_no, blk_id, encap_type,
                                                                 VTSS_PHY_TS_NEXT_COMP_PTP_OAM));
            if (rc != VTSS_RC_OK) {
                break;
            }
        }
        /* set ACH conf */
        rc = VTSS_RC_COLD(vtss_phy_ts_ach_def_conf_priv(vtss_state, eng_parm,
                                                        &flow_conf->flow_conf.oam.ach_opt,
                                                        &vtss_phy_ts_def_ach_conf_for_oam));
        if (rc != VTSS_RC_OK) {
            break;
        }
        memset(&flow_conf->flow_conf.oam.ach_opt, 0, sizeof(vtss_phy_ts_ach_conf_t));
        memcpy(&flow_conf->flow_conf.oam.ach_opt.comm_opt, &vtss_phy_ts_def_ach_conf_for_oam.comm_opt,
               sizeof(vtss_phy_ts_def_ach_conf_for_oam.comm_opt));
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));
        action_conf->action_ptp = FALSE;
        memcpy(&action_conf->action.oam_conf[0],
               &vtss_phy_ts_def_oam_action, sizeof(vtss_phy_ts_oam_engine_action_t));

        rc = vtss_phy_ts_oam_def_conf_priv(vtss_state, ingress, eng_parm);
        break;
    case VTSS_PHY_TS_ENCAP_ANY:
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                                    eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x0));
        if (rc != VTSS_RC_OK) {
            break;
        }

        ts_any_eth_conf.comm_opt = vtss_phy_ts_def_inner_eth_conf_for_ptp.comm_opt;
        for (i = flow_st_index; i < flow_end_index; i++) {
            ts_any_eth_conf.flow_opt[i].flow_en = TRUE;
            ts_any_eth_conf.flow_opt[i].addr_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST |
                                                          VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST;
        }
        rc = VTSS_RC_COLD(vtss_phy_ts_eth1_def_conf_priv(vtss_state, eng_parm,
                                                         &flow_conf->flow_conf.ptp.eth1_opt,
                                                         &ts_any_eth_conf));
        if (rc != VTSS_RC_OK) {
            break;
        }
        rc = VTSS_RC_COLD(vtss_phy_ts_ptp_ts_all_conf_priv(vtss_state, eng_parm));
        break;

    default:
        VTSS_N("Port(%u) engine_init:: invalid encapsulation type: %u", (u32)port_no, encap_type);
        break;
    }
    return rc;
}

static vtss_rc vtss_phy_ts_flow_match_mode_set_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const vtss_phy_ts_engine_flow_match_t flow_match_mode,
    const BOOL                  ingress)
{
    u32 value, temp, loc;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                 VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == FALSE) {
        temp = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_ENCAP_FLOW_MODE(value);
        loc = ((eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? eng_id : VTSS_PHY_TS_OAM_ENGINE_ID_2A);
        if (flow_match_mode == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) {
            temp |= (0x01 << loc);
        } else {
            temp &= (~((u32)(0x01 << loc))); /* do the typecast to make lint happy! */
        }
        temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_ENCAP_FLOW_MODE(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_ENCAP_FLOW_MODE) | temp;
    } else {
        temp = 0;
        loc = ((eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? eng_id : VTSS_PHY_TS_OAM_ENGINE_ID_2A);
        if ((flow_match_mode == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT)) {
            temp |= (0x01 << loc);
        } else {
            temp &= (~((u32)(0x01 << loc))); /* do the typecast to make lint happy! */
        }

        if (ingress == TRUE) {
            temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_FLOW_MODE(temp);
            temp = temp | VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_SPLIT_ENCAP_FLOW;
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_FLOW_MODE) | temp;
        } else {
            temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_FLOW_MODE(temp);
            temp = temp | VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_SPLIT_ENCAP_FLOW;
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_FLOW_MODE) | temp;
        }
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                  VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    return VTSS_RC_OK;
}
/* Forward declaration */
static vtss_rc vtss_phy_ts_engine_ptp_action_flow_delete_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const u8                   flow_index);

static vtss_rc vtss_phy_ts_engine_oam_action_flow_delete_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const u8                   flow_index);

static vtss_rc vtss_phy_ts_add_grp_mask_cold_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t                  port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_engine_t            eng_id,
    const u8                              flow_id);

static vtss_rc vtss_phy_ts_engine_clear_priv(
    vtss_state_t *vtss_state,
    const BOOL  ingress,
    const vtss_inst_t          inst,
    const vtss_port_no_t       port_no,
    const vtss_port_no_t       base_port_no,
    const vtss_phy_ts_engine_t eng_id)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_phy_ts_eng_conf_t *eng_conf, *next_eng_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf;
    vtss_phy_ts_blk_id_t blk_id;
    u32 value, temp, i;

    if (ingress) {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
    } else {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
    }

    if (!eng_conf->eng_used) {
        return VTSS_RC_ERROR;
    }
    flow_conf = &eng_conf->flow_conf;

    VTSS_RC(vtss_phy_ts_ana_blk_id_get(eng_id, ingress, &blk_id));

    /* disable the engine */
    if (flow_conf->eng_mode) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                     VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
        if (ingress) {
            temp = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(value);
        } else {
            temp = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(value);
        }
        if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            temp &= ~((u32)(0x01 << eng_id));  /* do the typecast to make lint happy! */
        } else {
            /* For OAM engine, if either 2A or 2B engine is used and enabled,
               don't disabled the engine */
            if (ingress) {
                if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                    next_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B];
                } else {
                    next_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A];
                }
            } else {
                if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                    next_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B];
                } else {
                    next_eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A];
                }
            }
            if (!next_eng_conf->flow_conf.eng_mode) {
                temp &= ~(0x01 << VTSS_PHY_TS_OAM_ENGINE_ID_2A);
            }
        }

        if (ingress) {
            temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(temp);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA) | temp;
        } else {
            temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(temp);
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA) | temp;
        }
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                      VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    }

    /* clear eth1 next comparator */
    /* PTP and OAM engine use different register naming */
    if (eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
        temp = VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR(0);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
        temp = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_NXT_COMPARATOR_A(0);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A_ETH1_NXT_COMPARATOR_A) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
    } else {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
        temp = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B(0);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B) | temp;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
    }
    /* disable each flow in ETH1 comparator */
    for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
        value = 0;
        if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
            eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
        } else {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
        }
    }

    switch (eng_conf->encap_type) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP1 comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP1 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }

        /* clear IP2 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP2 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        /* clear eth2 next comparator, only in PTP engine */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        /* clear eth2 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
        }
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP1 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        /* clear the MPLS next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP1 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        /* clear the MPLS next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }
        /* clear eth2 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        /* clear the MPLS next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }
        /* clear eth2 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
        }
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* clear IP1 UDP Checksum Configuration to default value */
        value = 0x20;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_UDP_CHKSUM_CFG, &value));
        /* disable each flow in IP1 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        /* clear the MPLS next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
        }
        /* clear IP1 next comparator */
        if ((vtss_state->phy_ts_port_conf[port_no].is_gen2 == FALSE) ||
            (eng_id != VTSS_PHY_TS_OAM_ENGINE_ID_2A)) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
            /* disable each flow in IP1 comp */
            for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            }
        } else {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ACH_COMPARATOR_A_ACH_PROT_OFFSET_A, &value));
        }

        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_OAM:
        /* clear eth2 next comparator */
        value = 0;
        if (eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        } else {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        }
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
                eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            } else {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            }
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        /* clear the MPLS next comparator */
        value = 0;
        if (eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        } else {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        }
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
                eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            } else {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            }
        }
        /* clear eth2 next comparator */
        value = 0;
        if (eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        } else {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        }
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
                eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            } else {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            }
        }

        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        /* clear the MPLS next comparator */
        value = 0;
        if (eng_id <= VTSS_PHY_TS_PTP_ENGINE_ID_1) {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        } else {
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        }
        /* disable the flows in the MPLS comparator */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
                eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            } else {
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            }
        }
        /* clear IP1 next comparator */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        /* disable each flow in IP1 comp */
        for (i = eng_conf->flow_st_index; i <= eng_conf->flow_end_index; i++) {
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        }
        break;
    case VTSS_PHY_TS_ENCAP_ANY:
        break;
    default:
        break;
    }

    return rc;
}

vtss_rc vtss_phy_ts_ingress_engine_init(const vtss_inst_t     inst,
                                        const vtss_port_no_t                  port_no,
                                        const vtss_phy_ts_engine_t            eng_id,
                                        const vtss_phy_ts_encap_t             encap_type,
                                        const u8                              flow_st_index,
                                        const u8                              flow_end_index,
                                        const vtss_phy_ts_engine_flow_match_t flow_match_mode)
{
    vtss_state_t         *vtss_state;
    vtss_rc              rc;
    vtss_port_no_t       base_port;
    vtss_phy_ts_blk_id_t blk_id;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }

        if ((rc = vtss_phy_ts_ana_blk_id_get(eng_id, TRUE, &blk_id)) == VTSS_RC_OK) {
            vtss_ts_engine_parm_t eng_parm = {port_no, base_port, blk_id,
                                              eng_id, encap_type,
                                              flow_st_index, flow_end_index
                                             };

            VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
            rc = vtss_phy_ts_engine_init_priv(vtss_state, TRUE, &eng_parm, flow_match_mode);
            VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
            if (rc != VTSS_RC_OK) {
                VTSS_E("Error code: %x", rc);
                break;
            }
            if (port_no != base_port) {
                memcpy(&vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[eng_id],
                       &vtss_state->phy_ts_port_conf[base_port].ingress_eng_conf[eng_id],
                       sizeof(vtss_phy_ts_eng_conf_t));
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ingress_engine_init_conf_get(const vtss_inst_t    inst,
                                                 const vtss_port_no_t                port_no,
                                                 const vtss_phy_ts_engine_t          eng_id,
                                                 vtss_phy_ts_eng_init_conf_t *const  eng_init_conf)
{
    vtss_state_t           *vtss_state;
    vtss_rc                rc;
    vtss_port_no_t         base_port;
    vtss_phy_ts_eng_conf_t *base_port_eng_conf;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    memset(eng_init_conf, 0, sizeof(vtss_phy_ts_eng_init_conf_t));

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }

        base_port_eng_conf = &vtss_state->phy_ts_port_conf[base_port].ingress_eng_conf[eng_id];
        eng_init_conf->eng_used = base_port_eng_conf->eng_used;
        if (eng_init_conf->eng_used) {
            eng_init_conf->encap_type = base_port_eng_conf->encap_type;
            eng_init_conf->flow_st_index = base_port_eng_conf->flow_st_index;
            eng_init_conf->flow_end_index = base_port_eng_conf->flow_end_index;
            eng_init_conf->flow_match_mode = base_port_eng_conf->flow_match_mode;
        } else {
            VTSS_D("Engine not init for engine ID: %d", eng_id);
            rc  = VTSS_RC_ERROR;
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}


vtss_rc vtss_phy_ts_ingress_engine_clear(const vtss_inst_t  inst,
                                         const vtss_port_no_t               port_no,
                                         const vtss_phy_ts_engine_t         eng_id)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 i;
    vtss_port_no_t base_port;
    vtss_phy_ts_eng_conf_t *eng_conf = NULL;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL;
    vtss_phy_ts_engine_action_t *action_conf = NULL;
    vtss_phy_ts_blk_id_t blk_id;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }

        eng_conf = &vtss_state->phy_ts_port_conf[base_port].ingress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        flow_conf = &eng_conf->flow_conf;
        action_conf = &eng_conf->action_conf;
        if (vtss_phy_ts_ana_blk_id_get(eng_id, TRUE, &blk_id) != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            break;
        }

        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
        rc = VTSS_RC_COLD(vtss_phy_ts_engine_clear_priv(vtss_state, TRUE, inst, port_no,
                                                        base_port, eng_id));
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);

        /* clear the engine config */
        memset(flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
        eng_conf->eng_used = FALSE;
        eng_conf->flow_st_index = 0;
        eng_conf->flow_end_index = 0;

        /* free the action */
        if (action_conf->action_ptp) {
            for (i = 0; i < 6; i++) {
                if (eng_conf->action_flow_map[i]) {
                    rc = VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, i));
                    eng_conf->action_flow_map[i] = 0;
                }
            }
        } else {
            for (i = 0; i < 6; i++) {
                if (eng_conf->action_flow_map[i]) {
                    rc = VTSS_RC_COLD(vtss_phy_ts_engine_oam_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, i));
                    eng_conf->action_flow_map[i] = 0;
                }
            }

        }
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));

        if (port_no != base_port) {
            memcpy(&vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[eng_id],
                   &vtss_state->phy_ts_port_conf[base_port].ingress_eng_conf[eng_id],
                   sizeof(vtss_phy_ts_eng_conf_t));
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_egress_engine_init(const vtss_inst_t      inst,
                                       const vtss_port_no_t                  port_no,
                                       const vtss_phy_ts_engine_t            eng_id,
                                       const vtss_phy_ts_encap_t             encap_type,
                                       const u8                              flow_st_index,
                                       const u8                              flow_end_index,
                                       const vtss_phy_ts_engine_flow_match_t flow_match_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port;
    vtss_phy_ts_blk_id_t blk_id;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("Invalid Engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }
        if (encap_type == VTSS_PHY_TS_ENCAP_ANY) {
            rc = VTSS_RC_ERROR;
            VTSS_E("Invalid encapsulation on egress side\n");
            break;
        }

        if ((rc = vtss_phy_ts_ana_blk_id_get(eng_id, FALSE, &blk_id)) == VTSS_RC_OK) {
            vtss_ts_engine_parm_t eng_parm = {port_no, base_port, blk_id,
                                              eng_id, encap_type,
                                              flow_st_index, flow_end_index
                                             };
            VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
            rc = vtss_phy_ts_engine_init_priv(vtss_state, FALSE, &eng_parm, flow_match_mode);
            VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);

            if (rc != VTSS_RC_OK) {
                VTSS_E("Error code: %x", rc);
                break;
            }
            if (port_no != base_port) {
                memcpy(&vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[eng_id],
                       &vtss_state->phy_ts_port_conf[base_port].egress_eng_conf[eng_id],
                       sizeof(vtss_phy_ts_eng_conf_t));
            }
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_egress_engine_init_conf_get(const vtss_inst_t    inst,
                                                const vtss_port_no_t               port_no,
                                                const vtss_phy_ts_engine_t         eng_id,
                                                vtss_phy_ts_eng_init_conf_t *const eng_init_conf)
{
    vtss_state_t           *vtss_state;
    vtss_rc                rc;
    vtss_port_no_t         base_port;
    vtss_phy_ts_eng_conf_t *base_port_eng_conf;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    memset(eng_init_conf, 0, sizeof(vtss_phy_ts_eng_init_conf_t));

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }

        base_port_eng_conf = &vtss_state->phy_ts_port_conf[base_port].egress_eng_conf[eng_id];
        eng_init_conf->eng_used = base_port_eng_conf->eng_used;
        if (eng_init_conf->eng_used) {
            eng_init_conf->encap_type = base_port_eng_conf->encap_type;
            eng_init_conf->flow_st_index = base_port_eng_conf->flow_st_index;
            eng_init_conf->flow_end_index = base_port_eng_conf->flow_end_index;
            eng_init_conf->flow_match_mode = base_port_eng_conf->flow_match_mode;
        } else {
            VTSS_D("Engine not init for engine ID: %d", eng_id);
            rc  = VTSS_RC_ERROR;
        }
    } while (0);
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_ts_egress_engine_clear(const vtss_inst_t  inst,
                                        const vtss_port_no_t              port_no,
                                        const vtss_phy_ts_engine_t        eng_id)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 i;
    vtss_port_no_t base_port;
    vtss_phy_ts_eng_conf_t *eng_conf = NULL;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL;
    vtss_phy_ts_engine_action_t *action_conf = NULL;
    vtss_phy_ts_blk_id_t blk_id;

    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port)) != VTSS_RC_OK) {
            break;
        }

        eng_conf = &vtss_state->phy_ts_port_conf[base_port].egress_eng_conf[eng_id];

        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        flow_conf = &eng_conf->flow_conf;
        action_conf = &eng_conf->action_conf;

        if (vtss_phy_ts_ana_blk_id_get(eng_id, FALSE, &blk_id) != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            break;
        }

        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
        rc = VTSS_RC_COLD(vtss_phy_ts_engine_clear_priv(vtss_state, FALSE, inst, port_no,
                                                        base_port, eng_id));
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
        /* clear the engine config */
        memset(flow_conf, 0, sizeof(vtss_phy_ts_engine_flow_conf_t));
        eng_conf->eng_used = FALSE;
        eng_conf->flow_st_index = 0;
        eng_conf->flow_end_index = 0;

        /* free the action */
        if (action_conf->action_ptp) {
            for (i = 0; i < 6; i++) {
                if (eng_conf->action_flow_map[i]) {
                    rc = VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, i));
                    eng_conf->action_flow_map[i] = 0;
                }
            }
        } else {
            for (i = 0; i < 6; i++) {
                if (eng_conf->action_flow_map[i]) {
                    rc = VTSS_RC_COLD(vtss_phy_ts_engine_oam_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, i));
                    eng_conf->action_flow_map[i] = 0;
                }
            }

        }
        memset(action_conf, 0, sizeof(vtss_phy_ts_engine_action_t));

        if (port_no != base_port) {
            memcpy(&vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[eng_id],
                   &vtss_state->phy_ts_port_conf[base_port].egress_eng_conf[eng_id],
                   sizeof(vtss_phy_ts_eng_conf_t));
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

static vtss_rc vtss_phy_ts_engine_config_priv(
    vtss_state_t *vtss_state,
    BOOL                   ingress,
    vtss_port_no_t         port_no,
    vtss_phy_ts_engine_t   eng_id,
    BOOL                   enable)
{
    u32 value, temp;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                 VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    if (ingress) {
        temp = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(value);
    } else {
        temp = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(value);
    }

    if (enable) {
        /* enable the engine if it say so in conf: for engine 2A or 2B
           if other engine is enable and disable at the begging of config
           we need to enable the whole OAM engine */
        temp |= (eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? (0x01 << eng_id) : (0x01 << VTSS_PHY_TS_OAM_ENGINE_ID_2A) ;
    } else {
        /* Disable the engine before changing the engine conf: for engine 2A or 2B
           if either of the engine is enable, disable the whole OAM engine */
        /* do the typecast to make lint happy! */
        temp &= ((eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? ~((u32)0x01 << eng_id) : ~((u32)0x01 << VTSS_PHY_TS_OAM_ENGINE_ID_2A));
    }
    if (ingress) {
        temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA) | temp;
    } else {
        temp = VTSS_F_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(temp);
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA) | temp;
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID_0,
                                  VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_flow_set_priv(
    vtss_state_t *vtss_state,
    BOOL       ingress,
    vtss_ts_engine_parm_t                *eng_parm,
    vtss_phy_ts_eng_conf_t               *eng_conf,
    const vtss_phy_ts_engine_flow_conf_t *const new_flow_conf)
{
    vtss_rc rc = VTSS_RC_OK;
    const vtss_port_no_t        port_no = eng_parm->port_no;
    const vtss_port_no_t        base_port_no = eng_parm->base_port_no;
    const vtss_phy_ts_engine_t  eng_id = eng_parm->eng_id;
    vtss_phy_ts_eng_conf_t *alt_eng_conf = NULL;


    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        alt_eng_conf = (ingress ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B] :
                        &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B]);
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        alt_eng_conf = (ingress ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A] :
                        &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A]);
    }

    /* Disable the engine before changing the engine conf: for engine 2A or 2B
       if either of the engine is enable, disable the whole OAM engine */
    if (eng_conf->flow_conf.eng_mode ||
        (alt_eng_conf && alt_eng_conf->flow_conf.eng_mode)) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, FALSE)));
    }

    switch (eng_conf->encap_type) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_priv(vtss_state, eng_parm, FALSE,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_priv(vtss_state, eng_parm, TRUE,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip2_conf_priv(vtss_state, eng_parm,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }

        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_priv(vtss_state, eng_parm, FALSE,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_priv(vtss_state, eng_parm, FALSE,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_priv(vtss_state, eng_parm, FALSE,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ach_conf_priv(vtss_state, eng_parm,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_priv(vtss_state, eng_parm,
                                             eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ach_conf_priv(vtss_state, eng_parm,
                                            eng_conf, new_flow_conf)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ANY:
        break;
    default:
        VTSS_E("Port %u:: invalid encap type", (u32)port_no);
        break;
    }

    /* enable the engine if it say so in conf: for engine 2A or 2B
       if other engine is enable and disable at the beginning of config
       we need to enable the whole OAM engine */
    if (new_flow_conf->eng_mode ||
        (alt_eng_conf && alt_eng_conf->flow_conf.eng_mode)) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, TRUE)));
    }

    return rc;
}

vtss_rc vtss_phy_ts_ingress_engine_conf_set(const vtss_inst_t  inst,
                                            const vtss_port_no_t                   port_no,
                                            const vtss_phy_ts_engine_t             eng_id,
                                            const vtss_phy_ts_engine_flow_conf_t   *const flow_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf, *alt_eng_conf;
    vtss_phy_ts_blk_id_t blk_id;

    VTSS_PHY_TS_ASSERT(flow_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }

        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
        alt_eng_conf = &vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        if ((rc = vtss_phy_ts_ana_blk_id_get(eng_id, TRUE, &blk_id)) == VTSS_RC_OK) {
            vtss_ts_engine_parm_t eng_parm = {port_no, base_port_no, blk_id,
                                              eng_id, eng_conf->encap_type,
                                              eng_conf->flow_st_index,
                                              eng_conf->flow_end_index
                                             };

            VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
            rc = vtss_phy_ts_engine_flow_set_priv(vtss_state, TRUE, &eng_parm,
                                                  eng_conf, flow_conf);
            VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
            /* save the flow config */
            if (rc == VTSS_RC_OK) {
                memcpy(&eng_conf->flow_conf, flow_conf,
                       sizeof(vtss_phy_ts_engine_flow_conf_t));
                if (port_no != base_port_no) {
                    memcpy(&alt_eng_conf->flow_conf, flow_conf,
                           sizeof(vtss_phy_ts_engine_flow_conf_t));
                }
            }
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_ingress_engine_conf_get(const vtss_inst_t   inst,
                                            const vtss_port_no_t            port_no,
                                            const vtss_phy_ts_engine_t      eng_id,
                                            vtss_phy_ts_engine_flow_conf_t  *const flow_conf)
{
    vtss_state_t           *vtss_state;
    vtss_rc                rc;
    vtss_port_no_t         base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf;

    VTSS_PHY_TS_ASSERT(flow_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        memcpy(flow_conf, &eng_conf->flow_conf, sizeof(vtss_phy_ts_engine_flow_conf_t));
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_egress_engine_conf_set(const vtss_inst_t  inst,
                                           const vtss_port_no_t                  port_no,
                                           const vtss_phy_ts_engine_t            eng_id,
                                           const vtss_phy_ts_engine_flow_conf_t  *const flow_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf, *alt_eng_conf;
    vtss_phy_ts_blk_id_t blk_id;

    VTSS_PHY_TS_ASSERT(flow_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
        alt_eng_conf = &vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }

        if ((rc = vtss_phy_ts_ana_blk_id_get(eng_id, FALSE, &blk_id)) == VTSS_RC_OK) {
            vtss_ts_engine_parm_t eng_parm = {port_no, base_port_no, blk_id,
                                              eng_id, eng_conf->encap_type,
                                              eng_conf->flow_st_index,
                                              eng_conf->flow_end_index
                                             };

            VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
            rc = vtss_phy_ts_engine_flow_set_priv(vtss_state, FALSE, &eng_parm,
                                                  eng_conf, flow_conf);
            VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
            /* save the flow config */
            if (rc == VTSS_RC_OK) {
                memcpy(&eng_conf->flow_conf, flow_conf,
                       sizeof(vtss_phy_ts_engine_flow_conf_t));
                if (port_no != base_port_no) {
                    memcpy(&alt_eng_conf->flow_conf, flow_conf,
                           sizeof(vtss_phy_ts_engine_flow_conf_t));
                }
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_egress_engine_conf_get(const vtss_inst_t    inst,
                                           const vtss_port_no_t            port_no,
                                           const vtss_phy_ts_engine_t      eng_id,
                                           vtss_phy_ts_engine_flow_conf_t  *const flow_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf;

    VTSS_PHY_TS_ASSERT(flow_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        VTSS_E("invalid engine ID");
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        memcpy(flow_conf, &eng_conf->flow_conf, sizeof(vtss_phy_ts_engine_flow_conf_t));
    } while (0);
    VTSS_EXIT();

    return rc;
}

#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
/* No of flows required for different clock mode and delay method */
u8 ptp_action_max_flow[5][2] = {
    {3, 2},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP, P2P & E2E */
    {3, 2},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP, P2P & E2E */
    {3, 4},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP, P2P & E2E */
    {3, 4},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP, P2P & E2E */
    {4, 4}   /* VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE, P2P & E2E */
};
#else
/* No of flows required for different clock mode and delay method */
u8 ptp_action_max_flow[4][2] = {
    {3, 2},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP, P2P & E2E */
    {3, 2},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP, P2P & E2E */
    {3, 4},  /* VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP, P2P & E2E */
    {3, 4}  /* VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP, P2P & E2E */
};
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */

/* No of flows required for different OAM delay method */
u8 y1731_oam_action_max_flow[2] = {
    1,  /* VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM */
    2,  /* VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM */
};

/* No of flows required for different OAM delay method */
u8 ietf_oam_action_max_flow[2] = {
    1,  /* VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_DMM */
    1,  /* VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_LDM */
};

typedef enum {
    Y1731_OAM_MSG_TYPE_1DM = 45, /* 1DM Message Type */
    Y1731_OAM_MSG_TYPE_DMR = 46, /* DMR Message Type */
    Y1731_OAM_MSG_TYPE_DMM = 47, /* DMM Message Type */
} vtss_phy_ts_y1731_oam_msg_type_t;

typedef enum {
    IETF_MPLS_ACH_OAM_DELAYM_DMM, /* Delay Measurement Message Type */
    IETF_MPLS_ACH_OAM_DELAYM_LDM, /* Loss & Delay Measurement Message Type */
} vtss_phy_ts_ietf_oam_msg_type_t;

typedef enum {
    PTP_MSG_TYPE_SYNC,
    PTP_MSG_TYPE_DELAY_REQ,
    PTP_MSG_TYPE_PDELAY_REQ,
    PTP_MSG_TYPE_PDELAY_RESP,
} vtss_phy_ts_ptp_msg_type_t;

typedef enum {
    PTP_ACTION_CMD_NOP = 0,
    PTP_ACTION_CMD_SUB = 1,
    PTP_ACTION_CMD_ADD = 3,
    PTP_ACTION_CMD_SUB_ADD = 4,
    PTP_ACTION_CMD_WRITE_1588 = 5,
    PTP_ACTION_CMD_WRITE_NS = 7,
    PTP_ACTION_CMD_WRITE_NS_P2P = 8,
    PTP_ACTION_CMD_SAVE_IN_TS_FIFO, /* not the 1588 command */
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    PTP_ACTION_CMD_DCE, /* not the 1588 command */
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
} vtss_phy_ts_ptp_action_cmd_t;

typedef enum {
    PTP_ACTION_ASYM_NONE,
    PTP_ACTION_ASYM_ADD,
    PTP_ACTION_ASYM_SUB,
} vtss_phy_ts_ptp_action_asym_t;

/* PTP action is applicable to PTP engine which is handled by action_ptp field */
static vtss_rc vtss_phy_ts_engine_ptp_action_flow_conf_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t                port_no,
    const vtss_phy_ts_blk_id_t          blk_id,
    const u8                            flow_index,
    const vtss_phy_ts_ptp_action_cmd_t  cmd,
    vtss_phy_ts_ptp_action_asym_t       asym,
    const vtss_phy_ts_ptp_msg_type_t    msg_type)
{
    u32 value;
    u8  rx_ts_pos = 0;

    if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
        rx_ts_pos = 16; /* timestamp at reserved bytes */
    }

    if (blk_id != VTSS_PHY_TS_ANA_BLK_ID_ING_2 && blk_id != VTSS_PHY_TS_ANA_BLK_ID_EGR_2) {
        /* by default no need to clear any field */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));

        switch (cmd) {
        case PTP_ACTION_CMD_NOP:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* NOP */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
        case PTP_ACTION_CMD_SUB:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* SUB */
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /*  nothing to write */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* CF bytes length */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
        case PTP_ACTION_CMD_ADD:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /*  nothing to write */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* for CF */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            /* clear frame bytes */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_BYTE_CNT(0); /* nothing to clear in Mode A */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
            break;
        case PTP_ACTION_CMD_SUB_ADD:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(rx_ts_pos); /* Ingress stored timestamp location */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* sub_add */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /* nothing to write */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* for CF */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            /* clear frame bytes */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_OFFSET(rx_ts_pos); /* stored timestamp in reserved btes should be clear */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_BYTE_CNT(4); /* 4 bytes stored timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
            break;
        case PTP_ACTION_CMD_WRITE_1588:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_1588 */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(34); /* origintimestamp offset */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(10); /* full timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
        case PTP_ACTION_CMD_WRITE_NS:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns */
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
            } else {
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
        case PTP_ACTION_CMD_WRITE_NS_P2P:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns_p2p */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            }
            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
            } else {
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
        case PTP_ACTION_CMD_SAVE_IN_TS_FIFO:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SAVE_LOCAL_TIME; /* save in FIFO */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
            } else {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP); /* need to save in FIFO only, no write */
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no rewrite */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
        case PTP_ACTION_CMD_DCE:
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
            if (asym == PTP_ACTION_ASYM_ADD) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
            } else if (asym == PTP_ACTION_ASYM_SUB) {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
            } else {
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));

            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0x22); /* no rewrite */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            break;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
        default:
            break;
        }
    } else {
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            /* by default no need to clear any field */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));

            switch (cmd) {
            case PTP_ACTION_CMD_NOP:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* NOP */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
            case PTP_ACTION_CMD_SUB:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* SUB */
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /*  nothing to write */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* CF bytes length */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
            case PTP_ACTION_CMD_ADD:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /*  nothing to write */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* for CF */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                /* clear frame bytes */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_BYTE_CNT(0); /* nothing to clear in Mode A */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
                break;
            case PTP_ACTION_CMD_SUB_ADD:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(rx_ts_pos); /* Ingress stored timestamp location */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* sub_add */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /* nothing to write */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* for CF */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                /* clear frame bytes */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_OFFSET(rx_ts_pos); /* stored timestamp in reserved btes should be clear */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_BYTE_CNT(4); /* 4 bytes stored timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
                break;
            case PTP_ACTION_CMD_WRITE_1588:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(34); /* origintimestamp offset */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(10); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
            case PTP_ACTION_CMD_WRITE_NS:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns */
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
                    value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
                } else {
                    value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
            case PTP_ACTION_CMD_WRITE_NS_P2P:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns_p2p */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                }
                if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_STAT_UPDATE;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_MOD_FRAME_BYTE_OFFSET(6);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
                    value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
                } else {
                    value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
            case PTP_ACTION_CMD_SAVE_IN_TS_FIFO:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SAVE_LOCAL_TIME; /* save in FIFO */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
                } else {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP); /* need to save in FIFO only, no write */
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no rewrite */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            case PTP_ACTION_CMD_DCE:
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
                if (asym == PTP_ACTION_ASYM_ADD) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
                } else if (asym == PTP_ACTION_ASYM_SUB) {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
                } else {
                    value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP);
                }
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));

                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0x22); /* no rewrite */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
                break;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            default:
                break;
            }
        }
    }

    return VTSS_RC_OK;
}

/* PTP action is applicable to PTP engine which is handled by action_ptp field */
static vtss_rc vtss_phy_ts_engine_ptp_action_flow_add_priv(
    vtss_state_t *vtss_state,
    BOOL    ingress,
    const vtss_port_no_t                  port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_engine_t            eng_id,
    const vtss_phy_ts_ptp_msg_type_t      msg_type,
    const u8                              flow_index,
    const vtss_phy_ts_ptp_engine_action_t *const action_conf)
{
    u32 value;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* flow enable */
        value = VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
        value |= VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));

        /* domain config */
        if (action_conf->ptp_conf.range_en) {
            value = VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(4);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->ptp_conf.domain.range.upper);
            value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->ptp_conf.domain.range.lower);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            /* clear mask related to domain */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        } else {
            /* disable domain range */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            value &= ~VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

            /* set mask */
            /* PTP_FLOW_MATCH_UPPER: msg type to msg length with msg type in MS bytes
               PTP_FLOW_MATCH_LOWER: domain number which will be MB byte */
            value = action_conf->ptp_conf.domain.value.mask << 24;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            value = action_conf->ptp_conf.domain.value.val << 24;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        }

        if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified && !ingress) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            value |= 0x8000;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            value |= 0xf000;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
        }

        /* set message type in flow match lower */
        /* PTP_FLOW_MATCH_UPPER: msg type to msg length with msg type in MS bytes
           PTP_FLOW_MATCH_LOWER: domain number which will be MB byte */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        value &= ~0x0F000000;
        value |= (msg_type << 24);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        value |= 0x0F000000;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
    } else {
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            /* flow enable */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
            if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x01);
            } else {
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x02);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));

            /* domain config */
            if (action_conf->ptp_conf.range_en) {
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(4);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->ptp_conf.domain.range.upper);
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->ptp_conf.domain.range.lower);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
                /* clear mask related to domain */
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            } else {
                /* disable domain range */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
                value &= ~VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

                /* set mask */
                /* PTP_FLOW_MATCH_UPPER: msg type to msg length with msg type in MS bytes
                   PTP_FLOW_MATCH_LOWER: domain number which will be MB byte */
                value = action_conf->ptp_conf.domain.value.mask << 24;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
                value = action_conf->ptp_conf.domain.value.val << 24;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
                value = 0;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            }

            if (vtss_state->phy_ts_port_conf[port_no].chk_ing_modified && !ingress) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
                value |= 0x8000;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
                value |= 0xf000;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            }

            /* set message type in flow match lower */
            /* PTP_FLOW_MATCH_UPPER: msg type to msg length with msg type in MS bytes
               PTP_FLOW_MATCH_LOWER: domain number which will be MB byte */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            value &= ~0x0F000000;
            value |= (msg_type << 24);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            value |= 0x0F000000;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        }
    }

    switch (msg_type) {
    case PTP_MSG_TYPE_SYNC:
        if (ingress) {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS_P2P,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField); add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField); add(PathDelay + Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS_P2P,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /*VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        } else {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* write(TX_timestamp, originTimestamp) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_1588,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(TX_timestamp, originTimestamp) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_1588,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* save(TX_timestamp, TXFifo) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* save(TX_timestamp, TXFifo) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /*VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */

            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        }
        break;
    case PTP_MSG_TYPE_DELAY_REQ:
        if (ingress) {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* write(RX_timestamp,  Reserved) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* write(RX_timestamp,  Reserved) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        } else {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
                /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_SUB, msg_type));
#else
                /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SUB_ADD,
                                                                     PTP_ACTION_ASYM_SUB, msg_type));
#endif
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                /* save(TX_timestamp, TXFiFo) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField); sub(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_SUB, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_SUB, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /*VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        }
        break;
    case PTP_MSG_TYPE_PDELAY_REQ:
        if (ingress) {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        } else {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
                /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_SUB, msg_type));
#else
                /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SUB_ADD,
                                                                     PTP_ACTION_ASYM_SUB, msg_type));
#endif
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* save(TX_timestamp,  TXFiFo) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField); sub(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_SUB, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_SUB, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_SUB, msg_type));
                }
#else
                /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SUB_ADD,
                                                                     PTP_ACTION_ASYM_SUB, msg_type));
#endif
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp,  TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        }
        break;
    case PTP_MSG_TYPE_PDELAY_RESP:
        if (ingress) {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_WRITE_NS,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* sub(RX_timestamp, correctionField); add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS, PTP_ACTION_ASYM_ADD, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_WRITE_NS,
                                                                         PTP_ACTION_ASYM_ADD, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif  /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        } else {
            if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
                action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* Subtract_add(TX_timestamp, Reserved, correctionField) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SUB_ADD,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                /* save(TX_timestamp,  TXFiFo) */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
                    /* add(TX_timestamp, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                } else if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B) {
                    /* Subtract_add(TX_timestamp, Reserved, correctionField) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SUB_ADD,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp, TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
                       action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
                if ((vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) ||
                    (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_B)) {
                    /* save(TX_timestamp,  TXFiFo) */
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                         blk_id, flow_index,
                                                                         PTP_ACTION_CMD_SAVE_IN_TS_FIFO,
                                                                         PTP_ACTION_ASYM_NONE, msg_type));
                }
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
            } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_DCE,
                                                                     PTP_ACTION_ASYM_ADD, msg_type));
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
            } else {
                /* NOP */
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_priv(vtss_state, port_no,
                                                                     blk_id, flow_index,
                                                                     PTP_ACTION_CMD_NOP,
                                                                     PTP_ACTION_ASYM_NONE, msg_type));
            }
        }
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ptp_ts_all_conf_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t *eng_parm)
{
    const vtss_port_no_t        port_no        = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t  blk_id         = eng_parm->blk_id;
    const vtss_phy_ts_engine_t  eng_id         = eng_parm->eng_id;
    const u8                    flow_st_index  = eng_parm->flow_st_index;
    const u8                    flow_end_index = eng_parm->flow_end_index;
    u32 value, i;

    /*Reduce the preamble by 4 bytes to append timestamp at the end */
    vtss_state->phy_ts_port_conf[port_no].rx_ts_pos = VTSS_PHY_TS_RX_TIMESTAMP_POS_AT_END;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0), VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
    value = value | VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_REDUCE_PREAMBLE;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0), VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        for (i = flow_st_index; i < flow_end_index; i++) {
            value = VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(3);
            value = value | VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(i), &value));
            /* Disable domain and flow matching */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(i), &value));
            /* Configure action as WRITE_NS */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(i), &value));
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND) |
                    VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_NS);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(i), &value));

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(i), &value));
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES);
            value = value | VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(i), &value));
        }
    } else {
        for (i = flow_st_index; i < flow_end_index; i++) {
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(3);
            value = value | VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(i), &value));
            /* Disable domain and flow matching */
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(i), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(i), &value));
            /* Configure action as WRITE_NS */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(i), &value));
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND) |
                    VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_NS);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(i), &value));

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(i), &value));
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES);
            value = value | VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(i), &value));
        }
    }

    return VTSS_RC_OK;
}


#define VTSS_PHY_TS_Y1731_OAM_DM_VERSION_POS           0
#define VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_1              4
#define VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_2             12
#define VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_3             20
#define VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_4             28
#define VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN                8
#define VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT   16
#define VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT    24
#define VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT     5

static vtss_rc vtss_phy_ts_engine_y1731_oam_action_flow_add_priv(
    vtss_state_t *vtss_state,
    BOOL    ingress,
    const vtss_port_no_t                    port_no,
    const vtss_phy_ts_blk_id_t              blk_id,
    const vtss_phy_ts_engine_t              eng_id,
    const vtss_phy_ts_y1731_oam_msg_type_t  msg_type,
    const u8                                flow_index,
    const vtss_phy_ts_oam_engine_action_t   *const action_conf)
{
    u32 value;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* flow enable */
        value = VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
        value |= VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));

        /* MEG level config */
        if (action_conf->oam_conf.y1731_oam_conf.range_en) {
            /* Note :: MEG Level range checking is valid for the OAM frames with version = 0 by default
             *         Application should mention the version.
             */
            value = VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_VERSION_POS);
            value |= (VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->oam_conf.y1731_oam_conf.meg_level.range.upper << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) | action_conf->version);
            value |= (VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->oam_conf.y1731_oam_conf.meg_level.range.lower << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) | action_conf->version);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            /* clear mask related to MEG level */
            /* Note :: meglevel and the message type are in the same 4 byte boundary */
            /* MEG_lvl - 3 bits, version - 5 bits [1], Msg_type - 1bytes [2] */

            value = 0xff << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            value = msg_type << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        } else {
            /* disable MEG level range */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            value &= ~VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

            /* set mask */
            /* Only MEG level is used for qualifying and requires only one matching field, so using upper
             */
            value = (u32)0xff << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            if (action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask) {
                value |= ((action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask | 0x1f) << VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            value = action_conf->version;
            if (action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask) {
                value |= (action_conf->oam_conf.y1731_oam_conf.meg_level.value.val << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) ;
            }
            value = (value << VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT) | msg_type << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        }


        switch (msg_type) {
        case Y1731_OAM_MSG_TYPE_1DM :
            if (ingress) {
                /* Ingress */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_2); /* TimeStamp - 2 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            } else {
                /* Egress */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_1); /* TimeStamp - 1 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            }
            break;
        case Y1731_OAM_MSG_TYPE_DMM :
            if (ingress) {
                /* Ingress */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_2); /* TimeStamp - 2 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte  timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));

            } else {
                /* Egress */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_1); /* TimeStamp - 1 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            }
            break;
        case Y1731_OAM_MSG_TYPE_DMR:
            if (ingress) {
                /* Used for DMR */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_4); /* TimeStamp - 4 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            } else {
                /* Used for DMR */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_3); /* TimeStamp - 3 */
                value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));

            }
            break;
        default:
            break;
        }
    } else {
        /* For Engine- 2A and Engine 2B */
        /* flow enable */
        value = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ? 0x01 : 0x02;
        value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(value);
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* MEG level config */
        if (action_conf->oam_conf.y1731_oam_conf.range_en) {
            /* Note :: MEG Level range checking is valid for the OAM frames with version = 0 by default
             *         Application should mention the version.
             */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA ;
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(0);
            value |= (VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->oam_conf.y1731_oam_conf.meg_level.range.upper << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) | action_conf->version);
            value |= (VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->oam_conf.y1731_oam_conf.meg_level.range.lower << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) | action_conf->version);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            /* clear mask related to MEG level */
            /* Note :: meglevel and the message type are in the same 4 byte boundary */
            /* MEG_lvl - 3 bits, version - 5 bits [1], Msg_type - 1bytes [2] */
            value = msg_type << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));

            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        } else {
            /* disable MEG level range */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            value &= ~VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

            /* set mask */
            /* Only MEG level is used for qualifying and requires only one matching field, so using upper
             */
            value = (u32)0xff << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            if (action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask) {
                value |= ((action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask | 0x1f) << VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT);
            }
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
            value = action_conf->version;
            if (action_conf->oam_conf.y1731_oam_conf.meg_level.value.mask) {
                value |= (action_conf->oam_conf.y1731_oam_conf.meg_level.value.val << VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT) ;
            }
            value = (value << VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT) | msg_type << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
            value = 0;
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        }
        switch (msg_type) {
        case Y1731_OAM_MSG_TYPE_1DM:
            if (ingress) {
                /* Ingress */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_2); /* TimeStamp - 2 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            } else {
                /* Egress */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_1); /* TimeStamp - 1 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            }
            break;
        case Y1731_OAM_MSG_TYPE_DMM:
            if (ingress) {
                /* Ingress */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));

                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_2); /* TimeStamp - 2 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));

            } else {
                /* Egress */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_1); /* TimeStamp - 1 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* full timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            }
            break;
        case Y1731_OAM_MSG_TYPE_DMR:
            if (ingress) {
                /* Used for DMR */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_4); /* TimeStamp - 4 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            } else {
                /* Egress */
                /* Used for DMR */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
                /* action_2 setting */
                value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_Y1731_OAM_DM_TS_POS_3); /* TimeStamp - 3 */
                value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_Y1731_OAM_DM_TS_LEN); /* 8 Byte timestamp */
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
            }
            break;
        default:
            break;
        }
    }
    return VTSS_RC_OK;
}

#define VTSS_PHY_TS_IETF_DM_T_FLAG_BIT_POS  26
#define VTSS_PHY_TS_IETF_DM_VERSION_POS     28
#define VTSS_PHY_TS_IETF_DM_RTF_FIELD_POS   24
#define VTSS_PHY_TS_IETF_LDM_RTF_FIELD_POS  20
#define VTSS_PHY_TS_IETF_DM_DS_FIELD_POS    11
#define VTSS_PHY_TS_IETF_DM_TX_TS_OFFSET    12
#define VTSS_PHY_TS_IETF_DM_RX_TS_OFFSET    20
#define VTSS_PHY_TS_IETF_DM_PTP_TSF_LENGTH   8

static vtss_rc vtss_phy_ts_engine_ietf_oam_action_flow_add_priv(
    vtss_state_t *vtss_state,
    BOOL    ingress,
    const vtss_port_no_t                    port_no,
    const vtss_phy_ts_blk_id_t              blk_id,
    const vtss_phy_ts_ietf_oam_msg_type_t   msg_type,
    const u8                                flow_index,
    const vtss_phy_ts_oam_engine_action_t   *const action_conf)
{
    u32 value = 0;
    u32 mask = 0;
    if (blk_id == VTSS_PHY_TS_ANA_BLK_ID_ING_2 || blk_id == VTSS_PHY_TS_ANA_BLK_ID_EGR_2 ) {
        /* flow enable */
        value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(1);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* Enable the DS [Traffic class]
         */
        value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
        /* Support only for one Traffic Class
         */
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->oam_conf.ietf_oam_conf.ds);
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->oam_conf.ietf_oam_conf.ds);
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(VTSS_PHY_TS_IETF_DM_DS_FIELD_POS);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

        /* set mask */
        /* Select the Traffic Class Bit; For DM This Flag is set
         */
        value = action_conf->version << VTSS_PHY_TS_IETF_DM_VERSION_POS;
        value |= (u32)1 << VTSS_PHY_TS_IETF_DM_T_FLAG_BIT_POS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        value = (u32)0xf << VTSS_PHY_TS_IETF_DM_VERSION_POS;
        value |= (u32)1 << VTSS_PHY_TS_IETF_DM_T_FLAG_BIT_POS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        value = mask = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &mask));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));

        if (ingress) {
            /* Ingress */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));

            /* action_2 setting */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_IETF_DM_RX_TS_OFFSET); /* RxTimestamp offset */
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_IETF_DM_PTP_TSF_LENGTH); /* full timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        } else {
            /* Egress */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_IETF_DM_TX_TS_OFFSET); /* TxTimestamp offset */
            value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_IETF_DM_PTP_TSF_LENGTH); /* full timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        }
    } else {
        /* flow enable */
        value = VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA;
        value |= VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(action_conf->channel_map);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* Enable the DS [Traffic class]
         */
        value = VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA;
        /* Support only for one Traffic Class
         */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(action_conf->oam_conf.ietf_oam_conf.ds);
        value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(action_conf->oam_conf.ietf_oam_conf.ds);
        value |= VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(VTSS_PHY_TS_IETF_DM_DS_FIELD_POS);
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

        /* set mask */
        /* Select the Traffic Class Bit; For DM This Flag is set
         */
        value = action_conf->version << VTSS_PHY_TS_IETF_DM_VERSION_POS;
        value |= (u32)1 << VTSS_PHY_TS_IETF_DM_T_FLAG_BIT_POS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        value = (u32)0xf << VTSS_PHY_TS_IETF_DM_VERSION_POS;
        value |= (u32)1 << VTSS_PHY_TS_IETF_DM_T_FLAG_BIT_POS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        value = mask = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &mask));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));

        if (ingress) {
            /* Ingress */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_IETF_DM_RX_TS_OFFSET); /* RxTimestamp offset */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_IETF_DM_PTP_TSF_LENGTH); /* full timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        } else {
            /* Egress */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* write_1588 */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(4);
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
            /* action_2 setting */
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(VTSS_PHY_TS_IETF_DM_TX_TS_OFFSET); /* TxTimestamp offset */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(VTSS_PHY_TS_IETF_DM_PTP_TSF_LENGTH); /* full timestamp */
            VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        }
    }
    return VTSS_RC_OK;
}


/* PTP action is only on PTP engine */
static vtss_rc vtss_phy_ts_engine_ptp_action_flow_delete_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const u8                   flow_index)
{
    u32 value, temp;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* flow disable */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* clear other action registers */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
    } else {
        /* For OAM engine, action may be shared, check before disable the flow */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        temp = VTSS_X_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(value);
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* Gr 2B is set, don't delete the entry, just remove Gr 2A */
            if (temp & 0x02) {
                temp = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x02);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK) | temp;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
                return VTSS_RC_OK;
            }
        } else {
            /* Gr 2A is set, don't delete the entry, just remove Gr 2B */
            if (temp & 0x01) {
                temp = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x01);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK) | temp;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
                return VTSS_RC_OK;
            }
        }

        /* that means flow is not shared, so need to be removed */
        /* flow disable */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* clear other action registers */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_oam_action_flow_delete_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t       port_no,
    const vtss_phy_ts_blk_id_t blk_id,
    const vtss_phy_ts_engine_t eng_id,
    const u8                   flow_index)
{
    u32 value, temp;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* flow disable */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* clear other action registers */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
    } else {
        /* For OAM engine, action may be shared, check before disable the flow */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        temp = VTSS_X_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(value);
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* Gr 2B is set, don't delete the entry, just remove Gr 2A */
            if (temp & 0x02) {
                temp = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x02);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK) | temp;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
                return VTSS_RC_OK;
            }
        } else {
            /* Gr 2A is set, don't delete the entry, just remove Gr 2B */
            if (temp & 0x01) {
                temp = VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x01);
                value = VTSS_PHY_TS_CLR_BITS(value, VTSS_M_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK) | temp;
                VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
                return VTSS_RC_OK;
            }
        }

        /* that means flow is not shared, so need to be removed */
        /* flow disable */
        value = 0;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        /* clear other action registers */
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION_2(flow_index), &value));
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &value));
    }

    return VTSS_RC_OK;
}

/* Add an PTP action in PTP engine: OAM engine does not support PTP */
static vtss_rc vtss_phy_ts_engine_ptp_action_add_priv(
    vtss_state_t *vtss_state,
    BOOL  ingress,
    const vtss_port_no_t                  port_no,
    const vtss_port_no_t                  base_port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_engine_t            eng_id,
    vtss_phy_ts_eng_conf_t                *eng_conf,
    const vtss_phy_ts_ptp_engine_action_t *const action_conf,
    const u8                              action_index)
{
    BOOL flow_index[6];
#ifndef VTSS_FEATURE_WARM_START
    u32 value;
#endif /* VTSS_FEATURE_WARM_START */
    u32 act_ct, empty_cnt = 0;
    u8 num_flow = ptp_action_max_flow[action_conf->clk_mode][action_conf->delaym_type], i;

    /* engine 2A and 2B shared the action; check if other engine has same
       action or not. If yes, no need to add another flow for that, rather
       add the engine into engine group mask */
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A || eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        vtss_phy_ts_engine_action_t *other_action;
        vtss_phy_ts_eng_conf_t      *int_eng_conf;
        u8 other_action_index = 0;
        BOOL found = FALSE;
        u8 *action_flow_map;
        vtss_phy_ts_engine_t shared_eng_id = 0, zero_based_shared_eng_id = 0;

        int_eng_conf = (ingress) ?
                       vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf  :
                       vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf;

        shared_eng_id = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?
                        VTSS_PHY_TS_OAM_ENGINE_ID_2B :
                        VTSS_PHY_TS_OAM_ENGINE_ID_2A;

        zero_based_shared_eng_id = shared_eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0;
        other_action = &int_eng_conf[zero_based_shared_eng_id].action_conf;
        action_flow_map = int_eng_conf[zero_based_shared_eng_id].action_flow_map;

        /* Search other engine for matching action */
        /* TOTAL_PTP_ACTIONS_PER_ENGINE - 2 */
        for (act_ct = 0; act_ct < 2; act_ct++) {
            if (memcmp(action_conf, &other_action->action.ptp_conf[act_ct], sizeof(vtss_phy_ts_ptp_engine_action_t)) == 0) {
                other_action_index = act_ct + 1; /* index start with 1 */
                found = TRUE;
                break;
            }
        }
        /* add the grp mask into those flows where other engine
           matching action has */
        if (found) {
            for (i = 0; i < 6; i++) {
                if (action_flow_map[i] == other_action_index) {
                    eng_conf->action_flow_map[i] = action_index + 1;
                    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_add_grp_mask_cold_priv(vtss_state, port_no, blk_id, eng_id, i)));
                }
            }
            return VTSS_RC_OK;
        }
    }

    /* For Engine 2A or 2B if not found matching index and also for
       engine 0 and 1 we need to add new flow for the action */
    memset(flow_index, FALSE, sizeof(flow_index));
    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* Find the flow index which are free and number of available flow
           should be at least reqd number of flow */
        for (i = 0; i < 6; i++) {
#if defined(VTSS_FEATURE_WARM_START)
            if (eng_conf->action_flow_map[i] == 0) {
#else
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(i), &value));
            if ((value & VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA) == FALSE) {
#endif/*VTSS_FEATURE_WARM_START*/
                if (eng_conf->action_flow_map[i] == 0) {
                    flow_index[empty_cnt] = i;
                    empty_cnt++;
                    if (num_flow == empty_cnt) {
                        break;
                    }
                }
            }
        }
    }
    else { /* Engine 2A or 2B */
        vtss_phy_ts_eng_conf_t  *shared_eng_conf = NULL;
        vtss_phy_ts_engine_t     shared_eng_id = 0,
                                 zero_based_shared_eng_id = 0;

        shared_eng_id = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?
                        VTSS_PHY_TS_OAM_ENGINE_ID_2B :
                        VTSS_PHY_TS_OAM_ENGINE_ID_2A;

        zero_based_shared_eng_id = shared_eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0;
        shared_eng_conf = (ingress) ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[zero_based_shared_eng_id] :
                          &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[zero_based_shared_eng_id];
        /* Find the flow index which are free and number of available flow
           should be at least reqd number of flow */
        for (i = 0; i < 6; i++) {
            if ((eng_conf->action_flow_map[i] == 0) && (shared_eng_conf->action_flow_map[i] == 0)) {
                flow_index[empty_cnt] = i;
                empty_cnt++;
                if (num_flow == empty_cnt) {
                    break;
                }
            }
        }
    }

    if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
        action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_DELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_DELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_DELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[3], action_conf)));
        eng_conf->action_flow_map[flow_index[3]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_DELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[3], action_conf)));
        eng_conf->action_flow_map[flow_index[3]] = action_index + 1;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_SYNC, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_DELAY_REQ, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_REQ, flow_index[2], action_conf)));
        eng_conf->action_flow_map[flow_index[2]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_add_priv(vtss_state, ingress,
                                                                         port_no, blk_id, eng_id, PTP_MSG_TYPE_PDELAY_RESP, flow_index[3], action_conf)));
        eng_conf->action_flow_map[flow_index[3]] = action_index + 1;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_add_grp_mask_cold_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t                  port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_engine_t            eng_id,
    const u8                              flow_id)
{
    u32 value;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_id), &value));
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x01); /* add group A */
    } else {
        value |= VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(0x02); /* add group B */
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_id), &value));
    return VTSS_RC_OK;
}

/* Add a Y.1731 OAM action either in PTP engine or OAM engine */
static vtss_rc vtss_phy_ts_engine_y1731_oam_action_add_priv(
    vtss_state_t *vtss_state,
    BOOL  ingress,
    const vtss_port_no_t                  port_no,
    const vtss_port_no_t                  base_port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_engine_t            eng_id,
    vtss_phy_ts_eng_conf_t                *eng_conf,
    const vtss_phy_ts_oam_engine_action_t *const action_conf,
    const u8                              action_index)
{
    BOOL flow_index[6];
    u32 act_ct, empty_cnt = 0;
    u8 num_flow = y1731_oam_action_max_flow[action_conf->oam_conf.y1731_oam_conf.delaym_type], i;

    /* engine 2A and 2B shared the action; check if other engine has same
       action or not. If yes, no need to add another flow for that, rather
       add the engine into engine group mask */
    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A || eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        vtss_phy_ts_engine_action_t *other_action;
        vtss_phy_ts_eng_conf_t      *int_eng_conf;
        u8 other_action_index = 0;
        BOOL found = FALSE;
        u8 *action_flow_map;
        vtss_phy_ts_engine_t shared_eng_id = 0, zero_based_shared_eng_id = 0;

        int_eng_conf = (ingress) ?
                       vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf  :
                       vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf;

        shared_eng_id = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?
                        VTSS_PHY_TS_OAM_ENGINE_ID_2B :
                        VTSS_PHY_TS_OAM_ENGINE_ID_2A;

        zero_based_shared_eng_id = shared_eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0;
        other_action = &int_eng_conf[zero_based_shared_eng_id].action_conf;
        action_flow_map = int_eng_conf[zero_based_shared_eng_id].action_flow_map;

        /* Search other engine for matching action */
        /* TOTAL_OAM_ACTIONS_PER_ENGINE - 6 */
        for (act_ct = 0; act_ct < 6; act_ct++) {
            if (memcmp(action_conf, &other_action->action.oam_conf[act_ct], sizeof(vtss_phy_ts_oam_engine_action_t)) == 0) {
                other_action_index = act_ct + 1; /* index start with 1 */
                found = TRUE;
                break;
            }
        }
        /* add the grp mask into those flows where other engine
           matching action has */
        if (found) {
            for (i = 0; i < 6; i++) {
                if (action_flow_map[i] == other_action_index) {
                    eng_conf->action_flow_map[i] = action_index + 1;
                    VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_add_grp_mask_cold_priv(vtss_state, port_no, blk_id, eng_id, i)));
                }
            }
            return VTSS_RC_OK;
        }
    }

    /* For Engine 2A or 2B if not found matching index and also for
       engine 0 and 1 we need to add new flow for the action */
    memset(flow_index, FALSE, sizeof(flow_index));
    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 || eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* Find the flow index which are free and number of available flow
           should be at least reqd number of flow */
        for (i = 0; i < 6; i++) {
            if (eng_conf->action_flow_map[i] == 0) {
                flow_index[empty_cnt] = i;
                empty_cnt++;
                if (num_flow == empty_cnt) {
                    break;
                }
            }
        }
    } else { /* Engine 2A or 2B */
        vtss_phy_ts_eng_conf_t  *shared_eng_conf = NULL;
        vtss_phy_ts_engine_t     shared_eng_id = 0,
                                 zero_based_shared_eng_id = 0;

        shared_eng_id = (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?
                        VTSS_PHY_TS_OAM_ENGINE_ID_2B :
                        VTSS_PHY_TS_OAM_ENGINE_ID_2A;

        zero_based_shared_eng_id = shared_eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0;
        shared_eng_conf = (ingress) ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[zero_based_shared_eng_id] :
                          &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[zero_based_shared_eng_id];
        /* Find the flow index which are free and number of available flow
           should be at least reqd number of flow */
        for (i = 0; i < 6; i++) {
            if ((eng_conf->action_flow_map[i] == 0) && (shared_eng_conf->action_flow_map[i] == 0)) {
                flow_index[empty_cnt] = i;
                empty_cnt++;
                if (num_flow == empty_cnt) {
                    break;
                }
            }
        }
    }

    if (num_flow > empty_cnt) {
        return VTSS_RC_ERROR;
    }
    if (action_conf->oam_conf.y1731_oam_conf.delaym_type == VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_y1731_oam_action_flow_add_priv(vtss_state, ingress,
                                                                               port_no, blk_id, eng_id, Y1731_OAM_MSG_TYPE_1DM, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
    } else if (action_conf->oam_conf.y1731_oam_conf.delaym_type == VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_y1731_oam_action_flow_add_priv(vtss_state, ingress,
                                                                               port_no, blk_id, eng_id, Y1731_OAM_MSG_TYPE_DMM, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_y1731_oam_action_flow_add_priv(vtss_state, ingress,
                                                                               port_no, blk_id, eng_id, Y1731_OAM_MSG_TYPE_DMR, flow_index[1], action_conf)));
        eng_conf->action_flow_map[flow_index[1]] = action_index + 1;
    }
    return VTSS_RC_OK;
}

/* Add a IETF OAM action either in PTP engine or OAM engine */
static vtss_rc vtss_phy_ts_engine_ietf_oam_action_add_priv(
    vtss_state_t *vtss_state,
    BOOL  ingress,
    const vtss_port_no_t                  port_no,
    const vtss_port_no_t                  base_port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    vtss_phy_ts_eng_conf_t                *eng_conf,
    const vtss_phy_ts_oam_engine_action_t *const action_conf,
    const u8                              action_index)
{
    BOOL flow_index[6];
    u32 i,/* act_ct,*/ empty_cnt = 0;
    u8 num_flow = ietf_oam_action_max_flow[action_conf->oam_conf.ietf_oam_conf.delaym_type];

    /* IETF is not applicable for Engine 2A or 2B , because it doesn't have the MPLS comparater */
    memset(flow_index, FALSE, sizeof(flow_index));
    /* Find the flow index which are free and number of available flow
       should be at least reqd number of flow */
    for (i = 0; i < 6; i++) {
#if defined(VTSS_FEATURE_WARM_START)
        if (eng_conf->action_flow_map[i] == 0) {
#else
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(i), &value));
        if ((value & VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA) == FALSE) {
#endif/*VTSS_FEATURE_WARM_START*/
            flow_index[empty_cnt] = i;
            empty_cnt++;
            if (num_flow == empty_cnt) {
                break;
            }
        }
    }
    if (num_flow > empty_cnt) {
        return VTSS_RC_ERROR;
    }
    if (action_conf->oam_conf.ietf_oam_conf.delaym_type == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_DMM) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ietf_oam_action_flow_add_priv(vtss_state, ingress,
                                                                              port_no, blk_id, IETF_MPLS_ACH_OAM_DELAYM_DMM, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
    } else if (action_conf->oam_conf.ietf_oam_conf.delaym_type == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_LDM) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ietf_oam_action_flow_add_priv(vtss_state, ingress,
                                                                              port_no, blk_id, IETF_MPLS_ACH_OAM_DELAYM_LDM, flow_index[0], action_conf)));
        eng_conf->action_flow_map[flow_index[0]] = action_index + 1;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ptp_def_conf_priv(
    vtss_state_t *vtss_state,
    BOOL                  ingress,
    vtss_ts_engine_parm_t *eng_parm)
{
    vtss_phy_ts_engine_channel_map_t chan_map;
    vtss_phy_ts_eng_conf_t *eng_conf;
    vtss_phy_ts_ptp_engine_action_t def_action;
    const vtss_port_no_t         port_no        = eng_parm->port_no;
    const vtss_port_no_t         base_port_no   = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t   blk_id         = eng_parm->blk_id;
    const vtss_phy_ts_engine_t   eng_id         = eng_parm->eng_id;

    /* set default action config */
    if (ingress) {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
    } else {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
    }
    /* set the default channel */
    chan_map = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
    eng_conf->action_conf.action.ptp_conf[0].channel_map = chan_map;

    def_action = vtss_phy_ts_def_ptp_action;
    def_action.channel_map = chan_map;

    VTSS_RC(vtss_phy_ts_engine_ptp_action_add_priv(vtss_state, ingress, port_no, base_port_no,
                                                   blk_id, eng_id, eng_conf, &def_action, 0));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_oam_def_conf_priv(
    vtss_state_t *vtss_state,
    BOOL                  ingress,
    vtss_ts_engine_parm_t *eng_parm)
{
    vtss_phy_ts_engine_channel_map_t chan_map;
    vtss_phy_ts_eng_conf_t *eng_conf;
    vtss_phy_ts_oam_engine_action_t def_action;
    const vtss_port_no_t         port_no        = eng_parm->port_no;
    const vtss_port_no_t         base_port_no   = eng_parm->base_port_no;
    const vtss_phy_ts_blk_id_t   blk_id         = eng_parm->blk_id;
    const vtss_phy_ts_engine_t   eng_id         = eng_parm->eng_id;

    /* set default action config */
    if (ingress) {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0];
    } else {
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id - VTSS_PHY_TS_PTP_ENGINE_ID_0];
    }
    /* set the default channel */
    chan_map = ((port_no == base_port_no) ? VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0 : VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1);
    eng_conf->action_conf.action.oam_conf[0].channel_map = chan_map;
    def_action = vtss_phy_ts_def_oam_action;
    def_action.channel_map = chan_map;
    if (def_action.y1731_en) {
        VTSS_RC(vtss_phy_ts_engine_y1731_oam_action_add_priv(vtss_state, ingress, port_no, base_port_no,
                                                             blk_id, eng_id, eng_conf, &def_action, 0));
    } else {
        VTSS_RC(vtss_phy_ts_engine_ietf_oam_action_add_priv(vtss_state, ingress, port_no, base_port_no,
                                                            blk_id, eng_conf, &def_action, 0));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_action_set_priv(
    vtss_state_t *vtss_state,
    BOOL  ingress,
    vtss_port_no_t                    port_no,
    vtss_port_no_t                    base_port_no,
    vtss_phy_ts_engine_t              eng_id,
    vtss_phy_ts_eng_conf_t            *eng_conf,
    const vtss_phy_ts_engine_action_t *const new_action_conf)
{
    vtss_phy_ts_blk_id_t blk_id;
    u32 i, j;
    vtss_phy_ts_engine_action_t  *old_action_conf;
    vtss_phy_ts_eng_conf_t *alt_eng_conf = NULL;

    old_action_conf = &eng_conf->action_conf;
    VTSS_RC(vtss_phy_ts_ana_blk_id_get(eng_id, ingress, &blk_id));

    /* action type can not be change and must be synced with encap_type */
    if (old_action_conf->action_ptp != new_action_conf->action_ptp) {
        return VTSS_RC_ERROR;
    }
    /* Validate the action parameters
     */
    if (new_action_conf->action_ptp == FALSE) {
        for (i = 0; i < 6; i++) {
            if (new_action_conf->action.oam_conf[i].enable == FALSE) {
                continue;
            }
            /* Check for the version type of the Supported OAM Headers
             * Currently, 0 is supported; other than 0 return error
             */
            if (new_action_conf->action.oam_conf[i].version != 0) {
                return VTSS_RC_ERROR;
            }
            if ((new_action_conf->action.oam_conf[i].y1731_en == FALSE) &&
                (new_action_conf->action.oam_conf[i].oam_conf.ietf_oam_conf.ts_format !=
                 VTSS_PHY_TS_IETF_MPLS_ACH_OAM_TS_FORMAT_PTP)) {
                return VTSS_RC_ERROR;
            }
        }
    }

    if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        alt_eng_conf = (ingress ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B] :
                        &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B]);
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        alt_eng_conf = (ingress ? &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A] :
                        &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A]);
    }
    /* Disable the engine before changing the engine conf: for engine 2A or 2B
       if either of the engine is enable, disable the whole OAM engine */
    if (eng_conf->flow_conf.eng_mode ||
        (alt_eng_conf && alt_eng_conf->flow_conf.eng_mode)) {
        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, FALSE)));
    }

    if (old_action_conf->action_ptp) {
        /* PTP action set */
        /* Traverse each engine action and check if config change.
           If so, if the old config was action enable, delete the flow first
           corresponding to the old action and add new flow based on new config */
        for (i = 0; i < 2; i++) {
            if (memcmp(&old_action_conf->action.ptp_conf[i],
                       &new_action_conf->action.ptp_conf[i],
                       sizeof(vtss_phy_ts_ptp_engine_action_t))) {
                if (old_action_conf->action.ptp_conf[i].enable) {
                    /* Delete all the old flow related to this action */
                    for (j = 0; j < 6; j++) {
                        if (eng_conf->action_flow_map[j] == (i + 1)) {
                            VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_ptp_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, j)));
                            eng_conf->action_flow_map[j] = 0;
                        }
                    }
                }
                /* add action flow for new action */
                if (new_action_conf->action.ptp_conf[i].enable) {
                    VTSS_RC(vtss_phy_ts_engine_ptp_action_add_priv(vtss_state, ingress,
                                                                   port_no, base_port_no, blk_id, eng_id, eng_conf,
                                                                   &new_action_conf->action.ptp_conf[i], i));
                }
            }
        }

        /* save the new conf */
        memcpy(old_action_conf, new_action_conf, sizeof(vtss_phy_ts_engine_action_t));
    } else {
        /* OAM action set */
        /* Traverse each engine action and check if config change.
           If so, if the old config was action enable, delete the flow first
           corresponding to the old action and add new flow based on new config */
        for (i = 0; i < 6; i++) {
            if (memcmp(&old_action_conf->action.oam_conf[i],
                       &new_action_conf->action.oam_conf[i],
                       sizeof(vtss_phy_ts_oam_engine_action_t))) {
                if (old_action_conf->action.oam_conf[i].enable) {
                    /* Delete all the old flow related to this action */
                    for (j = 0; j < 6; j++) {
                        if (eng_conf->action_flow_map[j] == (i + 1)) {
                            VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_oam_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, j)));
                            eng_conf->action_flow_map[j] = 0;
                        }
                    }
                }
                /* add action flow for new action */
                if (new_action_conf->action.oam_conf[i].enable) {
                    if (new_action_conf->action.oam_conf[i].y1731_en) {
                        VTSS_RC(vtss_phy_ts_engine_y1731_oam_action_add_priv(vtss_state, ingress,
                                                                             port_no, base_port_no, blk_id, eng_id, eng_conf,
                                                                             &new_action_conf->action.oam_conf[i], i));
                    } else {
                        VTSS_RC(vtss_phy_ts_engine_ietf_oam_action_add_priv(vtss_state, ingress,
                                                                            port_no, base_port_no, blk_id, eng_conf,
                                                                            &new_action_conf->action.oam_conf[i], i));
                    }
                }
            }
        }
        /* save the new conf */
        memcpy(old_action_conf, new_action_conf, sizeof(vtss_phy_ts_engine_action_t));
    }

    /* enable the engine if it say so in conf: for engine 2A or 2B
       if other engine is enable and disable at the begging of config
       we need to enable the whole OAM engine */
    if (eng_conf->flow_conf.eng_mode ||
        (alt_eng_conf && alt_eng_conf->flow_conf.eng_mode)) {

        VTSS_RC(VTSS_RC_COLD(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, TRUE)));
    }

    return VTSS_RC_OK;
}


vtss_rc vtss_phy_ts_ingress_engine_action_set(const vtss_inst_t   inst,
                                              const vtss_port_no_t              port_no,
                                              const vtss_phy_ts_engine_t        eng_id,
                                              const vtss_phy_ts_engine_action_t *const action_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf, *alt_eng_conf;

    VTSS_PHY_TS_ASSERT(action_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
        alt_eng_conf = &vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
        rc = vtss_phy_ts_engine_action_set_priv(vtss_state, TRUE, port_no, base_port_no,
                                                eng_id, eng_conf, action_conf);
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
        /* save the action config */
        if (rc == VTSS_RC_OK) {
            memcpy(&eng_conf->action_conf, action_conf,
                   sizeof(vtss_phy_ts_engine_action_t));
            if (port_no != base_port_no) {
                memcpy(&alt_eng_conf->action_conf, action_conf,
                       sizeof(vtss_phy_ts_engine_action_t));
            }
        }
    } while (0);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_ingress_engine_action_get(const vtss_inst_t   inst,
                                              const vtss_port_no_t        port_no,
                                              const vtss_phy_ts_engine_t  eng_id,
                                              vtss_phy_ts_engine_action_t *const action_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf;

    VTSS_PHY_TS_ASSERT(action_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        memcpy(action_conf, &eng_conf->action_conf,
               sizeof(vtss_phy_ts_engine_action_t));
    } while (0);
    VTSS_EXIT();

    return rc;

}

vtss_rc vtss_phy_ts_egress_engine_action_set(const vtss_inst_t    inst,
                                             const vtss_port_no_t                 port_no,
                                             const vtss_phy_ts_engine_t           eng_id,
                                             const vtss_phy_ts_engine_action_t *const action_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf, *alt_eng_conf;

    VTSS_PHY_TS_ASSERT(action_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
        alt_eng_conf = &vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
        rc = vtss_phy_ts_engine_action_set_priv(vtss_state, FALSE, port_no, base_port_no,
                                                eng_id, eng_conf, action_conf);
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
        /* save the action config */
        if (rc == VTSS_RC_OK) {
            memcpy(&eng_conf->action_conf, action_conf,
                   sizeof(vtss_phy_ts_engine_action_t));
            if (port_no != base_port_no) {
                memcpy(&alt_eng_conf->action_conf, action_conf,
                       sizeof(vtss_phy_ts_engine_action_t));
            }
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_egress_engine_action_get(const vtss_inst_t    inst,
                                             const vtss_port_no_t            port_no,
                                             const vtss_phy_ts_engine_t      eng_id,
                                             vtss_phy_ts_engine_action_t *const action_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    vtss_port_no_t base_port_no;
    vtss_phy_ts_eng_conf_t *eng_conf;

    VTSS_PHY_TS_ASSERT(action_conf == NULL);
    if (!VTSS_PHY_TS_ENGINE_ID_VALID(eng_id)) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
        eng_conf = &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[eng_id];
        if (!eng_conf->eng_used) {
            rc = VTSS_RC_ERROR;
            break;
        }
        memcpy(action_conf, &eng_conf->action_conf,
               sizeof(vtss_phy_ts_engine_action_t));
    } while (0);
    VTSS_EXIT();

    return rc;
}
/* ================================================================= *
 *  End of Engine functions
 * ================================================================= */

vtss_rc vtss_phy_ts_event_enable_set(const vtss_inst_t         inst,
                                     const vtss_port_no_t      port_no,
                                     const BOOL                enable,
                                     const vtss_phy_ts_event_t ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    BOOL mask_changed = FALSE;
    VTSS_PHY_TS_ASSERT((ev_mask & ~VTSS_PHY_TS_EVENT_MASK));
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            if (enable) {
                mask_changed = (0 == (vtss_state->phy_ts_port_conf[port_no].event_mask & ev_mask));
                vtss_state->phy_ts_port_conf[port_no].event_mask |= ev_mask;
            } else {
                mask_changed = (0 != (vtss_state->phy_ts_port_conf[port_no].event_mask & ev_mask));
                vtss_state->phy_ts_port_conf[port_no].event_mask &= ~ev_mask;
            }
            /* optimization: only update the register if the mask is changed */
            //VTSS_E("port %d, enable %d, mask 0x%x, mask_changed %d", port_no, enable, ev_mask, mask_changed);
            if (mask_changed) {
                VTSS_PHY_TS_SPI_PAUSE(port_no);
                rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_EVT_MASK_SET);
                VTSS_PHY_TS_SPI_UNPAUSE(port_no);
            }
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_event_enable_get(const vtss_inst_t     inst,
                                     const vtss_port_no_t  port_no,
                                     vtss_phy_ts_event_t   *const ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    *ev_mask = 0;

    VTSS_PHY_TS_ASSERT(ev_mask == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *ev_mask = vtss_state->phy_ts_port_conf[port_no].event_mask;
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_event_poll(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_phy_ts_event_t   *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(status == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                *status = 0;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            rc = vtss_phy_ts_csr_event_poll_priv(vtss_state, port_no, status);
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_stats_get(const vtss_inst_t     inst,
                              const vtss_port_no_t  port_no,
                              vtss_phy_ts_stats_t   *const statistics)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(statistics == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            rc = vtss_phy_ts_stats_get_priv(vtss_state, port_no, statistics);
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_correction_overflow_get(
    const vtss_inst_t     inst,
    const vtss_port_no_t  port_no,
    BOOL                  *const ingr_overflow,
    BOOL                  *const egr_overflow)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(ingr_overflow == NULL);
    VTSS_PHY_TS_ASSERT(egr_overflow  == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            VTSS_PHY_TS_SPI_PAUSE(port_no);
            rc = vtss_phy_ts_correction_overflow_get_priv(vtss_state, port_no,
                                                          ingr_overflow, egr_overflow);
            VTSS_PHY_TS_SPI_UNPAUSE(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_mode_set(const vtss_inst_t     inst,
                             const vtss_port_no_t  port_no,
                             const BOOL            enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            vtss_state->phy_ts_port_conf[port_no].port_ena = enable;
            VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);
            rc = VTSS_RC_COLD(vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_ENA_SET));
            VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_ts_mode_get(const vtss_inst_t     inst,
                             const vtss_port_no_t  port_no,
                             BOOL                  *const enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_PHY_TS_ASSERT(enable == NULL);
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                rc = VTSS_RC_ERROR;
                break;
            }
            *enable = vtss_state->phy_ts_port_conf[port_no].port_ena;
        }
    } while (0);

    VTSS_EXIT();
    return rc;
}

static vtss_rc vtss_phy_ts_rev_get_priv(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no,
                                        u16               *const revision)
{
#ifdef VTSS_CHIP_10G_PHY
    vtss_phy_10g_id_t phy_id_10g;
#endif
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_type_t   phy_id_1g;
#endif
#ifdef VTSS_ARCH_DAYTONA
    vtss_chip_id_t  chip_id;
#endif

#ifdef VTSS_CHIP_10G_PHY
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get_priv(vtss_state, port_no, &phy_id_10g) == VTSS_RC_OK) {
        *revision = phy_id_10g.revision;
        return VTSS_RC_OK;
    }
#endif
#ifdef VTSS_CHIP_CU_PHY
    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
    if (vtss_phy_id_get_priv(vtss_state, port_no, &phy_id_1g) == VTSS_RC_OK) {
        *revision = phy_id_1g.revision;
        return VTSS_RC_OK;
    }
#endif
#ifdef VTSS_ARCH_DAYTONA
    if (vtss_daytona_chip_id_get_priv(vtss_state, &chip_id) == VTSS_RC_OK) {
        /* 1588 block is present in port 2 and 3 i.e. line side */
        if (port_no == 2 || port_no == 3) {
            *revision = chip_id.revision;
            return VTSS_RC_OK;
        }
    }
#endif

    VTSS_E("Invalid port (%d)", (u32)port_no);
    return VTSS_RC_ERROR;
}
#ifndef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
#ifdef VTSS_CHIP_CU_PHY
static vtss_rc vtss_phy_ts_phy_latency_set_priv(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u16 dly_egr;
    u32 dly_egr32;
    u16 reg18;
    u16 reg17;

    //Broadcasts to all PHYs in chip containing specified port_no
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, 0 ));
    VTSS_RC(vtss_phy_wr_masked(vtss_state, port_no, 22, 0x0001, 0x0001));

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, 0x2a30 ));
    VTSS_RC(vtss_phy_wr_masked(vtss_state, port_no,  8, 0x8000, 0x8000));

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, 0x52b5 ));

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x0E ));   //      bit 23-12          bit 11-0
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0xF164 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9f90 )); //Write speed1000_lat_igr, speed100_lat_igr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x06 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0xaB7A ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9f92 )); //Write speed10_lat_egr, speed10_lat_igr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x01 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x805B ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9f94 )); //Write speed1000_lat_egr, speed100_lat_egr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x11 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x4057 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9f96 )); //Write fi_100fx_lat_igr, fi_1000_lat_igr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x01 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x801E ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9f98 )); //Write fi_100fx_lat_egr, fi_1000_lat_egr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x32 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x0320 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fa8 )); //Write i1588_byp_speed10_lat_igr, i1588_byp_speed10_lat_egr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x08 ));   //      bit 23-16                    bit 15-8                     bit 7-0
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x0850 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fac )); //Write i1588_byp_speed1000_lat_igr, i1588_byp_speed1000_lat_egr, i1588_byp_speed100_lat_igr

    dly_egr = 262 + 64 * VTSS_PHY_TS_EGR_DF_DEPTH;
    reg18 = dly_egr >> 4;
    reg17 = ((dly_egr & 0xf) << 12) + 342;
    VTSS_N("Write i1588_speed1000_proc_dly_egr, i1588_speed1000_proc_dly_igr reg 18 = %x, reg 18 = %x", reg18, reg17);
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, reg18 ));   //      bit 23-12                     bit 11-0
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, reg17 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fae )); //Write i1588_speed1000_proc_dly_egr, i1588_speed1000_proc_dly_igr

    dly_egr = 2775 + 640 * VTSS_PHY_TS_EGR_DF_DEPTH;
    reg18 = 0x50 >> 2;
    reg17 = (dly_egr & 0x3fff) + ((0x50 & 0x3) << 14);
    VTSS_N("Write i1588_byp_speed100_lat_egr, i1588_speed100_proc_dly_egr reg 18 = %x, reg 18 = %x", reg18, reg17);
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, reg18 ));   //      bit 21-14                   bit 13-0
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, reg17 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fb0 )); //Write i1588_byp_speed100_lat_egr, i1588_speed100_proc_dly_egr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x00 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x0d68 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fb2 )); //Write i1588_speed100_proc_dly_igr

    dly_egr32 = 28634 + 6400 * VTSS_PHY_TS_EGR_DF_DEPTH;
    reg18 = (dly_egr32 >> 16) & 0x3;
    reg17 = dly_egr32 & 0xffff;
    VTSS_N("Write i1588_speed10_proc_dly_egr reg 18 = %x, reg 18 = %x", reg18, reg17);
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, reg18 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, reg17));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fb4 )); //Write i1588_speed10_proc_dly_egr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 18, 0x00 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 17, 0x8490 ));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 16, 0x9fb6 )); //Write i1588_speed10_proc_dly_igr

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, 0x2a30 ));
    VTSS_RC(vtss_phy_wr_masked(vtss_state, port_no,  8, 0x0000, 0x8000));

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, 0 ));
    VTSS_RC(vtss_phy_wr_masked(vtss_state, port_no, 22, 0x0000, 0x0001));

    return VTSS_RC_OK;
}
#endif /* VTSS_CHIP_CU_PHY */
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */


vtss_rc vtss_phy_ts_init(const vtss_inst_t               inst,
                         const vtss_port_no_t            port_no,
                         const vtss_phy_ts_init_conf_t  *const conf)
{
    vtss_rc                  rc = VTSS_RC_OK;
#ifndef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    vtss_state_t             *vtss_state;
    u16                      phy_type;
    BOOL                     clause45, phy_type_match = FALSE;
    vtss_phy_ts_oper_mode_t  oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_port_no_t           base_port_no;
    vtss_phy_ts_init_conf_t  base_conf;
    u16                      revision;

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    u16     channel_id;
#endif
#ifdef VTSS_ARCH_DAYTONA
    vtss_config_mode_t channel_mode;
#endif /* VTSS_ARCH_DAYTONA */

#if defined (VTSS_SW_OPTION_REMOTE_TS_PHY)
    if (conf->remote_phy) {
        VTSS_E("Remote phy not yet supported");
    }
#endif /* VTSS_SW_OPTION_REMOTE_TS_PHY */

    VTSS_N("Port: %u:: ts_init", (u32)port_no);
    VTSS_PHY_TS_ASSERT(conf == NULL);
    memset(&base_conf, 0, sizeof(vtss_phy_ts_init_conf_t));


    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
        if ((rc = vtss_phy_ts_channel_id_get_priv(vtss_state, port_no, &channel_id)) != VTSS_RC_OK) {
            break;
        }
#endif
        if ((rc = vtss_phy_ts_rev_get_priv(vtss_state, port_no, &revision)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode)) != VTSS_RC_OK) {
            VTSS_E("Error in getting access_type port %u", port_no);
            break;
        }
#if defined VTSS_CHIP_CU_PHY
        if ((phy_type == VTSS_PHY_TYPE_8574) ||
            (phy_type == VTSS_PHY_TYPE_8572) ||
            (phy_type == VTSS_PHY_TYPE_8575) ||
            (phy_type == VTSS_PHY_TYPE_8582) ||
            (phy_type == VTSS_PHY_TYPE_8584) ||
            (phy_type == VTSS_PHY_TYPE_8586)) {
            phy_type_match = TRUE;
        }
        if ((phy_type == VTSS_PHY_TYPE_8575) ||
            (phy_type == VTSS_PHY_TYPE_8582) ||
            (phy_type == VTSS_PHY_TYPE_8584) ||
            (phy_type == VTSS_PHY_TYPE_8586)) {
            vtss_state->phy_ts_port_conf[port_no].is_gen2 = TRUE;
        }
#endif /* VTSS_CHIP_CU_PHY */
#if defined (VTSS_CHIP_10G_PHY)
        if ((phy_type == VTSS_PHY_TYPE_8487) ||
            (phy_type == VTSS_PHY_TYPE_8488) ||
            (phy_type == VTSS_PHY_TYPE_8489) ||
            (phy_type == VTSS_PHY_TYPE_8490) ||
            (phy_type == VTSS_PHY_TYPE_8491)) {
            phy_type_match = TRUE;
        }
        if ((phy_type == VTSS_PHY_TYPE_8489) ||
            (phy_type == VTSS_PHY_TYPE_8490) ||
            (phy_type == VTSS_PHY_TYPE_8491)) {
            vtss_state->phy_ts_port_conf[port_no].is_gen2 = TRUE;
        }
#endif /* VTSS_CHIP_10G_PHY */
#ifdef VTSS_ARCH_DAYTONA
        if ((phy_type == VTSS_DEV_TYPE_8492) || (phy_type == VTSS_DEV_TYPE_8494)) {
            phy_type_match = TRUE;
            if ((rc = daytona_port_2_mode(vtss_state, port_no, &channel_mode)) != VTSS_RC_OK) {
                break;
            }
            switch (channel_mode) {
            case VTSS_CONFIG_MODE_PEE_W:
            case VTSS_CONFIG_MODE_PEE_MW:
            case VTSS_CONFIG_MODE_PEO_WA:
            case VTSS_CONFIG_MODE_PEO_MWA:
            case VTSS_CONFIG_MODE_PEO_WS:
            case VTSS_CONFIG_MODE_PEO_MWS:
            case VTSS_CONFIG_MODE_PEE_P:
            case VTSS_CONFIG_MODE_PEE_MP:
            case VTSS_CONFIG_MODE_PEO_P2E_20:
            case VTSS_CONFIG_MODE_PEO_MP2E_20:
            case VTSS_CONFIG_MODE_PEO_P2E_100:
            case VTSS_CONFIG_MODE_PEO_P1E_100:
            case VTSS_CONFIG_MODE_TEO_PMP_2E:
            case VTSS_CONFIG_MODE_TEO_PMP_1E:
                rc = VTSS_RC_OK;
                break;
            default:
                rc = VTSS_RC_ERROR;
                break;
            }
            if (rc != VTSS_RC_OK) {
                VTSS_E("Invalid Daytona Mode (%d) for ts_init", channel_mode);
                break;
            }
        }
#endif
#if defined VTSS_CHIP_CU_PHY || defined VTSS_CHIP_10G_PHY || defined VTSS_ARCH_DAYTONA
        if (phy_type_match == FALSE) {
            VTSS_E("Invalid PHY type %d", (u32)phy_type);
            rc = VTSS_RC_ERROR;
            break;
        }
#endif /* VTSS_CHIP_CU_PHY */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
        /* Micro-patch restriction: both the channel within the block must
           have same TX_FIFO access mode */
        if (phy_type == VTSS_PHY_TYPE_8574 || phy_type == VTSS_PHY_TYPE_8572) {
            if (port_no != base_port_no) {
                if ((vtss_state->phy_ts_port_conf[base_port_no].port_ts_init_done == TRUE) &&
                    (vtss_state->phy_ts_port_conf[base_port_no].tx_fifo_mode != conf->tx_fifo_mode)) {
                    VTSS_E("Invalid TS_FIFO access mode for 2 ports within the same 1588 Block %d", conf->tx_fifo_mode);
                    rc = VTSS_RC_ERROR;
                    break;
                }
            } else {
                if ((vtss_state->phy_ts_port_conf[port_no].new_spi_conf.alt_port_init_done == TRUE) &&
                    (vtss_state->phy_ts_port_conf[port_no].new_spi_conf.alt_port_tx_fifo_mode != conf->tx_fifo_mode)) {
                    VTSS_E("Invalid TS_FIFO access mode for 2 ports within the same 1588 Block %d", conf->tx_fifo_mode);
                    rc = VTSS_RC_ERROR;
                    break;
                }
            }
        }
        vtss_state->phy_ts_port_conf[port_no].new_spi_conf.phy_type = phy_type;
        vtss_state->phy_ts_port_conf[port_no].new_spi_conf.channel_id = channel_id;
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */
        base_conf.clk_freq  = vtss_state->phy_ts_port_conf[base_port_no].clk_freq;
        base_conf.clk_src   = vtss_state->phy_ts_port_conf[base_port_no].clk_src;
        base_conf.rx_ts_pos = vtss_state->phy_ts_port_conf[base_port_no].rx_ts_pos;
        base_conf.rx_ts_len = vtss_state->phy_ts_port_conf[base_port_no].rx_ts_len;
        base_conf.tx_fifo_mode =
            vtss_state->phy_ts_port_conf[base_port_no].tx_fifo_mode;
        base_conf.tx_ts_len = vtss_state->phy_ts_port_conf[base_port_no].tx_ts_len;
        base_conf.tc_op_mode = vtss_state->phy_ts_port_conf[base_port_no].tc_op_mode;
        base_conf.auto_clear_ls = vtss_state->phy_ts_port_conf[base_port_no].auto_clear_ls;
        base_conf.chk_ing_modified = vtss_state->phy_ts_port_conf[base_port_no].chk_ing_modified;
#ifdef VTSS_CHIP_10G_PHY
        base_conf.xaui_sel_8487 = vtss_state->phy_ts_port_conf[base_port_no].xaui_sel_8487;
#endif
        base_conf.macsec_ena = vtss_state->phy_ts_port_conf[base_port_no].macsec_ena;

        if ((port_no != base_port_no) &&
            (vtss_state->phy_ts_port_conf[base_port_no].port_ts_init_done != FALSE)) {
            if (memcmp(&base_conf, conf, sizeof(vtss_phy_ts_init_conf_t)) != 0) {
                rc = VTSS_RC_ERROR;
                VTSS_E("conf not compatible with base port's, port_no %u, base_port %u", port_no, base_port_no);
                break;
            }
        }
        if (((phy_type == VTSS_PHY_TYPE_8574) ||
             (phy_type == VTSS_PHY_TYPE_8572) ||
             (phy_type == VTSS_PHY_TYPE_8575) ||
             (phy_type == VTSS_PHY_TYPE_8582) ||
             (phy_type == VTSS_PHY_TYPE_8584) ||
             (phy_type == VTSS_PHY_TYPE_8586)) &&
            (conf->clk_freq == VTSS_PHY_TS_CLOCK_FREQ_500M)) {
            rc = VTSS_RC_ERROR;
            VTSS_E("Clock frequency not supported for 1G, port_no %u", port_no);
            break;
        }

        vtss_state->phy_ts_port_conf[port_no].port_ts_init_done = TRUE;
        vtss_state->phy_ts_port_conf[port_no].clk_freq      = conf->clk_freq;
        vtss_state->phy_ts_port_conf[port_no].clk_src       = conf->clk_src;
        vtss_state->phy_ts_port_conf[port_no].rx_ts_pos     = conf->rx_ts_pos;
        vtss_state->phy_ts_port_conf[port_no].rx_ts_len     = conf->rx_ts_len;
        vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode  = conf->tx_fifo_mode;
        vtss_state->phy_ts_port_conf[port_no].tx_ts_len     = conf->tx_ts_len;
        vtss_state->phy_ts_port_conf[port_no].chk_ing_modified  = conf->chk_ing_modified;
        if (vtss_state->phy_ts_port_conf[port_no].is_gen2 == TRUE) {
            vtss_state->phy_ts_port_conf[port_no].auto_clear_ls = conf->auto_clear_ls;
        }
        vtss_state->phy_ts_port_conf[port_no].macsec_ena = conf->macsec_ena;
        switch (conf->tc_op_mode) {
        case VTSS_PHY_TS_TC_OP_MODE_A:
            vtss_state->phy_ts_port_conf[port_no].tc_op_mode = VTSS_PHY_TS_TC_OP_MODE_A;
            break;
        case VTSS_PHY_TS_TC_OP_MODE_B:
            vtss_state->phy_ts_port_conf[port_no].tc_op_mode = VTSS_PHY_TS_TC_OP_MODE_B;
            break;
        default:
            /* Keep backward compatibility in case user doesn't pass this parameter */
            vtss_state->phy_ts_port_conf[port_no].tc_op_mode = VTSS_PHY_TS_TC_OP_MODE_B;
            break;
        }

#ifdef VTSS_CHIP_10G_PHY
        vtss_state->phy_ts_port_conf[port_no].xaui_sel_8487 = conf->xaui_sel_8487;
#endif
        vtss_state->phy_ts_port_conf[port_no].base_port     = base_port_no;
        vtss_state->phy_ts_port_conf[port_no].ingress_latency = 0;
        vtss_state->phy_ts_port_conf[port_no].egress_latency  = 0;

        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);

        /* Initialize the 1588 block */
        if ((rc = VTSS_RC_COLD(vtss_phy_ts_block_init(vtss_state, port_no))) != VTSS_RC_OK) {
            VTSS_E("block init failed, port_no %u, base_port %u", port_no, base_port_no);
            break;
        }

        if ((rc = VTSS_RC_COLD(vtss_phy_ts_port_init(vtss_state, port_no, conf))) != VTSS_RC_OK) {
            VTSS_E("ts port init failed, port_no %u", port_no);
            break;
        }

#ifdef VTSS_CHIP_CU_PHY
        if ((phy_type == VTSS_PHY_TYPE_8574) ||
            (phy_type == VTSS_PHY_TYPE_8572) ||
            (phy_type == VTSS_PHY_TYPE_8575) ||
            (phy_type == VTSS_PHY_TYPE_8582) ||
            (phy_type == VTSS_PHY_TYPE_8584) ||
            (phy_type == VTSS_PHY_TYPE_8586)) {
            /* initialize all the 1588 engines: this should be done only once
               i.e. engine initialization should be done through base port or
               alternate port (for 2-channel PHY); otherwise engine config
               for one port might be lost by another port initialization.
               This is reqd for Tesla RevA, RevB fixed in HW.
             */
            if (revision == VTSS_PHY_TESLA_REV_A) {
                if (vtss_state->phy_ts_port_conf[base_port_no].eng_init_done == FALSE) {
                    VTSS_D("1588 Analyzer init, port_no %u", port_no);
                    if ((rc = VTSS_RC_COLD(vtss_phy_ts_analyzer_init_priv(vtss_state, base_port_no))) != VTSS_RC_OK) {
                        VTSS_E("1588 Analyzer init failed, port_no %u", port_no);
                        break;
                    }
                    vtss_state->phy_ts_port_conf[base_port_no].eng_init_done = TRUE;
                }
                vtss_state->phy_ts_port_conf[port_no].eng_init_done = TRUE;
            }

            /* Set the PHY latency for Tesla, for 10G it is not reqd */
            if ((rc = vtss_phy_ts_phy_latency_set_priv(vtss_state, port_no)) != VTSS_RC_OK) {
                VTSS_E("1588 PHY Latency config fail!, port_no %u", port_no);
                break;
            }
        }
#endif /* VTSS_CHIP_CU_PHY */
#ifdef VTSS_ARCH_DAYTONA
        if ((phy_type == VTSS_DEV_TYPE_8492) || (phy_type == VTSS_DEV_TYPE_8494)) {
            /* initialize all 1588 engine registers: this should be done only once
               i.e. engine initialization should be done through channel 0 or
               channel 1 port; otherwise engine config for one port might be
               lost by another port initialization. This is reqd for Daytona.
             */
            if (vtss_state->phy_ts_port_conf[base_port_no].eng_init_done == FALSE) {
                VTSS_D("1588 Analyzer init, port_no %u", port_no);
                if ((rc = VTSS_RC_COLD(vtss_phy_ts_analyzer_init_priv(vtss_state, base_port_no))) != VTSS_RC_OK) {
                    VTSS_E("1588 Analyzer init failed, port_no %u", port_no);
                    break;
                }
                vtss_state->phy_ts_port_conf[base_port_no].eng_init_done = TRUE;
            }
            vtss_state->phy_ts_port_conf[port_no].eng_init_done = TRUE;
        }
#endif /* VTSS_ARCH_DAYTONA */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
        if (port_no != base_port_no) {
            vtss_state->phy_ts_port_conf[base_port_no].new_spi_conf.alt_port_init_done = TRUE;
            vtss_state->phy_ts_port_conf[base_port_no].new_spi_conf.alt_port_tx_fifo_mode = conf->tx_fifo_mode;
        }
#endif
    } while (0);

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    if (vtss_state->phy_ts_state.is_spi_paused == TRUE) {
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
    }
#endif

    if (rc != VTSS_RC_OK) {
        vtss_state->phy_ts_port_conf[port_no].port_ts_init_done = FALSE;
        VTSS_D("Port: %u:: ts_init failed!", (u32)port_no);
    }
    VTSS_EXIT();
#endif
    return rc;
}

vtss_rc vtss_phy_ts_init_conf_get(const vtss_inst_t     inst,
                                  const vtss_port_no_t            port_no,
                                  BOOL  *const                    port_ts_init_done,
                                  vtss_phy_ts_init_conf_t         *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    memset(conf, 0, sizeof(vtss_phy_ts_init_conf_t));
    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            *port_ts_init_done = vtss_state->phy_ts_port_conf[port_no].port_ts_init_done;
            conf->clk_freq  = vtss_state->phy_ts_port_conf[port_no].clk_freq;
            conf->clk_src   = vtss_state->phy_ts_port_conf[port_no].clk_src;
            conf->rx_ts_pos = vtss_state->phy_ts_port_conf[port_no].rx_ts_pos;
            conf->rx_ts_len = vtss_state->phy_ts_port_conf[port_no].rx_ts_len;
            conf->tx_fifo_mode = vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode;
            conf->tx_ts_len = vtss_state->phy_ts_port_conf[port_no].tx_ts_len;
            conf->tc_op_mode = vtss_state->phy_ts_port_conf[port_no].tc_op_mode;
#ifdef VTSS_CHIP_10G_PHY
            conf->xaui_sel_8487 = vtss_state->phy_ts_port_conf[port_no].xaui_sel_8487;
#endif
#if defined (VTSS_SW_OPTION_REMOTE_TS_PHY)
            conf->remote_phy =  FALSE; //vtss_state->phy_ts_port_conf[port_no].remote_phy;
#endif
        } else {
            *port_ts_init_done = FALSE;
            VTSS_E("Port: %u:: ts_init_conf_get failed!", (u32)port_no);
            rc = VTSS_RC_ERROR;
        }
    } while (0);
    VTSS_EXIT();
    return rc;
}

#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
vtss_rc vtss_phy_dce_init(const vtss_inst_t inst,
                          const vtss_port_no_t port_no)
{
    vtss_state_t             *vtss_state;
    vtss_rc                  rc = VTSS_RC_OK;
    u16                      phy_type;
    BOOL                     clause45, phy_type_match = FALSE;
    vtss_phy_ts_oper_mode_t  oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_port_no_t           base_port_no;
    u16                      revision;

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    u16     channel_id;
#endif

#if defined (VTSS_SW_OPTION_REMOTE_TS_PHY)
    if (conf->remote_phy) {
        VTSS_E("Remote phy not yet supported");
    }
#endif /* VTSS_SW_OPTION_REMOTE_TS_PHY */

    VTSS_N("Port: %u:: dce_init", (u32)port_no);

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_base_port_get_priv(vtss_state, port_no, &base_port_no)) != VTSS_RC_OK) {
            break;
        }
#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
        if ((rc = vtss_phy_ts_channel_id_get_priv(vtss_state, port_no, &channel_id)) != VTSS_RC_OK) {
            break;
        }
#endif
        if ((rc = vtss_phy_ts_rev_get_priv(vtss_state, port_no, &revision)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode)) != VTSS_RC_OK) {
            VTSS_E("Error in getting access_type port %u", port_no);
            break;
        }
#if defined VTSS_CHIP_CU_PHY
        if (phy_type == VTSS_PHY_TYPE_8574 || phy_type == VTSS_PHY_TYPE_8572 ) {
            phy_type_match = TRUE;
        }
#endif /* VTSS_CHIP_CU_PHY */

#if defined VTSS_CHIP_CU_PHY || defined VTSS_CHIP_10G_PHY || defined VTSS_ARCH_DAYTONA
        if (phy_type_match == FALSE) {
            VTSS_E("Invalid PHY type %d", (u32)phy_type);
            rc = VTSS_RC_ERROR;
            break;
        }
#endif /* VTSS_CHIP_CU_PHY */

        vtss_state->phy_ts_port_conf[port_no].base_port     = base_port_no;
        vtss_state->phy_ts_port_conf[port_no].port_ts_init_done = TRUE;

        VTSS_PHY_TS_SPI_PAUSE_COLD(port_no);

        /* Initialize the 1588 block */
        if ((rc = VTSS_RC_COLD(vtss_phy_ts_block_init(vtss_state, port_no))) != VTSS_RC_OK) {
            VTSS_E("block init failed, port_no %u, base_port %u", port_no, base_port_no);
            break;
        }
        if ((rc = VTSS_RC_COLD(vtss_phy_dce_port_init(vtss_state, port_no))) != VTSS_RC_OK) {
            VTSS_E("ts port init failed, port_no %u", port_no);
            break;
        }
#ifdef VTSS_CHIP_CU_PHY
        if (phy_type == VTSS_PHY_TYPE_8574 || phy_type == VTSS_PHY_TYPE_8572) {
            /* initialize all the 1588 engines: this should be done only once
               i.e. engine initialization should be done through base port or
               alternate port (for 2-channel PHY); otherwise engine config
               for one port might be lost by another port initialization.
               This is reqd for Tesla RevA, RevB fixed in HW.
             */
            if (revision == VTSS_PHY_TESLA_REV_A) {
                if (vtss_state->phy_ts_port_conf[base_port_no].eng_init_done == FALSE) {
                    VTSS_D("1588 Analyzer init, port_no %u", port_no);
                    if ((rc = VTSS_RC_COLD(vtss_phy_ts_analyzer_init_priv(vtss_state, base_port_no))) != VTSS_RC_OK) {
                        VTSS_E("1588 Analyzer init failed, port_no %u", port_no);
                        break;
                    }
                    vtss_state->phy_ts_port_conf[base_port_no].eng_init_done = TRUE;
                }
                vtss_state->phy_ts_port_conf[port_no].eng_init_done = TRUE;
            }
        }
#endif /* VTSS_CHIP_CU_PHY */

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
        if (port_no != base_port_no) {
            vtss_state->phy_ts_port_conf[base_port_no].new_spi_conf.alt_port_init_done = TRUE;
        }
    } while (0);
#endif

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    if (vtss_state->phy_ts_state.is_spi_paused == TRUE) {
        VTSS_PHY_TS_SPI_UNPAUSE_COLD(port_no);
    }
#endif

    if (rc != VTSS_RC_OK) {
        vtss_state->phy_ts_port_conf[port_no].port_ts_init_done = FALSE;
        VTSS_D("Port: %u:: ts_init failed!", (u32)port_no);
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */



#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)

#define VTSS_PHY_TS_CONFIG_BASE          0x804D
#define VTSS_PHY_TS_CONFIG_RUN           0x8F4D
#define VTSS_PHY_TS_PAUSE_BLK_A          0x8A4D
#define VTSS_PHY_TS_PAUSE_BLK_B          0x854D
#define VTSS_PHY_TS_PAUSE_BLK_A_AND_B    0x804D
#define VTSS_PHY_TS_FIFO_CONF_STATUS     0x805D
#define VTSS_PHY_TS_CHANNEL_TO_BLK_ID(ch) (((ch) == 0 || (ch) == 2) ? VTSS_PHY_TS_BLOCK_A : VTSS_PHY_TS_BLOCK_B)

/* Defines the 8574 block ID */
typedef enum {
    VTSS_PHY_TS_BLOCK_A,  /**< Block A, includes channel 0 and 2 */
    VTSS_PHY_TS_BLOCK_B,  /**< Block B, includes channel 1 and 3 */
} vtss_phy_ts_block_t;

static vtss_rc vtss_phy_ts_channel_id_get_priv(vtss_state_t *vtss_state,
                                               const vtss_port_no_t  port_no,
                                               u16                   *const channel_id)
{
#ifdef VTSS_CHIP_10G_PHY
    vtss_phy_10g_id_t phy_id_10g;
#endif
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_type_t   phy_id_1g;
#endif
#ifdef VTSS_ARCH_DAYTONA
    vtss_chip_id_t  chip_id;
#endif

#ifdef VTSS_CHIP_10G_PHY
    memset(&phy_id_10g, 0, sizeof(vtss_phy_10g_id_t));
    if (vtss_phy_10g_id_get_priv(vtss_state, port_no, &phy_id_10g) == VTSS_RC_OK) {
        *channel_id = phy_id_10g.channel_id;
        return VTSS_RC_OK;
    }
#endif
#ifdef VTSS_CHIP_CU_PHY
    memset(&phy_id_1g, 0, sizeof(vtss_phy_type_t));
    if (vtss_phy_id_get_priv(vtss_state, port_no, &phy_id_1g) == VTSS_RC_OK) {
        *channel_id = phy_id_1g.channel_id;
        return VTSS_RC_OK;
    }
#endif
#ifdef VTSS_ARCH_DAYTONA
    if (vtss_daytona_chip_id_get_priv(vtss_state, &chip_id) == VTSS_RC_OK) {
        /* 1588 block is present in port 2 and 3 i.e. line side */
        if (port_no == 2 || port_no == 3) {
            *channel_id = ((port_no == 2) ? 0 : 1);
            return VTSS_RC_OK;
        }
    }
#endif

    VTSS_E("Invalid port (%d)", (u32)port_no);
    return VTSS_RC_ERROR;
}

static vtss_rc vtss_phy_ts_spi_pause_priv(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_rc      rc = VTSS_RC_OK;
    u16          val_18g;
    vtss_phy_ts_block_t blk_id;

    vtss_state->phy_ts_state.is_spi_paused = FALSE;
    vtss_state->phy_ts_state.spi_prev_status = 0;

    blk_id = VTSS_PHY_TS_CHANNEL_TO_BLK_ID(vtss_state->phy_ts_port_conf[port_no].new_spi_conf.channel_id);
    /* Change to GPIO page */
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    do {
        /* Set register 18G = 0x805d to fetch current SPI patch enabled status */
        if ((rc = PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, VTSS_PHY_TS_FIFO_CONF_STATUS)) != VTSS_RC_OK) {
            break;
        }
        /* Poll on 18G.15 to clear */
        if ((rc = vtss_phy_wait_for_micro_complete(vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        /* Above function moves back to std page, change to GPIO page */
        if ((rc = vtss_phy_page_gpio(vtss_state, port_no)) != VTSS_RC_OK) {
            break;
        }
        /* Now it can be read */
        if ((rc = PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &val_18g)) != VTSS_RC_OK) {
            break;
        }
        vtss_state->phy_ts_state.spi_prev_status = val_18g;

        /* Pause only when either of the ports in the block uses SPI */
        if ((blk_id == VTSS_PHY_TS_BLOCK_A) && (val_18g & 0x500)) {
            vtss_state->phy_ts_state.is_spi_paused = TRUE;
            /* pause both ports in block A */
            if ((rc = PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, (val_18g & 0xA00) | VTSS_PHY_TS_CONFIG_BASE)) != VTSS_RC_OK) {
                break;
            }
            /* Poll on 18G.15 to clear */
            if ((rc = vtss_phy_wait_for_micro_complete(vtss_state, port_no)) != VTSS_RC_OK) {
                break;
            }
        } else if ((blk_id == VTSS_PHY_TS_BLOCK_B) && (val_18g & 0xA00)) {
            vtss_state->phy_ts_state.is_spi_paused = TRUE;
            /* pause both ports in block B */
            if ((rc = PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, (val_18g & 0x500) | VTSS_PHY_TS_CONFIG_BASE)) != VTSS_RC_OK) {
                break;
            }
            /* Poll on 18G.15 to clear */
            if ((rc = vtss_phy_wait_for_micro_complete(vtss_state, port_no)) != VTSS_RC_OK) {
                break;
            }
        }
    } while (0);

    /* return to standard page */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}

static vtss_rc vtss_phy_ts_spi_unpause_priv(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    vtss_rc             rc = VTSS_RC_OK;

    if (vtss_state->phy_ts_state.is_spi_paused == FALSE) {
        return rc;
    }
    /* Change to GPIO page */
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    do {
        /* resume any ports that were previously enabled in the block that was paused */
        if ((rc = (PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, (vtss_state->phy_ts_state.spi_prev_status & 0xF00) | VTSS_PHY_TS_CONFIG_BASE))) != VTSS_RC_OK) {
            break;
        }
        /* Poll on 18G.15 to clear */
        if ((rc = (vtss_phy_wait_for_micro_complete(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
    } while (0);

    /* return to standard page */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}

static vtss_rc vtss_phy_ts_new_spi_mode_set_priv(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t    port_no,
                                                 const BOOL              enable)
{
    vtss_rc  rc = VTSS_RC_OK;
    u16      val_18g;
    u32      value = 0;
    u16      channel_id = vtss_state->phy_ts_port_conf[port_no].new_spi_conf.channel_id;

    /* Pause before 1588 reg access */
    if ((rc = vtss_phy_ts_spi_pause_priv(vtss_state, port_no)) != VTSS_RC_OK) {
        VTSS_E("SPI pause fail!, port %u", port_no);
        return rc;
    }

    /* Change the standard register for New SPI mode to activate SPI */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));
    if (enable) {
        /* If new SPI mode is enabled, we must clear bit to disable old SPI mode */
        value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA);
    } else {
        /* else it should switch to old SPI, since this function is not called
           for tx_fifo_mode MDIO */
        value |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA;
    }
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));

    /* 1588 reg access done: Un-pause now */
    if ((rc = vtss_phy_ts_spi_unpause_priv(vtss_state, port_no)) != VTSS_RC_OK) {
        VTSS_E("SPI Un-pause fail!, port %u", port_no);
        return rc;
    }

    /* Change to GPIO page */
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    do {
        /* Set register 18G = 0x805d to fetch current SPI patch enabled status */
        if ((rc = (PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, VTSS_PHY_TS_FIFO_CONF_STATUS))) != VTSS_RC_OK) {
            break;
        }
        /* Poll on 18G.15 to clear */
        if ((rc = (vtss_phy_wait_for_micro_complete(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
        /* micro_complete above moves back to std page, change to GPIO page */
        if ((rc = (vtss_phy_page_gpio(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
        /* Now it can be read */
        if ((rc = (PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &val_18g))) != VTSS_RC_OK) {
            break;
        }
        if (enable) {
            /* bits 11:8 keeps the SPI status */
            val_18g = (val_18g & 0xF00) | (1 << (8 + channel_id));  // set the correct bit (channel_id is in range 0-3 for Tesla)
        } else {
            /* bits 11:8 keeps the SPI status */
            val_18g = (val_18g & 0xF00) & (~((u16)(1 << (8 + channel_id))));  // mask off the channel
        }

        if ((rc = (PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, VTSS_PHY_TS_CONFIG_BASE | val_18g))) != VTSS_RC_OK) {
            break;
        }
        /* Poll on 18G.15 to clear */
        if ((rc = (vtss_phy_wait_for_micro_complete(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
    } while (0);

    /* return to standard page */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}

static vtss_rc vtss_phy_ts_new_spi_mode_sync_priv(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t    port_no,
                                                  const BOOL              enable)
{
    vtss_rc  rc = VTSS_RC_OK;
    u16      val_18g;
    u32      value = 0;
    u16      channel_id = vtss_state->phy_ts_port_conf[port_no].new_spi_conf.channel_id;
    BOOL     sync_req = FALSE;

    /* Pause before 1588 reg access */
    if ((rc = vtss_phy_ts_spi_pause_priv(vtss_state, port_no)) != VTSS_RC_OK) {
        VTSS_E("SPI pause fail!, port %u", port_no);
        return rc;
    }

    /* Change the standard register for New SPI mode to activate SPI */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));
    if (enable) {
        /* If new SPI mode is enabled, TS_FIFO_SI_ENA bit must be clear in HW */
        if (value & VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA) {
            sync_req = TRUE;
            value = VTSS_PHY_TS_CLR_BITS(value, VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA);
        }
    } else {
        /* else it should switch to old SPI, since this function is not called
           for tx_fifo_mode MDIO, TS_FIFO_SI_ENA bit must be set for old SPI mode */
        if (!(value & VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA)) {
            sync_req = TRUE;
            value |= VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA;
        }
    }
    if (sync_req) {
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));
    }

    /* 1588 reg access done: Un-pause now */
    if ((rc = vtss_phy_ts_spi_unpause_priv(vtss_state, port_no)) != VTSS_RC_OK) {
        VTSS_E("SPI Un-pause fail!, port %u", port_no);
        return rc;
    }

    /* Change to GPIO page */
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    sync_req = FALSE;
    do {
        /* Set register 18G = 0x805d to fetch current SPI patch enabled status */
        if ((rc = (PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, VTSS_PHY_TS_FIFO_CONF_STATUS))) != VTSS_RC_OK) {
            break;
        }
        /* Poll on 18G.15 to clear */
        if ((rc = (vtss_phy_wait_for_micro_complete(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
        /* micro_complete above moves back to std page, change to GPIO page */
        if ((rc = (vtss_phy_page_gpio(vtss_state, port_no))) != VTSS_RC_OK) {
            break;
        }
        /* Now it can be read */
        if ((rc = (PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &val_18g))) != VTSS_RC_OK) {
            break;
        }
        if (enable) {
            /* bits 11:8 keeps the SPI status */
            if (!((val_18g & 0xF00) & (1 << (8 + channel_id)))) {
                sync_req = TRUE;
                val_18g = (val_18g & 0xF00) | (1 << (8 + channel_id));  // set the correct bit (channel_id is in range 0-3 for Tesla)
            }
        } else {
            if (((val_18g & 0xF00) & (1 << (8 + channel_id)))) {
                sync_req = TRUE;
                val_18g = (val_18g & 0xF00) & (~((u16)(1 << (8 + channel_id))));  // mask off the channel
            }
        }

        if (sync_req) {
            if ((rc = (PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, VTSS_PHY_TS_CONFIG_BASE | val_18g))) != VTSS_RC_OK) {
                break;
            }
            /* Poll on 18G.15 to clear */
            if ((rc = (vtss_phy_wait_for_micro_complete(vtss_state, port_no))) != VTSS_RC_OK) {
                break;
            }
        }
    } while (0);

    /* return to standard page */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}


vtss_rc vtss_phy_ts_new_spi_mode_set(const vtss_inst_t          inst,
                                     const vtss_port_no_t       port_no,
                                     const BOOL                 enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_N("Port: %u:: spi_mode_set", (u32)port_no);

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
            VTSS_E("Port ts not initialized! port %u", port_no);
            rc = VTSS_RC_ERROR;
        } else if ((vtss_state->phy_ts_port_conf[port_no].new_spi_conf.phy_type != VTSS_PHY_TYPE_8574) && (vtss_state->phy_ts_port_conf[port_no].new_spi_conf.phy_type != VTSS_PHY_TYPE_8572)) {
            /* This requires only for Tesla (RevA and RevB) that SPI should be
               activated after all the config is done to use PPS0 pin as SPI_CLK.
               For 10G, we don't allow this explicit activating the SPI */
            rc = VTSS_RC_OK;
        } else if (vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode != VTSS_PHY_TS_FIFO_MODE_SPI) {
            VTSS_E("Invalid TS FIFO Mode in ts_init");
            rc = VTSS_RC_ERROR;
        } else if ((rc = VTSS_RC_COLD(vtss_phy_ts_new_spi_mode_set_priv(vtss_state, port_no, enable))) != VTSS_RC_OK) {
            VTSS_E("New SPI Mode set failed! port %u", port_no);
        } else {
            vtss_state->phy_ts_port_conf[port_no].new_spi_conf.enable = enable;
        }
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_phy_ts_new_spi_mode_get(const vtss_inst_t       inst,
                                     const vtss_port_no_t    port_no,
                                     BOOL                    *const new_spi_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    *new_spi_mode = FALSE;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
            VTSS_E("Port ts not initialized! port %u", port_no);
            rc = VTSS_RC_ERROR;
        } else {
            *new_spi_mode = vtss_state->phy_ts_port_conf[port_no].new_spi_conf.enable;
        }
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_CHIP_CU_PHY && VTSS_PHY_TS_SPI_CLK_THRU_PPS0 */

#ifdef VTSS_CHIP_10G_PHY
//TODO: This should be delayed in WARM start until conf_end
vtss_rc vtss_phy_ts_phy_oper_mode_change(const vtss_inst_t          inst,
                                         const vtss_port_no_t       port_no)
{
    vtss_state_t            *vtss_state;
    vtss_rc                 rc;
    u16                     phy_type = VTSS_PHY_TYPE_NONE;
    BOOL                    clause45 = FALSE;
    vtss_phy_ts_oper_mode_t oper_mode = VTSS_PHY_TS_OPER_MODE_INV;


    VTSS_N("Port: %u:: oper_mode_change", (u32)port_no);

    VTSS_ENTER();
    do {
        if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
            if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == FALSE) {
                VTSS_E("Init not done, port %u", port_no);
                rc = VTSS_RC_OK;
                break;
            }

            if (vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode) != VTSS_RC_OK) {
                VTSS_E("Phy access type get failed %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            /* This mode change is only possible for 8487-15 and 8488-15. */
            if ((phy_type != VTSS_PHY_TYPE_8487) &&
                (phy_type != VTSS_PHY_TYPE_8488) &&
                (phy_type != VTSS_PHY_TYPE_8489) &&
                (phy_type != VTSS_PHY_TYPE_8490) &&
                (phy_type != VTSS_PHY_TYPE_8491)) {
                rc = VTSS_RC_OK;
                VTSS_D(" Not supported on this PHY");
                break;
            }

            /* Update the ingress_latency and egress_latency. */
            if (VTSS_RC_COLD(vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_ING_LATENCY_SET)) != VTSS_RC_OK) {
                VTSS_E("Ingress Latency set fail, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (VTSS_RC_COLD(vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_EGR_LATENCY_SET)) != VTSS_RC_OK) {
                VTSS_E("Egress Latency set fail, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
            if (VTSS_RC_COLD(vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_OPER_MODE_CHANGE_SET)) != VTSS_RC_OK) {
                VTSS_E("PHY operational mode change set failed, port %u", port_no);
                rc = VTSS_RC_ERROR;
                break;
            }
        }
    } while (0);
    VTSS_EXIT();

    return rc;
}
#endif /* VTSS_CHIP_10G_PHY */

#if defined(VTSS_ARCH_DAYTONA) && defined(VTSS_FEATURE_OTN)
static vtss_rc vtss_phy_ts_fec_mode_set_priv(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no,
                                             const vtss_otn_och_fec_t   *const cfg)
{
    u32  value = 0, ingr_latency = 0, egr_latency = 0;
    vtss_config_mode_t  channel_mode;

    VTSS_RC(daytona_port_2_mode(vtss_state, port_no, &channel_mode));

    switch (cfg->type) {
    case VTSS_OTN_OCH_FEC_NONE:
    case VTSS_OTN_OCH_FEC_RS:
        ingr_latency = latency_ingr_daytona[channel_mode];
        egr_latency  = latency_egr_daytona[channel_mode];
        break;
    case VTSS_OTN_OCH_FEC_I4:
        ingr_latency = latency_ingr_daytona_i4fec[channel_mode];
        egr_latency  = latency_egr_daytona_i4fec[channel_mode];
        break;
    case VTSS_OTN_OCH_FEC_I7:
        ingr_latency = latency_ingr_daytona_i7fec[channel_mode];
        egr_latency = latency_egr_daytona_i7fec[channel_mode];
        break;
    default:
        VTSS_E("Latency values for %d type FEC not supported", cfg->type);
        break;
    }

    /* Load the default ingress latency in the ingress local latency register
     */
    value = ingr_latency;
    value = VTSS_F_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY_INGR_LOCAL_LATENCY(value);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_TSP_INGR_LOCAL_LATENCY, &value));
    /* Load the Values in the ingress time stamp block
     */

    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    value |= VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_LOAD_DELAYS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));

    /* Load the default egress latency in the egress local latency register
     */
    value = egr_latency;
    value = VTSS_F_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY_EGR_LOCAL_LATENCY(value);
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSP_EGR_LOCAL_LATENCY, &value));
    /* Load the Values in the egress time stamp block
     */
    value = 0;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));
    value |= VTSS_F_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL_EGR_LOAD_DELAYS;
    VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                  VTSS_PTP_EGR_IP_1588_TSP_EGR_TSP_CTRL, &value));

    return VTSS_RC_OK;
}

vtss_rc vtss_phy_ts_fec_mode_set(const vtss_inst_t          inst,
                                 const vtss_port_no_t       port_no,
                                 const vtss_otn_och_fec_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && cfg) {
        rc = VTSS_RC_ERROR;
        if ((u16)cfg->type < (u16)VTSS_OTN_OCH_FEC_MAX) {
            rc = VTSS_RC_COLD(vtss_phy_ts_fec_mode_set_priv(vtss_state, port_no, cfg));
        }
    }
    VTSS_EXIT();
    return rc;
}

#endif /* VTSS_ARCH_DAYTONA && VTSS_FEATURE_OTN */

#ifdef VTSS_FEATURE_WARM_START

#define VTSS_PHY_TS_MAX_ACTION_PTP 2
#define VTSS_PHY_TS_MAX_ACTION_OAM 6

static vtss_rc vtss_phy_ts_block_sync(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    BOOL *block_sync)
{
    u16                        mmd_addr = (u16)0x1E;
    u16                        reg_value_1 = 0;
    u16                        reg_value_2 = 0;
    u16                        phy_type = VTSS_PHY_TYPE_NONE;
    BOOL                       clause45 = FALSE;
    vtss_phy_ts_oper_mode_t    oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_mmd_read_t            mmd_read_func;
    vtss_mmd_write_t           mmd_write_func;

    mmd_read_func  = vtss_state->init_conf.mmd_read;
    mmd_write_func = vtss_state->init_conf.mmd_write;

    VTSS_RC(vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode));
    if (clause45) {
        VTSS_RC(mmd_read_func(vtss_state, port_no, mmd_addr, 0x001A, &reg_value_1));
        VTSS_RC(mmd_read_func(vtss_state, port_no, mmd_addr, 0x0102, &reg_value_2));
        *block_sync = (((reg_value_1 == (u16)0xFF01) && (reg_value_2 == (u16)0x8100)) ? TRUE : FALSE);
        if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done != *block_sync) {
            reg_value_1 = (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done) ? 0xFF01 : 0;
            VTSS_RC(mmd_write_func(vtss_state, port_no, mmd_addr, 0x001A, reg_value_1));
            reg_value_2 = (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done) ? 0x8100 : 0;
            VTSS_RC(mmd_write_func(vtss_state, port_no, mmd_addr, 0x0102, reg_value_2));
        }
    } else {
        /* initial setup of extended register 29 and 30 in 1588 extended page */
        /* 1588 - Page Selection */
        VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, port_no, VTSS_PHY_TS_1G_ADDR_EXT_REG, 0x1588));
        /* Read the lower word data from register 29
         */
        VTSS_RC(vtss_state->init_conf.miim_read(vtss_state, port_no, 29, &reg_value_1));
        /* Read the upper word data from register 30
         */
        VTSS_RC(vtss_state->init_conf.miim_read(vtss_state, port_no, 30, &reg_value_2));
        *block_sync = (((reg_value_1 == 0x7AE0) && (reg_value_2 == 0xB71C)) ? TRUE : FALSE);
        if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done != *block_sync) {
            reg_value_1 = (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done) ? 0x7AE0 : 0;
            VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, port_no, 29, reg_value_1));
            reg_value_2 = (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done) ? 0xB71C : 0;
            VTSS_RC(vtss_state->init_conf.miim_write(vtss_state, port_no, 30, reg_value_2));
            /* TODO :: Need to Reset the 1G registers: disable mode for the port and the engines */
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mode_sync(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    const vtss_port_no_t base_port_no,
    BOOL *phy_engine_sync_reqd)
{
    u32                        base_port_value = 0, port_actual_state;

    *phy_engine_sync_reqd = FALSE;

    if (port_no == base_port_no) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                     VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &base_port_value));

    } else {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(1),
                                     VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &base_port_value));
    }
    /* VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS is same as
       VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS for Gen1 devices */
    port_actual_state = ~base_port_value & VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS;


    /* Identify the cases for which we can directly update the engine
     * configuration with out validating the registers */
    if (vtss_state->phy_ts_port_conf[port_no].port_ena) {
        if (port_actual_state) {
            /* Port is Normal :: We need to sync */
            *phy_engine_sync_reqd = TRUE;

        } else {
            /* Interface should be enabled which is taken care after syncing all the engines */
        }
    } else {
        *phy_engine_sync_reqd = FALSE;
        /* VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS is same as
           VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_BYPASS for Gen1 devices */
        base_port_value |= VTSS_F_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL_EGR_BYPASS;
        VTSS_RC(VTSS_PHY_TS_WRITE_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                      VTSS_PTP_IP_1588_TOP_CFG_STAT_INTERFACE_CTL, &base_port_value));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_sync_priv(
    vtss_state_t *vtss_state,
    BOOL  ingress,
    const vtss_port_no_t port_no,
    const vtss_port_no_t base_port_no,
    const vtss_phy_ts_engine_t eng_id,
    const vtss_phy_ts_blk_id_t blk_id,
    BOOL *eng_ena,
    vtss_phy_ts_engine_flow_match_t *eng_match_mode)
{
    u32 value, hw_eng_conf0, hw_eng_conf1 = 0;
    vtss_phy_ts_engine_t engine_id;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
    if (ingress) {
        hw_eng_conf0 = (VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(value));
    } else {
        hw_eng_conf0 = (VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(value));
    }
    if (!hw_eng_conf0) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(base_port_no, VTSS_PHY_TS_PROC_BLK_ID(1),
                                     VTSS_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE, &value));
        if (ingress) {
            hw_eng_conf1 = (VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_INGR_ENCAP_ENGINE_ENA(value));
        } else {
            hw_eng_conf1 = (VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_EGR_ENCAP_ENGINE_ENA(value));
        }
        if (hw_eng_conf0 != hw_eng_conf1) {
            hw_eng_conf0 = hw_eng_conf1;
        }

    }

    engine_id = (eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) ?  eng_id :
                VTSS_PHY_TS_OAM_ENGINE_ID_2A;

    *eng_ena = ((hw_eng_conf0 & (0x01 << engine_id))  >> engine_id) ? TRUE : FALSE;

    if (*eng_ena && eng_id ==  VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
        /* Decide if this engine is enabled */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
        if (VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B(value)) {
            *eng_ena = TRUE;
        } else {
            *eng_ena = FALSE;
        }
    }
    hw_eng_conf0 = VTSS_X_PTP_IP_1588_TOP_CFG_STAT_ANALYZER_MODE_ENCAP_FLOW_MODE(value);

    *eng_match_mode =  ((hw_eng_conf0 & (0x01 << engine_id))  >> engine_id) ? VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT :
                       VTSS_PHY_TS_ENG_FLOW_MATCH_ANY;

    return VTSS_RC_OK;
}

#define VTSS_PHY_TS_ENGINE_FLOW_START 0
#define VTSS_PHY_TS_ENGINE_FLOW_COUNT 8
#define VTSS_PHY_TS_ENGINE_FLOW_END   (VTSS_PHY_TS_ENGINE_FLOW_START + VTSS_PHY_TS_ENGINE_FLOW_COUNT)

static vtss_rc vtss_phy_ts_eth1_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL                           *hw_engine_update_reqd)
{
    u32 value, temp, i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_eth_conf_t         *hw_eth_conf  = &(hw_flow_conf->flow_conf.ptp.eth1_opt);
    vtss_phy_ts_eth_conf_t         *sw_eth_conf  = &(sw_flow_conf->flow_conf.ptp.eth1_opt);
    u32 match_mode_val      = 0;
    u32 tag_rng_i_tag_lower = 0, tag_rng_i_tag_upper = 0;
    u32 tag2_i_tag_lower    = 0, tag2_i_tag_upper    = 0;
    u32 tag1_lower          = 0, tag1_upper          = 0;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* PTP engine Eth1 config */
        /* Tag mode config */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE, &value));
        hw_eth_conf->comm_opt.pbb_en = (value & VTSS_F_ANA_ETH1_NXT_PROTOCOL_ETH1_TAG_MODE_ETH1_PBB_ENA) ? TRUE : FALSE;

        /* TPID config */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG, &value));
        hw_eth_conf->comm_opt.tpid = VTSS_X_ANA_ETH1_NXT_PROTOCOL_ETH1_VLAN_TPID_CFG_ETH1_VLAN_TPID_CFG (value);

        /* etype match */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH, &value));
        hw_eth_conf->comm_opt.etype = VTSS_X_ANA_ETH1_NXT_PROTOCOL_ETH1_ETYPE_MATCH_ETH1_ETYPE_MATCH(value);

        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow config extraction*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            hw_eth_conf->flow_opt[i].flow_en = (value & VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE) ? TRUE : FALSE;
            if (!hw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(value);

            /* addr match mode, match select and last 2 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
            hw_eth_conf->flow_opt[i].addr_match_mode = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(value);
            hw_eth_conf->flow_opt[i].addr_match_select = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(value);
            temp = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(value);
            hw_eth_conf->flow_opt[i].mac_addr[4] = (u8)((temp & 0xff00) >> 8);
            hw_eth_conf->flow_opt[i].mac_addr[5] = (u8)((temp & 0x00ff));

            /* first 4 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));

            hw_eth_conf->flow_opt[i].mac_addr[0] = (u8)((value & 0xff000000) >> 24);
            hw_eth_conf->flow_opt[i].mac_addr[1] = (u8)((value & 0x00ff0000) >> 16);
            hw_eth_conf->flow_opt[i].mac_addr[2] = (u8)((value & 0x0000ff00) >>  8);
            hw_eth_conf->flow_opt[i].mac_addr[3] = (u8)((value & 0x000000ff));

            /* ETH1_MATCH_MODE */
            if (hw_eth_conf->flow_opt[i].flow_en) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
                hw_eth_conf->flow_opt[i].vlan_check = (match_mode_val & VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA) ? 1 : 0;
                hw_eth_conf->flow_opt[i].num_tag = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(match_mode_val);
                hw_eth_conf->flow_opt[i].tag_range_mode = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(match_mode_val);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
                tag_rng_i_tag_lower = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_LOWER(value);
                tag_rng_i_tag_upper = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
                tag1_lower = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MATCH(value);
                tag1_upper = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
                tag2_i_tag_lower = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MATCH(value);
                tag2_i_tag_upper = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(value);


                if (hw_eth_conf->comm_opt.pbb_en) {
                    hw_eth_conf->flow_opt[i].inner_tag.i_tag.val  = (((tag_rng_i_tag_upper & 0xFFF) << 12) | (tag_rng_i_tag_lower & 0xfff)); /* I-tag 24-bits */
                    hw_eth_conf->flow_opt[i].inner_tag.i_tag.mask = (((tag2_i_tag_upper & 0xFFF) << 12) | (tag2_i_tag_lower & 0xFFF)); /*I-tag 24-bits */
                    if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = tag1_lower & 0xFFF; /* 12-bits */
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = tag1_upper & 0xFFF; /* 12-bits */
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                    hw_eth_conf->flow_opt[i].outer_tag_type = (VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE & match_mode_val) ? VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE & match_mode_val) ? VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag2_i_tag_lower && (tag_rng_i_tag_lower || tag_rng_i_tag_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                    } else if (tag2_i_tag_lower && (tag1_lower || tag1_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 1) {
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE & match_mode_val) ?
                                                              VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag_rng_i_tag_lower || tag_rng_i_tag_upper) {
                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else {
                    hw_eth_conf->comm_opt.pbb_en = FALSE;
                    hw_eth_conf->flow_opt[i].num_tag = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.val   = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.mask  = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0;
                }
            }
        }
    } else {
        /* OAM engine Eth1 config */
        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            /* Tag mode config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A, &value));
            hw_eth_conf->comm_opt.pbb_en = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_TAG_MODE_A_ETH1_PBB_ENA_A & value;

            /* TPID config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A, &value));
            hw_eth_conf->comm_opt.tpid = VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_VLAN_TPID_CFG_A_ETH1_VLAN_TPID_CFG_A(value);

            /* etype match */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A, &value));
            hw_eth_conf->comm_opt.etype = VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_ETYPE_MATCH_A_ETH1_ETYPE_MATCH_A(value);

        } else {
            /* Tag mode config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B, &value));
            hw_eth_conf->comm_opt.pbb_en = VTSS_F_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_TAG_MODE_B_ETH1_PBB_ENA_B & value;

            /* TPID config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B, &value));
            hw_eth_conf->comm_opt.tpid = VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_VLAN_TPID_CFG_B_ETH1_VLAN_TPID_CFG_B(value);

            /* etype match */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B, &value));
            hw_eth_conf->comm_opt.etype = VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_ETYPE_MATCH_B_ETH1_ETYPE_MATCH_B(value);
        }

        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow enable config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE(i), &value));
            hw_eth_conf->flow_opt[i].flow_en = VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_FLOW_ENABLE & value;
            if (!hw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_FLOW_ENABLE_ETH1_CHANNEL_MASK(value);
            /* addr match mode, match select and last 2 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2(i), &value));
            hw_eth_conf->flow_opt[i].addr_match_mode = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_MODE(value);
            hw_eth_conf->flow_opt[i].addr_match_select = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(value);

            temp = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(value);
            hw_eth_conf->flow_opt[i].mac_addr[4] = (u8)((temp & 0xff00) >> 8);
            hw_eth_conf->flow_opt[i].mac_addr[5] = (u8)((temp & 0x00ff));

            /* first 4 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_1(i), &value));
            hw_eth_conf->flow_opt[i].mac_addr[0] = (u8)((value & 0xff000000) >> 24);
            hw_eth_conf->flow_opt[i].mac_addr[1] = (u8)((value & 0x00ff0000) >> 16);
            hw_eth_conf->flow_opt[i].mac_addr[2] = (u8)((value & 0x0000ff00) >>  8);
            hw_eth_conf->flow_opt[i].mac_addr[3] = (u8) (value & 0x000000ff);

            if (hw_eth_conf->flow_opt[i].flow_en) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE(i), &match_mode_val));
                hw_eth_conf->flow_opt[i].vlan_check = (match_mode_val & VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_VERIFY_ENA) ? 1 : 0;
                hw_eth_conf->flow_opt[i].num_tag = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAGS(match_mode_val);
                hw_eth_conf->flow_opt[i].tag_range_mode = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG_MODE(match_mode_val);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG(i), &value));
                tag_rng_i_tag_lower = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_LOWER(value);
                tag_rng_i_tag_upper = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG_RANGE_I_TAG_ETH1_VLAN_TAG_RANGE_UPPER(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1(i), &value));
                tag1_lower = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MATCH(value);
                tag1_upper = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG1_ETH1_VLAN_TAG1_MASK(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG(i), &value));
                tag2_i_tag_lower = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MATCH(value);
                tag2_i_tag_upper = VTSS_X_ANA_OAM_ETH1_FLOW_CFG_ETH1_VLAN_TAG2_I_TAG_ETH1_VLAN_TAG2_MASK(value);


                if (hw_eth_conf->comm_opt.pbb_en) {
                    hw_eth_conf->flow_opt[i].inner_tag.i_tag.val  = (((tag_rng_i_tag_upper & 0xFFF) << 12) | (tag_rng_i_tag_lower & 0xfff)); /* I-tag 24-bits */
                    hw_eth_conf->flow_opt[i].inner_tag.i_tag.mask = (((tag2_i_tag_upper & 0xFFF) << 12) | (tag2_i_tag_lower & 0xFFF)); /*I-tag 24-bits */
                    if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = tag1_lower & 0xFFF; /* 12-bits */
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = tag1_upper & 0xFFF; /* 12-bits */
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                    hw_eth_conf->flow_opt[i].outer_tag_type =
                        (VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE & match_mode_val) ?
                        VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    hw_eth_conf->flow_opt[i].inner_tag_type =
                        (VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG2_TYPE & match_mode_val) ?
                        VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag2_i_tag_lower && (tag_rng_i_tag_lower || tag_rng_i_tag_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                    } else if (tag2_i_tag_lower && (tag1_lower || tag1_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 1) {
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_OAM_ETH1_FLOW_CFG_ETH1_MATCH_MODE_ETH1_VLAN_TAG1_TYPE & match_mode_val) ?
                                                              VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag_rng_i_tag_lower || tag_rng_i_tag_upper) {
                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else {
                    hw_eth_conf->comm_opt.pbb_en = FALSE;
                    hw_eth_conf->flow_opt[i].num_tag = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.val   = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.mask  = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0;
                }
            }
        }
    }

    /*  compare the sw_flow_conf with hw_eth_conf */
    do {
        if ((hw_eth_conf->comm_opt.pbb_en != sw_eth_conf->comm_opt.pbb_en) ||
            (hw_eth_conf->comm_opt.etype  != sw_eth_conf->comm_opt.etype) ||
            (hw_eth_conf->comm_opt.tpid   != sw_eth_conf->comm_opt.tpid)) {
            VTSS_E("Config Mismatch : ETH1 pbb_en/etype/tpid mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {

            if (hw_eth_conf->flow_opt[i].flow_en           != sw_eth_conf->flow_opt[i].flow_en) {
                VTSS_E("Config Mismatch:ETH1 Flow Enable parameter mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (!sw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            if ((hw_eth_conf->flow_opt[i].addr_match_mode   != sw_eth_conf->flow_opt[i].addr_match_mode)   ||
                (hw_eth_conf->flow_opt[i].addr_match_select != sw_eth_conf->flow_opt[i].addr_match_select) ||
                (hw_eth_conf->flow_opt[i].vlan_check        != sw_eth_conf->flow_opt[i].vlan_check)     ||
                (hw_eth_conf->flow_opt[i].num_tag           != sw_eth_conf->flow_opt[i].num_tag)        ||
//               (hw_eth_conf->flow_opt[i].outer_tag_type    != sw_eth_conf->flow_opt[i].outer_tag_type) ||
//               (hw_eth_conf->flow_opt[i].inner_tag_type    != sw_eth_conf->flow_opt[i].inner_tag_type) ||
                (hw_eth_conf->flow_opt[i].tag_range_mode    != sw_eth_conf->flow_opt[i].tag_range_mode)) {
                VTSS_E("Config Mismatch:ETH1 Flow parameters mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);

                *hw_engine_update_reqd = TRUE;
                break;
            }

            /* Break for switch case 2 is removed for optimization */
            switch (sw_eth_conf->flow_opt[i].num_tag) {
            case 2 :
                /* Range checking is not supported when PBB is enabled */
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_NONE) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.value.val  != sw_eth_conf->flow_opt[i].outer_tag.value.val) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.value.mask != sw_eth_conf->flow_opt[i].outer_tag.value.mask)) {
                        VTSS_E("Config Mismatch:ETH1 Outer Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);
                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                }
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER && !sw_eth_conf->comm_opt.pbb_en) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.range.upper != sw_eth_conf->flow_opt[i].outer_tag.range.upper ) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.range.lower != sw_eth_conf->flow_opt[i].outer_tag.range.lower )) {
                        VTSS_E("Config Mismatch:ETH1 Outer Tag Range mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);
                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                }
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER && !sw_eth_conf->comm_opt.pbb_en) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.value.val  != sw_eth_conf->flow_opt[i].outer_tag.value.val) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.value.mask != sw_eth_conf->flow_opt[i].outer_tag.value.mask)) {
                        VTSS_E("Config Mismatch:ETH1 Outer Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }

                }
                /* break; */ /* Code is optimized to take care of both the tags */
            case 1:
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER && !sw_eth_conf->comm_opt.pbb_en) {
                    if ((hw_eth_conf->flow_opt[i].inner_tag.range.upper != sw_eth_conf->flow_opt[i].inner_tag.range.upper ) ||
                        (hw_eth_conf->flow_opt[i].inner_tag.range.lower != sw_eth_conf->flow_opt[i].inner_tag.range.lower )) {
                        VTSS_E("Config Mismatch:ETH1 Inner Tag Range mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                } else {
                    if (sw_eth_conf->comm_opt.pbb_en) {
                        if ((hw_eth_conf->flow_opt[i].inner_tag.i_tag.val  != sw_eth_conf->flow_opt[i].inner_tag.i_tag.val) ||
                            (hw_eth_conf->flow_opt[i].inner_tag.i_tag.mask != sw_eth_conf->flow_opt[i].inner_tag.i_tag.mask)) {
                            VTSS_E("Config Mismatch:ETH1 I-Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                                   (unsigned int)port_no, eng_parm->eng_id, i);

                            *hw_engine_update_reqd = TRUE;
                            break;
                        }
                    } else {
                        if ((hw_eth_conf->flow_opt[i].inner_tag.value.val  != sw_eth_conf->flow_opt[i].inner_tag.value.val) ||
                            (hw_eth_conf->flow_opt[i].inner_tag.value.mask != sw_eth_conf->flow_opt[i].inner_tag.value.mask)) {
                            VTSS_E("Config Mismatch:ETH1 Inner Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                                   (unsigned int)port_no, eng_parm->eng_id, i);

                            *hw_engine_update_reqd = TRUE;
                            break;
                        }
                    }
                }
                break;
            default :
                /* No need to check */
                break;
            }

            if (memcmp(hw_eth_conf->flow_opt[i].mac_addr, sw_eth_conf->flow_opt[i].mac_addr, sizeof(sw_eth_conf->flow_opt[i].mac_addr))) {
                VTSS_E("Config Mismatch: ETH1 MAC-address mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
        }
    } while (0);
    if (*hw_engine_update_reqd) {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_eth2_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL                           *hw_engine_update_reqd)
{
    u32 value, temp, i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_eth_conf_t         *hw_eth_conf  = &(hw_flow_conf->flow_conf.ptp.eth2_opt);
    vtss_phy_ts_eth_conf_t         *sw_eth_conf  = &(sw_flow_conf->flow_conf.ptp.eth2_opt);

    u32 match_mode_val      = 0;
    u32 tag_rng_i_tag_lower = 0, tag_rng_i_tag_upper = 0;
    u32 tag2_i_tag_lower    = 0, tag2_i_tag_upper    = 0;
    u32 tag1_lower          = 0, tag1_upper          = 0;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;


    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* PTP engine Eth2 config */
        /* TPID config */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG, &value));
        hw_eth_conf->comm_opt.tpid = VTSS_X_ANA_ETH2_NXT_PROTOCOL_ETH2_VLAN_TPID_CFG_ETH2_VLAN_TPID_CFG (value);

        /* etype match */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH, &value));
        hw_eth_conf->comm_opt.etype = VTSS_X_ANA_ETH2_NXT_PROTOCOL_ETH2_ETYPE_MATCH_ETH2_ETYPE_MATCH(value);

        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow config extraction*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            hw_eth_conf->flow_opt[i].flow_en = (value & VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE) ? TRUE : FALSE;
            if (!hw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(value);

            /* addr match mode, match select and last 2 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
            hw_eth_conf->flow_opt[i].addr_match_mode = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(value);
            hw_eth_conf->flow_opt[i].addr_match_select = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_SELECT(value);
            temp = VTSS_X_ANA_ETH1_FLOW_CFG_ETH1_ADDR_MATCH_2_ETH1_ADDR_MATCH_2(value);
            hw_eth_conf->flow_opt[i].mac_addr[4] = (u8)((temp & 0xff00) >> 8);
            hw_eth_conf->flow_opt[i].mac_addr[5] = (u8)(temp & 0x00ff);

            /* first 4 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));

            hw_eth_conf->flow_opt[i].mac_addr[0] = (u8)((value & 0xff000000) >> 24);
            hw_eth_conf->flow_opt[i].mac_addr[1] = (u8)((value & 0x00ff0000) >> 16);
            hw_eth_conf->flow_opt[i].mac_addr[2] = (u8)((value & 0x0000ff00) >>  8);
            hw_eth_conf->flow_opt[i].mac_addr[3] = (u8)(value & 0x000000ff);

            /* ETH1_MATCH_MODE */
            if (hw_eth_conf->flow_opt[i].flow_en) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
                hw_eth_conf->flow_opt[i].vlan_check = (match_mode_val & VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA) ? 1 : 0;
                hw_eth_conf->flow_opt[i].num_tag = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(match_mode_val);
                hw_eth_conf->flow_opt[i].tag_range_mode = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(match_mode_val);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
                tag_rng_i_tag_lower = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_LOWER(value);
                tag_rng_i_tag_upper = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
                tag1_lower = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MATCH(value);
                tag1_upper = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
                tag2_i_tag_lower = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MATCH(value);
                tag2_i_tag_upper = VTSS_X_ANA_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(value);

                if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                    hw_eth_conf->flow_opt[i].outer_tag_type = (VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE & match_mode_val) ? VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE & match_mode_val) ? VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag2_i_tag_lower && (tag_rng_i_tag_lower || tag_rng_i_tag_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                    } else if (tag2_i_tag_lower && (tag1_lower || tag1_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 1) {
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE & match_mode_val) ?
                                                              VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag_rng_i_tag_lower || tag_rng_i_tag_upper) {
                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else {
                    hw_eth_conf->comm_opt.pbb_en = FALSE;
                    hw_eth_conf->flow_opt[i].num_tag = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.val   = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.mask  = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0;
                }
            }
        }
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* OAM engine Eth2 config */
        /* TPID config */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A, &value));
        hw_eth_conf->comm_opt.tpid = VTSS_X_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_VLAN_TPID_CFG_A_ETH2_VLAN_TPID_CFG_A(value);

        /* etype match */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A, &value));
        hw_eth_conf->comm_opt.etype = VTSS_X_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_ETYPE_MATCH_A_ETH2_ETYPE_MATCH_A(value);

        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow config extraction*/
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE(i), &value));
            hw_eth_conf->flow_opt[i].flow_en = (value & VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_FLOW_ENABLE) ? TRUE : FALSE;
            if (!hw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_FLOW_ENABLE_ETH2_CHANNEL_MASK(value);

            /* addr match mode, match select and last 2 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2(i), &value));
            hw_eth_conf->flow_opt[i].addr_match_mode = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_MODE(value);
            hw_eth_conf->flow_opt[i].addr_match_select = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_SELECT(value);
            temp = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_2_ETH2_ADDR_MATCH_2(value);
            hw_eth_conf->flow_opt[i].mac_addr[4] = (u8)((temp & 0xff00) >> 8);
            hw_eth_conf->flow_opt[i].mac_addr[5] = (u8)(temp & 0x00ff);

            /* first 4 bytes of MAC */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_ADDR_MATCH_1(i), &value));

            hw_eth_conf->flow_opt[i].mac_addr[0] = (u8)((value & 0xff000000) >> 24);
            hw_eth_conf->flow_opt[i].mac_addr[1] = (u8)((value & 0x00ff0000) >> 16);
            hw_eth_conf->flow_opt[i].mac_addr[2] = (u8)((value & 0x0000ff00) >>  8);
            hw_eth_conf->flow_opt[i].mac_addr[3] = (u8)(value & 0x000000ff);

            /* ETH1_MATCH_MODE */
            if (hw_eth_conf->flow_opt[i].flow_en) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE(i), &match_mode_val));
                hw_eth_conf->flow_opt[i].vlan_check = (match_mode_val & VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_VERIFY_ENA) ? 1 : 0;
                hw_eth_conf->flow_opt[i].num_tag = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAGS(match_mode_val);
                hw_eth_conf->flow_opt[i].tag_range_mode = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG_MODE(match_mode_val);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG(i), &value));
                tag_rng_i_tag_lower = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_LOWER(value);
                tag_rng_i_tag_upper = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG_RANGE_I_TAG_ETH2_VLAN_TAG_RANGE_UPPER(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1(i), &value));
                tag1_lower = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MATCH(value);
                tag1_upper = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG1_ETH2_VLAN_TAG1_MASK(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG(i), &value));
                tag2_i_tag_lower = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MATCH(value);
                tag2_i_tag_upper = VTSS_X_ANA_OAM_ETH2_FLOW_CFG_ETH2_VLAN_TAG2_I_TAG_ETH2_VLAN_TAG2_MASK(value);

                if (hw_eth_conf->flow_opt[i].num_tag == 2) {
                    hw_eth_conf->flow_opt[i].outer_tag_type =
                        (VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE & match_mode_val) ?
                        VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    hw_eth_conf->flow_opt[i].inner_tag_type =
                        (VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG2_TYPE & match_mode_val) ?
                        VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;

                    if (tag2_i_tag_lower && (tag_rng_i_tag_lower || tag_rng_i_tag_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                    } else if (tag2_i_tag_lower && (tag1_lower || tag1_upper)) {
                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;

                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag2_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag2_i_tag_upper;

                        hw_eth_conf->flow_opt[i].outer_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].outer_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else if (hw_eth_conf->flow_opt[i].num_tag == 1) {
                    hw_eth_conf->flow_opt[i].inner_tag_type = (VTSS_F_ANA_OAM_ETH2_FLOW_CFG_ETH2_MATCH_MODE_ETH2_VLAN_TAG1_TYPE & match_mode_val) ?
                                                              VTSS_PHY_TS_TAG_TYPE_S : VTSS_PHY_TS_TAG_TYPE_C;
                    if (tag_rng_i_tag_lower || tag_rng_i_tag_upper) {
                        hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0xFFF & tag_rng_i_tag_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0xFFF & tag_rng_i_tag_upper;
                    } else {
                        hw_eth_conf->flow_opt[i].inner_tag.value.val  = 0xFFF & tag1_lower;
                        hw_eth_conf->flow_opt[i].inner_tag.value.mask = 0xFFF & tag1_upper;
                    }
                } else {
                    hw_eth_conf->comm_opt.pbb_en = FALSE;
                    hw_eth_conf->flow_opt[i].num_tag = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.val   = 0;
                    hw_eth_conf->flow_opt[i].outer_tag.value.mask  = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.lower = 0;
                    hw_eth_conf->flow_opt[i].inner_tag.range.upper = 0;
                }
            }
        }
    }

    /*  compare the sw_flow_conf with hw_eth_conf */
    do {
        if ( (hw_eth_conf->comm_opt.etype != sw_eth_conf->comm_opt.etype) ||
             (hw_eth_conf->comm_opt.tpid  != sw_eth_conf->comm_opt.tpid)) {
            VTSS_E("Config Mismatch : ETH2 pbb_en/etype/tpid mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            if (hw_eth_conf->flow_opt[i].flow_en           != sw_eth_conf->flow_opt[i].flow_en) {
                VTSS_E("Config Mismatch:ETH2 Flow Enable parameter mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (!sw_eth_conf->flow_opt[i].flow_en) {
                continue;
            }
            if ((hw_eth_conf->flow_opt[i].addr_match_mode   != sw_eth_conf->flow_opt[i].addr_match_mode)   ||
                (hw_eth_conf->flow_opt[i].addr_match_select != sw_eth_conf->flow_opt[i].addr_match_select) ||
                (hw_eth_conf->flow_opt[i].vlan_check        != sw_eth_conf->flow_opt[i].vlan_check)     ||
                (hw_eth_conf->flow_opt[i].num_tag           != sw_eth_conf->flow_opt[i].num_tag)        ||
//               (hw_eth_conf->flow_opt[i].outer_tag_type    != sw_eth_conf->flow_opt[i].outer_tag_type) ||
//               (hw_eth_conf->flow_opt[i].inner_tag_type    != sw_eth_conf->flow_opt[i].inner_tag_type) ||
                (hw_eth_conf->flow_opt[i].tag_range_mode    != sw_eth_conf->flow_opt[i].tag_range_mode)) {
                VTSS_E("Config Mismatch:ETH2 Flow parameters mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);

                *hw_engine_update_reqd = TRUE;
                break;
            }

            /* Break for switch case 2 is removed for optimization */
            switch (sw_eth_conf->flow_opt[i].num_tag) {
            case 2 :
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_NONE) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.value.val  != sw_eth_conf->flow_opt[i].outer_tag.value.val) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.value.mask != sw_eth_conf->flow_opt[i].outer_tag.value.mask)) {
                        VTSS_E("Config Mismatch:ETH2 Inner Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                }
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.range.upper != sw_eth_conf->flow_opt[i].outer_tag.range.upper ) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.range.lower != sw_eth_conf->flow_opt[i].outer_tag.range.lower )) {
                        VTSS_E("Config Mismatch:ETH2 Outer Tag Range mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                }
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    if ((hw_eth_conf->flow_opt[i].outer_tag.value.val  != sw_eth_conf->flow_opt[i].outer_tag.value.val) ||
                        (hw_eth_conf->flow_opt[i].outer_tag.value.mask != sw_eth_conf->flow_opt[i].outer_tag.value.mask)) {
                        VTSS_E("Config Mismatch:ETH2 Inner Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }

                }
                /* break; */ /* Code is optimized to take care of both the tags */
            case 1:
                if (sw_eth_conf->flow_opt[i].tag_range_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                    if ((hw_eth_conf->flow_opt[i].inner_tag.range.upper != sw_eth_conf->flow_opt[i].inner_tag.range.upper ) ||
                        (hw_eth_conf->flow_opt[i].inner_tag.range.lower != sw_eth_conf->flow_opt[i].inner_tag.range.lower )) {
                        VTSS_E("Config Mismatch:ETH2 Inner Tag Range mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                } else {
                    if ((hw_eth_conf->flow_opt[i].inner_tag.value.val  != sw_eth_conf->flow_opt[i].inner_tag.value.val) ||
                        (hw_eth_conf->flow_opt[i].inner_tag.value.mask != sw_eth_conf->flow_opt[i].inner_tag.value.mask)) {
                        VTSS_E("Config Mismatch:ETH2 Inner Tag value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                               (unsigned int)port_no, eng_parm->eng_id, i);

                        *hw_engine_update_reqd = TRUE;
                        break;
                    }
                }
                break;
            default :
                /* No need to check */
                break;
            }

            if (memcmp(hw_eth_conf->flow_opt[i].mac_addr, sw_eth_conf->flow_opt[i].mac_addr, sizeof(sw_eth_conf->flow_opt[i].mac_addr))) {
                VTSS_E("Config Mismatch: ETH2 MAC-address mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);

                *hw_engine_update_reqd = TRUE;
                break;
            }
        }
    } while (0);

    if (*hw_engine_update_reqd == TRUE) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip1_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    BOOL                            double_ip,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL *hw_engine_update_reqd)
{
    u32 value, temp, i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_ip_conf_t          *hw_ip_conf   = &(hw_flow_conf->flow_conf.ptp.ip1_opt);
    vtss_phy_ts_ip_conf_t          *sw_ip_conf   = &(sw_flow_conf->flow_conf.ptp.ip1_opt);

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id  = eng_parm->blk_id;

    *hw_engine_update_reqd = FALSE;
    if (double_ip) {

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        temp = VTSS_X_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(value);
        if (temp == 20) {
            hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (temp == 40) {
            hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_6;
        }

        hw_ip_conf->comm_opt.sport_val  = 0;
        hw_ip_conf->comm_opt.sport_mask = 0;
        hw_ip_conf->comm_opt.dport_val  = 0;
        hw_ip_conf->comm_opt.dport_mask = 0;

        /* Src and dest port number is not valid for IP1 i.e. IP over IP,
           so PROT_MATCH_2 no need to config, already set to default from
           def conf */

        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow enable, match mode config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            hw_ip_conf->flow_opt[i].flow_en = (value & VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA);
            if (!hw_ip_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(value);
            hw_ip_conf->flow_opt[i].match_mode = VTSS_X_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(value);

            /* IP address */
            if (hw_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv4.addr = value;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv4.mask = value;
            } else {
                /* Upper 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3] = value;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3] = value;
                /* Upper mid 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2] = value;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2] = value;
                /* Lower mid 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1] = value;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1] = value;
                /* Lower 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0] = value;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0] = value;
            }
        }
    } else {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        temp = VTSS_X_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(value);
        if (28 == temp) {
            hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (48 == temp) {
            hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_6;
        }

        /* Src and dest port number */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
        hw_ip_conf->comm_opt.sport_val = (u16)((value & 0xFFFF0000) >> 16);
        hw_ip_conf->comm_opt.dport_val = (u16)(value & 0x0000FFFF);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
        hw_ip_conf->comm_opt.sport_mask = (u16)((value & 0xFFFF0000) >> 16);
        hw_ip_conf->comm_opt.dport_mask = (u16)(value & 0x0000FFFF);

        /* UDP port number offset from IP header: taken care in def conf */
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map and flow enable, match mode config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
            hw_ip_conf->flow_opt[i].flow_en = value & VTSS_F_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_ENA;
            if (!hw_ip_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(value);
            hw_ip_conf->flow_opt[i].match_mode = VTSS_X_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_FLOW_MATCH_MODE(value);

            /* IP address */
            if (hw_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv4.addr = value;

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv4.mask = value;
            } else {
                /* Upper 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3] = value;

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3] = value;

                /* Upper mid 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_UPPER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2] = value;

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_UPPER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2] = value;

                /* Lower mid 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1] = value;

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER_MID(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1] = value;

                /* Lower 32-bit of ipv6 address */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MATCH_LOWER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0] = value;

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_MASK_LOWER(i), &value));
                hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0] = value;
            }
        }
    }

    do {
        /* compare the sw_flow_conf with hw_ip_conf for IP1 comparator */
        if (hw_ip_conf->comm_opt.ip_mode    != sw_ip_conf->comm_opt.ip_mode) {
            VTSS_E("Config Mismatch in IP1 mode mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        if (!double_ip && ((hw_ip_conf->comm_opt.sport_val  != sw_ip_conf->comm_opt.sport_val)  ||
                           (hw_ip_conf->comm_opt.sport_mask != sw_ip_conf->comm_opt.sport_mask) ||
                           (hw_ip_conf->comm_opt.dport_val  != sw_ip_conf->comm_opt.dport_val)  ||
                           (hw_ip_conf->comm_opt.dport_mask != sw_ip_conf->comm_opt.dport_mask))) {
            VTSS_E("Config Mismatch in IP1 common parameters mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            if ((hw_ip_conf->flow_opt[i].flow_en    != sw_ip_conf->flow_opt[i].flow_en)) {
                VTSS_E("Config Mismatch in IP1 Flow Enable parameter mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (!sw_ip_conf->flow_opt[i].flow_en) {
                continue;
            }
            if ((hw_ip_conf->flow_opt[i].match_mode != sw_ip_conf->flow_opt[i].match_mode)) {
                VTSS_E("Config Mismatch in IP1 Flow parameters mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (memcmp(&hw_ip_conf->flow_opt[i].ip_addr, &sw_ip_conf->flow_opt[i].ip_addr, sizeof(sw_ip_conf->flow_opt[i].ip_addr))) {
                VTSS_E("Config Mismatch in IP1 IP-address mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
        }
    } while (0);

    if (*hw_engine_update_reqd) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ip2_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL *hw_engine_update_reqd)
{
    u32 value, temp, i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_ip_conf_t          *hw_ip_conf   = &(hw_flow_conf->flow_conf.ptp.ip2_opt);
    vtss_phy_ts_ip_conf_t          *sw_ip_conf   = &(sw_flow_conf->flow_conf.ptp.ip2_opt);
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;


    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));
    temp = VTSS_X_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_PROTOCOL(value);
    if (temp == 28) {
        hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_4;
    } else if (temp == 48) {
        hw_ip_conf->comm_opt.ip_mode = VTSS_PHY_TS_IP_VER_6;
    }

    /* Src and dest port number */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MATCH_2_UPPER, &value));
    hw_ip_conf->comm_opt.sport_val = (u16)((value & 0xFFFF0000) >> 16);
    hw_ip_conf->comm_opt.dport_val = (u16)(value & 0x0000FFFF);
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_PROT_MASK_2_UPPER, &value));
    hw_ip_conf->comm_opt.sport_mask = (u16)((value & 0xFFFF0000) >> 16);
    hw_ip_conf->comm_opt.dport_mask = (u16)(value & 0x0000FFFF);

    /* UDP port number offset from IP header: taken care in def conf */
    for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
        /* channel map and flow enable, match mode config */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA(i), &value));
        hw_ip_conf->flow_opt[i].flow_en = value & VTSS_F_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_ENA;
        if (!hw_ip_conf->flow_opt[i].flow_en) {
            continue;
        }
        hw_flow_conf->channel_map[i] = VTSS_X_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_CHANNEL_MASK(value);
        hw_ip_conf->flow_opt[i].match_mode = VTSS_X_ANA_IP2_FLOW_CFG_IP2_FLOW_ENA_IP2_FLOW_MATCH_MODE(value);

        /* IP address */
        if (hw_ip_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv4.addr = value;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv4.mask = value;

        } else {
            /* Upper 32-bit of ipv6 address */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[3] = value;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[3] = value;

            /* Upper mid 32-bit of ipv6 address */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_UPPER_MID(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[2] = value;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_UPPER_MID(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[2] = value;

            /* Lower mid 32-bit of ipv6 address */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER_MID(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[1] = value;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER_MID(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[1] = value;
            /* Lower 32-bit of ipv6 address */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MATCH_LOWER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.addr[0] = value;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_FLOW_CFG_IP2_FLOW_MASK_LOWER(i), &value));
            hw_ip_conf->flow_opt[i].ip_addr.ipv6.mask[0] = value;
        }
    }

    /* compare the hw_ip_conf with sw_flow_conf */
    do {
        if (hw_ip_conf->comm_opt.ip_mode    != sw_ip_conf->comm_opt.ip_mode) {
            VTSS_E("Config Mismatch in IP2 mode mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }

        if ((hw_ip_conf->comm_opt.sport_val  != sw_ip_conf->comm_opt.sport_val)  ||
            (hw_ip_conf->comm_opt.sport_mask != sw_ip_conf->comm_opt.sport_mask) ||
            (hw_ip_conf->comm_opt.dport_val  != sw_ip_conf->comm_opt.dport_val)  ||
            (hw_ip_conf->comm_opt.dport_mask != sw_ip_conf->comm_opt.dport_mask)) {
            VTSS_E("Config Mismatch in IP2 common parameters mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            if (hw_ip_conf->flow_opt[i].flow_en    != sw_ip_conf->flow_opt[i].flow_en) {
                VTSS_E("Config Mismatch in IP2 Flow Enable parameter mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (!sw_ip_conf->flow_opt[i].flow_en) {
                continue;
            }
            if (hw_ip_conf->flow_opt[i].match_mode != sw_ip_conf->flow_opt[i].match_mode) {
                VTSS_E("Config Mismatch in IP2 Flow parameters mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (memcmp(&hw_ip_conf->flow_opt[i].ip_addr, &sw_ip_conf->flow_opt[i].ip_addr, sizeof(sw_ip_conf->flow_opt[i].ip_addr))) {
                VTSS_E("Config Mismatch in IP2 IP-address mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
        }
    } while (0);

    if (*hw_engine_update_reqd) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_mpls_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL *hw_engine_update_reqd)
{
    u32 value, i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_mpls_conf_t        *hw_mpls_conf = NULL;
    vtss_phy_ts_mpls_conf_t        *sw_mpls_conf = NULL;
    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;
    const vtss_phy_ts_engine_t eng_id = eng_parm->eng_id;

    if (hw_eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ||
        hw_eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        hw_mpls_conf = &(hw_flow_conf->flow_conf.oam.mpls_opt);
        sw_mpls_conf = &(sw_flow_conf->flow_conf.oam.mpls_opt);
    } else {
        hw_mpls_conf = &(hw_flow_conf->flow_conf.ptp.mpls_opt);
        sw_mpls_conf = &(sw_flow_conf->flow_conf.ptp.mpls_opt);
    }

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        /* MPLS common conf */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        hw_mpls_conf->comm_opt.cw_en = ((value & VTSS_F_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_CTL_WORD) >> 16);

        /* PTP engine MPLS config */
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map, flow enable, stack depth and stack ref point config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));
            hw_mpls_conf->flow_opt[i].flow_en = VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA & value;
            if (!hw_mpls_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(value);
            hw_mpls_conf->flow_opt[i].stack_depth = VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(value);
            hw_mpls_conf->flow_opt[i].stack_ref_point = ((VTSS_F_ANA_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT & value) >> 4);

            /* configure stack levels based on ref point */
            if (hw_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                /* top-of-stack referenced */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.top.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.top.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(value);

                /* 1st after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(value);

                /* 2nd after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(value);

                /* 3rd after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(value);

            } else {
                /* end-of-stack referenced */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(value);

                /* 1st before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(value);

                /* 2nd before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(value);

                /* 3rd before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper =
                    VTSS_X_ANA_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(value);

            }
        }
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        /* MPLS cw */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        hw_mpls_conf->comm_opt.cw_en = ((value & VTSS_F_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_CTL_WORD_A) >> 16);

        /* OAM engine MPLS config */
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            /* channel map, flow enable, stack depth and stack ref point config */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL(i), &value));

            hw_mpls_conf->flow_opt[i].flow_en = value & VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_FLOW_ENA;
            if (!hw_mpls_conf->flow_opt[i].flow_en) {
                continue;
            }
            hw_flow_conf->channel_map[i] = VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_CHANNEL_MASK(value);
            hw_mpls_conf->flow_opt[i].stack_depth = VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_STACK_DEPTH(value);
            hw_mpls_conf->flow_opt[i].stack_ref_point = ((value & VTSS_F_ANA_OAM_MPLS_FLOW_CFG_MPLS_FLOW_CONTROL_MPLS_REF_PNT) >> 4);

            /* configure stack levels based on ref point */
            if (hw_mpls_conf->flow_opt[i].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {

                /* top-of-stack referenced */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.top.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.top.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(value);
                /* 1st after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(value);

                /* 2nd after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(value);

                /* 3rd after top */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(value);
            } else {
                /* end-of-stack referenced */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_3_MPLS_LABEL_RANGE_LOWER_3(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_3_MPLS_LABEL_RANGE_UPPER_3(value);

                /* 1st before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_2_MPLS_LABEL_RANGE_LOWER_2(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_2_MPLS_LABEL_RANGE_UPPER_2(value);

                /* 2nd before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_1_MPLS_LABEL_RANGE_LOWER_1(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_1_MPLS_LABEL_RANGE_UPPER_1(value);
                /* 3rd before end */
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_LOWER_0_MPLS_LABEL_RANGE_LOWER_0(value);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0(i), &value));
                hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper =
                    VTSS_X_ANA_OAM_MPLS_FLOW_CFG_MPLS_LABEL_RANGE_UPPER_0_MPLS_LABEL_RANGE_UPPER_0(value);
            }
        }
    }

    /* compare the hw_mpls_conf with sw_flow_conf */
    do {
        if (hw_mpls_conf->comm_opt.cw_en != sw_mpls_conf->comm_opt.cw_en) {
            VTSS_E("Config Mismatch in MPLS Commom option mismatch on port - [%d] Engine_ID - [%d] ",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
            if (hw_mpls_conf->flow_opt[i].flow_en != sw_mpls_conf->flow_opt[i].flow_en) {
                VTSS_E("Config Mismatch in MPLS Flow Enable mismatch on port - [%d] Engine_ID - [%d] Flow-ID - [%d]",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (!sw_mpls_conf->flow_opt[i].flow_en) {
                continue;
            }
            if ((hw_mpls_conf->flow_opt[i].stack_depth      != sw_mpls_conf->flow_opt[i].stack_depth)  ||
                (hw_mpls_conf->flow_opt[i].stack_ref_point != sw_mpls_conf->flow_opt[i].stack_ref_point)) {
                VTSS_E("Config Mismatch in MPLS Flow configuration mismatch on port - [%d] Engine_ID - [%d] Flow-ID - [%d]",
                       (unsigned int)port_no, eng_parm->eng_id, i);
                *hw_engine_update_reqd = TRUE;
                break;
            }
            if (sw_mpls_conf->flow_opt[i].stack_ref_point) {
                if ((hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.end.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.frst_lvl_before_end.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper   !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower   !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.snd_lvl_before_end.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper  !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.upper) ||
                    (hw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower  !=
                     sw_mpls_conf->flow_opt[i].stack_level.bottom_up.thrd_lvl_before_end.lower)) {
                    VTSS_E("Config Mismatch in MPLS Stack level mismatch on port - [%d] Engine_ID - [%d] Flow-ID - [%d]",
                           (unsigned int)port_no, eng_parm->eng_id, i);
                    *hw_engine_update_reqd = TRUE;
                    break;
                }
            } else {
                if ((hw_mpls_conf->flow_opt[i].stack_level.top_down.top.upper !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.top.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.top.lower !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.top.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.frst_lvl_after_top.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper   !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.upper)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower   !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.snd_lvl_after_top.lower)  ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper  !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.upper) ||
                    (hw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower  !=
                     sw_mpls_conf->flow_opt[i].stack_level.top_down.thrd_lvl_after_top.lower)) {
                    VTSS_E("Config Mismatch in MPLS Stack level mismatch on port - [%d] Engine_ID - [%d] Flow-ID - [%d]",
                           (unsigned int)port_no, eng_parm->eng_id, i);
                    *hw_engine_update_reqd = TRUE;
                    break;
                }
            }
        }
    } while (0);

    if (*hw_engine_update_reqd) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_ach_conf_sync_priv(
    vtss_state_t *vtss_state,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL *hw_engine_update_reqd)
{
    u32 value;
    u8 i;
    vtss_phy_ts_engine_flow_conf_t *hw_flow_conf = &(hw_eng_conf->flow_conf);
    vtss_phy_ts_ach_conf_t         *hw_ach_conf  = NULL;
    vtss_phy_ts_ach_conf_t         *sw_ach_conf  = NULL;

    const vtss_port_no_t       port_no = eng_parm->port_no;
    const vtss_phy_ts_blk_id_t blk_id = eng_parm->blk_id;

    if (hw_eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        hw_ach_conf = &(hw_flow_conf->flow_conf.oam.ach_opt);
        sw_ach_conf = &(sw_flow_conf->flow_conf.oam.ach_opt);
    } else {
        hw_ach_conf = &(hw_flow_conf->flow_conf.ptp.ach_opt);
        sw_ach_conf = &(sw_flow_conf->flow_conf.ptp.ach_opt);
    }


    *hw_engine_update_reqd = FALSE;
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_UPPER, &value));
    hw_ach_conf->comm_opt.version.value      = (u8)((value & 0x0F000000) >> 24);
    hw_ach_conf->comm_opt.channel_type.value = (u16)(value & 0x0000FFFF);

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_UPPER, &value));
    hw_ach_conf->comm_opt.version.mask      = (u8)((value & 0x0F000000) >> 24);
    hw_ach_conf->comm_opt.channel_type.mask = (u16)(value & 0x0000FFFF);

    if (hw_eng_conf->encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MATCH_2_LOWER, &value));
        hw_ach_conf->comm_opt.proto_id.value = (u16)((value & 0xFFFF0000) >> 16);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_PROT_MASK_2_LOWER, &value));
        hw_ach_conf->comm_opt.proto_id.mask = (u16)((value & 0xFFFF0000) >> 16);
    }

    /* change the channel map flow_st_index which is the only index enabled for ACH */
    for (i = VTSS_PHY_TS_ENGINE_FLOW_START; i < VTSS_PHY_TS_ENGINE_FLOW_END; i++) {
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA(i), &value));
        hw_flow_conf->channel_map[i] = VTSS_X_ANA_IP1_FLOW_CFG_IP1_FLOW_ENA_IP1_CHANNEL_MASK(value);
    }

    /* compare the hw_eng_conf with sw_flow_conf */
    do {
        if ((hw_ach_conf->comm_opt.version.value != sw_ach_conf->comm_opt.version.value) ||
            (hw_ach_conf->comm_opt.version.mask  != sw_ach_conf->comm_opt.version.mask)) {

            VTSS_E("Config Mismatch in ACH version mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        if ((hw_ach_conf->comm_opt.channel_type.value != sw_ach_conf->comm_opt.channel_type.value) ||
            (hw_ach_conf->comm_opt.channel_type.mask  != sw_ach_conf->comm_opt.channel_type.mask)) {
            VTSS_E("Config Mismatch in ACH Channel Type mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
        if ((hw_ach_conf->comm_opt.proto_id.value != sw_ach_conf->comm_opt.proto_id.value) ||
            (hw_ach_conf->comm_opt.proto_id.mask  != sw_ach_conf->comm_opt.proto_id.mask)) {
            VTSS_E("Config Mismatch in ACH Protocol Id mismatch on port - [%d] Engine_ID - [%d]\n\r",
                   (unsigned int)port_no, eng_parm->eng_id);
            *hw_engine_update_reqd = TRUE;
            break;
        }
    } while (0);

    if (*hw_engine_update_reqd) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_flow_conf_sync_priv(
    vtss_state_t *vtss_state,
    BOOL                           ingress,
    vtss_ts_engine_parm_t          *eng_parm,
    vtss_phy_ts_eng_conf_t         *hw_eng_conf,
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf,
    BOOL *hw_update_reqd)
{
    vtss_rc rc = VTSS_RC_OK;
    const vtss_port_no_t        port_no = eng_parm->port_no;

    switch (hw_eng_conf->encap_type) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                             hw_eng_conf, sw_flow_conf, hw_update_reqd);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_sync_priv(vtss_state, eng_parm, FALSE,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_sync_priv(vtss_state, eng_parm, TRUE,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip2_conf_sync_priv(vtss_state, eng_parm,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_sync_priv(vtss_state, eng_parm, FALSE,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_sync_priv(vtss_state, eng_parm, FALSE,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ip1_conf_sync_priv(vtss_state, eng_parm, FALSE,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ach_conf_sync_priv(vtss_state, eng_parm,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_eth2_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        if ((rc = vtss_phy_ts_eth1_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_mpls_conf_sync_priv(vtss_state, eng_parm,
                                                  hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        if ((rc = vtss_phy_ts_ach_conf_sync_priv(vtss_state, eng_parm,
                                                 hw_eng_conf, sw_flow_conf, hw_update_reqd)) != VTSS_RC_OK) {
            break;
        }
        break;
    default:
        VTSS_E("Port %u:: invalid encap type", (u32)port_no);
        break;
    }

    return rc;
}

static vtss_rc vtss_phy_ts_engine_encap_conf_sync_priv(
    vtss_state_t *vtss_state,
    BOOL ingress,
    vtss_port_no_t port_no,
    vtss_port_no_t base_port_no,
    vtss_phy_ts_engine_t eng_id,
    vtss_phy_ts_blk_id_t blk_id,
    vtss_phy_ts_encap_t  *encap_type,
    BOOL *action_ptp)
{
    u32 value;
    u32 ach_proto = 0;
    u32 eth1_nxt_comp = 0;
    u32 eth2_nxt_comp = 0;
    u32 ip1_nxt_comp = 0;
    u32 ip2_nxt_comp = 0;
    u32 mpls_nxt_comp = 0;
    u32 action_ct = 0;
    u32 cf_offset = 0;
    vtss_rc rc = VTSS_RC_OK;

    *encap_type = VTSS_PHY_TS_ENCAP_NONE;
    *action_ptp = FALSE;

    if (eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_0 ||
        eng_id == VTSS_PHY_TS_PTP_ENGINE_ID_1) {

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL, &value));
        eth1_nxt_comp = VTSS_X_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR(value);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL, &value));
        eth2_nxt_comp = VTSS_X_ANA_ETH2_NXT_PROTOCOL_ETH2_NXT_PROTOCOL_ETH2_NXT_COMPARATOR(value);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR, &value));
        ip1_nxt_comp = VTSS_X_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_COMPARATOR(value);
        ach_proto    = VTSS_X_ANA_IP1_NXT_PROTOCOL_IP1_NXT_COMPARATOR_IP1_NXT_PROTOCOL(value);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR, &value));
        ip2_nxt_comp = VTSS_X_ANA_IP2_NXT_PROTOCOL_IP2_NXT_COMPARATOR_IP2_NXT_COMPARATOR(value);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR, &value));
        mpls_nxt_comp = VTSS_X_ANA_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR_MPLS_NXT_COMPARATOR(value);

        for (action_ct = 0; action_ct < 6; action_ct++) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(action_ct), &value));
            cf_offset = VTSS_X_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(value);
            if (cf_offset) {
                break;
            }
        }
    } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A ||
               eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {

        if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_A_ETH1_NXT_PROTOCOL_A, &value));
            eth1_nxt_comp = VTSS_X_ANA_ETH1_NXT_PROTOCOL_ETH1_NXT_PROTOCOL_ETH1_NXT_COMPARATOR(value);
        } else if (eng_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B, &value));
            eth1_nxt_comp = VTSS_X_ANA_OAM_ETH1_NXT_PROTOCOL_B_ETH1_NXT_PROTOCOL_B_ETH1_NXT_COMPARATOR_B(value);
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A, &value));
        eth2_nxt_comp = VTSS_X_ANA_OAM_ETH2_NXT_PROTOCOL_A_ETH2_NXT_PROTOCOL_A_ETH2_NXT_COMPARATOR_A(value);
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A, &value));
        mpls_nxt_comp = VTSS_X_ANA_OAM_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A_MPLS_NXT_COMPARATOR_A(value);

        for (action_ct = 0; action_ct < 6; action_ct++) {
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_ACTION(action_ct), &value));
            cf_offset = VTSS_X_ANA_OAM_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(value);
            if (cf_offset) {
                break;
            }
        }
    } else {
        rc = VTSS_RC_ERROR;
    }

    if (4 != cf_offset) {
        *action_ptp = TRUE;
        /* Encapsulation Type is PTP */
        if (VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth1_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_IP1 == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_IP_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_IP1 == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP2 == ip1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip2_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_ETH2 == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth2_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_ETH_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_ETH2 == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP1  == eth2_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP1 == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp &&
                   6 != ach_proto) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_ETH2 == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth2_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_ETH2 == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP1  == eth2_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP1  == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp &&
                   6 == ach_proto) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP;
        }
    } else {
        *action_ptp = FALSE;
        /* Encapsulation Type is OAM */
        if (VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth1_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_OAM;
        } else if (VTSS_PHY_TS_NEXT_COMP_ETH2 == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth2_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_ETH_OAM;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_ETH2 == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == eth2_nxt_comp) {
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM;
        } else if (VTSS_PHY_TS_NEXT_COMP_MPLS == eth1_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_IP1  == mpls_nxt_comp &&
                   VTSS_PHY_TS_NEXT_COMP_PTP_OAM == ip1_nxt_comp) {

            /* OAM encapsulations doesn't contain IP header, hence no contention */
            *encap_type = VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM;
        } else {
            *encap_type = VTSS_PHY_TS_ENCAP_NONE;
            rc = VTSS_RC_ERROR;
        }
    }
    if (rc == VTSS_RC_ERROR || action_ct == 6) {
        rc = VTSS_RC_ERROR;
    }

    return rc;
}

static vtss_rc vtss_phy_ts_engine_next_comp_sync_priv(
    vtss_state_t *vtss_state,
    const BOOL                      ingress,
    const vtss_port_no_t            port_no,
    const vtss_phy_ts_blk_id_t      blk_id,
    const vtss_phy_ts_engine_t      eng_id,
    const vtss_phy_ts_encap_t       encap_type)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (encap_type) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        /* PTP allows only in PTP engine */
        /* next comp: PTP */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set IP next comparator: PTP */
        rc = vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set IP1 next comparator */
        rc = vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_IP2, 20);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP2 next comparator */
        rc = vtss_phy_ts_ip2_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28);
        /* signature mask config */
        if (rc == VTSS_RC_OK && ingress == FALSE) {
            rc = vtss_phy_ts_ip2_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        /* No OAM engine for this encap type */
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7);
        if (rc != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        /* No OAM engine for this encap type */
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set IP1 next comparator */
        rc = vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_IP1);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 next comparator */
        rc = vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x88F7);
        if (rc != VTSS_RC_OK) {
            break;
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_IP1, 0x0800);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set IP1 next comparator */
        rc = vtss_phy_ts_ip1_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 28);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* signature mask config */
        if (ingress == FALSE) {
            rc = vtss_phy_ts_ip1_sig_mask_set_priv(vtss_state, port_no, eng_id, blk_id);
        }
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_IP1);
        if (rc != VTSS_RC_OK) {
            break;
        }
        /* set ACH next comparator */
        rc = vtss_phy_ts_ach_next_comp_set_priv(vtss_state, port_no, blk_id, encap_type,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_OAM:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2, 0x88E7);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_ETH2);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set eth2 next comparator */
        rc = vtss_phy_ts_eth2_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_PTP_OAM, 0x8902);
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        /* set eth1 next comparator */
        rc = vtss_phy_ts_eth1_next_comp_etype_set_priv(vtss_state, port_no, blk_id,
                                                       eng_id, VTSS_PHY_TS_NEXT_COMP_MPLS, 0x8847);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set MPLS next comparator */
        rc = vtss_phy_ts_mpls_next_comp_set_priv(vtss_state, port_no, blk_id,
                                                 eng_id, VTSS_PHY_TS_NEXT_COMP_IP1);
        if (rc != VTSS_RC_OK) {
            break;
        }

        /* set ACH next comparator */
        rc = vtss_phy_ts_ach_next_comp_set_priv(vtss_state, port_no, blk_id, encap_type,
                                                VTSS_PHY_TS_NEXT_COMP_PTP_OAM);
        break;
    default:
        VTSS_N("Port(%u) engine_init:: invalid encapsulation type: %u", (u32)port_no, encap_type);
        break;
    }
    return rc;
}

/* PTP action is applicable to PTP engine which is handled by action_ptp field */
/* Note:: Please update the changes here also while updating the vtss_phy_ts_engine_ptp_action_flow_conf_priv
 */
#define VTSS_PHY_TS_COMP_CSR(hwv, sv, bm, off)    \
                            if (hwv != sv) { bm |= 1 << off; }

#define VTSS_PHY_TS_ZERO_CTRL_BM 0
#define VTSS_PHY_TS_ACT1_CTRL_BM 1
#define VTSS_PHY_TS_ACT2_CTRL_BM 2

static vtss_rc vtss_phy_ts_engine_ptp_action_flow_conf_sync_priv(
    vtss_state_t *vtss_state,
    const vtss_port_no_t                port_no,
    const vtss_phy_ts_blk_id_t          blk_id,
    const u8                            flow_index,
    const vtss_phy_ts_ptp_action_cmd_t  cmd,
    vtss_phy_ts_ptp_action_asym_t       asym,
    const vtss_phy_ts_ptp_msg_type_t    msg_type,
    BOOL *hw_update_reqd)
{
    u32 value, reg_val;
    u8  rx_ts_pos = 0;
    u8  changed = 0;

    if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
        rx_ts_pos = 16; /* timestamp at reserved bytes */
    }

    switch (cmd) {
    case PTP_ACTION_CMD_NOP:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* NOP */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);
        value = 0;
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
    case PTP_ACTION_CMD_SUB_ADD:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(rx_ts_pos); /* Ingress stored timestamp location */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* sub_add */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);

        /* action_2 setting */
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(8); /* nothing to write */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(8); /* for CF */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);

        /* clear frame bytes */
        value = VTSS_F_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_OFFSET(rx_ts_pos); /* stored timestamp in reserved btes should be clear */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL_PTP_ZERO_FIELD_BYTE_CNT(4); /* 4 bytes stored timestamp */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ZERO_FIELD_CTL(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ZERO_CTRL_BM);
        break;
    case PTP_ACTION_CMD_WRITE_1588:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_1588 */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);

        /* action_2 setting */
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(34); /* origintimestamp offset */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(10); /* full timestamp */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
    case PTP_ACTION_CMD_WRITE_NS:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns */
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
        }
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);
        /* action_2 setting */
        if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
        } else {
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
    case PTP_ACTION_CMD_WRITE_NS_P2P:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(cmd); /* write_ns_p2p */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);

        /* action_2 setting */
        if (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos == VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP) {
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(rx_ts_pos); /* reserved bytes offset */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(4); /* rsvd bytes length */
        } else {
            value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no use */
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0xE); /* Append at end */
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
    case PTP_ACTION_CMD_SAVE_IN_TS_FIFO:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SAVE_LOCAL_TIME; /* save in FIFO */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
        } else {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP); /* need to save in FIFO only, no write */
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);

        /* action_2 setting */
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0); /* no rewrite */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
#ifdef VTSS_FEATURE_PTP_DELAY_COMP_ENGINE
    case PTP_ACTION_CMD_DCE:
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_TIME_STRG_FIELD_OFFSET(8);
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_CORR_FIELD_OFFSET(8);
        if (asym == PTP_ACTION_ASYM_ADD) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_ADD_DELAY_ASYM_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
        } else if (asym == PTP_ACTION_ASYM_SUB) {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_SUB_DELAY_ASYM_ENA;
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_WRITE_1588); /* To write CF, cmd NOP not work */
        } else {
            value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_PTP_COMMAND(PTP_ACTION_CMD_NOP);
        }
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT1_CTRL_BM);

        /* action_2 setting */
        value = VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_OFFSET(0x22); /* no rewrite */
        value |= VTSS_F_ANA_PTP_FLOW_PTP_ACTION_2_PTP_REWRITE_BYTES(0); /* no rewrite */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_ACTION_2(flow_index), &reg_val));
        VTSS_PHY_TS_COMP_CSR(reg_val, value, changed, VTSS_PHY_TS_ACT2_CTRL_BM);
        break;
#endif /* VTSS_FEATURE_PTP_DELAY_COMP_ENGINE */
    default:
        break;
    }

    if (changed) {
        *hw_update_reqd = TRUE;
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

typedef struct ptp_action_flow_conf_s {
    vtss_phy_ts_ptp_action_cmd_t    cmd;
    vtss_phy_ts_ptp_action_asym_t   asym;
} ptp_action_flow_conf_t;

static ptp_action_flow_conf_t ptp_action_flow_ingr_conf_TC_A[PTP_MSG_TYPE_PDELAY_RESP + 1]
[VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP + 1]
[VTSS_PHY_TS_PTP_DELAYM_E2E + 1] = {
    /* Sync Message */
    {   {   {PTP_ACTION_CMD_WRITE_NS_P2P, PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) BC1, P2P*/
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC2, E2E */
        {   {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) TC1, P2P*/
            {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) TC1, E2E*/
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},  /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    },/* write(RX_timestamp,  Reserved) TC2, E2E */

    /* Delay_Req */
    {   {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP BC1, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) BC1, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP BC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) BC2, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP TC1, P2P */
            {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) TC1, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */

    /* PDelay_Req */
    {   {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) BC1, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) BC2, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC1, P2P */
            {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) TC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */

    /* PDelay_Resp */
    {   {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},  /* write(RX_timestamp,  Reserved) BC1, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) Bc2, P2P*/
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField)  TC1, P2P */
            {PTP_ACTION_CMD_SUB,          PTP_ACTION_ASYM_ADD}
        },   /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField)  TC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */
};


static ptp_action_flow_conf_t ptp_action_flow_ingr_conf_TC_B[PTP_MSG_TYPE_PDELAY_RESP + 1]
[VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP + 1]
[VTSS_PHY_TS_PTP_DELAYM_E2E + 1] = {
    /* Sync Message */
    {   {   {PTP_ACTION_CMD_WRITE_NS_P2P, PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) BC1, P2P*/
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) BC2, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS_P2P, PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved), add(PathDelay+Asymmetry, correctionField) TC1, P2P*/
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },  /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField) TC1, E2E*/
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},  /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    },/* write(RX_timestamp,  Reserved) TC2, E2E */

    /* Delay_Req */
    {   {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP BC1, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) BC1, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP BC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) BC2, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP TC1, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) TC1, E2E */
        {   {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE},   /* NOP TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */

    /* PDelay_Req */
    {   {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) BC1, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) BC2, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC1, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        },  /* write(RX_timestamp,  Reserved) TC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */

    /* PDelay_Resp */
    {   {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},  /* write(RX_timestamp,  Reserved),add (Asymmetry, correctionField) BC1, P2P */
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},   /* write(RX_timestamp,  Reserved),add (Asymmetry, correctionField) Bc2, P2P*/
            {PTP_ACTION_CMD_NOP,          PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD},    /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField)  TC1, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_ADD}
        },   /* write(RX_timestamp,  Reserved), add(Asymmetry, correctionField)  TC1, E2E */
        {   {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE},   /* write(RX_timestamp,  Reserved) TC2, P2P */
            {PTP_ACTION_CMD_WRITE_NS,     PTP_ACTION_ASYM_NONE}
        }
    }, /* write(RX_timestamp,  Reserved) TC2, E2E */
};


static ptp_action_flow_conf_t ptp_action_flow_egr_conf_TC_A[PTP_MSG_TYPE_PDELAY_RESP + 1]
[VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP + 1]
[VTSS_PHY_TS_PTP_DELAYM_E2E + 1] = {
    /* Sync Message */
    {   {   {PTP_ACTION_CMD_WRITE_1588,      PTP_ACTION_ASYM_NONE},   /* write(TX_timestamp, originTimestamp) BC1, P2P */
            {PTP_ACTION_CMD_WRITE_1588,      PTP_ACTION_ASYM_NONE}
        },  /* write(TX_timestamp, originTimestamp) BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp, TXFifo) BC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        },  /* save(TX_timestamp, TXFifo) BC2, E2E */
        {   {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, P2P */
            {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_NONE}
        },  /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp, TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo) TC2, E2E */

    /* Delay_Req */
    {   {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP BC1, P2P */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB}
        },   /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) BC1, E2E */
#else
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) BC1, E2E */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP BC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        },  /* save(TX_timestamp, TXFiFo) BC2, E2E*/
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP TC1, P2P */
            {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo) TC2, E2E */

    /* PDelay_Req */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
    {   {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB},    /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) BC1, P2P */
#else
    {   {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB},    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo)  BC2, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB},    /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) TC1, P2P */
#else
        {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB},    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, P2P */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
            {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo)  TC2, E2E */

    /* PDelay_Resp */
    {   {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) BC1, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) BC2, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, P2P */
            {PTP_ACTION_CMD_ADD,             PTP_ACTION_ASYM_NONE}
        },  /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo)  TC2, E2E */

};

static ptp_action_flow_conf_t ptp_action_flow_egr_conf_TC_B[PTP_MSG_TYPE_PDELAY_RESP + 1]
[VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP + 1]
[VTSS_PHY_TS_PTP_DELAYM_E2E + 1] = {
    /* Sync Message */
    {   {   {PTP_ACTION_CMD_WRITE_1588,      PTP_ACTION_ASYM_NONE},   /* write(TX_timestamp, originTimestamp) BC1, P2P */
            {PTP_ACTION_CMD_WRITE_1588,      PTP_ACTION_ASYM_NONE}
        },  /* write(TX_timestamp, originTimestamp) BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp, TXFifo) BC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        },  /* save(TX_timestamp, TXFifo) BC2, E2E */
        {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, P2P */
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE}
        },  /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp, TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo) TC2, E2E */

    /* Delay_Req */
    {   {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP BC1, P2P */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB}
        },   /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) BC1, E2E */
#else
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) BC1, E2E */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP BC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        },  /* save(TX_timestamp, TXFiFo) BC2, E2E*/
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP TC1, P2P */
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE},   /* NOP TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo) TC2, E2E */

    /* PDelay_Req */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
    {   {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB},    /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) BC1, P2P */
#else
    {   {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB},    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo)  BC2, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
#if defined(VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION)
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_SUB},    /* save(TX_timestamp, TXFiFo);sub(Asymmetry, correctionField) TC1, P2P */
#else
        {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB},    /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, P2P */
#endif /* VTSS_FEATURE_PHY_TS_ONE_STEP_TXFIFO_OPTION */
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_SUB}
        },   /* Subtract_add(TX_timestamp - Asymmetry, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo)  TC2, E2E */

    /* PDelay_Resp */
    {   {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) BC1, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) BC2, P2P */
            {PTP_ACTION_CMD_NOP,             PTP_ACTION_ASYM_NONE}
        },  /* NOP BC2, E2E */
        {   {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE},   /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, P2P */
            {PTP_ACTION_CMD_SUB_ADD,         PTP_ACTION_ASYM_NONE}
        },  /* Subtract_add(TX_timestamp, Reserved, correctionField) TC1, E2E */
        {   {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE},   /* save(TX_timestamp,  TXFiFo) TC2, P2P */
            {PTP_ACTION_CMD_SAVE_IN_TS_FIFO, PTP_ACTION_ASYM_NONE}
        }
    }, /* save(TX_timestamp, TXFiFo)  TC2, E2E */

};



static vtss_rc vtss_phy_ts_engine_ptp_action_flow_sync_priv(
    vtss_state_t *vtss_state,
    BOOL    ingress,
    const vtss_port_no_t                  port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const vtss_phy_ts_ptp_msg_type_t      msg_type,
    const u8                              flow_index,
    const vtss_phy_ts_ptp_engine_action_t *const action_conf,
    BOOL *hw_update_reqd)
{

    if (vtss_state->phy_ts_port_conf[port_no].tc_op_mode == VTSS_PHY_TS_TC_OP_MODE_A) {
        if (ingress) {
            VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_sync_priv(vtss_state, port_no,
                                                                      blk_id, flow_index,
                                                                      ptp_action_flow_ingr_conf_TC_A[msg_type][action_conf->clk_mode][action_conf->delaym_type].cmd,
                                                                      ptp_action_flow_ingr_conf_TC_A[msg_type][action_conf->clk_mode][action_conf->delaym_type].asym,
                                                                      msg_type, hw_update_reqd));
        } else {
            VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_sync_priv(vtss_state, port_no,
                                                                      blk_id, flow_index,
                                                                      ptp_action_flow_egr_conf_TC_A[msg_type][action_conf->clk_mode][action_conf->delaym_type].cmd,
                                                                      ptp_action_flow_egr_conf_TC_A[msg_type][action_conf->clk_mode][action_conf->delaym_type].asym,
                                                                      msg_type, hw_update_reqd));
        }
    } else {
        if (ingress) {
            VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_sync_priv(vtss_state, port_no,
                                                                      blk_id, flow_index,
                                                                      ptp_action_flow_ingr_conf_TC_B[msg_type][action_conf->clk_mode][action_conf->delaym_type].cmd,
                                                                      ptp_action_flow_ingr_conf_TC_B[msg_type][action_conf->clk_mode][action_conf->delaym_type].asym,
                                                                      msg_type, hw_update_reqd));
        } else {
            VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_conf_sync_priv(vtss_state, port_no,
                                                                      blk_id, flow_index,
                                                                      ptp_action_flow_egr_conf_TC_B[msg_type][action_conf->clk_mode][action_conf->delaym_type].cmd,
                                                                      ptp_action_flow_egr_conf_TC_B[msg_type][action_conf->clk_mode][action_conf->delaym_type].asym,
                                                                      msg_type, hw_update_reqd));
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_ptp_action_get_bitmask(
    u8                                    *bitmask,
    const vtss_phy_ts_ptp_engine_action_t *const action_conf)
{
    *bitmask = 1 ; /* PTP_MSG_TYPE_SYNC is common for all */
    if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
        action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        *bitmask |= 1 << PTP_MSG_TYPE_DELAY_REQ;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        *bitmask |= 1 << PTP_MSG_TYPE_DELAY_REQ;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        *bitmask |= 1 << PTP_MSG_TYPE_DELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        *bitmask |= 1 << PTP_MSG_TYPE_DELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_REQ;
        *bitmask |= 1 << PTP_MSG_TYPE_PDELAY_RESP;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_ptp_action_sync_priv(
    vtss_state_t *vtss_state,
    BOOL ingress,
    const vtss_port_no_t                  port_no,
    const vtss_phy_ts_blk_id_t            blk_id,
    const u8                              flow_index,
    const vtss_phy_ts_ptp_engine_action_t *const action_conf,
    BOOL *hw_update_reqd)
{

    if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
        action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_DELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 2), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_DELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 2), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_DELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 2), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 3), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 2), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_E2E) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_DELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 2), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 3), action_conf, hw_update_reqd));
    } else if (action_conf->clk_mode == VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP &&
               action_conf->delaym_type == VTSS_PHY_TS_PTP_DELAYM_P2P) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_SYNC, flow_index, action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_REQ, (flow_index + 1), action_conf, hw_update_reqd));
        VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_sync_priv(vtss_state, ingress,
                                                             port_no, blk_id, PTP_MSG_TYPE_PDELAY_RESP, (flow_index + 2), action_conf, hw_update_reqd));
    }
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_ts_engine_oam_action_flow_conf_extract_from_hwpriv(
    vtss_state_t *vtss_state,
    BOOL                        ingress,
    vtss_port_no_t              port_no,
    vtss_phy_ts_engine_t        engine_id,
    vtss_phy_ts_blk_id_t        blk_id,
    vtss_phy_ts_eng_conf_t      *hw_eng_conf,
    vtss_phy_ts_eng_conf_t      *sw_eng_conf,
    BOOL *hw_update_reqd)
{

    u8 flow_index;
    u8 flow_enable_ct = 0;
    u8 action_ct = 0;
    u32 temp;
    vtss_phy_ts_oam_engine_action_t   *oam_flow_conf = NULL;
    vtss_phy_ts_oam_engine_action_t   *sw_conf = NULL;
    vtss_phy_ts_oam_engine_action_t   *hw_conf = NULL;
    vtss_phy_ts_engine_action_t       *hw_action_conf = &hw_eng_conf->action_conf;
    vtss_phy_ts_engine_action_t       *sw_action_conf = &sw_eng_conf->action_conf;
    vtss_phy_ts_y1731_oam_msg_type_t  y1731_msg_type;
    BOOL is_gen2 = vtss_state->phy_ts_port_conf[port_no].is_gen2;
    hw_action_conf->action_ptp = FALSE;
    for (flow_index = 0; flow_index < 6; flow_index++) {
        u32 value;

        oam_flow_conf = &hw_action_conf->action.oam_conf[action_ct]; /* Access each of the action */

        if ((engine_id == VTSS_PHY_TS_PTP_ENGINE_ID_0) ||
            (engine_id == VTSS_PHY_TS_PTP_ENGINE_ID_1)) {

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
            if (!(value & VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA)) {
                continue;
            }
            /* Extract the flow configuration fields */
            oam_flow_conf->enable      = TRUE;
            oam_flow_conf->y1731_en    = FALSE;
            oam_flow_conf->channel_map = VTSS_X_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(value);
            oam_flow_conf->version     = 0; /* we only support version 0 */
            flow_enable_ct++;
            action_ct++;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));

            /* Based on the Flow Type (i.e) Y1731/IETF update the fields in the structure */
            if ((value & VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA) &&
                (VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_OFFSET(value) ==
                 VTSS_PHY_TS_IETF_DM_DS_FIELD_POS)) {
                /* IETF :: Two Actions(Delay Measurement)
                 *      DMM - Two-way delay measurement method                - 1 FLOW
                 *      LDM - Loss/Delay Message combined Measurement Message - 1 FLOW
                 */

                oam_flow_conf->y1731_en = FALSE;
                oam_flow_conf->oam_conf.ietf_oam_conf.ds =
                    VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(value);
                oam_flow_conf->oam_conf.ietf_oam_conf.ts_format =
                    VTSS_PHY_TS_IETF_MPLS_ACH_OAM_TS_FORMAT_PTP;
                /* For IETF MPLS OAM, The qualifiers are same for both the delay
                 * measurement while comparing the fields we should omit them.
                 */
            } else {
                /* Y1731 :: Two Actions(Delay Measurement) are possible.
                 *       1DM - (one-way delay measurement),           - 1 FLOW.
                 *       DMM - (Two-way delay measurement) DMM & DMR  - 2 FLOW
                 */
                oam_flow_conf->y1731_en = TRUE;
                if (value & VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA) {
                    oam_flow_conf->oam_conf.y1731_oam_conf.range_en = TRUE;
                    /* Range Upper */
                    temp = VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(value);
                    oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.range.upper =
                        (u8)((temp & 0xFF) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

                    /* Range Lower */
                    temp = VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(value);
                    oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.range.lower =
                        (u8)((temp & 0xFF) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

                    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
                    y1731_msg_type =
                        ((value & (0xFF << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT)) >>
                         VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT);
                } else {
                    oam_flow_conf->oam_conf.y1731_oam_conf.range_en = FALSE;
                    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
                    temp = (u32)((value & 0xFF000000) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT);

                    oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.value.val = (u8)
                                                                                 ((temp & 0xE0) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

                    y1731_msg_type = (vtss_phy_ts_y1731_oam_msg_type_t)
                                     ((value & (0xFF << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT)) >>
                                      VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT);


                    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
                    temp = (u32)(value & 0xFF000000) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT;
                    oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.value.mask = (u8)
                                                                                  ((temp & 0xE0) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);
                }
                if (y1731_msg_type == Y1731_OAM_MSG_TYPE_1DM) {
                    /* Delay measurement Type is 1DM */
                    oam_flow_conf->oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM;
                } else if (y1731_msg_type == Y1731_OAM_MSG_TYPE_DMM) {
                    /* Delay measurement Type is DMM */
                    oam_flow_conf->oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM;
                    /* If this message type is DMM next flow should be DMR
                     * so ignore extracting the fields in next flow index
                     * and update the action config.
                     */
                    flow_index++;
                }
            }
        } else if ((engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) ||
                   (engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B)) {
            /* Engine 2A and 2B Doesn't support IETF_OAM */
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
            if (is_gen2 != TRUE && engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
                /* Check if Action flow is enabled */
                if (!(value & VTSS_F_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA)) {
                    continue;
                }
                temp = VTSS_X_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_NXT_PROT_GRP_MASK(value);
                /* check if the action is configured for the engine_id */
                if (((engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) && !(temp & 0x1)) ||
                    ((engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) && !(temp & 0x2))) {
                    /* This action flow is not configured for engine_id; hence continue */
                    continue;
                }
            }
            /* Extract the flow configuration fields */
            oam_flow_conf->enable      = TRUE;
            oam_flow_conf->y1731_en    = TRUE; /* Always TRUE */
            oam_flow_conf->channel_map = VTSS_X_ANA_OAM_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(value);
            oam_flow_conf->version     = 0; /* we only support version 0 */
            flow_enable_ct++;
            action_ct++;
            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
            if (value & VTSS_F_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA) {
                /* Range is Enabled */
                oam_flow_conf->oam_conf.y1731_oam_conf.range_en = TRUE;

                /* Range Upper */
                temp = VTSS_X_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(value);
                oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.range.upper =
                    (u8)((temp & 0xFF) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

                /* Range Lower */
                temp = VTSS_X_ANA_OAM_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(value);
                oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.range.lower =
                    (u8)((temp & 0xFF) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));

                y1731_msg_type = ((value & (0xFF << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT)) >>
                                  VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT);

            } else {
                /* Range is Disabled */
                oam_flow_conf->oam_conf.y1731_oam_conf.range_en = FALSE;
                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
                temp = (u32)(value & 0xFF000000) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT;
                oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.value.val =
                    (temp & 0xE0) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT;

                y1731_msg_type = ((value & (0xFF << VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT)) >>
                                  VTSS_PHY_TS_Y1731_OAM_DM_MSG_TYPE_SHIFT_CNT);

                VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_OAM_PTP_FLOW_PTP_FLOW_MASK_UPPER(flow_index), &value));
                temp = (u32)(value & 0xFF000000) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_VER_SHIFT_CNT;
                oam_flow_conf->oam_conf.y1731_oam_conf.meg_level.value.mask = (u8)
                                                                              ((temp & 0xE0) >> VTSS_PHY_TS_Y1731_OAM_DM_MEG_LVL_SHIFT_CNT);

            }
            if (y1731_msg_type == Y1731_OAM_MSG_TYPE_1DM) {
                /* Delay measurement Type is 1DM */
                oam_flow_conf->oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM;
            } else if (y1731_msg_type == Y1731_OAM_MSG_TYPE_DMM) {
                /* Delay measurement Type is DMM */
                oam_flow_conf->oam_conf.y1731_oam_conf.delaym_type = VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM;
                /* If this message type is DMM next flow should be DMR
                 * so ignore extracting the fields in next flow index
                 * and update the action config.
                 */
                flow_index++;
            }
        }
    }

    if (!flow_enable_ct) {
        /* Flows are not enabled; Mismatch in the configuration */
        *hw_update_reqd = TRUE;
        return VTSS_RC_ERROR;
    }

    /* Compare the vtss_state fields with extracted OAM comparator */
    *hw_update_reqd = (hw_action_conf->action_ptp != sw_action_conf->action_ptp) ? TRUE : FALSE;
    if (*hw_update_reqd)  {
        return VTSS_RC_ERROR;
    }

    for (flow_index = 0; flow_index < 6; flow_index++) {
        hw_conf = &hw_action_conf->action.oam_conf[flow_index];
        sw_conf = &sw_action_conf->action.oam_conf[flow_index];
        if (!hw_conf->enable && !sw_conf->enable) {
            continue;
        }
        *hw_update_reqd = ((hw_conf->enable      != sw_conf->enable) ||
                           (hw_conf->y1731_en    != sw_conf->y1731_en) ||
                           (hw_conf->channel_map != sw_conf->channel_map)) ? TRUE : FALSE;

        if (*hw_update_reqd)  {
            VTSS_E("Config Mismatch:OAM Flow mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                   (unsigned int)port_no, engine_id, flow_index);
            break;
        }

        if (hw_conf->y1731_en) {
            if ((hw_conf->oam_conf.y1731_oam_conf.range_en    != sw_conf->oam_conf.y1731_oam_conf.range_en) ||
                (hw_conf->oam_conf.y1731_oam_conf.delaym_type != sw_conf->oam_conf.y1731_oam_conf.delaym_type)) {
                *hw_update_reqd = TRUE;

                VTSS_E("Config Mismatch:Y1731 Flow parameters mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, engine_id, flow_index);
                break;
            }
            if (sw_conf->oam_conf.y1731_oam_conf.range_en) {
                if ((hw_conf->oam_conf.y1731_oam_conf.meg_level.range.upper != sw_conf->oam_conf.y1731_oam_conf.meg_level.range.upper) ||
                    (hw_conf->oam_conf.y1731_oam_conf.meg_level.range.lower != sw_conf->oam_conf.y1731_oam_conf.meg_level.range.lower)) {
                    *hw_update_reqd = TRUE;
                    VTSS_E("Config Mismatch:Y1731 Meg Level Range mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                           (unsigned int)port_no, engine_id, flow_index);

                    break;
                }
            } else {
                if ((hw_conf->oam_conf.y1731_oam_conf.meg_level.value.val  != sw_conf->oam_conf.y1731_oam_conf.meg_level.value.val) ||
                    (hw_conf->oam_conf.y1731_oam_conf.meg_level.value.mask != sw_conf->oam_conf.y1731_oam_conf.meg_level.value.mask)) {
                    VTSS_E("Config Mismatch:Y1731 Meg Level Value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                           (unsigned int)port_no, engine_id, flow_index);
                    *hw_update_reqd = TRUE;
                    break;
                }

            }

        } else {
            /* IETF, hw rules are same for both the flows so compare the ds field */
            *hw_update_reqd = (hw_conf->oam_conf.ietf_oam_conf.ds != sw_conf->oam_conf.ietf_oam_conf.ds) ? TRUE : FALSE;
            if (*hw_update_reqd) {
                VTSS_E("Config Mismatch:IETF DS Value mismatch on port - [%d] Engine_ID - [%d] Flow_Id - [%d]\n\r",
                       (unsigned int)port_no, engine_id, flow_index);

            }
        }
        if (*hw_update_reqd) {
            break;
        }
    }

    if (*hw_update_reqd) {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_ptp_action_flow_conf_extract_from_hwpriv(
    vtss_state_t *vtss_state,
    BOOL                        ingress,
    vtss_port_no_t              port_no,
    vtss_phy_ts_engine_t        engine_id,
    vtss_phy_ts_blk_id_t        blk_id,
    vtss_phy_ts_eng_conf_t      *hw_eng_conf,
    vtss_phy_ts_eng_conf_t      *sw_eng_conf,
    BOOL *hw_update_reqd)
{
    BOOL bool1 = FALSE, bool2 = FALSE, bool3 = FALSE, bool4 = FALSE;
    u8 flow_index;
    u8 no_of_sync;
    u8 channel_map[6] = {0};
    BOOL flow_enable[6] = {0};
    u8 flow_enable_ct = 0;
    u8 eng_ena_count = 0;
    u8 sync_index[2] = {0};
    u8 actbitmask[2] = {0}, state_actbitmask[2] = {0};
    u32 count = 0;
    vtss_phy_ts_ptp_msg_type_t msg_type[6];
    vtss_phy_ts_ptp_conf_t     ptp_action_flow[6] = {{0}};
    vtss_phy_ts_engine_action_t *sw_action_conf = &sw_eng_conf->action_conf;
    vtss_phy_ts_engine_action_t *hw_action_conf = &hw_eng_conf->action_conf;

    /* PTP is not supported on Engine 2A and 2B */
    /* Read through all the action flows in the engine.
     * flows with same configuration belong to same PTP action.
     */
    no_of_sync = 0;
    for (flow_index = 0; flow_index < 6; flow_index++) {
        u32 value;

        /* flow enable */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_ENA(flow_index), &value));
        if (!(value & VTSS_F_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_FLOW_ENA)) {
            continue;
        }
        flow_enable[flow_index] = TRUE;
        flow_enable_ct++;
        channel_map[flow_index] = VTSS_X_ANA_PTP_FLOW_PTP_FLOW_ENA_PTP_CHANNEL_MASK(value);

        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_DOMAIN_RANGE(flow_index), &value));
        if (value & VTSS_F_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_ENA) {
            ptp_action_flow[flow_index].range_en = TRUE;
            ptp_action_flow[flow_index].domain.range.upper =
                VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_UPPER(value);
            ptp_action_flow[flow_index].domain.range.lower =
                VTSS_X_ANA_PTP_FLOW_PTP_DOMAIN_RANGE_PTP_DOMAIN_RANGE_LOWER(value);
        } else {

            ptp_action_flow[flow_index].range_en = FALSE;

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_LOWER(flow_index), &value));
            ptp_action_flow[flow_index].domain.value.val = (u8)((value & 0xFF000000) >> 24);

            VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MASK_LOWER(flow_index), &value));
            ptp_action_flow[flow_index].domain.value.mask = (u8)((value & 0xFF000000) >> 24);

        }

        /* get message type from flow match lower */
        /* PTP_FLOW_MATCH_UPPER: msg type to msg length with msg type in MS bytes
           PTP_FLOW_MATCH_LOWER: domain number which will be MB byte */
        VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, blk_id, VTSS_ANA_PTP_FLOW_PTP_FLOW_MATCH_UPPER(flow_index), &value));
        msg_type[flow_index] = (vtss_phy_ts_ptp_msg_type_t)((value & 0x0F000000) >> 24);
        if (PTP_MSG_TYPE_SYNC == msg_type[flow_index]) {
            sync_index[no_of_sync] = flow_index;
            no_of_sync++;
            count = 0;
            actbitmask[no_of_sync] = 1;
        }
        actbitmask[no_of_sync - 1] |= 1 << msg_type[flow_index];
        if (!no_of_sync) {
            break;
        }
    }

    if ((!flow_enable_ct) || (!no_of_sync && flow_enable_ct)) {
        return VTSS_RC_ERROR;
    }

    if (sw_action_conf->action.ptp_conf[0].enable) {
        eng_ena_count++;
    }

    if (sw_action_conf->action.ptp_conf[1].enable) {
        eng_ena_count++;
    }

    if (eng_ena_count != no_of_sync) {
        *hw_update_reqd = TRUE;
        VTSS_E("Config Mismatch:PTP Action mismatch on port - [%d] Engine_ID - [%d] \n\r",
               (unsigned int)port_no, engine_id);
        return VTSS_RC_ERROR;
    }
    /* Always engine allocation starts from the begining */
    for (count = 0; count < 2; count++) {
        VTSS_RC(vtss_phy_ts_engine_ptp_action_get_bitmask(&state_actbitmask[count], &sw_action_conf->action.ptp_conf[count]));
    }

    if (actbitmask[0] == state_actbitmask[0]) {
        hw_action_conf->action_ptp = TRUE;
        hw_action_conf->action.ptp_conf[0].enable            =  flow_enable[sync_index[0]];
        hw_action_conf->action.ptp_conf[0].channel_map       =  channel_map[sync_index[0]];
        hw_action_conf->action.ptp_conf[0].ptp_conf.range_en =  ptp_action_flow[sync_index[0]].range_en;

        if (hw_action_conf->action.ptp_conf[0].enable) {
            if (hw_action_conf->action.ptp_conf[0].ptp_conf.range_en) {
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.range.upper = ptp_action_flow[sync_index[0]].domain.range.upper;
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.range.lower = ptp_action_flow[sync_index[0]].domain.range.lower;
            } else {
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.value.val  = ptp_action_flow[sync_index[0]].domain.value.val;
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.value.mask = ptp_action_flow[sync_index[0]].domain.value.mask;
            }
            if (vtss_phy_ts_engine_ptp_action_sync_priv(vtss_state, ingress, port_no, blk_id, sync_index[0],
                                                        &sw_action_conf->action.ptp_conf[0], hw_update_reqd) == VTSS_RC_OK) {
                bool1 = TRUE;
                hw_action_conf->action.ptp_conf[0].clk_mode    = sw_action_conf->action.ptp_conf[0].clk_mode;
                hw_action_conf->action.ptp_conf[0].delaym_type = sw_action_conf->action.ptp_conf[0].delaym_type;
            }
        }
    }
    if (actbitmask[0] == state_actbitmask[1]) {
        hw_action_conf->action_ptp = TRUE;
        hw_action_conf->action.ptp_conf[0].enable            =  flow_enable[sync_index[1]];
        if (hw_action_conf->action.ptp_conf[0].enable) {
            hw_action_conf->action.ptp_conf[0].channel_map       =  channel_map[sync_index[1]];
            hw_action_conf->action.ptp_conf[0].ptp_conf.range_en =  ptp_action_flow[sync_index[1]].range_en;
            if (hw_action_conf->action.ptp_conf[0].ptp_conf.range_en) {
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.range.upper = ptp_action_flow[sync_index[1]].domain.range.upper;
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.range.lower = ptp_action_flow[sync_index[1]].domain.range.lower;
            } else {
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.value.val  = ptp_action_flow[sync_index[1]].domain.value.val;
                hw_action_conf->action.ptp_conf[0].ptp_conf.domain.value.mask = ptp_action_flow[sync_index[1]].domain.value.mask;
            }
            if (vtss_phy_ts_engine_ptp_action_sync_priv(vtss_state, ingress, port_no, blk_id, sync_index[0],
                                                        &sw_action_conf->action.ptp_conf[1], hw_update_reqd) == VTSS_RC_OK) {
                bool2 = TRUE;
                hw_action_conf->action.ptp_conf[0].clk_mode    = sw_action_conf->action.ptp_conf[1].clk_mode;
                hw_action_conf->action.ptp_conf[0].delaym_type = sw_action_conf->action.ptp_conf[1].delaym_type;
            }
        }
    }
    if (actbitmask[1] == state_actbitmask[0]) {
        hw_action_conf->action_ptp = TRUE;
        hw_action_conf->action.ptp_conf[1].enable            =  flow_enable[sync_index[0]];
        if (hw_action_conf->action.ptp_conf[1].enable) {
            hw_action_conf->action.ptp_conf[1].channel_map       =  channel_map[sync_index[0]];
            hw_action_conf->action.ptp_conf[1].ptp_conf.range_en =  ptp_action_flow[sync_index[0]].range_en;
            if (hw_action_conf->action.ptp_conf[1].ptp_conf.range_en) {
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.range.upper = ptp_action_flow[sync_index[0]].domain.range.upper;
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.range.lower = ptp_action_flow[sync_index[0]].domain.range.lower;
            } else {
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.value.val  = ptp_action_flow[sync_index[0]].domain.value.val;
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.value.mask = ptp_action_flow[sync_index[0]].domain.value.mask;
            }
            if (vtss_phy_ts_engine_ptp_action_sync_priv(vtss_state, ingress, port_no, blk_id, sync_index[1],
                                                        &sw_action_conf->action.ptp_conf[0], hw_update_reqd) == VTSS_RC_OK) {
                bool3 = TRUE;
                hw_action_conf->action.ptp_conf[1].clk_mode    = sw_action_conf->action.ptp_conf[0].clk_mode;
                hw_action_conf->action.ptp_conf[1].delaym_type = sw_action_conf->action.ptp_conf[0].delaym_type;
            }
        }
    }
    if (actbitmask[1] == state_actbitmask[1]) {
        hw_action_conf->action_ptp = TRUE;
        hw_action_conf->action.ptp_conf[1].enable            =  flow_enable[sync_index[1]];
        if (hw_action_conf->action.ptp_conf[1].enable) {
            hw_action_conf->action.ptp_conf[1].channel_map       =  channel_map[sync_index[1]];
            hw_action_conf->action.ptp_conf[1].ptp_conf.range_en =  ptp_action_flow[sync_index[1]].range_en;
            if (hw_action_conf->action.ptp_conf[1].ptp_conf.range_en) {
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.range.upper = ptp_action_flow[sync_index[1]].domain.range.upper;
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.range.lower = ptp_action_flow[sync_index[1]].domain.range.lower;
            } else {
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.value.val  = ptp_action_flow[sync_index[1]].domain.value.val;
                hw_action_conf->action.ptp_conf[1].ptp_conf.domain.value.mask = ptp_action_flow[sync_index[1]].domain.value.mask;
            }

            if (vtss_phy_ts_engine_ptp_action_sync_priv(vtss_state, ingress, port_no, blk_id, sync_index[1],
                                                        &sw_action_conf->action.ptp_conf[1], hw_update_reqd) == VTSS_RC_OK) {

                bool4 = TRUE;
                hw_action_conf->action.ptp_conf[1].clk_mode    = sw_action_conf->action.ptp_conf[1].clk_mode;
                hw_action_conf->action.ptp_conf[1].delaym_type = sw_action_conf->action.ptp_conf[1].delaym_type;
            }
        }
    }

    if (bool1 || bool2 || bool3 || bool4) {
        *hw_update_reqd = FALSE;
    }

    if (hw_action_conf->action_ptp != sw_action_conf->action_ptp) {
        VTSS_E("Config Mismatch:PTP Action Type mismatch on port - [%d] Engine_ID - [%d] \n\r",
               (unsigned int)port_no, engine_id);

        *hw_update_reqd = TRUE;
        return VTSS_RC_OK;
    }
    flow_index = 2;
    do {
        flow_index--;
        if (hw_action_conf->action.ptp_conf[flow_index].enable != sw_action_conf->action.ptp_conf[flow_index].enable) {
            VTSS_E("Config Mismatch:PTP Action Enable mismatch on port - [%d] Engine_ID - [%d] Action_ID - [%d]\n",
                   (unsigned int)port_no, engine_id, flow_index);

            *hw_update_reqd = TRUE;
            break;
        }
        if (sw_action_conf->action.ptp_conf[flow_index].enable) {
            if ((hw_action_conf->action.ptp_conf[flow_index].channel_map       != sw_action_conf->action.ptp_conf[flow_index].channel_map) ||
                (hw_action_conf->action.ptp_conf[flow_index].ptp_conf.range_en != sw_action_conf->action.ptp_conf[flow_index].ptp_conf.range_en)) {
                VTSS_E("Config Mismatch:PTP Action Flow configuration mismatch on port - [%d] Engine_ID - [%d] Action_ID - [%d]\n",
                       (unsigned int)port_no, engine_id, flow_index);

                *hw_update_reqd = TRUE;
                break;
            }
            if (hw_action_conf->action.ptp_conf[flow_index].ptp_conf.range_en) {
                if ((hw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.range.upper !=
                     sw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.range.upper) ||
                    (hw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.range.lower !=
                     sw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.range.lower)) {
                    VTSS_E("Config Mismatch:PTP Action Flow domain range configuration mismatch on port - [%d] Engine_ID - [%d] Action_ID - [%d]\n",
                           (unsigned int)port_no, engine_id, flow_index);

                    *hw_update_reqd = TRUE;
                    break;
                }
            } else {
                if ((hw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.value.val  !=
                     sw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.value.val) ||
                    (hw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.value.mask !=
                     sw_action_conf->action.ptp_conf[flow_index].ptp_conf.domain.value.mask)) {
                    VTSS_E("Config Mismatch:PTP Action Flow domain Value configuration mismatch on port - [%d] Engine_ID - [%d] Action_ID - [%d]\n",
                           (unsigned int)port_no, engine_id, flow_index);

                    *hw_update_reqd = TRUE;
                    break;
                }
            }
            if ((hw_action_conf->action.ptp_conf[flow_index].clk_mode    != sw_action_conf->action.ptp_conf[flow_index].clk_mode) ||
                (hw_action_conf->action.ptp_conf[flow_index].delaym_type != sw_action_conf->action.ptp_conf[flow_index].delaym_type)) {
                VTSS_E("Config Mismatch:PTP Action Clock Type and Delay Measurement configuration mismatch on port - [%d] Engine_ID - [%d] Action_ID - [%d]\n",
                       (unsigned int)port_no, engine_id, flow_index);

                *hw_update_reqd = TRUE;
                break;
            }

        }
    } while (flow_index);

    if (*hw_update_reqd) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_action_flow_conf_check_hwpriv(
    vtss_state_t *vtss_state,
    BOOL                        ingress,
    vtss_port_no_t              port_no,
    vtss_phy_ts_engine_t        eng_id,
    vtss_phy_ts_blk_id_t        blk_id,
    BOOL                        action_ptp,
    vtss_phy_ts_eng_conf_t      *hw_eng_conf,
    vtss_phy_ts_eng_conf_t      *sw_eng_conf,
    BOOL *hw_update_reqd)
{
    u8      flow_idx;
    BOOL    hw_sync_error = FALSE;
    vtss_rc rc = VTSS_RC_OK;

    if (action_ptp) {
        rc = vtss_phy_ts_engine_ptp_action_flow_conf_extract_from_hwpriv(vtss_state, ingress, port_no, eng_id, blk_id, hw_eng_conf, sw_eng_conf, &hw_sync_error);
    } else {
        rc = vtss_phy_ts_engine_oam_action_flow_conf_extract_from_hwpriv(vtss_state, ingress, port_no, eng_id, blk_id, hw_eng_conf, sw_eng_conf, &hw_sync_error);
    }

    *hw_update_reqd = hw_sync_error;
    if (hw_sync_error) {
        /* flow mismatch, delete all the PTP/OAM flows */
        for (flow_idx = 0; flow_idx < 6; flow_idx++) {
            /* Engine should be disabled before configuring the registers */
            VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, FALSE));
            if (action_ptp) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, flow_idx));
            } else {
                VTSS_RC(vtss_phy_ts_engine_oam_action_flow_delete_priv(vtss_state, port_no, blk_id, eng_id, flow_idx));
            }
            /* Clear the action_flow_map */
            sw_eng_conf->action_flow_map[flow_idx] = 0;
            VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, eng_id, TRUE));
        }
    }

    return rc;
}


static vtss_rc vtss_phy_ts_engine_action_hwupdate(
    vtss_state_t *vtss_state,
    BOOL ingress,
    BOOL action_ptp,
    vtss_phy_ts_engine_t    engine_id,
    vtss_phy_ts_encap_t     encap_type,
    vtss_port_no_t          port_no,
    vtss_port_no_t          base_port_no,
    vtss_phy_ts_blk_id_t    blk_id,
    vtss_phy_ts_eng_conf_t *sw_eng_conf,
    vtss_phy_ts_eng_conf_t *hw_eng_conf)
{
    u8   action_idx;
    vtss_phy_ts_engine_action_t    *sw_action_conf = NULL;

    sw_action_conf = &sw_eng_conf->action_conf;
    /* Engine should be disabled before configuring the registers */
    VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, FALSE));

    if (action_ptp) {
        vtss_phy_ts_ptp_engine_action_t *ptp_action_conf = NULL;

        for (action_idx = 0; action_idx < VTSS_PHY_TS_MAX_ACTION_PTP; action_idx++) {
            ptp_action_conf = &sw_action_conf->action.ptp_conf[action_idx];
            if (ptp_action_conf->enable) {
                VTSS_RC(vtss_phy_ts_engine_ptp_action_add_priv(vtss_state, ingress, port_no, base_port_no,
                                                               blk_id, engine_id, hw_eng_conf, ptp_action_conf,
                                                               action_idx));
            }
        }
    } else {
        vtss_phy_ts_oam_engine_action_t *oam_action_conf = NULL;
        for (action_idx = 0; action_idx < VTSS_PHY_TS_MAX_ACTION_OAM; action_idx++) {
            oam_action_conf = &sw_action_conf->action.oam_conf[action_idx];
            if (oam_action_conf->enable) {
                if (oam_action_conf->y1731_en) {
                    VTSS_RC(vtss_phy_ts_engine_y1731_oam_action_add_priv(vtss_state, ingress, port_no,
                                                                         base_port_no, blk_id, engine_id, hw_eng_conf,
                                                                         oam_action_conf, action_idx));
                } else {
                    VTSS_RC(vtss_phy_ts_engine_ietf_oam_action_add_priv(vtss_state, ingress, port_no,
                                                                        base_port_no, blk_id, hw_eng_conf,
                                                                        oam_action_conf, action_idx));

                }
            }
        }
    }
    /* Engine should be enabled after configuring the Action registers */
    VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, TRUE));
    return VTSS_RC_OK;
}




static vtss_rc vtss_phy_ts_engine_flow_hwsync(
    vtss_state_t *vtss_state,
    BOOL ingress,
    BOOL action_ptp,
    BOOL *engine_update_required,
    BOOL *hw_action_update_reqd,
    vtss_phy_ts_engine_t    engine_id,
    vtss_phy_ts_encap_t     encap_type,
    vtss_port_no_t          port_no,
    vtss_port_no_t          base_port_no,
    vtss_phy_ts_blk_id_t    blk_id,
    vtss_phy_ts_eng_conf_t *sw_eng_conf,
    vtss_phy_ts_eng_conf_t *hw_eng_conf)
{
    vtss_ts_engine_parm_t       eng_parm = {port_no,
                                            base_port_no,
                                            blk_id,
                                            engine_id,
                                            encap_type,
                                            sw_eng_conf->flow_st_index,
                                            sw_eng_conf->flow_end_index
                                           };
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf = &sw_eng_conf->flow_conf;

    VTSS_RC(vtss_phy_ts_engine_flow_conf_sync_priv(vtss_state, ingress, &eng_parm,
                                                   hw_eng_conf, sw_flow_conf, engine_update_required));
    VTSS_RC(vtss_phy_ts_engine_action_flow_conf_check_hwpriv(vtss_state, ingress, port_no, engine_id,
                                                             blk_id, action_ptp, hw_eng_conf, sw_eng_conf, hw_action_update_reqd));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_engine_flow_hwupdate(
    vtss_state_t *vtss_state,
    BOOL ingress,
    vtss_phy_ts_engine_t    engine_id,
    vtss_phy_ts_encap_t     encap_type,
    vtss_port_no_t          port_no,
    vtss_port_no_t          base_port_no,
    vtss_phy_ts_blk_id_t    blk_id,
    vtss_phy_ts_eng_conf_t *sw_eng_conf,
    vtss_phy_ts_eng_conf_t *hw_eng_conf)
{
    vtss_ts_engine_parm_t       eng_parm = {port_no,
                                            base_port_no,
                                            blk_id,
                                            engine_id,
                                            encap_type,
                                            sw_eng_conf->flow_st_index,
                                            sw_eng_conf->flow_end_index
                                           };
    vtss_phy_ts_engine_flow_conf_t *sw_flow_conf = &sw_eng_conf->flow_conf;

    /* Engine should be disabled before configuring the registers */
    VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, FALSE));

    if (sw_eng_conf->eng_used) {
        VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, ingress, &blk_id));

        hw_eng_conf->flow_st_index           = sw_eng_conf->flow_st_index;
        hw_eng_conf->flow_end_index          = sw_eng_conf->flow_end_index;

        VTSS_RC(vtss_phy_ts_engine_flow_set_priv(vtss_state, ingress, &eng_parm, hw_eng_conf,
                                                 sw_flow_conf));
    } else {
        /* Clear the previously configured registers */
    }
    /* Engine should be enabled after configuring the Action registers */
    VTSS_RC(vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, TRUE));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_ts_port_hwsync(
    vtss_state_t *vtss_state,
    const vtss_port_no_t port_no,
    vtss_phy_ts_init_conf_t *conf)
{
    u32  value = 0;

    /* Extract the LTC clock source */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_IP_1588_LTC_LTC_CTRL, &value));
    /* get the clock source from the vtss_state
     */
    conf->clk_src = VTSS_X_PTP_IP_1588_LTC_LTC_CTRL_LTC_CLK_SEL(value);

    /* FIFO mode register */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG, &value));

    conf->tx_fifo_mode = (value & VTSS_F_PTP_TS_FIFO_SI_TS_FIFO_SI_CFG_TS_FIFO_SI_ENA) ?
                         VTSS_PHY_TS_FIFO_MODE_SPI : VTSS_PHY_TS_FIFO_MODE_NORMAL;

    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_RW_INGR_RW_CTRL, &value));
    conf->rx_ts_pos = (value & VTSS_F_PTP_INGR_IP_1588_RW_INGR_RW_CTRL_INGR_RW_REDUCE_PREAMBLE) ?
                      VTSS_PHY_TS_RX_TIMESTAMP_POS_AT_END : VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP;

    /* Time Stamp length configuration
     */
    VTSS_RC(VTSS_PHY_TS_READ_CSR(port_no, VTSS_PHY_TS_PROC_BLK_ID(0),
                                 VTSS_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL, &value));
    /* Get the Ingress timestamp length configuration
     */
    conf->rx_ts_len = (value & VTSS_F_PTP_INGR_IP_1588_TSP_INGR_TSP_CTRL_INGR_FRACT_NS_MODE) ?
                      VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT : VTSS_PHY_TS_RX_TIMESTAMP_LEN_32BIT;


    return VTSS_RC_OK;
}

#define VTSS_SYNC_RC(function) if ((rc = function) != VTSS_RC_OK) {vtss_state->sync_calling_private = FALSE; return rc;}
vtss_rc vtss_phy_ts_sync(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;
    u8   ingress_ct;
    u16  phy_type;
    BOOL clause45;
    BOOL phy_type_match = FALSE;
    BOOL ingress = FALSE;
    BOOL block_sync             = FALSE;
    BOOL generic_phy_engine_sync_reqd   = FALSE;
    BOOL phy_engine_sync_reqd   = FALSE;
    BOOL engine_enable_reqd     = FALSE;
    BOOL engine_enabled         = FALSE;
    BOOL action_ptp             = FALSE;
    BOOL engine_update_required = FALSE;
    BOOL hw_action_update_reqd  = FALSE;
    vtss_port_no_t              base_port_no;
    vtss_phy_ts_init_conf_t     init_conf = {0};
    vtss_phy_ts_oper_mode_t     oper_mode = VTSS_PHY_TS_OPER_MODE_INV;
    vtss_phy_ts_blk_id_t        blk_id;
    vtss_phy_ts_engine_t        engine_id;
    vtss_phy_ts_encap_t         encap_type = VTSS_PHY_TS_ENCAP_ETH_PTP;
    vtss_phy_ts_eng_conf_t      *sw_eng_conf = NULL;
    vtss_phy_ts_eng_conf_t      *sw_alt_eng_conf = NULL;
    vtss_phy_ts_engine_flow_match_t flow_match_mode = VTSS_PHY_TS_ENG_FLOW_MATCH_ANY;

    /* Using global enging configuration in vtss_state to reduce stack consumption */
    vtss_phy_ts_eng_conf_t      *hw_eng_conf = &vtss_state->phy_ts_state.eng_conf;

    memset(hw_eng_conf, 0, sizeof(*hw_eng_conf));
    if (vtss_phy_ts_register_access_type_get(vtss_state, port_no, &phy_type, &clause45, &oper_mode) != VTSS_RC_OK) {
        VTSS_D("Error in getting access_type port %u", port_no);
        return VTSS_RC_OK;
    }
#if defined VTSS_CHIP_CU_PHY
    if ((phy_type == VTSS_PHY_TYPE_8574) ||
        (phy_type == VTSS_PHY_TYPE_8572) ||
        (phy_type == VTSS_PHY_TYPE_8575) ||
        (phy_type == VTSS_PHY_TYPE_8582) ||
        (phy_type == VTSS_PHY_TYPE_8584) ||
        (phy_type == VTSS_PHY_TYPE_8586)) {
        phy_type_match = TRUE;
    }
#endif /* VTSS_CHIP_CU_PHY */
#if defined (VTSS_CHIP_10G_PHY)
    if ((phy_type == VTSS_PHY_TYPE_8487) ||
        (phy_type == VTSS_PHY_TYPE_8488) ||
        (phy_type == VTSS_PHY_TYPE_8489) ||
        (phy_type == VTSS_PHY_TYPE_8490) ||
        (phy_type == VTSS_PHY_TYPE_8491)) {
        phy_type_match = TRUE;
    }
#endif /* VTSS_CHIP_10G_PHY */
#if defined VTSS_CHIP_CU_PHY || defined VTSS_CHIP_10G_PHY
    if (phy_type_match == FALSE) {
        VTSS_D("Invalid PHY type %d", (u32)phy_type);
        return VTSS_RC_OK;
    }
#endif /* VTSS_CHIP_CU_PHY */

    VTSS_N("CONF_SYNC called for port_no : %d ", port_no);
    /* Check if the 1588 block is enabled */
    VTSS_RC(vtss_phy_ts_block_sync(vtss_state, port_no, &block_sync));

    /* PHY_TS init is done */
    if (!vtss_state->phy_ts_port_conf[port_no].port_ts_init_done) {
        return VTSS_RC_OK;
    }

    VTSS_RC(vtss_phy_ts_port_hwsync(vtss_state, port_no, &init_conf));

    if ((vtss_state->phy_ts_port_conf[port_no].clk_src   != init_conf.clk_src) ||
        (vtss_state->phy_ts_port_conf[port_no].rx_ts_pos != init_conf.rx_ts_pos) ||
        (vtss_state->phy_ts_port_conf[port_no].rx_ts_len != init_conf.rx_ts_len) ||
        (vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode != init_conf.tx_fifo_mode)) {
        VTSS_E("Config Mismatch :: port config mismatch");
        init_conf.clk_freq     = vtss_state->phy_ts_port_conf[port_no].clk_freq;
        init_conf.clk_src      = vtss_state->phy_ts_port_conf[port_no].clk_src;
        init_conf.rx_ts_pos    = vtss_state->phy_ts_port_conf[port_no].rx_ts_pos;
        init_conf.rx_ts_len    = vtss_state->phy_ts_port_conf[port_no].rx_ts_len;
        init_conf.tx_fifo_mode = vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode;
        init_conf.tx_ts_len    = vtss_state->phy_ts_port_conf[port_no].tx_ts_len;
        init_conf.tc_op_mode   = vtss_state->phy_ts_port_conf[port_no].tc_op_mode;
        init_conf.chk_ing_modified = vtss_state->phy_ts_port_conf[port_no].chk_ing_modified;

        VTSS_RC(vtss_phy_ts_port_init(vtss_state, port_no, &init_conf));
    }

#if defined(VTSS_CHIP_CU_PHY) && defined(VTSS_PHY_TS_SPI_CLK_THRU_PPS0)
    if ((phy_type == VTSS_PHY_TYPE_8574 || phy_type == VTSS_PHY_TYPE_8572) &&
        (vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode == VTSS_PHY_TS_FIFO_MODE_SPI)) {
        VTSS_RC(vtss_phy_ts_new_spi_mode_sync_priv(vtss_state, port_no, vtss_state->phy_ts_port_conf[port_no].new_spi_conf.enable));
    }
#endif

    /* Check if the channel is enabled */
    base_port_no = vtss_state->phy_ts_port_conf[port_no].base_port;
    VTSS_RC(vtss_phy_ts_mode_sync(vtss_state, port_no, base_port_no, &generic_phy_engine_sync_reqd));

    for (engine_id = VTSS_PHY_TS_PTP_ENGINE_ID_0; engine_id < VTSS_PHY_TS_ENGINE_ID_INVALID; engine_id++) {
        for (ingress_ct = 0; ingress_ct <= 1; ingress_ct++) {
            sw_alt_eng_conf = NULL;

            memset(hw_eng_conf, 0, sizeof(*hw_eng_conf));
            phy_engine_sync_reqd = generic_phy_engine_sync_reqd;
            engine_update_required = FALSE;
            hw_action_update_reqd  = FALSE;

            ingress = (!ingress_ct) ? TRUE : FALSE;
            VTSS_RC(vtss_phy_ts_ana_blk_id_get(engine_id, ingress, &blk_id));
            VTSS_RC(vtss_phy_ts_engine_sync_priv(vtss_state, ingress, port_no, base_port_no, engine_id,
                                                 blk_id, &engine_enabled, &flow_match_mode));
            if (vtss_phy_ts_engine_encap_conf_sync_priv(vtss_state, ingress, port_no, base_port_no,
                                                        engine_id, blk_id, &encap_type, &action_ptp) != VTSS_RC_OK) {
            }
            hw_eng_conf->eng_used           = engine_enabled;
            hw_eng_conf->flow_conf.eng_mode = engine_enabled;
            hw_eng_conf->encap_type         = encap_type;
            hw_eng_conf->flow_match_mode    = flow_match_mode;
            hw_eng_conf->flow_st_index      = 0;
            hw_eng_conf->flow_end_index     = 0;

            /* Action Type is identified:
             * Start updating the Action configuration.
             */
            hw_eng_conf->action_conf.action_ptp  = action_ptp;

            sw_eng_conf = (ingress) ?
                          &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[engine_id] :
                          &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[engine_id];

            if (engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
                sw_alt_eng_conf = (ingress) ?
                                  &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B] :
                                  &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2B];

            } else if (engine_id == VTSS_PHY_TS_OAM_ENGINE_ID_2B) {
                sw_alt_eng_conf = (ingress) ?
                                  &vtss_state->phy_ts_port_conf[base_port_no].ingress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A] :
                                  &vtss_state->phy_ts_port_conf[base_port_no].egress_eng_conf[VTSS_PHY_TS_OAM_ENGINE_ID_2A];
            }

            if ((hw_eng_conf->eng_used        != sw_eng_conf->eng_used) ||
                (hw_eng_conf->encap_type      != sw_eng_conf->encap_type) ||
                (hw_eng_conf->flow_match_mode != sw_eng_conf->flow_match_mode)) {
                /* Reconfigure the entire engine;
                 * 1. Clearing of engine (Required ?).
                 * 2. Disable the engine and reconfigure the flow_match_mode.
                 * 3. Reset the next_comparator configuration.
                 */
                if (sw_alt_eng_conf && sw_alt_eng_conf->eng_used) {
                    /* Other engine is enabled so do not disable the engine */
                } else {
                    rc = vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, FALSE);
                }
                if (sw_eng_conf->eng_used) {
                    engine_update_required = TRUE;
                    hw_action_update_reqd  = TRUE;
                    /* Engine is configured and HW mismatch so re configure the engine */
                    /* Configure the engine only if the engine is enabled */
                    rc = vtss_phy_ts_engine_next_comp_sync_priv(vtss_state, ingress, port_no,
                                                                blk_id, engine_id, sw_eng_conf->encap_type);
                    encap_type = sw_eng_conf->encap_type;
                    rc = vtss_phy_ts_flow_match_mode_set_priv(vtss_state, port_no, blk_id,
                                                              engine_id, sw_eng_conf->flow_match_mode, ingress);

                    action_ptp                          = sw_eng_conf->action_conf.action_ptp;
                    hw_eng_conf->eng_used                = sw_eng_conf->eng_used;
                    hw_eng_conf->encap_type              = sw_eng_conf->encap_type;
                    hw_eng_conf->flow_st_index           = sw_eng_conf->flow_st_index;
                    hw_eng_conf->flow_end_index          = sw_eng_conf->flow_end_index;
                    hw_eng_conf->flow_match_mode         = sw_eng_conf->flow_match_mode;
                    hw_eng_conf->action_conf.action_ptp  = sw_eng_conf->action_conf.action_ptp;

                } else {
                    engine_enable_reqd = FALSE;
                }
                phy_engine_sync_reqd = FALSE;
            }

            /* Even if there is Action Type mismatch then there is no need to synchronize the Engine.
             * Proceed with updating the Engine configuration.
             */
            if (hw_eng_conf->action_conf.action_ptp != sw_eng_conf->action_conf.action_ptp) {
                phy_engine_sync_reqd = FALSE;
            }

            if (!sw_eng_conf->eng_used && !hw_eng_conf->eng_used) {
                /* 1. Engine is not used in software.
                 * 2. Engine is not configured/enabled in Hardware.
                 * No mismatch in the configuration, hence no need to sync
                 */
                continue;
            }

            if (phy_engine_sync_reqd) {
                /* Reconceliation required for the engines*/
                /* Read all the analyzer registers and compare with vtss_state structure
                 * if both the values match then set engine_update_required to TRUE.
                 */
                VTSS_PHY_TS_SPI_PAUSE_PRIV(port_no);
                if ((rc = vtss_phy_ts_engine_flow_hwsync(vtss_state, ingress, action_ptp, &engine_update_required,
                                                         &hw_action_update_reqd, engine_id, encap_type,
                                                         port_no, base_port_no, blk_id,
                                                         sw_eng_conf, hw_eng_conf)) != VTSS_RC_OK) {
                    VTSS_E("HW SYNC Failure");
                }
                VTSS_PHY_TS_SPI_UNPAUSE_PRIV(port_no);
            }

            if (engine_enable_reqd) {
                VTSS_PHY_TS_SPI_PAUSE_PRIV(port_no);
                if ((rc = vtss_phy_ts_engine_config_priv(vtss_state, ingress, port_no, engine_id, TRUE))
                    != VTSS_RC_OK) {
                    VTSS_E("HW Engine Enable Failure");
                }
                VTSS_PHY_TS_SPI_UNPAUSE_PRIV(port_no);
            }

            if (engine_update_required) {
                VTSS_PHY_TS_SPI_PAUSE_PRIV(port_no);
                if ((rc = vtss_phy_ts_engine_flow_hwupdate(vtss_state, ingress, engine_id, encap_type,
                                                           port_no, base_port_no, blk_id,
                                                           sw_eng_conf, hw_eng_conf)) != VTSS_RC_OK) {
                    VTSS_E("HW Engine Update Failure");
                }
                VTSS_PHY_TS_SPI_UNPAUSE_PRIV(port_no);
            }
            /* TODO :: Need to decide if we need to add any delay here for SPI to push the timestamps
             */
            if (hw_action_update_reqd) {
                /* Action mismatch,
                 * Deleted all the Actions
                 * Add the actions again
                 */
                VTSS_PHY_TS_SPI_PAUSE_PRIV(port_no);
                if ((rc = vtss_phy_ts_engine_action_hwupdate(vtss_state, ingress, action_ptp, engine_id, encap_type,
                                                             port_no, base_port_no, blk_id, sw_eng_conf, hw_eng_conf)) != VTSS_RC_OK) {
                    VTSS_E("HW Action Update Failure");
                }
                VTSS_PHY_TS_SPI_UNPAUSE_PRIV(port_no);
            }
        }
    }
    if (!generic_phy_engine_sync_reqd && vtss_state->phy_ts_port_conf[port_no].port_ena) {
        /* Sync the Port states */
        rc = vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_ENA_SET);
    }

    VTSS_PHY_TS_SPI_PAUSE_PRIV(port_no);
    /* Update the ingress_latency and egress_latency. */
    if (vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_ING_LATENCY_SET) != VTSS_RC_OK) {
        VTSS_E("Ingress Latency set fail, port %u", port_no);
        rc = VTSS_RC_ERROR;
    }
    if (vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_EGR_LATENCY_SET) != VTSS_RC_OK) {
        VTSS_E("Egress Latency set fail, port %u", port_no);
        rc = VTSS_RC_ERROR;
    }
    if (vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_OPER_MODE_CHANGE_SET) != VTSS_RC_OK) {
        VTSS_E("PHY operational mode change set failed, port %u", port_no);
        rc = VTSS_RC_ERROR;
    }
    VTSS_PHY_TS_SPI_UNPAUSE_PRIV(port_no);

    vtss_state->sync_calling_private = TRUE;

    VTSS_SYNC_RC(vtss_phy_ts_csr_set_priv(vtss_state, port_no, VTSS_PHY_TS_PORT_EVT_MASK_SET));

    vtss_state->sync_calling_private = FALSE;

    return rc;
}

static void vtss_phy_ts_dis_eth_flow(vtss_phy_ts_eth_conf_t *econf, const vtss_debug_printf_t pr)
{
    u8 tloop;
    pr("PBB mode :: %s\n", econf->comm_opt.pbb_en ? "Enabled" : "Disabled");
    pr("Etype :: %.4x\t Tpid :: %.4x\n", econf->comm_opt.etype, econf->comm_opt.tpid);
    for (tloop = 0; tloop < 8; tloop++) {
        if (econf->flow_opt[tloop].flow_en) {
            char *tchr = NULL;
            u8 *mac = econf->flow_opt[tloop].mac_addr;
            pr("\nFlow %d :: Enabled\n", tloop + 1);
            switch (econf->flow_opt[tloop].addr_match_mode) {
            case VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT:
                tchr = "FULL";
                break;
            case VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST:
                tchr = "ANY UNICAST";
                break;
            case VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST:
                tchr = "ANY MULTICAST";
                break;
            }
            pr("Address Match mode :: %s\n", tchr);
            switch (econf->flow_opt[tloop].addr_match_select) {
            case VTSS_PHY_TS_ETH_MATCH_DEST_ADDR:
                tchr = "Dest. MAC";
                break;
            case VTSS_PHY_TS_ETH_MATCH_SRC_ADDR:
                tchr = "Src. MAC";
                break;
            case VTSS_PHY_TS_ETH_MATCH_SRC_OR_DEST:
                tchr = "Dest. or Src. MAC";
                break;
            }
            pr("Address Match select :: %s\n", tchr);
            pr("Mac address :: %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            pr("VLAN check  :: %s\n", econf->flow_opt[tloop].vlan_check ? "Enabled" : "Disabled");
            pr("Number of tags :: %d\n", econf->flow_opt[tloop].num_tag);
            tchr = 0;
            switch (econf->flow_opt[tloop].outer_tag_type) {
            case VTSS_PHY_TS_TAG_TYPE_C:
                tchr = "C_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_S:
                tchr = "S_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_I:
                tchr = "I_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_B:
                tchr = "B_Tag_Type";
                break;
            }
            pr("Outer Tag type :: %s\n", tchr);
            tchr = 0;
            switch (econf->flow_opt[tloop].inner_tag_type) {
            case VTSS_PHY_TS_TAG_TYPE_C:
                tchr = "C_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_S:
                tchr = "S_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_I:
                tchr = "I_Tag_Type";
                break;
            case VTSS_PHY_TS_TAG_TYPE_B:
                tchr = "B_Tag_Type";
                break;
            }
            pr("Inner Tag type :: %s\n", tchr);
            tchr = 0;
            switch (econf->flow_opt[tloop].tag_range_mode) {
            case VTSS_PHY_TS_TAG_RANGE_NONE:
                tchr = "None";
                break;
            case VTSS_PHY_TS_TAG_RANGE_OUTER:
                tchr = "Outer";
                break;
            case VTSS_PHY_TS_TAG_RANGE_INNER:
                tchr = "Inner";
                break;
            }
            pr("Tag Range :: %s\n", tchr);
        } else {
            pr("Flow %d :: Disabled\n", tloop + 1);
        }
    }

}


static void vtss_phy_ts_dis_ip_flow(vtss_phy_ts_ip_conf_t *ipconf, const vtss_debug_printf_t pr)
{
    u8 tloop;
    pr("Ip Version %s\n", ipconf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4 ? "IPV4" : (ipconf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_6 ? "IPV6" : "None"));

    pr("Sport value = %u, Mask = %u\n", ipconf->comm_opt.sport_val, ipconf->comm_opt.sport_mask);
    pr("Dport value = %u, Mask = %u\n", ipconf->comm_opt.dport_val, ipconf->comm_opt.dport_mask);
    for (tloop = 0; tloop < 8; tloop++) {
        if (ipconf->flow_opt[tloop].flow_en) {
            char *tchr = 0;
            pr("Flow %u: Enabled\n", tloop + 1);
            switch (ipconf->flow_opt[tloop].match_mode) {
            case VTSS_PHY_TS_IP_MATCH_SRC:
                tchr = "IP SRC";
                break;
            case VTSS_PHY_TS_IP_MATCH_DEST:
                tchr = "IP DEST";
                break;
            case VTSS_PHY_TS_IP_MATCH_SRC_OR_DEST:
                tchr = "IP SRC or DEST";
                break;

            }
            pr("Match mode %s\n", tchr);

            if (ipconf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                u8 *ptr = (u8 *) & (ipconf->flow_opt[tloop].ip_addr.ipv4.addr);
                pr("IP :: %u.%u.%u.%u\n", *(ptr + BYTE_OFFSET(24)), *(ptr + BYTE_OFFSET(16)), *(ptr + BYTE_OFFSET(8)), *ptr);
                ptr = (u8 *) & (ipconf->flow_opt[tloop].ip_addr.ipv4.mask);
                pr("IP Mask :: %u.%u.%u.%u\n", *(ptr + BYTE_OFFSET(24)), *(ptr + BYTE_OFFSET(16)), *(ptr + BYTE_OFFSET(8)), *ptr);

            } else if (ipconf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_6) {
                u8 *ptr = (u8 *) ipconf->flow_opt[tloop].ip_addr.ipv6.addr;
                pr("IP6 :: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                   *(ptr + BYTE_OFFSET((15 * 8))), *(ptr + BYTE_OFFSET((14 * 8))), *(ptr + BYTE_OFFSET((13 * 8))), *(ptr + BYTE_OFFSET((12 * 8))),
                   *(ptr + BYTE_OFFSET((11 * 8))), *(ptr + BYTE_OFFSET((10 * 8))), *(ptr + BYTE_OFFSET((9 * 8))), *(ptr + BYTE_OFFSET((8 * 8))),
                   *(ptr + BYTE_OFFSET((7 * 8))), *(ptr + BYTE_OFFSET((6 * 8))), *(ptr + BYTE_OFFSET((5 * 8))), *(ptr + BYTE_OFFSET((4 * 8))),
                   *(ptr + BYTE_OFFSET((3 * 8))), *(ptr + BYTE_OFFSET((2 * 8))), *(ptr + BYTE_OFFSET(8)), *ptr);
                ptr = (u8 *) ipconf->flow_opt[tloop].ip_addr.ipv6.mask;
                pr("IP6 mask :: %x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                   *(ptr + BYTE_OFFSET((15 * 8))), *(ptr + BYTE_OFFSET((14 * 8))), *(ptr + BYTE_OFFSET((13 * 8))), *(ptr + BYTE_OFFSET((12 * 8))),
                   *(ptr + BYTE_OFFSET((11 * 8))), *(ptr + BYTE_OFFSET((10 * 8))), *(ptr + BYTE_OFFSET((9 * 8))), *(ptr + BYTE_OFFSET((8 * 8))),
                   *(ptr + BYTE_OFFSET((7 * 8))), *(ptr + BYTE_OFFSET((6 * 8))), *(ptr + BYTE_OFFSET((5 * 8))), *(ptr + BYTE_OFFSET((4 * 8))),
                   *(ptr + BYTE_OFFSET((3 * 8))), *(ptr + BYTE_OFFSET((2 * 8))), *(ptr + BYTE_OFFSET(8)), *ptr);

            }

        } else {
            pr("Flow %u: Disabled\n", tloop + 1);
        }
    }
}

static void vtss_phy_ts_dis_mpls_flow(vtss_phy_ts_mpls_conf_t *mconf, const vtss_debug_printf_t pr)
{
    u8 tflw;
    pr("Control Word :: %s\n", mconf->comm_opt.cw_en ? "Enabled" : "Disabled" );
    for (tflw = 0; tflw < 8; tflw++) {
        if (mconf->flow_opt[tflw].flow_en) {
            char *tchr = 0;
            pr("Flow %u :: Enabled\n", tflw + 1);
            switch (mconf->flow_opt[tflw].stack_depth) {
            case VTSS_PHY_TS_MPLS_STACK_DEPTH_1:
                tchr = "STACK_DEPTH_1";
                break;
            case VTSS_PHY_TS_MPLS_STACK_DEPTH_2:
                tchr = "STACK_DEPTH_2";
                break;
            case VTSS_PHY_TS_MPLS_STACK_DEPTH_3:
                tchr = "STACK_DEPTH_3";
                break;
            case VTSS_PHY_TS_MPLS_STACK_DEPTH_4:
                tchr = "STACK_DEPTH_4";
                break;
            }
            pr("Stack Depth %s\n", tchr);
            if (mconf->flow_opt[tflw].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                pr("Stack Direction :: Top-Down\n");
                pr("Top Level    :: %u - %u\n", mconf->flow_opt[tflw].stack_level.top_down.top.lower,
                   mconf->flow_opt[tflw].stack_level.top_down.top.upper);
                pr("First Label  :: %u - %u\n", mconf->flow_opt[tflw].stack_level.top_down.frst_lvl_after_top.lower,
                   mconf->flow_opt[tflw].stack_level.top_down.frst_lvl_after_top.upper);
                pr("Second Label :: %u - %u\n", mconf->flow_opt[tflw].stack_level.top_down.snd_lvl_after_top.lower,
                   mconf->flow_opt[tflw].stack_level.top_down.snd_lvl_after_top.upper);
                pr("Third Label  :: %u - %u\n", mconf->flow_opt[tflw].stack_level.top_down.thrd_lvl_after_top.lower,
                   mconf->flow_opt[tflw].stack_level.top_down.thrd_lvl_after_top.upper);

            } else if (mconf->flow_opt[tflw].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_END) {
                pr("Stack Direction :: Bottom-Up\n");
                pr("End Level    :: %u - %u\n", mconf->flow_opt[tflw].stack_level.bottom_up.end.lower,
                   mconf->flow_opt[tflw].stack_level.bottom_up.end.upper);
                pr("First Label  :: %u - %u\n", mconf->flow_opt[tflw].stack_level.bottom_up.end.lower,
                   mconf->flow_opt[tflw].stack_level.bottom_up.end.upper);
                pr("Second Label :: %u - %u\n", mconf->flow_opt[tflw].stack_level.bottom_up.end.lower,
                   mconf->flow_opt[tflw].stack_level.bottom_up.end.upper);
                pr("Third Label  :: %u - %u\n", mconf->flow_opt[tflw].stack_level.bottom_up.end.lower,
                   mconf->flow_opt[tflw].stack_level.bottom_up.end.upper);
            }

        } else {
            pr("Flow %u :: Disabled\n", tflw + 1);
        }
    }
}

static void vtss_phy_ts_dis_ach_flow(vtss_phy_ts_ach_conf_t *aconf, const vtss_debug_printf_t pr)
{
    pr("Version Value       = %x Mask = %x\n", aconf->comm_opt.version.value, aconf->comm_opt.version.mask);
    pr("Channel Type  Value = %x Mask = %x\n", aconf->comm_opt.channel_type.value, aconf->comm_opt.channel_type.mask);
    pr("Protocol ID Value   = %x Mask = %x\n", aconf->comm_opt.proto_id.value, aconf->comm_opt.proto_id.mask);
}
static void vtss_phy_ts_dis_action(vtss_phy_ts_engine_action_t *act, const vtss_debug_printf_t pr)
{
    if (act->action_ptp) {
        u8 tloop;
        pr("Action :: PTP\n");

        for (tloop = 0; tloop < 2; tloop++) {
            char *tchr = 0;
            vtss_phy_ts_ptp_conf_t *pconf;
            pr("Action state :: %s\n", act->action.ptp_conf[tloop].enable ? "Active" : "Disabled" );
            pr("ptp conf %u\n", tloop + 1);
            switch (act->action.ptp_conf[tloop].channel_map) {
            case VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0:
                tchr = "Channel 0";
                break;
            case VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1:
                tchr = "channel 1";
                break;
            default:
                tchr = "None";
            }
            pr("Channel Map :: %s\n", tchr);

            pconf = &(act->action.ptp_conf[tloop].ptp_conf);
            if (pconf->range_en) {
                pr("Range is %u - %u\n", pconf->domain.range.upper, pconf->domain.range.lower);
            } else {
                pr("Value is %u, Mask %u\n", pconf->domain.value.val, pconf->domain.value.mask);
            }

            tchr = 0;
            switch (act->action.ptp_conf[tloop].clk_mode) {
            case VTSS_PHY_TS_PTP_CLOCK_MODE_BC1STEP:
                tchr = "Boundary Clock 1 step";
                break;
            case VTSS_PHY_TS_PTP_CLOCK_MODE_BC2STEP:
                tchr = "Boundary Clock 2 step";
                break;
            case VTSS_PHY_TS_PTP_CLOCK_MODE_TC1STEP:
                tchr = "Transparent Clock 1 step";
                break;
            case VTSS_PHY_TS_PTP_CLOCK_MODE_TC2STEP:
                tchr = "Transparent Clock 2 step";
                break;
            case VTSS_PHY_TS_PTP_DELAY_COMP_ENGINE:
                tchr = "Delay Compensation";
                break;
            }
            pr("Clock Mode :: %s\n", tchr);

            tchr = 0;
            switch (act->action.ptp_conf[tloop].delaym_type) {
            case VTSS_PHY_TS_PTP_DELAYM_P2P:
                tchr = "Peer to Peer";
                break;
            case VTSS_PHY_TS_PTP_DELAYM_E2E:
                tchr = "End to End";
                break;
            }
            pr("Delay Measurement Method :: %s\n\n", tchr);
        }
    } else {
        u8 tloop;
        pr("Action :: OAM\n");
        for (tloop = 0; tloop < 6; tloop++) {
            pr("OAM Conf %u\n", tloop + 1);
            pr("Conf :: %s\n", act->action.oam_conf[tloop].enable ? "Enabled" : "Disabled");
            if (act->action.oam_conf[tloop].channel_map == VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH0) {
                pr("Channel Map :: Channel0\n");
            } else if (act->action.oam_conf[tloop].channel_map == VTSS_PHY_TS_ENG_FLOW_VALID_FOR_CH1) {
                pr("Channel Map :: Channel1\n");
            }
            pr("Version :: %u\n", act->action.oam_conf[tloop].version);
            if (act->action.oam_conf[tloop].y1731_en) {
                vtss_phy_ts_y1731_oam_conf_t *oconf = &(act->action.oam_conf[tloop].oam_conf.y1731_oam_conf);
                pr("Y1731 :: Enabled\n" );
                if (oconf->delaym_type == VTSS_PHY_TS_Y1731_OAM_DELAYM_1DM) {
                    pr("Delay Measurement :: OneWay\n");
                } else if (oconf->delaym_type == VTSS_PHY_TS_Y1731_OAM_DELAYM_DMM) {
                    pr("Delay Measurement :: TwoWay\n");
                }
                if (oconf->range_en) {
                    pr("MEG Level Range %u - %u\n", oconf->meg_level.range.lower, oconf->meg_level.range.upper);
                } else {
                    pr("MEG Level Value %u, Mask %u\n", oconf->meg_level.value.val, oconf->meg_level.value.mask);
                }
            } else {
                vtss_phy_ts_ietf_mpls_ach_oam_conf_t *oconf = &(act->action.oam_conf[tloop].oam_conf.ietf_oam_conf);
                pr("Ietf Mpls :: Enabled\n");
                if (oconf->delaym_type == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_DMM) {
                    pr("Delay Measurement Method :: TwoWayDelayMeasurement\n");
                } else if (oconf->delaym_type == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_DELAYM_LDM) {
                    pr("Delay Measurement Method :: Loss/Delay CombinedMeasurement\n");
                }

                if (oconf->ts_format == VTSS_PHY_TS_IETF_MPLS_ACH_OAM_TS_FORMAT_PTP) {
                    pr("DM TS Format :: PTP\n");
                }
                pr("DSCP Value %u\n", oconf->ds);
            }
        }
    }
}

static void vtss_phy_ts_dis_action_map(u8 *fmap, const vtss_debug_printf_t pr)
{
    u8 tloop;
    for (tloop = 0; tloop < 6; tloop++) {
        pr("Action->flow %u\n", fmap[tloop]);
    }
}

static u8 vtss_phy_ts_dis_encap_type(vtss_phy_ts_encap_t etype, const vtss_debug_printf_t pr)
{
    char *tchr;
    u8 encap = 0;
    switch (etype) {
    case VTSS_PHY_TS_ENCAP_ETH_PTP:
        tchr = "ETH_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_PTP:
        tchr = "ETH_IP_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP:
        tchr = "ETH_IP_IP_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_PTP:
        tchr = "ETH_ETH_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP:
        tchr = "ETH_ETH_IP_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP:
        tchr = "ETH_MPLS_IP_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP:
        tchr = "ETH_MPLS_ETH_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP:
        tchr = "ETH_MPLS_ETH_IP_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP:
        tchr = "ETH_MPLS_ACH_PTP";
        encap = 1;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_OAM:
        tchr = "ETH_OAM";
        encap = 2;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM:
        tchr = "ETH_MPLS_ETH_OAM";
        encap = 2;
        break;
    case VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM:
        tchr = "ETH_MPLS_ACH_OAM";
        encap = 2;
        break;
    default :
        tchr = "None";

    }
    pr("encap_type                         : %s\n", tchr);
    return encap;
}

static void vtss_1588_api_dis_flow_match_mode(vtss_phy_ts_engine_flow_match_t mode, const vtss_debug_printf_t pr)
{
    char *tchr = "None";
    switch (mode) {
    case VTSS_PHY_TS_ENG_FLOW_MATCH_ANY:
        tchr = "Match any flow in Comparators";
        break;
    case VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT:
        tchr = "Strict flow match";
        break;
    }
    pr("flow_match_mode                    : %s\n", tchr);

}
static void vtss_1588_api_dis_state(vtss_state_t *vtss_state, vtss_port_no_t port_no, const vtss_debug_printf_t pr)
{
    u8 i;
    vtss_phy_ts_port_conf_t *pconf = vtss_state->phy_ts_port_conf + port_no;
    pr("1588 Base state dump portno:%u \n", port_no);
    pr("========================1588 Port Timestamp Configuration======================\n");
    pr("port_ts_init_done                     : %s\n", vtss_state->phy_ts_port_conf[port_no].port_ts_init_done ? "TRUE" : "FALSE");
    pr("eng_init_done                         : %s\n", vtss_state->phy_ts_port_conf[port_no].eng_init_done ? "TRUE" : "FALSE");
    pr("eng_init_done                         : %s\n", vtss_state->phy_ts_port_conf[port_no].port_ena ? "TRUE" : "FALSE");
    pr("base_port                             : %u\n", vtss_state->phy_ts_port_conf[port_no].base_port);
    pr("clk_freq                              : %u\n", vtss_state->phy_ts_port_conf[port_no].clk_freq);
    pr("clk_src                               : %u\n", vtss_state->phy_ts_port_conf[port_no].clk_src);
    pr("rx_ts_pos                             : %u\n", vtss_state->phy_ts_port_conf[port_no].rx_ts_pos);
    pr("rx_ts_len                             : %u\n", vtss_state->phy_ts_port_conf[port_no].rx_ts_len);
    pr("tx_fifo_mode                          : %u\n", vtss_state->phy_ts_port_conf[port_no].tx_fifo_mode);
    pr("tx_ts_len                             : %u\n", vtss_state->phy_ts_port_conf[port_no].tx_ts_len);
    pr("tc_op_mode                            : %u\n", vtss_state->phy_ts_port_conf[port_no].tc_op_mode);
#ifdef VTSS_CHIP_10G_PHY
    pr("xaui_sel_8487                         : %u\n", vtss_state->phy_ts_port_conf[port_no].xaui_sel_8487);
#endif
    pr("sig_mask                              : 0x%x\n", vtss_state->phy_ts_port_conf[port_no].sig_mask);
    pr("ingress_latency                       : %" PRIi64 "\n", vtss_state->phy_ts_port_conf[port_no].ingress_latency);
    pr("egress_latency                        : %" PRIi64 "\n", vtss_state->phy_ts_port_conf[port_no].egress_latency);
    pr("path_delay                            : %" PRIi64 "\n", vtss_state->phy_ts_port_conf[port_no].path_delay);
    pr("delay_asym                            : %" PRIi64 "\n", vtss_state->phy_ts_port_conf[port_no].delay_asym);
    pr("rate_adj                              : %" PRIi64 "\n", vtss_state->phy_ts_port_conf[port_no].rate_adj);
    pr("event_mask                            : 0x%x\n", vtss_state->phy_ts_port_conf[port_no].event_mask);
    pr("event_enable                          : %s\n", vtss_state->phy_ts_port_conf[port_no].event_enable ? "TRUE" : "FALSE");

    pr("\nIngress Engine Configuration\n");
    pr("-------------------------------------------------------\n");
    for (i = 0; i < 4; i++) {
        u8 encap;
        vtss_phy_ts_engine_flow_conf_t flw_cnf;
        u8 tloop;
        pr("Engine                             : %u\n", i);
        pr("eng_used                           : %s\n", vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].eng_used ? "TRUE" : "FALSE");
        encap = vtss_phy_ts_dis_encap_type(vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].encap_type, pr);
        vtss_1588_api_dis_flow_match_mode(vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].flow_match_mode, pr);
//      pr("flow_match_mode                    : %u\n", vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].flow_match_mode);
        pr("flow_st_index                      : %u\n", vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].flow_st_index);
        pr("flow_end_index                     : %u\n", vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].flow_end_index);
        flw_cnf = vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].flow_conf;
        pr("\n\tEngine Flow Configuration\n");
        pr("\t-------------------------\n");
        pr("Mode :: %s\n", flw_cnf.eng_mode ? "Enabled" : "Disabled" );
        for (tloop = 0; tloop < 8; tloop++) {
            pr("channel %u --> %u\n", tloop + 1, flw_cnf.channel_map[tloop]);
        }
        if (encap == 1) {
            pr("\nPTP Flow Configuration \n");
            pr("-----------------------\n");
            pr("\n\tETH1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.ptp.eth1_opt), pr);

            pr("\n\tETH2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.ptp.eth2_opt), pr);

            pr("\n\tIP1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ip_flow(&(flw_cnf.flow_conf.ptp.ip1_opt), pr);

            pr("\n\tIP2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ip_flow(&(flw_cnf.flow_conf.ptp.ip2_opt), pr);

            pr("\n\tMPLS Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_mpls_flow(&(flw_cnf.flow_conf.ptp.mpls_opt), pr);

            pr("\n\tACH Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ach_flow(&(flw_cnf.flow_conf.ptp.ach_opt), pr);
        } else if (encap == 2 ) {
            pr("\nOAM Flow Configuration \n");
            pr("-----------------------\n\n");

            pr("\n\tETH1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.oam.eth1_opt), pr);

            pr("\n\tETH2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.oam.eth2_opt), pr);

            pr("\n\tMPLS Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_mpls_flow(&(flw_cnf.flow_conf.oam.mpls_opt), pr);

            pr("\n\tACH Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ach_flow(&(flw_cnf.flow_conf.oam.ach_opt), pr);
        }

        pr("\nEngine Action Configuration \n");
        pr("----------------------------\n");
        vtss_phy_ts_dis_action(&(vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].action_conf), pr);

        pr("\n\tAction Flow Map \n");
        pr("\t----------------\n");
        vtss_phy_ts_dis_action_map(vtss_state->phy_ts_port_conf[port_no].ingress_eng_conf[i].action_flow_map, pr);

        pr("\n");
    }

    pr("\nEgress Engine Configuration\n");
    pr("-------------------------------------------------------\n");
    for (i = 0; i < 4; i++) {
        u8 encap;
        vtss_phy_ts_engine_flow_conf_t flw_cnf;
        u8 tloop;
        pr("Engine                             : %u\n", i);
        pr("eng_used                           : %s\n", vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].eng_used ? "TRUE" : "FALSE");
        encap = vtss_phy_ts_dis_encap_type(vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].encap_type, pr);
        vtss_1588_api_dis_flow_match_mode(vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].flow_match_mode, pr);
        //pr("flow_match_mode                    : %u\n", vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].flow_match_mode);
        pr("flow_st_index                      : %u\n", vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].flow_st_index);
        pr("flow_end_index                     : %u\n", vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].flow_end_index);
        flw_cnf = vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].flow_conf;

        pr("\n\tEngine Flow Configuration\n");
        pr("\t-------------------------\n");
        pr("Mode :: %s\n", flw_cnf.eng_mode ? "Enabled" : "Disabled" );
        for (tloop = 0; tloop < 8; tloop++) {
            pr("channel %u --> %u\n", tloop + 1, flw_cnf.channel_map[tloop]);
        }
        if (encap == 1) {
            pr("\nPTP Flow Configuration \n");
            pr("-----------------------\n\n");
            pr("\n\tETH1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.ptp.eth1_opt), pr);

            pr("\n\tETH2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.ptp.eth2_opt), pr);

            pr("\n\tIP1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ip_flow(&(flw_cnf.flow_conf.ptp.ip1_opt), pr);

            pr("\n\tIP2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ip_flow(&(flw_cnf.flow_conf.ptp.ip2_opt), pr);

            pr("\n\tMPLS Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_mpls_flow(&(flw_cnf.flow_conf.ptp.mpls_opt), pr);

            pr("\n\tACH Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ach_flow(&(flw_cnf.flow_conf.ptp.ach_opt), pr);
        } else if (encap == 2 ) {
            pr("\nOAM Flow Configuration \n");
            pr("-----------------------\n\n");

            pr("\n\tETH1 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.oam.eth1_opt), pr);

            pr("\n\tETH2 Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_eth_flow(&(flw_cnf.flow_conf.oam.eth2_opt), pr);

            pr("\n\tMPLS Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_mpls_flow(&(flw_cnf.flow_conf.oam.mpls_opt), pr);

            pr("\n\tACH Flow Configuration\n");
            pr("\t-----------------------\n");
            vtss_phy_ts_dis_ach_flow(&(flw_cnf.flow_conf.oam.ach_opt), pr);

        }


        pr("\nEngine Action Configuration \n");
        pr("----------------------------\n");
        vtss_phy_ts_dis_action(&(vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].action_conf), pr);

        pr("\n\tAction Flow Map \n");
        pr("\t----------------\n");
        vtss_phy_ts_dis_action_map(vtss_state->phy_ts_port_conf[port_no].egress_eng_conf[i].action_flow_map, pr);

        pr("\n");
    }

    pr("\nGen2 :: %s\n", pconf->is_gen2 ? "Enabled" : "Disabled");
    pr("\nALT. Clock Mode \n");
    pr("pps_ls_lpbk       :: %s\n", pconf->alt_clock_mode.pps_ls_lpbk ? "True" : "False" );
    pr("ls_lpbk           :: %s\n", pconf->alt_clock_mode.ls_lpbk ? "True" : "False" );
    pr("ls_pps_lpbk       :: %s\n", pconf->alt_clock_mode.ls_pps_lpbk ? "True" : "False" );
    pr("\nPPS Configuration\n");
    pr("pps_width_adj     :: %u\n", pconf->pps_conf.pps_width_adj);
    pr("pps_offset        :: %u\n", pconf->pps_conf.pps_offset);
    pr("pps_output_enable :: %s\n", pconf->pps_conf.pps_output_enable ? "True" : "False" );
    pr("\nSerial TOD configuration \n");
    pr("ip_enable         :: %s\n", pconf->sertod_conf.ip_enable ? "True" : "False");
    pr("op_enable         :: %s\n", pconf->sertod_conf.op_enable ? "True" : "False");
    pr("ls_inv            :: %s\n", pconf->sertod_conf.ls_inv ? "True" : "False");
    pr("Load Pulse Delay  :: %u\n", pconf->load_pulse_delay);
    pr("chk_ing_modified  :: %s\n", pconf->chk_ing_modified ? "True" : "False");
    pr("auto_clear_ls     :: %s\n", pconf->auto_clear_ls ? "True" : "False");
    pr("macsec            :: %s\n", pconf->macsec_ena ? "Enabled" : "Disabled");
}

void vtss_phy_ts_api_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{

    if (info->layer == 1) {
        vtss_port_no_t        port_no;
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            //if(!(info->port_list[port_no] || vtss_state->phy_state[port_no].type.part_number == VTSS_PHY_TYPE_NONE )){
            if (!(info->port_list[port_no] )) {
                continue;
            }
            vtss_1588_api_dis_state(vtss_state, port_no, pr);
            pr("\n");
        }
    } else if (info->layer == 2) {
        pr("CIL :: in function %s,TBD..\n", __FUNCTION__);
    }
}


#endif /* VTSS_FEATURE_WARM_START */
#endif /* VTSS_FEATURE_PHY_TIMESTAMP*/

