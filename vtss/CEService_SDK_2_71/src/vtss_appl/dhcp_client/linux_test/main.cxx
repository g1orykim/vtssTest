/*

 Vitesse software.

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

#include <pthread.h>
#include <linux/types.h>
#include <linux/filter.h>

#include "timer_service.hxx"
#include "linux_frame_service.hxx"
#include "dhcp_client.hxx"

static struct sock_filter raw_udp_dhcp_rules [] = {
    BPF_STMT(BPF_LD|BPF_B|BPF_ABS, 9),         // load proto type
    BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, IPPROTO_UDP, 0, 4), // cmp proto with UDP
    BPF_STMT(BPF_LDX|BPF_B|BPF_MSH, 0),        // skip IP header
    BPF_STMT(BPF_LD|BPF_H|BPF_IND, 0),         // load udp src port
    BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, 67, 0, 1), // cmp udp src port with 67
    BPF_STMT(BPF_RET|BPF_K, 0xffffffff),       // accept all bytes
    BPF_STMT(BPF_RET|BPF_K, 0),                // reject
};

static struct sock_fprog raw_udp_dhcp_filter  = {
  sizeof raw_udp_dhcp_rules / sizeof(raw_udp_dhcp_rules[0]),
  raw_udp_dhcp_rules
};

struct PosixLock
{
    PosixLock() {
        ::pthread_mutex_init(&m, 0);
    }

    void lock() {
        ::pthread_mutex_lock(&m);
    }

    void unlock() {
        ::pthread_mutex_unlock(&m);
    }

    ~PosixLock() {
        ::pthread_mutex_destroy(&m);
    }

private:
    ::pthread_mutex_t m;
};

int main(int argc, const char *argv[])
{
    pthread_t timer_thread;
    PosixLock lock;
    VTSS::LinuxTimerService ts;

    pthread_create(&timer_thread, NULL, LinuxTimerService_run, &ts);

    VTSS::LinuxRawFrameService fs("eth1", &raw_udp_dhcp_filter);

    VTSS::Dhcp::Client<
        VTSS::LinuxRawFrameService,
        VTSS::LinuxTimerService,
        PosixLock
    > dhcp_client(fs, ts, lock, 0);

    dhcp_client.start();

    while(1) {
        VTSS::Mac_t mac;
        typedef VTSS::Frame<> L0;
        L0 f;
        int res = fs.read(f, mac);
        std::cout << res <<std::endl;

        std::cout << "RX hex:" << std::endl;
        std::cout << f << std::endl;

        // We know it is a IP frame
        typedef VTSS::IpFrame<L0> L1;
        L1 ip_frame(f);
        std::cout << "RX ip:" << std::endl;
        std::cout << ip_frame << std::endl;

        if (ip_frame.protocol() != 17) { // udp frame
            std::cout << "Discarding ip frame of protocol: " <<
                ip_frame.protocol() << std::endl;
            continue;
        }

        typedef VTSS::UdpFrame<L1> L2;
        L2 udp_frame(ip_frame);
        std::cout << udp_frame << std::endl;

        typedef VTSS::Dhcp::DhcpFrame<L2> L3;
        L3 dhcp_frame(udp_frame);
        std::cout << dhcp_frame << std::endl;

        dhcp_client.frame_event(dhcp_frame, mac);
    }

    pthread_join(timer_thread, 0);

    return 0;
}

