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

#include "cli.h"
#include "cli_grp_help.h"

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* CLI command group structure */
typedef struct {
    char        *grp_name;  /* Name of the module/group */
    char        *descr;     /* Group description string */
} cli_cmd_grp_t;

static cli_cmd_grp_t cli_cmd_grp_table[] = {
    {
        "System",
        "System settings and reset options"
    },
#if VTSS_SWITCH_STACKABLE
    {
        "Stack",
        "Stack management"
    },
#endif
#if defined(VTSS_SW_OPTION_IP2)
    {
        "IP",
        "IP configuration and Ping"
    },
#endif /* defined(VTSS_SW_OPTION_IP2) */
    {
        "Port",
        "Port management"
    },
#ifdef VTSS_SW_OPTION_MAC
    {
        "MAC",
        "MAC address table"
    },
#endif
#ifdef VTSS_SW_OPTION_VLAN
    {
        "VLAN",
        "Virtual LAN"
    },
#endif
#ifdef VTSS_SW_OPTION_PVLAN
    {
        "PVLAN",
        "Private VLAN"
    },
#endif
#ifdef VTSS_CLI_SEC_GRP
    {
        "Security",
        "Security management"
    },
#endif
#ifndef VTSS_CLI_SEC_GRP
#ifdef VTSS_SW_OPTION_SSH
    {
        "SSH",
        "Secure Shell"
    },
#endif
#ifdef VTSS_SW_OPTION_HTTPS
    {
        "HTTPS",
        "Hypertext Transfer Protocol over Secure Socket Layer"
    },
#endif
#ifdef VTSS_SW_OPTION_USERS
    {
        "Users",
        "User management"
    },
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
    {
        "Privilege",
        "Privilege level"
    },
#endif
#ifdef VTSS_SW_OPTION_DOT1X
    {
        "NAS",
        "Network Access Server (IEEE 802.1X)"
    },
#endif
#endif /* VTSS_CLI_SEC_GRP */
#ifdef VTSS_SW_OPTION_MSTP
    {
        "STP",
        "Spanning Tree Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_IGMPS
    {
        "IGMP",
        "Internet Group Management Protocol snooping"
    },
#endif
#ifdef VTSS_SW_OPTION_AGGR
    {
        "Aggr",
        "Link Aggregation"
    },
#endif
#ifdef VTSS_SW_OPTION_LACP
    {
        "LACP",
        "Link Aggregation Control Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_LLDP
    {
        "LLDP",
        "Link Layer Discovery Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_LLDP_MED
    {
        "LLDPMED",
        "Link Layer Discovery Protocol Media"
    },
#endif
#if defined(VTSS_SW_OPTION_EEE) || defined(VTSS_SW_OPTION_FAN) || defined(VTSS_SW_OPTION_LED_POW_REDUC)
    {
        "GreenEthernet",
        "Power savings features"
    },
#endif
#ifdef VTSS_SW_OPTION_THERMAL_PROTECT
    {
        "Thermal",
        "Thermal Protection"
    },
#endif
#ifdef VTSS_SW_OPTION_POE
    {
        "PoE",
        "Power Over Ethernet"
    },
#endif
#ifdef VTSS_SW_OPTION_SYNCE
    {
        "SyncE",
        "Ethernet Synchronization"
    },
#endif
#ifdef VTSS_SW_OPTION_EVC
    {
        "EVC",
        "Ethernet Virtual Connections"
    },
#endif
#ifdef VTSS_SW_OPTION_EPS
    {
        "EPS",
        "Ethernet Protection Switching"
    },
#endif
#ifdef VTSS_SW_OPTION_MEP
    {
        "MEP",
        "Maintainence entity End Point"
    },
#endif
#ifndef VTSS_CLI_SEC_GRP
#ifdef VTSS_SW_OPTION_ACL
    {
        "ACL",
        "Access Control List"
    },
#endif
#endif /* VTSS_CLI_SEC_GRP */
#ifdef VTSS_SW_OPTION_MIRROR
    {
        "Mirror",
        "Port mirroring"
    },
#endif






#ifdef VTSS_SW_OPTION_FIRMWARE
    {
        "Firmware",
        "Download of firmware via TFTP"
    },
#endif
#ifndef VTSS_CLI_SEC_GRP
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    {
        "ARP",
        "Address Resolution Protocol"
    },
#endif
#if defined(VTSS_SW_OPTION_DHCP_RELAY) || defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    {
        "DHCP",
        "Dynamic Host Configuration Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_SNMP
    {
        "SNMP",
        "Simple Network Management Protocol"
    },
#endif
#endif /* VTSS_CLI_SEC_GRP */
#ifdef VTSS_SW_OPTION_UPNP
    {
        "UPnP",
        "Universal Plug and Play"
    },
#endif
#ifndef VTSS_CLI_SEC_GRP
#ifdef VTSS_SW_OPTION_PSEC
    {
        "Psec",
        "Port Security Status"
    },
#endif
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    {
        "Limit",
        "Port Security Limit Control"
    },
#endif
#endif /* VTSS_CLI_SEC_GRP */
#ifdef VTSS_SW_OPTION_PTP
    {
        "PTP",
        "IEEE1588 Precision Time Protocol"
    },
#endif






#ifdef VTSS_SW_OPTION_MVR
    {
        "MVR",
        "Multicast VLAN Registration"
    },
#endif
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    {
        "Voice VLAN",
        "Specific VLAN for voice traffic"
    },
#endif
#ifdef VTSS_SW_OPTION_ERPS
    {
        "ERPS",
        "Ethernet Ring Protection Switching"
    },
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    {
        "LOAM",
        "Ethernet Link OAM"
    },
#endif
#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    {
        "Loop Protect",
        "Loop Protection"
    },
#endif
#ifdef VTSS_SW_OPTION_MLDSNP
    {
        "MLD",
        "Multicast Listener Discovery (Snooping)"
    },
#endif
#ifdef VTSS_SW_OPTION_MPLS
    {
        "MPLS",
        "Multi Protocol Label Switching"
    },
#endif
#ifdef VTSS_SW_OPTION_IPMC
    {
        "IPMC",
        "MLD/IGMP Snooping"
    },
#endif
#ifdef VTSS_SW_OPTION_SFLOW
    {
        "sFlow",
        "sFlow Agent"
    },
#endif
#ifdef VTSS_SW_OPTION_VCL
    {
        "VCL",
        "VLAN Control List"
    },
#endif
























    {
        "Debug",
        "Switch debug facilities"
    }
};

void cli_cmd_grp_disp(void)
{
    int i, num_entries = sizeof(cli_cmd_grp_table) / sizeof(cli_cmd_grp_t);
    u32 max_len = 0;
    char format_buf[30];

    for (i = 0; i < num_entries; i++) {
        u32 len = strlen(cli_cmd_grp_table[i].grp_name);
        if (len > max_len) {
            max_len = len;
        }
    }

    sprintf(format_buf, "%%-%ds: %%s\n", max_len);

    for (i = 0; i < num_entries; i++) {
        CPRINTF(format_buf, cli_cmd_grp_table[i].grp_name, cli_cmd_grp_table[i].descr);
    }
}

#ifdef VTSS_CLI_SEC_GRP
static cli_cmd_grp_t cli_cmd_sec_grp_table[] = {
    {
        "Switch",
        "Switch security"
    },
    {
        "Network",
        "Network security"
    }
};

void cli_cmd_sec_grp_disp(void)
{
    int i, num_entries = sizeof(cli_cmd_sec_grp_table) / sizeof(cli_cmd_grp_t);

    cli_header_nl("Command Groups", 1, 0);
    for (i = 0; i < num_entries; i++) {
        CPRINTF("%-10s: %s\n", cli_cmd_sec_grp_table[i].grp_name,
                cli_cmd_sec_grp_table[i].descr);
    }

    CPRINTF("\nType '<group>' to enter command group\n");
    CPRINTF("Type '<group> ?' to get group help\n");
}

static cli_cmd_grp_t cli_cmd_sec_switch_grp_table[] = {
#ifdef VTSS_SW_OPTION_USERS
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "Users",
        "User management"
    },
#else
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "Password",
        "System password"
    },
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "Privilege",
        "Privilege level"
    },
#endif
#ifdef VTSS_SW_OPTION_SSH
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "SSH",
        "Secure Shell"
    },
#endif
#ifdef VTSS_SW_OPTION_HTTPS
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "HTTPS",
        "Hypertext Transfer Protocol over Secure Socket Layer"
    },
#endif
#ifdef VTSS_SW_OPTION_ACCESS_MGMT
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "Access",
        "Access management"
    },
#endif
#ifdef VTSS_SW_OPTION_SNMP
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "SNMP",
        "Simple Network Management Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_RMON
    {
        VTSS_CLI_GRP_SEC_SWITCH_PATH "RMON",
        "Remote Network Monitoring"
    },
#endif

};

void cli_cmd_sec_switch_grp_disp(void)
{
    int i, num_entries = sizeof(cli_cmd_sec_switch_grp_table) / sizeof(cli_cmd_grp_t);

    cli_header_nl("Command Groups", 1, 0);
    for (i = 0; i < num_entries; i++) {
        CPRINTF("%-25s: %s\n", cli_cmd_sec_switch_grp_table[i].grp_name,
                cli_cmd_sec_switch_grp_table[i].descr);
    }

    CPRINTF("\nType '<group>' to enter command group\n");
    CPRINTF("Type '<group> ?' to get list of group commands\n");
    CPRINTF("Type '<group> <command> ?' to get help on a command\n");
}

static cli_cmd_grp_t cli_cmd_sec_network_grp_table[] = {
#ifdef VTSS_SW_OPTION_PSEC
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "Psec",
        "Port Security Status"
    },
#endif
#ifdef VTSS_SW_OPTION_PSEC_LIMIT
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "Limit",
        "Port Security Limit Control"
    },
#endif
#ifdef VTSS_SW_OPTION_DOT1X
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "NAS",
        "Network Access Server (IEEE 802.1X)"
    },
#endif
#ifdef VTSS_SW_OPTION_ACL
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "ACL",
        "Access Control List"
    },
#endif
#if defined(VTSS_SW_OPTION_DHCP_RELAY) || defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "DHCP",
        "Dynamic Host Configuration Protocol"
    },
#endif
#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "IP",
        "IP Source Guard"
    },
#endif
#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    {
        VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP",
        "Address Resolution Protocol"
    },
#endif
};

void cli_cmd_sec_network_grp_disp(void)
{
    int i, num_entries = sizeof(cli_cmd_sec_network_grp_table) / sizeof(cli_cmd_grp_t);

    cli_header_nl("Command Groups", 1, 0);
    for (i = 0; i < num_entries; i++) {
        CPRINTF("%-25s: %s\n", cli_cmd_sec_network_grp_table[i].grp_name,
                cli_cmd_sec_network_grp_table[i].descr);
    }

    CPRINTF("\nType '<group>' to enter command group\n");
    CPRINTF("Type '<group> ?' to get list of group commands\n");
    CPRINTF("Type '<group> <command> ?' to get help on a command\n");
}

#endif /* VTSS_CLI_SEC_GRP */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
