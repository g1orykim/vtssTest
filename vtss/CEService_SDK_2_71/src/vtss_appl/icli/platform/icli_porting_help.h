/*

 Vitesse Switch Application software.

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

/*
******************************************************************************

    Revision history
    > CP.Wang, 2012/09/12 13:25
        - create

******************************************************************************
*/
#ifndef __ICLI_PORTING_HELP_H__
#define __ICLI_PORTING_HELP_H__
//****************************************************************************

/*
******************************************************************************

    HELP String
    Please sort the insertion by ICLI_HELP_'X'

******************************************************************************
*/

/* A */
#define ICLI_HELP_AGGREGATION       "Aggregation mode"

/* B */
#define ICLI_HELP_BANNER            "Define a login banner"

/* C */
#define ICLI_HELP_CLEAR             "Reset functions"
#define ICLI_HELP_CLEAR_IP          "IP protocol"
#define ICLI_HELP_CLEAR_IP_DHCP     "Delete items from the DHCP database"

/* D */
#define ICLI_HELP_DEBUG             "Debugging functions"
#define ICLI_HELP_DEFAULT           "Set a command to its defaults"
#define ICLI_HELP_DEFAULT_ROUTE     "Establish default route"
#define ICLI_HELP_DHCP              "Dynamic Host Configuration Protocol"
#define ICLI_HELP_DHCP_BINDING      "DHCP address bindings"
#define ICLI_HELP_DHCP_POOL_NAME    "Pool name in 32 characters"
#define ICLI_HELP_DHCP_RELAY        "DHCP relay agent configuration"
#define ICLI_HELP_DNS               "Domain Name System"
#define ICLI_HELP_DO                "To run exec commands in config mode"
#define ICLI_HELP_DO_LINE           "Exec Command"

/* E */
#define ICLI_HELP_EDITING           "Enable command line editing"
#define ICLI_HELP_END               "Go back to EXEC mode"
#define ICLI_HELP_EXEC_TIMEOUT      "Set the EXEC timeout"
#define ICLI_HELP_EXEC_MIN          "Timeout in minutes"
#define ICLI_HELP_EXEC_SEC          "Timeout in seconds"

/* F */

/* G */

/* H */
#define ICLI_HELP_HISTORY           "Control the command history function"
#define ICLI_HELP_HISTORY_SIZE      "Set history buffer size"
#define ICLI_HELP_HISTORY_NUM       "Number of history commands, 0 means disable"
#define ICLI_HELP_HTTP              "Hypertext Transfer Protocol "

/* I */
#define ICLI_HELP_IGMP              "Internet Group Management Protocol"
#define ICLI_HELP_INTERFACE         "Select an interface to configure"
#define ICLI_HELP_INTF_COMPAT       "Interface compatibility"
#define ICLI_HELP_INTF_PRI          "Interface CoS priority"
#define ICLI_HELP_INTF_RV           "Robustness Variable"
#define ICLI_HELP_INTF_QI           "Query Interval in seconds"
#define ICLI_HELP_INTF_QRI          "Query Response Interval in tenths of seconds"
#define ICLI_HELP_INTF_LMQI         "Last Member Query Interval in tenths of seconds"
#define ICLI_HELP_INTF_URI          "Unsolicited Report Interval in seconds"
#define ICLI_HELP_IMD_LEAVE         "Immediate leave configuration"
#define ICLI_HELP_IP                "Interface Internet Protocol config commands"
#define ICLI_HELP_IP_DHCP           "Configure DHCP server parameters"
#define ICLI_HELP_IP4_ADRS          "Configure the IPv4 address of an interface"
#define ICLI_HELP_IP6_ADRS          "Configure the IPv6 address of an interface"
#define ICLI_HELP_IPMC              "IPv4/IPv6 multicast configuration"
#define ICLI_HELP_IPMC_PROFILE      "IPMC profile configuration"
#define ICLI_HELP_IPMC_RANGE        "A range of IPv4/IPv6 multicast addresses for the profile"
#define ICLI_HELP_IPMC_VID          "VLAN identifier(s): VID"
#define ICLI_HELP_IPV6              "IPv6 configuration commands"

/* J */

/* K */

/* L */
#define ICLI_HELP_LENGTH            "Set number of lines on a screen"
#define ICLI_HELP_LENGTH_NUM        "Number of lines on screen (0 for no pausing)"
#define ICLI_HELP_LINE              "Configure a terminal line"
#define ICLI_HELP_LOCATION          "Enter terminal location description"

/* M */
#define ICLI_HELP_MLD               "Multicasat Listener Discovery"
#define ICLI_HELP_MOTD              "Set Message of the Day banner"
#define ICLI_HELP_MROUTER           "Multicast router port configuration"
#define ICLI_HELP_MVR               "Multicast VLAN Registration configuration"
#define ICLI_HELP_MVR_NAME          "MVR multicast name"
#define ICLI_HELP_MVR_VLAN          "MVR multicast vlan"

/* N */
#define ICLI_HELP_NO                "Negate a command or set its defaults"

/* O */

/* P */
#define ICLI_HELP_PING              "Send ICMP echo messages"
#define ICLI_HELP_PORT_ID           "Port ID in the format of switch-id/port-id, ex, 1/5"
#define ICLI_HELP_PORT_LIST         "List of port ID, ex, 1/1,3-5;2/2-4,6"
#define ICLI_HELP_PORT_TYPE         "Port type in Fast, Gigabit or Tengigabit ethernet"
#define ICLI_HELP_PORT_TYPE_LIST    "List of port type and port ID, ex, Fast 1/1 Gigabit 2/3-5 Gigabit 3/2-4 Tengigabit 4/6"
#define ICLI_HELP_PRIVILEGE         "Change privilege level for line"
#define ICLI_HELP_PRIVILEGE_LEVEL   "Assign default privilege level for line"
#define ICLI_HELP_PROFILE_NAME      "Profile name in 16 char's"

/* Q */

/* R */
#define ICLI_HELP_ROUTE             "Configure static routes"
#define ICLI_HELP_ROUTE4_NH         "Next hop router's IPv4 address"
#define ICLI_HELP_ROUTE6_NH         "Next hop router's IPv6 address"

/* S */
#define ICLI_HELP_SERVICE           "Modify use of network based services"
#define ICLI_HELP_SHOW              "Show running system information"
#define ICLI_HELP_SHOW_INTERFACE    "Interface status and configuration"
#define ICLI_HELP_SHOW_IP           "IP information"
#define ICLI_HELP_SHOW_IP_DHCP      "Show items in the DHCP database"
#define ICLI_HELP_SNMP              "Set SNMP server's configurations"
#define ICLI_HELP_SNMP_HOST         "Set SNMP host's configurations"
#define ICLI_HELP_SNMP_HOST_NAME    "Name of the host configuration"
#define ICLI_HELP_STATELESS         "Obtain IPv6 address using autoconfiguration"
#define ICLI_HELP_STATISTICS        "Traffic statistics"
#define ICLI_HELP_STATUS            "Status"
#define ICLI_HELP_STP               "Spanning Tree protocol"
#define ICLI_HELP_SSH               "Secure Shell"
#define ICLI_HELP_SWITCHPORT        "Set switching mode characteristics"
#define ICLI_HELP_SWITCH            "Switch"
#define ICLI_HELP_SWITCH_ID         "Switch ID"
#define ICLI_HELP_SWITCH_LIST       "List of switch ID, ex, 1,3-5,6"

/* T */

/* U */

/* V */

/* W */
#define ICLI_HELP_WIDTH             "Set width of the display terminal"
#define ICLI_HELP_WIDTH_NUM         "Number of characters on a screen line (0 for unlimited width)"

/* X */

/* Y */

/* Z */


//****************************************************************************
#endif //__ICLI_PORTING_HELP_H__

