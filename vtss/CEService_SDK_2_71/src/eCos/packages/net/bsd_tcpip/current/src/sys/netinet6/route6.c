//==========================================================================
//
//      src/sys/netinet6/route6.c
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

/*	$KAME: route6.c,v 1.28 2001/09/21 08:50:05 keiichi Exp $	*/

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
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/queue.h>

#include <net/if.h>
#ifdef NEW_STRUCT_ROUTE
#include <net/route.h>
#endif

#include <netinet/in.h>
#include <netinet6/in6_var.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>

#include <netinet/icmp6.h>

#ifdef MIP6
#include <netinet6/mip6.h>
#endif

static int ip6_rthdr0 __P((struct mbuf *, struct ip6_hdr *,
    struct ip6_rthdr0 *));

int
route6_input(mp, offp, proto)
	struct mbuf **mp;
	int *offp, proto;	/* proto is unused */
{
	struct ip6_hdr *ip6;
	struct mbuf *m = *mp;
	struct ip6_rthdr *rh;
	int off = *offp, rhlen;
	struct mbuf *n;

	n = ip6_findaux(m);
	if (n) {
		struct ip6aux *ip6a = mtod(n, struct ip6aux *);
		/* XXX reject home-address option before rthdr */
		if (ip6a->ip6a_flags & IP6A_SWAP) {
			ip6stat.ip6s_badoptions++;
			m_freem(m);
			return IPPROTO_DONE;
		}
	}

#ifndef PULLDOWN_TEST
	IP6_EXTHDR_CHECK(m, off, sizeof(*rh), IPPROTO_DONE);
	ip6 = mtod(m, struct ip6_hdr *);
	rh = (struct ip6_rthdr *)((caddr_t)ip6 + off);
#else
	ip6 = mtod(m, struct ip6_hdr *);
	IP6_EXTHDR_GET(rh, struct ip6_rthdr *, m, off, sizeof(*rh));
	if (rh == NULL) {
		ip6stat.ip6s_tooshort++;
        IFSTAT_InHdrErrors_INC(-1, m->m_pkthdr.rcvif, 1);

		return IPPROTO_DONE;
	}
#endif

	switch (rh->ip6r_type) {
	case IPV6_RTHDR_TYPE_0:
		rhlen = (rh->ip6r_len + 1) << 3;
#ifndef PULLDOWN_TEST
		/*
		 * note on option length:
		 * due to IP6_EXTHDR_CHECK assumption, we cannot handle
		 * very big routing header (max rhlen == 2048).
		 */
		IP6_EXTHDR_CHECK(m, off, rhlen, IPPROTO_DONE);
#else
		/*
		 * note on option length:
		 * maximum rhlen: 2048
		 * max mbuf m_pulldown can handle: MCLBYTES == usually 2048
		 * so, here we are assuming that m_pulldown can handle
		 * rhlen == 2048 case.  this may not be a good thing to
		 * assume - we may want to avoid pulling it up altogether.
		 */
		IP6_EXTHDR_GET(rh, struct ip6_rthdr *, m, off, rhlen);
		if (rh == NULL) {
			ip6stat.ip6s_tooshort++;
            IFSTAT_InHdrErrors_INC(-1, m->m_pkthdr.rcvif, 1);

			return IPPROTO_DONE;
		}
#endif

#if (defined(VTSS_IP6_IMPL) && VTSS_IP6_IMPL)
		if (rh->ip6r_segleft != 0) {
			icmp6_error(m, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER,
						(caddr_t)&rh->ip6r_type - (caddr_t)ip6);

			return(IPPROTO_DONE);
		}

		/* Final; Use the original ip6_rthdr0 for further process */
#endif /* VTSS_IP6_IMPL */

		if (ip6_rthdr0(m, ip6, (struct ip6_rthdr0 *)rh))
			return(IPPROTO_DONE);
		break;
	default:
		/* unknown routing type */
		if (rh->ip6r_segleft == 0) {
			rhlen = (rh->ip6r_len + 1) << 3;
			break;	/* Final dst. Just ignore the header. */
		}
		ip6stat.ip6s_badoptions++;
		icmp6_error(m, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER,
			    (caddr_t)&rh->ip6r_type - (caddr_t)ip6);
		return(IPPROTO_DONE);
	}

	*offp += rhlen;
	return(rh->ip6r_nxt);
}

/*
 * Type0 routing header processing
 *
 * RFC2292 backward compatibility warning: no support for strict/loose bitmap,
 * as it was dropped between RFC1883 and RFC2460.
 */
static int
ip6_rthdr0(m, ip6, rh0)
	struct mbuf *m;
	struct ip6_hdr *ip6;
	struct ip6_rthdr0 *rh0;
{
	int addrs, index;
	struct in6_addr *nextaddr, tmpaddr;

	if (rh0->ip6r0_segleft == 0)
#ifdef MIP6
	{
		struct hif_softc *sc;
		struct mip6_bu *mbu;
		int lastindex;
		struct in6_addr *prevhop;
		struct mbuf *n;
		struct ip6aux *ip6a;

		if (!MIP6_IS_MN)
			return (0);

		/*
		 * check if the ip6_dst is my home address or not.  if
		 * true, check if the previous hop is the coa of the
		 * home address stored in the ip6_dst field.  if above
		 * two is true, this packet is route optimized packet.
		 */
		for (sc = TAILQ_FIRST(&hif_softc_list);
		     sc;
		     sc = TAILQ_NEXT(sc, hif_entry)) {
			for (mbu = LIST_FIRST(&sc->hif_bu_list);
			     mbu;
			     mbu = LIST_NEXT(mbu, mbu_entry)) {
				if ((mbu->mbu_flags & IP6_BUF_HOME) == 0)
					continue;

				/* XXX registration status ? */

				/* check the final dest is a home address. */
				if (!IN6_ARE_ADDR_EQUAL(&ip6->ip6_dst,
							&mbu->mbu_haddr))
					continue;

				/* check the last hop is a coa. */
				lastindex = rh0->ip6r0_len / 2;
				prevhop = (struct in6_addr *)(rh0 + 1)
					+ lastindex - 1;
				if (!IN6_ARE_ADDR_EQUAL(prevhop,
							&mbu->mbu_coa))
					continue;

				/*
				 * route is already optimized.  set
				 * optimized flag in m_aux.
				 */
				n = ip6_addaux(m);
				if (n) {
					ip6a = mtod(n, struct ip6aux *);
					ip6a->ip6a_flags = IP6A_ROUTEOPTIMIZED;
				}
			}
		}
		return (0);
	}
#else /* MIP6 */
		return(0);
#endif /* MIP6 */

	if (rh0->ip6r0_len % 2
#ifdef COMPAT_RFC1883
	    || rh0->ip6r0_len > 46
#endif
		) {
		/*
		 * Type 0 routing header can't contain more than 23 addresses.
		 * RFC 2462: this limitation was removed since strict/loose
		 * bitmap field was deleted.
		 */
		ip6stat.ip6s_badoptions++;
		icmp6_error(m, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER,
			    (caddr_t)&rh0->ip6r0_len - (caddr_t)ip6);
		return(-1);
	}

	if ((addrs = rh0->ip6r0_len / 2) < rh0->ip6r0_segleft) {
		ip6stat.ip6s_badoptions++;
		icmp6_error(m, ICMP6_PARAM_PROB, ICMP6_PARAMPROB_HEADER,
			    (caddr_t)&rh0->ip6r0_segleft - (caddr_t)ip6);
		return(-1);
	}

	index = addrs - rh0->ip6r0_segleft;
	rh0->ip6r0_segleft--;
	nextaddr = ((struct in6_addr *)(rh0 + 1)) + index;

	/*
	 * reject invalid addresses.  be proactive about malicious use of
	 * IPv4 mapped/compat address.
	 * XXX need more checks?
	 */
	if (IN6_IS_ADDR_MULTICAST(nextaddr) ||
	    IN6_IS_ADDR_UNSPECIFIED(nextaddr) ||
	    IN6_IS_ADDR_V4MAPPED(nextaddr) ||
	    IN6_IS_ADDR_V4COMPAT(nextaddr)) {
		ip6stat.ip6s_badoptions++;
		m_freem(m);
		return(-1);
	}
	if (IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst) ||
	    IN6_IS_ADDR_UNSPECIFIED(&ip6->ip6_dst) ||
	    IN6_IS_ADDR_V4MAPPED(&ip6->ip6_dst) ||
	    IN6_IS_ADDR_V4COMPAT(&ip6->ip6_dst)) {
		ip6stat.ip6s_badoptions++;
		m_freem(m);
		return(-1);
	}

	/*
	 * Swap the IPv6 destination address and nextaddr. Forward the packet.
	 */
	tmpaddr = *nextaddr;
	*nextaddr = ip6->ip6_dst;
	if (IN6_IS_ADDR_LINKLOCAL(nextaddr))
		nextaddr->s6_addr16[1] = 0;
	ip6->ip6_dst = tmpaddr;
	if (IN6_IS_ADDR_LINKLOCAL(&ip6->ip6_dst))
		ip6->ip6_dst.s6_addr16[1] = htons(m->m_pkthdr.rcvif->if_index);

#ifdef COMPAT_RFC1883
	if (rh0->ip6r0_slmap[index / 8] & (1 << (7 - (index % 8))))
		ip6_forward(m, IPV6_SRCRT_NEIGHBOR);
	else
		ip6_forward(m, IPV6_SRCRT_NOTNEIGHBOR);
#else
	ip6_forward(m, 1);
#endif

	return(-1);			/* m would be freed in ip6_forward() */
}
