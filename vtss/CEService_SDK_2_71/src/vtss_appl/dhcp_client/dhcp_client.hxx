// Many comments in this file is from RFC2131 and RFC2132
// This is the copyright informations from those documents:
//         "Distribution of this memo is unlimited."

/*

 Vitesse software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef __DHCP_CLIENT_HXX__
#define __DHCP_CLIENT_HXX__

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_CLIENT

extern "C" {
//#include "main.h"
#include "vtss_types.h"
#include "dhcp_client_api.h"
#include "ip2_utils.h"
}
#include "dhcp_frame.hxx"
#include "subject.hxx"
#include "optional.hxx"

size_t vtss_dhcp_client_hostname_get(char * buf, int max);

/*
 * From rfc2131 page 35
 * Removed reboot logic
 *                                        -------
 *                                       |       |<-------------------+
 *                 +-------------------->| INIT  |                    |
 *                 |         +---------->|       |<---+               |
 *                 |         |            -------     |               |
 *              DHCPNAK/     |               |                        |
 *           Discard offer   |      -/Send DHCPDISCOVER               |
 *                 |         |               |                        |
 *                 |      DHCPACK            v        |               |
 *                 |   (not accept.)/   -----------   |               |
 *                 |  Send DHCPDECLINE |           |                  |
 *                 |         |         | SELECTING |<----+            |
 *                 |        /          |           |     |DHCPOFFER/  |
 *                 |       /            -----------   |  |Collect     |
 *                 |      /                  |   |       |  replies   |
 *                 |     /  +----------------+   +-------+            |
 *                 |    |   v   Select offer/                         |
 *                ------------  send DHCPREQUEST      |               |
 *        +----->|            |             DHCPNAK, Lease expired/   |
 *        |      | REQUESTING |                  Halt network         |
 *    DHCPOFFER/ |            |                       |               |
 *    Discard     ------------                        |               |
 *        |        |        |                   -----------           |
 *        +--------+     DHCPACK/              |           |          |
 *                   Record lease, set    -----| REBINDING |          |
 *                     timers T1, T2     /     |           |          |
 *                          |        DHCPACK/   -----------           |
 *                          v     Record lease, set   ^               |
 *                       -------      /timers T1,T2   |               |
 *               +----->|       |<---+                |               |
 *               |      | BOUND |<---+                |               |
 *  DHCPOFFER, DHCPACK, |       |    |            T2 expires/   DHCPNAK/
 *   DHCPNAK/Discard     -------     |             Broadcast  Halt network
 *               |       | |         |            DHCPREQUEST         |
 *               +-------+ |        DHCPACK/          |               |
 *                    T1 expires/   Record lease, set |               |
 *                 Send DHCPREQUEST timers T1, T2     |               |
 *                 to leasing server |                |               |
 *                         |   ----------             |               |
 *                         |  |          |------------+               |
 *                         +->| RENEWING |                            |
 *                            |          |----------------------------+
 *                             ----------
 */

namespace VTSS {
namespace Dhcp {

struct ConfPacket
{
    ConfPacket() {
        memset(&data, 0, sizeof(data));
    }

    ConfPacket(const ConfPacket& rhs) {
        memcpy(&data, &rhs.data, sizeof(data));
    }

    ConfPacket(const vtss_dhcp_fields_t& rhs) {
        memcpy(&data, &rhs, sizeof(data));
    }

    ConfPacket& operator=(const ConfPacket& rhs) {
        memcpy(&data, &rhs.data, sizeof(data));
        return *this;
    }

    bool operator==(const ConfPacket& rhs) {
        return memcmp(&data, &rhs.data, sizeof(data)) == 0;
    }

    bool operator!=(const ConfPacket& rhs) {
        return memcmp(&data, &rhs.data, sizeof(data)) != 0;
    }

    vtss_dhcp_fields_t data;
};

template<typename L>
struct ScopeLock {
    ScopeLock(L & l) : l_(l) {
        l_.lock();
    }

    ~ScopeLock() {
        l_.unlock();
    }

private:
        L & l_;
};

// Copy selected options from a DHCP frame (including native fields)
template<typename LL>
void copy_dhcp_options(const DhcpFrame<LL>& f,
                       const Mac_t&         src_mac,
                       vtss_dhcp_fields_t  *copy)
{
    { // derive and copy IP address
        u32 ip_ = f.yiaddr().as_int();
        u32 prefix;

        Option::SubnetMask o;
        if (f.option_get(o)) {
            T_I("  add mask");
            vtss_conv_ipv4mask_to_prefix(o.ip().as_int(), &prefix);
        } else {
            T_I("  derive mask from IP class");
            prefix = vtss_ipv4_addr_to_prefix(ip_);
        }

        T_I("  add IP");
        copy->ip.address = ip_;
        copy->ip.prefix_size = prefix;
    }

    { // copy mac address
        src_mac.copy_to(copy->server_mac.addr);
    }

    { // server identifier
        Option::ServerIdentifier op_server_identifier;
        if (f.option_get(op_server_identifier)) {
            copy->server_ip = op_server_identifier.ip().as_int();
            copy->has_server_ip = TRUE;
            T_I("  add server identifier");
        } else {
            T_I("  no server identifier");
        }
    }

    { // extract default route
        Option::RouterOption o;
        if (f.option_get(o)) {
            copy->default_gateway = o.ip().as_int();
            copy->has_default_gateway = TRUE;
            T_I("  add gateway");
        } else {
            T_I("  no gateway");
        }
    }

    { // extract dns server
        Option::DnsServer o;
        if (f.option_get(o)) {
            copy->domain_name_server = o.ip().as_int();
            copy->has_domain_name_server = TRUE;
            T_I("  add domain name server");
        } else {
            T_I("  no domain name server");
        }
    }
}

template<typename FrameTxService,
         typename TimerService,
         typename Lock
        >
struct Client : public AbstraceTimeHandler
{
    // LOCKING: This class expected that the lock reference provided in the
    // constructor is allready locked, before a menber function is called. With
    // exception of the handle(Timer *t) function.

    typedef typename FrameTxService::template UdpStackType<
        DhcpFrame
    >::T FrameStack;

    typedef Optional<
        ConfPacket
    > AckConfPacket;

    enum { MAX_DHCP_MESSAGE_SIZE = 1024 };

    // Constructor and destructor
    Client(FrameTxService & tx, TimerService & ts, Lock & lock, u16 vlan);
    ~Client();

    // This is the public interface which may be used to control the dhcp client
    vtss_rc start();
    vtss_rc stop();
    vtss_rc if_down();
    vtss_rc if_up();
    vtss_rc release();
    vtss_rc decline();
    vtss_rc fallback();
    u16 vlan() const { return vlan_; }

    // Accept one the the received offers
    bool accept(unsigned idx);

    // Access to internal state
    vtss_dhcp_client_offer_list_t offers() const { return offer_list;}
    AckConfPacket ack_conf() const { return ack_conf_.get(); }
    AckConfPacket ack_conf(Trigger& t) { return ack_conf_.get(t); }
    vtss_dhcp4c_state_t state() const { return state_; }
    vtss_dhcp_client_status_t status() const;

    // The dhcp state machine is implemented in frame_event and handle(Timer *).
    // The dhcp clients assumes that the integrator injects DHCP frames in here.
    template<typename LL>
    void frame_event(const DhcpFrame<LL>& f, const Mac_t src_mac);

    // The second half of the dhcp state machine. Events requested by the dhcp
    // client will be delivered through this interface.
    // WARNING, this is a async call, and must be locked inside this class
    void handle(Timer *t);

private:
    template<typename LL>
    void latch_settings(const DhcpFrame<LL>& f,
                        const Mac_t&         src_mac);

    // Reset all state, arm the network halt timer, and start over when it fires
    void timed_retry();

    // clear all state, and cancel all timers
    void clear_state();

    void rearm_network_halt_timer();

    // Reset all state, goto to selection and send a new discovery
    void start_over();

    // validate and set the timers  timer_renew(T1), timer_rebind(T2) and
    // timer_ip_lease
    template<typename LL> bool latch_timers(const DhcpFrame<LL>& f);

    // Add the client identifier option to the frame
    void add_client_identifier(FrameStack &f);

    // Build and send a discovery frame
    bool send_discover();

    // Build and send a request frame
    bool send_request();

    // Build and send a broadcast request frame
    bool send_request_broadcast();

    // Build and send a release frame
    bool send_release();

    // Build and send a decline frame
    bool send_decline();

    // Helper functions to emit the frames on the wire
    bool send_unicast(FrameStack& f);
    bool send_broadcast(FrameStack& f);

    void update_xid();
    void sample_time();

    // The actually DHCP client state
    vtss_dhcp4c_state_t state_;

    // Policy services
    FrameTxService & tx_service;
    TimerService & timer_service;
    Lock lock_;

    // The four times used to drive the DHCP state machine
    Timer timer_network_halt;
    Timer timer_ip_lease;
    Timer timer_renew;  // T1
    Timer timer_rebind; // T2

    // The time configurations is set in the ACK frame. When this frame is
    // received we validate and latch the information here.
    seconds ip_address_lease_time;
    seconds renewal_time_value;
    seconds rebinding_time_value;

    // Latched when offer is requested
    vtss_dhcp_client_offer_list_t offer_list;
    IPv4_t yiaddr, server_identifier;
    Mac_t server_mac;

    // Latch the entire ack frame to provide options
    Subject<AckConfPacket> ack_conf_;

    // The vlan we operate on. Can not be changed!
    const u16 vlan_;

    // session id
    u32 xid_;
    u32 xid_seed_;

    eCosClock::time_t secs_;
};

// IMPLEMENTATION ------------------------------------------------------------
template<typename FS, typename TS, typename Lock>
Client<FS, TS, Lock>::Client(FS& tx, TS& ts, Lock & lock, u16 vlan) :
    state_(DHCP4C_STATE_STOPPED), tx_service(tx), timer_service(ts), lock_(lock),
    timer_network_halt(this), timer_ip_lease(this), timer_renew(this),
    timer_rebind(this), ack_conf_(ts), vlan_(vlan), xid_(0), xid_seed_(0)
{
    offer_list.valid_offers = 0;
}

template<typename FS, typename TS, typename Lock>
Client<FS, TS, Lock>::~Client()
{
    clear_state();
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::start()
{
    start_over();
    return VTSS_RC_OK;
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::stop()
{
    clear_state();
    state_ = DHCP4C_STATE_STOPPED;
    return VTSS_RC_OK;
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::fallback()
{
    clear_state();
    state_ = DHCP4C_STATE_FALLBACK;
    return VTSS_RC_OK;
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::if_down()
{
    return stop();
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::if_up()
{
    start_over();
    return VTSS_RC_OK;
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::release()
{
    vtss_rc rc;

    switch(state_) {
        case DHCP4C_STATE_REQUESTING:
        case DHCP4C_STATE_REBINDING:
        case DHCP4C_STATE_BOUND:
        case DHCP4C_STATE_RENEWING:
            rc = send_release() ? VTSS_RC_OK : VTSS_RC_ERROR;
            start_over();
            return rc;

        default:
            return VTSS_RC_ERROR;
    }
}

template<typename FS, typename TS, typename Lock>
vtss_rc Client<FS, TS, Lock>::decline()
{
    vtss_rc rc;

    switch(state_) {
        case DHCP4C_STATE_REQUESTING:
        case DHCP4C_STATE_REBINDING:
        case DHCP4C_STATE_BOUND:
        case DHCP4C_STATE_RENEWING:
            rc = send_decline() ? VTSS_RC_OK : VTSS_RC_ERROR;
            start_over();
            return rc;

        default:
            return VTSS_RC_ERROR;
    }
}

template<typename FS, typename TS, typename Lock>
vtss_dhcp_client_status_t Client<FS, TS, Lock>::status() const
{
    vtss_dhcp_client_status_t status;

    status.state = state_;
    status.offers = offer_list;

    switch(state_) {
        case DHCP4C_STATE_REQUESTING:
        case DHCP4C_STATE_REBINDING:
        case DHCP4C_STATE_BOUND:
        case DHCP4C_STATE_RENEWING:
            status.server_ip = server_identifier.as_int();
            break;

        default:
            status.server_ip = 0;
    }

    return status;
}


template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::accept(unsigned idx)
{
    if (state_ != DHCP4C_STATE_SELECTING) {
        T_I("Can only accept when state is SELECTING, current state: %s",
                vtss_dhcp4c_state_to_txt(state_));

        return false;
    }

    if (idx >= offer_list.valid_offers) {
        T_I("Can not accept offer: %u as the number of valid offers is %u",
                idx, offer_list.valid_offers);
        return false;
    }

    // latch selected parameters offered
    yiaddr.as_int(offer_list.list[idx].ip.address);
    server_identifier.as_int(offer_list.list[idx].server_ip);
    server_mac.copy_from(offer_list.list[idx].server_mac.addr);

    // Latch the option set we have accepted
    ack_conf_.set(AckConfPacket(offer_list.list[idx]));

    // Offer accepted, send request
    state_ = DHCP4C_STATE_REQUESTING;
    T_I("%u Offer accepted", vlan_);
    if (!send_request_broadcast()) {
        timed_retry(); // network error, will go to init
        return false;
    }

    rearm_network_halt_timer();
    return true;
}

template<typename FS, typename TS, typename Lock>
template<typename LL>
void Client<FS, TS, Lock>::frame_event(const DhcpFrame<LL>& f,
                                       const Mac_t src_mac)
{
    Option::MessageType message_type_;

    if (!f.option_get(message_type_)) {
        T_I("%u Not message type", vlan_);
        return;
    }

    Dhcp::MessageType message_type = message_type_.message_type();
    T_I("%u Got message: %d state: %s", vlan_,
        message_type, vtss_dhcp4c_state_to_txt(state_));

    if (f.xid() != xid_) {
        T_D("%u Xid not matched: %x != %x, skipping", vlan_,
            (unsigned int)xid_, (unsigned int)f.xid());
        return;
    }

    switch (state_) {
    case DHCP4C_STATE_SELECTING: {
        // Only react on offers
        if (message_type != DHCPOFFER) {
            return;
        }

        // Verify that offer is useful
        Option::ServerIdentifier op_server_identifier;
        if (!f.option_get(op_server_identifier)) {
            T_D("%u SELECTING, no server identifier... will retry", vlan_);
            timed_retry();
            return;
        }

        u32 prefix = 0;
        Option::SubnetMask mask;
        if (f.option_get(mask)) {
            (void) vtss_conv_ipv4mask_to_prefix(mask.ip().as_int(), &prefix);
        }
        if (prefix == 0) {
            prefix = vtss_ipv4_addr_to_prefix(f.yiaddr().as_int());
        }

        // Cache the received offer, and notify callbacks such that
        // they know there is something to accept
        if (offer_list.valid_offers < VTSS_DHCP_MAX_OFFERS) {
            vtss_dhcp_fields_t *offer =
                &offer_list.list[offer_list.valid_offers];
            T_I("%u Gather options from offer", vlan_);
            copy_dhcp_options(f, src_mac, offer);

            offer_list.valid_offers ++;
            ack_conf_.signal(Trigger::success);

        } else {
            T_I("%u Offer overflow. Can not cache more than %d offers. ",
                vlan_, VTSS_DHCP_MAX_OFFERS);
        }

        return;
    }

    case DHCP4C_STATE_REQUESTING: {
        // We need to start over
        if (message_type == DHCPNAK) {
            T_D("%u REQUESTING, got nak", vlan_);
            timed_retry();
            return;
        }

        // Ignore everyting beside ACK and NAK
        if (message_type != DHCPACK) {
            T_D("%u REQUESTING, not an ack", vlan_);
            return;
        }

        // We got an ack
        // TODO we should to an ARP to verify that the addresses
        // are actually free. (no 5, page 16 rfc2131)

        // DHCPACK/ Record lease, set timers T1, T2 -> BOUND
        T_D("%u Latching timers", vlan_);
        if (!latch_timers(f)) {
            T_I("%u Failed to patch timers", vlan_);
            timed_retry(); // Invalid ACK message
            return;
        }

        update_xid();
        T_D("%u REQUESTING, goto bound", vlan_);
        timer_service.timer_del(timer_network_halt);
        timer_service.timer_add(timer_renew, renewal_time_value);    // T1
        timer_service.timer_add(timer_rebind, rebinding_time_value); // T2
        timer_service.timer_add(timer_ip_lease, ip_address_lease_time);
        state_ = DHCP4C_STATE_BOUND;
        latch_settings(f, src_mac); // will notify listners
        return;
    }

    case DHCP4C_STATE_REBINDING:
    case DHCP4C_STATE_RENEWING:
        // DHCPNAK -> INIT
        if (message_type == DHCPNAK) {
            T_D("%u got nak", vlan_);
            timed_retry();
            return;
        }

        // Ignore everyting beside ACK and NAK
        if (message_type != DHCPACK) {
            T_D("%u not an ack", vlan_);
            return;
        }

        // TODO, if we got a new IP then do an ARP to verify that
        // it is free

        // DHCPACK/ Record lease, set/timers T1,T2 -> BOUND
        if (!latch_timers(f)) {
            T_D("%u invlaid timers", vlan_);
            timed_retry(); // Invalid ACK message
            return;
        }

        update_xid();
        timer_service.timer_del(timer_network_halt);
        timer_service.timer_add(timer_renew, renewal_time_value); // T1
        timer_service.timer_add(timer_rebind, rebinding_time_value); // T2
        timer_service.timer_add(timer_ip_lease, ip_address_lease_time);
        state_ = DHCP4C_STATE_BOUND;
        latch_settings(f, src_mac); // will notify listners
        return;

    default:
        break;
    }
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::handle(Timer *t)
{
    // Explicit locked here!!!
    ScopeLock<Lock> locked(lock_);

    if (!t->valid())
        return;

    T_D("%u Handle time event. state: %s", vlan_,
        vtss_dhcp4c_state_to_txt(state_));

    switch (state_) {
    case DHCP4C_STATE_STOPPED:
        T_D("%u STOPPED", vlan_);
        break;

    case DHCP4C_STATE_FALLBACK:
        T_D("%u FALLBACK", vlan_);
        break;

    case DHCP4C_STATE_INIT:
        if (t == &timer_network_halt) {
            T_D("%u INIT: goto to selecting", vlan_);
            state_ = DHCP4C_STATE_SELECTING;
            if (!send_discover()) {
                timed_retry();
                return;
            }
            rearm_network_halt_timer();
        }
        break;

    case DHCP4C_STATE_SELECTING:
        if (t == &timer_network_halt) {
            T_D("%u SELECTING: time out, will retry", vlan_);
            start_over();
        }
        break;

    case DHCP4C_STATE_REQUESTING:
        if (t == &timer_network_halt) {
            T_D("%u REQUESTING: time out, will retry", vlan_);
            start_over();
        }
        break;

    case DHCP4C_STATE_REBINDING:
        // remain in same state, but retransmit request
        if (t == &timer_network_halt) {
            T_D("%u REBINDING: time out, will retry", vlan_);
            if (!send_request_broadcast()) {
                timed_retry(); // will go to init state
                return;
            }
            rearm_network_halt_timer();
            return;
        }

        // Lease expired/ Halt network -> INIT
        if (t == &timer_ip_lease) {
            T_D("%u REBINDING: lease time out, goto to init", vlan_);
            // out of luck
            timed_retry(); // will go to init state
            return;
        }
        break;

    case DHCP4C_STATE_BOUND:
        // T1 expires/Send DHCPREQUEST to leasing server -> RENEWING
        if (t == &timer_renew) {
            T_D("%u BOUND: start renew", vlan_);

            // in rfc2131  the secs fields is defined as:
            //
            //     Filled in by client, seconds elapsed since client
            //     began address acquisition or renewal process.
            //
            // we therefor need to sample the current time when we send a new
            // discovery and when we leave bind
            sample_time();

            state_ = DHCP4C_STATE_RENEWING;
            if (!send_request()) {
                timed_retry(); // network error, will go to init state
                return;
            }

            rearm_network_halt_timer();
            return;
        }
        break;

    case DHCP4C_STATE_RENEWING:
        // remain in same state, but retransmit request
        if (t == &timer_network_halt) {
            T_D("%u RENEWING: time out, will retry", vlan_);
            if (!send_request()) {
                timed_retry(); // network error, will go to init state
                return;
            }
            rearm_network_halt_timer();
            return;
        }

        // T2 expires/ Broadcast DHCPREQUEST -> REBINDING
        if (t == &timer_rebind) {
            T_D("%u RENEWING: time out, goto rebind", vlan_);
            state_ = DHCP4C_STATE_REBINDING;
            if (!send_request_broadcast()) {
                timed_retry(); // network error, will go to init state
                return;
            }

            rearm_network_halt_timer();
            return;
        }
        break;
    }
}

template<typename FS, typename TS, typename Lock>
template<typename LL>
void Client<FS, TS, Lock>::latch_settings(const DhcpFrame<LL>& f,
                                          const Mac_t&         src_mac)
{
    T_I("%u setting ack conf", vlan_);
    vtss_dhcp_fields_t copy = ack_conf_.get()->data;
    copy_dhcp_options(f, src_mac, &copy);
    ack_conf_.set(AckConfPacket(copy), true);
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::timed_retry()
{
    clear_state();
    rearm_network_halt_timer();
    state_ = DHCP4C_STATE_INIT;
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::clear_state()
{
    state_ = DHCP4C_STATE_STOPPED;
    timer_service.timer_del(timer_network_halt);
    timer_service.timer_del(timer_ip_lease);
    timer_service.timer_del(timer_renew);
    timer_service.timer_del(timer_rebind);
    renewal_time_value = seconds(0);
    rebinding_time_value = seconds(0);
    ip_address_lease_time = seconds(0);
    offer_list.valid_offers = 0;

    T_I("%u clearing ack frame", vlan_);
    ack_conf_.set(AckConfPacket(), true);
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::rearm_network_halt_timer()
{
    timer_service.timer_del(timer_network_halt);
    timer_service.timer_add(timer_network_halt, seconds(15));
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::start_over()
{
    clear_state();
    rearm_network_halt_timer();
    state_ = DHCP4C_STATE_SELECTING;
    send_discover(); // error is handled by network halt timer
}

template<typename FS, typename TS, typename Lock>
template<typename LL>
bool Client<FS, TS, Lock>::latch_timers(const DhcpFrame<LL>& f)
{
    // Nice for debugging
    // ip_address_lease_time = seconds(240);
    // rebinding_time_value = seconds(120);
    // renewal_time_value = seconds(15);
    // return true;

    // get ip address lease time. This option MUST be
    // present in ACK messages. rfc2131 page 29
    Option::IpAddressLeaseTime op_ip_address_lease_time;
    if (f.option_get(op_ip_address_lease_time)) {
        ip_address_lease_time =
            seconds(op_ip_address_lease_time.val());

    } else {
        // Invalid ACK message
        T_W("%u no IpAddressLeaseTime", vlan_);
        return false;
    }

    // get renewval time. defaults to 0.5 * ip_address_lease_time.
    // rfc2131 4.4.5 page 41
    Option::RenewalTimeValue op_renewal_time_value;
    if (f.option_get(op_renewal_time_value)) {
        renewal_time_value = seconds(op_renewal_time_value.val());
    } else {
        renewal_time_value = seconds(0.5 * ip_address_lease_time.raw());
    }

    // get rebinding time. defaults to 0.875 *
    // ip_address_lease_time. rfc2131 4.4.5 page 41
    Option::RebindingTimeValue op_rebinding_time_value;
    if (f.option_get(op_rebinding_time_value)) {
        rebinding_time_value =
            seconds(op_rebinding_time_value.val());
    } else {
        rebinding_time_value =
            seconds(0.875 * ip_address_lease_time.raw());
    }

    // sanity checks
    if (ip_address_lease_time < renewal_time_value ||
        ip_address_lease_time < rebinding_time_value) {
        T_W("%u check 1 failed", vlan_);
        return false;
    }

    // sanity checks
    if (rebinding_time_value < renewal_time_value) {
        T_W("%u check 2 failed", vlan_);
        return false;
    }

    T_D("ip_address_lease_time = %llu seconds", ip_address_lease_time.raw());
    T_D("rebinding_time_value = %llu seconds", rebinding_time_value.raw());
    T_D("renewal_time_value = %llu seconds", renewal_time_value.raw());
    return true;
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::add_client_identifier(FrameStack &f)
{
    // Trying to be (bug-)complient with the old dhcp client
#define MAX_HOST_NAME 60
    char buf[MAX_HOST_NAME];
    buf[0] = 0;
    size_t host_name_size = vtss_dhcp_client_hostname_get(buf + 1,
                                                          MAX_HOST_NAME - 1);
    if (host_name_size) {
        Dhcp::Option::HostName host_name;
        host_name.set((const u8*)buf+1, host_name_size);
        f->add_option(host_name);
    }
    if (host_name_size) {
        Dhcp::Option::ClientIdentifier client_identifier;
        client_identifier.set((const u8*)buf, host_name_size);
        f->add_option(client_identifier);
        T_I("Adding client identifier:");
        T_I_HEX((const unsigned char *)buf, host_name_size);
    }
#undef MAX_HOST_NAME
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_discover()
{
    // choose a id to use for this dhcp session
    update_xid();

    // in rfc2131  the secs fields is defined as:
    //
    //     Filled in by client, seconds elapsed since client
    //     began address acquisition or renewal process.
    //
    // we therefor need to sample the current time when we send a new
    // discovery and when we leave bind
    sample_time();
    seconds sec(eCosClock::to_seconds(eCosClock::now() - secs_));

    FrameStack f;
    f->build_req_frame(tx_service.mac_address(vlan_), xid_, sec.raw32());

#ifdef VTSS_SW_OPTION_ARCH_JAGUAR_1
#  ifndef VTSS_SW_OPTION_DHCP_HELPER
    // Bug#10213
    f->flags(0x8000); // request server to send broadcast frames
#  endif
#endif

    f->add_option(Dhcp::Option::MessageType(Dhcp::DHCPDISCOVER));
    f->add_option(Dhcp::Option::MaximumDHCPMessageSize(MAX_DHCP_MESSAGE_SIZE));
    {
        // Require the same set of options as the existing eCos client
        Dhcp::Option::ParameterRequestList p;
        p.add(Dhcp::Option::CODE_SERVER_IDENTIFIER);
        p.add(Dhcp::Option::CODE_IP_ADDRESS_LEASE_TIME);
        p.add(Dhcp::Option::CODE_RENEWAL_TIME_VALUE);
        p.add(Dhcp::Option::CODE_REBINDING_TIME_VALUE);
        p.add(Dhcp::Option::CODE_SUBNET_MASK);
        p.add(Dhcp::Option::CODE_ROUTER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME_SERVER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME);
        p.add(Dhcp::Option::CODE_BROADCAST_ADDRESS_OPTION);
        p.add(Dhcp::Option::CODE_NETWORK_TIME_PROTOCOL_SERVERS_OPTION);
        f->add_option(p);
    }

    add_client_identifier(f);
    f->add_option(Dhcp::Option::End());
    return send_broadcast(f);
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_request()
{
    seconds sec(eCosClock::to_seconds(eCosClock::now() - secs_));

    FrameStack f;
    f->build_req_frame(tx_service.mac_address(vlan_), xid_, sec.raw32());
    f->add_option(Dhcp::Option::MessageType(Dhcp::DHCPREQUEST));
    f->add_option(Dhcp::Option::MaximumDHCPMessageSize(MAX_DHCP_MESSAGE_SIZE));
    {
        // Require the same set of options as the existing eCos client
        Dhcp::Option::ParameterRequestList p;
        p.add(Dhcp::Option::CODE_SERVER_IDENTIFIER);
        p.add(Dhcp::Option::CODE_IP_ADDRESS_LEASE_TIME);
        p.add(Dhcp::Option::CODE_RENEWAL_TIME_VALUE);
        p.add(Dhcp::Option::CODE_REBINDING_TIME_VALUE);
        p.add(Dhcp::Option::CODE_SUBNET_MASK);
        p.add(Dhcp::Option::CODE_ROUTER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME_SERVER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME);
        p.add(Dhcp::Option::CODE_BROADCAST_ADDRESS_OPTION);
        p.add(Dhcp::Option::CODE_NETWORK_TIME_PROTOCOL_SERVERS_OPTION);
        f->add_option(p);
    }
    f->add_option(Dhcp::Option::ServerIdentifier(server_identifier));
    f->add_option(Dhcp::Option::RequestedIPAddress(yiaddr));
    add_client_identifier(f);
    f->add_option(Dhcp::Option::End());
    return send_unicast(f);
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_request_broadcast()
{
    seconds sec(eCosClock::to_seconds(eCosClock::now() - secs_));

    FrameStack f;
    f->build_req_frame(tx_service.mac_address(vlan_), xid_, sec.raw32());
    f->add_option(Dhcp::Option::MessageType(Dhcp::DHCPREQUEST));
    {
        // Require the same set of options as the existing eCos client
        Dhcp::Option::ParameterRequestList p;
        p.add(Dhcp::Option::CODE_SERVER_IDENTIFIER);
        p.add(Dhcp::Option::CODE_IP_ADDRESS_LEASE_TIME);
        p.add(Dhcp::Option::CODE_RENEWAL_TIME_VALUE);
        p.add(Dhcp::Option::CODE_REBINDING_TIME_VALUE);
        p.add(Dhcp::Option::CODE_SUBNET_MASK);
        p.add(Dhcp::Option::CODE_ROUTER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME_SERVER_OPTION);
        p.add(Dhcp::Option::CODE_DOMAIN_NAME);
        p.add(Dhcp::Option::CODE_BROADCAST_ADDRESS_OPTION);
        p.add(Dhcp::Option::CODE_NETWORK_TIME_PROTOCOL_SERVERS_OPTION);
        f->add_option(p);
    }
    f->add_option(Dhcp::Option::MaximumDHCPMessageSize(MAX_DHCP_MESSAGE_SIZE));
    f->add_option(Dhcp::Option::ServerIdentifier(server_identifier));
    f->add_option(Dhcp::Option::RequestedIPAddress(yiaddr));
    add_client_identifier(f);
    f->add_option(Dhcp::Option::End());
    return send_broadcast(f);
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_release()
{
    seconds sec(eCosClock::to_seconds(eCosClock::now() - secs_));

    FrameStack f;
    f->build_req_frame(tx_service.mac_address(vlan_), xid_, sec.raw32());
    f->ciaddr(yiaddr);
    f->add_option(Dhcp::Option::MessageType(Dhcp::DHCPRELEASE));
    add_client_identifier(f);
    f->add_option(Dhcp::Option::End());
    return send_unicast(f);
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_decline()
{
    seconds sec(eCosClock::to_seconds(eCosClock::now() - secs_));

    FrameStack f;
    f->build_req_frame(tx_service.mac_address(vlan_), xid_, sec.raw32());
    f->add_option(Dhcp::Option::MessageType(Dhcp::DHCPDECLINE));
    add_client_identifier(f);
    f->add_option(Dhcp::Option::End());
    return send_unicast(f);
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_unicast(FrameStack& f)
{
    return tx_service.send_udp(f,
                               yiaddr, 68,            // src-udp
                               server_identifier, 67, // dst-udp
                               server_mac, vlan_);    // dst-mac
}

template<typename FS, typename TS, typename Lock>
bool Client<FS, TS, Lock>::send_broadcast(FrameStack& f)
{
    return tx_service.send_udp(f,
                               IPv4_t(0u), 68,                // src-udp
                               IPv4_t(0xffffffffu), 67,       // dst-udp
                               build_mac_broadcast(), vlan_); // dst-mac
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::update_xid()
{
    if (xid_seed_ == 0) {
        // Trying to gathere some entropy
        // using an xor of: mac-address, time-since boot, and vlan
        Mac_t mac = tx_service.mac_address(vlan_);
        u64 v1 = 0, v2 = 0;
        u16 * p16 = (u16 *)&v2;
        u8 * p8 = (u8 *)&v2;

        v1 = eCosClock::now();

        mac.copy_to(p8);
        p16 += 3;
        *p16 = vlan_;

        u64 v3 = v1 ^ v2;
        xid_seed_ = v3 ^ (v3 >> 32);
    }

    xid_ = rand_r((unsigned int *)&xid_seed_);
    T_D("%u XID = %x", vlan_, xid_);
}

template<typename FS, typename TS, typename Lock>
void Client<FS, TS, Lock>::sample_time()
{
    secs_ = eCosClock::now();
}

} /* Dhcp */
} /* VTSS */
#undef VTSS_TRACE_MODULE_ID
#endif
