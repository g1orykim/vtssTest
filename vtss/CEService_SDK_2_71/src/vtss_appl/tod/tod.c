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

*/

#include "main.h"
#include "vtss_types.h"
#include "tod.h"
#include "tod_api.h"
#include "vtss_tod_api.h"
#include "critd_api.h"
#include "conf_api.h"
#include "interrupt_api.h"      /* interrupt handling */
#include "misc_api.h"

#include "port_api.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_tod_mod_man.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "tod_cli.h"
#endif
#endif

//#define CALC_TEST

#ifdef CALC_TEST
#include "vtss_ptp_types.h"
#endif

#if defined(VTSS_ARCH_JAGUAR_1)
#include <cyg/io/i2c_vcoreiii.h>
#define CYG_I2C_SI5338_DEVICE CYG_I2C_VCOREIII_DEVICE

#define SI5338_I2C_ADDR 0x70
#define SI5338_REG_CLK0_INTDIV_15_8 54
#define SI5338_REG_CLK0_INTDIV_125_MHZ 8
#define SI5338_REG_CLK0_INTDIV_156P25_MHZ 6
#define MAX_5338_I2C_LEN 2

#endif

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */


#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "tod",
    .descr     = "Time of day for PTP and OAM etc."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (TOD core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [VTSS_TRACE_GRP_CLOCK] = {
        .name      = "clock",
        .descr     = "TOD time functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    [VTSS_TRACE_GRP_MOD_MAN] = {
        .name      = "mod_man",
        .descr     = "PHY Timestamp module manager",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PHY_TS] = {
        .name      = "phy_ts",
        .descr     = "PHY Timestamp feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_REM_PHY] = {
        .name      = "rem_phy",
        .descr     = "PHY Remote Timestamp feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PHY_ENG] = {
        .name      = "phy_eng",
        .descr     = "PHY Engine allocation feature",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
#endif        
};
#endif /* VTSS_TRACE_ENABLED */

static struct {
    BOOL ready;                 /* TOD Initited  */
    critd_t datamutex;          /* Global data protection */
} tod_global;


/* ================================================================= *
 *  Configuration definitions
 * ================================================================= */

typedef struct tod_config_t {
    i8 version;             /* configuration version, to be changed if this structure is changed */
    vtss_tod_internal_tc_mode_t int_mode;  /* internal timestamping mode */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
    vtss_phy_ts_clockfreq_t     freq;      /* timestamping reference clock frequency */
#endif
#if defined (VTSS_ARCH_SERVAL)
    BOOL phy_ts_enable[VTSS_PORTS];  /* enable PHY timestamping mode */
#endif // (VTSS_ARCH_SERVAL)
} tod_config_t;

static tod_config_t config_data ;
/*
 * Propagate the PTP (module) configuration to the PTP core
 * library.
 */
static void
tod_conf_propagate(void)
{
    T_I("TOD Configuration has been reset, you need to reboot to activate the changed conf." );
}

static void
tod_conf_save(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    tod_config_t    *conf;  /* run-time configuration data */
    ulong size;
    if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOD_CONF, &size)) != NULL) {
        if (size == sizeof(*conf)) {
            T_IG(0, "Saving configuration");
            TOD_DATA_LOCK();
            *conf = config_data;
            TOD_DATA_UNLOCK();
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOD_CONF);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/**
 * Read the PTP configuration.
 * \param create indicates a new default configuration block should be created.
 *
 */
static void
tod_conf_read(BOOL create)
{
    ulong           size;
    BOOL            do_create;
	BOOL            tod_conf_changed = FALSE;
    tod_config_t    *conf;  /* run-time configuration data */
#if defined (VTSS_ARCH_SERVAL) && !defined (VTSS_CHIP_SERVAL_LITE)
    int i;
#endif

    if (misc_conf_read_use()) {
        if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_TOD_CONF, &size)) == NULL ||
                size != sizeof(*conf)) {
            conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_TOD_CONF, sizeof(*conf));
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            T_W("actual size = %d,expected size = %d", size, sizeof(*conf));
            do_create = TRUE;
        } else if (conf->version != TOD_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        conf      = NULL;
        do_create = TRUE;
    }

    TOD_DATA_LOCK();
    if (do_create) {
        /* Check if configuration changed */
            if (config_data.int_mode != VTSS_TOD_INTERNAL_TC_MODE_30BIT) tod_conf_changed = TRUE;
        /* initialize run-time options to reasonable values */
        config_data.version = TOD_CONF_VERSION;
        config_data.int_mode = VTSS_TOD_INTERNAL_TC_MODE_30BIT;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
        config_data.freq = VTSS_PHY_TS_CLOCK_FREQ_250M; /* 250 MHz is default on the PCB107 board */
#endif
#if defined (VTSS_ARCH_SERVAL) && !defined (VTSS_CHIP_SERVAL_LITE)
        for (i = 0; i < 4 && i < VTSS_PORTS; i++) {
            if (config_data.phy_ts_enable[i] != TRUE) tod_conf_changed = TRUE;
            config_data.phy_ts_enable[i] = TRUE;
        }
#endif
        if (conf != NULL) {
            *conf = config_data;
        }
        if (tod_conf_changed) {
            T_I("TOD Configuration has been reset, you need to reboot to activate the changed conf." );
        }

    } else {
        if (conf != NULL) {  // Quiet lint
            config_data = *conf;
        }
    }
    TOD_DATA_UNLOCK();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_TOD_CONF);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}
/****************************************************************************
 * Configuration API
 ****************************************************************************/

BOOL tod_tc_mode_get(vtss_tod_internal_tc_mode_t *mode)
{
    BOOL ok = FALSE;
    TOD_DATA_LOCK();
    ok = TRUE;
    *mode = config_data.int_mode;
    TOD_DATA_UNLOCK();
    return ok;
}
BOOL tod_tc_mode_set(vtss_tod_internal_tc_mode_t *mode)
{
    BOOL ok = FALSE;
    TOD_DATA_LOCK();
    if (*mode < VTSS_TOD_INTERNAL_TC_MODE_MAX) {
        ok = TRUE;
        config_data.int_mode = *mode;
    }
    TOD_DATA_UNLOCK();
    if (ok)
        tod_conf_save();
    return ok;
}

BOOL tod_port_phy_ts_get(BOOL *ts, vtss_port_no_t portnum)
{
    BOOL ok = FALSE;
    *ts = FALSE;
#if defined (VTSS_ARCH_SERVAL)
    TOD_DATA_LOCK();
    if (portnum < port_isid_port_count(VTSS_ISID_LOCAL)) {
        *ts = config_data.phy_ts_enable[portnum];
        ok = TRUE;
    }
    TOD_DATA_UNLOCK();
#endif /*defined (VTSS_ARCH_SERVAL) */
    return ok;
}

BOOL tod_port_phy_ts_set(BOOL *ts, vtss_port_no_t portnum)
{
    BOOL ok = FALSE;
#if defined (VTSS_ARCH_SERVAL)
    TOD_DATA_LOCK();
    if (portnum < port_isid_port_count(VTSS_ISID_LOCAL)) {
        config_data.phy_ts_enable[portnum] = *ts;
        ok = TRUE;
    }
    TOD_DATA_UNLOCK();
    if (ok)
        tod_conf_save();
#endif /* defined (VTSS_ARCH_SERVAL) */
    return ok;
}


#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
BOOL tod_ref_clock_freg_get(vtss_phy_ts_clockfreq_t *freq)
{
    BOOL ok = FALSE;
    TOD_DATA_LOCK();
    ok = TRUE;
    *freq = config_data.freq;
    TOD_DATA_UNLOCK();
    return ok;
}


BOOL tod_ref_clock_freg_set(vtss_phy_ts_clockfreq_t *freq)
{
    BOOL ok = FALSE;
    TOD_DATA_LOCK();
    if (*freq < VTSS_PHY_TS_CLOCK_FREQ_MAX) {
        ok = TRUE;
        config_data.freq = *freq;
        if (!tod_global.ready) {
            T_I("The 1588 reference clock is set");
        } else {
            T_W("The 1588 reference clock has been changed, please save configuration and reboot to apply the change");
        }
    }
    TOD_DATA_UNLOCK();
    if (ok)
        tod_conf_save();
    return ok;
}
#endif //defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)


BOOL tod_ready(void)
{
	return tod_global.ready;
}

/****************************************************************************
 * Callbacks
 ****************************************************************************/
#ifdef CALC_TEST

#define TC(t,c) ((t<<16) + c)
static void calcTest(void)
{
    u32 r, x, y;
    vtss_timeinterval_t t;
    char str1 [30];
    /* Ecos time counter = (10 ms ticks)<<16 + clocks , wraps at 1 sec*/

/* sub */    
    x = TC(43,4321);
    y = TC(41,1234); 
    vtss_tod_ts_cnt_sub(&r, x, y); /* no wrap */
    T_W("%ld,%ld - %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(43,4321);
    y = TC(41,4321); 
    vtss_tod_ts_cnt_sub(&r, x, y); /* no wrap */
    T_W("%ld,%ld - %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,4321);
    y = TC(41,1234); 
    vtss_tod_ts_cnt_sub(&r, x, y); /* no wrap */
    T_W("%ld,%ld - %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,4321);
    y = TC(42,1234); 
    vtss_tod_ts_cnt_sub(&r, x, y); /* tick wrap */
    T_W("%ld,%ld - %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(43,4321);
    y = TC(41,4322); 
    vtss_tod_ts_cnt_sub(&r, x, y); /* clk wrap */
    T_W("%ld,%ld - %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );

/* add */    
    x = TC(41,1234);
    y = TC(43,4321); 
    vtss_tod_ts_cnt_add(&r, x, y); /* no wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,1234);
    y = TC(43,8781); 
    vtss_tod_ts_cnt_add(&r, x, y); /* no wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,1234);
    y = TC(43,8782); 
    vtss_tod_ts_cnt_add(&r, x, y); /* clk wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,1234);
    y = TC(43,8783); 
    vtss_tod_ts_cnt_add(&r, x, y); /* clk wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(41,1234);
    y = TC(58,4321); 
    vtss_tod_ts_cnt_add(&r, x, y); /* no wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );
    x = TC(48,1234);
    y = TC(52,4321); 
    vtss_tod_ts_cnt_add(&r, x, y); /* tick wrap */
    T_W("%ld,%ld + %ld,%ld = %ld,%ld", x>>16,x & 0xffff,y>>16,y & 0xffff,r>>16,r & 0xffff );


/* timeinterval to cnt */
    t = VTSS_SEC_NS_INTERVAL(0,123456);
    vtss_tod_timeinterval_to_ts_cnt(&r, t);
    T_W("TimeInterval: %s => cnt %ld,%ld", TimeIntervalToString (&t, str1, '.'),r>>16,r & 0xffff );
    t = VTSS_SEC_NS_INTERVAL(0,123456789);
    vtss_tod_timeinterval_to_ts_cnt(&r, t);
    T_W("TimeInterval: %s => cnt %ld,%ld", TimeIntervalToString (&t, str1, '.'),r>>16,r & 0xffff );
    t = VTSS_SEC_NS_INTERVAL(0,999999000);
    vtss_tod_timeinterval_to_ts_cnt(&r, t);
    T_W("TimeInterval: %s => cnt %ld,%ld", TimeIntervalToString (&t, str1, '.'),r>>16,r & 0xffff );
    t = VTSS_SEC_NS_INTERVAL(0,999999999);
    vtss_tod_timeinterval_to_ts_cnt(&r, t);
    T_W("TimeInterval: %s => cnt %ld,%ld", TimeIntervalToString (&t, str1, '.'),r>>16,r & 0xffff );
    t = VTSS_SEC_NS_INTERVAL(1,0);
    vtss_tod_timeinterval_to_ts_cnt(&r, t);
    T_W("TimeInterval: %s => cnt %ld,%ld", TimeIntervalToString (&t, str1, '.'),r>>16,r & 0xffff );

/* cnt to timeinterval */
    x = TC(48,1234);
    vtss_tod_ts_cnt_to_timeinterval(&t, x);
    T_W("cnt %ld,%ld => TimeInterval: %s",r>>16,r & 0xffff , TimeIntervalToString (&t, str1, '.'));
    
}
#endif

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
#if defined(VTSS_ARCH_SERVAL)
static vtss_ts_internal_fmt_t serval_internal(void)
{
    vtss_tod_internal_tc_mode_t mode;
    if (tod_tc_mode_get(&mode)) {
        T_D("Internal mode: %d", mode);
        switch (mode) {
            case VTSS_TOD_INTERNAL_TC_MODE_30BIT: return TS_INTERNAL_FMT_RESERVED_LEN_30BIT;
            case VTSS_TOD_INTERNAL_TC_MODE_32BIT: return TS_INTERNAL_FMT_RESERVED_LEN_32BIT;
            case VTSS_TOD_INTERNAL_TC_MODE_44BIT: return TS_INTERNAL_FMT_SUB_ADD_LEN_44BIT_CF62;
            case VTSS_TOD_INTERNAL_TC_MODE_48BIT: return TS_INTERNAL_FMT_RESERVED_LEN_48BIT_CF_0;
            default: return TS_INTERNAL_FMT_NONE;
        }
    } else {
        return TS_INTERNAL_FMT_NONE;
    }
}
#endif /* (VTSS_ARCH_SERVAL) */

/*
 * TOD Synchronization 1 pps pulse update handler
 */
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static void one_pps_pulse_interrupt_handler(vtss_interrupt_source_t     source_id,
        u32                         instance_id)
{
    BOOL ongoing_adj;
    T_N("One sec pulse event: source_id %d, instance_id %u", source_id, instance_id);
    TOD_RC(vtss_ts_adjtimer_one_sec(0, &ongoing_adj));
    /* trig ModuleManager */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    TOD_RC(ptp_module_man_trig(ongoing_adj));
#endif    
    TOD_RC(vtss_interrupt_source_hook_set(one_pps_pulse_interrupt_handler,
                                          INTERRUPT_SOURCE_SYNC,
                                          INTERRUPT_PRIORITY_NORMAL));
}
#endif

static void tod_ts_port_state_change_callback(vtss_isid_t isid,
                               vtss_port_no_t port_no,
                               port_info_t *info)
{
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    vtss_tod_ts_phy_topo_t phy_ts;
#endif
    if (!info->stack && info->link) {
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        tod_ts_phy_topo_get(port_no, &phy_ts);
        if (phy_ts.ts_feature == VTSS_PTP_TS_JTS || phy_ts.ts_feature == VTSS_PTP_TS_CTS || phy_ts.ts_feature == VTSS_PTP_TS_NONE) 
#endif
        {
            if (vtss_ts_status_change(NULL,port_no, info->speed) != VTSS_RC_OK) {
                T_I("vtss_phy_ts_phy_status_change failed");
            } else {
                T_I("port_change link up, port %d, speed %d", port_no, info->speed);
            }
        }
    } else {
        T_I("port_change link down  or stack port");
    }
}




vtss_rc
tod_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        tod_global.ready = FALSE;
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        critd_init(&tod_global.datamutex, "tod_data", VTSS_MODULE_ID_TOD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        TOD_DATA_UNLOCK();
        //critd_init(&tod_global.coremutex, "tod_core", VTSS_MODULE_ID_TOD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        //TOD_CORE_UNLOCK();
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        TOD_RC(ptp_module_man_init());
#ifdef VTSS_SW_OPTION_VCLI
        tod_cli_init();
#endif
#endif        
        T_I("INIT_CMD_INIT TOD" );
        break;
    case INIT_CMD_START:
        T_I("INIT_CMD_START TOD");
        break;
    case INIT_CMD_CONF_DEF:
        tod_conf_read(TRUE);
        tod_conf_propagate();
        T_I("INIT_CMD_CONF_DEF TOD" );
        break;
    case INIT_CMD_MASTER_UP:
        T_I("INIT_CMD_MASTER_UP ");
        tod_conf_read(FALSE);
#ifdef CALC_TEST
        calcTest();
#endif
        break;
    case INIT_CMD_SWITCH_ADD:
        if (isid == VTSS_ISID_START) {
#if defined(VTSS_ARCH_SERVAL)
            vtss_ts_internal_mode_t mode;
            mode.int_fmt = serval_internal();
            T_D("Internal mode.int_fmt: %d", mode.int_fmt);
            TOD_RC(vtss_ts_internal_mode_set(NULL, &mode));
#endif

#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
            vtss_phy_ts_clockfreq_t freq;
            if (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF) {
                if (tod_ref_clock_freg_get(&freq)) {
                    if (freq < VTSS_PHY_TS_CLOCK_FREQ_MAX) {
                        // update the clock divider in the Si5338 clock generator for clock 0.
                        // Fortunately the can set to 125, 156,25 or 250 MHz by only changing the CLK0_INTDIV register.
                        // For other frequencies we need to change the internal frequency, which will disturb the other clock 
                        // outputs from the clock generator, including the CPU clock. Therefore only these frequencies are supported.
                        CYG_I2C_SI5338_DEVICE(si_5338_device,SI5338_I2C_ADDR);
                        u8 buf[MAX_5338_I2C_LEN];
                        buf[0] = SI5338_REG_CLK0_INTDIV_15_8;  /* reg addr */
                        buf[1] = freq == VTSS_PHY_TS_CLOCK_FREQ_125M ? 8 :
                                 freq == VTSS_PHY_TS_CLOCK_FREQ_15625M ? 6 : 3;
                        if (MAX_5338_I2C_LEN != cyg_i2c_tx(&si_5338_device,&buf[0],MAX_5338_I2C_LEN)) {
                            T_W("Error writing to si_5338_device");
                        }
                        T_I("The 1588 reference clock is set");
                    }
                }
            }
#endif 
            (void) port_global_change_register(VTSS_MODULE_ID_TOD, tod_ts_port_state_change_callback);
            
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
            TOD_RC(ptp_module_man_resume());
#endif 
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            TOD_RC(vtss_interrupt_source_hook_set(one_pps_pulse_interrupt_handler,
                                                  INTERRUPT_SOURCE_SYNC,
                                                  INTERRUPT_PRIORITY_NORMAL));
#endif
            tod_global.ready = TRUE;
        } else {
            T_E("INIT_CMD_SWITCH_ADD - unknown ISID %u", isid);
        }
        T_I("INIT_CMD_SWITCH_ADD - ISID %u", isid);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("INIT_CMD_MASTER_DOWN - ISID %u", isid);
        break;
    default:
        break;
    }

    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
