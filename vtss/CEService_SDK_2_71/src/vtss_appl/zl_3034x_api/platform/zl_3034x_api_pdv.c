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
#include "vtss_types.h"
#include "vtss_api.h"
#include "zl_3034x_api_pdv.h"
#include "zl_3034x_api_pdv_api.h"
#include "zl_3034x_api_api.h"
#include "critd_api.h"
#include "conf_api.h"

// xl3034x base interface 
#include "zl303xx_Error.h"
#include "zl303xx_Apr.h"
#include "zl303xx_Api.h"
#include "zl303xx_Var.h"
#include "zl303xx_LogToMsgQ.h"
#include "zl303xx_ExampleAprBinding.h"
#include "zl303xx_DebugApr.h"
#include "zl303xx_DebugMisc.h"
#include "zl303xx_ExampleApr.h"
#include "zl303xx_AprStatistics.h"
#include "zl303xx_TodMgrApi.h"
#include "zl303xx_Apr1Hz.h"
#include "interrupt_api.h"

/* interface to other components */
#include "vtss_tod_api.h"
#include "vtss_ptp_local_clock.h"

#ifdef VTSS_SW_OPTION_CLI
#include "zl_3034x_api_pdv_cli.h"
#endif

extern zl303xx_ParamsS *zl_3034x_zl303xx_params; /* device instance returned from zl303xx_CreateDeviceInstance*/


/* ================================================================= *
 *  Trace definitions
 * ================================================================= */


#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "zl_3034x",
    .descr     = "ZL3034x Filter algorithm Module."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (ZL3034x core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical Regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_OS_PORT] = {
        .name      = "os_port",
        .descr     = "ZL OS porting functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_PTP_INTF] = {
        .name      = "ptp_intf",
        .descr     = "ZL-PTP application interfacefunctions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_ZL_TRACE] = {
        .name      = "zl-trace",
        .descr     = "ZL-code trace level",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

#ifdef VTSS_ARCH_LUTON26
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 10
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl_pdv, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#endif

#ifdef VTSS_ARCH_JAGUAR_1
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 17
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl_pdv, 2, VTSS_SPI_GPIO_NONE);
#endif

#ifdef VTSS_ARCH_SERVAL
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 6
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl_pdv, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#endif

static struct {
    BOOL ready;                 /* ZL3034X Initited  */
    critd_t datamutex;          /* Global data protection */
} zl3034x_global;

/* ================================================================= *
 *  Persistent Configuration definitions
 * ================================================================= */

typedef struct zl3034x_config_t {
    i8 version;             /* configuration version, to be changed if this structure is changed */
    zl303xx_AprAlgTypeModeE algTypeMode;
    zl303xx_AprOscillatorFilterTypesE oscillatorFilter;
    zl303xx_AprFilterTypesE filter;
    zl303xx_BooleanE enable1Hz;
    zl303xx_BooleanE bHoldover;
    BOOL apr_server_notify_flag;
    u32 mode;
    u32 adj_freq_min_phase;
} zl3034x_config_t;

static zl3034x_config_t config_data ;
/*
 * Propagate the zl3034x (module) configuration to the zl3034x core
 * library.
 */
static void
zl3034x_conf_propagate(void)
{
    T_I("zl3034x Configuration has been reset, you need to reboot to activate the changed conf." );
}

static void
zl3034x_conf_save(void)
{
#if 0
    zl3034x_config_t    *conf;  /* run-time configuration data */
    ulong size;
    if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ZL3034X_CONF, &size)) != NULL) {
        if (size == sizeof(*conf)) {
            T_IG(0, "Saving configuration");
            ZL_3034X_DATA_LOCK();
            *conf = config_data;
            ZL_3034X_DATA_UNLOCK();
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ZL3034X_CONF);
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
zl3034x_conf_read(BOOL create)
{
    //ulong           size;
    BOOL            do_create;
    //if ((conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ZL3034X_CONF, &size)) == NULL ||
    //        size != sizeof(*conf)) {
    //    conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_ZL3034X_CONF, sizeof(*conf));
    //    T_W("conf_sec_open failed or size mismatch, creating defaults");
    //    T_W("actual size = %d,expected size = %d", size, sizeof(*conf));
    //    do_create = TRUE;
    //} else if (conf->version != ZL3034X_CONF_VERSION) {
    //    T_W("version mismatch, creating defaults");
    //    do_create = TRUE;
    //} else {
    //    do_create = create;
    //}
    do_create = TRUE;
    ZL_3034X_DATA_LOCK();
    if (do_create) {
        /* initialize run-time options to reasonable values */
        config_data.version = ZL3034X_CONF_VERSION;
        config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ;
        config_data.oscillatorFilter = ZL303XX_OCXO_S3E;
        config_data.filter = ZL303XX_BW_0_FILTER;
        config_data.enable1Hz = ZL303XX_FALSE;
        config_data.bHoldover = ZL303XX_FALSE;
        config_data.apr_server_notify_flag = FALSE;
        config_data.mode = 1;
        config_data.adj_freq_min_phase = 20; /* zl default value */
        //*conf = config_data;
		//if (zl3034x_conf_changed) {
        //	T_W("ZL_3034X Configuration has been reset, you need to reboot to activate the changed conf." );
        //}
		
    }
    ZL_3034X_DATA_UNLOCK();

#if 0
    if (conf) {
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ZL3034X_CONF);
    }
#endif

}

/* ================================================================= *
 *  Configuration definitions
 * ================================================================= */
typedef struct
{
    void *cguId;            
    zl303xx_ParamsS *zl303xx_Params; /* device instance returned from zl303xx_CreateDeviceInstance*/

    zl303xx_AprAddDeviceS device;
    zl303xx_AprAddServerS server;

} apr_clock_createS;
static apr_clock_createS apr_clock;
static BOOL zarlink=FALSE;
static BOOL first_time = TRUE;

static char *apr_alg_type_2_txt(zl303xx_AprAlgTypeModeE value)
{
    switch (value) {
        case ZL303XX_NATIVE_PKT_FREQ                   : return "NATIVE_PKT_FREQ";
        case ZL303XX_NATIVE_PKT_FREQ_UNI               : return "NATIVE_PKT_FREQ_UNI";
        case ZL303XX_NATIVE_PKT_FREQ_CES               : return "NATIVE_PKT_FREQ_CES";
        case ZL303XX_NATIVE_PKT_FREQ_ACCURACY          : return "NATIVE_PKT_FREQ_ACCURACY";
        case ZL303XX_NATIVE_PKT_FREQ_ACCURACY_UNI      : return "NATIVE_PKT_FREQ_ACCURACY_UNI";
        case ZL303XX_NATIVE_PKT_FREQ_FLEX              : return "NATIVE_PKT_FREQ_FLEX";
        case ZL303XX_BOUNDARY_CLK                      : return "BOUNDARY_CLK";
        case ZL303XX_NATIVE_PKT_FREQ_ACCURACY_FDD      : return "NATIVE_PKT_FREQ_ACCURACY_FDD";
        case ZL303XX_XDSL_FREQ_ACCURACY                : return "XDSL_FREQ_ACCURACY";
        case ZL303XX_CUSTOM_FREQ_ACCURACY_200          : return "CUSTOM_FREQ_ACCURACY_200";
        case ZL303XX_CUSTOM_FREQ_ACCURACY_15           : return "CUSTOM_FREQ_ACCURACY_15";
    }
    return "INVALID";
}

static char *apr_osc_filter_type_2_txt(zl303xx_AprOscillatorFilterTypesE value)
{
    switch (value) {
        case ZL303XX_TCXO      : return "TCXO";
        case ZL303XX_TCXO_FAST : return "TCXO_FAST";
        case ZL303XX_OCXO_S3E  : return "OCXO_S3E";
    }
    return "INVALID";
}

static char *apr_filter_type_2_txt(zl303xx_AprFilterTypesE value)
{
    switch (value) {
        case ZL303XX_BW_0_FILTER      : return "BW_0_FILTER";
        case ZL303XX_BW_1_FILTER      : return "BW_1_FILTER";
        case ZL303XX_BW_2_FILTER      : return "BW_2_FILTER";
        case ZL303XX_BW_3_FILTER      : return "BW_3_FILTER";
        case ZL303XX_BW_4_FILTER      : return "BW_4_FILTER";
    }
    return "INVALID";
}



/****************************************************************************
 * Configuration API
 ****************************************************************************/
#include "zl303xx_TsEngInternal.h"

vtss_rc zl_3034x_event_enable_set(zl_3034x_event_t events,  BOOL enable)
{
    if (!zarlink) return(VTSS_RC_OK);
    if (first_time) {
        T_D("not ready to handle interrupt");
         return(VTSS_RC_OK);
    }

    if (events & ZL_TOP_ISR_REF_TS_ENG) {
/*lint -e{506}  The Zarlink ZL303XX_TOP_ISR0_MASK_REG macro is calling a macro with a Constant value BOOL - we do not want to change that macro */
        if (zl303xx_ReadModWrite(apr_clock.zl303xx_Params, NULL, ZL303XX_TOP_ISR0_MASK_REG, (enable ? ZL303XX_TOP_ISR_REF_TS_ENG : 0), ZL303XX_TOP_ISR_REF_TS_ENG, NULL) != ZL303XX_OK)
            T_D("Error during Zarlink read mod write");
    }

    return(VTSS_RC_OK);
}

vtss_rc zl_3034x_event_poll(zl_3034x_event_t *const events)
{
    Uint32T  isr_reg = 0;

    if (!zarlink) return(VTSS_RC_OK);
    if (first_time) {
        T_D("not ready to handle interrupt");
        return(VTSS_RC_OK);
    }

    *events = 0;
/*lint -e{506}  The Zarlink ZL303XX_TOP_ISR_REG macro is calling a macro with a Constant value BOOL - we do not want to change that macro */
    if (zl303xx_Read(apr_clock.zl303xx_Params, NULL, ZL303XX_TOP_ISR_REG, &isr_reg) != ZL303XX_OK)
        T_D("Error during Zarlink read");
    else {
        if (isr_reg & ZL303XX_TOP_ISR_REF_TS_ENG)
            *events = ZL_TOP_ISR_REF_TS_ENG;
    }
    return(VTSS_RC_OK);
}




/*
 * Process timestamps received in the PTP protocol.
 */
BOOL  zl_3034x_process_timestamp(vtss_zl_3034x_process_timestamp_t *ts)
{
    zl303xx_AprTimestampS aprTs;
    zlStatusE status = ZL303XX_OK;
    BOOL rc = TRUE;

    if (!zarlink) return(rc);

    memset(&aprTs, 0, sizeof(aprTs));
#if 1  // problem with Zarlink corrfield
    if (ts->corr != 0LL) {
        vtss_tod_add_TimeInterval(&ts->tx_ts, &ts->tx_ts, &ts->corr);
        ts->corr = 0LL;
    }
#endif
    aprTs.serverId = apr_clock.server.serverId;
    aprTs.txTs.second.lo = ts->tx_ts.seconds;
    aprTs.txTs.second.hi = ts->tx_ts.sec_msb;
    aprTs.txTs.subSecond = ts->tx_ts.nanoseconds;
    aprTs.rxTs.second.lo = ts->rx_ts.seconds;
    aprTs.rxTs.second.hi = ts->rx_ts.sec_msb;
    aprTs.rxTs.subSecond = ts->rx_ts.nanoseconds;
    aprTs.bForwardPath = ts->fwd_path;
    
    aprTs.corr.hi = ts->corr>>32;
    aprTs.corr.lo = ts->corr & 0xffffffff;
    
    
    
    T_NG(TRACE_GRP_PTP_INTF, "aprTs txTs %u:%u, rxTs %u:%u, fwd %d, corr %u:%u (%lld ns)", aprTs.txTs.second.lo, aprTs.txTs.subSecond, 
                aprTs.rxTs.second.lo, aprTs.rxTs.subSecond, aprTs.bForwardPath,
                aprTs.corr.hi, aprTs.corr.lo, ts->corr>>16);
    status = zl303xx_AprProcessTimestamp(apr_clock.zl303xx_Params, &aprTs);
    if (status == ZL303XX_OK) {
        T_DG(TRACE_GRP_PTP_INTF, "timestamp updated");
    } else {
        T_WG(TRACE_GRP_PTP_INTF, "zl303xx_AprProcessTimestamp failed with status = %u", status);
        rc = FALSE;
    }
    return rc;
}

vtss_rc zl_3034x_apr_device_status_get(void)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_GetAprDeviceStatus(apr_clock.zl303xx_Params) != ZL303XX_OK) {
        T_D("Error during Zarlink APR device status get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_apr_server_config_get(void)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_GetAprServerConfigInfo(apr_clock.zl303xx_Params,
                        apr_clock.server.serverId) != ZL303XX_OK) {
        T_D("Error during Zarlink APR server config get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_apr_server_status_get(void)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_GetAprServerStatus(apr_clock.zl303xx_Params,
                        apr_clock.server.serverId) != ZL303XX_OK) {
        T_D("Error during Zarlink APR server status get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_apr_force_holdover_set(BOOL enable)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    ZL_3034X_DATA_LOCK();
    config_data.bHoldover = enable ? ZL303XX_TRUE : ZL303XX_FALSE;
    if (zl303xx_AprSetHoldover(apr_clock.zl303xx_Params, config_data.bHoldover) != ZL303XX_OK) {
        T_D("Error during Zarlink APR force holdover set");
        rc = VTSS_RC_ERROR;
    }
    ZL_3034X_DATA_UNLOCK();
    if (rc == VTSS_RC_OK)
        zl3034x_conf_save();
    return(rc);
}

vtss_rc zl_3034x_apr_force_holdover_get(BOOL *enable)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    ZL_3034X_DATA_LOCK();
    *enable = config_data.bHoldover ? TRUE : FALSE;
    ZL_3034X_DATA_UNLOCK();
    return(rc);
}

vtss_rc zl_3034x_apr_statistics_get(void)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_DebugGetAllAprStatistics(apr_clock.zl303xx_Params) != ZL303XX_OK) {
        T_D("Error during Zarlink APR Statistics get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_apr_statistics_reset(void)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_AprResetPerfStatistics(apr_clock.zl303xx_Params) != ZL303XX_OK) {
        T_D("Error during Zarlink APR Statistics reset");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

static void apr_server_notify(zl303xx_AprServerNotifyS *msg)
{
    ZL_3034X_DATA_LOCK();
    if (config_data.apr_server_notify_flag == TRUE) {
        exampleAprServerNotify(msg);
    }
    ZL_3034X_DATA_UNLOCK();
    return;
}

vtss_rc apr_server_notify_set(BOOL enable)
{
    ZL_3034X_DATA_LOCK();
    config_data.apr_server_notify_flag = enable;
    ZL_3034X_DATA_UNLOCK();
    zl3034x_conf_save();
    return(VTSS_RC_OK);
}

vtss_rc apr_server_notify_get(BOOL *enable)
{
    ZL_3034X_DATA_LOCK();
    *enable = config_data.apr_server_notify_flag;
    ZL_3034X_DATA_UNLOCK();
    return(VTSS_RC_OK);
}


vtss_rc apr_server_one_hz_set(BOOL enable)
{
    vtss_rc rc = VTSS_RC_OK;
    
    ZL_3034X_DATA_LOCK();
    config_data.enable1Hz = enable;
    T_D("Set 1 Hz mode if %p != NULL",apr_clock.cguId);
    if (apr_clock.cguId != NULL) {
        if (zl303xx_AprSetDevice1HzEnabled(apr_clock.zl303xx_Params, enable) != ZL303XX_OK) {
            rc = VTSS_RC_ERROR;
        }
    }    
    ZL_3034X_DATA_UNLOCK();
    if (rc == VTSS_RC_OK)
        zl3034x_conf_save();
    return(rc);
}

vtss_rc apr_server_one_hz_get(BOOL *enable)
{
    ZL_3034X_DATA_LOCK();
    *enable = config_data.enable1Hz;
    ZL_3034X_DATA_UNLOCK();
    return(VTSS_RC_OK);
}

/**

  Function Name:
    apr_set_config_parameters

  Details:
   Example code to set global APR parameters for the given config ID.

  Parameters:
   [in]   hwParams      pointer to the device instance
   [in]   configId      the given config ID


  Return Value:
   zlStatusE

 *******************************************************************************/
static zlStatusE apr_set_config_parameters(
            void *hwParams,
            exampleAprConfigIdentifiersE configId)
{
   zlStatusE status = ZL303XX_OK;
   if( status == ZL303XX_OK ) {
      status = ZL303XX_CHECK_POINTER(hwParams);
   }

   if( status == ZL303XX_OK ) {
      if( configId >= ACI_last ) {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }
   if( status == ZL303XX_OK ) {
      switch( configId )
      {
         case ACI_FREQ_TCXO:
            /* APR */
            config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ;
            config_data.oscillatorFilter = ZL303XX_TCXO;
            config_data.filter = ZL303XX_BW_0_FILTER;
            /* 1Hz: disabled */
            config_data.enable1Hz = ZL303XX_FALSE;
            break;
         case ACI_FREQ_OCXO_S3E:
            /* APR */
            config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ;
            config_data.oscillatorFilter = ZL303XX_OCXO_S3E;
            config_data.filter = ZL303XX_BW_0_FILTER;
            /* 1Hz: disabled */
            config_data.enable1Hz = ZL303XX_FALSE;
            break;
         case ACI_BC_PARTIAL_ON_PATH_FREQ:
            /* APR */
            config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ_FLEX;
            config_data.filter = ZL303XX_BW_0_FILTER;
            /* 1Hz: disabled */
            config_data.enable1Hz = ZL303XX_FALSE;
            break;
         case ACI_BC_PARTIAL_ON_PATH_PHASE:
            /* APR */
            config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ_FLEX;
            config_data.filter = ZL303XX_BW_0_FILTER;
            /* 1Hz: enabled */
            config_data.enable1Hz = ZL303XX_TRUE;
            /* NOTE: it is recommended that the realignment interval be set to 300s */
            break;
         case ACI_BC_PARTIAL_ON_PATH_SYNCE:
            /* APR - not shown because this is hybrid */
            /* hybrid:  not supported*/
            status = ZL303XX_INVALID_MODE;
            /* NOTE: it is recommended that the realignment interval be set to 10s */
            break;
         case ACI_BC_FULL_ON_PATH_FREQ:
            /* APR */
            config_data.algTypeMode = ZL303XX_BOUNDARY_CLK;
            config_data.filter = ZL303XX_BW_1_FILTER;
            /* 1Hz: disabled */
            config_data.enable1Hz = ZL303XX_FALSE;
            break;
         case ACI_BC_FULL_ON_PATH_PHASE:
            /* APR */
            config_data.algTypeMode = ZL303XX_BOUNDARY_CLK;
            config_data.filter = ZL303XX_BW_1_FILTER;
            /* 1Hz: enabled */
            config_data.enable1Hz = ZL303XX_TRUE;
            break;
         case ACI_BC_FULL_ON_PATH_SYNCE:
            /* APR - not shown because this is hybrid */
            /* hybrid: not supported */
            status = ZL303XX_INVALID_MODE;
            /* NOTE: it is recommended that the realignment interval be set to 10s */
            break;
         case ACI_FREQ_ACCURACY_FDD:
            /* APR */
            config_data.algTypeMode = ZL303XX_NATIVE_PKT_FREQ_ACCURACY_FDD;
            config_data.filter = ZL303XX_BW_2_FILTER;
            /* 1Hz: disabled */
            config_data.enable1Hz = ZL303XX_FALSE;
            break;
         case ACI_last:
         default:
            break;
      }
   }

   return( status );
}


vtss_rc apr_config_parameters_set(u32 mode)
{
    vtss_rc rc = VTSS_RC_OK;
    ZL_3034X_DATA_LOCK();
    if (apr_set_config_parameters(apr_clock.zl303xx_Params,
                                  (exampleAprConfigIdentifiersE) mode) != ZL303XX_OK) {
        rc = VTSS_RC_ERROR;
    } else {
        config_data.mode = mode;
    }
    ZL_3034X_DATA_UNLOCK();
    if (rc == VTSS_RC_OK) {
        zl3034x_conf_save();
    }
    return(rc);
}

vtss_rc apr_config_parameters_get(u32 *mode, char *zl_config)
{
    vtss_rc rc = VTSS_RC_OK;
    ZL_3034X_DATA_LOCK();
    *mode = config_data.mode;
    sprintf(zl_config, "server algorithm %s, oscillator %s, filter %s",apr_alg_type_2_txt(apr_clock.server.algTypeMode),
            apr_osc_filter_type_2_txt(apr_clock.server.oscillatorFilterType), 
            apr_filter_type_2_txt(apr_clock.server.filterType));
    ZL_3034X_DATA_UNLOCK();
    return(rc);
}

vtss_rc apr_adj_min_set(u32 adj)
{
    vtss_rc rc = VTSS_RC_OK;
    ZL_3034X_DATA_LOCK();
    config_data.adj_freq_min_phase = adj;
    ZL_3034X_DATA_UNLOCK();
    zl3034x_conf_save();
    return(rc);
}

vtss_rc apr_adj_min_get(u32 *adj)
{
    vtss_rc rc = VTSS_RC_OK;
    ZL_3034X_DATA_LOCK();
    *adj = config_data.adj_freq_min_phase;
    ZL_3034X_DATA_UNLOCK();
    return(rc);
}


vtss_rc zl_3034x_apr_config_dump(void)
{
    vtss_rc rc = VTSS_RC_OK;

  /* Debugging Api Calls */

    printf("### zl303xx_GetAprDeviceConfigInfo\n");
    if (ZL303XX_OK != zl303xx_GetAprDeviceConfigInfo(apr_clock.zl303xx_Params)) rc = VTSS_RC_ERROR;
    printf("### zl303xx_GetAprServerConfigInfo\n");
    if (ZL303XX_OK != zl303xx_GetAprServerConfigInfo(apr_clock.zl303xx_Params, 0)) rc = VTSS_RC_ERROR;
    printf("### zl303xx_DebugDpllConfig\n");
    if (ZL303XX_OK != zl303xx_DebugDpllConfig(apr_clock.zl303xx_Params, 1)) rc = VTSS_RC_ERROR;
    printf("### zl303xx_DebugDpllStatus\n");
    if (ZL303XX_OK != zl303xx_DebugDpllStatus(apr_clock.zl303xx_Params, 1)) rc = VTSS_RC_ERROR;
    printf("### zl303xx_DebugAprGet1HzData FWD Path\n");
    if (ZL303XX_OK != zl303xx_DebugAprGet1HzData(apr_clock.zl303xx_Params, 0, 0)) rc = VTSS_RC_ERROR;
    printf("### 1Hz data FWD ####\n");
    if (ZL303XX_OK != zl303xx_AprPrint1HzData(apr_clock.zl303xx_Params, 0, 0)) rc = VTSS_RC_ERROR;
    printf("### zl303xx_DebugAprGet1HzData  Rev Path\n");
    if (ZL303XX_OK != zl303xx_DebugAprGet1HzData(apr_clock.zl303xx_Params, 1, 0)) rc = VTSS_RC_ERROR;
    printf("### 1Hz data REV ####\n");
    if (ZL303XX_OK != zl303xx_AprPrint1HzData(apr_clock.zl303xx_Params, 1, 0)) rc = VTSS_RC_ERROR;
    printf("### Other Params ####\n");
    if (ZL303XX_OK != zl303xx_DebugPrintAprByReferenceId   (apr_clock.zl303xx_Params, 0)) rc = VTSS_RC_ERROR;
    return(rc);
}


vtss_rc zl_3034x_apr_log_level_set(u8 level)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (zl303xx_SetAprLogLevel(level) != ZL303XX_OK) {
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_apr_log_level_get(u8 *level)
{
    vtss_rc rc = VTSS_RC_OK;

    *level = 0;

    if (!zarlink) return(VTSS_RC_OK);

    *level = zl303xx_GetAprLogLevel();
    return(rc);
}

vtss_rc zl_3034x_apr_ts_log_level_set(u8 level)
{
    vtss_rc rc = VTSS_RC_OK;

    if (!zarlink) return(VTSS_RC_OK);

    if (level <= 2) {
        zl303xx_AprLogTimestampInputStart(level);
    } else {
        zl303xx_AprLogTimestampInputStop();
    }
    return(rc);
}

vtss_rc zl_3034x_apr_ts_log_level_get(u8 *level)
{
    vtss_rc rc = VTSS_RC_OK;
    *level = 3; // tbd (saved persistent)
    return(rc);
}
/*
 * Process packet rate indications received in the PTP protocol.
 */

static  zl303xx_AprPktRateE rate_table [] = {
          ZL303XX_128_PPS,
          ZL303XX_64_PPS,
          ZL303XX_32_PPS,
          ZL303XX_16_PPS,
          ZL303XX_8_PPS,
          ZL303XX_4_PPS,
          ZL303XX_2_PPS,
          ZL303XX_1_PPS,
          ZL303XX_1_PP_2S,
          ZL303XX_1_PP_4S,
          ZL303XX_1_PP_8S,
          ZL303XX_1_PP_16S};
static i8 current_rate [] = {5,5};
      
BOOL  zl_3034x_packet_rate_set(i8 ptp_rate, BOOL forward)
{
    zlStatusE status = ZL303XX_OK;
    zl303xx_BooleanE b_fwd;
    zl303xx_AprPktRateE new_rate = ZL303XX_1_PPS;
    BOOL rc = TRUE;
    
    if (!zarlink) return(rc);
    if (ptp_rate == 0x7f) {
        if (forward) ptp_rate = -6;
        else ptp_rate = -4;
    }
        
    if (ptp_rate != current_rate[forward ? 0 : 1]) {
        if (ptp_rate < -7 || ptp_rate > 4) {
            ptp_rate = 0;
        }
        current_rate[forward ? 0 : 1] = ptp_rate;
        new_rate = rate_table[ptp_rate+7];
        b_fwd = forward;
        status = zl303xx_AprNotifyServerPktRateChange(apr_clock.zl303xx_Params,
                      apr_clock.server.serverId, b_fwd, new_rate);
        if (status == ZL303XX_OK) {
            T_IG(TRACE_GRP_PTP_INTF, "packet rate updated, rate = %d , forw = %d", new_rate, b_fwd);
        } else {
            T_WG(TRACE_GRP_PTP_INTF, "zl303xx_AprNotifyServerPktRateChange failed with status = %u", status);
            rc = FALSE;
        }
    }
    return rc;
}

BOOL zl_3034x_pdv_status_get(u32 *pdv_clock_state, i32 *freq_offset) 
{
    BOOL rc = TRUE;
    zl303xx_AprServerStatusFlagsS status_flags;
    zlStatusE status = ZL303XX_OK;
    status = zl303xx_AprGetServerStatusFlags(apr_clock.zl303xx_Params,
                                    apr_clock.server.serverId,&status_flags);
    if (status != ZL303XX_OK) {
        T_WG(TRACE_GRP_PTP_INTF, "zl303xx_AprGetServerStatusFlags failed with status = %u", status);
        rc = FALSE;
    }
    switch (status_flags.state) { 
        case ZL303XX_FREQ_LOCK_ACQUIRING:   *pdv_clock_state = ZL_3034X_PDV_FREQ_LOCKING; break;
        case ZL303XX_FREQ_LOCK_ACQUIRED:    *pdv_clock_state = ZL_3034X_PDV_FREQ_LOCKED; break;
        case ZL303XX_PHASE_LOCK_ACQUIRED:   *pdv_clock_state = ZL_3034X_PDV_PHASE_LOCKED; break;
        case ZL303XX_HOLDOVER:              *pdv_clock_state = ZL_3034X_PDV_PHASE_HOLDOVER; break;
        case ZL303XX_REF_FAILED:
        case ZL303XX_UNKNOWN:
        case ZL303XX_MANUAL_FREERUN:
        case ZL303XX_MANUAL_HOLDOVER:
        case ZL303XX_MANUAL_SERVO_HOLDOVER: *pdv_clock_state =  ZL_3034X_PDV_INITIAL; break;
    }
       
    status = zl303xx_AprGetServerFreqOffset(apr_clock.zl303xx_Params,
                 freq_offset);
    if (status != ZL303XX_OK) {
        T_WG(TRACE_GRP_PTP_INTF, "zl303xx_AprGetServerFreqOffset failed with status = %u", status);
        rc = FALSE;
    }
    return rc;
        
}

static char *state_2_txt(zl303xx_AprStateE value)
{
    switch (value) {
        case ZL303XX_FREQ_LOCK_ACQUIRING: return "FREQ_LOCK_ACQUIRING";
        case ZL303XX_FREQ_LOCK_ACQUIRED: return "FREQ_LOCK_ACQUIRED";
        case ZL303XX_PHASE_LOCK_ACQUIRED: return "PHASE_LOCK_ACQUIRED";
        case ZL303XX_HOLDOVER : return "HOLDOVER ";
        case ZL303XX_REF_FAILED: return "REF_FAILED";
        case ZL303XX_UNKNOWN: return "UNKNOWN";
        case ZL303XX_MANUAL_FREERUN: return "MANUAL_FREERUN";
        case ZL303XX_MANUAL_HOLDOVER: return "MANUAL_HOLDOVER";
        case ZL303XX_MANUAL_SERVO_HOLDOVER: return "MANUAL_SERVO_HOLDOVER";
    }
    return "INVALID";
}

static void apr_cgu_notify(zl303xx_AprCGUNotifyS *msg)
{

    if(msg->type == ZL303XX_CGU_STATE) {
        T_I("STATE flag changed to: %s", state_2_txt(msg->flags.state));
    }
    exampleAprCguNotify(msg);
}


/****************************************************************************
 * Callbacks
 ****************************************************************************/
static void top_isr_interrupt_function(vtss_interrupt_source_t   source_id,
                                       u32                       instance_id)
{
//static u32 count=0;
//count++;
//if (count < 10) printf("top_isr_interrupt_function\n");
    if (vtss_interrupt_source_hook_set(top_isr_interrupt_function, source_id, INTERRUPT_PRIORITY_HIGHEST) != VTSS_RC_OK)
        T_D("Error during interrupt hook");

    (void)zl303xx_TsEngInterruptHandler(apr_clock.zl303xx_Params);
}



/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
static Sint32T apr_set_time_tsu(void *clkGenId, Uint64S deltaTimeSec, Uint32T deltaTimeNanoSec,
                             zl303xx_BooleanE negative)
{
    zlStatusE status = ZL303XX_OK;
    vtss_timestamp_t t;
    
    if (!clkGenId) {;} /* Warning removal */
    t.nanoseconds = deltaTimeNanoSec;
    t.seconds = deltaTimeSec.lo;
    t.sec_msb = deltaTimeSec.hi;
    
    T_IG(TRACE_GRP_PTP_INTF, "Delta time: %u.%u:%u, negative %d", t.sec_msb, t.seconds, t.nanoseconds, negative);
    vtss_tod_settimeofday_delta(&t, negative);

    return (Sint32T)status;
}

static Sint32T apr_step_time_tsu(void *clkGenId, Sint32T deltaTimeNs)
{
    zlStatusE status = ZL303XX_OK;

    if (!clkGenId) {;} /* Warning removal */

    T_IG(TRACE_GRP_PTP_INTF, "Delta nanosec: %d", deltaTimeNs);
    /* the vtss_local_clock_adj_offset subtracts the time from current time */
    vtss_local_clock_adj_offset(-deltaTimeNs, 0);

    return (Sint32T)status;
}


static Sint32T apr_adj_freq_tsu(void *clkGenId, Sint32T deltaFreq)
{
    zlStatusE status = ZL303XX_OK;

    if (!clkGenId) {;} /* Warning removal */

    T_WG(TRACE_GRP_PTP_INTF, "AdjustFreq: %d ppt", deltaFreq);

    return (Sint32T)status;
}


/* apr_stream_create */
/**
   An example of how to start a APR stream/server

*******************************************************************************/
static zlStatusE apr_stream_create(apr_clock_createS *pStream)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AprConfigure1HzS config1Hz;
   zl303xx_BooleanE bSinglePath = ZL303XX_FALSE;

   if ((status = zl303xx_AprAddServerStructInit(&pStream->server)) != ZL303XX_OK)
   {
       T_E("zl303xx_AprAddServerStructInit() failed with status = %u", status);
   }
   pStream->server.serverId = 0;
   pStream->server.packetDiscardDurationInSec = 5;  // 5 sec is selected because the PHY's needs 4 sec to become synchronized 
   
   pStream->server.algTypeMode = config_data.algTypeMode;
   pStream->server.oscillatorFilterType = config_data.oscillatorFilter;
   pStream->server.filterType = config_data.filter;
   
   T_I("server algorithm %s, oscillator %s, filter %s",apr_alg_type_2_txt(pStream->server.algTypeMode),
       apr_osc_filter_type_2_txt(pStream->server.oscillatorFilterType), 
       apr_filter_type_2_txt(pStream->server.filterType));
   if ((status == ZL303XX_OK) &&
        (status = zl303xx_AprAddServer(pStream->cguId, &pStream->server)) != ZL303XX_OK)
   {
       T_E("zl303xx_AprAddServer(%p, %p) failed with status = %u",
           pStream->cguId, &pStream->server, status);
   }

   if (config_data.mode == 1 && config_data.enable1Hz) {
       /***********************
         *
         *  Code added from here suggested by Mirosemi (Marcel)
         */
       T_I("set up extra parameters for mode 1 phase");

       /* Get default 1Hz defaults */
       if (status == ZL303XX_OK)
       {
          status = zl303xx_AprGetServer1HzConfigData(pStream->cguId, pStream->server.serverId, ZL303XX_TRUE, &config1Hz);    /* Fwd Path */
       }

       /* Do ForwardPath */
       if (status == ZL303XX_OK)
       {
           /* 1Hz default configuration:
            * config1Hz.realignmentType = ZL303XX_1HZ_REALIGNMENT_PERIODIC;           - Realignment is performed periodically.
            * config1Hz.realignmentInterval = 120;                                  - The interval between performing
            *                                                                         realignment is set to 120 seconds.
            */
           /* config1Hz.disable = ZL303XX_FALSE;  */
           config1Hz.realignmentInterval = 300;
           config1Hz.xParams.p[2] = 64*(config1Hz.realignmentInterval-7);
           config1Hz.xParams.p[5] = ZL303XX_FALSE;
           config1Hz.xParams.p[9] = 488;
           config1Hz.xParams.p[10] = 2;
           config1Hz.xParams.p[12] = (config1Hz.realignmentInterval - 30);

           status = zl303xx_AprConfigServer1Hz(pStream->cguId, pStream->server.serverId, ZL303XX_TRUE, &config1Hz);    /* Fwd Path */
           T_I("fwd path zl303xx_AprConfigServer1Hz returned %x",status);
       }

       /* Check if using reverse path or not */
       if (status == ZL303XX_OK)
       {
           status = zl303xx_AprIsServerUseSinglePath(pStream->cguId, pStream->server.serverId, &bSinglePath);
       }

       if ((status == ZL303XX_OK) && (bSinglePath == ZL303XX_FALSE))
       {
           /* If the reverse path is used, configure the reverse path */
           status = zl303xx_AprGetServer1HzConfigData(pStream->cguId, pStream->server.serverId, ZL303XX_FALSE, &config1Hz);

           if (status == ZL303XX_OK)
           {
              /* 1Hz default configuration:
               * config1Hz.realignmentType = ZL303XX_1HZ_REALIGNMENT_PERIODIC;           - Realignment is performed periodically.
               * config1Hz.realignmentInterval = 120;                                  - The interval between performing
               *                                                                         realignment is set to 120 seconds.
               */
               /* config1Hz.disable = ZL303XX_FALSE;  */
               config1Hz.realignmentInterval = 300;
               config1Hz.xParams.p[2] = 16*(config1Hz.realignmentInterval-6);
               config1Hz.xParams.p[5] = ZL303XX_FALSE;
               config1Hz.xParams.p[9] = 488;
               config1Hz.xParams.p[10] = 5;
               config1Hz.xParams.p[12] = (config1Hz.realignmentInterval - 30);

               status = zl303xx_AprConfigServer1Hz(pStream->cguId, pStream->server.serverId, ZL303XX_FALSE, &config1Hz);    /* Rev Path */
               T_I("rev path zl303xx_AprConfigServer1Hz returned %x",status);

           }
       }
   }

 /*
  *  Code added stops here
  *
  ***********************/

   
   /* set 1 HZ alignment */
   ZL_3034X_CHECK(zl303xx_AprSetDevice1HzEnabled(apr_clock.zl303xx_Params, config_data.enable1Hz));
   T_I("1Hz alignment %s", config_data.enable1Hz ? "enable" : "disable");

   if (status == ZL303XX_OK)
   {
      T_D("APR STREAM created. handle=%u", pStream->server.serverId);
   }

   if ((status == ZL303XX_OK) &&
           (status = zl303xx_AprSetActiveRef (pStream->cguId, pStream->server.serverId)) != ZL303XX_OK)
   {
       T_E("zl303xx_AprSetActiveRef(%p, %d) failed with status = %u",
           pStream->cguId, pStream->server.serverId, status);
   }

   if ((status = zl303xx_AprSetHoldover(apr_clock.zl303xx_Params, config_data.bHoldover)) != ZL303XX_OK) {
       T_D("Error during Zarlink APR force holdover set");
   }

   return status;
}

/* apr_clock_create */
/**
   An example of how to start an APR clock/device. An APR clock represents a
   local DCO. Each clock runs as its own OS task.

*******************************************************************************/
static zlStatusE apr_clock_create(apr_clock_createS *pClock)
{
   zlStatusE status = ZL303XX_OK;

   if ((status = zl303xx_AprAddDeviceStructInit(pClock->cguId,
                                      &pClock->device)) != ZL303XX_OK)
   {
       T_E("zl303xx_AprAddDeviceStructInit() failed with status = %u", status);
   }
   /* Overwrite defaults with values set through example globals. */
   if (status == ZL303XX_OK)
   {
       pClock->device.devMode = ZL303XX_PACKET_MODE;
       pClock->device.enterPhaseLockStatusThreshold = ZL303XX_ENTER_PHASE_LOCK_STATUS_THRESHOLD;
       pClock->device.exitPhaseLockStatusThreshold = ZL303XX_EXIT_PHASE_LOCK_STATUS_THRESHOLD;
       pClock->device.timeNo1HzAdjust2ExitPhaseLockStatus = ZL303XX_NO_1HZ_ADJUST_EXIT_PHASE_LOCK;
       pClock->device.PFConfig.adjustFreqMinPhase = config_data.adj_freq_min_phase; /* tbd */
       pClock->device.clkStabDelayLimit = 0; /* tbd */
       T_I("adjustFreqMinPhase = %u", config_data.adj_freq_min_phase);
   
       /* disable the centralized Tod manager */
       pClock->device.defaultCGU[ZL303XX_ADCGU_SET_TIME] = ZL303XX_FALSE; 
       pClock->device.defaultCGU[ZL303XX_ADCGU_STEP_TIME] = ZL303XX_FALSE;
             
       pClock->device.cguNotify = apr_cgu_notify;  /* Hook the default notify handlers */
       pClock->device.elecNotify = exampleAprElecNotify;
       pClock->device.serverNotify = apr_server_notify;
       pClock->device.oneHzNotify = exampleAprOneHzNotify;
   
       /* Bind in APR hardware driver handlers. */
   
   
       /* Function bindings required for 1Hz recovery. */
       pClock->device.setTime = apr_set_time_tsu;
       pClock->device.stepTime = apr_step_time_tsu;
       pClock->device.adjustFreq = apr_adj_freq_tsu;
       
       if (config_data.mode == 1 && config_data.enable1Hz) {
           /*******
            code added starts here (suggestion from Microsemi (Marcel)
            *******/
            
           pClock->device.PFConfig.phaseSlopeLimit = 25;
           pClock->device.PFConfig.fastLockEnabled = ZL303XX_TRUE;
           pClock->device.PFConfig.fastLockFreqChange = 4000;
           pClock->device.PFConfig.lockedFreqLimit = 10;
           pClock->device.PFConfig.notLockedFreqLimit = 4000;
           pClock->device.PFConfig.lockInThreshold = 1500;
           pClock->device.PFConfig.lockInCount = 3;
           pClock->device.PFConfig.lockOutThreshold = 2500;
           pClock->device.enterPhaseLockStatusThreshold = 1000;
           pClock->device.exitPhaseLockStatusThreshold = 2000;
           pClock->device.timeNo1HzAdjust2ExitPhaseLockStatus = 1800;
           T_I("set up extra parameters for mode 1 phase");
            
            
           /*******
            code added stops here
            *******/
       }       
   }

   
   
   /* Register a hardware device with APR. */
   T_D("calling AprAddDevice, cguid %p, device %p", pClock->cguId, &pClock->device);
   if ((status == ZL303XX_OK) &&
           (status = zl303xx_AprAddDevice(pClock->cguId,
                                    &pClock->device)) != ZL303XX_OK)
   {
      T_E("zl303xx_AprAddDevice() failed with status = %u", status);
   }
   T_D("called AprAddDevice");

   
   if (status == ZL303XX_OK)
   {
       T_D("APR Device added. cguId=%p", pClock->cguId);
   }
   return status;
}

static zlStatusE user_delay_func(Uint32T requiredDelayMs, Uint64S startOfRun, Uint64S endofRun)
{
    zlStatusE status = ZL303XX_OK;

    if (OS_TASK_DELAY(requiredDelayMs) != OS_OK)               /* New delay ms to compensate */
        status = ZL303XX_ERROR;
    return status;
}

/* apr_env_init */
/**
   Initializes the APR environment and the global variables used in vitesse
   PTP code.

*******************************************************************************/
static zlStatusE apr_env_init(void)
{
   zlStatusE status = ZL303XX_OK;

   static zl303xx_AprInitS aprInit;

   if ((status == ZL303XX_OK) &&
       ((status = zl303xx_AprInitStructInit(&aprInit)) != ZL303XX_OK))
   {
       T_E("zl303xx_AprInitStructInit() failed with status = %u", status );
   }

   T_D("zl303xx_AprInitStructInit() succeded with status = %u", status );
   if (status == ZL303XX_OK)
   {
      /* Change any APR init defaults here: */
      /*
       * ZL303XX_APR_MAX_NUM_DEVICES = 1                         - Maximum number of clock generation device in APR;
       * ZL303XX_APR_MAX_NUM_MASTERS = 8                         - Maximum number of packet/hybrid server clock for each clock device;
       * aprInit.logLevel = 0                                  - APR log level */

       aprInit.PFInitParams.useHardwareTimer = ZL303XX_FALSE; /* - Use PSLFCL and Sample delay binding (see aprAddDevice) */
       aprInit.PFInitParams.userDelayFunc = (swFuncPtrUserDelay)user_delay_func; /* - Hook PSLFCL delay binding to handler */
       aprInit.AprSampleInitParams.userDelayFunc = (swFuncPtrUserDelay)user_delay_func; /* - Hook Sample delay binding to handler */
       aprInit.logLevel = 0;  /* least detailed log level, can be set from CLI */
       aprInit.PFInitParams.logLevel = 0;  /* least detailed log level, can be set from CLI */
   }

   if ((status == ZL303XX_OK) &&
       (status = zl303xx_AprInit(&aprInit)) != ZL303XX_OK)
   {
       T_E("zl303xx_AprInit() failed with status = %u", status );
   }

   return status;
}

vtss_rc zl_3034x_pdv_create(BOOL enable)
{
    zl303xx_InitDeviceS initDevice;
    zl303xx_TodMgrInitParamsS initParams;
    if (!zarlink) {
        T_W("Zarlink device NOT detected");
        return VTSS_RC_ERROR;
    }
    if (enable && first_time) {
        /* create the PDV instance */

        /* moved from start (MASTER_UP) to here */
        /* Launch APR application */
        ZL_3034X_CHECK(zl303xx_SetupLogToMsgQ(NULL));
        ZL_3034X_CHECK(apr_env_init());
        T_D("apr_env_init finished");
        ZL_3034X_CHECK(zl303xx_InitApi());
        /* Start the Time-of-Day Update Manager */
        ZL_3034X_CHECK(zl303xx_TodMgrInit(&initParams));
        
        /* Create a HW Driver */
        apr_clock.zl303xx_Params = zl_3034x_zl303xx_params;
        T_I("got HW driver params from API %p", apr_clock.zl303xx_Params);
        
        memset(&initDevice, 0, sizeof(zl303xx_InitDeviceS));
        ZL_3034X_CHECK(zl303xx_InitDeviceStructInit(apr_clock.zl303xx_Params, &initDevice, ZL303XX_MODE_REF_TOP));
        
        initDevice.tsEngInit.tsProtocol = ZL303XX_TS_FORMAT_PTP;
        /* Initialize PLL parameters. */
        initDevice.pllInit.TopClientMode = ZL303XX_TRUE;
        initDevice.tsEngInit.intervalVal = ZL303XX_9HZ_LOG2_SYS_INTERRUPT_PERIOD;
        
        ZL_3034X_CHECK(zl303xx_InitDevice(apr_clock.zl303xx_Params, &initDevice));
        
        apr_clock.cguId = apr_clock.zl303xx_Params; /* identifies HW clock */

        /* moved from start (SWITCH ADD) to here */
        
        /* Create a CGU */
        ZL_3034X_CHECK(apr_clock_create(&apr_clock));
        
        T_D("calling DisableLogToMsgQ");
        ZL_3034X_CHECK(zl303xx_DisableLogToMsgQ());     /* Default disable logging */
        T_D("called DisableLogToMsgQ");
        /* Add a Timing Server (PTP protocol stream) to the CGU */
        ZL_3034X_CHECK(apr_stream_create(&apr_clock));
            
        Uint16T server_id;
        zl303xx_AprConfigure1HzS fwd_1hz;
        zl303xx_AprConfigure1HzS rev_1hz;
            
        ZL_3034X_CHECK(zl303xx_AprGetCurrent1HzConfigData(apr_clock.zl303xx_Params, &server_id, &fwd_1hz, &rev_1hz));
        T_N("1Hz configuration, serverId %d", server_id);
        T_N("1Hz fwd configuration, dis %d, realign %d, interval %d", fwd_1hz.disabled, fwd_1hz.realignmentType, fwd_1hz.realignmentInterval);
        T_N("1Hz rev configuration, dis %d, realign %d, interval %d", rev_1hz.disabled, rev_1hz.realignmentType, rev_1hz.realignmentInterval);
        
        if (vtss_interrupt_source_hook_set(top_isr_interrupt_function, INTERRUPT_SOURCE_TOP_ISR_REF_TS_ENG, INTERRUPT_PRIORITY_HIGHEST) != VTSS_OK)
            T_D("Error during interrupt source hook");
        
        
        first_time = FALSE;
    } else {
        T_W("ZL_3034X mode Configuration has been changed, you need to save the configuration and reboot to activate the changed conf.");
    }
    return VTSS_RC_OK;
}




vtss_rc zl_3034x_pdv_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    u8 reg;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        critd_init(&zl3034x_global.datamutex, "zl_3034x_data", VTSS_MODULE_ID_ZL_3034X_PDV, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        ZL_3034X_DATA_UNLOCK();
#if defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_LUTON26)
        dev_zl_pdv.delay = 1;
#endif
#ifdef VTSS_ARCH_JAGUAR_1
        dev_zl_pdv.delay = 1;
#endif
#ifdef VTSS_SW_OPTION_CLI
        zl_3034x_pdv_cli_req_init();
#endif

        T_D("INIT_CMD_INIT ZL_3034X" );
        break;
    case INIT_CMD_START:
        T_D("INIT_CMD_START ZL_3034X");

        reg = 0;
        zl_3034x_spi_write(0x64, &reg, 1);   /* Set page register to '0' */
        zl_3034x_spi_read(0x00, &reg, 1);   /* read ID register */
        zarlink = (((reg & 0x1F) == 0x0C) || ((reg & 0x1F) == 0x0D));
        if (!zarlink) {
            T_W("Zarlink device NOT detected");
            break;
        }
        apr_clock.cguId = NULL;
        apr_clock.zl303xx_Params = NULL;
        T_D("INIT_CMD_START - ISID %u", isid);
        break;
    case INIT_CMD_CONF_DEF:
        zl3034x_conf_read(TRUE);
        zl3034x_conf_propagate();
        T_D("INIT_CMD_CONF_DEF ZL_3034X" );
        break;
    case INIT_CMD_MASTER_UP:
        if (!zarlink) {
            T_I("Zarlink device NOT detected");
            break;
        }
        /* the initialization is moved to the zl_3034x_pdv_create function, only done ig the PDV is enabled */
        /* Create a HW Driver */
        apr_clock.zl303xx_Params = zl_3034x_zl303xx_params;
        T_I("got HW driver params from API %p", apr_clock.zl303xx_Params);
        
        zl3034x_conf_read(FALSE);
        T_D("INIT_CMD_MASTER_UP ");
        break;
    case INIT_CMD_SWITCH_ADD:
        if (!zarlink)   break;
        T_D("INIT_CMD_SWITCH_ADD");
        /* the initialization is moved to the zl_3034x_pdv_create function, only done ig the PDV is enabled */
        
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("INIT_CMD_MASTER_DOWN - ISID %u", isid);
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

