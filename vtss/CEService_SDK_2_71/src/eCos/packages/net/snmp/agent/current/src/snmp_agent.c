//==========================================================================
//
//      ./agent/current/src/snmp_agent.c
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
 * snmp_agent.c
 *
 * Simple Network Management Protocol (RFC 1067).
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
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
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <errno.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#define SNMP_NEED_REQUEST_LIST
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp.h"
#include "mib.h"
#include "snmp_client.h"

#include "snmp_vars.h"
#include "snmpd.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include "var_struct.h"
#include "read_config.h"
#include "snmp_logging.h"
#include "snmp_debug.h"
#include "mib_module_config.h"

#include "default_store.h"
#include "ds_agent.h"
#include "snmp_agent.h"
#include "agent_trap.h"
#include "snmp_alarm.h"

static int snmp_vars_inc;

static struct agent_snmp_session *agent_session_list = NULL;


static void dump_var(oid *, size_t, int, void *, size_t);

static void dump_var (
    oid *var_name,
    size_t var_name_len,
    int statType,
    void *statP,
    size_t statLen)
{
    char buf [SPRINT_MAX_LEN];
    struct variable_list temp_var;

    temp_var.type = statType;
    temp_var.val.string = (u_char *)statP;
    temp_var.val_len = statLen;
    sprint_variable (buf, var_name, var_name_len, &temp_var);
    snmp_log(LOG_INFO, "    >> %s\n", buf);
}


int getNextSessID()
{
    static int SessionID = 0;

    return ++SessionID;
}

int
agent_check_and_process(int block) {
  int numfds;
  fd_set fdset;
  struct timeval timeout = { LONG_MAX, 0 }, *tvp = &timeout;
  int count;
  int fakeblock=0;
  
  numfds = 0;
  FD_ZERO(&fdset);
  snmp_select_info(&numfds, &fdset, tvp, &fakeblock);
  if (block != 0 && fakeblock != 0) {
    /*  There are no alarms registered, and the caller asked for blocking, so
	let select() block forever.  */

    tvp = NULL;
  } else if (block != 0 && fakeblock == 0) {
    /*  The caller asked for blocking, but there is an alarm due sooner than
	LONG_MAX seconds from now, so use the modified timeout returned by
	snmp_select_info as the timeout for select().  */

  } else if (block == 0) {
    /*  The caller does not want us to block at all.  */

    tvp->tv_sec  = 0;
    tvp->tv_usec = 0;
  }

  count = select(numfds, &fdset, 0, 0, tvp);

  if (count > 0) {
    /* packets found, process them */
    snmp_read(&fdset);
  } else switch(count) {
    case 0:
      snmp_timeout();
      break;
    case -1:
      if (errno != EINTR) {
        snmp_log_perror("select");
      }
      return -1;
    default:
      snmp_log(LOG_ERR, "select returned %d\n", count);
      return -1;
  }  /* endif -- count>0 */

  /*  Run requested alarms.  */
  run_alarms();

  return count;
}


/*
 * The session is created using the "traditional API" routine snmp_open()
 * so is linked into the global library Sessions list.  It also opens a
 * socket that listens for incoming requests.
 * 
 *   The agent runs in an infinite loop (in the 'receive()' routine),
 * which calls snmp_read() when such a request is received on this socket.
 * This routine then traverses the library 'Sessions' list to identify the
 * relevant session and eventually invokes '_sess_read'.
 *   This then processes the incoming packet, calling the pre_parse, parse,
 * post_parse and callback routines in turn.
 */

	/* Global access to the primary session structure for this agent.
		for Index Allocation use initially. */
struct snmp_session *main_session;

int
init_master_agent(int dest_port, 
                  int (*pre_parse) (struct snmp_session *, snmp_ipaddr),
                  int (*post_parse) (struct snmp_session *, struct snmp_pdu *,int))
{
    struct snmp_session sess, *session;
    char *cptr, *cptr2;
    char buf[SPRINT_MAX_LEN];
    int flags;

    if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE) != MASTER_AGENT )
	return 0; /* no error if ! MASTER_AGENT */

#ifdef USING_AGENTX_MASTER_MODULE
    if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_AGENTX_MASTER) == 1 )
        real_init_master();
#endif

    /* has something been specified before? */
    cptr = ds_get_string(DS_APPLICATION_ID, DS_AGENT_PORTS);
                      
    /* set the specification string up */
    if (cptr && dest_port)
        /* append to the older specification string */
        snprintf(buf, sizeof(buf), "%d,%s", dest_port, cptr);
    else if (cptr)
        snprintf(buf, sizeof(buf), "%s",cptr);
    else if (dest_port)
        sprintf(buf,"%d",dest_port);
    else
        sprintf(buf,"%d",SNMP_PORT);
    buf[ sizeof(buf)-1 ] = 0;

    DEBUGMSGTL(("snmpd_ports","final port spec: %s\n", buf));
    cptr = strtok(buf, ",");
    while(cptr) {
        /* XXX: surely, this creates memory leaks */
        /* specification format: [transport:]port[@interface/address],... */
        DEBUGMSGTL(("snmpd_open","installing master agent on port %s\n", cptr));

        flags = ds_get_int(DS_APPLICATION_ID, DS_AGENT_FLAGS);

        /* transport type */
        if ((cptr2 = strchr(cptr, ':'))) {
            if (strncasecmp(cptr,"tcp",3) == 0)
                flags |= SNMP_FLAGS_STREAM_SOCKET;
            else if (strncasecmp(cptr,"udp",3) == 0)
                flags &= ~SNMP_FLAGS_STREAM_SOCKET;
            else {
                snmp_log(LOG_ERR, "illegal port transport %s\n", buf);
                return 1;
            }
            cptr = cptr2+1;
        }

        /* null? */
        if (!cptr || !(*cptr)) {
            snmp_log(LOG_ERR, "improper port specification\n");
            return 1;
        }

        dest_port = strtol(cptr, &cptr2, 0);

        if (dest_port <= 0 || (*cptr2 && *cptr2 != '@')) {
            /* XXX: add getservbyname lookups */
            snmp_log(LOG_ERR, "improper port specification %s\n", cptr);
            return 1;
        }

        memset(&sess, 0, sizeof(sess));
        snmp_sess_init( &sess );
    
        sess.version = SNMP_DEFAULT_VERSION;
        /* XXX: add interface binding (ie, eth0) */
        if (cptr2 && *cptr2 == '@' && *(cptr2+1))
            sess.peername = strdup(cptr2+1);
        else
            sess.peername = SNMP_DEFAULT_PEERNAME;
        
        sess.community_len = SNMP_DEFAULT_COMMUNITY_LEN;
     
        sess.local_port = dest_port;
        sess.callback = handle_snmp_packet;
        sess.authenticator = NULL;
        sess.flags = flags;
        sess.isAuthoritative = SNMP_SESS_AUTHORITATIVE;
        session = snmp_open_ex( &sess, pre_parse, 0, post_parse, 0, 0 );

        if ( session == NULL ) {
            /* diagnose snmp_open errors with the input struct
               snmp_session pointer */
            snmp_sess_perror("init_master_agent", &sess);
            return 1;
        }
        if (!main_session)
            main_session = session;

        /* next */
        cptr = strtok(NULL, ",");
    }
    return 0;
}

#ifdef CYGPKG_NET_FREEBSD_INET6
int
init_master_agent_v6(int dest_port, 
                  int (*pre_parse) (struct snmp_session *, snmp_ipaddr),
                  int (*post_parse) (struct snmp_session *, struct snmp_pdu *,int))
{
    struct snmp_session sess, *session;

    char *cptr, *cptr2;
    char buf[SPRINT_MAX_LEN];
    int flags;

    if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE) != MASTER_AGENT )
	return 0; /* no error if ! MASTER_AGENT */

#ifdef USING_AGENTX_MASTER_MODULE
    if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_AGENTX_MASTER) == 1 )
        real_init_master();
#endif

    /* has something been specified before? */
    cptr = ds_get_string(DS_APPLICATION_ID, DS_AGENT_PORTS);
                      
    /* set the specification string up */
    if (cptr && dest_port)
        /* append to the older specification string */
        snprintf(buf, sizeof(buf), "%d,%s", dest_port, cptr);
    else if (cptr)
        snprintf(buf, sizeof(buf), "%s",cptr);
    else if (dest_port)
        sprintf(buf,"%d",dest_port);
    else
        sprintf(buf,"%d",SNMP_PORT);
    buf[ sizeof(buf)-1 ] = 0;

    DEBUGMSGTL(("snmpd_ports","final port spec: %s\n", buf));
    cptr = strtok(buf, ",");
    while(cptr) {
        /* XXX: surely, this creates memory leaks */
        /* specification format: [transport:]port[@interface/address],... */
        DEBUGMSGTL(("snmpd_open","installing master agent on port %s\n", cptr));

        flags = ds_get_int(DS_APPLICATION_ID, DS_AGENT_FLAGS);

        /* transport type */
        if ((cptr2 = strchr(cptr, ':'))) {
            if (strncasecmp(cptr,"tcp",3) == 0)
                flags |= SNMP_FLAGS_STREAM_SOCKET;
            else if (strncasecmp(cptr,"udp",3) == 0)
                flags &= ~SNMP_FLAGS_STREAM_SOCKET;
            else {
                snmp_log(LOG_ERR, "illegal port transport %s\n", buf);
                return 1;
            }
            cptr = cptr2+1;
        }

        /* null? */
        if (!cptr || !(*cptr)) {
            snmp_log(LOG_ERR, "improper port specification\n");
            return 1;
        }

        dest_port = strtol(cptr, &cptr2, 0);

        if (dest_port <= 0 || (*cptr2 && *cptr2 != '@')) {
            /* XXX: add getservbyname lookups */
            snmp_log(LOG_ERR, "improper port specification %s\n", cptr);
            return 1;
        }

        memset(&sess, 0, sizeof(sess));
        snmp_sess_init( &sess );
   
        sess.version = SNMP_DEFAULT_VERSION;
        /* XXX: add interface binding (ie, eth0) */
        if (cptr2 && *cptr2 == '@' && *(cptr2+1))
            sess.peername = strdup(cptr2+1);
        else
            sess.peername = SNMP_DEFAULT_PEERNAME;
        
        sess.community_len = SNMP_DEFAULT_COMMUNITY_LEN;
     
        sess.local_port = dest_port;
        sess.callback = handle_snmp_packet;
        sess.authenticator = NULL;
        sess.flags = flags;
        sess.isAuthoritative = SNMP_SESS_AUTHORITATIVE;
        session = snmp_open_ex_v6( &sess, pre_parse, 0, post_parse, 0, 0 );

        if ( session == NULL ) {
            /* diagnose snmp_open errors with the input struct
               snmp_session pointer */
            snmp_sess_perror("init_master_agent_v6", &sess);
            return 1;
        }
        if (!main_session)
            main_session = session;

        /* next */
        cptr = strtok(NULL, ",");
    }
    return 0;
}
#endif /* CYGPKG_NET_FREEBSD_INET6 */

struct agent_snmp_session  *
init_agent_snmp_session( struct snmp_session *session, struct snmp_pdu *pdu )
{
    struct agent_snmp_session  *asp;

    asp = (struct agent_snmp_session *) malloc( sizeof( struct agent_snmp_session ));
    if ( asp == NULL )
	return NULL;
    asp->session = session;
    asp->pdu      = snmp_clone_pdu(pdu);
    asp->orig_pdu = snmp_clone_pdu(pdu);
    asp->rw      = READ;
    asp->exact   = TRUE;
    asp->outstanding_requests = NULL;
    asp->next    = NULL;
    asp->mode    = RESERVE1;
    asp->status  = SNMP_ERR_NOERROR;
    asp->index   = 0;

    asp->start = asp->pdu->variables;
    asp->end   = asp->pdu->variables;
    if ( asp->end != NULL )
	while ( asp->end->next_variable != NULL )
	    asp->end = asp->end->next_variable;

    return asp;
}

void
free_agent_snmp_session(struct agent_snmp_session *asp)
{
    if (!asp)
	return;
    if (asp->orig_pdu)
	snmp_free_pdu(asp->orig_pdu);
    if (asp->pdu)
	snmp_free_pdu(asp->pdu);

    free(asp);
}

int
count_varbinds( struct snmp_pdu *pdu )
{
  int count = 0;
  struct variable_list *var_ptr;
  
  for ( var_ptr = pdu->variables ; var_ptr != NULL ;
		  	var_ptr = var_ptr->next_variable )
	count++;

  return count;
}

int
handle_snmp_packet(int operation, struct snmp_session *session, int reqid,
                   struct snmp_pdu *pdu, void *magic)
{
    struct agent_snmp_session  *asp;
    int status, allDone, i, error_index = 0, rc;
    struct variable_list *var_ptr, *var_ptr2;

    if ( operation != RECEIVED_MESSAGE) {
        return 1;
    }

    if ( magic == NULL ) {
	asp = init_agent_snmp_session( session, pdu );
	status = SNMP_ERR_NOERROR;
    }
    else {
	asp = (struct agent_snmp_session *)magic;
        status =   asp->status;
    }

    if (asp->outstanding_requests != NULL)
	return 1;

#if 0 /* peter, 2007/7, modified community check for EstaX-34 project */
    if (check_access(pdu) != 0) {
        /* access control setup is incorrect */
	    send_easy_trap(SNMP_TRAP_AUTHFAIL, 0);
        if (asp->pdu->version != SNMP_VERSION_1 && asp->pdu->version != SNMP_VERSION_2c) {
            asp->pdu->errstat = SNMP_ERR_AUTHORIZATIONERROR;
            asp->pdu->command = SNMP_MSG_RESPONSE;
            snmp_increment_statistic(STAT_SNMPOUTPKTS);
            if (! snmp_send( asp->session, asp->pdu ))
	            snmp_free_pdu(asp->pdu);
	        asp->pdu = NULL;
	        free_agent_snmp_session(asp);
            return 1;
        } else {
            /* drop the request */
            free_agent_snmp_session( asp );
            return 0;
        }
    }
#endif

    if ((rc = check_access(pdu)) != 0) {
        asp->pdu->errstat = SNMP_ERR_GENERR;
        if (rc & 0x1 || rc & 0x4 || rc & 0x8) { /* SNMP_ACCESS_TRAP_AUTH_FAIL, declared in vtss_snmp.c */
        /* peter, 2007/12, using new function to suit v1/v2c
            send_easy_trap(SNMP_TRAP_AUTHFAIL, 0); */
            long arg = 4;
            SnmpdSendTrap(SNMP_TRAP_AUTHFAIL, &arg);
            asp->pdu->errstat = SNMP_ERR_AUTHORIZATIONERROR;
        }
        if (rc & 0x2) { /* SNMP_ACCESS_BAD_VER, declared in vtss_snmp.c */
            snmp_increment_statistic(STAT_SNMPINBADVERSIONS);
        }
        if (rc & 0x4) { /* SNMP_ACCESS_BAD_COMMUNITY, declared in vtss_snmp.c */
            snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYNAMES);
            if (asp->pdu->version == SNMP_VERSION_1 || asp->pdu->version == SNMP_VERSION_2c) {
                /* RFC 1901: The 'authorizationError(16)' value defined for the
                   error-status component of an SNMPv2 PDU [4] is unused. */
                /* drop the request */
                free_agent_snmp_session( asp );
                return 0;
            }
        }
        if (rc & 0x8) { /* SNMP_ACCESS_BAD_COMMUNITY_USE, declared in vtss_snmp.c */
            snmp_increment_statistic(STAT_SNMPINBADCOMMUNITYUSES);
        }
        if (rc & 0x20) { /* SNMP_ACCESS_BAD_COMMAND, declared in vtss_snmp.c */
            snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
        }
        if (rc & 0x40) { /* SNMP_ACCESS_READ_ONLY, declared in vtss_snmp.c */
            snmp_increment_statistic(STAT_SNMPINREADONLYS);
            asp->pdu->errstat = SNMP_ERR_NOTWRITABLE;
        }
        asp->pdu->command = SNMP_MSG_RESPONSE;
        snmp_increment_statistic(STAT_SNMPOUTPKTS);
        if (! snmp_send( asp->session, asp->pdu ))
	        snmp_free_pdu(asp->pdu);
	    asp->pdu = NULL;
        free_agent_snmp_session( asp );
        return 1;
    }

    /* peter, 2007/7, add for statistic counter */
    switch (pdu->errstat) {
	    case SNMP_ERR_TOOBIG:
                snmp_increment_statistic(STAT_SNMPINTOOBIGS);
	        break;
	    case SNMP_ERR_NOSUCHNAME:
                snmp_increment_statistic(STAT_SNMPINNOSUCHNAMES);
	        break;
	    case SNMP_ERR_BADVALUE:
                snmp_increment_statistic(STAT_SNMPINBADVALUES);
	        break;
	    case SNMP_ERR_READONLY:
                snmp_increment_statistic(STAT_SNMPINREADONLYS);
	        break;
	    case SNMP_ERR_GENERR:
                snmp_increment_statistic(STAT_SNMPINGENERRS);
	        break;
	}

    switch (pdu->command) {
    case SNMP_MSG_GET:
	if ( asp->mode != RESERVE1 )
	    break;			/* Single pass */
        snmp_increment_statistic(STAT_SNMPINGETREQUESTS);
	status = handle_next_pass( asp );
	asp->mode = RESERVE2;
	break;

    case SNMP_MSG_GETNEXT:
	if ( asp->mode != RESERVE1 )
	    break;			/* Single pass */
        snmp_increment_statistic(STAT_SNMPINGETNEXTS);
	asp->exact   = FALSE;
	status = handle_next_pass( asp );
	asp->mode = RESERVE2;
	break;

    case SNMP_MSG_GETBULK:
	    /*
	     * GETBULKS require multiple passes. The first pass handles the
	     * explicitly requested varbinds, and subsequent passes append
	     * to the existing var_op_list.  Each pass (after the first)
	     * uses the results of the preceeding pass as the input list
	     * (delimited by the start & end pointers.
	     * Processing is terminated if all entries in a pass are
	     * EndOfMib, or the maximum number of repetitions are made.
	     */
	if ( asp->mode == RESERVE1 ) {
            snmp_increment_statistic(STAT_SNMPINGETREQUESTS);
	    asp->exact   = FALSE;
		    /*
		     * Limit max repetitions to something reasonable
		     *	XXX: We should figure out what will fit somehow...
		     */
        /* peter, 2009/11, reduce 100 to 40 for avoid timeout response. */
	    if ( asp->pdu->errindex > 40 /* 100 */ )
	        asp->pdu->errindex = 40 /* 100 */;

        /* Peter, 2009/11, Netsnmp 4.1.2 doesn't implement subtree cache.
           We limit the Repeaters to 20 that can avoid the response packet > 1500 */
        if ( asp->pdu->errstat > 20)
            asp->pdu->errstat = 20;

	    if ( asp->pdu->errindex < 0 )
	        asp->pdu->errindex = 0;
	    if ( asp->pdu->errstat < 0 )
	        asp->pdu->errstat = 0;
    
		    /*
		     * If max-repetitions is 0, we shouldn't
		     *   process the non-nonrepeaters at all
		     *   so set up 'asp->end' accordingly
		     */
	    if ( asp->pdu->errindex == 0 ) {
		if ( asp->pdu->errstat == 0 ) {
				/* Nothing to do at all */
		    snmp_free_varbind(asp->pdu->variables);
		    asp->pdu->variables=NULL;
		    asp->start=NULL;
		}
		else {
		    asp->end   = asp->pdu->variables;
		    i = asp->pdu->errstat;
		    while ( --i > 0 ) 
			if ( asp->end )
			    asp->end = asp->end->next_variable;
                    if ( asp->end ) {
                        snmp_free_varbind(asp->end->next_variable);
                        asp->end->next_variable = NULL;
                    }
		}
	    }

	    status = handle_next_pass( asp );	/* First pass */
	    asp->mode = RESERVE2;
	    if ( status != SNMP_ERR_NOERROR )
	        break;

	    while ( asp->pdu->errstat-- > 0 )	/* Skip non-repeaters */
	    {
	        if ( NULL != asp->start )	/* if there are variables ... */
		    asp->start = asp->start->next_variable;
	    }
	    asp->pdu->errindex--;           /* Handled first repetition */

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( NULL != asp->start )	/* if there are variables ... */
	while ( asp->pdu->errindex-- > 0 ) {	/* Process repeaters */
		/*
		 * Add new variable structures for the
		 * repeating elements, ready for the next pass.
		 * Also check that these are not all EndOfMib
		 */
	    allDone = TRUE;		/* Check for some content */
	    for ( var_ptr = asp->start;
		  var_ptr != asp->end->next_variable;
		  var_ptr = var_ptr->next_variable ) {
				/* XXX: we don't know the size of the next
					OID, so assume the maximum length */
		if ( var_ptr->type != SNMP_ENDOFMIBVIEW ) {
		    var_ptr2 = snmp_add_null_var(asp->pdu, var_ptr->name, MAX_OID_LEN);
		    for ( i=var_ptr->name_length ; i<MAX_OID_LEN ; i++)
			var_ptr2->name[i] = 0;
		    var_ptr2->name_length = var_ptr->name_length;

		    allDone = FALSE;
		}
	    }
	    if ( allDone )
		break;

	    asp->start = asp->end->next_variable;
	    while ( asp->end->next_variable != NULL )
		asp->end = asp->end->next_variable;
	    
	    status = handle_next_pass( asp );
	    if ( status != SNMP_ERR_NOERROR )
		break;
	    if ( asp->outstanding_requests != NULL )
		return 1;
	}
	break;

    case SNMP_MSG_SET:
    	    /*
	     * SETS require 3-4 passes through the var_op_list.  The first two
	     * passes verify that all types, lengths, and values are valid
	     * and may reserve resources and the third does the set and a
	     * fourth executes any actions.  Then the identical GET RESPONSE
	     * packet is returned.
	     * If either of the first two passes returns an error, another
	     * pass is made so that any reserved resources can be freed.
	     * If the third pass returns an error, another pass is made so that
	     * any changes can be reversed.
	     * If the fourth pass (or any of the error handling passes)
	     * return an error, we'd rather not know about it!
	     */
	if ( asp->mode == RESERVE1 ) {
            snmp_increment_statistic(STAT_SNMPINSETREQUESTS);
	    asp->rw      = WRITE;

	    status = handle_next_pass( asp );

            if ( status != SNMP_ERR_NOERROR ){
	        asp->mode = FREE;
                error_index = asp->index;
            }
	    else
	        asp->mode = RESERVE2;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == RESERVE2 ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR ){
	        asp->mode = FREE;
                error_index = asp->index;
            }
	    else
	        asp->mode = ACTION;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == ACTION ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR ){
	        asp->mode = UNDO;
                error_index = asp->index;
            }
	    else
	        asp->mode = COMMIT;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == COMMIT ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR ) {
		status    = SNMP_ERR_COMMITFAILED;
	        asp->mode = FINISHED_FAILURE;
                error_index = asp->index;
	    }
	    else
	        asp->mode = FINISHED_SUCCESS;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == UNDO ) {
            if (handle_next_pass( asp ) != SNMP_ERR_NOERROR ) {
		status = SNMP_ERR_UNDOFAILED;
                error_index = 0;
            }

	    asp->mode = FINISHED_FAILURE;
	}

	if ( asp->mode == FREE ) {
	    (void) handle_next_pass( asp );
	}
        asp->index = error_index;
	break;

    case SNMP_MSG_RESPONSE:
        snmp_increment_statistic(STAT_SNMPINGETRESPONSES);
	free_agent_snmp_session( asp );
	return 0;
    case SNMP_MSG_TRAP:
    case SNMP_MSG_TRAP2:
        snmp_increment_statistic(STAT_SNMPINTRAPS);
	free_agent_snmp_session( asp );
	return 0;
    default:
        snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
	free_agent_snmp_session( asp );
	return 0;
    }

    if ( asp->outstanding_requests != NULL ) {
	asp->status = status;
	asp->next = agent_session_list;
	agent_session_list = asp;
    }
    else {
		/*
		 * May need to "dumb down" a SET error status for a
		 *  v1 query.  See RFC2576 - section 4.3
		 */
	if (( asp->pdu                          ) &&
	    ( asp->pdu->command == SNMP_MSG_SET ) &&
	    ( asp->pdu->version == SNMP_VERSION_1 )) {
	    switch ( status ) {
		case SNMP_ERR_WRONGVALUE:
		case SNMP_ERR_WRONGENCODING:
		case SNMP_ERR_WRONGTYPE:
		case SNMP_ERR_WRONGLENGTH:
		case SNMP_ERR_INCONSISTENTVALUE:
			status = SNMP_ERR_BADVALUE;
			break;
		case SNMP_ERR_NOACCESS:
		case SNMP_ERR_NOTWRITABLE:
		case SNMP_ERR_NOCREATION:
		case SNMP_ERR_INCONSISTENTNAME:
		case SNMP_ERR_AUTHORIZATIONERROR:
			status = SNMP_ERR_NOSUCHNAME;
			break;
		case SNMP_ERR_RESOURCEUNAVAILABLE:
		case SNMP_ERR_COMMITFAILED:
		case SNMP_ERR_UNDOFAILED:
			status = SNMP_ERR_GENERR;
			break;
	    }
	}
		/*
		 * Similarly we may need to "dumb down" v2 exception
		 *  types to throw an error for a v1 query.
		 *  See RFC2576 - section 4.1.2.3
		 */
	if (( asp->pdu                          ) &&
	    ( asp->pdu->command != SNMP_MSG_SET ) &&
	    ( asp->pdu->version == SNMP_VERSION_1 )) {
		for ( var_ptr = asp->pdu->variables, i=1 ;
			var_ptr != NULL ;
			var_ptr = var_ptr->next_variable, i++ ) {
	    	    switch ( var_ptr->type ) {
			case SNMP_NOSUCHOBJECT:
			case SNMP_NOSUCHINSTANCE:
			case SNMP_ENDOFMIBVIEW:
			case ASN_COUNTER64:
				status = SNMP_ERR_NOSUCHNAME;
				asp->index=i;
				break;
		    }
	    }
	}

	    /*
             * Update the snmp error-count statistics
             *   XXX - should we include the V2 errors in this or not?
             */
#define INCLUDE_V2ERRORS_IN_V1STATS

	switch ( status ) {
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
	    case SNMP_ERR_WRONGVALUE:
	    case SNMP_ERR_WRONGENCODING:
	    case SNMP_ERR_WRONGTYPE:
	    case SNMP_ERR_WRONGLENGTH:
	    case SNMP_ERR_INCONSISTENTVALUE:
#endif
	    case SNMP_ERR_BADVALUE:
                snmp_increment_statistic(STAT_SNMPOUTBADVALUES);
	        break;
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
	    case SNMP_ERR_NOACCESS:
	    case SNMP_ERR_NOTWRITABLE:
	    case SNMP_ERR_NOCREATION:
	    case SNMP_ERR_INCONSISTENTNAME:
	    case SNMP_ERR_AUTHORIZATIONERROR:
#endif
	    case SNMP_ERR_NOSUCHNAME:
                snmp_increment_statistic(STAT_SNMPOUTNOSUCHNAMES);
	        break;
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
	    case SNMP_ERR_RESOURCEUNAVAILABLE:
	    case SNMP_ERR_COMMITFAILED:
	    case SNMP_ERR_UNDOFAILED:
#endif
	    case SNMP_ERR_GENERR:
                snmp_increment_statistic(STAT_SNMPOUTGENERRS);
	        break;

	    case SNMP_ERR_TOOBIG:
                snmp_increment_statistic(STAT_SNMPOUTTOOBIGS);
	        break;
	}

	if (( status == SNMP_ERR_NOERROR ) && ( asp->pdu )) {
	    snmp_increment_statistic_by(
		(asp->pdu->command == SNMP_MSG_SET ?
			STAT_SNMPINTOTALSETVARS : STAT_SNMPINTOTALREQVARS ),
	    	count_varbinds( asp->pdu ));
	}
	else {
		/*
		 * Use a copy of the original request
		 *   to report failures.
		 */
	    snmp_free_pdu( asp->pdu );
	    asp->pdu = asp->orig_pdu;
	    asp->orig_pdu = NULL;
	}
	if ( asp->pdu ) {
	    asp->pdu->command  = SNMP_MSG_RESPONSE;
	    asp->pdu->errstat  = status;
	    if (status == SNMP_ERR_NOERROR) {
		asp->pdu->errindex = 0;
	    } else {
		asp->pdu->errindex = asp->index;
	    }
            do {
                if (! snmp_send( asp->session, asp->pdu )) {
                    snmp_free_pdu(asp->pdu);
                    snmp_increment_statistic(STAT_SNMPOUTGENERRS);
                    break;
                }
                snmp_increment_statistic(STAT_SNMPOUTPKTS);
                snmp_increment_statistic(STAT_SNMPOUTGETRESPONSES);
            } while (0);
	    asp->pdu = NULL;
	    free_agent_snmp_session( asp );
	}
    }

    return 1;
}

int
handle_next_pass(struct agent_snmp_session  *asp)
{
    int status;
    struct request_list *req_p, *next_req;


        if ( asp->outstanding_requests != NULL )
	    return SNMP_ERR_NOERROR;
	status = handle_var_list( asp );
        if ( asp->outstanding_requests != NULL ) {
	    if ( status == SNMP_ERR_NOERROR ) {
		/* Send out any subagent requests */
		for ( req_p = asp->outstanding_requests ;
			req_p != NULL ; req_p = next_req ) {

		    next_req = req_p->next_request;
		    if ( snmp_async_send( req_p->session,  req_p->pdu,
				      req_p->callback, req_p->cb_data ) == 0) {
				/*
				 * Send failed - call callback to handle this
				 */
			(void)req_p->callback( SEND_FAILED,
					 req_p->session,
					 req_p->pdu->reqid,
					 req_p->pdu,
					 req_p->cb_data );
			return SNMP_ERR_GENERR;
		    }
		}
	    }
	    else {
	    	/* discard outstanding requests */
		for ( req_p = asp->outstanding_requests ;
			req_p != NULL ; req_p = next_req ) {
			
			next_req = req_p->next_request;
			if ( req_p->pdu ) {
			   snmp_free_pdu( req_p->pdu );
			   req_p->pdu = NULL;
			}
			if ( req_p->cb_data ) {
			   free( req_p->cb_data );
			   req_p->cb_data = NULL;
			}
			free( req_p );
		}
		asp->outstanding_requests = NULL;
	    }
	}
	return status;
}

	/*
	 *  Private structure to save the results of a getStatPtr call.
	 *  This data can then be used to avoid repeating this call on
	 *  subsequent SET handling passes.
	 */
struct saved_var_data {
    WriteMethod *write_method;
    u_char	*statP;
    u_char	statType;
    size_t	statLen;
    u_short	acl;
};


int
handle_var_list(struct agent_snmp_session  *asp)
{
    struct variable_list *varbind_ptr;
    int     status;
    int     saved_status = SNMP_ERR_NOERROR;
    int     saved_index = 0;
    int     count;

    count = 0;
    varbind_ptr = asp->start;

    while (1) {
        if ( !varbind_ptr ) {
	    break;
        }
	count++;
	asp->index = count;
	status = handle_one_var(asp, varbind_ptr);

	if ( status != SNMP_ERR_NOERROR ) {
	    if (asp->rw == WRITE ) {
	        saved_status = status;
	        saved_index  = count;
	    }
	    else
	        return status;
	}

	if ( varbind_ptr == asp->end )
	     break;
	varbind_ptr = varbind_ptr->next_variable;
	if ( asp->mode == RESERVE1 )
	    snmp_vars_inc++;
    }
    if (saved_status != SNMP_ERR_NOERROR ) {
       asp->index = saved_index;
    }
    return saved_status;
}

int
handle_one_var(struct agent_snmp_session  *asp, struct variable_list *varbind_ptr)
{
    u_char  statType;
    u_char *statP;
    size_t  statLen;
    u_short acl;
    struct saved_var_data *saved;
    WriteMethod *write_method;
    AddVarMethod *add_method;
    int	    noSuchObject = TRUE;
    int     view;
    oid	    save[MAX_OID_LEN];
    size_t  savelen = 0;
    
statp_loop:
	if ( asp->rw == WRITE && varbind_ptr->data != NULL ) {
		/*
		 * SET handling is "multi-pass", so restore
		 *  results from an earlier 'getStatPtr' call
		 *  to avoid repeating this processing.
		 */
	    saved = (struct saved_var_data *) varbind_ptr->data;
	    write_method = saved->write_method;
	    statP        = saved->statP;
	    statType     = saved->statType;
	    statLen      = saved->statLen;
	    acl          = saved->acl;
	}
	else {
		/*
		 *  For exact requests (i.e. GET/SET),
		 *   check whether this object is accessible
		 *   before actually doing any work
		 */
	    if ( asp->exact )
		view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
				   asp->pdu, varbind_ptr->type);
	    else
		view = 0;	/* Assume accessible */
	    
            memcpy(save, varbind_ptr->name,
			varbind_ptr->name_length*sizeof(oid));
            savelen = varbind_ptr->name_length;
	    if ( view == 0 )
	        statP = getStatPtr(  varbind_ptr->name,
			   &varbind_ptr->name_length,
			   &statType, &statLen, &acl,
			   asp->exact, &write_method, asp->pdu, &noSuchObject);
	    else {
        /* peter, 2007/12, using new function to suit v1/v2c
	    if (view != 5) send_easy_trap(SNMP_TRAP_AUTHFAIL, 0); */
		if (view != 5) { long arg = 4; SnmpdSendTrap(SNMP_TRAP_AUTHFAIL, &arg); }
		statP        = NULL;
		write_method = NULL;
	    }
	}
			   
		/*
		 * For a valid SET request, having no existing value
		 *  is (possibly) acceptable (and implies creation).
		 * In all other situations, this indicates failure.
		 */
	if (statP == NULL && (asp->rw != WRITE || write_method == NULL)) {
	        /*  Careful -- if the varbind was lengthy, it will have
		    allocated some memory.  */
	        snmp_set_var_value(varbind_ptr, NULL, 0);
		if ( asp->exact ) {
	            if ( noSuchObject == TRUE ){
		        statType = SNMP_NOSUCHOBJECT;
		    } else {
		        statType = SNMP_NOSUCHINSTANCE;
		    }
		} else {
	            statType = SNMP_ENDOFMIBVIEW;
		}
		if (asp->pdu->version == SNMP_VERSION_1) {
		    return SNMP_ERR_NOSUCHNAME;
		}
		else if (asp->rw == WRITE) {
            /* 
                Bugzilla#1228 - SET using a non-existent variable in 8021 PAE MIB returns improper error status.
                2.4.1.1   The purpose of this test is to verify that the agent returns the
                correct error-status when the test writes or intentionally miswrites variables. 

                This test issues SET using a non-existent variable.

                The expected outcome is for the agent to return notWriteable or noAccess if the
                non-existent variable is not in the appropriate MIB view.
             */
		    return
			( noSuchObject	? SNMP_ERR_NOTWRITABLE
					: SNMP_ERR_NOCREATION ) ; 					
		}
		else
		    varbind_ptr->type = statType;
	}
                /*
		 * Delegated variables should be added to the
                 *  relevant outgoing request
		 */
        else if ( IS_DELEGATED(statType)) {
                add_method = (AddVarMethod*)statP;
                return (*add_method)( asp, varbind_ptr );
        }
		/*
		 * GETNEXT/GETBULK should just skip inaccessible entries
		 */
	else if (!asp->exact &&
		 (view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
				   asp->pdu, varbind_ptr->type))) {

        /* peter, 2007/12, using new function to suit v1/v2c
	    if (view != 5) send_easy_trap(SNMP_TRAP_AUTHFAIL, 0); */
		if (view != 5) { long arg = 4; SnmpdSendTrap(SNMP_TRAP_AUTHFAIL, &arg); }
		goto statp_loop;
	}
#ifdef USING_AGENTX_PROTOCOL_MODULE
		/*
		 * AgentX GETNEXT/GETBULK requests need to take
		 *   account of the end-of-range value
		 *
		 * This doesn't really belong here, but it works!
		 */
	else if (!asp->exact && asp->pdu->version == AGENTX_VERSION_1 &&
		 snmp_oid_compare( varbind_ptr->name,
			  	   varbind_ptr->name_length,
				   varbind_ptr->val.objid,
				   varbind_ptr->val_len/sizeof(oid)) > 0 ) {
            	memcpy(varbind_ptr->name, save, savelen*sizeof(oid));
            	varbind_ptr->name_length = savelen;
		varbind_ptr->type = SNMP_ENDOFMIBVIEW;
	}
#endif
		/*
		 * Other access problems are permanent
		 *   (i.e. writing to non-writeable objects)
		 */
	else if ( asp->rw == WRITE && !((acl & 2) && write_method)) {
        /* peter, 2007/12, using new function to suit v1/v2c
	    send_easy_trap(SNMP_TRAP_AUTHFAIL, 0); */
        long arg = 4;
        SnmpdSendTrap(SNMP_TRAP_AUTHFAIL, &arg);
	    if (asp->pdu->version == SNMP_VERSION_1 ) {
		return SNMP_ERR_NOSUCHNAME;
	    }
	    else {
		return SNMP_ERR_NOTWRITABLE;
	    }
        }
		
	else {
		/*
		 *  Things appear to have worked (so far)
		 *  Dump the current value of this object
		 */
	    if (ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE) && statP)
	        dump_var(varbind_ptr->name, varbind_ptr->name_length,
				statType, statP, statLen);

		/*
		 *  FINALLY we can act on SET requests ....
		 */
	    if ( asp->rw == WRITE ) {
		    if ( varbind_ptr->data == NULL ) {
				/*
				 * Save the results from 'getStatPtr'
				 *  to avoid repeating this call
				 */
			saved = (struct saved_var_data *)malloc(sizeof(struct saved_var_data));
			if ( saved == NULL ) {
			    return SNMP_ERR_GENERR;
			}
	    		saved->write_method = write_method;
	    		saved->statP        = statP;
	    		saved->statType     = statType;
	    		saved->statLen      = statLen;
	    		saved->acl          = acl;
			varbind_ptr->data = (void *)saved;
		    }
				/*
				 * Call the object's write method
				 */
		    return (*write_method)(asp->mode,
                                               varbind_ptr->val.string,
                                               varbind_ptr->type,
                                               varbind_ptr->val_len, statP,
                                               varbind_ptr->name,
                                               varbind_ptr->name_length);
	    }
		/*
		 * ... or save the results from assorted GET requests
		 */
	    else {
		snmp_set_var_value(varbind_ptr, statP, statLen);
		varbind_ptr->type = statType;
	    }
	}

	return SNMP_ERR_NOERROR;
	
}
