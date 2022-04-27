/*

 Vitesse Switch Software.

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
// Many comments in this file is from RFC2131 and RFC2132
// This is the copyright informations from those documents:
//         "Distribution of this memo is unlimited."

#ifndef __DHCP_FRAME_HXX__
#define __DHCP_FRAME_HXX__

#include "meta.hxx"
#include "frame_utils.hxx"
#include "algorithm.hxx"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_CLIENT

namespace VTSS { namespace Dhcp {

template<unsigned SIZE>
struct Buf_t
{
    Buf_t() {}

    template<typename I>
    Buf_t(I d) {
        for(unsigned i = 0; i < SIZE; ++i) {
            data_[i] = *d++;
        }
    }

    unsigned size() const { return SIZE; }

    void copy_to(u8 *buf) const {
        for(unsigned i = 0; i < SIZE; ++i) {
            *buf++ = data_[i];
        }
    }

    template<typename I>
    void copy_from(I buf) {
        for(unsigned i = 0; i < SIZE; ++i) {
            data_[i] = *buf++;
        }
    }

    u8 data_[SIZE];
};

enum Op {
    OP_BOOTREQUEST = 1,
    OP_BOOTREPLY = 2,
};

enum Htype {
    HTYPE_10MB = 1,
};

enum MessageType {
    DHCPDISCOVER = 1,
    DHCPOFFER    = 2,
    DHCPREQUEST  = 3,
    DHCPDECLINE  = 4,
    DHCPACK      = 5,
    DHCPNAK      = 6,
    DHCPRELEASE  = 7,
    DHCPINFORM   = 8,
};

enum MagicValue {
    MAGIC = 0x63825363,
};

Mac_t build_mac_broadcast()
{
    u8 tmp [] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    return Mac_t(tmp);
}

struct SName_t : public Buf_t<64>
{
    SName_t() {}

    template<typename I>
    SName_t(I d) : Buf_t<64>(d) { }
};

#ifdef __linux
std::ostream& operator<<(std::ostream& o, const SName_t& n)
{
    for (unsigned i = 0; i < n.size() && n.data_[i] != 0; ++i)
        o.put(n.data_[i]);
    return o;
}
#endif

struct File_t : public Buf_t<128>
{
    File_t() {}
    template<typename I>
    File_t(I d) : Buf_t<128>(d) { }
};

#ifdef __linux
std::ostream& operator<<(std::ostream& o, const File_t& n)
{
    for (unsigned i = 0; i < n.size() && n.data_[i] != 0; ++i)
        o.put(n.data_[i]);
    return o;
}
#endif


namespace Option
{

enum Code {
    CODE_PAD                                     =  0,
    CODE_SUBNET_MASK                             =  1,
    CODE_TIME_OFFSET                             =  2,
    CODE_ROUTER_OPTION                           =  3,
    CODE_TIME_SERVER_OPTION                      =  4,
    CODE_NAME_SERVER_OPTION                      =  5,
    CODE_DOMAIN_NAME_SERVER_OPTION               =  6,
    CODE_LOG_SERVER_OPTION                       =  7,
    CODE_COOKIE_SERVER_OPTION                    =  8,
    CODE_LPR_SERVER_OPTION                       =  9,
    CODE_IMPRESS_SERVER_OPTION                   = 10,
    CODE_RESOURCE_LOCATION_SERVER_OPTION         = 11,
    CODE_HOST_NAME_OPTION                        = 12,
    CODE_BOOT_FILE_SIZE_OPTION                   = 13,
    CODE_MERIT_DUMP_FILE                         = 14,
    CODE_DOMAIN_NAME                             = 15,
    CODE_SWAP_SERVER                             = 16,
    CODE_ROOT_PATH                               = 17,
    CODE_EXTENSIONS_PATH                         = 18,
    CODE_IP_FORWARDING_ENABLE_DISABLE_OPTION     = 19,
    CODE_NON_LOCAL_SOURCE_ROUTING_OPTION         = 20,
    CODE_POLICY_FILTER_OPTION                    = 21,
    CODE_MAXIMUM_DATAGRAM_REASSEMBLY_SIZE        = 22,
    CODE_DEFAULT_IP_TIME_TO_LIVE                 = 23,
    CODE_PATH_MTU_AGING_TIMEOUT_OPTION           = 24,
    CODE_PATH_MTU_PLATEAU_TABLE_OPTION           = 25,
    CODE_INTERFACE_MTU_OPTION                    = 26,
    CODE_BROADCAST_ADDRESS_OPTION                = 28,
    CODE_MASK_SUPPLIER_OPTION                    = 30,
    CODE_ROUTER_SOLICITATION_ADDRESS_OPTION      = 32,
    CODE_TRAILER_ENCAPSULATION_OPTION            = 34,
    CODE_ETHERNET_ENCAPSULATION_OPTION           = 36,
    CODE_TCP_KEEPALIVE_INTERVAL_OPTION           = 38,
    CODE_NIS_DOMAIN_OPTION                       = 40,
    CODE_NETWORK_TIME_PROTOCOL_SERVERS_OPTION    = 42,
    CODE_NETBIOS_OVER_TCPIP_NAME_SERVER_OPTION   = 44,
    CODE_NETBIOS_OVER_TCPIP_NODE_TYPE_OPTION     = 46,
    CODE_NETBIOS_OVER_TCPIP_SCOPE_OPTION         = 47,
    CODE_X_WINDOW_SYSTEM_DISPLAY_MANAGER_OPTION  = 49,
    CODE_REQUESTED_IP_ADDRESS                    = 50,
    CODE_IP_ADDRESS_LEASE_TIME                   = 51,
    CODE_OPTION_OVERLOAD                         = 52,
    CODE_DHCP_MESSAGE_TYPE                       = 53,
    CODE_SERVER_IDENTIFIER                       = 54,
    CODE_PARAMETER_REQUEST_LIST                  = 55,
    CODE_MESSAGE                                 = 56,
    CODE_MAXIMUM_DHCP_MESSAGE_SIZE               = 57,
    CODE_RENEWAL_TIME_VALUE                      = 58,
    CODE_REBINDING_TIME_VALUE                    = 59,
    CODE_VENDOR_CLASS_IDENTIFIER                 = 60,
    CODE_CLIENT_IDENTIFIER                       = 61,
    CODE_NIS_PLUS_SERVERS_OPTION                 = 65,
    CODE_TFTP_SERVER_NAME                        = 66,
    CODE_SMTP_SERVER_OPTION                      = 69,
    CODE_NNTP_SERVER_OPTION                      = 71,
    CODE_DEFAULT_FINGER_SERVER_OPTION            = 73,
    CODE_STREETTALK_SERVER_OPTION                = 75,
    CODE_END                                     = 255,
};

template <u8 CODE, u8 LEN>
struct Base
{
    Base() {
        data_[0] = CODE;
        data_[1] = LEN - 2;
    }

    u8 code() const {
        return CODE;
    }

    const u8 * buf() const {
        return data_;
    }

    u8 length() const {
        return LEN;
    }

    // todo, make private
    template<typename I>
    void copy_from(I buf, unsigned len) {
        for(unsigned i = 0; i < min(len, (unsigned)LEN); ++i) {
            data_[i] = *buf++;
        }
    }

protected:
    u8 data_[LEN];
};

template <u8 CODE>
struct Base<CODE, 1>
{
    Base() {
        data_[0] = CODE;
    }

    const u8 * buf() const {
        return data_;
    }

    u8 length() const {
        return 1;
    }

protected:
    u8 data_[1];
};

template<u8 CODE>
struct Uint16Base : public Base<CODE, 4>
{
    Uint16Base() : Base<CODE, 4>() {}
    Uint16Base(u16 i) : Base<CODE, 4>() { val(i); }

    void val(u16 v) {
        write_uint16(v, Base<CODE, 4>::data_ + 2);
    }

    u16 val() const {
        return read_uint16(Base<CODE, 4>::data_ + 2);
    }
};


template<u8 CODE>
struct Uint32Base : public Base<CODE, 6>
{
    Uint32Base() : Base<CODE, 6>() {}
    Uint32Base(u32 i) : Base<CODE, 6>() { val(i); }

    void val(u32 v) {
        write_uint32(v, Base<CODE, 6>::data_ + 2);
    }

    u32 val() const {
        return read_uint32(Base<CODE, 6>::data_ + 2);
    }
};

template <u8 CODE>
struct IP1Base : public Base<CODE, 6>
{
    IP1Base() : Base<CODE, 6>() {}
    IP1Base(IPv4_t i) : Base<CODE, 6>() {
        ip(i);
    }

    IPv4_t ip() const {
        return IPv4_t(read_uint32(Base<CODE, 6>::data_ + 2));
    }

    void ip(IPv4_t i) {
        write_uint32(i.as_int(), Base<CODE, 6>::data_ + 2);
    }
};

// TODO, rewrite
template <u8 CODE>
struct IPnBase : public Base<CODE, 6>
{
    IPnBase() : Base<CODE, 6>() {}
    IPnBase(IPv4_t i) : Base<CODE, 6>() {
        ip(i);
    }

    IPv4_t ip() const {
        u32 val;
        u8 *p = (u8 *)(&val);
        p[0] = Base<CODE, 6>::data_[2];
        p[1] = Base<CODE, 6>::data_[3];
        p[2] = Base<CODE, 6>::data_[4];
        p[3] = Base<CODE, 6>::data_[5];
        return IPv4_t(ntohl(val));
    }

    void ip(IPv4_t i) {
        u32 val = htonl(i.as_int());
        u8 *p = (u8 *)(&val);
        Base<CODE, 6>::data_[2] = p[0];
        Base<CODE, 6>::data_[3] = p[1];
        Base<CODE, 6>::data_[4] = p[2];
        Base<CODE, 6>::data_[5] = p[3];
    }
};

template <u8 CODE>
struct StringBase : public Base<CODE, 255>
{
    StringBase() {
        Base<CODE, 255>::data_[1] = 0;
    }

    bool set(const u8 *data, size_t length) {
        u8 cnt = 0;

        if (length + 2 >= 255) {
            return false;
        }

        for (cnt = 0; cnt < length; ) {
            Base<CODE, 255>::data_[cnt + 2] = *data++;
            ++cnt;
        }
        Base<CODE, 255>::data_[1] = cnt;

        return true;
    }

    u8 length() const {
        return Base<CODE, 255>::data_[1] + 2;
    }

    const u8 * data() const {
        return Base<CODE, 255>::data_ + 2;
    }
};

// 3.1. Pad Option
//
//    The pad option can be used to cause subsequent fields to align on
//    word boundaries.
//
//    The code for the pad option is 0, and its length is 1 octet.
//
//     Code
//    +-----+
//    |  0  |
//    +-----+
struct Pad : public Base<CODE_PAD, 1> { };

// 3.2. End Option
//
//    The end option marks the end of valid information in the vendor
//    field.  Subsequent octets should be filled with pad options.
//
//    The code for the end option is 255, and its length is 1 octet.
//
//     Code
//    +-----+
//    | 255 |
//    +-----+
struct End : public Base<CODE_END, 1> { };

// 3.3. Subnet Mask
//
//    The subnet mask option specifies the client's subnet mask as per RFC
//    950 [5].
//
//    If both the subnet mask and the router option are specified in a DHCP
//    reply, the subnet mask option MUST be first.
//
//    The code for the subnet mask option is 1, and its length is 4 octets.
//
//     Code   Len        Subnet Mask
//    +-----+-----+-----+-----+-----+-----+
//    |  1  |  4  |  m1 |  m2 |  m3 |  m4 |
//    +-----+-----+-----+-----+-----+-----+
struct SubnetMask : public IP1Base<CODE_SUBNET_MASK> { };

// 3.4. Time Offset
//
//    The time offset field specifies the offset of the client's subnet in
//    seconds from Coordinated Universal Time (UTC).  The offset is
//    expressed as a two's complement 32-bit integer.  A positive offset
//    indicates a location east of the zero meridian and a negative offset
//    indicates a location west of the zero meridian.
//
//    The code for the time offset option is 2, and its length is 4 octets.
//
//     Code   Len        Time Offset
//    +-----+-----+-----+-----+-----+-----+
//    |  2  |  4  |  n1 |  n2 |  n3 |  n4 |
//    +-----+-----+-----+-----+-----+-----+
struct TimeOffset : public IP1Base<CODE_TIME_OFFSET> { };

// 3.5. Router Option
//
//    The router option specifies a list of IP addresses for routers on the
//    client's subnet.  Routers SHOULD be listed in order of preference.
//
//    The code for the router option is 3.  The minimum length for the
//    router option is 4 octets, and the length MUST always be a multiple
//    of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  3  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
struct RouterOption : public IPnBase<CODE_ROUTER_OPTION> { };

// 3.6. Time Server Option
//
//    The time server option specifies a list of RFC 868 [6] time servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for the time server option is 4.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  4  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
struct TimeServer : public IPnBase<CODE_TIME_SERVER_OPTION> { };

// 3.7. Name Server Option
//
//    The name server option specifies a list of IEN 116 [7] name servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for the name server option is 5.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  5  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct NameServer : public IPnBase<CODE_NAME_SERVER_OPTION> { };

// 3.8. Domain Name Server Option
//
//    The domain name server option specifies a list of Domain Name System
//    (STD 13, RFC 1035 [8]) name servers available to the client.  Servers
//    SHOULD be listed in order of preference.
//
//    The code for the domain name server option is 6.  The minimum length
//    for this option is 4 octets, and the length MUST always be a multiple
//    of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  6  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
struct DnsServer : public IPnBase<CODE_DOMAIN_NAME_SERVER_OPTION> { };

// 3.9. Log Server Option
//
//    The log server option specifies a list of MIT-LCS UDP log servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for the log server option is 7.  The minimum length for this
//    option is 4 octets, and the length MUST always be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  7  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct LogServer : public IPnBase<CODE_LOG_SERVER_OPTION> { };

// 3.10. Cookie Server Option
//
//    The cookie server option specifies a list of RFC 865 [9] cookie
//    servers available to the client.  Servers SHOULD be listed in order
//    of preference.
//
//    The code for the log server option is 8.  The minimum length for this
//    option is 4 octets, and the length MUST always be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  8  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct CookieServer : public IPnBase<CODE_COOKIE_SERVER_OPTION> { };

// 3.11. LPR Server Option
//
//    The LPR server option specifies a list of RFC 1179 [10] line printer
//    servers available to the client.  Servers SHOULD be listed in order
//    of preference.
//
//    The code for the LPR server option is 9.  The minimum length for this
//    option is 4 octets, and the length MUST always be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  9  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct LPRServer : public IPnBase<CODE_LPR_SERVER_OPTION> { };

// 3.12. Impress Server Option
//
//    The Impress server option specifies a list of Imagen Impress servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for the Impress server option is 10.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  10 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct ImpressServer : public IPnBase<CODE_IMPRESS_SERVER_OPTION> { };

// 3.13. Resource Location Server Option
//
//    This option specifies a list of RFC 887 [11] Resource Location
//    servers available to the client.  Servers SHOULD be listed in order
//    of preference.
//
//    The code for this option is 11.  The minimum length for this option
//    is 4 octets, and the length MUST always be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  11 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//struct ResourceLocationServer :
//      public IPnBase<CODE_RESOURCE_LOCATION_SERVER_OPTION> { };

// 3.14. Host Name Option
//
//    This option specifies the name of the client.  The name may or may
//    not be qualified with the local domain name (see section 3.17 for the
//    preferred way to retrieve the domain name).  See RFC 1035 for
//    character set restrictions.
//
//    The code for this option is 12, and its minimum length is 1.
//
//     Code   Len                 Host Name
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  12 |  n  |  h1 |  h2 |  h3 |  h4 |  h5 |  h6 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
struct HostName : public StringBase<CODE_HOST_NAME_OPTION> { };

// 3.15. Boot File Size Option
//
//    This option specifies the length in 512-octet blocks of the default
//    boot image for the client.  The file length is specified as an
//    unsigned 16-bit integer.
//
//    The code for this option is 13, and its length is 2.
//
//     Code   Len   File Size
//    +-----+-----+-----+-----+
//    |  13 |  2  |  l1 |  l2 |
//    +-----+-----+-----+-----+

// 3.16. Merit Dump File
//
//    This option specifies the path-name of a file to which the client's
//    core image should be dumped in the event the client crashes.  The
//    path is formatted as a character string consisting of characters from
//    the NVT ASCII character set.
//
//    The code for this option is 14.  Its minimum length is 1.
//
//     Code   Len      Dump File Pathname
//    +-----+-----+-----+-----+-----+-----+---
//    |  14 |  n  |  n1 |  n2 |  n3 |  n4 | ...
//    +-----+-----+-----+-----+-----+-----+---

// 3.17. Domain Name
//
//    This option specifies the domain name that client should use when
//    resolving hostnames via the Domain Name System.
//
//    The code for this option is 15.  Its minimum length is 1.
//
//     Code   Len        Domain Name
//    +-----+-----+-----+-----+-----+-----+--
//    |  15 |  n  |  d1 |  d2 |  d3 |  d4 |  ...
//    +-----+-----+-----+-----+-----+-----+--
struct DomainName: public StringBase<CODE_DOMAIN_NAME> { };

// 3.18. Swap Server
//
//    This specifies the IP address of the client's swap server.
//
//    The code for this option is 16 and its length is 4.
//
//     Code   Len    Swap Server Address
//    +-----+-----+-----+-----+-----+-----+
//    |  16 |  n  |  a1 |  a2 |  a3 |  a4 |
//    +-----+-----+-----+-----+-----+-----+
//    CODE_SWAP_SERVER

// 3.19. Root Path
//
//    This option specifies the path-name that contains the client's root
//    disk.  The path is formatted as a character string consisting of
//    characters from the NVT ASCII character set.
//
//    The code for this option is 17.  Its minimum length is 1.
//
//     Code   Len      Root Disk Pathname
//    +-----+-----+-----+-----+-----+-----+---
//    |  17 |  n  |  n1 |  n2 |  n3 |  n4 | ...
//    +-----+-----+-----+-----+-----+-----+---
//    CODE_ROOT_PATH

// 3.20. Extensions Path
//
//    A string to specify a file, retrievable via TFTP, which contains
//    information which can be interpreted in the same way as the 64-octet
//    vendor-extension field within the BOOTP response, with the following
//    exceptions:
//
//           - the length of the file is unconstrained;
//           - all references to Tag 18 (i.e., instances of the
//             BOOTP Extensions Path field) within the file are
//             ignored.
//
//    The code for this option is 18.  Its minimum length is 1.
//
//     Code   Len      Extensions Pathname
//    +-----+-----+-----+-----+-----+-----+---
//    |  18 |  n  |  n1 |  n2 |  n3 |  n4 | ...
//    +-----+-----+-----+-----+-----+-----+---
//    CODE_EXTENSIONS_PATH
//
// 4. IP Layer Parameters per Host
//
//    This section details the options that affect the operation of the IP
//    layer on a per-host basis.
//
// 4.1. IP Forwarding Enable/Disable Option
//
//    This option specifies whether the client should configure its IP
//    layer for packet forwarding.  A value of 0 means disable IP
//    forwarding, and a value of 1 means enable IP forwarding.
//
//    The code for this option is 19, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  19 |  1  | 0/1 |
//    +-----+-----+-----+
//    CODE_IP_FORWARDING_ENABLE_DISABLE_OPTION
//
// 4.2. Non-Local Source Routing Enable/Disable Option
//
//    This option specifies whether the client should configure its IP
//    layer to allow forwarding of datagrams with non-local source routes
//    (see Section 3.3.5 of [4] for a discussion of this topic).  A value
//    of 0 means disallow forwarding of such datagrams, and a value of 1
//    means allow forwarding.
//
//    The code for this option is 20, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  20 |  1  | 0/1 |
//    +-----+-----+-----+
//    CODE_NON_LOCAL_SOURCE_ROUTING_OPTION
//
// 4.3. Policy Filter Option
//
//    This option specifies policy filters for non-local source routing.
//    The filters consist of a list of IP addresses and masks which specify
//    destination/mask pairs with which to filter incoming source routes.
//
//    Any source routed datagram whose next-hop address does not match one
//    of the filters should be discarded by the client.
//
//    See [4] for further information.
//
//    The code for this option is 21.  The minimum length of this option is
//    8, and the length MUST be a multiple of 8.
//
//     Code   Len         Address 1                  Mask 1
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//    |  21 |  n  |  a1 |  a2 |  a3 |  a4 |  m1 |  m2 |  m3 |  m4 |
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//            Address 2                  Mask 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//    |  a1 |  a2 |  a3 |  a4 |  m1 |  m2 |  m3 |  m4 | ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//    CODE_POLICY_FILTER_OPTION
//
//
// 4.4. Maximum Datagram Reassembly Size
//
//    This option specifies the maximum size datagram that the client
//    should be prepared to reassemble.  The size is specified as a 16-bit
//    unsigned integer.  The minimum value legal value is 576.
//
//    The code for this option is 22, and its length is 2.
//
//     Code   Len      Size
//    +-----+-----+-----+-----+
//    |  22 |  2  |  s1 |  s2 |
//    +-----+-----+-----+-----+
//    CODE_MAXIMUM_DATAGRAM_REASSEMBLY_SIZE
//
// 4.5. Default IP Time-to-live
//
//    This option specifies the default time-to-live that the client should
//    use on outgoing datagrams.  The TTL is specified as an octet with a
//    value between 1 and 255.
//
//    The code for this option is 23, and its length is 1.
//
//     Code   Len   TTL
//    +-----+-----+-----+
//    |  23 |  1  | ttl |
//    +-----+-----+-----+
//    CODE_DEFAULT_IP_TIME_TO_LIVE
//
// 4.6. Path MTU Aging Timeout Option
//
//    This option specifies the timeout (in seconds) to use when aging Path
//    MTU values discovered by the mechanism defined in RFC 1191 [12].  The
//    timeout is specified as a 32-bit unsigned integer.
//
//    The code for this option is 24, and its length is 4.
//
//     Code   Len           Timeout
//    +-----+-----+-----+-----+-----+-----+
//    |  24 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
//    CODE_PATH_MTU_AGING_TIMEOUT_OPTION
//
// 4.7. Path MTU Plateau Table Option
//
//    This option specifies a table of MTU sizes to use when performing
//    Path MTU Discovery as defined in RFC 1191.  The table is formatted as
//    a list of 16-bit unsigned integers, ordered from smallest to largest.
//    The minimum MTU value cannot be smaller than 68.
//
//    The code for this option is 25.  Its minimum length is 2, and the
//    length MUST be a multiple of 2.
//
//     Code   Len     Size 1      Size 2
//    +-----+-----+-----+-----+-----+-----+---
//    |  25 |  n  |  s1 |  s2 |  s1 |  s2 | ...
//    +-----+-----+-----+-----+-----+-----+---
//    CODE_PATH_MTU_PLATEAU_TABLE_OPTION
//
// 5. IP Layer Parameters per Interface
//
//    This section details the options that affect the operation of the IP
//    layer on a per-interface basis.  It is expected that a client can
//    issue multiple requests, one per interface, in order to configure
//    interfaces with their specific parameters.
//
// 5.1. Interface MTU Option
//
//    This option specifies the MTU to use on this interface.  The MTU is
//    specified as a 16-bit unsigned integer.  The minimum legal value for
//    the MTU is 68.
//
//    The code for this option is 26, and its length is 2.
//
//     Code   Len      MTU
//    +-----+-----+-----+-----+
//    |  26 |  2  |  m1 |  m2 |
//    +-----+-----+-----+-----+
//    CODE_INTERFACE_MTU_OPTION
//
// 5.2. All Subnets are Local Option
//
//    This option specifies whether or not the client may assume that all
//    subnets of the IP network to which the client is connected use the
//    same MTU as the subnet of that network to which the client is
//    directly connected.  A value of 1 indicates that all subnets share
//    the same MTU.  A value of 0 means that the client should assume that
//    some subnets of the directly connected network may have smaller MTUs.
//
//    The code for this option is 27, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  27 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 5.3. Broadcast Address Option
//
//    This option specifies the broadcast address in use on the client's
//    subnet.  Legal values for broadcast addresses are specified in
//    section 3.2.1.3 of [4].
//
//    The code for this option is 28, and its length is 4.
//
//     Code   Len     Broadcast Address
//    +-----+-----+-----+-----+-----+-----+
//    |  28 |  4  |  b1 |  b2 |  b3 |  b4 |
//    +-----+-----+-----+-----+-----+-----+
//
// 5.4. Perform Mask Discovery Option
//
//    This option specifies whether or not the client should perform subnet
//    mask discovery using ICMP.  A value of 0 indicates that the client
//    should not perform mask discovery.  A value of 1 means that the
//    client should perform mask discovery.
//
//    The code for this option is 29, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  29 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 5.5. Mask Supplier Option
//
//    This option specifies whether or not the client should respond to
//    subnet mask requests using ICMP.  A value of 0 indicates that the
//    client should not respond.  A value of 1 means that the client should
//    respond.
//
//    The code for this option is 30, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  30 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 5.6. Perform Router Discovery Option
//
//    This option specifies whether or not the client should solicit
//    routers using the Router Discovery mechanism defined in RFC 1256
//    [13].  A value of 0 indicates that the client should not perform
//    router discovery.  A value of 1 means that the client should perform
//    router discovery.
//
//    The code for this option is 31, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  31 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 5.7. Router Solicitation Address Option
//
//    This option specifies the address to which the client should transmit
//    router solicitation requests.
//
//    The code for this option is 32, and its length is 4.
//
//     Code   Len            Address
//    +-----+-----+-----+-----+-----+-----+
//    |  32 |  4  |  a1 |  a2 |  a3 |  a4 |
//    +-----+-----+-----+-----+-----+-----+
//
// 5.8. Static Route Option
//
//    This option specifies a list of static routes that the client should
//    install in its routing cache.  If multiple routes to the same
//    destination are specified, they are listed in descending order of
//    priority.
//
//    The routes consist of a list of IP address pairs.  The first address
//    is the destination address, and the second address is the router for
//    the destination.
//
//    The default route (0.0.0.0) is an illegal destination for a static
//    route.  See section 3.5 for information about the router option.
//
//    The code for this option is 33.  The minimum length of this option is
//    8, and the length MUST be a multiple of 8.
//
//     Code   Len         Destination 1           Router 1
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//    |  33 |  n  |  d1 |  d2 |  d3 |  d4 |  r1 |  r2 |  r3 |  r4 |
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//            Destination 2           Router 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//    |  d1 |  d2 |  d3 |  d4 |  r1 |  r2 |  r3 |  r4 | ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//
// 6. Link Layer Parameters per Interface
//
//    This section lists the options that affect the operation of the data
//    link layer on a per-interface basis.
//
// 6.1. Trailer Encapsulation Option
//
//    This option specifies whether or not the client should negotiate the
//    use of trailers (RFC 893 [14]) when using the ARP protocol.  A value
//    of 0 indicates that the client should not attempt to use trailers.  A
//    value of 1 means that the client should attempt to use trailers.
//
//    The code for this option is 34, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  34 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 6.2. ARP Cache Timeout Option
//
//    This option specifies the timeout in seconds for ARP cache entries.
//    The time is specified as a 32-bit unsigned integer.
//
//    The code for this option is 35, and its length is 4.
//
//     Code   Len           Time
//    +-----+-----+-----+-----+-----+-----+
//    |  35 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
//
// 6.3. Ethernet Encapsulation Option
//
//    This option specifies whether or not the client should use Ethernet
//    Version 2 (RFC 894 [15]) or IEEE 802.3 (RFC 1042 [16]) encapsulation
//    if the interface is an Ethernet.  A value of 0 indicates that the
//    client should use RFC 894 encapsulation.  A value of 1 means that the
//    client should use RFC 1042 encapsulation.
//
//    The code for this option is 36, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  36 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 7. TCP Parameters
//
//    This section lists the options that affect the operation of the TCP
//    layer on a per-interface basis.
//
// 7.1. TCP Default TTL Option
//
//    This option specifies the default TTL that the client should use when
//    sending TCP segments.  The value is represented as an 8-bit unsigned
//    integer.  The minimum value is 1.
//
//    The code for this option is 37, and its length is 1.
//
//     Code   Len   TTL
//    +-----+-----+-----+
//    |  37 |  1  |  n  |
//    +-----+-----+-----+
//
// 7.2. TCP Keepalive Interval Option
//
//    This option specifies the interval (in seconds) that the client TCP
//    should wait before sending a keepalive message on a TCP connection.
//    The time is specified as a 32-bit unsigned integer.  A value of zero
//    indicates that the client should not generate keepalive messages on
//    connections unless specifically requested by an application.
//
//    The code for this option is 38, and its length is 4.
//
//     Code   Len           Time
//    +-----+-----+-----+-----+-----+-----+
//    |  38 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
//
// 7.3. TCP Keepalive Garbage Option
//
//    This option specifies the whether or not the client should send TCP
//    keepalive messages with a octet of garbage for compatibility with
//    older implementations.  A value of 0 indicates that a garbage octet
//    should not be sent. A value of 1 indicates that a garbage octet
//    should be sent.
//
//    The code for this option is 39, and its length is 1.
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  39 |  1  | 0/1 |
//    +-----+-----+-----+
//
// 8. Application and Service Parameters
//
//    This section details some miscellaneous options used to configure
//    miscellaneous applications and services.
//
// 8.1. Network Information Service Domain Option
//
//    This option specifies the name of the client's NIS [17] domain.  The
//    domain is formatted as a character string consisting of characters
//    from the NVT ASCII character set.
//
//    The code for this option is 40.  Its minimum length is 1.
//
//     Code   Len      NIS Domain Name
//    +-----+-----+-----+-----+-----+-----+---
//    |  40 |  n  |  n1 |  n2 |  n3 |  n4 | ...
//    +-----+-----+-----+-----+-----+-----+---
//
// 8.2. Network Information Servers Option
//
//    This option specifies a list of IP addresses indicating NIS servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for this option is 41.  Its minimum length is 4, and the
//    length MUST be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  41 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.3. Network Time Protocol Servers Option
//
//    This option specifies a list of IP addresses indicating NTP [18]
//    servers available to the client.  Servers SHOULD be listed in order
//    of preference.
//
//    The code for this option is 42.  Its minimum length is 4, and the
//    length MUST be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  42 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.4. Vendor Specific Information
//
//    This option is used by clients and servers to exchange vendor-
//    specific information.  The information is an opaque object of n
//    octets, presumably interpreted by vendor-specific code on the clients
//    and servers.  The definition of this information is vendor specific.
//    The vendor is indicated in the vendor class identifier option.
//    Servers not equipped to interpret the vendor-specific information
//    sent by a client MUST ignore it (although it may be reported).
//    Clients which do not receive desired vendor-specific information
//    SHOULD make an attempt to operate without it, although they may do so
//    (and announce they are doing so) in a degraded mode.
//
//    If a vendor potentially encodes more than one item of information in
//    this option, then the vendor SHOULD encode the option using
//    "Encapsulated vendor-specific options" as described below:
//
//    The Encapsulated vendor-specific options field SHOULD be encoded as a
//    sequence of code/length/value fields of identical syntax to the DHCP
//    options field with the following exceptions:
//
//       1) There SHOULD NOT be a "magic cookie" field in the encapsulated
//          vendor-specific extensions field.
//
//       2) Codes other than 0 or 255 MAY be redefined by the vendor within
//          the encapsulated vendor-specific extensions field, but SHOULD
//          conform to the tag-length-value syntax defined in section 2.
//
//       3) Code 255 (END), if present, signifies the end of the
//          encapsulated vendor extensions, not the end of the vendor
//          extensions field. If no code 255 is present, then the end of
//          the enclosing vendor-specific information field is taken as the
//          end of the encapsulated vendor-specific extensions field.
//
//    The code for this option is 43 and its minimum length is 1.
//
//    Code   Len   Vendor-specific information
//    +-----+-----+-----+-----+---
//    |  43 |  n  |  i1 |  i2 | ...
//    +-----+-----+-----+-----+---
//
//    When encapsulated vendor-specific extensions are used, the
//    information bytes 1-n have the following format:
//
//     Code   Len   Data item        Code   Len   Data item       Code
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//    |  T1 |  n  |  d1 |  d2 | ... |  T2 |  n  |  D1 |  D2 | ... | ... |
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//
// 8.5. NetBIOS over TCP/IP Name Server Option
//
//    The NetBIOS name server (NBNS) option specifies a list of RFC
//    1001/1002 [19] [20] NBNS name servers listed in order of preference.
//
//    The code for this option is 44.  The minimum length of the option is
//    4 octets, and the length must always be a multiple of 4.
//
//     Code   Len           Address 1              Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
//    |  44 |  n  |  a1 |  a2 |  a3 |  a4 |  b1 |  b2 |  b3 |  b4 | ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
//
// 8.6. NetBIOS over TCP/IP Datagram Distribution Server Option
//
//    The NetBIOS datagram distribution server (NBDD) option specifies a
//    list of RFC 1001/1002 NBDD servers listed in order of preference. The
//    code for this option is 45.  The minimum length of the option is 4
//    octets, and the length must always be a multiple of 4.
//
//     Code   Len           Address 1              Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
//    |  45 |  n  |  a1 |  a2 |  a3 |  a4 |  b1 |  b2 |  b3 |  b4 | ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
//
// 8.7. NetBIOS over TCP/IP Node Type Option
//
//    The NetBIOS node type option allows NetBIOS over TCP/IP clients which
//    are configurable to be configured as described in RFC 1001/1002.  The
//    value is specified as a single octet which identifies the client type
//    as follows:
//
//       Value         Node Type
//       -----         ---------
//       0x1           B-node
//       0x2           P-node
//       0x4           M-node
//       0x8           H-node
//
//    In the above chart, the notation '0x' indicates a number in base-16
//    (hexadecimal).
//
//    The code for this option is 46.  The length of this option is always
//    1.
//
//     Code   Len  Node Type
//    +-----+-----+-----------+
//    |  46 |  1  | see above |
//    +-----+-----+-----------+
//
// 8.8. NetBIOS over TCP/IP Scope Option
//
//    The NetBIOS scope option specifies the NetBIOS over TCP/IP scope
//    parameter for the client as specified in RFC 1001/1002. See [19],
//    [20], and [8] for character-set restrictions.
//
//    The code for this option is 47.  The minimum length of this option is
//    1.
//
//     Code   Len       NetBIOS Scope
//    +-----+-----+-----+-----+-----+-----+----
//    |  47 |  n  |  s1 |  s2 |  s3 |  s4 | ...
//    +-----+-----+-----+-----+-----+-----+----
//
// 8.9. X Window System Font Server Option
//
//    This option specifies a list of X Window System [21] Font servers
//    available to the client. Servers SHOULD be listed in order of
//    preference.
//
//    The code for this option is 48.  The minimum length of this option is
//    4 octets, and the length MUST be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//    |  48 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |   ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//
// 8.10. X Window System Display Manager Option
//
//    This option specifies a list of IP addresses of systems that are
//    running the X Window System Display Manager and are available to the
//    client.
//
//    Addresses SHOULD be listed in order of preference.
//
//    The code for the this option is 49. The minimum length of this option
//    is 4, and the length MUST be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//    |  49 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |   ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+---
//
// 9.1. Requested IP Address
//
//    This option is used in a client request (DHCPDISCOVER) to allow the
//    client to request that a particular IP address be assigned.
//
//    The code for this option is 50, and its length is 4.
//
//     Code   Len          Address
//    +-----+-----+-----+-----+-----+-----+
//    |  50 |  4  |  a1 |  a2 |  a3 |  a4 |
//    +-----+-----+-----+-----+-----+-----+
struct RequestedIPAddress : public IP1Base<CODE_REQUESTED_IP_ADDRESS> {
    RequestedIPAddress () : IP1Base<CODE_REQUESTED_IP_ADDRESS>() { }
    RequestedIPAddress (IPv4_t ip) : IP1Base<CODE_REQUESTED_IP_ADDRESS>(ip) { }
};

// 8.11. Network Information Service+ Domain Option
//
//    This option specifies the name of the client's NIS+ [17] domain.  The
//    domain is formatted as a character string consisting of characters
//    from the NVT ASCII character set.
//
//    The code for this option is 64.  Its minimum length is 1.
//
//     Code   Len      NIS Client Domain Name
//    +-----+-----+-----+-----+-----+-----+---
//    |  64 |  n  |  n1 |  n2 |  n3 |  n4 | ...
//    +-----+-----+-----+-----+-----+-----+---
//
// 8.12. Network Information Service+ Servers Option
//
//    This option specifies a list of IP addresses indicating NIS+ servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for this option is 65.  Its minimum length is 4, and the
//    length MUST be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    |  65 |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.13. Mobile IP Home Agent option
//
//    This option specifies a list of IP addresses indicating mobile IP
//    home agents available to the client.  Agents SHOULD be listed in
//    order of preference.
//
//    The code for this option is 68.  Its minimum length is 0 (indicating
//    no home agents are available) and the length MUST be a multiple of 4.
//    It is expected that the usual length will be four octets, containing
//    a single home agent's address.
//
//     Code Len    Home Agent Addresses (zero or more)
//    +-----+-----+-----+-----+-----+-----+--
//    | 68  |  n  | a1  | a2  | a3  | a4  | ...
//    +-----+-----+-----+-----+-----+-----+--
//
// 8.14. Simple Mail Transport Protocol (SMTP) Server Option
//
//    The SMTP server option specifies a list of SMTP servers available to
//    the client.  Servers SHOULD be listed in order of preference.
//
//    The code for the SMTP server option is 69.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 69  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.15. Post Office Protocol (POP3) Server Option
//
//    The POP3 server option specifies a list of POP3 available to the
//    client.  Servers SHOULD be listed in order of preference.
//
//    The code for the POP3 server option is 70.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 70  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.16. Network News Transport Protocol (NNTP) Server Option
//
//    The NNTP server option specifies a list of NNTP available to the
//    client.  Servers SHOULD be listed in order of preference.
//
//    The code for the NNTP server option is 71. The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 71  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.17. Default World Wide Web (WWW) Server Option
//
//    The WWW server option specifies a list of WWW available to the
//    client.  Servers SHOULD be listed in order of preference.
//
//    The code for the WWW server option is 72.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 72  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.18. Default Finger Server Option
//
//    The Finger server option specifies a list of Finger available to the
//    client.  Servers SHOULD be listed in order of preference.
//
//    The code for the Finger server option is 73.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 73  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.19. Default Internet Relay Chat (IRC) Server Option
//
//    The IRC server option specifies a list of IRC available to the
//    client.  Servers SHOULD be listed in order of preference.
//
//    The code for the IRC server option is 74.  The minimum length for
//    this option is 4 octets, and the length MUST always be a multiple of
//    4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 74  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.20. StreetTalk Server Option
//
//    The StreetTalk server option specifies a list of StreetTalk servers
//    available to the client.  Servers SHOULD be listed in order of
//    preference.
//
//    The code for the StreetTalk server option is 75.  The minimum length
//    for this option is 4 octets, and the length MUST always be a multiple
//    of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 75  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 8.21. StreetTalk Directory Assistance (STDA) Server Option
//
//    The StreetTalk Directory Assistance (STDA) server option specifies a
//    list of STDA servers available to the client.  Servers SHOULD be
//    listed in order of preference.
//
//    The code for the StreetTalk Directory Assistance server option is 76.
//    The minimum length for this option is 4 octets, and the length MUST
//    always be a multiple of 4.
//
//     Code   Len         Address 1               Address 2
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//    | 76  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 |  ...
//    +-----+-----+-----+-----+-----+-----+-----+-----+--
//
// 9. DHCP Extensions
//
//    This section details the options that are specific to DHCP.
//
// 9.1. Requested IP Address
//
//    This option is used in a client request (DHCPDISCOVER) to allow the
//    client to request that a particular IP address be assigned.
//
//    The code for this option is 50, and its length is 4.
//
//     Code   Len          Address
//    +-----+-----+-----+-----+-----+-----+
//    |  50 |  4  |  a1 |  a2 |  a3 |  a4 |
//    +-----+-----+-----+-----+-----+-----+
//
// 9.2. IP Address Lease Time
//
//    This option is used in a client request (DHCPDISCOVER or DHCPREQUEST)
//    to allow the client to request a lease time for the IP address.  In a
//    server reply (DHCPOFFER), a DHCP server uses this option to specify
//    the lease time it is willing to offer.
//
//    The time is in units of seconds, and is specified as a 32-bit
//    unsigned integer.
//
//    The code for this option is 51, and its length is 4.
//
//     Code   Len         Lease Time
//    +-----+-----+-----+-----+-----+-----+
//    |  51 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
struct IpAddressLeaseTime :
    public Uint32Base<CODE_IP_ADDRESS_LEASE_TIME> { };

// 9.3. Option Overload
//
//    This option is used to indicate that the DHCP 'sname' or 'file'
//    fields are being overloaded by using them to carry DHCP options. A
//    DHCP server inserts this option if the returned parameters will
//    exceed the usual space allotted for options.
//
//    If this option is present, the client interprets the specified
//    additional fields after it concludes interpretation of the standard
//    option fields.
//
//    The code for this option is 52, and its length is 1.  Legal values
//    for this option are:
//
//            Value   Meaning
//            -----   --------
//              1     the 'file' field is used to hold options
//              2     the 'sname' field is used to hold options
//              3     both fields are used to hold options
//
//     Code   Len  Value
//    +-----+-----+-----+
//    |  52 |  1  |1/2/3|
//    +-----+-----+-----+
//
// 9.4 TFTP server name
//
//    This option is used to identify a TFTP server when the 'sname' field
//    in the DHCP header has been used for DHCP options.
//
//    The code for this option is 66, and its minimum length is 1.
//
//        Code  Len   TFTP server
//       +-----+-----+-----+-----+-----+---
//       | 66  |  n  |  c1 |  c2 |  c3 | ...
//       +-----+-----+-----+-----+-----+---
//
// 9.5 Bootfile name
//
//    This option is used to identify a bootfile when the 'file' field in
//    the DHCP header has been used for DHCP options.
//
//    The code for this option is 67, and its minimum length is 1.
//
//        Code  Len   Bootfile name
//       +-----+-----+-----+-----+-----+---
//       | 67  |  n  |  c1 |  c2 |  c3 | ...
//       +-----+-----+-----+-----+-----+---
//
// 9.6. DHCP Message Type
//
//    This option is used to convey the type of the DHCP message.  The code
//    for this option is 53, and its length is 1.  Legal values for this
//    option are:
//
//            Value   Message Type
//            -----   ------------
//              1     DHCPDISCOVER
//              2     DHCPOFFER
//              3     DHCPREQUEST
//              4     DHCPDECLINE
//              5     DHCPACK
//              6     DHCPNAK
//              7     DHCPRELEASE
//              8     DHCPINFORM
//
//     Code   Len  Type
//    +-----+-----+-----+
//    |  53 |  1  | 1-9 |
//    +-----+-----+-----+
struct MessageType : public Base<CODE_DHCP_MESSAGE_TYPE, 3> {
    typedef Base<CODE_DHCP_MESSAGE_TYPE, 3> This_t;
    MessageType() : This_t() { }
    MessageType(Dhcp::MessageType t) : This_t() {
        data_[2] = t;
    }
    Dhcp::MessageType message_type() const {
        return (Dhcp::MessageType) This_t::data_[2];
    }
    void message_type(Dhcp::MessageType t) {
        This_t::data_[2] = t;
    }
    Dhcp::MessageType message_type() {
        return (Dhcp::MessageType) This_t::data_[2];
    }
};

// 9.7. Server Identifier
//
//    This option is used in DHCPOFFER and DHCPREQUEST messages, and may
//    optionally be included in the DHCPACK and DHCPNAK messages.  DHCP
//    servers include this option in the DHCPOFFER in order to allow the
//    client to distinguish between lease offers.  DHCP clients use the
//    contents of the 'server identifier' field as the destination address
//    for any DHCP messages unicast to the DHCP server.  DHCP clients also
//    indicate which of several lease offers is being accepted by including
//    this option in a DHCPREQUEST message.
//
//    The identifier is the IP address of the selected server.
//
//    The code for this option is 54, and its length is 4.
//
//     Code   Len            Address
//    +-----+-----+-----+-----+-----+-----+
//    |  54 |  4  |  a1 |  a2 |  a3 |  a4 |
//    +-----+-----+-----+-----+-----+-----+
struct ServerIdentifier : public IP1Base<CODE_SERVER_IDENTIFIER> {
    ServerIdentifier() : IP1Base<CODE_SERVER_IDENTIFIER>() { }
    ServerIdentifier(IPv4_t ip) : IP1Base<CODE_SERVER_IDENTIFIER>(ip) { }
};

//
// 9.8. Parameter Request List
//
//    This option is used by a DHCP client to request values for specified
//    configuration parameters.  The list of requested parameters is
//    specified as n octets, where each octet is a valid DHCP option code
//    as defined in this document.
//
//    The client MAY list the options in order of preference.  The DHCP
//    server is not required to return the options in the requested order,
//    but MUST try to insert the requested options in the order requested
//    by the client.
//
//    The code for this option is 55.  Its minimum length is 1.
//
//     Code   Len   Option Codes
//    +-----+-----+-----+-----+---
//    |  55 |  n  |  c1 |  c2 | ...
//    +-----+-----+-----+-----+---
struct ParameterRequestList : public Base<55, 255>
{
    ParameterRequestList() {
        size = 0;
        data_[1] = size;
    }

    bool add(Code c) {
        if (size + 2 >= 255) {
            return false;
        }

        data_[size + 2] = c;
        ++size;
        data_[1] = size;

        return true;
    }

    u8 length() const {
        return size + 2;
    }

private:
    unsigned size;
};

// 9.9. Message
//
//    This option is used by a DHCP server to provide an error message to a
//    DHCP client in a DHCPNAK message in the event of a failure. A client
//    may use this option in a DHCPDECLINE message to indicate the why the
//    client declined the offered parameters.  The message consists of n
//    octets of NVT ASCII text, which the client may display on an
//    available output device.
//
//    The code for this option is 56 and its minimum length is 1.
//
//     Code   Len     Text
//    +-----+-----+-----+-----+---
//    |  56 |  n  |  c1 |  c2 | ...
//    +-----+-----+-----+-----+---
//
// 9.10. Maximum DHCP Message Size
//
//    This option specifies the maximum length DHCP message that it is
//    willing to accept.  The length is specified as an unsigned 16-bit
//    integer.  A client may use the maximum DHCP message size option in
//    DHCPDISCOVER or DHCPREQUEST messages, but should not use the option
//    in DHCPDECLINE messages.
//
//    The code for this option is 57, and its length is 2.  The minimum
//    legal value is 576 octets.
//
//     Code   Len     Length
//    +-----+-----+-----+-----+
//    |  57 |  2  |  l1 |  l2 |
//    +-----+-----+-----+-----+
struct MaximumDHCPMessageSize :
    public Uint16Base<CODE_MAXIMUM_DHCP_MESSAGE_SIZE>
{
    MaximumDHCPMessageSize (u16 v) :
        Uint16Base<CODE_MAXIMUM_DHCP_MESSAGE_SIZE>(v) { }
    MaximumDHCPMessageSize () :
        Uint16Base<CODE_MAXIMUM_DHCP_MESSAGE_SIZE>() { }
};

// 9.11. Renewal (T1) Time Value
//
//    This option specifies the time interval from address assignment until
//    the client transitions to the RENEWING state.
//
//    The value is in units of seconds, and is specified as a 32-bit
//    unsigned integer.
//
//    The code for this option is 58, and its length is 4.
//
//     Code   Len         T1 Interval
//    +-----+-----+-----+-----+-----+-----+
//    |  58 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
struct RenewalTimeValue :
    public Uint32Base<CODE_RENEWAL_TIME_VALUE> { };

// 9.12. Rebinding (T2) Time Value
//
//    This option specifies the time interval from address assignment until
//    the client transitions to the REBINDING state.
//
//    The value is in units of seconds, and is specified as a 32-bit
//    unsigned integer.
//
//    The code for this option is 59, and its length is 4.
//
//     Code   Len         T2 Interval
//    +-----+-----+-----+-----+-----+-----+
//    |  59 |  4  |  t1 |  t2 |  t3 |  t4 |
//    +-----+-----+-----+-----+-----+-----+
struct RebindingTimeValue :
    public Uint32Base<CODE_REBINDING_TIME_VALUE> { };

// 9.13. Vendor class identifier
//
//    This option is used by DHCP clients to optionally identify the vendor
//    type and configuration of a DHCP client.  The information is a string
//    of n octets, interpreted by servers.  Vendors may choose to define
//    specific vendor class identifiers to convey particular configuration
//    or other identification information about a client.  For example, the
//    identifier may encode the client's hardware configuration.  Servers
//    not equipped to interpret the class-specific information sent by a
//    client MUST ignore it (although it may be reported). Servers that
//
//    respond SHOULD only use option 43 to return the vendor-specific
//    information to the client.
//
//    The code for this option is 60, and its minimum length is 1.
//
//    Code   Len   Vendor class Identifier
//    +-----+-----+-----+-----+---
//    |  60 |  n  |  i1 |  i2 | ...
//    +-----+-----+-----+-----+---
//
// 9.14. Client-identifier
//
//    This option is used by DHCP clients to specify their unique
//    identifier.  DHCP servers use this value to index their database of
//    address bindings.  This value is expected to be unique for all
//    clients in an administrative domain.
//
//    Identifiers SHOULD be treated as opaque objects by DHCP servers.
//
//    The client identifier MAY consist of type-value pairs similar to the
//    'htype'/'chaddr' fields defined in [3]. For instance, it MAY consist
//    of a hardware type and hardware address. In this case the type field
//    SHOULD be one of the ARP hardware types defined in STD2 [22].  A
//    hardware type of 0 (zero) should be used when the value field
//    contains an identifier other than a hardware address (e.g. a fully
//    qualified domain name).
//
//    For correct identification of clients, each client's client-
//    identifier MUST be unique among the client-identifiers used on the
//    subnet to which the client is attached.  Vendors and system
//    administrators are responsible for choosing client-identifiers that
//    meet this requirement for uniqueness.
//
//    The code for this option is 61, and its minimum length is 2.
//
//    Code   Len   Type  Client-Identifier
//    +-----+-----+-----+-----+-----+---
//    |  61 |  n  |  t1 |  i1 |  i2 | ...
//    +-----+-----+-----+-----+-----+---
struct ClientIdentifier: public StringBase<CODE_CLIENT_IDENTIFIER> { };

}; // namespace Option

//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//0  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
//4  +---------------+---------------+---------------+---------------+
//   |                            xid (4)                            |
//8  +-------------------------------+-------------------------------+
//   |           secs (2)            |           flags (2)           |
//12 +-------------------------------+-------------------------------+
//   |                          ciaddr  (4)                          |
//16 +---------------------------------------------------------------+
//   |                          yiaddr  (4)                          |
//20 +---------------------------------------------------------------+
//   |                          siaddr  (4)                          |
//24 +---------------------------------------------------------------+
//   |                          giaddr  (4)                          |
//28 +---------------------------------------------------------------+
//   |                                                               |
//   |                          chaddr  (16)                         |
//   |                                                               |
//   |                                                               |
//44 +---------------------------------------------------------------+
//   |                                                               |
//   |                          sname   (64)                         |
//108+---------------------------------------------------------------+
//   |                                                               |
//   |                          file    (128)                        |
//236+---------------------------------------------------------------+
//   |                                                               |
//   |                          options (variable)                   |
//   +---------------------------------------------------------------+
//
//    options     var  Optional parameters field.  See the options
//                     documents for a list of defined options.
template<typename LL>
struct DhcpFrame
{
    typedef typename Meta::add_const<LL>::type const_ll_t;
    typedef typename LL::iterator iterator;
    typedef typename LL::const_iterator const_iterator;

    enum Offsets {
        OFFSET_OP = 0,
        OFFSET_HTYPE = 1,
        OFFSET_HLEN = 2,
        OFFSET_HOPS = 3,
        OFFSET_XID = 4,
        OFFSET_SECS = 8,
        OFFSET_FLAGS = 10,
        OFFSET_CIADDR = 12,
        OFFSET_YIADDR = 16,
        OFFSET_SIADDR = 20,
        OFFSET_GIADDR = 24,
        OFFSET_CHADDR = 28,
        OFFSET_SNAME = 44,
        OFFSET_FILE = 108,
        OFFSET_MAGIC = 236,
        OFFSET_OPTION_BEGIN = 240,
    };

    DhcpFrame(LL & l) : lower_layer_(l) {
        option_write_ptr = OFFSET_OPTION_BEGIN;
    }

private:
    u8 & hdr(unsigned idx) { return *(lower_layer().payload() + idx); }
    const u8 & hdr(unsigned idx) const { return *(lower_layer().payload() + idx); }
    iterator hdrptr(unsigned idx) { return lower_layer().payload() + idx; }
    const_iterator hdrptr(unsigned idx) const { return lower_layer().payload() + idx; }

public:
    //Message op code / message type.  1 = BOOTREQUEST, 2 = BOOTREPLY
    Op op() const { return (Op)hdr(OFFSET_OP); }
    void op(unsigned char o) { hdr(OFFSET_OP) = o; }

    // Hardware address type, see ARP section in "Assigned Numbers"
    // RFC; e.g., '1' = 10mb ethernet.
    unsigned char htype() const { return hdr(OFFSET_HTYPE); }
    void htype(unsigned char t) { hdr(OFFSET_HTYPE) = t; }

    // Hardware address length (e.g.  '6' for 10mb ethernet).
    unsigned char hlen() const { return hdr(OFFSET_HLEN); }
    void hlen(unsigned char l) { hdr(OFFSET_HLEN) = l; }

    // Client sets to zero, optionally used by relay agents when
    // booting via a relay agent.
    unsigned char hops() const { return hdr(OFFSET_HOPS); }
    void hops(unsigned char h) { hdr(OFFSET_HOPS) = h; }

    // Transaction ID, a random number chosen by the client, used by
    // the client and server to associate messages and responses
    // between a client and a server.
    u32 xid() const { return read_uint32(hdrptr(OFFSET_XID)); }
    void xid(u32 x) { write_uint32(x, hdrptr(OFFSET_XID)); }

    // Filled in by client, seconds elapsed since client began address
    // acquisition or renewal process.
    u16 secs() const { return read_uint32(hdrptr(OFFSET_SECS)); }
    void secs(u16 s) { write_uint32(s, hdrptr(OFFSET_SECS)); }

    //  Flags (see figure 2).
    u16 flags() const { return read_uint16(hdrptr(OFFSET_FLAGS)); }
    void flags(u16 f) { write_uint16(f, hdrptr(OFFSET_FLAGS)); }

    // Client IP address; only filled in if client is in BOUND, RENEW or
    // REBINDING state and can respond to ARP requests.
    IPv4_t ciaddr() const { return IPv4_t(read_uint32(hdrptr(OFFSET_CIADDR))); }
    void ciaddr(IPv4_t i) { write_uint32(i.as_int(), hdrptr(OFFSET_CIADDR)); }

    // 'your' (client) IP address.
    IPv4_t yiaddr() const { return IPv4_t(read_uint32(hdrptr(OFFSET_YIADDR))); }
    void yiaddr(IPv4_t i) { write_uint32(i.as_int(), hdrptr(OFFSET_YIADDR)); }

    // IP address of next server to use in bootstrap; returned in
    // DHCPOFFER, DHCPACK by server.
    IPv4_t siaddr() const { return IPv4_t(read_uint32(hdrptr(OFFSET_SIADDR))); }
    void siaddr(IPv4_t i) { write_uint32(i.as_int(), hdrptr(OFFSET_SIADDR)); }

    // Relay agent IP address, used in booting via a relay agent.
    IPv4_t giaddr() const { return IPv4_t(read_uint32(hdrptr(OFFSET_GIADDR))); }
    void giaddr(IPv4_t i) { write_uint32(i.as_int(), hdrptr(OFFSET_GIADDR)); }

    // Client hardware address.
    Mac_t chaddr() const { return Mac_t(hdrptr(OFFSET_CHADDR)); }
    void chaddr(const Mac_t & m) { m.copy_to(hdrptr(OFFSET_CHADDR)); }

    // Optional server host name, null terminated string.
    SName_t sname() const { return SName_t(hdrptr(OFFSET_SNAME)); }
    void sname(const SName_t & s) { s.copy_to(hdrptr(OFFSET_SNAME)); }

    // Boot file name, null terminated string; "generic" name or null
    // in DHCPDISCOVER, fully qualified directory-path name in
    // DHCPOFFER.
    File_t file() const { return File_t(hdrptr(OFFSET_FILE)); }
    void file(const File_t & f) { f.copy_to(hdrptr(OFFSET_FILE)); }

    u32 magic() const { return read_uint32(hdrptr(OFFSET_MAGIC)); }
    void magic(u32 i) { write_uint32(i, hdrptr(OFFSET_MAGIC)); }

    bool check() const {
        u8 len_;
        unsigned offset_;

        if (lower_layer().payload_length() < OFFSET_OPTION_BEGIN) {
            T_D("Payload too short");
            return false;
        }

        if (magic() != MAGIC) {
            T_D("Wrong magic: %x", magic());
            return false;
        }

        if (!option_find(Option::CODE_END, &len_, &offset_)) {
            T_D("No end option");
            return false;
        }

        return true;
    }

    unsigned size() const {
        return option_write_ptr;
    }

    void build_req_frame(const Mac_t& m, const u32 x, const u16 s) {
        op(OP_BOOTREQUEST);
        htype(HTYPE_10MB);
        magic(MAGIC);
        chaddr(m);
        secs(s);
        hlen(6);
        xid(x);
    }

    // TODO, offload to global function
    template<typename OPTION>
    bool add_option(const OPTION& o) {
        VTSS_ASSERT(option_write_ptr < lower_layer().max_size());
        VTSS_ASSERT(option_write_ptr >= DhcpFrame::OFFSET_OPTION_BEGIN);

        // insert padding to align
        while (option_write_ptr % 4 &&
               option_write_ptr < lower_layer().max_size()) {
            hdr(option_write_ptr++) = Option::CODE_PAD;
        }

        unsigned i = 0;
        bool overflow = false;
        unsigned old_ptr = option_write_ptr;
        const u8 * p = o.buf();

        while (i < o.length()) {
            if (option_write_ptr >= lower_layer().max_size()) {
                T_D("Frame overflow");
                overflow = true;
                break;
            }

            hdr(option_write_ptr++) = *p++;
            ++i;
        }

        if (overflow) {
            option_write_ptr = old_ptr;
            return false;
        }

        return true;
    }

    bool option_next(unsigned offset, u8 *code, u8 *len,
                     unsigned *next_offset) const
    {
        T_N("offset = %u, payload_length = %u", offset,
            lower_layer().payload_length());
        if (offset < DhcpFrame::OFFSET_OPTION_BEGIN ||
            offset > lower_layer().payload_length()) {
            T_D("err");
            return false;
        }

        *code = hdr(offset);
        *len = 0;

        switch (*code) {
        case Option::CODE_PAD:
            *next_offset = offset + 1;
            return true;

        case Option::CODE_END:
            return true;

        default:
            // check that length field is valid
            if (offset + 1 >= lower_layer().payload_length()) {
                T_D("err %u", offset + 1);
                return false;
            }

            // check that the message is complete
            if ((unsigned)hdr(offset + 1) + 2 >= lower_layer().payload_length()) {
                T_D("err %u", (unsigned)hdr(offset + 1) + 2);
                return false;
            }

            // safe length
            *len = hdr(offset + 1);

            // jump length of payload + code and length field
            *next_offset = offset + (*len) + 2;
            return true;
        }
    }

    bool option_find(u8 code, u8 *len, unsigned *offset) const {
        u8 code_, len_;
        unsigned n = 0;
        unsigned i = DhcpFrame::OFFSET_OPTION_BEGIN;

        //T_D("looking for option %d", (int) code);
        while (option_next(i, &code_, &len_, &n)) {
            T_N("  got %d", (int) code_);
            if (code == code_) {
                *len = len_;
                *offset = i;
                return true;
            }

            if (code_ == Option::CODE_END) {
                break;
            }

            i = n;
        }

        return false;
    }

    template<typename OPTION>
    bool option_get(OPTION& o) const {
        unsigned offset;
        u8 len;

        VTSS_ASSERT(o.code() != Option::CODE_PAD);
        VTSS_ASSERT(o.code() != Option::CODE_END);

        if (!option_find(o.code(), &len, &offset)) {
            return false;
        }

        o.copy_from(hdrptr(offset), ((int)len) + 2);
        return true;
    }

    int option_copy(u8 code, u8 *buf, unsigned buf_length,
                    unsigned offset) const
    {
        unsigned cnt = 0;
        unsigned offset_, op_length;
        u8 len8;

        VTSS_ASSERT(code != Option::CODE_PAD);
        VTSS_ASSERT(code != Option::CODE_END);

        if (!option_find(code, &len8, &offset_)) {
            //T_D("Failed to find option %u", code);
            return -1;
        }

        op_length = (unsigned)len8 + 2;

        //T_D("Found option %u, op_length %u, offset %u, buf_length %u",
        //        code, op_length, offset, buf_length);
        for (unsigned i = offset; i < op_length && cnt < buf_length;
             ++i, ++cnt) {
            *buf ++ = hdr(offset + offset_ + cnt);
        }

        return (int)cnt;
    }

    void update_deep() {
        lower_layer_.payload_length(option_write_ptr + 1);
        lower_layer_.update_deep();
    }

    LL & lower_layer() { return lower_layer_; }
    const_ll_t& lower_layer() const { return lower_layer_; }

private:
    unsigned option_write_ptr;
    LL & lower_layer_;
};

#ifdef __linux
template<typename T>
std::ostream& operator<<(std::ostream& o, const DhcpFrame<T>& f) {
    using namespace std;

    Option::MessageType message_type;

    o << std::dec << "DHCP> op:" << (int)f.op() << " htype:" <<
        (int)f.htype() << " hlen:" << f.hlen() << " hops:" << f.hops()
        << " xid:" << f.xid() << " secs:" << f.secs() << " flags:" <<
        f.flags() << " ciaddr:" << f.ciaddr() << " yiaddr:" <<
        f.yiaddr() << " siaddr:" << f.siaddr() << " giaddr:" <<
        f.giaddr() << " chaddr:" << f.chaddr() << " sname:" <<
        f.sname() << " file:" << f.file() << std::hex << " magic:0x" <<
        f.magic() << std::dec << " size:" << f.size();

    if (f.get_option(message_type)) {
        o << " Message type: " << (int) message_type.message_type();
    } else {
        o << " Message type: <NOT PRESENT>";
    }

    return o;
}
#endif

} // namespace DHCP
} // namespace VTSS

#endif //__DHCP_FRAME_HXX__

