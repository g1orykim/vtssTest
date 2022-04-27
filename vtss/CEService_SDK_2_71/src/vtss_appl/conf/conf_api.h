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

#ifndef _VTSS_CONF_API_H_
#define _VTSS_CONF_API_H_

#include "main.h"

/* ================================================================= *
 *  How to use the configuration system.
 * ================================================================= */

/* 
   The recommended approach for controlling configuration data 
   of a module is:

   1. Determine the configuration section to use. You will most likely
      use the GLOBAL section, which contains data replicated to slave
      switches in a stackable system. The LOCAL section is only used
      if you want to have data stored for the local switch only,
      avoiding any kind of stack copying.

   2. Determine which configuration blocks you need. It is recommended
      to split up your configuration data in blocks corresponding to the 
      scope of the data. For example:
      - One block for data for the whole stack, for example MAC age time.
      - One block for data for each switch port, for example learn mode.

   3. For each block, allocate ID and add block name in this file.

   4. For each block, fill out the data structures in a private header file
      for the module. It is recommended to start each structure with a 
      32-bit block version number, which can be used to handle future 
      changes of the data layout. It may also be a good idea to add some
      amount of reserved fields in the structures for minor future additions.

   5. Implement the C code accessing the data:
      - On INIT_CMD_MASTER_UP events, read the configuration data using
      conf_sec_open(). If the block does not exist or the block size does 
      not match, create a new block using conf_sec_create(). If a new block
      is created or the block version number does not match, default values
      are normally applied to the block. If configuration upgrades must be 
      supported and the block version is smaller than the current version, 
      the block data is updated to the current version. After any access,
      the block must be closed using conf_sec_close().
      - On INIT_CMD_CONF_DEF events, create defaults for the blocks using
      the conf_sec_open() and conf_sec_close() calls.
      - On configuration changes from the user interfaces, write the changed
      configuration data using the conf_sec_open() and conf_sec_close() calls.

   6. Debugging: If CLI is available, the "Debug Configuration Blocks" command
      can be used to provide information about the blocks stored in each section.

*/

/* ================================================================= *
 *  Board configuration access
 * ================================================================= */

/* Board configuration */
typedef struct {
    uchar mac_address[6];
    ulong board_id;
    int   board_type;
} conf_board_t;

/* Get board configuration */
int conf_mgmt_board_get(conf_board_t *conf);

/* Set board configuration */
int conf_mgmt_board_set(conf_board_t *conf);

/* Get MAC address (index 0 is the base MAC address). */
int conf_mgmt_mac_addr_get(uchar *mac, uint index);

/* ================================================================= *
 *  Configuration section IDs, block IDs and debug access
 * ================================================================= */

typedef enum {
    CONF_SEC_LOCAL = 0, /* Local section */
    CONF_SEC_GLOBAL,    /* GLobal section */
    CONF_SEC_CNT        /* Section count */
} conf_sec_t;

typedef enum {
    CONF_BLK_CONF_HDR = 0,
    CONF_BLK_PORT_CONF_TABLE,
    CONF_BLK_IP_CONF,
    CONF_BLK_ACL_PORT_TABLE,
    CONF_BLK_ACL_BIND_TABLE,
    CONF_BLK_ACL_POLICER_TABLE,
    CONF_BLK_ACL_ACE_TABLE,
    CONF_BLK_MIRROR_CONF_TABLE,
    CONF_BLK_MAC_AGE_CONF_TABLE,
    CONF_BLK_MAC_STATIC_CONF_TABLE,
    CONF_BLK_TOPO,
    CONF_BLK_VLAN_PORT_TABLE,
    CONF_BLK_VLAN_TABLE,
    CONF_BLK_VLAN_NAME_TABLE,
    CONF_BLK_AGGR_TABLE,
    CONF_BLK_QOS_QCL_QCE_TABLE,
    CONF_BLK_QOS_PORT_TABLE,
    CONF_BLK_QOS_TABLE,
    CONF_BLK_TRACE,
    CONF_BLK_PVLAN_TABLE,
    CONF_BLK_PVLAN_ISOLATE_TABLE,
    CONF_BLK_SYSTEM,
    CONF_BLK_MAC_LEARN_MODE_TABLE,
    CONF_BLK_DOT1X,
    CONF_BLK_RSTP_CONF,
    CONF_BLK_LACP_CONF,
    CONF_BLK_LLDP_CONF,
    CONF_BLK_SNMP_CONF,
    CONF_BLK_SNMP_RMON_STAT_TABLE,
    CONF_BLK_SNMP_RMON_HISTORY_TABLE,
    CONF_BLK_SNMP_RMON_ALARM_TABLE,
    CONF_BLK_SNMP_RMON_EVENT_TABLE,
    CONF_BLK_IPMCLIB_CONF,
    CONF_BLK_SNMP_PORT_CONF,
    CONF_BLK_SNMPV3_COMMUNITIES_CONF,
    CONF_BLK_SNMPV3_USERS_CONF,
    CONF_BLK_SNMPV3_GROUPS_CONF,
    CONF_BLK_SNMPV3_VIEWS_CONF,
    CONF_BLK_SNMPV3_ACCESSES_CONF,
    CONF_BLK_SYNCE_CONF_TABLE,
    CONF_BLK_POE_CONF,
    CONF_BLK_EPS_CONF_TABLE,
    CONF_BLK_MEP_CONF_TABLE,
    CONF_BLK_HTTPS_CONF,
    CONF_BLK_AUTH_TABLE,
    CONF_BLK_SSH_CONF,
    CONF_BLK_ACCESS_MGMT_CONF,
    CONF_BLK_MSTP_CONF,
    CONF_BLK_UPNP_CONF,
    CONF_BLK_DNS_CONF,
    CONF_BLK_DHCP_RELAY_CONF,
    CONF_BLK_DHCP_SNOOPING_CONF,
    CONF_BLK_DHCP_SNOOPING_PORT_CONF,
    CONF_BLK_NTP_CONF,
    CONF_BLK_USERS_CONF,
    CONF_BLK_PRIVILEGE_CONF,
    CONF_BLK_EVC_CONF,
    CONF_BLK_EVC_UNI_CONF,
    CONF_BLK_ARP_INSPECTION_CONF,
    CONF_BLK_IP_SOURCE_GUARD_CONF,
    CONF_BLK_PTP_CONF,
    CONF_BLK_PSEC_LIMIT_CONF,
    CONF_BLK_MVR_CONF,
    CONF_BLK_VOICE_VLAN_CONF,
    CONF_BLK_VOICE_VLAN_PORT_CONF,
    CONF_BLK_VOICE_VLAN_OUI_TABLE,
    CONF_BLK_EVC_PORT_CONF,
    CONF_BLK_ETH_LINK_OAM_CONF_TABLE,
    CONF_BLK_EEE_CONF,
    CONF_BLK_ERPS_CONF,
    CONF_BLK_FAN_CONF,
    CONF_BLK_LED_POW_REDUC_CONF,
    CONF_BLK_THERMAL_PROTECT_CONF,
    CONF_BLK_DEPRECATED_MCSNP4_CONF,
    CONF_BLK_MPLS_CONF,
    CONF_BLK_IPMC_CONF,
    CONF_BLK_IP_ROUTING,
    CONF_BLK_STACKING,
    CONF_BLK_VCL_MAC_VLAN_TABLE,
    CONF_BLK_VCL_PROTO_VLAN_PRO_TABLE,
    CONF_BLK_VCL_PROTO_VLAN_VID_TABLE,
    CONF_BLK_EVC_POL_CONF,
    CONF_BLK_EVC_ECE_CONF,
    CONF_BLK_PHY_1588_SIM_CONF,
    CONF_BLK_SYSLOG_CONF,
    CONF_BLK_VLAN_TRANS_VLAN_TABLE,
    CONF_BLK_VLAN_TRANS_PORT_TABLE,
    CONF_BLK_CE_MAX_CONF,
    CONF_BLK_MRP_CONF_TABLE,
    CONF_BLK_MVRP_CONF_TABLE,
    CONF_BLK_QOS_LPORT_TABLE,
    CONF_BLK_VLAN_FORBIDDEN,
    CONF_BLK_SNMP_SMON_STAT_TABLE,
    CONF_BLK_SNMP_PORT_COPY_TABLE,
    CONF_BLK_LOOP_PROTECT,
    CONF_BLK_RMON_STAT_TABLE,
    CONF_BLK_RMON_HISTORY_TABLE,
    CONF_BLK_RMON_ALARM_TABLE,
    CONF_BLK_RMON_EVENT_TABLE,
    CONF_BLK_VCL_IP_VLAN_TABLE,
    CONF_BLK_DAYLIGHT_SAVING,
    CONF_BLK_PHY_CONF,
    CONF_BLK_TOD_CONF,
    CONF_BLK_EVC_GLOBAL_CONF,
    CONF_BLK_DEPRECATED_MCSNP6_CONF,
    CONF_BLK_OS_FILE_CONF,
    CONF_BLK_ICLI,
    CONF_BLK_TRAP_CONF,
    CONF_BLK_SNTP_CONF,
    CONF_BLK_RFC2544_PROFILES,
    CONF_BLK_MSG,
    CONF_BLK_RFC2544_REPORTS,

    CONF_BLK_GARP_CONF_TABLE,
    CONF_BLK_GVRP_CONF_TABLE,
    CONF_BLK_PM_LM_REPORTS,
    CONF_BLK_PM_DM_REPORTS,
    CONF_BLK_PM_EVC_REPORTS,
    CONF_BLK_PM_ECE_REPORTS,

    /* New block IDs are added above this line. Block names are added in the array below. */
    CONF_BLK_COUNT
} conf_blk_id_t;

#ifdef __VTSS_CONF_C__
/* Block names are placed here for convenience, but only compiled for conf.c */
static char *conf_blk_name[] = {
    [CONF_BLK_CONF_HDR]                 = "CONF_HDR",
    [CONF_BLK_PORT_CONF_TABLE]          = "PORT_CONF_TABLE",
    [CONF_BLK_IP_CONF]                  = "IP_CONF",
    [CONF_BLK_ACL_PORT_TABLE]           = "ACL_PORT_TABLE",
    [CONF_BLK_ACL_BIND_TABLE]           = "ACL_BIND_TABLE",
    [CONF_BLK_ACL_POLICER_TABLE]        = "ACL_POLICER_TABLE",
    [CONF_BLK_ACL_ACE_TABLE]            = "ACL_ACE_TABLE",
    [CONF_BLK_MIRROR_CONF_TABLE]        = "MIRROR_CONF_TABLE",
    [CONF_BLK_MAC_AGE_CONF_TABLE]       = "MAC_AGE_CONF_TABLE",
    [CONF_BLK_MAC_STATIC_CONF_TABLE]    = "MAC_STATIC_CONF_TABLE",
    [CONF_BLK_TOPO]                     = "TOPO",
    [CONF_BLK_VLAN_PORT_TABLE]          = "VLAN_PORT_TABLE",
    [CONF_BLK_VLAN_TABLE]               = "VLAN_TABLE",
    [CONF_BLK_VLAN_NAME_TABLE]          = "VLAN_NAME_TABLE",
    [CONF_BLK_AGGR_TABLE]               = "AGGR_TABLE",
    [CONF_BLK_QOS_QCL_QCE_TABLE]        = "QOS_QCL_QCE_TABLE",
    [CONF_BLK_QOS_PORT_TABLE]           = "QOS_PORT_TABLE",
    [CONF_BLK_QOS_TABLE]                = "QOS_TABLE",
    [CONF_BLK_TRACE]                    = "TRACE",
    [CONF_BLK_PVLAN_TABLE]              = "PVLAN_TABLE",
    [CONF_BLK_PVLAN_ISOLATE_TABLE]      = "PVLAN_ISOLATE_TABLE",
    [CONF_BLK_SYSTEM]                   = "SYSTEM",
    [CONF_BLK_MAC_LEARN_MODE_TABLE]     = "MAC_LEARN_MODE_TABLE",
    [CONF_BLK_DOT1X]                    = "802.1X",
    [CONF_BLK_RSTP_CONF]                = "RSTP",
    [CONF_BLK_LACP_CONF]                = "LACP",
    [CONF_BLK_LLDP_CONF]                = "LLDP",
    [CONF_BLK_SNMP_CONF]                = "SNMP",
    [CONF_BLK_SNMP_RMON_STAT_TABLE]     = "SNMP_RMON_STAT_TABLE",
    [CONF_BLK_SNMP_RMON_HISTORY_TABLE]  = "SNMP_RMON_HISTORY_TABLE",
    [CONF_BLK_SNMP_RMON_ALARM_TABLE]    = "SNMP_RMON_ALARM_TABLE",
    [CONF_BLK_SNMP_RMON_EVENT_TABLE]    = "SNMP_RMON_EVENT_TABLE",
    [CONF_BLK_IPMCLIB_CONF]             = "IPMCLIB",
    [CONF_BLK_SNMP_PORT_CONF]           = "SNMP_PORT",
    [CONF_BLK_SNMPV3_COMMUNITIES_CONF]  = "SNMPV3_COMMUNITIES",
    [CONF_BLK_SNMPV3_USERS_CONF]        = "SNMPV3_USERS",
    [CONF_BLK_SNMPV3_GROUPS_CONF]       = "SNMPV3_GROUPS",
    [CONF_BLK_SNMPV3_VIEWS_CONF]        = "SNMPV3_VIEWS",
    [CONF_BLK_SNMPV3_ACCESSES_CONF]     = "SNMPV3_ACCESSES",
    [CONF_BLK_SYNCE_CONF_TABLE]         = "SYNCE",
    [CONF_BLK_POE_CONF]                 = "POE",
    [CONF_BLK_EPS_CONF_TABLE]           = "EPS",
    [CONF_BLK_MEP_CONF_TABLE]           = "MEP",
    [CONF_BLK_HTTPS_CONF]               = "HTTPS",
    [CONF_BLK_AUTH_TABLE]               = "AUTH",
    [CONF_BLK_SSH_CONF]                 = "SSH",
    [CONF_BLK_ACCESS_MGMT_CONF]         = "ACCESS_MGMT",
    [CONF_BLK_MSTP_CONF]                = "MSTP",
    [CONF_BLK_UPNP_CONF]                = "UPNP",
    [CONF_BLK_DNS_CONF]                 = "DNS",
    [CONF_BLK_DHCP_RELAY_CONF]          = "DHCP_RELAY",
    [CONF_BLK_DHCP_SNOOPING_CONF]       = "DHCP_SNOOPING",
    [CONF_BLK_DHCP_SNOOPING_PORT_CONF]  = "DHCP_SNOOPING_PORT",
    [CONF_BLK_NTP_CONF]                 = "NTP",
    [CONF_BLK_USERS_CONF]               = "USERS",
    [CONF_BLK_PRIVILEGE_CONF]           = "PRIVILEGE",
    [CONF_BLK_EVC_CONF]                 = "EVC",
    [CONF_BLK_EVC_UNI_CONF]             = "EVC_UNI",
    [CONF_BLK_ARP_INSPECTION_CONF]      = "ARP_INSPECTION",
    [CONF_BLK_IP_SOURCE_GUARD_CONF]     = "IP_SOURCE_GUARD",
    [CONF_BLK_PTP_CONF]                 = "PTP",
    [CONF_BLK_PSEC_LIMIT_CONF]          = "PSEC_LIMIT",
    [CONF_BLK_MVR_CONF]                 = "MVR",
    [CONF_BLK_VOICE_VLAN_CONF]          = "VOICE_VLAN",
    [CONF_BLK_VOICE_VLAN_PORT_CONF]     = "VOICE_VLAN_PORT",
    [CONF_BLK_VOICE_VLAN_OUI_TABLE]     = "VOICE_VLAN_OUI",
    [CONF_BLK_EVC_PORT_CONF]            = "EVC_PORT",
    [CONF_BLK_ETH_LINK_OAM_CONF_TABLE]  = "LINK_OAM",
    [CONF_BLK_EEE_CONF]                 = "EEE",
    [CONF_BLK_ERPS_CONF]                = "ERPS",
    [CONF_BLK_FAN_CONF]                 = "FAN",
    [CONF_BLK_LED_POW_REDUC_CONF]       = "POW_REDUC",
    [CONF_BLK_THERMAL_PROTECT_CONF]     = "THERMAL",
    [CONF_BLK_DEPRECATED_MCSNP4_CONF]   = "DEPRECATED_MCSNP4",
    [CONF_BLK_MPLS_CONF]                = "MPLS",
    [CONF_BLK_IPMC_CONF]                = "IPMC",
    [CONF_BLK_IP_ROUTING]               = "IP_ROUTING",
    [CONF_BLK_STACKING]                 = "STACKING",
    [CONF_BLK_VCL_MAC_VLAN_TABLE]       = "VCL_MAC_CONF",
    [CONF_BLK_VCL_PROTO_VLAN_PRO_TABLE] = "VCL_PROTOVLAN_PROTO_CONF",
    [CONF_BLK_VCL_PROTO_VLAN_VID_TABLE] = "VCL_PROTOVLAN_VLAN_CONF",
    [CONF_BLK_EVC_POL_CONF]             = "EVC_POL_CONF",
    [CONF_BLK_EVC_ECE_CONF]             = "EVC_ECE_CONF",
    [CONF_BLK_PHY_1588_SIM_CONF]        = "PHY_1588_SIM_CONF",
    [CONF_BLK_SYSLOG_CONF]              = "SYSLOG",
    [CONF_BLK_VLAN_TRANS_VLAN_TABLE]    = "VLAN_TRANS_VLAN_CONF",
    [CONF_BLK_VLAN_TRANS_PORT_TABLE]    = "VLAN_TRANS_PORT_CONF",
    [CONF_BLK_CE_MAX_CONF]              = "CE_MAX_CONF",
    [CONF_BLK_MRP_CONF_TABLE]           = "MRP",
    [CONF_BLK_MVRP_CONF_TABLE]          = "MVRP",
    [CONF_BLK_QOS_LPORT_TABLE]          = "QOS_LPORT_TABLE",
    [CONF_BLK_VLAN_FORBIDDEN]           = "VLAN_FORBIDDEN_TABLE",
    [CONF_BLK_SNMP_SMON_STAT_TABLE]     = "SNMP_SMON_STAT_TABLE",
    [CONF_BLK_SNMP_PORT_COPY_TABLE]     = "SNMP_PORT_COPY_TABLE",
    [CONF_BLK_LOOP_PROTECT]             = "LOOP_PROTECT",
    [CONF_BLK_RMON_STAT_TABLE]          = "RMON_STAT_TABLE",
    [CONF_BLK_RMON_HISTORY_TABLE]       = "RMON_HISTORY_TABLE",
    [CONF_BLK_RMON_ALARM_TABLE]         = "RMON_ALARM_TABLE",
    [CONF_BLK_RMON_EVENT_TABLE]         = "RMON_EVENT_TABLE",
    [CONF_BLK_VCL_IP_VLAN_TABLE]        = "VCL_IP_VLAN_TABLE",
    [CONF_BLK_DAYLIGHT_SAVING]          = "DAYLIGHT_SAVING",
    [CONF_BLK_PHY_CONF]                 = "PHY_CONF",
    [CONF_BLK_TOD_CONF]                 = "TOD_CONF",
    [CONF_BLK_EVC_GLOBAL_CONF]          = "EVC_GLOBAL_CONF",
    [CONF_BLK_DEPRECATED_MCSNP6_CONF]   = "DEPRECATED_MCSNP6",
    [CONF_BLK_OS_FILE_CONF]             = "OS_FILE",
    [CONF_BLK_ICLI]                     = "ICLI",
    [CONF_BLK_TRAP_CONF]                = "TRAP",
    [CONF_BLK_SNTP_CONF]                = "SNTP",
    [CONF_BLK_RFC2544_PROFILES]         = "RFC2544_PROFILES",
    [CONF_BLK_MSG]                      = "MSG",
    [CONF_BLK_RFC2544_REPORTS]          = "RFC2544_REPORTS",
    [CONF_BLK_PM_LM_REPORTS]            = "PM_LM_REPORTS",
    [CONF_BLK_PM_DM_REPORTS]            = "PM_DM_REPORTS",
    [CONF_BLK_PM_EVC_REPORTS]           = "PM_EVC_REPORTS",
    [CONF_BLK_PM_ECE_REPORTS]           = "PM_ECE_REPORTS",

    [CONF_BLK_COUNT]                    = "NONE"
};
#endif /* __VTSS_CONF_C__ */

/* Configuration block management/debug information */
typedef struct {
    conf_blk_id_t id;           /* Block ID */
    ulong         size;         /* Block size */
    ulong         crc;          /* CRC for change detection */
    ulong         change_count; /* Number of changes */
    char          name[64];     /* Block name */
    void          *data;        /* Data pointer */
} conf_mgmt_blk_t;

/* Get Local block information (for management/debug only) */
vtss_rc conf_mgmt_sec_blk_get(conf_sec_t sec, conf_blk_id_t id,
                              conf_mgmt_blk_t *mgmt_blk, BOOL next);

/* Configuration module configuration */
typedef struct {
    BOOL stack_copy;    /* Copy messages in stack */
    BOOL flash_save;    /* Save changes to Flash */
    BOOL change_detect; /* Detect changes on conf_sec_close() */
} conf_mgmt_conf_t;

/* Get module configuration */
vtss_rc conf_mgmt_conf_get(conf_mgmt_conf_t *conf);

/* Set module configuration */
vtss_rc conf_mgmt_conf_set(conf_mgmt_conf_t *conf);


/* ================================================================= *
 *  Configuration access for section
 * ================================================================= */

/* Create/resize block, returns NULL on error.
   Use size zero to delete block */
void *conf_sec_create(conf_sec_t sec, conf_blk_id_t id, ulong size);

/* Open block for read/write, returns NULL on error */
void *conf_sec_open(conf_sec_t sec, conf_blk_id_t id, ulong *size);

/* Close block after conf_create/conf_open */
void conf_sec_close(conf_sec_t sec, conf_blk_id_t id);

/* Configuration section information */
typedef struct {
    ulong save_count;
} conf_sec_info_t;

/* Get configuration section information */
void conf_sec_get(conf_sec_t sec, conf_sec_info_t *info);

/* Renew configuration section */
void conf_sec_renew(conf_sec_t sec);

/* ================================================================= *
 *  Configuration access local section
 * ================================================================= */

/* Create/resize block, returns NULL on error.
   Use size zero to delete block */
void *conf_create(conf_blk_id_t id, ulong size);

/* Open block for read/write, returns NULL on error */
void *conf_open(conf_blk_id_t id, ulong *size);

/* Close block after conf_create/conf_open */
void conf_close(conf_blk_id_t id);

/* Force immediate configuration flushing to flash (iff pending) */
void conf_flush(void);

/* ================================================================= *
 *  Configuration module initialization
 * ================================================================= */

vtss_rc conf_init(vtss_init_data_t *data);

#endif /* _VTSS_CONF_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
