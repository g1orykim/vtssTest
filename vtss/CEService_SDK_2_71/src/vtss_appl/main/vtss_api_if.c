/*

 Vitesse Switch API software.

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

 $Id$
 $Revision$

*/

#include "main.h"
#include "port_api.h"
#include "port_custom_api.h"
#include "vtss_api_if_api.h"
#include "critd_api.h"
#include "conf_api.h"
#include "misc_api.h"
#if defined(VTSS_PERSONALITY_STACKABLE)
#include "topo_api.h"
#endif










#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_API_AI /* Can't distinguish between AIL and CIL */
/* API semaphore */
static critd_t vtss_api_crit;

/* Mutex for get-modify-set API operations */
static vtss_os_crit_t vtss_appl_api_crit;

/* Globally exposed variable! */
const port_custom_entry_t *port_custom_table;

/* Port info inherited from port module */
static struct {
    vtss_port_no_t stack_a, stack_b;
} port;

/* Board information */
vtss_board_info_t board_info;

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_API_AI
#define TRACE_GRP_CNT        (VTSS_TRACE_GROUP_COUNT + 1)

#if (VTSS_TRACE_ENABLED)
static char api_trace_buf[2*1024];

static vtss_trace_reg_t trace_reg_ail =
{
    .module_id = VTSS_MODULE_ID_API_AI,
    .name      = "api_ail",
    .descr     = "VTSS API - Application Interface Layer"
};

static vtss_trace_reg_t trace_reg_cil =
{
    .module_id = VTSS_MODULE_ID_API_CI,
    .name      = "api_cil",
    .descr     = "VTSS API - Chip Interface Layer"
};

/* Default trace level */
#define VTSS_API_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
static vtss_trace_grp_t trace_grps_ail[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GROUP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_PORT] = {
        .name      = "port",
        .descr     = "Port",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_PHY] = {
        .name      = "phy",
        .descr     = "PHY",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_PACKET] = {
        .name      = "packet",
        .descr     = "Packet",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_QOS] = {
        .name      = "qos",
        .descr     = "QoS",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_L2] = {
        .name      = "l2",
        .descr     = "Layer 2",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_L3] = {
        .name      = "l3",
        .descr     = "Layer 3",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_SECURITY] = {
        .name      = "security",
        .descr     = "Security",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_EVC] = {
        .name      = "evc",
        .descr     = "EVC",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_FDMA_NORMAL] = {
        .name      = "fdma",
        .descr     = "FDMA when scheduler/interrupts is/are enabled",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GROUP_FDMA_IRQ] = {
        .name      = "fdma_irq",
        .descr     = "FDMA when scheduler/interrupts is/are disabled",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 0,
        .usec      = 1, // Getting hold of microseconds seems to work (because it reads the MIPS' instruction counter).
        .ringbuf   = 1, // Do not attempt to print directly to the console, since that'll use mutexes to get hold of the print buffers.
        .irq       = 1, // Do not attempt to wait for anything, dear trace module.
    },

    [VTSS_TRACE_GROUP_REG_CHECK] = {
        .name      = "reg_check",
        .descr     = "Register Access Checks (level = error)",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 0,
        .usec      = 1,
        .ringbuf   = 1, // Register access errors may be printed while the scheduler/interrupts are disabled, so use the ringbuf.
        .irq       = 1, // Do not attempt to wait for anything, dear trace module.
    },

    [VTSS_TRACE_GROUP_MPLS] = {
        .name      = "mpls",
        .descr     = "MPLS/MPLS-TP",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },

    [VTSS_TRACE_GROUP_MACSEC] = {
        .name      = "macsec",
        .descr     = "MacSec",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },

    [VTSS_TRACE_GROUP_VCAP] = {
        .name      = "vcap",
        .descr     = "VCAP",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },

    [VTSS_TRACE_GROUP_OAM] = {
        .name      = "oam",
        .descr     = "OAM",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },

    [VTSS_TRACE_GROUP_TS] = {
        .name      = "ts",
        .descr     = "Timestamp",
        .lvl       = VTSS_API_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },

    /* CRITD trace group, MUST be last */
    [TRACE_GRP_CNT - 1] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

static vtss_trace_grp_t trace_grps_cil[TRACE_GRP_CNT];

/* Using function name is more informative than file name in this case */
#define API_CRIT_ENTER(function) critd_enter(&vtss_api_crit, VTSS_TRACE_GROUP_COUNT, VTSS_TRACE_LVL_NOISE, function, 0)
#define API_CRIT_EXIT(function)  critd_exit(&vtss_api_crit, VTSS_TRACE_GROUP_COUNT, VTSS_TRACE_LVL_NOISE, function, 0)
#else
#define API_CRIT_ENTER(function) critd_enter(&vtss_api_crit)
#define API_CRIT_EXIT(function)  critd_exit( &vtss_api_crit)
#endif /* VTSS_TRACE_ENABLED */

/* ================================================================= *
 *  API lock/unlock
 * ================================================================= */

void vtss_callout_lock(const vtss_api_lock_t *const lock)
{
//    T_N("Locking function %s", lock->function);
    API_CRIT_ENTER(lock->function);
}

void vtss_callout_unlock(const vtss_api_lock_t *const lock)
{
    //   T_N("Unlocking function %s", lock->function);
    API_CRIT_EXIT(lock->function);
}

void vtss_appl_api_lock(void)
{
    VTSS_OS_CRIT_ENTER(&vtss_appl_api_crit);
}

void vtss_appl_api_unlock(void)
{
    VTSS_OS_CRIT_EXIT(&vtss_appl_api_crit);
}

void *vtss_callout_malloc(size_t size, vtss_mem_flags_t flags)
{
    return VTSS_MALLOC(size);
}

void vtss_callout_free(void *ptr, vtss_mem_flags_t flags)
{
    VTSS_FREE(ptr);
}

/* ================================================================= *
 *  Trace
 * ================================================================= */

#if (VTSS_TRACE_ENABLED)
/* Convert API trace level to application trace level */
static int api2appl_level(vtss_trace_level_t level)
{
    int lvl;

    switch (level) {
    case VTSS_TRACE_LEVEL_NONE:
        lvl = VTSS_TRACE_LVL_NONE;
        break;
    case VTSS_TRACE_LEVEL_ERROR:
        lvl = VTSS_TRACE_LVL_ERROR;
        break;
    case VTSS_TRACE_LEVEL_INFO:
        lvl = VTSS_TRACE_LVL_INFO;
        break;
    case VTSS_TRACE_LEVEL_DEBUG:
        lvl = VTSS_TRACE_LVL_DEBUG;
        break;
    case VTSS_TRACE_LEVEL_NOISE:
        lvl = VTSS_TRACE_LVL_NOISE;
        break;
    default:
        lvl = VTSS_TRACE_LVL_RACKET; /* Should never happen */
        break;
    }
    return lvl;
}

/* Convert application trace level to API trace level */
static vtss_trace_level_t appl2api_level(int lvl)
{
    vtss_trace_level_t level;

    switch (lvl) {
    case VTSS_TRACE_LVL_ERROR:
    case VTSS_TRACE_LVL_WARNING:
        level = VTSS_TRACE_LEVEL_ERROR;
        break;
    case VTSS_TRACE_LVL_INFO:
        level = VTSS_TRACE_LEVEL_INFO;
        break;
    case VTSS_TRACE_LVL_DEBUG:
        level = VTSS_TRACE_LEVEL_DEBUG;
        break;
    case VTSS_TRACE_LVL_NOISE:
    case VTSS_TRACE_LVL_RACKET:
        level = VTSS_TRACE_LEVEL_NOISE;
        break;
    case VTSS_TRACE_LVL_NONE:
        level = VTSS_TRACE_LEVEL_NONE;
        break;
    default:
        level = VTSS_TRACE_LEVEL_ERROR; /* Should never happen */
        break;
    }
    return level;
}

static int api2appl_module_id(const vtss_trace_layer_t layer)
{
    return (layer == VTSS_TRACE_LAYER_AIL ? trace_reg_ail.module_id : trace_reg_cil.module_id);
}
#endif /* VTSS_TRACE_ENABLED */

/* Trace callout function */
void vtss_callout_trace_printf(const vtss_trace_layer_t layer,
                               const vtss_trace_group_t group,
                               const vtss_trace_level_t level,
                               const char *file,
                               const int line,
                               const char *function,
                               const char *format,
                               ...)
{
#if (VTSS_TRACE_ENABLED)
    va_list args;
    int     module_id = api2appl_module_id(layer);
    int     lvl = api2appl_level(level);
    
    /* Map API trace to WebStaX trace */
    va_start(args, format);
    vsprintf(api_trace_buf, format, args);
    va_end(args);
    vtss_trace_printf(module_id, group, lvl, function, line, api_trace_buf);
#endif /* VTSS_TRACE_ENABLED */
}

/* Trace hex-dump callout function */
void vtss_callout_trace_hex_dump(const vtss_trace_layer_t layer,
                                 const vtss_trace_group_t group,
                                 const vtss_trace_level_t level,
                                 const char               *file,
                                 const int                line,
                                 const char               *function,
                                 const unsigned char      *byte_p,
                                 const int                byte_cnt)
{
#if (VTSS_TRACE_ENABLED)
    int module_id = api2appl_module_id(layer);
    int lvl = api2appl_level(level);

    /* Map API trace to WebStaX trace */
    vtss_trace_hex_dump(module_id, group, lvl, function, line, byte_p, byte_cnt);
#endif /* VTSS_TRACE_ENABLED */
}

/* Called when module trace levels have been changed */
void vtss_api_trace_update(void)
{
#if (VTSS_TRACE_ENABLED)
    int               grp;
    vtss_trace_conf_t conf;
    int               global_lvl = vtss_trace_global_lvl_get();

    for (grp = 0; grp < VTSS_TRACE_GROUP_COUNT; grp++) {
        /* Map WebStaX trace level to API trace level */
        conf.level[VTSS_TRACE_LAYER_AIL] = global_lvl > trace_grps_ail[grp].lvl ? appl2api_level(global_lvl) : appl2api_level(trace_grps_ail[grp].lvl);
        conf.level[VTSS_TRACE_LAYER_CIL] = global_lvl > trace_grps_cil[grp].lvl ? appl2api_level(global_lvl) : appl2api_level(trace_grps_cil[grp].lvl);
        vtss_trace_conf_set(grp, &conf);
    }
#endif /* VTSS_TRACE_ENABLED */
}


/* ================================================================= *
 *  I2C
 * ================================================================= */

/**
 * \brief Function for doing i2c reads from the switch i2c controller (Using the eCos driver)
 *
 * \param port_no [IN] Port number
 * \param i2c_addr [IN] I2C device address
 * \param addr [IN]   Register address
 * \param data [OUT]  Pointer the register(s) data value.
 * \param cnt [IN]    Number of registers to read
 *
 * \return Return code.
 **/
vtss_rc i2c_read(const vtss_port_no_t port_no, const u8 i2c_addr, const u8 addr, u8 *const data, const u8 cnt, const i8 i2c_clk_sel)
{
    u8 reg_addr = addr;
    return vtss_i2c_wr_rd(NULL, i2c_addr,  &reg_addr, 1, data, cnt, 100, i2c_clk_sel);
}

/**
 * \brief Function for doing i2c reads from the switch i2c controller (Using the eCos driver)
 *
 * \param port_no [IN] Port number
 * \param i2c_addr [IN] I2C device address
 * \param addr [IN]   Register address
 * \param data [OUT]  Pointer the register(s) data value.
 * \param cnt [IN]    Number of registers to read
 *
 * \return Return code.
 **/
vtss_rc i2c_write(const vtss_port_no_t port_no, const u8 i2c_addr, u8 *const data, const u8 cnt, const i8 i2c_clk_sel)
{
    return vtss_i2c_wr(NULL, i2c_addr, data, cnt, 100, i2c_clk_sel);
}

/* ================================================================= *
 *  Stack port info
 * ================================================================= */

/* Port number of stack port 0 or 1 - Optimized version - uses cached info */
vtss_port_no_t port_no_stack(BOOL port_1)
{
    return (port_1 ? port.stack_b : port.stack_a);
}

/* Port number is stack port? */
BOOL port_no_is_stack(vtss_port_no_t port_no)
{
#if VTSS_SWITCH_STACKABLE
    return (port_no == port.stack_a) || (port_no == port.stack_b);
#else
    return 0;
#endif /* VTSS_SWITCH_STACKABLE */
}

BOOL vtss_stacking_enabled(void)
{
    return (port.stack_a == VTSS_PORT_NO_NONE ? 0 : 1);
}

/* ================================================================= *
 *  API register access
 * ================================================================= */

/* Memory mapped IO with base offset */
static volatile u32 *base_addr = (u32 *)VTSS_MEMORYMAPPEDIO_BASE;
static volatile u32 *base_addr_1; /* Board specific */

#ifdef VTSS_SW_OPTION_DEBUG
static u64 reg_reads[2];
static u64 reg_writes[2];
#endif

static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    if (chip_no == 0) {
        *value = base_addr[addr];
    } else {
        *value = base_addr_1[addr];
    }

#ifdef VTSS_SW_OPTION_DEBUG
    cyg_scheduler_lock();
    reg_reads[chip_no]++;
    cyg_scheduler_unlock();
#endif
    return VTSS_RC_OK;
}

static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{
    if (chip_no == 0) {
        base_addr[addr] = value;
    } else {
        base_addr_1[addr] = value;
    }

#ifdef VTSS_SW_OPTION_DEBUG
    cyg_scheduler_lock();
    reg_writes[chip_no]++;
    cyg_scheduler_unlock();
#endif
    return VTSS_RC_OK;
}

#ifdef VTSS_SW_OPTION_DEBUG
void vtss_api_if_reg_access_cnt_get(u64 read_cnts[2], u64 write_cnts[2])
{
    int c;
    cyg_scheduler_lock();
    for (c = 0; c < 2; c++) {
        read_cnts[c] = reg_reads[c];
        write_cnts[c] = reg_writes[c];
    }
    cyg_scheduler_unlock();
}
#endif

/* ================================================================= *
 *  API initialization
 * ================================================================= */

/* For the targets below, board probing is done before API initialization.
   This is neccessary for a single image supporting multiple boards.
   After board detection, the API is instantiated based on the detected target. */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
#define VTSS_SW_OPTION_BOARD_PROBE_PRE_INIT
#endif /* VTSS_ARCH_JAGUAR_1 || VTSS_ARCH_LUTON26 */

static void api_init(void)
{
    vtss_rc            rc;
    vtss_inst_create_t create;
    vtss_init_conf_t   conf;
    vtss_port_no_t     port_no;
    vtss_port_map_t    port_map[VTSS_PORT_ARRAY_SIZE];
    conf_board_t       board_conf;





    /* Default board information */
    conf_mgmt_board_get(&board_conf);
    board_info.board_type = board_conf.board_type;
    board_info.target = vtss_api_chipid();
    board_info.port_count = VTSS_PORTS;
    board_info.reg_read = reg_read;
    board_info.reg_write = reg_write;
    board_info.i2c_read = i2c_read;
    board_info.i2c_write = i2c_write;

#if defined(VTSS_SW_OPTION_BOARD_PROBE_PRE_INIT)
    /* Detect board type before API is initialized */
    vtss_board_probe(&board_info, &port_custom_table);
    VTSS_ASSERT(port_custom_table != NULL);
#endif /* VTSS_SW_OPTION_BOARD_PROBE_PRE_INIT */

    vtss_inst_get(board_info.target, &create);
    vtss_inst_create(&create, NULL);

    (void) vtss_init_conf_get(NULL, &conf);
    conf.reg_read = board_info.reg_read;
    conf.reg_write = board_info.reg_write;
    base_addr_1 = board_info.base_addr_1;


















    /* Optional port mux mode */
#if defined(VTSS_SW_OPTION_PORT_MUX)
#if (VTSS_SW_OPTION_PORT_MUX == 1)
    conf.mux_mode = VTSS_PORT_MUX_MODE_1;
#endif /* VTSS_SW_OPTION_PORT_MUX == 1 */
#if (VTSS_SW_OPTION_PORT_MUX == 7)
    conf.mux_mode = VTSS_PORT_MUX_MODE_7;
#endif /* VTSS_SW_OPTION_PORT_MUX == 7 */
#if defined(VTSS_ARCH_JAGUAR_1_DUAL)
    conf.mux_mode_2 = conf.mux_mode;
#endif /* VTSS_ARCH_JAGUAR_1_DUAL */
#endif /* VTSS_SW_OPTION_PORT_MUX */

#if defined(VTSS_SW_OPTION_DUAL_CONNECT_MODE) && (VTSS_SW_OPTION_DUAL_CONNECT_MODE == 1)
    /* Use XAUI_2 and XAUI_3 as interconnnect */
    conf.dual_connect_mode = VTSS_DUAL_CONNECT_MODE_1;
#endif /* VTSS_SW_OPTION_DUAL_CONNECT_MODE == 1*/

    rc = vtss_init_conf_set(NULL, &conf);
    VTSS_ASSERT(rc == VTSS_RC_OK);

    /* Open up API only after initialization */
    API_CRIT_EXIT(__FUNCTION__);

#if !defined(VTSS_SW_OPTION_BOARD_PROBE_PRE_INIT)
    /* Detect board type after API is initialized */
    vtss_board_probe(&board_info, &port_custom_table);
    VTSS_ASSERT(port_custom_table != NULL);
#endif /* VTSS_SW_OPTION_BOARD_PROBE_PRE_INIT */

    port.stack_a = port.stack_b = VTSS_PORT_NO_NONE;
#if defined(VTSS_FEATURE_VSTAX) && defined(VTSS_PERSONALITY_STACKABLE)
    if(vtss_board_type() && VTSS_BOARD_FEATURE_STACKING) {
        stack_config_t *stack_config;
        ulong stack_conf_size;
        BOOL configured = FALSE;

        if ((stack_config = conf_sec_open(CONF_SEC_LOCAL, CONF_BLK_STACKING, &stack_conf_size)) &&
            stack_conf_size == sizeof(*stack_config)) {

            if (stack_config->stacking) {
                if(stack_config->port_0 < VTSS_PORT_NO_END &&
                   stack_config->port_1 < VTSS_PORT_NO_END &&
                   port_custom_table[stack_config->port_0].cap & PORT_CAP_STACKING &&
                   port_custom_table[stack_config->port_1].cap & PORT_CAP_STACKING) {
                    configured = TRUE;
                    port.stack_a = stack_config->port_0;
                    port.stack_b = stack_config->port_1;
                    T_I("Explicit stack configuration: A:%d B:%d", port.stack_a, port.stack_b);
                }
            } else {
                // Stacking disabled, i.e. default to VTSS_PORT_NO_NONE
                configured = TRUE;
            }
        }

        if(!configured) {
            T_I("Default stack configuration: A:%d B:%d", port.stack_a, port.stack_b);
            port.stack_a = vtss_board_default_stackport(1);
            port.stack_b = vtss_board_default_stackport(0);
        }
    }
#endif  /* VTSS_FEATURE_VSTAX */

    /* Setup port map for board */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        port_map[port_no] = port_custom_table[port_no].map;
    vtss_port_map_set(NULL, port_map);

}

unsigned int vtss_api_chipid(void)
{
    /* Use detected target if valid */
    if (board_info.target)
        return board_info.target;

    /* The cumbersome mapping of VTSS_CHIP_XXX to VTSS_TARGET_XXX */
#if defined(VTSS_CHIP_E_STAX_34)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_E_STAX_34
#elif defined(VTSS_CHIP_SPARX_II_16)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_II_16
#elif defined(VTSS_CHIP_SPARX_II_24)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_II_24
#elif defined(VTSS_CHIP_SPARX_III_11)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_11
#elif defined(VTSS_CHIP_SERVAL_LITE)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SERVAL_LITE
#elif defined(VTSS_CHIP_SERVAL)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SERVAL
#elif defined(VTSS_CHIP_SPARX_III_10)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_10
#elif defined(VTSS_CHIP_SPARX_III_18)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_18
#elif defined(VTSS_CHIP_SPARX_III_24)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_24
#elif defined(VTSS_CHIP_SPARX_III_26)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_26
#elif defined(VTSS_CHIP_SPARX_III_10_01)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_SPARX_III_10_01
#elif defined(VTSS_CHIP_CARACAL_1)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_CARACAL_1
#elif defined(VTSS_CHIP_CARACAL_2)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_CARACAL_2
#elif defined(VTSS_CHIP_JAGUAR_1)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_JAGUAR_1
#elif defined(VTSS_CHIP_LYNX_1)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_LYNX_1
#elif defined(VTSS_CHIP_CE_MAX_24)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_CE_MAX_24
#elif defined(VTSS_CHIP_CE_MAX_12)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_CE_MAX_12
#elif defined(VTSS_CHIP_E_STAX_III_48)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_E_STAX_III_48
#elif defined(VTSS_CHIP_E_STAX_III_68)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_E_STAX_III_68
#elif defined(VTSS_CHIP_E_STAX_III_24_DUAL)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_E_STAX_III_24_DUAL
#elif defined(VTSS_CHIP_E_STAX_III_68_DUAL)
#define VTSS_SWITCH_API_TARGET VTSS_TARGET_E_STAX_III_68_DUAL
#else
#error Unable to determine Switch API target - check VTSS_CHIP_XXX define!
#endif
    return VTSS_SWITCH_API_TARGET;
}

u32 vtss_api_if_chip_count(void)
{
    return vtss_board_chipcount();
}

void vtss_board_info_get(vtss_board_info_t *info)
{
    *info = board_info;
}
#if defined(VTSS_FEATURE_PORT_LOOP_TEST)
/* Port loopback test                                                                                 */
/* -----------------                                                                                  */
/* This is a standalone function which uses the API to perform a port loopback test.                  */
/* This function should only be used  before the main application is started.                         */
/* Each switch port is enabled and the phy is reset and set in loopback.                              */
/* The CPU transmits 1 frame to each port and verifies that it is received again to the CPU buffer.   */
/* After the test completes the MAC address and counters are cleared.                                 */
static void port_loop_test(void)
{
    vtss_port_conf_t        conf;
    vtss_phy_reset_conf_t   phy_reset;
    vtss_port_no_t          port_no;
    BOOL                    testpassed=1;
    vtss_mac_table_entry_t  entry;
    u8                      frame[100];
    vtss_packet_rx_header_t header;

    /* Initilize */
    memset(frame,0,sizeof(frame));
    memset(&phy_reset,0,sizeof(vtss_phy_reset_conf_t));
    memset(&conf, 0, sizeof(vtss_port_conf_t));
    memset(&entry, 0, sizeof(vtss_mac_table_entry_t));
    frame[5] = 0xFF;  /* Test frame DMAC: 00-00-00-00-00-00-FF */
    frame[11] = 0x1;  /* Test frame SMAC: 00-00-00-00-00-00-01 */
    conf.if_type = VTSS_PORT_INTERFACE_SGMII;
    conf.speed = VTSS_SPEED_1G;
    conf.fdx = 1;
    conf.max_frame_length = 1518;
    entry.vid_mac.vid = 1;
    entry.locked = TRUE;
    entry.vid_mac.mac.addr[5] = 0xff;
    entry.copy_to_cpu = 1;
    (void) vtss_mac_table_add(NULL, &entry);

    /* Board and Phy init */
    port_custom_init();
    port_custom_reset();
    for(port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (port_phy(port_no)) {
            vtss_phy_post_reset(NULL, port_no);
            break;
        }
    }
    /* Reset and enable switch ports and phys. Enable phy loopback. */
    T_D("Port Loop Test Start:");
    for(port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (port_phy(port_no)) {
            conf.if_type = VTSS_PORT_INTERFACE_SGMII;
            phy_reset.mac_if = port_custom_table[port_no].mac_if;
            (void)vtss_phy_reset(NULL, port_no, &phy_reset);       
            (void)vtss_phy_write(NULL, port_no, 0, 0x4040);
        } else {
            conf.if_type = VTSS_PORT_INTERFACE_LOOPBACK;
        }
        (void)vtss_port_conf_set(NULL, port_no, &conf);
    }
    /* Wait while the Phys syncs up with the SGMII interface */
    VTSS_MSLEEP(2000);

    /* Enable frame forwarding and send one frame towards each port from the CPU.  Verify that the frame is received again. */
    for(port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        (void)vtss_port_state_set(NULL, port_no, 1);
        (void)vtss_packet_tx_frame_port(NULL, port_no, frame, 64);
        VTSS_MSLEEP(1); /* Wait until the frame is received again */
        if (vtss_packet_rx_frame_get(NULL, 0, &header, frame, 100) != VTSS_RC_OK) {
            T_E("Port %lu failed self test \n",port_no);
            testpassed = 0;
        } 
        (void)vtss_port_state_set(NULL, port_no, 0);
        if (port_phy(port_no)) {
            (void)vtss_phy_write(NULL, port_no, 0, 0x8000);
        }
        (void)vtss_port_counters_clear(NULL, port_no);
    }
    /* Clean up */
   (void)vtss_mac_table_del(NULL, &entry.vid_mac);
   (void)vtss_mac_table_flush(NULL);
    if (testpassed) {
        T_D("...Passed\n");
    }
}
#endif /* VTSS_FEATURE_PORT_LOOP_TEST */

vtss_rc vtss_api_if_init(vtss_init_data_t *data)
{
    int i;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Register AIL trace groups */
        VTSS_TRACE_REG_INIT(&trace_reg_ail, trace_grps_ail, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg_ail);
        
        /* Register CIL trace groups */
        for (i = 0; i < TRACE_GRP_CNT; i++)
            trace_grps_cil[i] = trace_grps_ail[i];
        VTSS_TRACE_REG_INIT(&trace_reg_cil, trace_grps_cil, TRACE_GRP_CNT - 1);
        VTSS_TRACE_REGISTER(&trace_reg_cil);

        /* Update API trace levels to initialization settings */
        vtss_api_trace_update();

        /* Create API semaphore (initially locked) */
        critd_init(&vtss_api_crit, "vtss_api_crit", VTSS_TRACE_MODULE_ID, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

        /* Create application API critical section (initially unlocked - this is a mutex) */
        VTSS_OS_CRIT_CREATE(&vtss_appl_api_crit);

        /* Initialize API */
        api_init();

#if defined(VTSS_FEATURE_PORT_LOOP_TEST)
       /* Perform a port loop test */
        port_loop_test();
#endif /*VTSS_FEATURE_PORT_LOOP_TEST*/

        break;
    case INIT_CMD_START:
        /* Update API trace levels to settings loaded from Flash by trace module */
        vtss_api_trace_update();

#ifdef VTSS_SW_OPTION_DEBUG
        // Enable register access checking.
        (void)vtss_debug_reg_check_set(NULL, TRUE);
#endif
        break;
    default:
        break;
    }

    return VTSS_OK;
}
