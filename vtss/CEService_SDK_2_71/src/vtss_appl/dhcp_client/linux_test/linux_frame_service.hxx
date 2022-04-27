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
#ifndef __LINUX_FRAME_SERVICE_HXX__
#define __LINUX_FRAME_SERVICE_HXX__

#include "frame_utils.hxx"
#include <string.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/types.h>
#include <linux/filter.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>

//#include <iomanip>
//#include <iostream>

namespace VTSS {

struct LinuxRawFrameService
{
    LinuxRawFrameService(const char * ifname, struct sock_fprog * filter = 0) :
        socket_(-1),
        ifname_(ifname),
        ifindex_(ifname_to_ifindex(ifname_)),
        ifmac_(ifname_to_ifmac(ifname_))
    {
        struct sockaddr_ll addr;

        memset(&addr, 0, sizeof(addr));
        addr.sll_family = PF_PACKET;
        addr.sll_protocol = 0;
        addr.sll_ifindex = ifindex_;

        if ((socket_ = ::socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL))) < 0) {
            perror("socket: ");
            return;
        }

        if (filter && setsockopt(socket_, SOL_SOCKET, SO_ATTACH_FILTER, filter,
                                 sizeof(struct sock_fprog)) < 0) {
            perror("SO_ATTACH_FILTER: ");
        }

        if (::bind(socket_, (const sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind failed");
        }
    }

    template<template<class LL> class F>
    struct UdpStackType
    {
        typedef Stack3<Frame<1500>, IpFrame, UdpFrame, F> T;
    };

    /* Construct a ip/udp header for a packet, send packet */
    template<typename F>
    int send_udp(F& f,
                 const IPv4_t sip, const int sport,
                 const IPv4_t dip, const int dport,
                 const Mac_t & dst)
    {
        int fd, res;
        struct sockaddr_ll sll;

        f->update_deep();

        f.l1.src(sip);
        f.l1.dst(dip);
        f.l1.set_defaults();

        f.l2.src(sport);
        f.l2.dst(dport);
        f.l2.set_defaults();

        if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
            perror("Socket: ");
            abort();
        }

        memset(&sll, 0, sizeof(sll));
        sll.sll_family = AF_PACKET;
        sll.sll_protocol = htons(ETH_P_IP);
        sll.sll_ifindex = ifindex_;
        sll.sll_halen = 6;
        dst.copy_to((uint8_t *)sll.sll_addr);

        if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
            perror("Bind: ");
            abort();
        }

        res = sendto(fd, &(*f.buf.payload_begin()), f.buf.payload_length(),
                /*flags:*/ 0, (struct sockaddr *) &sll, sizeof(sll));

        if (res < 0) {
            perror("sendto: ");
        }

        if ((uint32_t)res != f.buf.payload_length()) {
            perror("short write: ");
        }

        close(fd);
        return res;
    }

    template<typename Buf>
    ssize_t read(Buf& b, Mac_t &src)
    {
        struct sockaddr_ll sll;
        socklen_t len = sizeof(sll);

        ssize_t res = recvfrom(socket_, &(*b.payload_begin()),
                b.max_size(), 0, (struct sockaddr *)&sll, &len);

        if (res >= 0) {
            assert(sll.sll_family == AF_PACKET);
            assert(sll.sll_halen == 6);
            src.copy_from((uint8_t *)sll.sll_addr);
            b.payload_length(res);
        }

        return res;
    }

    static int ifname_to_ifindex(const char * name)
    {
        int s;
        struct ifreq ifr;

        if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
            perror("socket(AF_NETLINK, SOCK_DGRAM, 0) failed");
            return -1;
        }

        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, name);
        if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
            perror("SIOCGIFINDEX failed: ");
            return -1;
        }
        close(s);

        return ifr.ifr_ifindex;
    }

    static Mac_t ifname_to_ifmac(const char * name)
    {
        int s;
        Mac_t mac;
        struct ifreq ifr;

        if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
            perror("socket(AF_NETLINK, SOCK_DGRAM, 0) failed");
            return mac;
        }

        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, name);
        if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
            perror("SIOCGIFHWADDR: ");
            close(s);
            return mac;
        }
        close(s);

        mac.copy_from((uint8_t *)&ifr.ifr_hwaddr.sa_data);
        return mac;
    }

    const Mac_t & mac_address() const  {
        return ifmac_;
    }

private:
    int socket_;
    const char * ifname_;
    const unsigned ifindex_;
    const Mac_t ifmac_;
};

} /* VTSS */

#endif
