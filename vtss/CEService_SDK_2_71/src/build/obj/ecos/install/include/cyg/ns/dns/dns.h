#ifndef CYGONCE_NS_DNS_DNS_H
#define CYGONCE_NS_DNS_DNS_H
//=============================================================================
//
//      dns.h
//
//      DNS client code.
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   andrew.lunn
// Contributors:andrew.lunn, jskov
// Date:        2001-09-18
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <network.h>
#include <netinet/in.h>

#define VTSS_DNS_ENHANCEMENT

#ifndef _POSIX_SOURCE
/* Initialise the DNS client with the address of the server. The
   address should be a IPv4 or IPv6 numeric address */
__externC int cyg_dns_res_start(char * server);

/* Old interface which  is deprecated */
__externC int cyg_dns_res_init(struct in_addr *dns_server);

/* Functions to manipulate the domainname */
__externC int getdomainname(char *name, size_t len);
__externC int setdomainname(const char *name, size_t len);
#endif

// Host name / IP mapping
struct hostent {
    char    *h_name;        /* official name of host */
    char    **h_aliases;    /* alias list */
    int     h_addrtype;     /* host address type */
    int     h_length;       /* length of address */
    char    **h_addr_list;  /* list of addresses */
};
#define h_addr  h_addr_list[0]  /* for backward compatibility */

#ifdef VTSS_DNS_ENHANCEMENT
// Host name / IP mapping entry, it is used for DNS cache
struct host_entry {
    char                query_host_name[256]; /* query name of host */
    int                 length_of_qname;
    char                real_host_name[256];  /* real name of host */
    struct in_addr      ip_addr;              /* mapping IP address */
    unsigned long       ttl;                  /* TTL */
    int                 valid;                /* 1: valid entry, 0: invalid entry */
};
#endif //VTSS_DNS_ENHANCEMENT

__externC struct hostent *gethostbyname(const char *host);
__externC struct hostent *gethostbyaddr(const char *addr, int len, int type);

// Error reporting
__externC int h_errno;
__externC const char* hstrerror(int err);

#define DNS_SUCCESS  0
#define HOST_NOT_FOUND 1
#define TRY_AGAIN      2
#define NO_RECOVERY    3
#define NO_DATA        4

// Interface between the DNS client and getaddrinfo
__externC int 
cyg_dns_getaddrinfo(const char * hostname, 
                    struct sockaddr addrs[], int num,
                    int family, char **canon);
// Interface between the DNS client and getnameinfo
__externC int
cyg_dns_getnameinfo(const struct sockaddr * sa, char * host, size_t hostlen);

#ifdef VTSS_DNS_ENHANCEMENT
/* Validate a hostname is legal as defined in RFC 1035 */
__externC int
cyg_valid_hostname(const char *hostname);

/* used for aging host cache table */
__externC void
cyg_host_cache_tab_aging(int tick);

__externC int 
build_qname(unsigned char *ptr, const char *hostname);

__externC void 
cyg_get_dns_query_name(unsigned char *dns_header_ptr, unsigned char *current_ptr, unsigned char *name);

__externC unsigned long
cyg_get_current_dns_server(void);

__externC void
cyg_set_current_dns_server(unsigned long dns_server_ip);
#endif //VTSS_DNS_ENHANCEMENT
//-----------------------------------------------------------------------------
#endif // CYGONCE_NS_DNS_DNS_H
// End of dns.h
