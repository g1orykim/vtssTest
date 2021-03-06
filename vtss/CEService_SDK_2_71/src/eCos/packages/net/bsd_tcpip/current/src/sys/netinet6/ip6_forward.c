//==========================================================================
//
//      src/sys/netinet6/ip6_forward.c
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

/*	$KAME: ip6_forward.c,v 1.87 2001/12/18 01:30:43 keiichi Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet6/in6_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet/icmp6.h>
#include <netinet6/nd6.h>

#if (defined(__FreeBSD__) && __FreeBSD__ >= 3) || defined(__OpenBSD__) || (defined(__bsdi__) && _BSDI_VERSION >= 199802)
#include <netinet/in_pcb.h>
#endif
#if !((defined(__FreeBSD__) && __FreeBSD__ >= 3) || defined(__OpenBSD__) || (defined(__bsdi__) && _BSDI_VERSION >= 199802))
#include <netinet6/in6_pcb.h>
#endif

#ifdef __OpenBSD__
#if NPF > 0
#include <net/pfvar.h>
#endif
#endif

#ifdef __OpenBSD__ /* KAME IPSEC */
#undef IPSEC
#endif

#ifdef IPSEC
#include <netinet6/ipsec.h>
#include <netkey/key.h>
#endif /* IPSEC */

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
#include <netinet6/ip6_fw.h>
#endif
#if defined(__NetBSD__) && defined(PFIL_HOOKS)
#include <net/pfil.h>
#endif

#ifdef MIP6
#include <netinet6/mip6.h>
extern struct mip6_bc_list mip6_bc_list;
#endif /* MIP6 */

//#include <net/net_osdep.h>

#ifdef NEW_STRUCT_ROUTE
struct	route ip6_forward_rt;
#else
struct	route_in6 ip6_forward_rt;
#endif

#ifdef MEASURE_PERFORMANCE
#define OURS_CHECK_ALG_RTABLE 0	/* XXX: duplicated def. */

extern int ip6_logentry;
extern int ip6_logsize;
extern unsigned long long ip6_performance_log[];
extern long long ip6_performance_log2[];
extern int ip6_ours_check_algorithm;

int ip6_forward_cache_miss;

static unsigned long long ctr_beg, ctr_end;

static __inline unsigned long long read_tsc __P((void));
static __inline void add_performance_log2 __P((unsigned long long)); 

/* XXX: duplicated code */
static __inline unsigned long long 
read_tsc(void)
{
     unsigned int h,l;
     /* read Pentium counter */
     __asm__(".byte 0x0f,0x31" :"=a" (l), "=d" (h));
     return ((unsigned long long)h<<32) | l;
}

static __inline void
add_performance_log2(val)
	unsigned long long val;
{
	ip6_performance_log[ip6_logentry] += val;
	ip6_performance_log2[ip6_logentry] = val;
}
#endif

/*
 * Forward a packet.  If some error occurs return the sender
 * an icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repertoire
 * of codes and types.
 *
 * If not forwarding, just drop the packet.  This could be confusing
 * if ipforwarding was zero but some routing protocol was advancing
 * us as a gateway to somewhere.  However, we must let the routing
 * protocol deal with that.
 *
 */

void
ip6_forward(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	struct ip6_hdr *ip6 = mtod(m, struct ip6_hdr *);
	struct sockaddr_in6 *dst;
	struct rtentry *rt;
	int error, type = 0, code = 0;
	struct mbuf *mcopy = NULL;
	struct ifnet *origifp;	/* maybe unnecessary */
	int64_t srczone, dstzone;
#ifdef IPSEC
	struct secpolicy *sp = NULL;
#endif
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
	long time_second = time.tv_sec;
#endif

    origifp = NULL;
    if (m) {
        origifp = m->m_pkthdr.rcvif;
    }

#ifdef IPSEC
	/*
	 * Check AH/ESP integrity.
	 */
	/*
	 * Don't increment ip6s_cantforward because this is the check
	 * before forwarding packet actually.
	 */
	if (ipsec6_in_reject(m, NULL)) {
		ipsec6stat.in_polvio++;
        IFSTAT_InDiscards_INC(-1, origifp, 1);
		m_freem(m);
		return;
	}
#endif /* IPSEC */

	/*
	 * Do not forward packets to multicast destination (should be handled
	 * by ip6_mforward().
	 * Do not forward packets with unspecified source.  It was discussed
	 * in July 2000, on ipngwg mailing list.
	 */
	if ((m->m_flags & (M_BCAST|M_MCAST)) != 0 ||
	    IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst) ||
	    IN6_IS_ADDR_UNSPECIFIED(&ip6->ip6_src)) {
		ip6stat.ip6s_cantforward++;
		/* XXX in6_ifstat_inc(rt->rt_ifp, ifs6_in_discard) */
        IFSTAT_InDiscards_INC(-1, origifp, 1);
        IFSTAT_InAddrErrors_INC(-1, origifp, 1);

		if (ip6_log_time + ip6_log_interval < time_second) {
			ip6_log_time = time_second;
			log(LOG_DEBUG,
			    "cannot forward "
			    "from %s to %s nxt %d received on %s\n",
			    ip6_sprintf(&ip6->ip6_src),
			    ip6_sprintf(&ip6->ip6_dst),
			    ip6->ip6_nxt,
			    if_name(m->m_pkthdr.rcvif));
		}
		m_freem(m);
		return;
	}

    IFSTAT_InForwDatagrams_INC(-1, origifp, 1);
	if (ip6->ip6_hlim <= IPV6_HLIMDEC) {
		/* XXX in6_ifstat_inc(rt->rt_ifp, ifs6_in_discard) */
        IFSTAT_InDiscards_INC(-1, origifp, 1);
		icmp6_error(m, ICMP6_TIME_EXCEEDED,
				ICMP6_TIME_EXCEED_TRANSIT, 0);
		return;
	}
	ip6->ip6_hlim -= IPV6_HLIMDEC;

	/*
	 * Save at most ICMPV6_PLD_MAXLEN (= the min IPv6 MTU -
	 * size of IPv6 + ICMPv6 headers) bytes of the packet in case
	 * we need to generate an ICMP6 message to the src.
	 * Thanks to M_EXT, in most cases copy will not occur.
	 *
	 * It is important to save it before IPsec processing as IPsec
	 * processing may modify the mbuf.
	 */
	mcopy = m_copy(m, 0, imin(m->m_pkthdr.len, ICMPV6_PLD_MAXLEN));

#ifdef IPSEC
	/* get a security policy for this packet */
	sp = ipsec6_getpolicybyaddr(m, IPSEC_DIR_OUTBOUND, IP_FORWARDING,
	    &error);
	if (sp == NULL) {
		ipsec6stat.out_inval++;
		ip6stat.ip6s_cantforward++;
        IFSTAT_InDiscards_INC(-1, origifp, 1);

		if (mcopy) {
#if 0
			/* XXX: what icmp ? */
#else
			m_freem(mcopy);
#endif
		}
		m_freem(m);
		return;
	}

	error = 0;

	/* check policy */
	switch (sp->policy) {
	case IPSEC_POLICY_DISCARD:
		/*
		 * This packet is just discarded.
		 */
		ipsec6stat.out_polvio++;
		ip6stat.ip6s_cantforward++;
        IFSTAT_InDiscards_INC(-1, origifp, 1);
		key_freesp(sp);
		if (mcopy) {
#if 0
			/* XXX: what icmp ? */
#else
			m_freem(mcopy);
#endif
		}
		m_freem(m);
		return;

	case IPSEC_POLICY_BYPASS:
	case IPSEC_POLICY_NONE:
		/* no need to do IPsec. */
		key_freesp(sp);
		goto skip_ipsec;

	case IPSEC_POLICY_IPSEC:
		if (sp->req == NULL) {
			/* XXX should be panic ? */
			printf("ip6_forward: No IPsec request specified.\n");
			ip6stat.ip6s_cantforward++;
            IFSTAT_InDiscards_INC(-1, origifp, 1);
			key_freesp(sp);
			if (mcopy) {
#if 0
				/* XXX: what icmp ? */
#else
				m_freem(mcopy);
#endif
			}
			m_freem(m);
			return;
		}
		/* do IPsec */
		break;

	case IPSEC_POLICY_ENTRUST:
	default:
		/* should be panic ?? */
		printf("ip6_forward: Invalid policy found. %d\n", sp->policy);
		key_freesp(sp);
		goto skip_ipsec;
	}

    {
	struct ipsec_output_state state;

	/*
	 * All the extension headers will become inaccessible
	 * (since they can be encrypted).
	 * Don't panic, we need no more updates to extension headers
	 * on inner IPv6 packet (since they are now encapsulated).
	 *
	 * IPv6 [ESP|AH] IPv6 [extension headers] payload
	 */
	bzero(&state, sizeof(state));
	state.m = m;
	state.ro = NULL;	/* update at ipsec6_output_tunnel() */
	state.dst = NULL;	/* update at ipsec6_output_tunnel() */

	error = ipsec6_output_tunnel(&state, sp, 0);

	m = state.m;
	key_freesp(sp);

	if (error) {
		/* mbuf is already reclaimed in ipsec6_output_tunnel. */
		switch (error) {
		case EHOSTUNREACH:
		case ENETUNREACH:
		case EMSGSIZE:
		case ENOBUFS:
		case ENOMEM:
			break;
		default:
			printf("ip6_output (ipsec): error code %d\n", error);
			/* fall through */
		case ENOENT:
			/* don't show these error codes to the user */
			break;
		}
		ip6stat.ip6s_cantforward++;
        switch ( error ) {
        case ENETUNREACH:
        case ENETDOWN:
        case EHOSTUNREACH:
        case EHOSTDOWN:
            IFSTAT_InNoRoutes_INC(-1, origifp, 1);
        default:
            IFSTAT_InDiscards_INC(-1, origifp, 1);
            break;
        }

		if (mcopy) {
#if 0
			/* XXX: what icmp ? */
#else
			m_freem(mcopy);
#endif
		}
		m_freem(m);
		return;
	}
    }
    skip_ipsec:
#endif /* IPSEC */

#ifdef MIP6
	{
		/*
		 * intercept and tunnel packets for home addresses
		 * which we are acting as a home agent for.
		 */
		struct mip6_bc *mbc;

		mbc = mip6_bc_list_find_withphaddr(&mip6_bc_list,
						   &ip6->ip6_dst);
		if (mbc &&
		    (mbc->mbc_flags & IP6_BUF_HOME) &&
		    (mbc->mbc_encap != NULL)) {
			/*
			 * if we have a binding cache entry for the
			 * ip6_dst, we are acting as a home agent for
			 * that node.  before sending a packet as a
			 * tunneled packet, we must make sure that
			 * encaptab is ready.  if dad is enabled and
			 * not completed yet, encaptab will be NULL.
			 */
			if (mip6_tunnel_output(&m, mbc) != 0) {
				ip6stat.ip6s_cantforward++;
                IFSTAT_InDiscards_INC(-1, origifp, 1);
			}
			if (mcopy)
				m_freem(mcopy);
			return;
		}
	}
#endif /* MIP6 */

	dst = (struct sockaddr_in6 *)&ip6_forward_rt.ro_dst;
	if (!srcrt) {
#ifdef MEASURE_PERFORMANCE
		ctr_beg = read_tsc();
#endif

		/*
		 * ip6_forward_rt.ro_dst.sin6_addr is equal to ip6->ip6_dst
		 */
		if (ip6_forward_rt.ro_rt == 0 ||
		    (ip6_forward_rt.ro_rt->rt_flags & RTF_UP) == 0
#ifdef MEASURE_PERFORMANCE
		    || (ip6_ours_check_algorithm != OURS_CHECK_ALG_RTABLE &&
			!IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst, &dst->sin6_addr))
#endif
			) {
			if (ip6_forward_rt.ro_rt) {
				RTFREE(ip6_forward_rt.ro_rt);
				ip6_forward_rt.ro_rt = 0;
			}
#ifdef MEASURE_PERFORMANCE
			ip6_forward_cache_miss++;
			bzero(dst, sizeof(*dst));
			dst->sin6_family = AF_INET6;
			dst->sin6_len = sizeof(*dst);
			dst->sin6_addr = ip6->ip6_dst;
#endif
			/* this probably fails but give it a try again */
#ifdef __FreeBSD__
			rtalloc_ign((struct route *)&ip6_forward_rt,
				    RTF_PRCLONING);
#else
			rtalloc((struct route *)&ip6_forward_rt);
#endif
		}

#ifdef MEASURE_PERFORMANCE
		ctr_end = read_tsc();
#ifdef MEASURE_PERFORMANCE_UDPONLY
		if (ip6->ip6_nxt == IPPROTO_UDP)
#endif
			add_performance_log2(ctr_end - ctr_beg);
#endif

		if (ip6_forward_rt.ro_rt == 0) {
			ip6stat.ip6s_noroute++;
            IP_STAT_OUT_NO_ROUTES_INC(-1, 1);
			in6_ifstat_inc(m->m_pkthdr.rcvif, ifs6_in_noroute);
            IFSTAT_InDiscards_INC(-1, origifp, 1);
            IFSTAT_InNoRoutes_INC(-1, origifp, 1);
			if (mcopy) {
				icmp6_error(mcopy, ICMP6_DST_UNREACH,
					    ICMP6_DST_UNREACH_NOROUTE, 0);
			}
			m_freem(m);
			return;
		}
	} else if ((rt = ip6_forward_rt.ro_rt) == 0 ||
		 !IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst, &dst->sin6_addr)) {
		if (ip6_forward_rt.ro_rt) {
			RTFREE(ip6_forward_rt.ro_rt);
			ip6_forward_rt.ro_rt = 0;
		}
		bzero(dst, sizeof(*dst));
		dst->sin6_len = sizeof(struct sockaddr_in6);
		dst->sin6_family = AF_INET6;
		dst->sin6_addr = ip6->ip6_dst;

#ifdef __FreeBSD__
  		rtalloc_ign((struct route *)&ip6_forward_rt, RTF_PRCLONING);
#else
		rtalloc((struct route *)&ip6_forward_rt);
#endif
		if (ip6_forward_rt.ro_rt == 0) {
			ip6stat.ip6s_noroute++;
            IP_STAT_OUT_NO_ROUTES_INC(-1, 1);
			in6_ifstat_inc(m->m_pkthdr.rcvif, ifs6_in_noroute);
            IFSTAT_InDiscards_INC(-1, origifp, 1);
            IFSTAT_InNoRoutes_INC(-1, origifp, 1);

			if (mcopy) {
				icmp6_error(mcopy, ICMP6_DST_UNREACH,
					    ICMP6_DST_UNREACH_NOROUTE, 0);
			}
			m_freem(m);
			return;
		}
	}
	rt = ip6_forward_rt.ro_rt;

	/*
	 * Scope check: if a packet can't be delivered to its destination
	 * for the reason that the destination is beyond the scope of the
	 * source address, discard the packet and return an icmp6 destination
	 * unreachable error with Code 2 (beyond scope of source address).
	 * [draft-ietf-ipngwg-icmp-v3-00.txt, Section 3.1]
	 */
	if ((srczone = in6_addr2zoneid(m->m_pkthdr.rcvif, &ip6->ip6_src)) < 0
	    || (dstzone = in6_addr2zoneid(rt->rt_ifp, &ip6->ip6_src)) < 0) {
		/* XXX: will this really happen?  should return an icmp error? */
		ip6stat.ip6s_cantforward++;
		ip6stat.ip6s_badscope++;
        IFSTAT_InDiscards_INC(-1, origifp, 1);
        IFSTAT_InAddrErrors_INC(-1, origifp, 1);
		m_freem(m);
		return;
	}
	if (srczone != dstzone) {
		ip6stat.ip6s_cantforward++;
		ip6stat.ip6s_badscope++;
		in6_ifstat_inc(rt->rt_ifp, ifs6_in_discard);
        IFSTAT_InDiscards_INC(-1, origifp, 1);
        IFSTAT_InAddrErrors_INC(-1, origifp, 1);

		if (ip6_log_time + ip6_log_interval < time_second) {
			ip6_log_time = time_second;
			log(LOG_DEBUG,
			    "cannot forward "
			    "src %s, dst %s, nxt %d, rcvif %s, outif %s\n",
			    ip6_sprintf(&ip6->ip6_src),
			    ip6_sprintf(&ip6->ip6_dst),
			    ip6->ip6_nxt,
			    if_name(m->m_pkthdr.rcvif), if_name(rt->rt_ifp));
		}
		if (mcopy)
			icmp6_error(mcopy, ICMP6_DST_UNREACH,
				    ICMP6_DST_UNREACH_BEYONDSCOPE, 0);
		m_freem(m);
		return;
	}

	if (m->m_pkthdr.len > rt->rt_ifp->if_mtu) {
		in6_ifstat_inc(rt->rt_ifp, ifs6_in_toobig);
        IFSTAT_InDiscards_INC(-1, origifp, 1);

		if (mcopy) {
			u_long mtu;
#ifdef IPSEC
			struct secpolicy *sp;
			int ipsecerror;
			size_t ipsechdrsiz;
#endif

			mtu = rt->rt_ifp->if_mtu;
#ifdef IPSEC
			/*
			 * When we do IPsec tunnel ingress, we need to play
			 * with if_mtu value (decrement IPsec header size
			 * from mtu value).  The code is much simpler than v4
			 * case, as we have the outgoing interface for
			 * encapsulated packet as "rt->rt_ifp".
			 */
			sp = ipsec6_getpolicybyaddr(mcopy, IPSEC_DIR_OUTBOUND,
				IP_FORWARDING, &ipsecerror);
			if (sp) {
				ipsechdrsiz = ipsec6_hdrsiz(mcopy,
					IPSEC_DIR_OUTBOUND, NULL);
				if (ipsechdrsiz < mtu)
					mtu -= ipsechdrsiz;
			}

			/*
			 * if mtu becomes less than minimum MTU,
			 * tell minimum MTU (and I'll need to fragment it).
			 */
			if (mtu < IPV6_MMTU)
				mtu = IPV6_MMTU;
#endif
			icmp6_error(mcopy, ICMP6_PACKET_TOO_BIG, 0, mtu);
		}
		m_freem(m);
		return;
 	}

	if (rt->rt_flags & RTF_GATEWAY)
		dst = (struct sockaddr_in6 *)rt->rt_gateway;

	/*
	 * If we are to forward the packet using the same interface
	 * as one we got the packet from, perhaps we should send a redirect
	 * to sender to shortcut a hop.
	 * Only send redirect if source is sending directly to us,
	 * and if packet was not source routed (or has any options).
	 * Also, don't send redirect if forwarding using a route
	 * modified by a redirect.
	 */
	if (rt->rt_ifp == m->m_pkthdr.rcvif && !srcrt &&
	    (rt->rt_flags & (RTF_DYNAMIC|RTF_MODIFIED)) == 0) {
		if ((rt->rt_ifp->if_flags & IFF_POINTOPOINT) &&
		    nd6_is_addr_neighbor((struct sockaddr_in6 *)&ip6_forward_rt.ro_dst, rt->rt_ifp)) {
            IFSTAT_InDiscards_INC(-1, origifp, 1);
			/*
			 * If the incoming interface is equal to the outgoing
			 * one, the link attached to the interface is
			 * point-to-point, and the IPv6 destination is
			 * regarded as on-link on the link, then it will be
			 * highly probable that the destination address does
			 * not exist on the link and that the packet is going
			 * to loop.  Thus, we immediately drop the packet and
			 * send an ICMPv6 error message.
			 * For other routing loops, we dare to let the packet
			 * go to the loop, so that a remote diagnosing host
			 * can detect the loop by traceroute.
			 * type/code is based on suggestion by Rich Draves.
			 * not sure if it is the best pick.
			 */
			icmp6_error(mcopy, ICMP6_DST_UNREACH,
				    ICMP6_DST_UNREACH_ADDR, 0);
			m_freem(m);
			return;
		}
		type = ND_REDIRECT;
	}

#if defined(IPV6FIREWALL) || (defined(__FreeBSD__) && __FreeBSD__ >= 4)
	/*
	 * Check with the firewall...
	 */
#if defined(__FreeBSD__) && __FreeBSD__ >= 4
	if (ip6_fw_enable && ip6_fw_chk_ptr) {
#else
	if (ip6_fw_chk_ptr) {
#endif
		/* If ipfw says divert, we have to just drop packet */
		if ((*ip6_fw_chk_ptr)(&ip6, rt->rt_ifp, &m)) {
            IFSTAT_InDiscards_INC(-1, origifp, 1);
			m_freem(m);
			goto freecopy;
		}
		if (!m)
			goto freecopy;
	}
#endif

	/*
	 * Fake scoped addresses. Note that even link-local source or
	 * destinaion can appear, if the originating node just sends the
	 * packet to us (without address resolution for the destination).
	 * Since both icmp6_error and icmp6_redirect_output fill the embedded
	 * link identifiers, we can do this stuff after making a copy for
	 * returning an error.
	 */
	if ((rt->rt_ifp->if_flags & IFF_LOOPBACK) != 0) {
		/*
		 * See corresponding comments in ip6_output.
		 * XXX: but is it possible that ip6_forward() sends a packet
		 *      to a loopback interface? I don't think so, and thus
		 *      I bark here. (jinmei@kame.net)
		 * XXX: it is common to route invalid packets to loopback.
		 *	also, the codepath will be visited on use of ::1 in
		 *	rthdr. (itojun)
		 */
#if 1
		if (0)
#else
		if ((rt->rt_flags & (RTF_BLACKHOLE|RTF_REJECT)) == 0)
#endif
		{
			printf("ip6_forward: outgoing interface is loopback. "
			       "src %s, dst %s, nxt %d, rcvif %s, outif %s\n",
			       ip6_sprintf(&ip6->ip6_src),
			       ip6_sprintf(&ip6->ip6_dst),
			       ip6->ip6_nxt, if_name(m->m_pkthdr.rcvif),
			       if_name(rt->rt_ifp));
		}

		/* we can just use rcvif in forwarding. */
		origifp = m->m_pkthdr.rcvif;
	}
	else
		origifp = rt->rt_ifp;
#ifndef SCOPEDROUTING
	/*
	 * clear embedded scope identifiers if necessary.
	 * in6_clearscope will touch the addresses only when necessary.
	 */
	in6_clearscope(&ip6->ip6_src);
	in6_clearscope(&ip6->ip6_dst);
#endif

#if defined(__NetBSD__) && defined(PFIL_HOOKS)
    {
	struct packet_filter_hook *pfh;
	struct mbuf *m1;
	int rv;

	/*
	 * Run through list of hooks for output packets.
	 */
	m1 = m;
	pfh = pfil_hook_get(PFIL_OUT, &inetsw[ip_protox[IPPROTO_IPV6]].pr_pfh);
	for (; pfh; pfh = pfh->pfil_link.tqe_next)
		if (pfh->pfil_func) {
		    	rv = pfh->pfil_func(ip6, sizeof(*ip6), rt->rt_ifp, 1, &m1);
			if (rv) {
				error = EHOSTUNREACH;
				goto senderr;
			}
			m = m1;
			if (m == NULL)
				goto freecopy;
			ip6 = mtod(m, struct ip6_hdr *);
		}
    }
#endif /* PFIL_HOOKS */

#if defined(__OpenBSD__) && NPF > 0
	if (pf_test6(PF_OUT, rt->rt_ifp, &m) != PF_PASS) {
		m_freem(m);
		goto senderr;
	}
	ip6 = mtod(m, struct ip6_hdr *);
#endif

	// The route we have found must be promoted to a HW route
	if (!(rt->rt_flags & RTF_HW_RT)) {
		rt_newnaddrmsg(RTM_VTSS_ADD, rt);
	}

	error = nd6_output(rt->rt_ifp, origifp, m, dst, rt);
	if (error) {
		in6_ifstat_inc(rt->rt_ifp, ifs6_out_discard);
		ip6stat.ip6s_cantforward++;
        switch ( error ) {
        case ENETUNREACH:
        case ENETDOWN:
        case EHOSTDOWN:
        case EHOSTUNREACH:
            IFSTAT_InNoRoutes_INC(-1, origifp, 1);
        default:
            IFSTAT_OutDiscards_INC(-1, rt->rt_ifp, 1);
            break;
        }
	} else {
		ip6stat.ip6s_forward++;
		in6_ifstat_inc(rt->rt_ifp, ifs6_out_forward);
        IFSTAT_OutForwDatagrams_INC(-1, rt->rt_ifp, 1);
		if (type)
			ip6stat.ip6s_redirectsent++;
		else {
			if (mcopy)
				goto freecopy;
		}
	}

#if (defined(__NetBSD__) && defined(PFIL_HOOKS)) || (defined(__OpenBSD__) && NPF > 0)
 senderr:
#endif
	if (mcopy == NULL)
		return;
	switch (error) {
	case 0:
#if 1
		if (type == ND_REDIRECT) {
			icmp6_redirect_output(mcopy, rt);
			return;
		}
#endif
		goto freecopy;

	case EMSGSIZE:
		/* xxx MTU is constant in PPP? */
		goto freecopy;

	case ENOBUFS:
		/* Tell source to slow down like source quench in IP? */
		goto freecopy;

	case ENETUNREACH:	/* shouldn't happen, checked above */
	case EHOSTUNREACH:
	case ENETDOWN:
	case EHOSTDOWN:
	default:
		type = ICMP6_DST_UNREACH;
		code = ICMP6_DST_UNREACH_ADDR;
		break;
	}
	icmp6_error(mcopy, type, code, 0);
	return;

 freecopy:
	m_freem(mcopy);
	return;
}
