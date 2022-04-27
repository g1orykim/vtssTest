/*

 Vitesse API software.

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

#include "types.hxx"
#include "meta.hxx"

namespace VTSS {
// Basic scalar types /////////////////////////////////////////////////////////
//ostream& operator<< (ostream& o, const char c)
//{
//    o.push(c);
//    return o;
//}
//
//ostream& operator<< (ostream& o, const char * s)
//{
//    while( *s != 0 )
//        o << *s++;
//    return o;
//}
//
//ostream& operator<< (ostream& o, const str& s)
//{
//    copy(s.begin(), s.end(), obuffer_iterator<char>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, long i)
//{
//    BufStream<SBuf32> buf;
//    signed_to_dec_rbuf(i, buf, false);
//    copy_backward(buf.begin(), buf.end(), obuffer_iterator<char>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, int i)
//{
//    BufStream<SBuf16> buf;
//    signed_to_dec_rbuf(i, buf, false);
//    copy_backward(buf.begin(), buf.end(), obuffer_iterator<char>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, short i)
//{
//    BufStream<SBuf8> buf;
//    signed_to_dec_rbuf(i, buf, false);
//    copy_backward(buf.begin(), buf.end(), obuffer_iterator<char, ostream>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, unsigned long i)
//{
//    BufStream<SBuf32> buf;
//    unsigned_to_dec_rbuf(i, buf);
//    copy_backward(buf.begin(), buf.end(), obuffer_iterator<char, ostream>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, unsigned int i)
//{
//    BufStream<SBuf16> buf;
//    unsigned_to_dec_rbuf(i, buf);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char, ostream>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, unsigned short i)
//{
//    BufStream<SBuf8> buf;
//    unsigned_to_dec_rbuf(i, buf);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char, ostream>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, unsigned char i)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_dec_rbuf(i, buf);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char, ostream>(o));
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<long>& h)
//{
//    BufStream<SBuf8> buf;
//    unsigned_to_hex_rbuf((unsigned long)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<int>& h)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<short>& h)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<char>& h)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<unsigned long>& h)
//{
//    BufStream<SBuf8> buf;
//    unsigned_to_hex_rbuf((unsigned long)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<unsigned int>& h)
//{
//    BufStream<SBuf8> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<unsigned short>& h)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<unsigned char>& h)
//{
//    BufStream<SBuf4> buf;
//    unsigned_to_hex_rbuf((unsigned int)h.t0, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//
//    return o;
//}
// End of basic scalar types //////////////////////////////////////////////////

//// IPv4 ///////////////////////////////////////////////////////////////////////
//ostream& operator<< (ostream& o, const Ipv4& i) {
//    if( o.muted() ) return o;
//    o << ((i.raw >> 24) & 0xff) << '.' <<
//         ((i.raw >> 16) & 0xff) << '.' <<
//         ((i.raw >>  8) & 0xff) << '.' <<
//         ((i.raw      ) & 0xff);
//    return o;
//}
//
//ostream& operator<< (ostream& o, const FormatHex<Ipv4>& h)
//{
//    if( o.muted() ) return o;
//    BufStream<SBuf8> buf;
//    unsigned_to_hex_rbuf(h.t.raw, buf, h.base);
//    copy_backward (buf.begin(), buf.end(), obuffer_iterator<char>(o));
//    return o;
//}
//// End of Ipv4 ////////////////////////////////////////////////////////////////
//
//ostream& operator<< (ostream& o, const Ipv4Network& i)
//{
//    if( o.muted() ) return o;
//    o << i.address << "/" << i.prefix;
//    return o;
//}
//
//ostream& operator<< (ostream& o, const Ipv6& x)
//{
//    if( o.muted() ) return o;
//    uint16_t ip[8];
//    uint16_t * iter = ip, * end = ip + 8;
//    uint16_t * zero_begin = 0, * zero_end = 0;
//
//    // Copy to an array of shorts
//    for( int i = 0; i < 8; ++i )
//        ip[i] = x.addr[2*i] << 8 | x.addr[2*i + 1];
//
//    // Find the longest sequence of zero's
//    while( iter != end ) {
//        uint16_t * b = find(iter, end, 0);
//        uint16_t * e = find_not(b, end, 0);
//
//        if( b == end ) // no zero group found
//            break;
//
//        iter = e; // next search should start where this ended
//        if ((zero_end - zero_begin) < (e - b)) {
//            zero_end = e;
//            zero_begin = b;
//        }
//    }
//
//    // Print the IPv6 address
//    if( zero_begin == zero_end ) {
//        join(o, ":", ip, end, &hex<uint16_t>); // regular join-print
//
//    } else if( zero_begin == ip && zero_end == ip + 6 ) { // Print as ipv4
//        o << "::";
//        uint32_t _v4 = ip[6]; _v4 <<= 16; _v4 |= ip[7];
//        Ipv4 v4(_v4);
//        o << v4;
//
//    } else if( zero_begin == ip && zero_end == ip + 5 && ip[5] == 0xffff ) {
//        // Print as ipv4 (almost )
//        o << "::";
//        o << hex(ip[5]) << ":";
//        uint32_t _v4 = ip[6]; _v4 <<= 16; _v4 |= ip[7];
//        Ipv4 v4(_v4);
//        o << v4;
//
//    } else {
//        join(o, ":", ip, zero_begin, &hex<uint16_t>); // before zero-sequence
//        o << "::";
//        join(o, ":", zero_end, end, &hex<uint16_t>); // after zero-sequence
//
//    }
//
//    return o;
//}
//
//ostream& operator<< (ostream& o, const Ipv6Network& i)
//{
//    if( o.muted() ) return o;
//    o << i.address << "/" << i.prefix;
//    return o;
//}





} // namespace VTSS
