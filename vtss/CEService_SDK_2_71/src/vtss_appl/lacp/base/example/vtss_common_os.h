/*

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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

/**
 * This file contains the macros for the all protocol modules
 * that must be adapted for the operating environment.
 *
 * This is the version for the switch_app environment.
 */

#ifndef _VTSS_COMMON_OS_H_
#define _VTSS_COMMON_OS_H_ 1

/* Standard include files */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <memory.h>                     /* for memcmp */
#include <netinet/in.h>                 /* for the ntohs/htons */
#include <endian.h>
#define __KERNEL__
#include <asm/unaligned.h>
#undef __KERNEL__

typedef unsigned short vtss_common_port_t; /* Port numbers counted from 1 to VTSS_XXX_MAX_PORTS */

typedef unsigned short vtss_common_vlanid_t; /* VLAN numbers counted from 1 to VTSS_XXX_MAX_VLANS */

typedef unsigned char vtss_common_octet_t;

typedef unsigned long vtss_common_counter_t; /* Statistical counters */

typedef unsigned int vtss_common_framelen_t;

typedef unsigned char vtss_common_bool_t;
#define VTSS_COMMON_BOOL_FALSE          ((vtss_common_bool_t)0)
#define VTSS_COMMON_BOOL_TRUE           ((vtss_common_bool_t)1)

typedef unsigned long vtss_common_linkspeed_t; /* Link speed in Kbps */

typedef unsigned char vtss_common_linkstate_t; /* Port link state - down or up */
#define VTSS_COMMON_LINKSTATE_DOWN      ((vtss_common_linkstate_t)0)
#define VTSS_COMMON_LINKSTATE_UP        ((vtss_common_linkstate_t)1)

typedef unsigned char vtss_common_duplex_t;
#define VTSS_COMMON_LINKDUPLEX_HALF     ((vtss_common_duplex_t)0)
#define VTSS_COMMON_LINKDUPLEX_FULL     ((vtss_common_duplex_t)1)

/* Basically, the STP state is about receiving frames on a port */
typedef unsigned char vtss_common_stpstate_t; /* Port Spanning Tree state */
#define VTSS_COMMON_STPSTATE_DISABLED   ((vtss_common_stpstate_t)0)
#define VTSS_COMMON_STPSTATE_BLOCKING   ((vtss_common_stpstate_t)1)
#define VTSS_COMMON_STPSTATE_LEARNING   ((vtss_common_stpstate_t)2)
#define VTSS_COMMON_STPSTATE_FORWARDING ((vtss_common_stpstate_t)3)
#define VTSS_COMMON_STPSTATE_ENABLED    ((vtss_common_stpstate_t)4)

/* And FWD state is about transmitting frames out to a given port */
typedef unsigned char vtss_common_fwdstate_t; /* Port forwarding state */
#define VTSS_COMMON_FWDSTATE_DISABLED   ((vtss_common_fwdstate_t)0)
#define VTSS_COMMON_FWDSTATE_ENABLED    ((vtss_common_fwdstate_t)1)

#define VTSS_COMMON_MACADDR_SIZE 6
typedef struct {
    vtss_common_octet_t macaddr[VTSS_COMMON_MACADDR_SIZE];
} vtss_common_macaddr_t;

typedef unsigned short vtss_common_ethtype_t;

#define VTSS_COMMON_MACADDR_CMP(M1, M2) (memcmp((M1), (M2), VTSS_COMMON_MACADDR_SIZE))
#define VTSS_COMMON_MACADDR_ASSIGN(MDST, MSRC)  memcpy((MDST), (MSRC), VTSS_COMMON_MACADDR_SIZE)
#define HOST2NETS(S)			htons(S)
#define NET2HOSTS(S)			ntohs(S)
#define HOST2NETL(S)			htonl(S)
#define NET2HOSTL(S)			ntohl(S)

#define VTSS_COMMON_UNALIGNED_PUT_2B(DP, V)  put_unaligned(V, (unsigned short *)(DP))
#define VTSS_COMMON_UNALIGNED_GET_2B(DP)     get_unaligned((const unsigned short *)(DP))
#define VTSS_COMMON_UNALIGNED_PUT_4B(DP, V)  put_unaligned(V, (unsigned long *)(DP))
#define VTSS_COMMON_UNALIGNED_GET_4B(DP)     get_unaligned((const unsigned long *)(DP))

#define UNAL_NET2HOSTS(SP) NET2HOSTS(VTSS_COMMON_UNALIGNED_GET_2B(SP))
#define UNAL_HOST2NETS(SP) HOST2NETS(VTSS_COMMON_UNALIGNED_GET_2B(SP))
#define UNAL_NET2HOSTL(SP) NET2HOSTS(VTSS_COMMON_UNALIGNED_GET_4B(SP))
#define UNAL_HOST2NETL(SP) HOST2NETS(VTSS_COMMON_UNALIGNED_GET_4B(SP))

#ifdef NDEBUG
#define VTSS_COMMON_NDEBUG 1
#else
extern void vtss_common_savetrace(int lvl, const char *funcname, int lineno);
extern void vtss_common_trace(const char *fmt, ...)
#ifdef __GNUC__
__attribute__ ((format(printf,1,2)))
#endif
;
#endif /* !NDEBUG */
#define vtss_printf     printf

#ifdef __GNUC__
#define VTSS_COMMON_RETURN_ADDRESS(LVL) __builtin_return_address(LVL)
#else
#define VTSS_COMMON_RETURN_ADDRESS(LVL) 0 /* Compiler/OS/CPU specific */
#endif /* __GNUC__ */

extern const vtss_common_macaddr_t vtss_common_zeromac;

#define VTSS_COMMON_ZEROMAC             (vtss_common_zeromac.macaddr)

#define VTSS_COMMON_BUFMEM_ATTRIB       /* Attribute for network buffer memmory */
#define VTSS_COMMON_DATA_ATTRIB         /* Attribute for general data */
#define VTSS_COMMON_PTR_ATTRIB          /* Attribute for misc data pointers */
#define VTSS_COMMON_CODE_ATTRIB         /* Attribute for code pointers */

typedef void *vtss_common_bufref_t;

extern const char *vtss_common_str_macaddr(const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *mac);
extern const char *vtss_common_str_linkstate(vtss_common_linkstate_t state);
extern const char *vtss_common_str_linkduplex(vtss_common_duplex_t duplex);
extern const char *vtss_common_str_stpstate(vtss_common_stpstate_t stpstate);

/**
 * vtss_os_get_linkspeed - Deliver the current link speed (in Kbps) of a specific port.
 */
extern vtss_common_linkspeed_t vtss_os_get_linkspeed(vtss_common_port_t portno);

/**
 * vtss_os_get_linkstate - Deliver the current link state of a specific port.
 */
extern vtss_common_linkstate_t vtss_os_get_linkstate(vtss_common_port_t portno);

/**
 * vtss_os_get_linkduplex - Deliver the current link duplex mode of a specific port.
 */
extern vtss_common_duplex_t vtss_os_get_linkduplex(vtss_common_port_t portno);

/**
 * vtss_os_get_systemmac - Deliver the MAC address for the switch.
 */
extern void vtss_os_get_systemmac(vtss_common_macaddr_t *system_macaddr);

/**
 * vtss_os_get_portmac - Deliver the MAC address for the a specific port.
 */
extern void vtss_os_get_portmac(vtss_common_port_t portno, vtss_common_macaddr_t *port_macaddr);

/**
 * vtss_os_set_stpstate - Set the Spanning Tree state of a specific port.
 */
extern void vtss_os_set_stpstate(vtss_common_port_t portno, vtss_common_stpstate_t new_state);

/**
 * vtss_os_get_stpstate - Get the Spanning Tree state of a specific port.
 */
extern vtss_common_stpstate_t vtss_os_get_stpstate(vtss_common_port_t portno);

/**
 * vtss_os_set_fwdstate - Set the forwarding state of a specific port.
 */
extern void vtss_os_set_fwdstate(vtss_common_port_t portno, vtss_common_fwdstate_t new_state);

/**
 * vtss_os_get_fwdstate - Get the forwarding state of a specific port.
 */
extern vtss_common_fwdstate_t vtss_os_get_fwdstate(vtss_common_port_t portno);

/**
 * vtss_os_flush_fdb - Clear the filtering database for a specific port.
 */
void vtss_os_flush_fdb(vtss_common_bool_t flushport, vtss_common_port_t portno,
                       vtss_common_bool_t flushvlan, vtss_common_vlanid_t vlanid);

/**
 * vtss_os_alloc_xmit - Allocate a buffer to be used for transmitting a frame.
 */
extern void VTSS_COMMON_BUFMEM_ATTRIB *vtss_os_alloc_xmit(vtss_common_port_t portno, vtss_common_framelen_t len, vtss_common_bufref_t *pbufref);

/**
 * vtss_os_xmit - Rransmit a frame on a specific port.
 */
extern int vtss_os_xmit(vtss_common_port_t portno, void VTSS_COMMON_BUFMEM_ATTRIB *frame, vtss_common_framelen_t len, vtss_common_bufref_t bufref);

/* Return codes from the vtss_os_xmit() function */
#define VTSS_COMMON_CC_OK         0
#define VTSS_COMMON_CC_GENERR     1 /* General error */

#endif /* _VTSS_COMMON_OS_H__ */
