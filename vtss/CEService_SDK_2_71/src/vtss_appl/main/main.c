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

 $Id$
 $Revision$

*/

#include "main.h"
#include "control_api.h"
#include "vtss_ecos_mutex_api.h" /* For recursive mutex */
#include "conf_api.h"
#include "os_file_api.h"
#include "vtss_api_if_api.h"
#include "port_api.h"
#include "critd_api.h"
#include "interrupt_api.h"
#include "led_api.h"
#include "misc_api.h"
#include "msg_api.h"
#include "msg_test_api.h"
#include "topo_api.h"
#include "dumpbuffer_api.h"

#ifdef VTSS_SW_OPTION_CLI
#include "cli_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICLI
#include "icli_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif





#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

#ifdef VTSS_SW_OPTION_FIRMWARE
#include "firmware_api.h"
#endif

#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"
#endif

#ifdef VTSS_SW_OPTION_QOS
#include "qos_api.h"
#endif

#ifdef VTSS_SW_OPTION_VLAN
#include "vlan_api.h"
#endif

#ifdef VTSS_SW_OPTION_WEB
#include "web_api.h"
#endif

#ifdef VTSS_SW_OPTION_MAC
#include "mac_api.h"
#endif

#ifdef VTSS_SW_OPTION_LOOP_DETECT
#include "vtss_lb_api.h"
#endif

#ifdef VTSS_SW_OPTION_MIRROR
#include "mirror_api.h"
#endif

#ifdef VTSS_SW_OPTION_ACL
#include "acl_api.h"
#endif

#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif

#if defined(VTSS_SW_OPTION_IP2)
#include "ping_api.h"
#include "ip2_api.h"
#ifdef VTSS_SW_OPTION_SNTP
#include "vtss_sntp_api.h"
#endif
#endif

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
#include "dhcp_client_api.h"
#endif

#ifdef VTSS_SW_OPTION_SYNCE
#include "synce.h"
#endif

#ifdef VTSS_SW_OPTION_EVC
#include "evc_api.h"
#endif

#ifdef VTSS_SW_OPTION_EPS
#include "eps_api.h"
#endif

#ifdef VTSS_SW_OPTION_MEP
#include "mep_api.h"
#endif

#ifdef VTSS_SW_OPTION_PVLAN
#include "pvlan_api.h"
#endif

#if defined(VTSS_SW_OPTION_DOT1X)
#include "dot1x_api.h"
#endif

#if VTSS_SWITCH_UNMANAGED
#include "unmgd_api.h"
#endif

#ifdef VTSS_SW_OPTION_L2PROTO
#include "l2proto_api.h"
#endif

#ifdef VTSS_SW_OPTION_LACP
#include "lacp_api.h"
#endif

#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif

#ifdef VTSS_SW_OPTION_SNMP
#include "vtss_snmp_api.h"
#endif

#ifdef VTSS_SW_OPTION_RMON
#include "rmon_api.h"
#endif

#ifdef VTSS_SW_OPTION_IGMPS
#include "igmps_api.h"
#endif





#ifdef VTSS_SW_OPTION_DNS
#include "ip_dns_api.h"
#endif

#ifdef VTSS_SW_OPTION_POE
#include "poe_api.h"
#endif

#ifdef VTSS_SW_OPTION_HTTPS
#include "vtss_https_api.h"
#endif

#ifdef VTSS_SW_OPTION_SSH
#include "vtss_ssh_api.h"
#endif

#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif

#ifdef VTSS_SW_OPTION_UPNP
#include "vtss_upnp_api.h"
#endif

#ifdef VTSS_SW_OPTION_RADIUS
#include "vtss_radius_api.h"
#endif

#ifdef VTSS_SW_OPTION_ACCESS_MGMT
#include "access_mgmt_api.h"
#endif

#if defined(VTSS_SW_OPTION_MSTP)
#include "mstp_api.h"
#endif

#if defined(VTSS_SW_OPTION_PTP)
#include "ptp_api.h"
#endif

#ifdef VTSS_SW_OPTION_DHCP_HELPER
#include "dhcp_helper_api.h"
#endif

#ifdef VTSS_SW_OPTION_DHCP_RELAY
#include "dhcp_relay_api.h"
#endif

#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
#include "dhcp_snooping_api.h"
#endif

#if defined(VTSS_SW_OPTION_NTP)
#include "vtss_ntp_api.h"
#endif

#if defined(VTSS_SW_OPTION_USERS)
#include "vtss_users_api.h"
#endif

#if defined(VTSS_SW_OPTION_PRIV_LVL)
#include "vtss_privilege_api.h"
#endif

#ifdef VTSS_SW_OPTION_ARP_INSPECTION
#include "arp_inspection_api.h"
#endif

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
#include "ip_source_guard_api.h"
#endif

#ifdef VTSS_SW_OPTION_PSEC
#include "psec_api.h"
#endif

#ifdef VTSS_SW_OPTION_PSEC_LIMIT
#include "psec_limit_api.h"
#endif

#ifdef VTSS_SW_OPTION_IGMP_HELPER
#include "igmp_rx_helper_api.h"
#endif

#ifdef VTSS_SW_OPTION_IPMC_LIB
#include "ipmc_lib_api.h"
#endif

#ifdef VTSS_SW_OPTION_MVR
#include "mvr_api.h"
#endif

#ifdef VTSS_SW_OPTION_VOICE_VLAN
#include "voice_vlan_api.h"
#endif

#ifdef VTSS_SW_OPTION_ERPS
#include "erps_api.h"
#endif

#ifdef VTSS_SW_OPTION_EEE
#include "eee_api.h"
#endif

#ifdef VTSS_SW_OPTION_FAN
#include "fan_api.h"
#endif

#ifdef VTSS_SW_OPTION_THERMAL_PROTECT
#include "thermal_protect_api.h"
#endif

#ifdef VTSS_SW_OPTION_LED_POW_REDUC
#include "led_pow_reduc_api.h"
#endif

#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#include "eth_link_oam_api.h"
#endif

#if defined(VTSS_SW_OPTION_TOD)
#include "tod_api.h"
#endif

#ifdef VTSS_SW_OPTION_VCL
#include "vcl_api.h"
#endif

#ifdef VTSS_SW_OPTION_MLDSNP
#include "mldsnp_api.h"
#endif

#ifdef VTSS_SW_OPTION_MPLS
#include "mpls_api.h"
#endif

#ifdef VTSS_SW_OPTION_IPMC
#include "ipmc_api.h"
#endif

#ifdef VTSS_SW_OPTION_SFLOW
#include "sflow_api.h"
#endif

#ifdef VTSS_SW_OPTION_SYMREG
#include "symreg_api.h"
#endif





#ifdef VTSS_SW_OPTION_VLAN_TRANSLATION
#include "vlan_translation_api.h"
#endif





#ifdef VTSS_SW_OPTION_DUALCPU
#include "dualcpu_api.h"
#endif









#ifdef VTSS_SW_OPTION_XXRP
#include "xxrp_api.h"
#endif

#ifdef VTSS_SW_OPTION_LOOP_PROTECT
#include "loop_protect_api.h"
#endif

#ifdef VTSS_SW_OPTION_RPCSERVER
#include "rpc_server_api.h"
#endif

#ifdef VTSS_SW_OPTION_TIMER
#include "vtss_timer_api.h"
#endif

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif

#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif

#ifdef VTSS_SW_OPTION_ZL_3034X_API
#include "zl_3034x_api_api.h"
#endif

#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
#include "zl_3034x_api_pdv_api.h"
#endif

#ifdef VTSS_SW_OPTION_DHCP_SERVER
#include "dhcp_server_api.h"
#endif

#ifdef VTSS_SW_OPTION_RFC2544
#include "rfc2544_api.h"
#endif









#ifdef VTSS_SW_OPTION_PERF_MON
#include "perf_mon_api.h"
#endif

#if defined(CYGPKG_IO_FILEIO) && defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
#include <unistd.h>
#include <cyg/fileio/fileio.h>
#endif

#include <cyg/io/config_keys.h>
#include <cyg/io/flash.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_io.h>   /* For HAL defines    */
#include <cyg/hal/hal_arch.h> /* HAL_SavedRegisters */

#if defined(VTSS_ARCH_LUTON28)
#include <cyg/io/i2c_vcoreii.h>
#else
#include <cyg/io/i2c_vcoreiii.h>
#endif

// ===========================================================================
//  Trace definitions
// ---------------------------------------------------------------------------
#include <vtss_module_id.h>

#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MAIN

#define VTSS_TRACE_GRP_DEFAULT      0
#define VTSS_TRACE_GRP_INIT_MODULES 1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>
#include <vtss_trace_vtss_switch_api.h>

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "main",
    .descr     = "Main module"
};

#ifndef MAIN_DEFAULT_TRACE_LVL
#define MAIN_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = MAIN_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_INIT_MODULES] = {
        .name      = "initmods",
        .descr     = "init_modules() calls",
        .lvl       = MAIN_DEFAULT_TRACE_LVL,
        .timestamp = 1
    },
};
#endif /* VTSS_TRACE_ENABLED */
// ===========================================================================

// Thread variables
static cyg_handle_t      main_thread_handle;
static cyg_thread        main_thread_block;
static char              main_thread_stack[2 * (THREAD_DEFAULT_STACK_SIZE)];
static cyg_flag_t        control_flags;
static vtss_ecos_mutex_t reset_mutex; /* The vtss_ecos_mutex_t is an eCos mutex wrapper, that allows for recursive calls to the lock function */
static vtss_ecos_mutex_t init_modules_mutex;
static vtss_os_sem_t     reset_sem;   /* Reset lock semaphore */

// Reset (shutdown) callbacks
static int reset_callbacks;
static control_system_reset_callback_t callback_list[10];

// Debugging stuff
static vtss_init_data_t dbg_latest_init_modules_data;
static ulong            dbg_latest_init_modules_init_fun_idx;

#define CTLFLAG_SYSTEM_RESET    (1 << 1)
#define CTLFLAG_CONFIG_RESTORE  (1 << 2)

#define INITFUN(x)   {x, #x},

static struct {
    vtss_rc (*func)(vtss_init_data_t *data);
    char *name;
    cyg_tick_count_t max_callback_ticks;
    init_cmd_t       max_callback_cmd;
} initfun[] = {
    INITFUN(vtss_trace_init)
    INITFUN(critd_module_init)
    INITFUN(conf_init)
    INITFUN(vtss_api_if_init)
#ifdef VTSS_SW_OPTION_ICFG
    INITFUN(icfg_early_init)
#endif
    INITFUN(port_init)
    INITFUN(interrupt_init)




#if defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
    INITFUN(os_file_init)
#endif
#ifdef VTSS_SW_OPTION_ICLI
    INITFUN(icli_init)
#endif
#ifdef VTSS_SW_OPTION_CLI
    INITFUN(cli_init)
#endif
#ifdef VTSS_SW_OPTION_PHY
    INITFUN(phy_init)
#endif
#ifdef VTSS_SW_OPTION_SYSLOG
    INITFUN(syslog_init)
#endif
    INITFUN(dumpbuffer_init)
#ifdef VTSS_SW_OPTION_PSEC
    INITFUN(psec_init)
#endif
#ifdef VTSS_SW_OPTION_VLAN
    INITFUN(vlan_init)
#endif
    INITFUN(led_init)
#ifdef VTSS_SW_OPTION_SYNCE
    INITFUN(synce_init)
#endif
#ifdef VTSS_SW_OPTION_EVC
    INITFUN(evc_init)
#endif
#ifdef VTSS_SW_OPTION_EPS
    INITFUN(eps_init)
#endif
#if VTSS_SWITCH_STACKABLE
    INITFUN(topo_init)
#else
    INITFUN(standalone_init)
#endif
#ifdef VTSS_SW_OPTION_ACCESS_MGMT
    INITFUN(access_mgmt_init)
#endif
#ifdef VTSS_SW_OPTION_WEB
    INITFUN(web_init)
#endif
#ifdef VTSS_SW_OPTION_PACKET
    INITFUN(packet_init)
#endif
#ifdef VTSS_SW_OPTION_SYSUTIL
    INITFUN(system_init)
#endif
#ifdef VTSS_SW_OPTION_DHCP_CLIENT
    INITFUN(vtss_dhcp_client_init)
#endif
#ifdef VTSS_SW_OPTION_IP2
    INITFUN(ping_init)
    INITFUN(vtss_ip2_init)
#ifndef VTSS_SW_OPTION_NTP
#ifdef VTSS_SW_OPTION_SNTP
    INITFUN(vtss_sntp_init)
#endif
#endif
#endif
#ifdef VTSS_SW_OPTION_ACL
    INITFUN(acl_init)
#endif
#ifdef VTSS_SW_OPTION_MEP
    INITFUN(mep_init)
#endif
#ifdef VTSS_SW_OPTION_MIRROR
    INITFUN(mirror_init)
#endif
#ifdef VTSS_SW_OPTION_LOOP_DETECT
    INITFUN(vtss_lb_init)
#endif
#ifdef VTSS_SW_OPTION_MAC
    INITFUN(mac_init)
#endif
#ifdef VTSS_SW_OPTION_QOS
    INITFUN(qos_init)
#endif
#ifdef VTSS_SW_OPTION_AGGR
    INITFUN(aggr_init)
#endif
#ifdef VTSS_SW_OPTION_PVLAN
    INITFUN(pvlan_init)
#endif
#ifdef VTSS_SW_OPTION_FIRMWARE
    INITFUN(firmware_init)
#endif
    INITFUN(msg_init)
    INITFUN(msg_test_init)
    INITFUN(misc_init)
#if defined(VTSS_SW_OPTION_MSTP) || defined(VTSS_SW_OPTION_DOT1X) || defined(VTSS_SW_OPTION_LACP) || defined(VTSS_SW_OPTION_SNMP)
    INITFUN(l2_init)
#endif
#ifdef VTSS_SW_OPTION_MSTP
    INITFUN(mstp_init)
#endif
#ifdef VTSS_SW_OPTION_PTP
    INITFUN(ptp_init)
#endif
#ifdef VTSS_SW_OPTION_LACP
    INITFUN(lacp_init)
#endif
#ifdef VTSS_SW_OPTION_DOT1X
    INITFUN(dot1x_init)
#endif
#ifdef VTSS_SW_OPTION_LLDP
    INITFUN(lldp_init)
#endif
#ifdef VTSS_SW_OPTION_EEE
    INITFUN(eee_init)
#endif
#ifdef VTSS_SW_OPTION_FAN
    INITFUN(fan_init)
#endif
#ifdef VTSS_SW_OPTION_THERMAL_PROTECT
    INITFUN(thermal_protect_init)
#endif
#ifdef VTSS_SW_OPTION_LED_POW_REDUC
    INITFUN(led_pow_reduc_init)
#endif
#ifdef VTSS_SW_OPTION_SNMP
    INITFUN(snmp_init)
#endif
#ifdef VTSS_SW_OPTION_RMON
    INITFUN(rmon_init)
#endif
#ifdef VTSS_SW_OPTION_DNS
    INITFUN(ip_dns_init)
#endif



#ifdef VTSS_SW_OPTION_POE
    INITFUN(poe_init)
#endif
#ifdef VTSS_SW_OPTION_HTTPS
    INITFUN(https_init)
#endif
#ifdef VTSS_SW_OPTION_SSH
    INITFUN(ssh_init)
#endif
#ifdef VTSS_SW_OPTION_AUTH
    INITFUN(vtss_auth_init)
#endif
#ifdef VTSS_SW_OPTION_UPNP
    INITFUN(upnp_init)
#endif
#ifdef VTSS_SW_OPTION_RADIUS
    INITFUN(vtss_radius_init)
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
    INITFUN(dhcp_helper_init)
#endif
#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
    INITFUN(dhcp_snooping_init)
#endif
#ifdef VTSS_SW_OPTION_DHCP_RELAY
    INITFUN(dhcp_relay_init)
#endif
#ifdef VTSS_SW_OPTION_NTP
    INITFUN(ntp_init)
#endif
#ifdef VTSS_SW_OPTION_USERS
    INITFUN(vtss_users_init)
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
    INITFUN(vtss_priv_init)
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    INITFUN(arp_inspection_init)
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    INITFUN(ip_source_guard_init)
#endif
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    INITFUN(psec_limit_init)
#endif
#ifdef VTSS_SW_OPTION_IPMC_LIB
    INITFUN(ipmc_lib_init)
#endif
#ifdef VTSS_SW_OPTION_MVR
    INITFUN(mvr_init)
#endif
#ifdef VTSS_SW_OPTION_IPMC
    INITFUN(ipmc_init)
#endif
#ifdef VTSS_SW_OPTION_IGMP_HELPER
    INITFUN(vtss_igmp_helper_init)
#endif
#ifdef VTSS_SW_OPTION_IGMPS
    INITFUN(igmps_init)
#endif
#ifdef VTSS_SW_OPTION_MLDSNP
    INITFUN(mldsnp_init)
#endif
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    INITFUN(voice_vlan_init)
#endif
#ifdef VTSS_SW_OPTION_ERPS
    INITFUN(erps_init)
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    INITFUN(eth_link_oam_init)
#endif
#ifdef VTSS_SW_OPTION_TOD
    INITFUN(tod_init)
#endif
#ifdef VTSS_SW_OPTION_MPLS
    INITFUN(mpls_init)
#endif
#ifdef VTSS_SW_OPTION_SFLOW
    INITFUN(sflow_init)
#endif
#ifdef VTSS_SW_OPTION_VCL
    INITFUN(vcl_init)
#endif
#ifdef VTSS_SW_OPTION_SYMREG
    INITFUN(symreg_init)
#endif



#ifdef VTSS_SW_OPTION_VLAN_TRANSLATION
    INITFUN(vlan_trans_init)
#endif



#ifdef VTSS_SW_OPTION_DUALCPU
    INITFUN(dualcpu_init)
#endif






#ifdef VTSS_SW_OPTION_XXRP
    INITFUN(xxrp_init)
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    INITFUN(loop_protect_init)
#endif
#ifdef VTSS_SW_OPTION_RPCSERVER
    INITFUN(rpcserver_init)
#endif
#ifdef VTSS_SW_OPTION_TIMER
    INITFUN(vtss_timer_init)
#endif
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
    INITFUN(time_dst_init)
#endif
#ifdef VTSS_SW_OPTION_ZL_3034X_API
    INITFUN(zl_3034x_api_init)
#endif
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    INITFUN(zl_3034x_pdv_init)
#endif
#ifdef VTSS_SW_OPTION_DHCP_SERVER
    INITFUN(dhcp_server_init)
#endif
#ifdef VTSS_SW_OPTION_RFC2544
    INITFUN(rfc2544_init)
#endif








#ifdef VTSS_SW_OPTION_PERF_MON
    INITFUN(perf_mon_init)
#endif

// **** NOTE: icfg_late_init must be last in this list ****
#ifdef VTSS_SW_OPTION_ICFG
    INITFUN(icfg_late_init)
#endif
};

/* Error code interpretation */
const char *error_txt(vtss_rc rc)
{
    const char  *txt;
    int         module_id;
    int         code;
    static char txt_default[100];

    module_id = VTSS_RC_GET_MODULE_ID(rc);
    code      = VTSS_RC_GET_MODULE_CODE(rc);

    /* Default text if no module-specific decoding available */
    sprintf(txt_default, "module_id=%s, code=%d", vtss_module_names[module_id], code);
    T_D("%s",txt_default);
    if (rc <= VTSS_OK) {
        switch (module_id) {
        case VTSS_MODULE_ID_API_IO:
        case VTSS_MODULE_ID_API_CI:
        case VTSS_MODULE_ID_API_AI:
            txt = vtss_error_txt(rc);
            break;
#ifdef VTSS_SW_OPTION_ACL
        case VTSS_MODULE_ID_ACL:
            txt = acl_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_MAC
        case VTSS_MODULE_ID_MAC:
            txt = mac_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_VLAN
        case VTSS_MODULE_ID_VLAN:
            txt = vlan_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_QOS
        case VTSS_MODULE_ID_QOS:
            txt = qos_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_IP2
        case VTSS_MODULE_ID_IP2:
            txt = ip2_error_txt(rc);
            break;
        case VTSS_MODULE_ID_IP2_CHIP:
            txt = ip2_chip_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_DHCP_CLIENT
        case VTSS_MODULE_ID_DHCP_CLIENT:
            txt = dhcp_client_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_PVLAN
        case VTSS_MODULE_ID_PVLAN:
            txt = pvlan_error_txt(rc);
            break;
#endif
#if defined(VTSS_SW_OPTION_DOT1X)
        case VTSS_MODULE_ID_DOT1X:
            txt = dot1x_error_txt(rc);
            break;
#endif
#if defined(VTSS_SW_OPTION_MIRROR)
        case VTSS_MODULE_ID_MIRROR:
            txt = mirror_error_txt(rc);
            break;
#endif
#if defined(VTSS_SW_OPTION_SYNCE)
        case VTSS_MODULE_ID_SYNCE:
            txt = synce_error_txt(rc);
            break;
#endif

#if VTSS_SWITCH_STACKABLE
        case VTSS_MODULE_ID_TOPO:
            txt = topo_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_FIRMWARE
        case VTSS_MODULE_ID_FIRMWARE:
            txt = firmware_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_AGGR
        case VTSS_MODULE_ID_AGGR:
          txt = aggr_error_txt(rc);
          break;
#endif
#ifdef VTSS_SW_OPTION_SNMP
        case VTSS_MODULE_ID_SNMP:
            txt = snmp_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_HTTPS
        case VTSS_MODULE_ID_HTTPS:
            txt = https_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_AUTH
        case VTSS_MODULE_ID_AUTH:
            txt = vtss_auth_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_ACCESS_SSH
        case VTSS_MODULE_ID_SSH:
            txt = ssh_error_txt(rc);
            break;
#endif
#if defined(VTSS_SW_OPTION_RADIUS)
        case VTSS_MODULE_ID_RADIUS:
            txt = vtss_radius_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_ACCESS_MGMT
        case VTSS_MODULE_ID_ACCESS_MGMT:
            txt = access_mgmt_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_UPNP
        case VTSS_MODULE_ID_UPNP:
            txt = upnp_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_DHCP_HELPER
        case VTSS_MODULE_ID_DHCP_HELPER:
            txt = dhcp_helper_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_DHCP_RELAY
        case VTSS_MODULE_ID_DHCP_RELAY:
            txt = dhcp_relay_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_DHCP_SNOOPING
        case VTSS_MODULE_ID_DHCP_SNOOPING:
            txt = dhcp_snooping_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_NTP
        case VTSS_MODULE_ID_NTP:
            txt = ntp_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_USERS
        case VTSS_MODULE_ID_USERS:
            txt = vtss_users_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
        case VTSS_MODULE_ID_PRIV_LVL:
            txt = vtss_privilege_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
        case VTSS_MODULE_ID_ARP_INSPECTION:
            txt = arp_inspection_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
        case VTSS_MODULE_ID_IP_SOURCE_GUARD:
            txt = ip_source_guard_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_PSEC
        case VTSS_MODULE_ID_PSEC:
            txt = psec_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
        case VTSS_MODULE_ID_PSEC_LIMIT:
            txt = psec_limit_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_VOICE_VLAN
        case VTSS_MODULE_ID_VOICE_VLAN:
            txt = voice_vlan_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_PORT
        case VTSS_MODULE_ID_PORT:
            txt = port_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_LLDP
        case VTSS_MODULE_ID_LLDP:
            txt = lldp_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_EEE
        case VTSS_MODULE_ID_EEE:
            txt = eee_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_FAN
        case VTSS_MODULE_ID_FAN:
            txt = fan_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_LED_POW_REDUC
        case VTSS_MODULE_ID_LED_POW_REDUC:
            txt = led_pow_reduc_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_VCL
        case VTSS_MODULE_ID_VCL:
            txt = vcl_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_SYSLOG
        case VTSS_MODULE_ID_SYSLOG:
            txt = syslog_error_txt(rc);
            break;
#endif





#ifdef VTSS_SW_OPTION_VLAN_TRANSLATION
        case VTSS_MODULE_ID_VLAN_TRANSLATION:
            txt = vlan_trans_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_XXRP
        case VTSS_MODULE_ID_XXRP:
            txt = xxrp_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_ICLI
        case VTSS_MODULE_ID_ICLI:
            txt = icli_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_SFLOW
        case VTSS_MODULE_ID_SFLOW:
            txt = sflow_error_txt(rc);
            break;
#endif





#ifdef VTSS_SW_OPTION_DHCP_SERVER
        case VTSS_MODULE_ID_DHCP_SERVER:
            txt = dhcp_server_error_txt(rc);
            break;
#endif
#ifdef VTSS_SW_OPTION_RFC2544
        case VTSS_MODULE_ID_RFC2544:
            txt = rfc2544_error_txt(rc);
            break;
#endif





#if defined(VTSS_SW_OPTION_SYNCE)
        case VTSS_MODULE_ID_PTP:
            txt = ptp_error_txt(rc);
            break;
#endif
#if defined(VTSS_SW_OPTION_MEP)
        case VTSS_MODULE_ID_MEP:
            txt = mep_error_txt(rc);
            break;
#endif
        default:
            txt = txt_default;
            break;
        }
    } else {
        txt = "No error";
    }
    return txt;
}

char *control_system_restart_to_str(vtss_restart_t restart)
{
    switch (restart) {
    case VTSS_RESTART_COLD:
        return "cold";

    case VTSS_RESTART_COOL:
        return "cool";

    case VTSS_RESTART_WARM:
        return "warm";

    default:
        T_E("Unknown restart type: %d", restart);
        return "unknown";
    }
}

vtss_rc init_modules(vtss_init_data_t *data)
{
    int i;
    vtss_rc rc, rc_last = VTSS_RC_OK;
    cyg_tick_count_t start, total;

    // All initfun[i].func() calls below must occur
    // before another thread can intervene with other
    // parameters.
    if(data->cmd != INIT_CMD_INIT) {
        // Scheduler started, so we can actually get the lock.
        VTSS_ECOS_MUTEX_LOCK(&init_modules_mutex);
    }

    dbg_latest_init_modules_data = *data;
    for (i = 0; i < ARRSZ(initfun); i++) {
        dbg_latest_init_modules_init_fun_idx = i;
        T_IG(VTSS_TRACE_GRP_INIT_MODULES, "%s(%s, cmd: %d, isid: %x, flags: 0x%x)", __FUNCTION__, initfun[i].name, data->cmd, data->isid, data->flags);
        start = cyg_current_time();
        if ((rc = initfun[i].func(data)) != VTSS_RC_OK) {
            rc_last = rc; /* Last error is returned */
        }
        total = cyg_current_time() - start;
        T_IG(VTSS_TRACE_GRP_INIT_MODULES, "%s: %llu ms", initfun[i].name, total * ECOS_MSECS_PER_HWTICK);
        if (total > initfun[i].max_callback_ticks) {
            initfun[i].max_callback_ticks = total;
            initfun[i].max_callback_cmd   = data->cmd;
        }
    }

    if(data->cmd != INIT_CMD_INIT) {
        // Scheduler started
        VTSS_ECOS_MUTEX_UNLOCK(&init_modules_mutex);
    }

    return rc_last;
}

static int control_printf(const char *fmt, ...)
{
    va_list args;
    char    buf[256];
    int     len;

    va_start(args, fmt);
    len = vsprintf(buf, fmt, args);
    va_end(args);
    if (len > 0 && strcmp(buf, "\n") != 0) {
        /* Avoid empty lines and trailing newlines */
        if (buf[len-1] == '\n')
            buf[len-1] = '\0';
        T_D(buf);
    }

    return len;
}

/*
 * Local restore to defaults
 */
static ulong __flags = 0;
static void control_config_reset_local(ulong flags)
{
    __flags = flags;            /* ... */
    cyg_flag_setbits(&control_flags, CTLFLAG_CONFIG_RESTORE);
}

static u32 __restart;

/*
 * Message indication function
 */
static BOOL
control_msg_rx(void *contxt,
               const void *rx_msg,
               size_t len,
               vtss_module_id_t modid,
               ulong isid)
{
    if(len == sizeof(control_msg_t)) {
        const control_msg_t *mmsg = (void*)rx_msg;
        T_D("Sid %u, rx %zd bytes, msg %d", isid, len, mmsg->msg_id);
        switch (mmsg->msg_id) {
        case CONTROL_MSG_ID_SYSTEM_REBOOT:
            (void) control_system_reset(TRUE, VTSS_USID_ALL, mmsg->flags);
            break;
        case CONTROL_MSG_ID_CONFIG_RESET:
            control_config_reset_local(mmsg->flags);
            break;
        default:
            T_W("Unhandled msg %d", mmsg->msg_id);
        }
    } else {
        T_W("Bogus msg, len %zd", len);
    }
    return TRUE;
}

/*
 * Stack Register
 */
static vtss_rc
control_stack_register(void)
{
    msg_rx_filter_t filter;
    memset(&filter, 0, sizeof(filter));
    filter.cb = control_msg_rx;
    filter.modid = VTSS_MODULE_ID_MAIN;
    return msg_rx_filter_register(&filter);
}

#ifdef CYGPKG_CPULOAD
static cyg_uint32 cpuload_calibrate;
static cyg_cpuload_t cpuload_obj;
static cyg_handle_t cpuload_handle;
#endif

cyg_bool_t
control_sys_get_cpuload(cyg_uint32 *average_point1s,
                        cyg_uint32 *average_1s,
                        cyg_uint32 *average_10s)
{
    cyg_bool_t rc = false;
#ifdef CYGPKG_CPULOAD
    if(cpuload_handle) {
        cyg_cpuload_get(cpuload_handle, average_point1s, average_1s, average_10s);
        rc = true;
    }
#endif
    return rc;
}

// Module thread
static void main_thread(cyg_addrword_t thread_data)
{
    vtss_init_data_t data;
#ifdef CYGPKG_CPULOAD
    cyg_cpuload_calibrate(&cpuload_calibrate);
    cyg_cpuload_create(&cpuload_obj, cpuload_calibrate, &cpuload_handle);
#endif

    control_stack_register();

    memset(&data, 0, sizeof(data));
    for(;;) {
        cyg_flag_value_t flags = cyg_flag_wait(&control_flags,
                                               0xFF,
                                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (flags & CTLFLAG_SYSTEM_RESET) {
            T_D("System %s reboot armed", control_system_restart_to_str(__restart));
            control_system_reset_sync(__restart);
            /* NOTREACHED */
        } else if(flags & CTLFLAG_CONFIG_RESTORE) {
            T_W("Reset local configuration");
            data.cmd = INIT_CMD_CONF_DEF;
            data.isid = VTSS_ISID_LOCAL;
            data.switch_info[VTSS_ISID_LOCAL].configurable = TRUE;
            data.flags = __flags;
            init_modules(&data);
        }
    }
}

void
dump_exception_data(int (*func_printf)(const char *fmt, ...), cyg_code_t exception_number, const HAL_SavedRegisters *regs, BOOL dump_threads)
{
#if defined(CYGHWR_HAL_MIPS_MIPS32_CORE)
    const char *exception[] = {
        "External interrupt",
        "TLB modification exception",
        "TLB miss (Load or IFetch)",
        "TLB miss (Store)",
        "Address error (Load or Ifetch)",
        "Address error (store)",
        "Bus error (Ifetch)",
        "Bus error (data load or store)",
        "System call",
        "Break point",
        "Reserved instruction",
        "Coprocessor unusable",
        "Arithmetic overflow",
        "Reserved",
        "Division-by-zero",
        "Floating point exception",
    };
#elif defined(CYGINT_HAL_ARM_ARCH_ARM9)
    const char *exception[] = {
        "Reset",
        "Reserved instruction",
        "Software interrupt",
        "Abort prefetch",
        "Abort data",
        "Reserved",
        "External interrupt",
        "FIQ",
    };
    const char *dtype[] = {
        "(none)",                             /* 0b0000 */
        "Alignment",                          /* 0b0001 */
        "(none)",                             /* 0b0010 */
        "Alignment",                          /* 0b0011 */
        "(none)",                             /* 0b0100 */
        "Translation (sect)",                 /* 0b0101 */
        "(none)",                             /* 0b0110 */
        "Translation (page)",                 /* 0b0111 */
        "External Abort",                     /* 0b1000 */
        "Domain (sect)",                      /* 0b1001 */
        "External Abort",                     /* 0b1010 */
        "Domain (page)",                      /* 0b1011 */
        "Ext abt on translation (1st)",       /* 0b1100 */
        "Permisssion (sect)",                 /* 0b1101 */
        "Ext abt on translation (2nd)",       /* 0b1110 */
        "Permisssion (page)",                 /* 0b1111 */
    };
    cyg_uint32 far, dfsr;
#endif
    func_printf("Exception %d caught at PC 0x%08x - %s\n", exception_number, regs->pc,
                exception_number < ARRSZ(exception) ? exception[exception_number] : "?");
#if defined(CYGHWR_HAL_MIPS_MIPS32_CORE)
    func_printf(".at    %08x   .v0-v1 %08x %08x\n", regs->d[1], regs->d[2], regs->d[3]);
    func_printf(".a0-a3 %08x %08x %08x %08x\n", regs->d[4], regs->d[5], regs->d[6], regs->d[7]);
    func_printf(".t0-t7 %08x %08x %08x %08x %08x %08x %08x %08x\n",
                regs->d[8], regs->d[9], regs->d[10], regs->d[11], regs->d[12], regs->d[13], regs->d[14], regs->d[15]);
    func_printf(".s0-s7 %08x %08x %08x %08x %08x %08x %08x %08x\n",
                regs->d[16], regs->d[17], regs->d[18], regs->d[19], regs->d[20], regs->d[21], regs->d[22], regs->d[23]);
    func_printf(".t8-t9 %08x %08x  .fp/.s8 %08x   .k0-k1 %08x %08x\n", regs->d[24], regs->d[25], regs->d[30], regs->d[26], regs->d[27]);
    func_printf(".gp    %08x      .sp %08x      .ra %08x\n",
                regs->d[28], regs->d[29], regs->d[31]);
    func_printf(".sr    %08x   .cache %08x   .cause %08x .badvadr %08x\n\n",
                regs->sr, regs->cache, regs->cause, regs->badvr);
#elif defined(CYGINT_HAL_ARM_ARCH_ARM9)
    int i;
    /* Pull DFSR - Data Fault Status register */
    asm volatile("mrc p15, 0, %[result], c5, c0, 0" : [result] "=r" (dfsr));
    func_printf("DFSR: %08X - %s\n", dfsr, dtype[dfsr & 0xF]);
    func_printf("Registers:");
    for (i = HAL_THREAD_CONTEXT_FIRST;  i <= HAL_THREAD_CONTEXT_LAST;  i++) {
        if ((i - HAL_THREAD_CONTEXT_FIRST) % 8 == 0) func_printf("\nR%d: ", i);
        func_printf("%08X ", regs->d[i-HAL_THREAD_CONTEXT_FIRST]);
    }
    func_printf("\n");
    /* Pull FAR - Fault Address Register */
    asm volatile("mrc p15, 0, %[result], c6, c0, 0" : [result] "=r" (far));
    func_printf("FP: %08X, IP: %08X, SP: %08X\n",
                regs->fp, regs->ip, regs->sp);
    func_printf("LR: %08X, PSR: %08X, FAR: %08X\n",
                regs->lr, regs->cpsr, far);
#else
#warning Unknown architecture!
#endif
    if (dump_threads) {
        cli_print_thread_status(func_printf, TRUE, FALSE);
    }
}

static void
generic_exception_handler(cyg_addrword_t data,
                          cyg_code_t exception_number,
                          cyg_addrword_t info)
{
    static BOOL double_fault = FALSE;

    if (!double_fault) {
        // Double fault. Prevent execution of the exception data dumper since that may have been the reason for the double fault, and if executed again could cause a recursive exception invokation.
        double_fault = TRUE;
        dump_buffer_save_exeption(exception_number, (HAL_SavedRegisters*)info);
        dump_exception_data(diag_printf, exception_number, (HAL_SavedRegisters*)info, TRUE);
    }
    control_system_assert_do_reset();
}

#if defined(CYGHWR_HAL_MIPS_MIPS32_CORE)
// This function is invoked if eCos dies on an assertion.
static void reset_hook(void)
{
    static BOOL double_fault = FALSE;

    if (!double_fault) {
        // This function may be recursively invoked, if e.g. a stack-overwrite has occurred, in which case cli_print_thread_status() may cause another eCos assertion.
        double_fault = TRUE;
        cli_print_thread_status(diag_printf, TRUE, FALSE);
    }
    hal_vcoreiii_reset();
}
#endif

void cyg_user_start(void)
{
    vtss_init_data_t data;
    cyg_code_t exception;

    memset(&data, 0, sizeof(data));

    for(exception = CYGNUM_HAL_EXCEPTION_MIN; exception <= CYGNUM_HAL_EXCEPTION_MAX; exception++) {
        cyg_exception_set_handler(exception,
                                  generic_exception_handler,
                                  0,
                                  NULL,
                                  NULL);
    }

#if defined(CYGHWR_HAL_MIPS_MIPS32_CORE)
    // Set-up a handler to all eCos related assertions.
    vtss_system_reset_hook = reset_hook;
#endif

#if defined(CYGPKG_IO_FILEIO) && defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)
    if (mount( "", "/", "ramfs" ) < 0) {
        perror("mount failed:");
    }

    if (chdir( "/" ) < 0) {
        perror("chdir(\"/\"):");
    }
#endif  /* CYGPKG_FS_RAM && VTSS_SW_OPTION_ICFG */

#if defined(VTSS_SW_OPTION_DEBUG) && defined(VTSS_SW_OPTION_VCLI)
   extern void control_access_statistics_start(void);
   control_access_statistics_start();
#endif

    cyg_flag_init(&control_flags);

    // The reset_mutex serves two purposes: It makes sure that
    // the chip cannot reset (software-wise) when taken, and
    // it makes the flash program and erase function non-
    // reentrant, since it is taken by these two functions.
    // A recursive mutex, that allows the same thread to take
    // the mutex more than once, is used, because sometimes
    // the flash program and flash erase programs must be
    // called without intervening resets. This is the case for
    // conf and firmware update. In these cases, the mutex is
    // locked before erasing and released after final programming.
    // In other cases, the two functions may be interleaved
    // by a system reset. This is the case for the syslog module,
    // which doesn't necessarily write to the flash after it has
    // erased it.
    memset(&reset_mutex, 0, sizeof(reset_mutex));
    reset_mutex.type = VTSS_ECOS_MUTEX_TYPE_RECURSIVE;
    vtss_ecos_mutex_init(&reset_mutex);

    // The init_modules_mutex is used to protect the call
    // to init_modules(), so that all modules get the same
    // event before a new event can be propagated to the
    // modules from another thread. The mutex is recursive
    // because we want to allow the restore-to-defaults
    // function to call init_modules() several times without
    // intervening init_modules() calls from other threads.
    // This is opposed to normal calls to init_modules(),
    // which are not a sequence of calls to init_modules(),
    // and therefore such callers don't need to know about
    // the mutex.
    memset(&init_modules_mutex, 0, sizeof(init_modules_mutex));
    init_modules_mutex.type = VTSS_ECOS_MUTEX_TYPE_RECURSIVE;
    vtss_ecos_mutex_init(&init_modules_mutex);

    /* Create reset semaphore */
    VTSS_OS_SEM_CREATE(&reset_sem, 1);

    // Create main control thread
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      main_thread,
                      0,
                      "Main",
                      main_thread_stack,
                      sizeof(main_thread_stack),
                      &main_thread_handle,
                      &main_thread_block);
    // Resume thread
    cyg_thread_resume(main_thread_handle);

    // Initialize and register trace ressources
    VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
    VTSS_TRACE_REGISTER(&trace_reg);

    if (flash_init(control_printf) != FLASH_ERR_OK)
        T_E("flash_init failed");

    /* Module initialization - Scheduler NOT started */
    data.cmd = INIT_CMD_INIT;
    init_modules(&data);

    /* Scheduler start */
    cyg_scheduler_start();
    /* NOTREACHED */
}

/*
 * Leaching - control API functions
 */

static void control_config_change_detect_set(BOOL enable)
{
    conf_mgmt_conf_t conf;

    if (conf_mgmt_conf_get(&conf) == VTSS_OK) {
        conf.change_detect = enable;
        (void)conf_mgmt_conf_set(&conf);
    }
}

void control_config_reset(vtss_usid_t usid, ulong flags)
{
    vtss_init_data_t data;
    vtss_isid_t      isid;

    memset(&data, 0, sizeof(data));
    data.cmd = INIT_CMD_CONF_DEF;
    data.flags = flags;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        // Pretend that all switches are configurable. Only the fact
        // that a switch is not configurable in INIT_CMD_CONF_DEF
        // events must be used for anything.
        data.switch_info[isid].configurable = TRUE;
    }

    // We want all the calls to init_modules()
    // below to be non-intervenable.
    // Notice that the semaphore is recursive, so that
    // the same thread may take it several times. This
    // is used in this case, because the init_modules()
    // function itself also takes it.
    VTSS_ECOS_MUTEX_LOCK(&init_modules_mutex);

    /* For improved performance, configuration change detection is disabled
       during default creation */
    control_config_change_detect_set(0);

    if(msg_switch_is_master()) {
        if(usid == VTSS_USID_ALL) {
            T_W("Resetting global configuration, flags 0x%x", flags);
            data.isid = VTSS_ISID_GLOBAL;
            init_modules(&data);
        }
        if(vtss_switch_stackable()) { /* Reset selected switches individually */
            static control_msg_t cfg_reset_message = { .msg_id = CONTROL_MSG_ID_CONFIG_RESET };
            vtss_usid_t i;
            cfg_reset_message.flags = flags;
            for(i = VTSS_USID_START; i < VTSS_USID_END; i++) {
                if(usid == VTSS_USID_ALL || i == usid) {
                    vtss_isid_t isid = topo_usid2isid(i);
                    T_W("Reset switch %d configuration", isid);
                    data.isid = isid;
                    init_modules(&data);
                    if(msg_switch_exists(isid)) {
                        T_W("Requesting local config reset, sid %d (%d)", i, isid);
                        msg_tx_adv(NULL, NULL, MSG_TX_OPT_DONT_FREE,
                                   VTSS_MODULE_ID_MAIN, isid,
                                   (const void *)&cfg_reset_message, sizeof(cfg_reset_message));
                    }
                }
            }
        } else {
            T_W("Reset switch %d configuration", VTSS_ISID_START);
            data.isid = VTSS_ISID_START;
            init_modules(&data);
        }
    }

    /* Local config reset */
    if(!vtss_switch_stackable() || !msg_switch_is_master())  {
        T_W("Resetting local configuration, flags 0x%x", flags);
        data.isid = VTSS_ISID_LOCAL;
        init_modules(&data);
    }

    /* Enable configuration change detection again */
    control_config_change_detect_set(1);

    VTSS_ECOS_MUTEX_UNLOCK(&init_modules_mutex);

    T_I("Reset configuration done");
}

static void control_system_reset_callback(vtss_restart_t restart)
{
    int i;
    for(i = 0; i < reset_callbacks; i++) {
        T_D("Reset callback %d enter (@ %p)", i, callback_list[i]);
        callback_list[i](restart);
        T_D("Reset callback %d return", i);
    }
}

static void cold_restart(void)
{
    HAL_PLATFORM_RESET();

    // Unreachable
    while (1) {
    }
}

static void warm_restart(void)
{
#if defined(VTSS_OPT_VCORE_II)
    // This is the most dirty hack, I've ever made (haven't I said that before?)
    // Here comes the 'design-spec' that hopefully justifies the way it's implemented.
    // One of the requirements is that we don't want to create a new RedBoot
    // for use with warm-start. So we have to live with the poor warm-start
    // capabilities that the existing RedBoots have. It's OK if we only support
    // RedBoot v. 1.06 and v. 1.07 - but at least these two versions must be
    // supported.
    // These are the problems with the existing RedBoot:
    //   1) It starts a membist of most memories, and it shouldn't in a warm-start sequence.
    //   2) It initialize the DDR SDRAM memory controller, and it shouldn't in a warm-start
    //      sequence, since it might be that some memory must survive a warm-start.
    //   3) It copies itself from 0x00000000 to 0x40000000, which in a cold start sequence
    //      corresponds to a copy from flash to DDR SDRAM. But in a warm-start sequence,
    //      0x00000000 is already mapped to DDR SDRAM, so it would correspond to a copy
    //      from RAM to RAM. Instead, it should have copied itself from
    //      0x80000000 to 0x400000000, which would always (despite the value of BOOT_ENA)
    //      would be a copy from flash to RAM.
    //   4) If the boot script contains a "diag -a" (test DDR SDRAM, perform port-loopback test,
    //      membist selected memories) or other destructive commands, the warm-start won't
    //      work.
    // And here's how to solve those issues if we decide to make a new RedBoot some day:
    //   1) Read the BOOT_ENA flag. This flag is 0 if the flash is mapped into address 0x00000000
    //      and 1 if the DDR SDRAM is mapped into address 0x00000000. Once set, BOOT_ENA can
    //      only be cleared by a CPU system reset.
    //   2) If BOOT_ENA is set, RedBoot must skip membist and DDR SDRAM controller initialization.
    //   3) The relocation code must use the addresses 0x40000000 and 0x80000000 to always refer
    //      directly to DDR SDRAM and flash.
    //   4) A function that returns the value of the BOOT_ENA flag must be implemented, and all
    //      boot script commands must read this value and determine if they really are going to
    //      perform the required operation.
    // With these changes, the application could simply disable caches, MMU, and interrupts
    // and jump to physical address 0.
    //
    // Well, well. Given the limitations of the current RedBoots, I have considered/made these
    // attempts to implement warm-start on VCore-II:
    // First attempt:
    //   Ask RedBoot for the entry-point of the currently running image, and jump
    //   to that address (note: getting the name of the currently running image is not possible in
    //   RedBoot versions prior to 1.06, which was the first one supporting dual-boot).
    //   This won't work, because the currently running image has modified the .data segment, and
    //   most of the start-up code expects .data to be initialized to the value that is stored in
    //   flash. Otherwise it will mess-up. Also, global variables that are explicitly initialized
    //   to 0 will (with the current compiler) be stored in the .data segment - not the .bss segment
    //   (the .bss segment is initialized to 0 by the image itself).
    //   The next thing is to consider reloading the .data segment from flash
    //   into RAM. This would be possible if it wasn't because of the fact that
    //   the currently running image is zipped in flash. It's impossible to find
    //   the correct position to start un-zipping to RAM (and even if possible, I don't think
    //   the application includes the ZLIB packet - only RedBoot does).
    //   Besides, if a new image has just been written into flash, doing a jump to the beginnning
    //   of the currently running image wouldn't cause the newly written image to be loaded.
    //   This limitation was otherwise OK, since this is only a demo-application.
    // Second attempt:
    //   Jump to a suitable location in RedBoot to force it to re-load the application
    //   from flash and jump to it.
    //   This won't work - again - because of the .data segment. RedBoot also expects the .data
    //   segment to have initial values like the ones stored in flash.
    //   Here, it wouldn't be possible to simply reload the .data section from flash to RAM,
    //   since the running image doesn't have a clue about the locations of this segment,
    //   and - as you know - there is no API in RedBoot that the running application can
    //   call to obtain these addresses.
    // Third attempt:
    //   Since we can obtain the size of the current RedBoot, and since we know that the current
    //   RedBoot is located in DDR SDRAM starting at offset 0, we can simply copy it from
    //   flash to DDR SDRAM. This will (a.o.) restore the whole .data segment.
    //   The next thing is to find a suitable location to jump to in RedBoot. We must
    //   avoid the membisting, DDR SDRAM controller initialization, and relocation
    //   copy code (since it's erroneous and since we've already copied it).
    //   This code is located in the _platform_setup1 macro defined in
    //     .../eCos/packages/hal/arm/arm9/vcoreii/current/include/hal_platform_setup.h
    //   RedBoot v. 1.06 is built with v. 1.9 of this file.
    //   RedBoot v. 1.07 is build with v. 1.12 of this file.
    //   The difference from 1.9 to 1.12 is mainly naming of constants and support
    //   for a 32MB DDR SDRAM.
    //   This means that the address of the first instruction after this macro is the same
    //   for both versions of RedBoot, namely 0x1D0.
    //   This is the address we jump to after having re-copied from flash to RAM.

    CYG_INTERRUPT_STATE old;

    // Gracefully stop the Packet Module (and thereby the FDMA).
    packet_uninit();

    // Before jumping, we need to shut-down a few things.
    cyg_scheduler_lock();
#if VTSS_OPT_FDMA
    vtss_fdma_uninit(0);
#endif
    HAL_DISABLE_INTERRUPTS(old);
    HAL_DCACHE_SYNC();    /* Flush D-cache to DDR SDRAM  */
    HAL_DCACHE_DISABLE(); /* Controlled disable of D-cache */
    HAL_ICACHE_DISABLE(); /* Controlled disable of I-cache */

    // Also disable the MMU. We needed to do controlled disabled of the caches, due to
    // pipeline-cleaning, writebuffer-draining, etc.
    // Now, we simply disable everything:
    asm volatile (
        "mrc  p15,0,r1,c1,c0,0;"
        "bic  r1,r1,#0x000F;"
        "bic  r1,r1,#0x1000;"
        "mcr  p15,0,r1,c1,c0,0;"
        :
        :
        : "r1" /* Clobber list */
    );

    {
        // Time to copy a fresh RedBoot from flash to RAM:
        memcpy((void *)VCOREII_SDRAM_PHYS_BASE, (void *)VCOREII_FLASH_PHYS_BASE, 128 * 1024);

        // Jump to RedBoot's warm_start (which is the position right after the last instruction in the relocation code).
        void (*jump_to_redboot_warm_start)(void) = (void (*)(void))0x1D0; // Jump to RedBoot's warm_reset.
        jump_to_redboot_warm_start();
        // Unreachable
    }
#elif defined(VTSS_OPT_VCORE_III)

    // Before reset, we need to shut-down a few things.
    cyg_scheduler_lock();
#if VTSS_OPT_FDMA
    vtss_fdma_uninit(0);
#endif

    // This is the real reset (of the CPU _only_)
    hal_vcoreiii_cpu_reset();
#endif
}

static void cool_restart(void)
{
#if defined(VTSS_OPT_VCORE_II)
    CYG_INTERRUPT_STATE old;
    int                 i;
    u32                 mbox;

    // Get the current value of the MBOX value.
    // This is where the API has persisted its
    // restart state in the previous call to
    // vtss_restart_conf_set().
    // Unfortunately, there's no other way to
    // keep this persisted, because we reset
    // the switch core in just a split second,
    // and therefore we need to remember it.
    mbox = VCOREII_SYSTEM_MBOX_VAL;

    // Stop the scheduler
    cyg_scheduler_lock();

    // Disable interrupts
    HAL_DISABLE_INTERRUPTS(old);

    // Disable both D- and I-caches.
    // We have to do that because a reset of the switchcore somehow messes up the on-chip RAMs
    // of all domains.
    // But first flush the write buffer, so that we now that the value we just read
    // from VCOREII_SYSTEM_MBOX_VAL into mbox really penetrates its way to the DDR SDRAM.
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_ICACHE_DISABLE();

    // Reset switch core, only.
    // Start by protecting the iCPU
    VCOREII_SYSTEM_GLORESET = VCOREII_F_SYSTEM_GLORESET_STROBE | VCOREII_F_SYSTEM_GLORESET_ICPU_LOCK;
    // Then reset the switch core.
    VCOREII_SYSTEM_GLORESET = VCOREII_F_SYSTEM_GLORESET_MASTER_RESET;

    // Wait for a while (specified as 125 us in the datasheet)
    for (i = 0; i < 100000; i++) {
      // Do nothing
    }

    // Hopefully we're still alive
    // Clear the memory and iCPU lock bits again
    VCOREII_SYSTEM_GLORESET = VCOREII_F_SYSTEM_GLORESET_STROBE;

    // Restore the MBOX value, which is not reset
    // during the up-coming CPU system reset.
    VCOREII_SYSTEM_MBOX_CLR = 0xFFFFFFFF;
    VCOREII_SYSTEM_MBOX_SET = mbox;

    // Reset the CPU system only.
    VTSS_CPU_SYSTEM_CTRL_RESET = VTSS_F_SOFT_RST;
#elif defined(VTSS_OPT_VCORE_III)
    // Ask the HAL layer to perform the cool restart.
    // It takes one parameter, which is the address of a register
    // in the switch core domain that must survive the reset.
#if defined(VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE)
    hal_cool_restart((cyg_uint32 *)&VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE);
#elif defined(VTSS_DEVCPU_GCB_CHIP_REGS_GPR)
    hal_cool_restart((cyg_uint32 *)&VTSS_DEVCPU_GCB_CHIP_REGS_GPR);
#else
//#error Unknown GPR, please fix
#endif
#else
    // Not supported on this system, but must be, since this is
    // the normal reboot method. Fall-back on the cold restart.
    cold_restart();
#endif

    // Unreachable
    while (1) {
    }
}

static void control_system_reset_do_sync(vtss_restart_t restart, BOOL bypass_callback)
{
    if (restart == VTSS_RESTART_WARM) {












        // The system doesn't support warm start, so we do a cool start.
        restart = VTSS_RESTART_COOL;

    }

    T_W("Performing %s reboot of system", control_system_restart_to_str(restart));

    // Because the I2C hw controller doesn't like to be reset while
    // there is a i2c access in progress, we signal to the i2c driver that we are rebooting
    cyg_scheduler_lock();
    ((cyg_vcore_i2c_extra*)hal_vcore_i2c_bus.i2c_extra)->i2c_flags |= I2C_FLAG_REBOOT;
    cyg_scheduler_unlock();

    // Let those modules that are interested in this know about the upcoming restart.
    // In some situations, this is not possible and could cause a deadlock or
    // a recursive call to this function (through VTSS_ASSERT()). In such situations,
    // we bypass both callbacks and grabbing the reset lock (since that also might
    // cause a deadlock).
    if (!bypass_callback) {
      control_system_reset_callback(restart);
       /* Boost current holder of lock - if any */
      cyg_thread_set_priority(cyg_thread_self(), THREAD_HIGHEST_PRIO);
      control_system_flash_lock();
      VTSS_OS_MSLEEP(1000);
    }

    // Tell the method to the API.
    vtss_restart_conf_set(NULL, restart);

    switch (restart) {
    case VTSS_RESTART_COLD:
        // This should only be reachable through the "debug system reboot cold" command.
        cold_restart();
        break;

    case VTSS_RESTART_WARM:
        warm_restart();
        break;

    case VTSS_RESTART_COOL: {
        cool_restart();
        break;
      }

    default:
        // If we get here, the reboot method is not supported on this platform.
        T_E("Unsupported reboot method: %s", control_system_restart_to_str(restart));
        break;
    }

    T_E("Reset(%d) trying to return", restart);
    VTSS_ASSERT(0);             /* Don't return */
}

void control_system_reset_sync(vtss_restart_t restart)
{
    control_system_reset_do_sync(restart, FALSE);
    while(1); /* Don't return */
}

void control_system_assert_do_reset(void)
{
    warm_restart();
    while(1); /* Don't return */
}


int control_system_reset(BOOL local, vtss_usid_t usid, vtss_restart_t restart)
{
    /* Force imediate configuration flush */
    conf_flush();

    if (local) {
        T_D("Resetting system");
        __restart = restart;
        cyg_flag_setbits(&control_flags, CTLFLAG_SYSTEM_RESET);
    } else {
        if (msg_switch_is_master()) {
            static control_msg_t sys_reboot_message = {.msg_id = CONTROL_MSG_ID_SYSTEM_REBOOT};
            sys_reboot_message.flags = restart;
            vtss_usid_t i;
            BOOL local_reset = FALSE;
            for(i = VTSS_USID_START; i < VTSS_USID_END; i++) {
                if(usid == VTSS_USID_ALL || i == usid) {
                    vtss_isid_t sid = topo_usid2isid(i);
                    if(msg_switch_is_local(sid)) {
                        /* Reset myself last! */
                        local_reset = TRUE;
                    } else {
                        if(msg_switch_exists(sid)) {
                            T_D("Requesting stack reset, sid %d (%d)", i, sid);
                            msg_tx_adv(NULL, NULL, MSG_TX_OPT_DONT_FREE,
                                       VTSS_MODULE_ID_MAIN, sid,
                                       (const void *)&sys_reboot_message, sizeof(sys_reboot_message));
                        }
                    }
                }
            }
            if (usid == VTSS_USID_ALL)
                VTSS_OS_MSLEEP(2000);  /* Allow to settle */
            if (local_reset) {
                T_D("Requesting local reset");
                msg_tx_adv(NULL, NULL, MSG_TX_OPT_DONT_FREE,
                           VTSS_MODULE_ID_MAIN, VTSS_ISID_LOCAL,
                           (const void *)&sys_reboot_message, sizeof(sys_reboot_message));
            }
        } else
            T_D("Non-local reset only possible on master");
    }

    return 0;
}

void control_system_flash_lock(void)
{
    T_D("Locking system reset/flash");
    VTSS_ECOS_MUTEX_LOCK(&reset_mutex);
    T_D("System reset/flash locked, level %d", reset_mutex.lock_cnt);
}

BOOL control_system_flash_trylock(void)
{
    BOOL result;
    T_D("Attempting to lock system reset/flash");
    result = VTSS_ECOS_MUTEX_TRYLOCK(&reset_mutex);
    if (result) {
      T_D("System reset/flash locked, level %d", reset_mutex.lock_cnt);
    } else {
      T_D("System reset/flash try lock failed");
    }
    return result;
}

void control_system_flash_unlock(void)
{
    VTSS_ECOS_MUTEX_UNLOCK(&reset_mutex);
    T_D("Unlocked system reset/flash, level %d", reset_mutex.lock_cnt);
}

/* Retrun TRUE: mutex locked, FALSE: mutex unlocked */
BOOL control_system_flash_islock(void)
{
    if (reset_mutex.cyg_mutex.locked) {
        return TRUE;
    }
    return FALSE;
}

void control_system_reset_register(control_system_reset_callback_t cb)
{
    VTSS_ASSERT(reset_callbacks < ARRSZ(callback_list));
    cyg_scheduler_lock();
    callback_list[reset_callbacks++] = cb;
    cyg_scheduler_unlock();
}

/****************************************************************************/
/****************************************************************************/
int control_flash_erase(cyg_flashaddr_t base, size_t len)
{
  cyg_flashaddr_t err_address;
  int err;
  control_system_flash_lock(); // Prevent system from resetting and this function from being re-entrant
  err = cyg_flash_erase(base, len, &err_address);
  control_system_flash_unlock();
  if(err != CYG_FLASH_ERR_OK) {
      T_E("An error occurred when attempting to erase %zu bytes at 0x%08x @ 0x%08x: %s", len, base, err_address, cyg_flash_errmsg(err));
  }
  return err;
}

/****************************************************************************/
/****************************************************************************/
int control_flash_program(cyg_flashaddr_t flash_base, const void *ram_base, size_t len)
{
  cyg_flashaddr_t err_address;
  int err;
  control_system_flash_lock(); // Prevent system from resetting and this function from being re-entrant
  err = cyg_flash_program(flash_base, ram_base, len, &err_address);
  control_system_flash_unlock();
  if(err != CYG_FLASH_ERR_OK) {
      T_E("An error occurred when attempting to program %zu bytes to 0x%08x @ 0x%08x: %s", len, flash_base, err_address, cyg_flash_errmsg(err));
  }
  return err;
}

/****************************************************************************/
/****************************************************************************/
int control_flash_read(cyg_flashaddr_t flash_base, void *dest, size_t len)
{
  cyg_flashaddr_t err_address;
  int err;
  control_system_flash_lock(); // Prevent system from resetting and this function from being re-entrant
  err = cyg_flash_read(flash_base, dest, len, &err_address);
  control_system_flash_unlock();
  if(err != CYG_FLASH_ERR_OK) {
      T_E("An error occurred when attempting to read %zu bytes from 0x%08x @ 0x%08x: %s", len, flash_base, err_address, cyg_flash_errmsg(err));
  }
  return err;
}

/****************************************************************************/
/****************************************************************************/
void control_flash_get_info(cyg_flashaddr_t *start, size_t *size)
{
  cyg_flash_info_t flash_info;
  if(cyg_flash_get_info(0, &flash_info) == CYG_FLASH_ERR_OK) {
    *start = flash_info.start;
    *size  = flash_info.end - flash_info.start + 1;
  } else {
    *start = 0;
    *size  = 0;
  }
}

/****************************************************************************/
/****************************************************************************/
void control_dbg_latest_init_modules_get(vtss_init_data_t *data, char **init_module_func_name)
{
    *data  = dbg_latest_init_modules_data;
    *init_module_func_name = initfun[dbg_latest_init_modules_init_fun_idx].name;
}

char *control_init_cmd2str(init_cmd_t cmd)
{
    switch (cmd) {
    case INIT_CMD_INIT:
        return "INIT";
    case INIT_CMD_START:
        return "START";
    case INIT_CMD_CONF_DEF:
        return "CONF DEFAULT";
    case INIT_CMD_MASTER_UP:
        return "MASTER UP";
    case INIT_CMD_MASTER_DOWN:
        return "MASTER DOWN";
    case INIT_CMD_SWITCH_ADD:
        return "SWITCH ADD";
    case INIT_CMD_SWITCH_DEL:
        return "SWITCH DELETE";
    case INIT_CMD_SUSPEND_RESUME:
        return "SUSPEND/RESUME";
    case INIT_CMD_WARMSTART_QUERY:
        return "WARMSTART QUERY";
    default:
        return "UNKNOWN COMMAND";
    }
}

/****************************************************************************/
/****************************************************************************/
void control_dbg_init_modules_callback_time_max_print(msg_dbg_printf_t dbg_printf, BOOL clear)
{
    int i;
    if (!clear) {
        dbg_printf("Init Modules Callback Max time [ms] Callback type\n");
        dbg_printf("--------------------- ------------- ---------------\n");
    }
    for (i = 0; i < sizeof(initfun) / sizeof(initfun[i]); i++) {
        if (clear) {
            initfun[i].max_callback_ticks = 0;
            initfun[i].max_callback_cmd   = 0;
        } else {
            dbg_printf("%21s %13llu %-15s\n", initfun[i].name, initfun[i].max_callback_ticks * ECOS_MSECS_PER_HWTICK, control_init_cmd2str(initfun[i].max_callback_cmd));
        }
    }

    if (clear) {
        dbg_printf("Init Modules callback statistics cleared\n");
    }
}

/* - Assertions ----------------------------------------------------- */

vtss_common_assert_cb_t vtss_common_assert_cb=NULL;

void vtss_common_assert_cb_set(vtss_common_assert_cb_t cb)
{
    VTSS_ASSERT(vtss_common_assert_cb == NULL);
    vtss_common_assert_cb = cb;
}

#if defined(VTSS_SW_OPTION_DEBUG)

// Layout of overlaid memory for memory check functionality:
//
// u32_user_ptr[-2] = magic #1 (3 bytes) bitwise ORed with (modid << 24)
// u32_user_ptr[-1] = User-requested size (sz)
// sz bytes of user memory
// u8_user_ptr[sz]  = magic #2 (1 byte, for simplicity, or we would have to R/W a byte at a time due to alignment problems). To test memory overwrites at the end of the allocation.

#define VTSS_MEMALLOC_MAGIC_1          0xC0FFEEU /* 3 bytes */
#define VTSS_MEMALLOC_MAGIC_2          0xBAU     /* 1 byte  */
#define VTSS_MEMALLOC_ADDITIONAL_BYTES 9         /* We need 2 * 4 + 1 bytes for this */

heap_usage_t heap_usage_cur[VTSS_MODULE_ID_NONE + 1];
u32          heap_usage_tot_max_cur, heap_usage_tot_cur;

static void vtss_memalloc_check_modid(char *caller, vtss_module_id_t *modid)
{
   if (*modid < 0 || *modid > VTSS_MODULE_ID_NONE) {
       // Allow VTSS_MODULE_ID_NONE for code that doesn't have a module ID.
       T_E("%s: Invalid module id (%d). Using VTSS_MODULE_ID_NONE", caller, *modid);
       *modid = VTSS_MODULE_ID_NONE;
   }
}

static void vtss_set_free(void *ptr, vtss_module_id_t check_against_modid, const char *const file, const int line)
{
    vtss_module_id_t modid;
    u32              *p, magics[2];
    BOOL             modid_ok, magics_ok;
    size_t           sz;

    if (!ptr) {
        return;
    }

    p         = (u32 *)ptr - 2;
    modid     = (p[0] >> 24) & 0xFF;
    sz        = p[1];
    modid_ok  = modid >= 0 && modid <= VTSS_MODULE_ID_NONE;
    magics[0] = p[0] & 0xFFFFFF;
    magics[1] = ((u8 *)ptr)[sz];
    magics_ok = magics[0] == VTSS_MEMALLOC_MAGIC_1 && magics[1] == VTSS_MEMALLOC_MAGIC_2;

    if (!magics_ok) {
        T_E("%s#%d: Memory corruption at %p. One or more invalid magics: Got <0x%x, 0x%x>, expected <0x%x, 0x%x>. Module = %d = %s", file, line, ptr, magics[0], magics[1], VTSS_MEMALLOC_MAGIC_1, VTSS_MEMALLOC_MAGIC_2, modid, modid_ok ? vtss_module_names[modid] : "");
    }

    if (!modid_ok) {
        T_E("%s#%d: Memory corruption at %p. Invalid module ID (%d)", file, line, ptr, modid);
    }

    if (check_against_modid != -1 && check_against_modid != modid) {
        T_E("%s#%d: realloc(%p) called with a module id (%d = %s) different from what alloc originally was called with (%d = %s)", file, line, ptr, check_against_modid, vtss_module_names[check_against_modid], modid, modid_ok ? vtss_module_names[modid] : "<Unknown>");
    }

    if (magics_ok && modid_ok) {
        BOOL m_sz_ok = TRUE, tot_sz_ok = TRUE;
        u32  m_mem_in_use = 0, tot_mem_in_use = 0; // Initialize to satisfy Lint
        heap_usage_t *m = &heap_usage_cur[modid];

        cyg_scheduler_lock();

        // Module memory consumption updates
        m->frees++;
        if (m->usage < sz) {
            m_mem_in_use = m->usage;
            m_sz_ok = FALSE;
        } else {
            m->usage -= sz;
        }

        // Total memory consumption updates
        if (heap_usage_tot_cur < sz) {
            tot_mem_in_use = heap_usage_tot_cur;
            tot_sz_ok = FALSE;
        } else {
            heap_usage_tot_cur -= sz;
        }
        cyg_scheduler_unlock();

        if (!m_sz_ok) {
            T_E("%s:%d: Something fishy going on (%p). Module %d = %s hasn't allocated %u bytes. Only %u bytes still unfreed", file, line, ptr, modid, vtss_module_names[modid], sz, m_mem_in_use);
        }
        if (!tot_sz_ok) {
            T_E("%s:%d: Something fishy going on (%p). Total memory currently allocated = %u bytes. Module %d = %s attempted to deallocate %u bytes", file, line, ptr, tot_mem_in_use, modid, vtss_module_names[modid], sz);
        }
    }

    p[0] &= 0xFF000000; // Clear magic only, so that we can detect double-freeing and still produce valuable module information.
}

void vtss_free(void *ptr, const char *const file, const int line)
{
    if (!ptr) {
        return;
    }
    vtss_set_free(ptr, -1, file, line);
    free((u32 *)ptr - 2);
}

static void *vtss_do_alloc(vtss_module_id_t modid, size_t sz, void *ptr, char *caller)
{
    u32 *p;
    vtss_memalloc_check_modid(caller, &modid);

    if (ptr) {
        p = realloc((u32 *)ptr - 2, sz + VTSS_MEMALLOC_ADDITIONAL_BYTES);
    } else {
        p = malloc(sz + VTSS_MEMALLOC_ADDITIONAL_BYTES);
    }

    if (p != NULL) {
        heap_usage_t *m = &heap_usage_cur[modid];

        cyg_scheduler_lock();

        // Module memory consumption updates
        m->usage += sz;
        if (m->usage > m->max) {
            m->max = m->usage;
        }
        m->allocs++;
        m->total += sz;

        // Total memory consumption updates
        heap_usage_tot_cur += sz;
        if (heap_usage_tot_cur > heap_usage_tot_max_cur) {
            heap_usage_tot_max_cur = heap_usage_tot_cur;
        }

        cyg_scheduler_unlock();

        p[0] = ((VTSS_MEMALLOC_MAGIC_1 & 0xFFFFFF) << 0) | (modid & 0xFF) << 24;
        p[1] = sz;
        ((u8 *)p)[2 * sizeof(u32) + sz] = VTSS_MEMALLOC_MAGIC_2;
        return p + 2;
    }

    return NULL;
}

void *vtss_malloc(vtss_module_id_t modid, size_t sz)
{
   return vtss_do_alloc(modid, sz, NULL, "malloc");
}

void *vtss_calloc(vtss_module_id_t modid, size_t nm, size_t sz)
{
    // We have to implement our own version of calloc() because
    // we cannot just request another X bytes.
    size_t real_size = nm * sz;
    void *ptr = vtss_do_alloc(modid, real_size, NULL, "calloc");

    if (!ptr) {
        return NULL;
    }

    memset(ptr, 0, real_size);
    return ptr;
}

void *vtss_realloc(vtss_module_id_t modid, void *ptr, size_t sz, const char *const file, const int line)
{
    vtss_set_free(ptr, modid, file, line);
    return vtss_do_alloc(modid, sz, ptr, "realloc");
}

char *vtss_strdup(vtss_module_id_t modid, const char *str)
{
    char *x = vtss_malloc(modid, strlen(str) + 1);
    if (x != NULL) {
        strcpy(x, str);
    }
    return x;
}

#endif /* VTSS_SW_OPTION_DEBUG */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
