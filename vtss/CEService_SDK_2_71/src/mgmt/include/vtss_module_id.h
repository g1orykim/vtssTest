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

#ifndef _VTSS_MODULE_ID_H_
#define _VTSS_MODULE_ID_H_

/* Module IDs */
/*
 * !!!!! IMPORTANT !!!!!
 * ---------------------
 * When adding new module IDs, these MUST be added at the end of the current
 * list. Also module IDs MUST NEVER be deleted from the list.
 * This is necessary to ensure that the Msg protocol can rely on consistent
 * module IDs between different SW versions.
 */
enum {
    /* Switch API */
    VTSS_MODULE_ID_API_IO = 0,    /* API I/O Layer */
    VTSS_MODULE_ID_API_CI,        /* API Chip Interface Layer */
    VTSS_MODULE_ID_API_AI,        /* API Application Interface Layer */
    VTSS_MODULE_ID_SPROUT,        /* SPROUT (3) */
    VTSS_MODULE_ID_MAIN,
    VTSS_MODULE_ID_TOPO,
    VTSS_MODULE_ID_CONF,
    VTSS_MODULE_ID_MSG,
    VTSS_MODULE_ID_PACKET,
    VTSS_MODULE_ID_TRACE,
    VTSS_MODULE_ID_IP_STACK_GLUE, /* 10 */
    VTSS_MODULE_ID_PORT,
    VTSS_MODULE_ID_MAC,
    VTSS_MODULE_ID_VLAN,
    VTSS_MODULE_ID_QOS,
    VTSS_MODULE_ID_MIRROR,
    VTSS_MODULE_ID_MISC,
    VTSS_MODULE_ID_ACL,
    VTSS_MODULE_ID_IP,
    VTSS_MODULE_ID_AGGR,
    VTSS_MODULE_ID_RSTP,          /* 20 */
    VTSS_MODULE_ID_DOT1X,
    VTSS_MODULE_ID_IGMP,
    VTSS_MODULE_ID_PVLAN,
    VTSS_MODULE_ID_SYSTEM,
    VTSS_MODULE_ID_CLI,
    VTSS_MODULE_ID_WEB,
    VTSS_MODULE_ID_PING,
    VTSS_MODULE_ID_FIRMWARE,
    VTSS_MODULE_ID_UNMGD,         /* Appl. code for unmanaged, single-threaded switch */
    VTSS_MODULE_ID_MSG_TEST,      /* 30. Used for test-purposes only. */
    VTSS_MODULE_ID_LED,           /* LED handling */
    VTSS_MODULE_ID_CRITD,         /* Critical regions with debug facilities */
    VTSS_MODULE_ID_L2PROTO,
    VTSS_MODULE_ID_LLDP,
    VTSS_MODULE_ID_LACP,
    VTSS_MODULE_ID_SNMP,
    VTSS_MODULE_ID_SYSLOG,
    VTSS_MODULE_ID_IPMC_LIB,
    VTSS_MODULE_ID_CONF_XML,
    VTSS_MODULE_ID_VTSS_LB,       /* 40 */
    VTSS_MODULE_ID_INTERRUPT,
    VTSS_MODULE_ID_SYNCE,
    VTSS_MODULE_ID_POE,
    VTSS_MODULE_ID_MODULE_CONFIG,
    VTSS_MODULE_ID_EPS,
    VTSS_MODULE_ID_MEP,
    VTSS_MODULE_ID_HTTPS,
    VTSS_MODULE_ID_AUTH,
    VTSS_MODULE_ID_SSH,
    VTSS_MODULE_ID_RADIUS,        /* 50 */
    VTSS_MODULE_ID_ACCESS_MGMT,
    VTSS_MODULE_ID_UPNP,
    VTSS_MODULE_ID_IP_DNS,
    VTSS_MODULE_ID_DHCP_HELPER,
    VTSS_MODULE_ID_DHCP_RELAY,
    VTSS_MODULE_ID_DHCP_SNOOPING,
    VTSS_MODULE_ID_NTP,
    VTSS_MODULE_ID_USERS,
    VTSS_MODULE_ID_PRIV_LVL,
    VTSS_MODULE_ID_SECURITY,      /* 60 */
    VTSS_MODULE_ID_DEBUG,
    VTSS_MODULE_ID_EVC,
    VTSS_MODULE_ID_ARP_INSPECTION,
    VTSS_MODULE_ID_IP_SOURCE_GUARD,
    VTSS_MODULE_ID_PTP,
    VTSS_MODULE_ID_PSEC,
    VTSS_MODULE_ID_PSEC_LIMIT,
    VTSS_MODULE_ID_MVR,
    VTSS_MODULE_ID_IPMC,
    VTSS_MODULE_ID_VOICE_VLAN,    /* 70 */
    VTSS_MODULE_ID_LLDPMED,
    VTSS_MODULE_ID_ERPS,
    VTSS_MODULE_ID_ETH_LINK_OAM,
    VTSS_MODULE_ID_EEE,
    VTSS_MODULE_ID_FAN,
    VTSS_MODULE_ID_TOD,
    VTSS_MODULE_ID_LED_POW_REDUC,
    VTSS_MODULE_ID_THERMAL_PROTECT,
    VTSS_MODULE_ID_VCL,
    VTSS_MODULE_ID_IGMP_HELPER,   /* 80 */
    VTSS_MODULE_ID_MPLS,
    VTSS_MODULE_ID_IGMPS,
    VTSS_MODULE_ID_SFLOW,
    VTSS_MODULE_ID_PHY_1588_SIM,
    VTSS_MODULE_ID_VLAN_TRANSLATION,
    VTSS_MODULE_ID_CE_MAX,
    VTSS_MODULE_ID_RABBIT_PHY,
    VTSS_MODULE_ID_DUALCPU,
    VTSS_MODULE_ID_XXRP,
    VTSS_MODULE_ID_IP_ROUTING,   /* 90 */
    VTSS_MODULE_ID_LOOP_PROTECT,
    VTSS_MODULE_ID_RMON,
    VTSS_MODULE_ID_TIMER,
    VTSS_MODULE_ID_MLDSNP,
    VTSS_MODULE_ID_ICLI,
    VTSS_MODULE_ID_REMOTE_TS_PHY,
    VTSS_MODULE_ID_DAYLIGHT_SAVING,
    VTSS_MODULE_ID_PHY,
    VTSS_MODULE_ID_CONSOLE,
    VTSS_MODULE_ID_GREEN_ETHERNET, /* 100 */
    VTSS_MODULE_ID_ICFG,
    VTSS_MODULE_ID_IP2,
    VTSS_MODULE_ID_IP2_CHIP,
    VTSS_MODULE_ID_DHCP_CLIENT,
    VTSS_MODULE_ID_SNTP,
    VTSS_MODULE_ID_RPC,
    VTSS_MODULE_ID_ZL_3034X_API,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    VTSS_MODULE_ID_DHCP_SERVER,
    VTSS_MODULE_ID_RFC2544          = 110, /* <-- Number from the module ID database */
    VTSS_MODULE_ID_JSON_RPC         = 111,
    VTSS_MODULE_ID_MACSEC           = 112,
    VTSS_MODULE_ID_BASICS           = 113,
    VTSS_MODULE_ID_DHCP             = 114,
    VTSS_MODULE_ID_DBGFS            = 115,
    VTSS_MODULE_ID_SNMP_DEMO        = 116,
    VTSS_MODULE_ID_PERF_MON         = 117,
    /*
     * INSERT NEW MODULE IDS HERE. AND ONLY HERE!!!
     *
     * REMEMBER to add a new entry in the module id database on our twiki
     * before adding the entry here!!!
     *
     * Assign the module ID number from the database to the enum value here
     * like shown in VTSS_MODULE_ID_DHCP_SERVER above.
     * This will allow for 'holes' in the enum ranges on different products/branches.
     *
     * REMEMBER ALSO TO ADD ENTRY IN vtss_module_names BELOW!!!
     * REMEMBER ALSO TO ADD ENTRY IN vtss_priv_lvl_groups_filter BELOW!!!
     **/

    /* Last entry, default */
    VTSS_MODULE_ID_NONE
};

typedef int vtss_module_id_t;

extern const char * const vtss_module_names[VTSS_MODULE_ID_NONE+1];
extern const int vtss_priv_lvl_groups_filter[VTSS_MODULE_ID_NONE+1];

#ifdef _VTSS_MODULE_ID_C_
/* This code is compiled in vtss_module_id.c, but placed here for convenience.
   These module name will shown as privilege group name.
   Please don't use space in module name, use under line instead of it.
   The module name can be used as a command keyword. */
const char * const vtss_module_names[VTSS_MODULE_ID_NONE+1] =
{
    [VTSS_MODULE_ID_API_IO]             = "obsolete_api_io", /* OBSOLETE */
    [VTSS_MODULE_ID_API_CI]             = "obsolete_api_ci", /* OBSOLETE */
    [VTSS_MODULE_ID_API_AI]             = "api",    /* VTSS_API */
    [VTSS_MODULE_ID_SPROUT]             = "sprout",
    [VTSS_MODULE_ID_MAIN]               = "main",
    [VTSS_MODULE_ID_TOPO]               = "Stack",
    [VTSS_MODULE_ID_CONF]               = "conf",
    [VTSS_MODULE_ID_MSG]                = "msg",
    [VTSS_MODULE_ID_PACKET]             = "packet",
    [VTSS_MODULE_ID_TRACE]              = "trace",
    [VTSS_MODULE_ID_IP_STACK_GLUE]      = "ip_stack_glue",
    [VTSS_MODULE_ID_PORT]               = "Ports",
    [VTSS_MODULE_ID_MAC]                = "MAC_Table",
    [VTSS_MODULE_ID_VLAN]               = "VLANs",
    [VTSS_MODULE_ID_QOS]                = "QoS",
    [VTSS_MODULE_ID_MIRROR]             = "Mirroring",
    [VTSS_MODULE_ID_MISC]               = "Maintenance",
    [VTSS_MODULE_ID_ACL]                = "ACL",
    [VTSS_MODULE_ID_IP]                 = "IP",
    [VTSS_MODULE_ID_AGGR]               = "Aggregation",
    [VTSS_MODULE_ID_L2PROTO]            = "l2proto",
    [VTSS_MODULE_ID_RSTP]               = "Spanning_Tree",
    [VTSS_MODULE_ID_DOT1X]              = "802.1X",
    [VTSS_MODULE_ID_IGMP]               = "IGMP",
    [VTSS_MODULE_ID_PVLAN]              = "Private_VLANs",
    [VTSS_MODULE_ID_SYSTEM]             = "System",
    [VTSS_MODULE_ID_CLI]                = "cli",
    [VTSS_MODULE_ID_WEB]                = "web",
    [VTSS_MODULE_ID_PING]               = "Diagnostics",
    [VTSS_MODULE_ID_FIRMWARE]           = "firmware",
    [VTSS_MODULE_ID_UNMGD]              = "unmgd",
    [VTSS_MODULE_ID_MSG_TEST]           = "msg_test",
    [VTSS_MODULE_ID_LED]                = "led",
    [VTSS_MODULE_ID_CRITD]              = "critd",
    [VTSS_MODULE_ID_L2PROTO]            = "l2",
    [VTSS_MODULE_ID_LLDP]               = "LLDP",
    [VTSS_MODULE_ID_LLDPMED]            = "LLDP_MED",
    [VTSS_MODULE_ID_LACP]               = "LACP",
    [VTSS_MODULE_ID_SNMP]               = "SNMP",
    [VTSS_MODULE_ID_SYSLOG]             = "System_Log",
    [VTSS_MODULE_ID_IPMC_LIB]           = "IPMC_LIB",
    [VTSS_MODULE_ID_CONF_XML]           = "conf_xml",
    [VTSS_MODULE_ID_VTSS_LB]            = "vtss_lb",
    [VTSS_MODULE_ID_INTERRUPT]          = "interrupt",
    [VTSS_MODULE_ID_SYNCE]              = "SyncE",
    [VTSS_MODULE_ID_POE]                = "POE",
    [VTSS_MODULE_ID_MODULE_CONFIG]      = "module_config",
    [VTSS_MODULE_ID_EPS]                = "EPS",
    [VTSS_MODULE_ID_MEP]                = "MEP",
    [VTSS_MODULE_ID_HTTPS]              = "HTTPS",
    [VTSS_MODULE_ID_AUTH]               = "Authentication",
    [VTSS_MODULE_ID_SSH]                = "SSH",
    [VTSS_MODULE_ID_RADIUS]             = "Radius",
    [VTSS_MODULE_ID_ACCESS_MGMT]        = "Access_Management",
    [VTSS_MODULE_ID_UPNP]               = "UPnP",
    [VTSS_MODULE_ID_IP_DNS]             = "DNS",
    [VTSS_MODULE_ID_DHCP_HELPER]        = "DHCP_Helper",
    [VTSS_MODULE_ID_DHCP_RELAY]         = "DHCP_Relay",
    [VTSS_MODULE_ID_DHCP_SNOOPING]      = "DHCP_Snooping",
    [VTSS_MODULE_ID_NTP]                = "NTP",
    [VTSS_MODULE_ID_USERS]              = "Users",
    [VTSS_MODULE_ID_PRIV_LVL]           = "Privilege_Levels",
    [VTSS_MODULE_ID_SECURITY]           = "Security",
    [VTSS_MODULE_ID_DEBUG]              = "Debug",
    [VTSS_MODULE_ID_EVC]                = "EVC",
    [VTSS_MODULE_ID_ARP_INSPECTION]     = "ARP_Inspection",
    [VTSS_MODULE_ID_IP_SOURCE_GUARD]    = "IP_Source_Guard",
    [VTSS_MODULE_ID_PTP]                = "PTP",
    [VTSS_MODULE_ID_PSEC]               = "Port_Security",
    [VTSS_MODULE_ID_PSEC_LIMIT]         = "PSec_Limit_Ctrl",
    [VTSS_MODULE_ID_MVR]                = "MVR",
    [VTSS_MODULE_ID_IPMC]               = "IPMC_Snooping",
    [VTSS_MODULE_ID_VOICE_VLAN]         = "Voice_VLAN",
    [VTSS_MODULE_ID_ERPS]               = "ERPS",
    [VTSS_MODULE_ID_ETH_LINK_OAM]       = "ETH_LINK_OAM",
    [VTSS_MODULE_ID_EEE]                = "EEE",
    [VTSS_MODULE_ID_FAN]                = "Fan_Control",
    [VTSS_MODULE_ID_TOD]                = "Time_Of_Day_Control",
    [VTSS_MODULE_ID_LED_POW_REDUC]      = "LED_Power_Reduced",
    [VTSS_MODULE_ID_THERMAL_PROTECT]    = "Thermal_Protection",
    [VTSS_MODULE_ID_VCL]                = "VCL",
    [VTSS_MODULE_ID_IGMP_HELPER]        = "IGMP_Helper",
    [VTSS_MODULE_ID_MPLS]               = "MPLS",
    [VTSS_MODULE_ID_IGMPS]              = "IGMP_Snooping",
    [VTSS_MODULE_ID_SFLOW]              = "sFlow",
    [VTSS_MODULE_ID_PHY_1588_SIM]       = "PHY_1588_Sim",
    [VTSS_MODULE_ID_VLAN_TRANSLATION]   = "VLAN_Translation",
    [VTSS_MODULE_ID_CE_MAX]             = "CE_MaX",
    [VTSS_MODULE_ID_RABBIT_PHY]         = "Rabbit_PHY",
    [VTSS_MODULE_ID_DUALCPU]            = "DualCPU",
    [VTSS_MODULE_ID_XXRP]               = "XXRP",
    [VTSS_MODULE_ID_IP_ROUTING]         = "Ip_Routing",
    [VTSS_MODULE_ID_LOOP_PROTECT]       = "Loop_Protect",
    [VTSS_MODULE_ID_RMON]               = "RMON",
    [VTSS_MODULE_ID_TIMER]              = "Timer",
    [VTSS_MODULE_ID_MLDSNP]             = "MLD_Snooping",
    [VTSS_MODULE_ID_ICLI]               = "Industrial_CLI",
    [VTSS_MODULE_ID_REMOTE_TS_PHY]      = "RemoteTs_PHY",
    [VTSS_MODULE_ID_DAYLIGHT_SAVING]    = "Daylight_Saving",
    [VTSS_MODULE_ID_PHY]                = "PHY",
    [VTSS_MODULE_ID_CONSOLE]            = "Console",
    [VTSS_MODULE_ID_GREEN_ETHERNET]     = "Green_Ethernet",
    [VTSS_MODULE_ID_ICFG]               = "Industrial_Config",
    [VTSS_MODULE_ID_IP2]                = "IP2",
    [VTSS_MODULE_ID_IP2_CHIP]           = "IP2_chip",
    [VTSS_MODULE_ID_DHCP_CLIENT]        = "Dhcp_Client",
    [VTSS_MODULE_ID_SNTP]               = "SNTP",
    [VTSS_MODULE_ID_RPC]                = "RPC",
    [VTSS_MODULE_ID_ZL_3034X_API]       = "ZL_3034X_API",
    [VTSS_MODULE_ID_ZL_3034X_PDV]       = "ZL_3034X_PDV",
    [VTSS_MODULE_ID_DHCP_SERVER]        = "DHCP_SERVER",
    [VTSS_MODULE_ID_RFC2544]            = "RFC2544",
    [VTSS_MODULE_ID_JSON_RPC]           = "JSON_RPC", 
    [VTSS_MODULE_ID_MACSEC]             = "MAC_Security",
    [VTSS_MODULE_ID_BASICS]             = "VTSS_BASICS",
    [VTSS_MODULE_ID_DHCP]               = "DHCP",
    [VTSS_MODULE_ID_DBGFS]              = "dbgfs",
    [VTSS_MODULE_ID_SNMP_DEMO]          = "SNMP_Demo",
    [VTSS_MODULE_ID_PERF_MON]           = "Performance_Monitor",

    /* Add new module name above it. And please don't use space
       in module name, use underscore instead. */
    [VTSS_MODULE_ID_NONE]               = "none"
};

/* In most cases, a privilege level group consists of a single module
   (e.g. LACP, RSTP or QoS), but a few of them contains more than one.
   For example, the "security" privilege group consists of authentication,
   system access management, port security, TTPS, SSH, ARP inspection and
   IP source guard modules.
   The privilege level groups shares the same array of "vtss_module_names[]".
   And use "vtss_priv_lvl_groups_filter[]" to filter the privilege level group which
   we don't need them.
   For a new module, if the module needs an independent privilege level group
   then the filter value should be equal 0. If this module is included by other
   privilege level group then the filter value should be equal 1.
   Set filter value '0' means a privilege level group mapping to a single module
   Set filter value '1' means this module will be filetered in privilege groups */
const int vtss_priv_lvl_groups_filter[VTSS_MODULE_ID_NONE+1] =
{
    [VTSS_MODULE_ID_API_IO]         = 1,
    [VTSS_MODULE_ID_API_CI]         = 1,
    [VTSS_MODULE_ID_API_AI]         = 1,
    [VTSS_MODULE_ID_SPROUT]         = 1,
    [VTSS_MODULE_ID_MAIN]           = 1,
#if VTSS_SWITCH_STACKABLE
    [VTSS_MODULE_ID_TOPO]           = 0,
#else
    [VTSS_MODULE_ID_TOPO]           = 1,
#endif
    [VTSS_MODULE_ID_CONF]           = 1,
    [VTSS_MODULE_ID_MSG]            = 1,
    [VTSS_MODULE_ID_PACKET]         = 1,
    [VTSS_MODULE_ID_TRACE]          = 1,
    [VTSS_MODULE_ID_IP_STACK_GLUE]  = 1,
#ifdef VTSS_SW_OPTION_PORT
    [VTSS_MODULE_ID_PORT]           = 0,
#else
    [VTSS_MODULE_ID_PORT]           = 1,
#endif
#ifdef VTSS_SW_OPTION_MAC
    [VTSS_MODULE_ID_MAC]            = 0,
#else
    [VTSS_MODULE_ID_MAC]            = 1,
#endif
#ifdef VTSS_SW_OPTION_VLAN
    [VTSS_MODULE_ID_VLAN]           = 0,
#else
    [VTSS_MODULE_ID_VLAN]           = 1,
#endif
#ifdef VTSS_SW_OPTION_QOS
    [VTSS_MODULE_ID_QOS]            = 0,
#else
    [VTSS_MODULE_ID_QOS]            = 1,
#endif
#ifdef VTSS_SW_OPTION_MIRROR
    [VTSS_MODULE_ID_MIRROR]         = 0,
#else
    [VTSS_MODULE_ID_MIRROR]         = 1,
#endif
    [VTSS_MODULE_ID_ACL]            = 1,
#if defined(VTSS_SW_OPTION_IP)
    [VTSS_MODULE_ID_IP]             = 0,
#else
    [VTSS_MODULE_ID_IP]             = 1,
#endif
#ifdef VTSS_SW_OPTION_AGGR
    [VTSS_MODULE_ID_AGGR]           = 0,
#else
    [VTSS_MODULE_ID_AGGR]           = 1,
#endif
    [VTSS_MODULE_ID_L2PROTO]        = 1,
#ifdef VTSS_SW_OPTION_MSTP
    [VTSS_MODULE_ID_RSTP]           = 0,
#else
    [VTSS_MODULE_ID_RSTP]           = 1,
#endif
    [VTSS_MODULE_ID_DOT1X]          = 1,
    [VTSS_MODULE_ID_IGMP]           = 1,
#ifdef VTSS_SW_OPTION_PVLAN
    [VTSS_MODULE_ID_PVLAN]          = 0,
#else
    [VTSS_MODULE_ID_PVLAN]          = 1,
#endif
    [VTSS_MODULE_ID_CLI]            = 1,
    [VTSS_MODULE_ID_WEB]            = 1,
    [VTSS_MODULE_ID_FIRMWARE]       = 1,
    [VTSS_MODULE_ID_UNMGD]          = 1,
    [VTSS_MODULE_ID_MSG_TEST]       = 1,
    [VTSS_MODULE_ID_LED]            = 1,
    [VTSS_MODULE_ID_CRITD]          = 1,
    [VTSS_MODULE_ID_L2PROTO]        = 1,
#ifdef VTSS_SW_OPTION_LLDP
    [VTSS_MODULE_ID_LLDP]           = 0,
#else
    [VTSS_MODULE_ID_LLDP]           = 1,
#endif
    [VTSS_MODULE_ID_LLDPMED]        = 1,
#ifdef VTSS_SW_OPTION_LACP
    [VTSS_MODULE_ID_LACP]           = 0,
#else
    [VTSS_MODULE_ID_LACP]           = 1,
#endif
    [VTSS_MODULE_ID_SNMP]           = 1,
    [VTSS_MODULE_ID_SYSLOG]         = 1,
    [VTSS_MODULE_ID_IPMC_LIB]       = 1,
    [VTSS_MODULE_ID_CONF_XML]       = 1,
    [VTSS_MODULE_ID_VTSS_LB]        = 1,
    [VTSS_MODULE_ID_INTERRUPT]      = 1,
#ifdef VTSS_MODULE_ID_SYNCE
    [VTSS_MODULE_ID_SYNCE]          = 0,
#else
    [VTSS_MODULE_ID_SYNCE]          = 1,
#endif
#ifdef VTSS_SW_OPTION_POE
    [VTSS_MODULE_ID_POE]            = 0,
#else
    [VTSS_MODULE_ID_POE]            = 1,
#endif
    [VTSS_MODULE_ID_MODULE_CONFIG]  = 1,
#ifdef VTSS_SW_OPTION_EPS
    [VTSS_MODULE_ID_EPS]            = 0,
#else
    [VTSS_MODULE_ID_EPS]            = 1,
#endif
#ifdef VTSS_SW_OPTION_EPS
    [VTSS_MODULE_ID_MEP]            = 0,
#else
    [VTSS_MODULE_ID_MEP]            = 1,
#endif
    [VTSS_MODULE_ID_HTTPS]          = 1,
    [VTSS_MODULE_ID_AUTH]           = 1,
    [VTSS_MODULE_ID_SSH]            = 1,
    [VTSS_MODULE_ID_RADIUS]         = 1,
    [VTSS_MODULE_ID_ACCESS_MGMT]    = 1,
#ifdef VTSS_SW_OPTION_UPNP
    [VTSS_MODULE_ID_UPNP]           = 0,
#else
    [VTSS_MODULE_ID_UPNP]           = 1,
#endif
    [VTSS_MODULE_ID_IP_DNS]         = 1,
    [VTSS_MODULE_ID_DHCP_HELPER]    = 1,
    [VTSS_MODULE_ID_DHCP_RELAY]     = 1,
    [VTSS_MODULE_ID_DHCP_SNOOPING]  = 1,
    [VTSS_MODULE_ID_DHCP_SERVER]    = 1,
#ifdef VTSS_SW_OPTION_NTP
    [VTSS_MODULE_ID_NTP]            = 0,
#else
    [VTSS_MODULE_ID_NTP]            = 1,
#endif
    [VTSS_MODULE_ID_USERS]          = 1,
    [VTSS_MODULE_ID_PRIV_LVL]       = 1,
#ifdef VTSS_SW_OPTION_EVC
    [VTSS_MODULE_ID_EVC]            = 0,
#else
    [VTSS_MODULE_ID_EVC]            = 1,
#endif
    [VTSS_MODULE_ID_ARP_INSPECTION] = 1,
    [VTSS_MODULE_ID_IP_SOURCE_GUARD]= 1,
#ifdef VTSS_SW_OPTION_PTP
    [VTSS_MODULE_ID_PTP]            = 0,
#else
    [VTSS_MODULE_ID_PTP]            = 1,
#endif
    [VTSS_MODULE_ID_PSEC]           = 1,
    [VTSS_MODULE_ID_PSEC_LIMIT]     = 1,
#ifdef VTSS_SW_OPTION_MVR
    [VTSS_MODULE_ID_MVR]            = 0,
#else
    [VTSS_MODULE_ID_MVR]            = 1,
#endif /* VTSS_SW_OPTION_MVR */
#ifdef VTSS_SW_OPTION_IPMC
    [VTSS_MODULE_ID_IPMC]           = 0,
#else
    [VTSS_MODULE_ID_IPMC]           = 1,
#endif /* VTSS_SW_OPTION_IPMC */
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    [VTSS_MODULE_ID_VOICE_VLAN]     = 0,
#else
    [VTSS_MODULE_ID_VOICE_VLAN]     = 1,
#endif
#ifdef VTSS_SW_OPTION_ERPS
    [VTSS_MODULE_ID_ERPS]           = 0,
#else
    [VTSS_MODULE_ID_ERPS]           = 1,
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    [VTSS_MODULE_ID_ETH_LINK_OAM]   = 0,
#else
    [VTSS_MODULE_ID_ETH_LINK_OAM]   = 1,
#endif
#if defined(VTSS_SW_OPTION_EEE) || defined(VTSS_SW_OPTION_FAN) || defined(VTSS_SW_OPTION_LED_POW_REDUC)
    [VTSS_MODULE_ID_GREEN_ETHERNET] = 0,
#else
    [VTSS_MODULE_ID_GREEN_ETHERNET] = 1,
#endif
#ifdef VTSS_SW_OPTION_EEE
    [VTSS_MODULE_ID_EEE] = 0,
#else
    [VTSS_MODULE_ID_EEE] = 1,
#endif
#ifdef VTSS_SW_OPTION_FAN
    [VTSS_MODULE_ID_FAN]            = 0,
#else
    [VTSS_MODULE_ID_FAN]            = 1,
#endif
    [VTSS_MODULE_ID_TOD]            = 1,
    [VTSS_MODULE_ID_LED_POW_REDUC]  = 1,
    [VTSS_MODULE_ID_THERMAL_PROTECT]= 1,
#ifdef VTSS_SW_OPTION_VCL
    [VTSS_MODULE_ID_VCL]            = 0,
#else
    [VTSS_MODULE_ID_VCL]            = 1,
#endif
    [VTSS_MODULE_ID_IGMP_HELPER]    = 1,
#ifdef VTSS_SW_OPTION_MPLS
    [VTSS_MODULE_ID_MPLS]           = 0,
#else
    [VTSS_MODULE_ID_MPLS]           = 1,
#endif
#ifdef VTSS_SW_OPTION_IGMPS
    [VTSS_MODULE_ID_IGMPS]          = 0,
#else
    [VTSS_MODULE_ID_IGMPS]          = 1,
#endif

#ifdef VTSS_SW_OPTION_SFLOW
    [VTSS_MODULE_ID_SFLOW]           = 0,
#else
    [VTSS_MODULE_ID_SFLOW]           = 1,
#endif

#ifdef VTSS_SW_OPTION_PHY_1588_SIM
    [VTSS_MODULE_ID_PHY_1588_SIM]   = 0,
#else
    [VTSS_MODULE_ID_PHY_1588_SIM]   = 1,
#endif
#ifdef VTSS_SW_OPTION_VLAN_TRANSLATION
    [VTSS_MODULE_ID_VLAN_TRANSLATION]= 0,
#else
    [VTSS_MODULE_ID_VLAN_TRANSLATION]= 1,
#endif
#ifdef VTSS_SW_OPTION_CE_MAX
    [VTSS_MODULE_ID_CE_MAX]         = 0,
#else
    [VTSS_MODULE_ID_CE_MAX]         = 1,
#endif
#ifdef VTSS_SW_OPTION_RABBIT_PHY
    [VTSS_MODULE_ID_RABBIT_PHY]     = 0,
#else
    [VTSS_MODULE_ID_RABBIT_PHY]     = 1,
#endif
#ifdef VTSS_SW_OPTION_REMOTE_TS_PHY
    [VTSS_MODULE_ID_REMOTE_TS_PHY]  = 0,
#else
    [VTSS_MODULE_ID_REMOTE_TS_PHY]  = 1,
#endif
    [VTSS_MODULE_ID_DUALCPU]        = 1,
#if defined(VTSS_SW_OPTION_XXRP) || defined(VTSS_SW_OPTION_GVRP)
    [VTSS_MODULE_ID_XXRP]           = 0,
#else
    [VTSS_MODULE_ID_XXRP]           = 1,
#endif
#ifdef VTSS_SW_OPTION_IP_ROUTING
    [VTSS_MODULE_ID_IP_ROUTING]     = 0,
#else
    [VTSS_MODULE_ID_IP_ROUTING]     = 1,
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    [VTSS_MODULE_ID_LOOP_PROTECT]   = 0,
#else
    [VTSS_MODULE_ID_LOOP_PROTECT]   = 1,
#endif
    [VTSS_MODULE_ID_DAYLIGHT_SAVING]= 1,
    [VTSS_MODULE_ID_RMON]           = 1,
    [VTSS_MODULE_ID_TIMER]          = 0,
#ifdef VTSS_SW_OPTION_MLDSNP
    [VTSS_MODULE_ID_MLDSNP]         = 0,
#else
    [VTSS_MODULE_ID_MLDSNP]         = 1,
#endif
    [VTSS_MODULE_ID_ICLI]           = 1,
    [VTSS_MODULE_ID_PHY]            = 1,
#ifdef VTSS_SW_OPTION_CONSOLE
    [VTSS_MODULE_ID_CONSOLE]        = 0,
#else
    [VTSS_MODULE_ID_CONSOLE]        = 1,
#endif
    [VTSS_MODULE_ID_ICFG]           = 1,
#if defined(VTSS_SW_OPTION_IP2)
    [VTSS_MODULE_ID_IP2]            = 0,
#else
    [VTSS_MODULE_ID_IP2]            = 1,
#endif
    [VTSS_MODULE_ID_IP2_CHIP]       = 1,
#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    [VTSS_MODULE_ID_DHCP_CLIENT]    = 0,
#else
    [VTSS_MODULE_ID_DHCP_CLIENT]    = 1,
#endif
#if defined(VTSS_SW_OPTION_SNTP)
    [VTSS_MODULE_ID_SNTP]           = 0,
#else
    [VTSS_MODULE_ID_SNTP]           = 1,
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_API)
    [VTSS_MODULE_ID_ZL_3034X_API]   = 0,
#else
    [VTSS_MODULE_ID_ZL_3034X_API]   = 1,
#endif
#if defined(VTSS_SW_OPTION_ZL_3034X_PDV)
    [VTSS_MODULE_ID_ZL_3034X_PDV]   = 0,
#else
    [VTSS_MODULE_ID_ZL_3034X_PDV]   = 1,
#endif
#if defined(VTSS_SW_OPTION_RFC2544)
    [VTSS_MODULE_ID_RFC2544]        = 0,
#else
    [VTSS_MODULE_ID_RFC2544]        = 1,
#endif
    [VTSS_MODULE_ID_JSON_RPC]       = 1,
    [VTSS_MODULE_ID_MACSEC]         = 1,
    [VTSS_MODULE_ID_BASICS]         = 1,
#if defined(VTSS_SW_OPTION_DHCP_HELPER)
    [VTSS_MODULE_ID_DHCP]           = 0,
#else
    [VTSS_MODULE_ID_DHCP]           = 1,
#endif
    [VTSS_MODULE_ID_DBGFS]          = 1,
    [VTSS_MODULE_ID_SNMP_DEMO]      = 1,
#if defined(VTSS_MODULE_ID_PERF_MON)
    [VTSS_MODULE_ID_PERF_MON]       = 0,
#else
    [VTSS_MODULE_ID_PERF_MON]       = 1,
#endif

    [VTSS_MODULE_ID_NONE]           = 1
};
#endif /* _VTSS_MODULE_ID_C_ */


#endif /* VTSS_MODULE_ID_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
