/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef RFC1213_MIB2_H
#define RFC1213_MIB2_H

#define RFC1213_SUPPORTED_SYSTEM       1
#define RFC1213_SUPPORTED_ORTABLE      1
#define RFC1213_SUPPORTED_INTERFACES   1
#define RFC1213_SUPPORTED_IP           1
#define RFC1213_SUPPORTED_ICMP         0 /* deprecated by RFC4293 */
#define RFC1213_SUPPORTED_TCP          1
#define RFC1213_SUPPORTED_UDP          1
#define RFC1213_SUPPORTED_SNMP         1

#include "ifIndex_api.h"
#include "iana_ifType.h"

/*
 * Function declarations
 */
#if RFC1213_SUPPORTED_SYSTEM
/* system ----------------------------------------------------------*/
void init_mib2_system(void);
FindVarMethod var_system;
WriteMethod write_sysName;
WriteMethod write_sysLocation;
WriteMethod write_sysContact;
#endif /* RFC1213_SUPPORTED_SYSTEM */

#if RFC1213_SUPPORTED_INTERFACES
/* interfaces ----------------------------------------------------------*/
#include "ifIndex_api.h"

#define IFDESCR_MAX_LEN       255

typedef struct {
    u_long ifIndex;
    u_char ifDescr[IFDESCR_MAX_LEN + 1];
    u_long ifType;
    u_long ifMtu;
    u_long ifSpeed;
    u_char ifPhysAddress[6];
    u_long ifAdminStatus;
    u_long ifOperStatus;
    u_long ifLastChange;
    u_long ifInOctets;
    u_long ifInUcastPkts;
    u_long ifInNUcastPkts;
    u_long ifInDiscards;
    u_long ifInErrors;
    u_long ifInUnknownProtos;
    u_long ifOutOctets;
    u_long ifOutUcastPkts;
    u_long ifOutNUcastPkts;
    u_long ifOutDiscards;
    u_long ifOutErrors;
    u_long ifOutQLen;
    oid    ifSpecific[MAX_OID_LEN];
    size_t ifSpecific_len;
} ifTable_entry_t;

void init_mib2_interfaces(void);
ulong rfc1213_get_ifNumber(void);
BOOL get_ifTable_entry(iftable_info_t *table_info, ifTable_entry_t *table_entry_p);
vtss_rc rfc1213_set_ifAdminStatus(vtss_isid_t isid, vtss_port_no_t port_no, BOOL status);
void interfaces_port_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info);
WriteMethod write_ifAdminStatus;
#endif /* RFC1213_SUPPORTED_INTERFACES */

#if RFC1213_SUPPORTED_IP
/* ip ----------------------------------------------------------*/
void init_mib2_ip(void);
#endif /* RFC1213_SUPPORTED_IP */

#if RFC1213_SUPPORTED_ICMP
/* icmp ----------------------------------------------------------*/
void init_mib2_icmp(void);
#endif /* RFC1213_SUPPORTED_ICMP */

#if RFC1213_SUPPORTED_TCP
/* tcp ----------------------------------------------------------*/
void init_mib2_tcp(void);
#endif /* RFC1213_SUPPORTED_TCP */

#if RFC1213_SUPPORTED_UDP
/* udp ----------------------------------------------------------*/
void init_mib2_udp(void);
#endif /* RFC1213_SUPPORTED_UDP */

#if RFC1213_SUPPORTED_SNMP
/* snmp ----------------------------------------------------------*/
void init_mib2_snmp(void);
#endif /* RFC1213_SUPPORTED_SNMP */

int get_available_lport_ifTableIndex(int if_num);
int get_available_lag_ifTableIndex(int if_num);
int get_available_vlan_ifTableIndex(int if_num);
int get_available_ifTableIndex(int if_num);
BOOL get_ifTableIndex_info(int if_index, iftable_info_t *table_info_p);
BOOL RFC1213_MIB2C_is_nvt_string(char *str);

#endif /* RFC1213_MIB2_H */

