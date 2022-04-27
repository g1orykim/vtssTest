//==========================================================================
//
//      ./agent/current/include/agent_trap.h
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
#ifndef AGENT_TRAP_H
#define AGENT_TRAP_H

void send_easy_trap (int, int);
void send_trap_pdu(struct snmp_pdu *);
void send_v2trap ( struct variable_list *);
void send_trap_vars (int, int, struct variable_list *);

void snmpd_parse_config_authtrap (const char *, char *);
void snmpd_parse_config_trapsink (const char *, char *);
void snmpd_parse_config_trap2sink (const char *, char *);
void snmpd_parse_config_informsink (const char *, char *);
void snmpd_free_trapsinks (void);
void snmpd_parse_config_trapcommunity (const char *, char *);
void snmpd_free_trapcommunity (void);

int create_trap_session (char *, u_short, char *, int, int);
int add_trap_session( struct snmp_session *, int, int, int, int);

#ifdef CYGPKG_NET_FREEBSD_INET6
int create_trap_session_v6 (char *, u_short, char *, int, int);
#endif /* CYGPKG_NET_FREEBSD_INET6 */

#ifdef HAVE_MULTI_TRAP_DEST

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpv3.h"
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

#define VTSS_UCD_TRAP_REQUEST_MAX 64
#define VTSS_UCD_TRAP_HOSTNAME_LEN 45
#define VTSS_UCD_TRAP_SECURITY_NAME_LEN 32
typedef struct {
    unsigned long          conf_id;
    struct snmp_session    *sesp;
    unsigned char          valid;
    int                    next_sess_id;
    int                    prev_sess_id;
}ucd_trap_session_t;

typedef struct {
    u_long      conf_id;
    char        trap_sip_str[VTSS_UCD_TRAP_HOSTNAME_LEN + 1];
    u_char      ipv6_flag;
    char        trap_dip_str[VTSS_UCD_TRAP_HOSTNAME_LEN + 1];
    u_short     trap_port;
    u_long      trap_version;
    char        trap_community[COMMUNITY_MAX_LEN + 1];
    u_char      trap_inform_mode;
    u_long      trap_inform_timeout;
    u_long      trap_inform_retries;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    u_char      trap_probe_engineid;
    u_char      trap_engineid[MAX_ENGINEID_LENGTH];
    u_long      trap_engineid_len;
    u_char      trap_security_name[VTSS_UCD_TRAP_SECURITY_NAME_LEN + 1];
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
    int         generic;
    u_long      oid[MAX_OID_LEN];
    u_long      oid_len;
    struct      variable_list *vars;
}ucd_trap_conf_t;


void ucd_trap_init (void);

/**
  * \brief Send v1/v2 trap message
  *
  * \param trap_conf      [IN]: The trap information for sending trap
instance with its current value.
  */
int ucd_send_trap_message (ucd_trap_conf_t *trap_conf);

/**
  * \brief Send SNMPv3 probe message
  *
  * \param trap_conf      [IN]: The trap information for sending probe trap
instance with its current value.
  */
int ucd_send_probe_message (ucd_trap_conf_t *trap_conf);

/**
  * \brief Destroy trap session based on trap configuration ID, that is, trap_conf ->conf_id in ucd_send_trap_message()
  *
  * \param conf_id      [IN]: Index of the trap configuration
  */
void ucd_destroy_session (u_long conf_id);

#endif /* HAVE_MULTI_TRAP_DEST */

#endif /* AGENT_TRAP_H */
