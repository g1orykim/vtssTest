//==========================================================================
//
//      ./agent/current/src/agent_trap.c
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
/* agent_trap.c: define trap generation routines for mib modules, etc,
   to use */

#include <config.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#elif HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp_client.h"
#include "snmp.h"
#include "system.h"
#include "read_config.h"
#include "snmp_debug.h"
#include "callback.h"
#include "agent_callbacks.h"
#include "snmpd.h"
#include "snmpusm.h"
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpv3.h"
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

#include "mt_support.h"

struct trap_sink {
    struct snmp_session	*sesp;
    struct trap_sink	*next;
    int			pdutype;
    int			version;
    /* James: add the following field to indicate if the destination is ipv6 global/link local */    
    int         is_v6;
    int         is_link_local;
};

struct trap_sink *sinks	  = NULL;

extern struct timeval	starttime;

#ifdef CYGPKG_NET_FREEBSD_INET6
//char current_ipv6_ip_addr[40] = "fe80::201:c1ff:fe00:3760";
char current_ipv6_ip_addr[40];
#endif /* CYGPKG_NET_FREEBSD_INET6 */

/* peter, 2007/8, define in asn1.h
#define OID_LENGTH(x)  (sizeof(x)/sizeof(x[0])) */

/* peter, 2007/12,
   EstaX-34 project doesn't support rfc1213 - private/enterprises (1.3.6.1.4.1) now,
   and reserve the value for custom defined themselves.
   Please refer to the value of 'SNMP_CONF_SYS_OBJECT_ID' in ..\sw_custom_switch\snmp_custom_api.h
oid objid_enterprisetrap[] = { EXTENSIBLEMIB, 251 };
oid version_id[]	   = { EXTENSIBLEMIB, AGENTID, OSTYPE }; */
oid objid_enterprisetrap[] = { 0, 0 };
oid version_id[MAX_OID_LEN];
int enterprisetrap_len = OID_LENGTH( objid_enterprisetrap );
int version_id_len     = 2 /*OID_LENGTH( version_id )*/;

#define SNMPV2_TRAPS_PREFIX	SNMP_OID_SNMPMODULES,1,1,5
oid  cold_start_oid[] =		{ SNMPV2_TRAPS_PREFIX, 1 };	/* SNMPv2-MIB */
oid  warm_start_oid[] =		{ SNMPV2_TRAPS_PREFIX, 2 };	/* SNMPv2-MIB */
oid  link_down_oid[] =		{ SNMPV2_TRAPS_PREFIX, 3 };	/* IF-MIB */
oid  link_up_oid[] =		{ SNMPV2_TRAPS_PREFIX, 4 };	/* IF-MIB */
oid  auth_fail_oid[] =		{ SNMPV2_TRAPS_PREFIX, 5 };	/* SNMPv2-MIB */
oid  egp_xxx_oid[] =		{ SNMPV2_TRAPS_PREFIX, 99 };	/* ??? */

#define SNMPV2_TRAP_OBJS_PREFIX	SNMP_OID_SNMPMODULES,1,1,4
oid  snmptrap_oid[] 	      =	{ SNMPV2_TRAP_OBJS_PREFIX, 1, 0 };
oid  snmptrapenterprise_oid[] =	{ SNMPV2_TRAP_OBJS_PREFIX, 3, 0 };
oid  sysuptime_oid[] 	      =	{ SNMP_OID_MIB2,1,3,0 };
int  snmptrap_oid_len 	      =	OID_LENGTH(snmptrap_oid);
int  snmptrapenterprise_oid_len = OID_LENGTH(snmptrapenterprise_oid);
int  sysuptime_oid_len 	      =	OID_LENGTH(sysuptime_oid);


#define SNMP_AUTHENTICATED_TRAPS_ENABLED	1
#define SNMP_AUTHENTICATED_TRAPS_DISABLED	2

int	 snmp_enableauthentraps	= SNMP_AUTHENTICATED_TRAPS_DISABLED;
char	*snmp_trapcommunity	= NULL;

/* Prototypes */
 /*
static int create_v1_trap_session (const char *, u_short, const char *);
static int create_v2_trap_session (const char *, u_short, const char *);
static int create_v2_inform_session (const char *, u_short, const char *);
static void free_trap_session (struct trap_sink *sp);
static void send_v1_trap (struct snmp_session *, int, int);
static void send_v2_trap (struct snmp_session *, int, int, int);
 */


	/*******************
	 *
	 * Trap session handling
	 *
	 *******************/
int add_trap_session( struct snmp_session *ss, int pdutype, int version, int is_v6, int is_link_local)
{
    struct trap_sink *new_sink =
      (struct trap_sink *) malloc (sizeof (*new_sink));
    if ( new_sink == NULL )
	return 0;

    new_sink->sesp    = ss;
    new_sink->pdutype = pdutype;
    new_sink->version = version;
    new_sink->next    = sinks;
    
    /* James: add the following field to indicate if the destination is ipv6 global/link local */    
    new_sink->is_v6            = is_v6;
    new_sink->is_link_local    = is_link_local;
    sinks = new_sink;
    return 1;
}

int create_trap_session (char *sink, u_short sinkport,
				char *com,
				int version, int pdutype)
{
    struct snmp_session	 session, *sesp;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    u_long trap_security_engineid_probe;
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

    memset (&session, 0, sizeof (struct snmp_session));
    session.peername = sink;
    session.version = version;
    if (com) {
        session.community = (u_char *)com;
        session.community_len = strlen (com);
    }

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (version == SNMP_VERSION_3) {
        SnmpdGetTrapSecurityEngineIdProbe(&trap_security_engineid_probe);
        if (!trap_security_engineid_probe) {
            session.flags |= SNMP_FLAGS_DONT_PROBE;
        }
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

    /* peter, 2007/12, set retries for trap inform */
    if (pdutype == SNMP_MSG_INFORM) {
        SnmpdGetTrapInformTimeout((u_long *)&session.timeout);
        SnmpdGetTrapInformRetries((u_long *)&session.retries);
    }

    session.remote_port = sinkport;
    sesp = snmp_open (&session);

    if (sesp) {
        /* James: add the following field to indicate if the destination is ipv6 global/link local */    
	    return( add_trap_session( sesp, pdutype, version, 0, 0));
    }

    /* peter, 2007/12,
       diagnose snmp_open errors with the input struct snmp_session pointer
    snmp_sess_perror("snmpd: create_trap_session", &session); */
    return 0;
}

#ifdef CYGPKG_NET_FREEBSD_INET6
int create_trap_session_v6 (char *sink, u_short sinkport,
				char *com,
				int version, int pdutype)
{
    struct snmp_session	 session, *sesp;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    u_long trap_security_engineid_probe;
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
    int is_link_local = 0; 
    
    memset (&session, 0, sizeof (struct snmp_session));
    session.peername = sink;
    session.version = version;
    if (com) {
        session.community = (u_char *)com;
        session.community_len = strlen (com);
    }

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (version == SNMP_VERSION_3) {
        SnmpdGetTrapSecurityEngineIdProbe(&trap_security_engineid_probe);
        if (!trap_security_engineid_probe) {
            session.flags |= SNMP_FLAGS_DONT_PROBE;
        }
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

    /* peter, 2007/12, set retries for trap inform */
    if (pdutype == SNMP_MSG_INFORM) {
        SnmpdGetTrapInformTimeout((u_long *)&session.timeout);
        SnmpdGetTrapInformRetries((u_long *)&session.retries);
    }

    session.remote_port = sinkport;
    sesp = snmp_open_v6 (&session);

    if (sesp) {
        /* James: add the following field to indicate if the destination is ipv6 global/link local */    
	    if (memcmp(&session.peername[0], "fe80", 4) == 0) {
            is_link_local = 1;
        }
	    return( add_trap_session( sesp, pdutype, version, 1 , is_link_local));
    }

    /* peter, 2007/12,
       diagnose snmp_open errors with the input struct snmp_session pointer
    snmp_sess_perror("snmpd: create_trap_session", &session); */
    return 0;
}
#endif /* CYGPKG_NET_FREEBSD_INET6 */

static int create_v1_trap_session (char *sink, u_short sinkport,
				   char *com)
{
    return create_trap_session( sink, sinkport, com,
				SNMP_VERSION_1, SNMP_MSG_TRAP );
}

static int create_v2_trap_session (char *sink,  u_short sinkport,
				   char *com)
{
    return create_trap_session( sink, sinkport, com,
				SNMP_VERSION_2c, SNMP_MSG_TRAP2 );
}

static int create_v2_inform_session (char *sink,  u_short sinkport,
				     char *com)
{
    return create_trap_session( sink, sinkport, com,
				SNMP_VERSION_2c, SNMP_MSG_INFORM );
}


static void free_trap_session (struct trap_sink *sp)
{
    snmp_close(sp->sesp);
    free (sp);
}


void snmpd_free_trapsinks (void)
{
    struct trap_sink *sp = sinks;
    while (sp) {
	sinks = sinks->next;
	free_trap_session(sp);
	sp = sinks;
    }
}

	/*******************
	 *
	 * Trap handling
	 *
	 *******************/

void send_enterprise_trap_vars (int trap,
		     int specific,
		     oid *enterprise, int enterprise_length,
		     struct variable_list *vars)
{
    struct variable_list uptime_var, snmptrap_var, enterprise_var;
    struct variable_list *v2_vars, *last_var=NULL;
    struct snmp_pdu	*template_pdu, *pdu;
    struct timeval	 now;
    struct timespec now_n;
    long uptime;
    struct sockaddr_in *pduIp;
#ifdef CYGPKG_NET_FREEBSD_INET6
    struct sockaddr_in6 *pduIp_v6;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
    struct trap_sink *sink;
    oid temp_oid[MAX_OID_LEN];

		/*
		 * Initialise SNMPv2 required variables
		 */
    //James: gettimeofday(&now, NULL);
    clock_gettime(CLOCK_MONOTONIC, &now_n);
    now.tv_sec = now_n.tv_sec;
    now.tv_usec = now_n.tv_nsec / 1000;
    uptime = calculate_time_diff(&now, &starttime);
    memset (&uptime_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &uptime_var, sysuptime_oid, OID_LENGTH(sysuptime_oid));
    snmp_set_var_value( &uptime_var, (u_char *)&uptime, sizeof(uptime) );
    uptime_var.type           = ASN_TIMETICKS;
    uptime_var.next_variable  = &snmptrap_var;

    memset (&snmptrap_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &snmptrap_var, snmptrap_oid, OID_LENGTH(snmptrap_oid));
	/* value set later .... */
    snmptrap_var.type           = ASN_OBJECT_ID;
    if ( vars )
	snmptrap_var.next_variable  = vars;
    else
	snmptrap_var.next_variable  = &enterprise_var;

			/* find end of provided varbind list,
			   ready to append the enterprise info if necessary */
    last_var = vars;
    while ( last_var && last_var->next_variable )
	last_var = last_var->next_variable;

    memset (&enterprise_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &enterprise_var,
		 snmptrapenterprise_oid, OID_LENGTH(snmptrapenterprise_oid));
    snmp_set_var_value( &enterprise_var, (u_char *)enterprise, enterprise_length*sizeof(oid));
    enterprise_var.type           = ASN_OBJECT_ID;
    enterprise_var.next_variable  = NULL;

    v2_vars = &uptime_var;

		/*
		 *  Create a template PDU, ready for sending
		 */
    template_pdu = snmp_pdu_create( SNMP_MSG_TRAP );
    if ( template_pdu == NULL ) {
		/* Free memory if value stored dynamically */
	snmp_set_var_value( &enterprise_var, NULL, 0);
	return;
    }
    template_pdu->trap_type     = trap;
    template_pdu->specific_type = specific;
    if ( snmp_clone_mem((void **)&template_pdu->enterprise,
				enterprise, enterprise_length*sizeof(oid))) {
	snmp_free_pdu( template_pdu );
	snmp_set_var_value( &enterprise_var, NULL, 0);
	return;
    }
    template_pdu->enterprise_length = enterprise_length;
    template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;
    pduIp = (struct sockaddr_in *)&template_pdu->agent_addr;
    pduIp->sin_family		 = AF_INET;
    pduIp->sin_len               = sizeof(*pduIp);
    pduIp->sin_addr.s_addr	 = get_myaddr();

#ifdef CYGPKG_NET_FREEBSD_INET6
    pduIp_v6 = (struct sockaddr_in6 *)&template_pdu->agent_addr;
    pduIp_v6->sin6_family            = AF_INET6;
    pduIp_v6->sin6_len               = sizeof(*pduIp_v6);
    inet_pton(AF_INET6, current_ipv6_ip_addr, (void *)&pduIp_v6->sin6_addr);
#endif /* CYGPKG_NET_FREEBSD_INET6 */

    template_pdu->time		 	 = uptime;

		/*
		 *  Now use the parameters to determine
		 *    which v2 variables are needed,
		 *    and what values they should take.
		 */
    switch ( trap ) {
	case -1:	/*
			 *	SNMPv2 only
			 *  Check to see whether the variables provided
			 *    are sufficient for SNMPv2 notifications
			 */
		if (vars && snmp_oid_compare(vars->name, vars->name_length,
				sysuptime_oid, OID_LENGTH(sysuptime_oid)) == 0 )
			v2_vars = vars;
		else
		if (vars && snmp_oid_compare(vars->name, vars->name_length,
				snmptrap_oid, OID_LENGTH(snmptrap_oid)) == 0 )
			uptime_var.next_variable = vars;
		else {
			/* Hmmm... we don't seem to have a value - oops! */
			snmptrap_var.next_variable = vars;
		}
		last_var = NULL;	/* Don't need enterprise info */
		break;

			/* "Standard" SNMPv1 traps */

	case SNMP_TRAP_COLDSTART:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)cold_start_oid,
				    sizeof(cold_start_oid));
		break;
	case SNMP_TRAP_WARMSTART:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)warm_start_oid,
				    sizeof(warm_start_oid));
		break;
	case SNMP_TRAP_LINKDOWN:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)link_down_oid,
				    sizeof(link_down_oid));
		break;
	case SNMP_TRAP_LINKUP:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)link_up_oid,
				    sizeof(link_up_oid));
		break;
	case SNMP_TRAP_AUTHFAIL:
                if (snmp_enableauthentraps == SNMP_AUTHENTICATED_TRAPS_DISABLED) {
                    snmp_free_pdu( template_pdu );
		    snmp_set_var_value( &enterprise_var, NULL, 0);
		    return;
                }
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)auth_fail_oid,
				    sizeof(auth_fail_oid));
		break;
	case SNMP_TRAP_EGPNEIGHBORLOSS:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)egp_xxx_oid,
				    sizeof(egp_xxx_oid));
		break;

	case SNMP_TRAP_ENTERPRISESPECIFIC:
		memcpy( &temp_oid,
				    (char *)enterprise,
				    (enterprise_length)*sizeof(oid));
		temp_oid[ enterprise_length   ] = 0;
		temp_oid[ enterprise_length+1 ] = specific;
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)&temp_oid,
				    (enterprise_length+2)*sizeof(oid));
		snmptrap_var.next_variable  = vars;
		last_var = NULL;	/* Don't need version info */
		break;
    }


		/*
		 *  Now loop through the list of trap sinks,
		 *   sending an appropriately formatted PDU to each
		 */
    for ( sink = sinks ; sink ; sink=sink->next ) {
	if ( sink->version == SNMP_VERSION_1 && trap == -1 )
		continue;	/* Skip v1 sinks for v2 only traps */
	template_pdu->version = sink->version;
	template_pdu->command = sink->pdutype;
	if ( sink->version != SNMP_VERSION_1 ) {
	    template_pdu->variables = v2_vars;
	    if ( last_var )
		last_var->next_variable = &enterprise_var;
	}
	else
	    template_pdu->variables = vars;

    /* peter, 2007/12, send stuff to registered callbacks */
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (template_pdu->version == SNMP_VERSION_3) {
        if (template_pdu->securityEngineIDLen == 0) {
	        if (sink->sesp->securityEngineIDLen) {
	          snmpv3_clone_engineID(&template_pdu->securityEngineID,
	        			&template_pdu->securityEngineIDLen,
	        			sink->sesp->securityEngineID,
	        			sink->sesp->securityEngineIDLen);
	        }
        }
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_SEND_TRAP2,
                        template_pdu);
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

#ifdef CYGPKG_NET_FREEBSD_INET6
	/* James: fill agent ipaddress */
    if (sink->is_v6) {
	    get_myipv6addr(sink->is_link_local, current_ipv6_ip_addr);
	    inet_pton(AF_INET6, current_ipv6_ip_addr, (void *)&pduIp_v6->sin6_addr);
	}
#endif	
	
	pdu = snmp_clone_pdu( template_pdu );
	pdu->sessid = sink->sesp->sessid;	/* AgentX only ? */
	    
	if ( snmp_send( sink->sesp, pdu) == 0 ) {
            snmp_sess_perror ("snmpd: send_trap", sink->sesp);
	    snmp_free_pdu( pdu );
	}
	else {
	    snmp_increment_statistic(STAT_SNMPOUTTRAPS);
	    snmp_increment_statistic(STAT_SNMPOUTPKTS);
	}

	if ( sink->version != SNMP_VERSION_1 && last_var )
	    last_var->next_variable = NULL;
    }

		/* Free memory if values stored dynamically */
    snmp_set_var_value( &enterprise_var, NULL, 0);
    snmp_set_var_value( &snmptrap_var, NULL, 0);
	/* Ensure we don't free anything we shouldn't */
    if ( last_var )
	last_var->next_variable = NULL;
    template_pdu->variables = NULL;
    snmp_free_pdu( template_pdu );
}

void send_trap_vars (int trap,
		     int specific,
		     struct variable_list *vars)
{
    if ( trap == SNMP_TRAP_ENTERPRISESPECIFIC )
        send_enterprise_trap_vars( trap, specific, objid_enterprisetrap,
			OID_LENGTH(objid_enterprisetrap), vars );
    else
        send_enterprise_trap_vars( trap, specific, version_id,
			version_id_len/*OID_LENGTH(version_id)*/, vars );
}

void send_easy_trap (int trap,
		     int specific)
{
    send_trap_vars( trap, specific, NULL );
}

void send_v2trap ( struct variable_list *vars)
{
    send_trap_vars( -1, -1, vars );
}

void
send_trap_pdu(struct snmp_pdu *pdu)
{
    send_trap_vars( -1, -1, pdu->variables );
}



	/*******************
	 *
	 * Config file handling
	 *
	 *******************/

void snmpd_parse_config_authtrap(const char *token,
				 char *cptr)
{
    int i;

    i = atoi(cptr);
    if ( i == 0 ) {
	if ( !strcmp( cptr, "enable" ))
	    i = SNMP_AUTHENTICATED_TRAPS_ENABLED;
	else if ( !strcmp( cptr, "disable" ))
	    i = SNMP_AUTHENTICATED_TRAPS_DISABLED;
    }
    if (i < 1 || i > 2)
	config_perror("authtrapenable must be 1 or 2");
    else
	snmp_enableauthentraps = i;
}

void snmpd_parse_config_trapsink(const char *token,
				 char *cptr)
{
    char tmpbuf[1024];
    char *sp, *cp, *pp = NULL;
    u_short sinkport;

    if (!snmp_trapcommunity) snmp_trapcommunity = strdup("public");
    sp = strtok(cptr, " \t\n");
    cp = strtok(NULL, " \t\n");
    if (cp) pp = strtok(NULL, " \t\n");
    if (cp && pp) {
	sinkport = atoi(pp);
	if ((sinkport < 1) || (sinkport > 0xffff)) {
	    config_perror("trapsink port out of range");
	    sinkport = SNMP_TRAP_PORT;
	}
    } else {
	sinkport = SNMP_TRAP_PORT;
    }
    if (create_v1_trap_session(sp, sinkport,
			       cp ? cp : snmp_trapcommunity) == 0) {
	sprintf(tmpbuf,"cannot create trapsink: %s", cptr);
	config_perror(tmpbuf);
    }
}


void
snmpd_parse_config_trap2sink(const char *word, char *cptr)
{
    char tmpbuf[1024];
    char *sp, *cp, *pp = NULL;
    u_short sinkport;

    if (!snmp_trapcommunity) snmp_trapcommunity = strdup("public");
    sp = strtok(cptr, " \t\n");
    cp = strtok(NULL, " \t\n");
    if (cp) pp = strtok(NULL, " \t\n");
    if (cp && pp) {
	sinkport = atoi(pp);
	if ((sinkport < 1) || (sinkport > 0xffff)) {
	    config_perror("trapsink port out of range");
	    sinkport = SNMP_TRAP_PORT;
	}
    } else {
	sinkport = SNMP_TRAP_PORT;
    }
    if (create_v2_trap_session(sp, sinkport,
			       cp ? cp : snmp_trapcommunity) == 0) {
	sprintf(tmpbuf,"cannot create trap2sink: %s", cptr);
	config_perror(tmpbuf);
    }
}

void
snmpd_parse_config_informsink(const char *word, char *cptr)
{
    char tmpbuf[1024];
    char *sp, *cp, *pp = NULL;
    u_short sinkport;

    if (!snmp_trapcommunity) snmp_trapcommunity = strdup("public");
    sp = strtok(cptr, " \t\n");
    cp = strtok(NULL, " \t\n");
    if (cp) pp = strtok(NULL, " \t\n");
    if (cp && pp) {
	sinkport = atoi(pp);
	if ((sinkport < 1) || (sinkport > 0xffff)) {
	    config_perror("trapsink port out of range");
	    sinkport = SNMP_TRAP_PORT;
	}
    } else {
	sinkport = SNMP_TRAP_PORT;
    }
    if (create_v2_inform_session(sp, sinkport,
				 cp ? cp : snmp_trapcommunity) == 0) {
	sprintf(tmpbuf,"cannot create informsink: %s", cptr);
	config_perror(tmpbuf);
    }
}

void
snmpd_parse_config_trapcommunity(const char *word, char *cptr)
{
    if (snmp_trapcommunity) free(snmp_trapcommunity);
    snmp_trapcommunity = malloc (strlen(cptr)+1);
    copy_word(cptr, snmp_trapcommunity);
}

void snmpd_free_trapcommunity (void)
{
    if (snmp_trapcommunity) {
	free(snmp_trapcommunity);
	snmp_trapcommunity = NULL;
    }
}

/* peter, 2007/6, add new function for configure SNMP trap modlue */
void SnmpdEnableAuthenTraps(int is_enable)
{
    if (is_enable) {
        snmp_enableauthentraps = SNMP_AUTHENTICATED_TRAPS_ENABLED;
    } else {
        snmp_enableauthentraps = SNMP_AUTHENTICATED_TRAPS_DISABLED;
    }
}

void SnmpdSetVersionIdOid(oid *version_id_oid, int length)
{
    version_id_len = length;
    memcpy(version_id, version_id_oid, sizeof(oid) * length);
}

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
/* peter, 2007/12, add new function convenient to get trap destination engine ID */
int SnmpdGetRemoteEngineId(char *peername, u_char *engineID, unsigned long *engineID_len)
{
    struct trap_sink *sink;

    for (sink = sinks; sink; sink=sink->next) {
	    if (sink->version != SNMP_VERSION_3) {
		    continue;
        }
        if (sink->sesp->version == SNMP_VERSION_3 && (!strcmp(sink->sesp->peername, peername))) {
            if (sink->sesp->securityEngineIDLen > MAX_ENGINEID_LENGTH) {
                memset(engineID, 0x0, sizeof(engineID));
                *engineID_len = 0;
                return 1;
            }
            *engineID_len = sink->sesp->securityEngineIDLen;
            memcpy(engineID, sink->sesp->securityEngineID, sink->sesp->securityEngineIDLen);
            return 0;
        }
    }

    memset(engineID, 0x0, sizeof(engineID));
    *engineID_len = 0;
    return 1;
}
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */




#ifdef HAVE_MULTI_TRAP_DEST

#include "agent_trap.h"

ucd_trap_session_t sess[VTSS_UCD_TRAP_REQUEST_MAX];
static int    sess_next_idx = 0;
static int    sess_list_head, sess_list_tail;

#define UCD_SESS_FULL_ID 0xffff 
#define IS_UCD_SESS_FULL ( sess_next_idx == UCD_SESS_FULL_ID )

static void update_sess_next_idx(void)
{
    int i = 0;
    ucd_trap_session_t *sesp;

    for (i = sess_next_idx, sesp = &sess[sess_next_idx]; i < VTSS_UCD_TRAP_REQUEST_MAX; i++, sesp++) {
        if ( 0 == sesp->valid) {
            sess_next_idx = i;
            return;
        }
    }

    sess_next_idx = UCD_SESS_FULL_ID;
}


void ucd_trap_init (void)
{
    sess_next_idx = 0;
    sess_list_head = -1;
    sess_list_tail = -1;
    memset(sess, 0, sizeof(sess));

}

void append_sess_list(ucd_trap_session_t   *ucd_sesp) 
{
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    ucd_trap_session_t   *prev_sesp;

    prev_sesp = (sess_list_head < 0 ) ? NULL: &sess[sess_list_tail];

    if (prev_sesp) {
        prev_sesp->next_sess_id = sess_next_idx;
    } else {
        sess_list_head = sess_next_idx;
    }

    ucd_sesp->valid     = 1;
    ucd_sesp->prev_sess_id = sess_list_tail;
    ucd_sesp->next_sess_id = -1;

    sess_list_tail = sess_next_idx;

    update_sess_next_idx();

    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
}

void delete_sess_list(int session_id) 
{
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    ucd_trap_session_t   *ucd_sesp = &sess[session_id], *prev_sesp, *next_sesp;

    ucd_sesp->valid = 0;

    prev_sesp = (ucd_sesp->prev_sess_id < 0 ) ? NULL: &sess[ucd_sesp->prev_sess_id];
    next_sesp = (ucd_sesp->next_sess_id < 0 ) ? NULL: &sess[ucd_sesp->next_sess_id];

    if ( session_id == sess_list_head ) {
        sess_list_head = ucd_sesp->next_sess_id;
    } 

    if ( session_id == sess_list_tail ){
        sess_list_tail = ucd_sesp->prev_sess_id;
    }

    if (prev_sesp) {
        prev_sesp->next_sess_id = ucd_sesp->next_sess_id;
    }

    if (next_sesp) {
        next_sesp->prev_sess_id = ucd_sesp->prev_sess_id;
    }

    sess_next_idx = (session_id < sess_next_idx) ? session_id: sess_next_idx;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
}

/**
  * \brief Destroy trap session
  * \return
  *    < 0 indicate fail.\n
  *    = 0 indicates destroying successfully.
  */
static int destroy_session (int session_id)
{

    ucd_trap_session_t   *ucd_sesp = &sess[session_id];
    snmp_close(ucd_sesp->sesp);

    delete_sess_list(session_id);
    
    return 0;
}


int
inform_session_delete_cb(int op,
		 struct snmp_session *session,
		 int reqid,
		 struct snmp_pdu *pdu,
		 void *magic)
{
    void*   head = &sess[0];
    int     sess_id = ((int)magic - (int)head) / sizeof(ucd_trap_session_t);
    switch (op)
    {
        case DELETE_SESSION:
            destroy_session(sess_id);
            break;
        case TIMED_OUT:
            session->s_snmp_errno = SNMPERR_TIMEOUT;
            break;
        default:
            break;
    }
    return 1;
}



/**
  * \brief Create trap session
  * 
  * \return
  *    < 0 indicate fail.\n
  *    >= 0 indicates session ID in the specific trap Configuration.
  */
static int create_session (ucd_trap_conf_t *trap_conf, int noProbe)
{
    struct snmp_session	 session, *sesp;
    ucd_trap_session_t   *ucd_sesp = NULL;
    int sess_id = sess_next_idx;

//    if ( IS_UCD_SESS_FULL ) {
//        destroy_session(sess_list_head);
//    }
    memset (&session, 0, sizeof (struct snmp_session));
    session.peername = trap_conf->trap_dip_str;
    session.version = trap_conf->trap_version;
    if (trap_conf->trap_community[0] != 0) {
        session.community = (u_char *) trap_conf->trap_community;
        session.community_len = strlen (trap_conf->trap_community);
    }

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (trap_conf->trap_version == SNMP_VERSION_3) {
        if (!trap_conf->trap_probe_engineid && 1 == noProbe) {
            session.flags |= SNMP_FLAGS_DONT_PROBE;
        }
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

    /* peter, 2007/12, set retries for trap inform */
    if (trap_conf->trap_inform_mode == SNMP_MSG_INFORM) {
        session.timeout = trap_conf->trap_inform_timeout;
        session.retries = trap_conf->trap_inform_retries;
    }

    session.remote_port = trap_conf->trap_port;

    if (trap_conf->trap_version == SNMP_VERSION_3 && trap_conf->trap_probe_engineid && !noProbe) {
            usm_set_reportErrorOnUnknownID(0);
    }
    if ( 0 == trap_conf->ipv6_flag) {
        sesp = snmp_open (&session);
    } else {
        sesp = snmp_open_v6 (&session);
    }

    if ( NULL == sesp) {
        usm_set_reportErrorOnUnknownID(1);
        return -1;
    }

    if (trap_conf->trap_version == SNMP_VERSION_3 && trap_conf->trap_probe_engineid && !noProbe) {
            usm_set_reportErrorOnUnknownID(1);
            if(sesp->securityEngineIDLen != 0 ) {
                trap_conf->trap_engineid_len = sesp->securityEngineIDLen;
                memcpy(trap_conf->trap_engineid, sesp->securityEngineID, sesp->securityEngineIDLen);
            }

    }

    sesp->securityName = strdup((char*)trap_conf->trap_security_name);
    sesp->securityNameLen = strlen((char*)trap_conf->trap_security_name);

    ucd_sesp          = &sess[sess_next_idx];
    ucd_sesp->sesp    = sesp;
    ucd_sesp->conf_id = trap_conf->conf_id;
    if (trap_conf->trap_inform_mode == SNMP_MSG_INFORM) {
        sesp->callback = inform_session_delete_cb;
        sesp->callback_magic = &sess[sess_next_idx];
    }

    if ( IS_UCD_SESS_FULL ) {
        destroy_session(sess_list_head);
    }

    append_sess_list(ucd_sesp);

    return sess_id;
}






static void send_trap_sess( struct snmp_session	 *sesp, u_char ipv6_flag, int is_link_local, char* trap_sip_str, int pdutype, int trap,
		     int specific,
		     oid *enterprise, int enterprise_length,
		     struct variable_list *vars)
{
    struct variable_list uptime_var, snmptrap_var, enterprise_var;
    struct variable_list *v2_vars, *last_var=NULL;
    struct snmp_pdu	*template_pdu, *pdu;
    struct timeval	 now;
    struct timespec now_n;
    long uptime;
    struct sockaddr_in *pduIp;
#ifdef CYGPKG_NET_FREEBSD_INET6
    struct sockaddr_in6 *pduIp_v6 = NULL;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
//    oid temp_oid[MAX_OID_LEN];

		/*
		 * Initialise SNMPv2 required variables
		 */
    //James: gettimeofday(&now, NULL);
    clock_gettime(CLOCK_MONOTONIC, &now_n);
    now.tv_sec = now_n.tv_sec;
    now.tv_usec = now_n.tv_nsec / 1000;
    uptime = calculate_time_diff(&now, &starttime);
    memset (&uptime_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &uptime_var, sysuptime_oid, OID_LENGTH(sysuptime_oid));
    snmp_set_var_value( &uptime_var, (u_char *)&uptime, sizeof(uptime) );
    uptime_var.type           = ASN_TIMETICKS;
    uptime_var.next_variable  = &snmptrap_var;

    memset (&snmptrap_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &snmptrap_var, snmptrap_oid, OID_LENGTH(snmptrap_oid));
	/* value set later .... */
    snmptrap_var.type           = ASN_OBJECT_ID;
    if ( vars )
	snmptrap_var.next_variable  = vars;
    else
	snmptrap_var.next_variable  = &enterprise_var;

			/* find end of provided varbind list,
			   ready to append the enterprise info if necessary */
    last_var = vars;
    while ( last_var && last_var->next_variable )
	last_var = last_var->next_variable;

    memset (&enterprise_var, 0, sizeof (struct variable_list));
    snmp_set_var_objid( &enterprise_var,
		 snmptrapenterprise_oid, OID_LENGTH(snmptrapenterprise_oid));
    snmp_set_var_value( &enterprise_var, (u_char *)enterprise, enterprise_length*sizeof(oid));
    enterprise_var.type           = ASN_OBJECT_ID;
    enterprise_var.next_variable  = NULL;

    v2_vars = &uptime_var;

		/*
		 *  Create a template PDU, ready for sending
		 */
    template_pdu = snmp_pdu_create( pdutype );
    if ( template_pdu == NULL ) {
		/* Free memory if value stored dynamically */
	snmp_set_var_value( &enterprise_var, NULL, 0);
	return;
    }
    template_pdu->trap_type     = trap;
    template_pdu->specific_type = specific;
    if ( snmp_clone_mem((void **)&template_pdu->enterprise,
				enterprise, enterprise_length*sizeof(oid))) {
	snmp_free_pdu( template_pdu );
	snmp_set_var_value( &enterprise_var, NULL, 0);
	return;
    }
    template_pdu->enterprise_length = enterprise_length;
    template_pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;
    pduIp = (struct sockaddr_in *)&template_pdu->agent_addr;
    pduIp->sin_family		 = AF_INET;
    pduIp->sin_len               = sizeof(*pduIp);
    if ( trap_sip_str[0] == 0 ) {
        pduIp->sin_addr.s_addr	 = get_myaddr();
    } else {
        inet_pton(AF_INET, trap_sip_str, (void*)&pduIp->sin_addr.s_addr);
    }

#ifdef CYGPKG_NET_FREEBSD_INET6
    if ( 1 == ipv6_flag) {
        pduIp_v6 = (struct sockaddr_in6 *)&template_pdu->agent_addr;
        pduIp_v6->sin6_family            = AF_INET6;
        pduIp_v6->sin6_len               = sizeof(*pduIp_v6);
        inet_pton(AF_INET6, current_ipv6_ip_addr, (void *)&pduIp_v6->sin6_addr);
    }
#endif /* CYGPKG_NET_FREEBSD_INET6 */

    template_pdu->time		 	 = uptime;

		/*
		 *  Now use the parameters to determine
		 *    which v2 variables are needed,
		 *    and what values they should take.
		 */
    switch ( trap ) {
	case -1:	/*
			 *	SNMPv2 only
			 *  Check to see whether the variables provided
			 *    are sufficient for SNMPv2 notifications
			 */
		if (vars && snmp_oid_compare(vars->name, vars->name_length,
				sysuptime_oid, OID_LENGTH(sysuptime_oid)) == 0 )
			v2_vars = vars;
		else
		if (vars && snmp_oid_compare(vars->name, vars->name_length,
				snmptrap_oid, OID_LENGTH(snmptrap_oid)) == 0 )
			uptime_var.next_variable = vars;
		else {
			/* Hmmm... we don't seem to have a value - oops! */
			snmptrap_var.next_variable = vars;
		}
		last_var = NULL;	/* Don't need enterprise info */
		break;

			/* "Standard" SNMPv1 traps */

	case SNMP_TRAP_COLDSTART:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)cold_start_oid,
				    sizeof(cold_start_oid));
		break;
	case SNMP_TRAP_WARMSTART:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)warm_start_oid,
				    sizeof(warm_start_oid));
		break;
	case SNMP_TRAP_LINKDOWN:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)link_down_oid,
				    sizeof(link_down_oid));
		break;
	case SNMP_TRAP_LINKUP:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)link_up_oid,
				    sizeof(link_up_oid));
		break;
	case SNMP_TRAP_AUTHFAIL:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)auth_fail_oid,
				    sizeof(auth_fail_oid));
		break;
	case SNMP_TRAP_EGPNEIGHBORLOSS:
		snmp_set_var_value( &snmptrap_var,
				    (u_char *)egp_xxx_oid,
				    sizeof(egp_xxx_oid));
		break;

	case SNMP_TRAP_ENTERPRISESPECIFIC:
//		memcpy( &temp_oid,
//				    (char *)enterprise,
//				    (enterprise_length)*sizeof(oid));
//		temp_oid[ enterprise_length   ] = 0;
//		temp_oid[ enterprise_length+1 ] = specific;
//		snmp_set_var_value( &snmptrap_var,
//				    (u_char *)&temp_oid,
//				    (enterprise_length+2)*sizeof(oid));
//		snmptrap_var.next_variable  = vars;
                uptime_var.next_variable = vars;
		last_var = NULL;	/* Don't need version info */
		break;
    }


		/*
		 *  Now loop through the list of trap sinks,
		 *   sending an appropriately formatted PDU to each
		 */
	template_pdu->version = sesp->version;
	if ( sesp->version != SNMP_VERSION_1 ) {
	    template_pdu->variables = v2_vars;
	    if ( last_var )
		last_var->next_variable = &enterprise_var;
	}
	else
	    template_pdu->variables = vars;

    /* peter, 2007/12, send stuff to registered callbacks */
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (template_pdu->version == SNMP_VERSION_3) {
        if (template_pdu->securityEngineIDLen == 0) {
	        if (sesp->securityEngineIDLen) {
	          snmpv3_clone_engineID(&template_pdu->securityEngineID,
	        			&template_pdu->securityEngineIDLen,
	        			sesp->securityEngineID,
	        			sesp->securityEngineIDLen);
	        }
        }
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_SEND_TRAP2,
                        template_pdu);
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

#ifdef CYGPKG_NET_FREEBSD_INET6
    /* TODO */
	/* James: fill agent ipaddress */
    if ( 1 == ipv6_flag ) {
	    get_myipv6addr(is_link_local, current_ipv6_ip_addr);
	    inet_pton(AF_INET6, current_ipv6_ip_addr, (void *)&pduIp_v6->sin6_addr);
	}
#endif	
	
	pdu = snmp_clone_pdu( template_pdu );
	pdu->sessid = sesp->sessid;	/* AgentX only ? */
	    

	if ( snmp_async_send( sesp, pdu, NULL, NULL) == 0) {
            snmp_sess_perror ("snmpd: send_trap", sesp);
	    snmp_free_pdu( pdu );
	}
	else {
	    snmp_increment_statistic(STAT_SNMPOUTTRAPS);
	    snmp_increment_statistic(STAT_SNMPOUTPKTS);
	}

	if ( sesp->version != SNMP_VERSION_1 && last_var )
	    last_var->next_variable = NULL;

		/* Free memory if values stored dynamically */
    snmp_set_var_value( &enterprise_var, NULL, 0);
    snmp_set_var_value( &snmptrap_var, NULL, 0);
	/* Ensure we don't free anything we shouldn't */
    if ( last_var )
	last_var->next_variable = NULL;
    template_pdu->variables = NULL;
    snmp_free_pdu( template_pdu );
}

static void  send_session(int sess_id, ucd_trap_conf_t *trap_conf)
{
    int is_link_local = 0;
    struct snmp_session	 *sesp = sess[sess_id].sesp;
    int pdutype = (trap_conf->trap_version == SNMP_VERSION_1)? SNMP_MSG_TRAP:trap_conf->trap_inform_mode;

    if ( 1 == trap_conf->ipv6_flag ) {
        if (memcmp(&trap_conf->trap_dip_str[0], "fe80", 4) == 0) {
            is_link_local = 1;
        }
    }

    if ( trap_conf->generic == SNMP_TRAP_ENTERPRISESPECIFIC )
        send_trap_sess( sesp, trap_conf->ipv6_flag, is_link_local, trap_conf->trap_sip_str, pdutype, trap_conf->generic, 1, objid_enterprisetrap,
			OID_LENGTH(objid_enterprisetrap), trap_conf->vars );
    else
        send_trap_sess( sesp, trap_conf->ipv6_flag, is_link_local, trap_conf->trap_sip_str, pdutype, trap_conf->generic, 0, version_id,
			version_id_len/*OID_LENGTH(version_id)*/, trap_conf->vars );

}

/**
  * \brief Send v1/v2 trap message
  *
  * \param trap_conf      [IN]: The trap information for sending trap
instance with its current value.
  */
int ucd_send_trap_message (ucd_trap_conf_t *trap_conf)
{
    int sess_id;
    if ( (sess_id = create_session (trap_conf, 1)) < 0 ) {
        return -1;
    }

    send_session(sess_id, trap_conf);

    if ( trap_conf->trap_inform_mode != SNMP_MSG_INFORM ) {
        destroy_session (sess_id);
    }
    return 0;
}

/**
  * \brief Send SNMPv3 probe message
  *
  * \param trap_conf      [IN]: The trap information for sending probe trap
instance with its current value.
  */
int ucd_send_probe_message (ucd_trap_conf_t *trap_conf)
{
    int sess_id;
    if ( (sess_id = create_session (trap_conf, 0)) < 0 ) {
        return -1;
    }

    destroy_session (sess_id);
    return 0;
}

/**
  * \brief Destroy trap session based on trap configuration ID, that is, trap_conf ->conf_id in ucd_send_trap_message()
  *
  * \param conf_id      [IN]: Index of the trap configuration
  */
void ucd_destroy_session (u_long conf_id)
{
    int sess_id = sess_list_head;
    ucd_trap_session_t   *ucd_sesp;

    while (sess_id > 0) {
        ucd_sesp = &sess[sess_id];
        if ( ucd_sesp->conf_id == conf_id ) {
            destroy_session(sess_id);
        }
        sess_id = ucd_sesp->next_sess_id;
    }
}
#endif /* HAVE_MULTI_TRAP_DEST */

