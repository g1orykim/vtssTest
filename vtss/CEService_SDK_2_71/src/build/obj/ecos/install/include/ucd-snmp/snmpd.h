//==========================================================================
//
//      ./agent/current/include/snmpd.h
//
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 * snmpd.h
 */

#include "snmp_vars.h"

#define MASTER_AGENT 0
#define SUB_AGENT    1
extern int agent_role;

extern int snmp_dump_packet;
extern int verbose;
extern int (*sd_handlers[])(int);
extern int smux_listen_sd;

extern int snmp_read_packet (int);
extern u_char *getStatPtr (oid *, size_t *, u_char *, size_t *,
	u_short *, int, WriteMethod **write_method, struct snmp_pdu *, int *);

/* config file parsing routines */
void agentBoots_conf (char *, char *);

/* peter, 2007/6, add new function for configure SNMP module */
void SnmpdSetVersion(int version);
void SnmpdGetVersion(int *version);
void SnmpdSetMode(u_long mode);
void SnmpdGetPort(u_short *snmp_port);
void SnmpdSetPort(u_short snmp_port);
int SnmpdGetAgentState(void);
extern void cyg_net_snmp_init(void);
extern void cyg_net_snmp_thread_suspend(void);
extern void cyg_net_snmp_thread_resume(void);

/* peter, 2007/6, add new function for configure SNMP trap modlue */
#ifndef COMMUNITY_MAX_LEN /* declared in snmp_impl.h */
#define COMMUNITY_MAX_LEN	256
#endif

void SnmpdGetTrapMode(u_long *mode);
void SnmpdSetTrapMode(u_long mode);
void SnmpdSetTrapPort(u_short trap_port);
void SnmpdSetTrapDestinationIp(u_char ip_string[16]);
#ifdef CYGPKG_NET_FREEBSD_INET6
void SnmpdSetTrapDestinationIpv6(u_char ip_string[40]);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
void SnmpdSetTrapCommunity(u_char community_string[COMMUNITY_MAX_LEN]);
void SnmpdGetTrapVersion(int *trap_version);
int  SnmpdSetTrapVersion(int trap_version);
void SnmpdSendTrap(int trap_type, long *args);
void SnmpdGetTrapInformMode(u_long *mode);
void SnmpdSetTrapInformMode(u_long mode);
void SnmpdGetTrapInformTimeout(u_long *timeout);
void SnmpdSetTrapInformTimeout(u_long timeout);
void SnmpdGetTrapInformRetries(u_long *retries);
void SnmpdSetTrapInformRetries(u_long retries);
extern void SnmpdEnableAuthenTraps(int is_enable); /* declared in agent_trap.c */
typedef void (*mib_modules_init_callback_t)(void);
void SnmpdMibModulesRegister(mib_modules_init_callback_t cb);
void SnmpdSetVersionIdOid(oid *version_id_oid, int length);

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
/* peter, 2007/12, add new function for configure SNMP modlue */
extern int SnmpdSetEngineId(const char *engine_id, int len); /* declared in snmpv3.c */
extern void SnmpdDelCommunityEntry (unsigned char *community_p); /* declared in vacm_vars.c */
extern void SnmpdDestroyAllUserEntries(void); /* declared in snmpusm.c */
extern int SnmpdChangeAuthPd(u_char *engineID, size_t engineIDLen, char *name, char *newPassword); /* declared in snmpusm.c */
extern int SnmpdChangePrivPd(u_char *engineID, size_t engineIDLen, char *name, char *newPassword); /* declared in snmpusm.c */
extern int SnmpdGetRemoteEngineId(char *peername, u_char *engineID, unsigned long *engineID_len);
void SnmpdGetTrapSecurityEngineIdProbe(u_long *mode);
void SnmpdSetTrapSecurityEngineIdProbe(u_long mode);
void SnmpdNtpPostConfig(void);
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

//Get netsnmp_thread_handle
void cyg_net_snmp_thread_handle_get(cyg_handle_t *thread_handle);
