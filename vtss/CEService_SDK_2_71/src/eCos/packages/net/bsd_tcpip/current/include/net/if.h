//==========================================================================
//
//      include/net/if.h
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/net/if.h,v 1.58.2.2 2001/07/24 19:10:18 brooks Exp $
 */

#ifndef _NET_IF_H_
#define	_NET_IF_H_

#include <sys/queue.h>

/*
 * <net/if.h> does not depend on <sys/time.h> on most other systems.  This
 * helps userland compatability.  (struct timeval ifi_lastchange)
 */
#ifndef __ECOS
#ifndef _KERNEL
#include <sys/time.h>
#endif
#else
#include <sys/time.h>
#endif

struct ifnet;

/*
 * Length of interface external name, including terminating '\0'.
 * Note: this is the same size as a generic device's external name.
 */
#define		IFNAMSIZ	16
#define		IF_NAMESIZE	IFNAMSIZ

/*
 * Structure describing a `cloning' interface.
 */
struct if_clone {
	LIST_ENTRY(if_clone) ifc_list;	/* on list of cloners */
	const char *ifc_name;		/* name of device, e.g. `gif' */
	size_t ifc_namelen;		/* length of name */

	int	(*ifc_create)(struct if_clone *, int *);
	void	(*ifc_destroy)(struct ifnet *);
};

#define IF_CLONE_INITIALIZER(name, create, destroy)			\
	{ { 0 }, name, sizeof(name) - 1, create, destroy }

/*
 * Structure used to query names of interface cloners.
 */

struct if_clonereq {
	int	ifcr_total;		/* total cloners (out) */
	int	ifcr_count;		/* room for this many in user buffer */
	char	*ifcr_buffer;		/* buffer for cloner names */
};

#if 1 /* etliang */
#if (defined(INET) || defined(INET6))
#define VTSS_RFC4293_CNTR   1
#else
#define VTSS_RFC4293_CNTR   0
#endif /* defined(INET) || defined(INET6) */

#ifndef	VTSS_IP4_IMPL
#define VTSS_IP4_IMPL       1
#endif /* VTSS_IP4_IMPL */
#ifndef	VTSS_IP6_IMPL
#define VTSS_IP6_IMPL       1
#endif /* VTSS_IP6_IMPL */

#ifndef	VTSS_IPS_IMPL
#define VTSS_IPS_IMPL       1
#endif /* VTSS_IPS_IMPL */
#endif /* etliang */

#if VTSS_RFC4293_CNTR
struct if_rfc4293_stats {
/*
RFC-4293
IP Statistics Tables (ip.31->ipTrafficStats)
    ipIfStatsTable (ipTrafficStats.3):Per-Interface IP statistics
        ipIfStatsEntry (ipIfStatsTable.1)->INDEX { ipIfStatsIPVersion, ipIfStatsIfIndex }

ipIfStatsIPVersion          InetVersion,
ipIfStatsIfIndex            InterfaceIndex,

ipIfStatsInReceives         Counter32,
ipIfStatsHCInReceives       Counter64,
ipIfStatsInOctets           Counter32,
ipIfStatsHCInOctets         Counter64,
ipIfStatsInHdrErrors        Counter32,
ipIfStatsInNoRoutes         Counter32,
ipIfStatsInAddrErrors       Counter32,
ipIfStatsInUnknownProtos    Counter32,
ipIfStatsInTruncatedPkts    Counter32,
ipIfStatsInForwDatagrams    Counter32,
ipIfStatsHCInForwDatagrams  Counter64,
ipIfStatsReasmReqds         Counter32,
ipIfStatsReasmOKs           Counter32,
ipIfStatsReasmFails         Counter32,
ipIfStatsInDiscards         Counter32,
ipIfStatsInDelivers         Counter32,
ipIfStatsHCInDelivers       Counter64,
ipIfStatsOutRequests        Counter32,
ipIfStatsHCOutRequests      Counter64,
ipIfStatsOutForwDatagrams   Counter32,
ipIfStatsHCOutForwDatagrams Counter64,
ipIfStatsOutDiscards        Counter32,
ipIfStatsOutFragReqds       Counter32,
ipIfStatsOutFragOKs         Counter32,
ipIfStatsOutFragFails       Counter32,
ipIfStatsOutFragCreates     Counter32,
ipIfStatsOutTransmits       Counter32,
ipIfStatsHCOutTransmits     Counter64,
ipIfStatsOutOctets          Counter32,
ipIfStatsHCOutOctets        Counter64,
ipIfStatsInMcastPkts        Counter32,
ipIfStatsHCInMcastPkts      Counter64,
ipIfStatsInMcastOctets      Counter32,
ipIfStatsHCInMcastOctets    Counter64,
ipIfStatsOutMcastPkts       Counter32,
ipIfStatsHCOutMcastPkts     Counter64,
ipIfStatsOutMcastOctets     Counter32,
ipIfStatsHCOutMcastOctets   Counter64,
ipIfStatsInBcastPkts        Counter32,
ipIfStatsHCInBcastPkts      Counter64,
ipIfStatsOutBcastPkts       Counter32,
ipIfStatsHCOutBcastPkts     Counter64,

ipIfStatsDiscontinuityTime  TimeStamp,
ipIfStatsRefreshRate        Unsigned32
*/
    u_int           ifi_collisions;         /* collisions on csma interfaces */
    u_int           ifi_ierrors;            /* input errors on interface */
    u_int           ifi_oerrors;            /* output errors on interface */

    /* ipIfStatsInReceives/ipIfStatsHCInReceives*/
    u_quad_t        ifi_InReceives;         /* packets received on interface */
    /* ipIfStatsInOctets/ipIfStatsHCInOctets*/
    u_quad_t        ifi_InOctets;           /* total number of octets received */
    /* ipIfStatsInHdrErrors */
    u_int           ifi_InHdrErrors;        /* header content error on interface */
    /* ipIfStatsInNoRoutes */
    u_int           ifi_InNoRoutes;         /* discarded because no route could be found */
    /* ipIfStatsInAddrErrors */
    u_int           ifi_InAddrErrors;       /* addr in header content error on interface */
    /* ipIfStatsInUnknownProtos */
    u_int           ifi_InUnknownProtos;    /* destined for unsupported protocol */
    /* ipIfStatsInTruncatedPkts */
    u_int           ifi_InTruncatedPkts;    /* discarded because the datagram frame didn't carry enough data */
    /* ipIfStatsInForwDatagrams/ipIfStatsHCInForwDatagrams */
    u_quad_t        ifi_InForwDatagrams;    /* datagrams should be routed (DIP != OWN) */
    /* ipIfStatsReasmReqds */
    u_int           ifi_ReasmReqds;         /* fragments needed to be reassembled */
    /* ipIfStatsReasmOKs */
    u_int           ifi_ReasmOKs;           /* fragments that have been successfully reassembled */
    /* ipIfStatsReasmFails */
    u_int           ifi_ReasmFails;         /* failures detected by the IP re-assembly algorithm */
    /*  ipIfStatsInDiscards*/
    u_int           ifi_InDiscards;         /* dropped on input, this interface (e.g. lack of buffer) */
    /* ipIfStatsInDelivers/ipIfStatsHCInDelivers */
    u_quad_t        ifi_InDelivers;         /* successfully delivered to IP user-protocols (including ICMP) */
    /* ipIfStatsHCOutRequests/ipIfStatsOutRequests */
    u_quad_t        ifi_OutRequests;        /* local IP user-protocols (including ICMP) request xmit */
    /* ipIfStatsOutForwDatagrams/ipIfStatsHCOutForwDatagrams */
    u_quad_t        ifi_OutForwDatagrams;   /* datagrams successfully routed */
    /* ipIfStatsOutDiscards */
    u_int           ifi_OutDiscards;        /* no problem was encountered to xmit but were discarded (e.g. lack of buffer) */
    /* ipIfStatsOutFragReqds */
    u_int           ifi_OutFragReqds;       /* datagrams that would require fragmentation */
    /* ipIfStatsOutFragOKs */
    u_int           ifi_OutFragOKs;         /* datagrams that have been successfully fragmented */
    /* ipIfStatsOutFragFails */
    u_int           ifi_OutFragFails;       /* needed to be fragmented but could not be */
    /* ipIfStatsOutFragCreates */
    u_int           ifi_OutFragCreates;     /* datagram fragments that have been generated */
    /* ipIfStatsOutTransmits/ipIfStatsHCOutTransmits */
    u_quad_t        ifi_OutTransmits;       /* packets sent on interface */
    /* ipIfStatsOutOctets/ipIfStatsHCOutOctets */
    u_quad_t        ifi_OutOctets;          /* total number of octets sent */
    /* ipIfStatsInMcastPkts/ipIfStatsHCInMcastPkts */
    u_quad_t        ifi_InMcastPkts;        /* packets received via multicast */
    /* ipIfStatsInMcastOctets/ipIfStatsHCInMcastOctets */
    u_quad_t        ifi_InMcastOctets;      /* total number of octets received via multicast */
    /* ipIfStatsOutMcastPkts/ipIfStatsHCOutMcastPkts */
    u_quad_t        ifi_OutMcastPkts;       /* packets sent via multicast */
    /* ipIfStatsOutMcastOctets/ipIfStatsHCOutMcastOctets */
    u_quad_t        ifi_OutMcastOctets;     /* total number of octets sent via multicast */
    /* ipIfStatsInBcastPkts/ipIfStatsHCInBcastPkts */
    u_quad_t        ifi_InBcastPkts;        /* packets received via broadcast */
    /* ipIfStatsOutBcastPkts/ipIfStatsHCOutBcastPkts */
    u_quad_t        ifi_OutBcastPkts;       /* packets sent via broadcast */
    /* ipIfStatsDiscontinuityTime */
    struct timeval  ifi_DiscontinuityTime;  /* timestamp when counters suffered a discontinuity */
};
#endif /* VTSS_RFC4293_CNTR */

/*
 * Structure describing information about an interface
 * which may be of interest to management entities.
 */
struct if_data {
	/* generic interface information */
	u_char	ifi_type;		/* ethernet, tokenring, etc */
	u_char	ifi_physical;		/* e.g., AUI, Thinnet, 10base-T, etc */
	u_char	ifi_addrlen;		/* media address length */
	u_char	ifi_hdrlen;		/* media header length */
	u_char	ifi_recvquota;		/* polling quota for receive intrs */
	u_char	ifi_xmitquota;		/* polling quota for xmit intrs */
	u_long	ifi_mtu;		/* maximum transmission unit */
	u_long	ifi_metric;		/* routing metric (external only) */
	u_long	ifi_baudrate;		/* linespeed */
	/* volatile statistics */
#if VTSS_RFC4293_CNTR
#if defined(INET)
    struct  if_rfc4293_stats    statistics4;
#endif /* defined(INET) */
#if defined(INET6)
    struct  if_rfc4293_stats    statistics6;
#endif /* defined(INET6) */
#else
	u_long	ifi_ipackets;		/* packets received on interface */
	u_long	ifi_ierrors;		/* input errors on interface */
	u_long	ifi_opackets;		/* packets sent on interface */
	u_long	ifi_oerrors;		/* output errors on interface */
	u_long	ifi_collisions;		/* collisions on csma interfaces */
	u_long	ifi_ibytes;		/* total number of octets received */
	u_long	ifi_obytes;		/* total number of octets sent */
	u_long	ifi_imcasts;		/* packets received via multicast */
	u_long	ifi_omcasts;		/* packets sent via multicast */
	u_long	ifi_ibcasts;		/* peter, 2007/7, packets received via broadcast */
	u_long	ifi_obcasts;		/* peter, 2007/7, packets sent via broadcast */
	u_long	ifi_iqdrops;		/* dropped on input, this interface */
	u_long	ifi_noproto;		/* destined for unsupported protocol */
#endif /* VTSS_RFC4293_CNTR */
	u_long	ifi_hwassist;		/* HW offload capabilities */
	u_long	ifi_unused;		/* XXX was ifi_xmittiming */
	struct	timeval ifi_lastchange;	/* time of last administrative change */
};

#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_SMART	0x20		/* interface manages own routes */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define	IFF_OACTIVE	0x400		/* transmission in progress */
#define	IFF_SIMPLEX	0x800		/* can't hear own transmissions */
#define	IFF_LINK0	0x1000		/* per link layer defined bit */
#define	IFF_LINK1	0x2000		/* per link layer defined bit */
#define	IFF_LINK2	0x4000		/* per link layer defined bit */
#define	IFF_ALTPHYS	IFF_LINK2	/* use alternate physical connection */
#define	IFF_MULTICAST	0x8000		/* supports multicast */

/* flags set internally only: */
#define	IFF_CANTCHANGE \
	(IFF_BROADCAST|IFF_POINTOPOINT|IFF_RUNNING|IFF_OACTIVE|\
	    IFF_LOOPBACK|IFF_SIMPLEX|IFF_MULTICAST|IFF_ALLMULTI|IFF_SMART)

#define	IFQ_MAXLEN	320
#define	IFNET_SLOWHZ	1		/* granularity is 1 second */

/*
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket
 */
struct if_msghdr {
	u_short	ifm_msglen;	/* to skip over non-understood messages */
	u_char	ifm_version;	/* future binary compatability */
	u_char	ifm_type;	/* message type */
	int	ifm_addrs;	/* like rtm_addrs */
	int	ifm_flags;	/* value of if_flags */
	u_short	ifm_index;	/* index for associated ifp */
	struct	if_data ifm_data;/* statistics and other data about if */
};

/*
 * Message format for use in obtaining information about interface addresses
 * from getkerninfo and the routing socket
 */
struct ifa_msghdr {
	u_short	ifam_msglen;	/* to skip over non-understood messages */
	u_char	ifam_version;	/* future binary compatability */
	u_char	ifam_type;	/* message type */
	int	ifam_addrs;	/* like rtm_addrs */
	int	ifam_flags;	/* value of ifa_flags */
	u_short	ifam_index;	/* index for associated ifp */
	int	ifam_metric;	/* value of ifa_metric */
};

/*
 * Message format for use in obtaining information about multicast addresses
 * from the routing socket
 */
struct ifma_msghdr {
	u_short	ifmam_msglen;	/* to skip over non-understood messages */
	u_char	ifmam_version;	/* future binary compatability */
	u_char	ifmam_type;	/* message type */
	int	ifmam_addrs;	/* like rtm_addrs */
	int	ifmam_flags;	/* value of ifa_flags */
	u_short	ifmam_index;	/* index for associated ifp */
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
#define IFHWADDRLEN  6
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		struct	sockaddr ifru_hwaddr;
		short	ifru_flags[2];
		int	ifru_metric;
		int	ifru_mtu;
		int	ifru_phys;
		int	ifru_media;
		caddr_t	ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* MAC address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags[0]	/* flags */
#define	ifr_prevflags	ifr_ifru.ifru_flags[1]	/* flags */
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric */
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu */
#define ifr_phys	ifr_ifru.ifru_phys	/* physical wire */
#define ifr_media	ifr_ifru.ifru_media	/* physical media */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};

#define	_SIZEOF_ADDR_IFREQ(ifr) \
	((ifr).ifr_addr.sa_len > sizeof(struct sockaddr) ? \
	 (sizeof(struct ifreq) - sizeof(struct sockaddr) + \
	  (ifr).ifr_addr.sa_len) : sizeof(struct ifreq))

struct ifaliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr ifra_addr;
	struct	sockaddr ifra_broadaddr;
	struct	sockaddr ifra_mask;
};

struct ifmediareq {
	char	ifm_name[IFNAMSIZ];	/* if name, e.g. "en0" */
	int	ifm_current;		/* current media options */
	int	ifm_mask;		/* don't care mask */
	int	ifm_status;		/* media status */
	int	ifm_active;		/* active options */
	int	ifm_count;		/* # entries in ifm_ulist array */
	int	*ifm_ulist;		/* media words */
};

/* 
 * Structure used to retrieve aux status data from interfaces.
 * Kernel suppliers to this interface should respect the formatting
 * needed by ifconfig(8): each line starts with a TAB and ends with
 * a newline.  The canonical example to copy and paste is in if_tun.c.
 */

#define	IFSTATMAX	800		/* 10 lines of text */
struct ifstat {
	char	ifs_name[IFNAMSIZ];	/* if name, e.g. "en0" */
	char	ascii[IFSTATMAX + 1];
};

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};


/*
 * Structure for SIOC[AGD]LIFADDR
 */
struct if_laddrreq {
	char	iflr_name[IFNAMSIZ];
	u_int	flags;
#define	IFLR_PREFIX	0x8000  /* in: prefix given  out: kernel fills id */
	u_int	prefixlen;         /* in/out */
	struct	sockaddr_storage addr;   /* in/out */
	struct	sockaddr_storage dstaddr; /* out */
};

#ifdef _KERNEL
#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_IFADDR);
MALLOC_DECLARE(M_IFMADDR);
#endif
#endif

#ifndef _KERNEL
struct if_nameindex {
	u_int	if_index;	/* 1, 2, ... */
	char	*if_name;	/* null terminated name: "le0", ... */
};

__BEGIN_DECLS
u_int	 if_nametoindex __P((const char *));
char	*if_indextoname __P((u_int, char *));
struct	 if_nameindex *if_nameindex __P((void));
void	 if_freenameindex __P((struct if_nameindex *));
__END_DECLS
#endif

#ifdef _KERNEL
/* XXX - this should go away soon. */
#include <net/if_var.h>
#endif

#endif /* !_NET_IF_H_ */
