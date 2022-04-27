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
#ifndef __FRAME_UTILS_HXX__
#define __FRAME_UTILS_HXX__

extern "C" {
#include "vtss_types.h"
#include <assert.h>
#include <string.h>
}

#ifdef __linux
#include <iomanip>
#include <iostream>
#include <arpa/inet.h> // for htonl...
#else
extern "C" {
#include <sys/cdefs.h> // to make sys/endian.h parse..
#include <sys/endian.h> // for htonl...
}
#endif

namespace VTSS {
struct Mac_t
{
    Mac_t () {}

    template<typename I>
    explicit Mac_t (I buf) {
        copy_from(buf);
    }

    template<typename Iter>
    void copy_from(Iter buf) {
        for (int i = 0; i < 6; ++i)
            data[i] = *buf++;
    }

    template<typename Iter>
    void copy_to(Iter buf) const {
        for (int i = 0; i < 6; ++i)
            *buf++ = data[i];
    }

    u8 data[6];
};

#ifdef __linux
std::ostream& operator<< (std::ostream& o, const Mac_t & m)
{
    using namespace std;
    o << hex << setw(2) << setfill('0') << (int)m.data[0] << ":" <<
         hex << setw(2) << setfill('0') << (int)m.data[1] << ":" <<
         hex << setw(2) << setfill('0') << (int)m.data[2] << ":" <<
         hex << setw(2) << setfill('0') << (int)m.data[3] << ":" <<
         hex << setw(2) << setfill('0') << (int)m.data[4] << ":" <<
         hex << setw(2) << setfill('0') << (int)m.data[5];
    return o;
}
#endif

u16 inet_chksum(u32 sum, const u16 * buf, int length)
{
    while (length > 1) {
        //std::cout << std::setfill('0') << std::setw(4) << std::hex << *buf << std::endl;
        sum += *buf++;
        length -= 2;
    }

    if (length == 1) {
        u16 tmp = *(u8*)buf;
#ifdef __BIG_ENDIAN__
        tmp <<= 8;
#endif
        //std::cout << std::setfill('0') << std::setw(4) << std::hex << tmp << std::endl;
        sum += tmp;
    }
    //std::cout << std::endl;

    sum = ~((sum >> 16) + (sum & 0xffff));
    sum &= 0xffff;
    //std::cout << "sum is: " << std::setfill('0') << std::setw(4) << std::hex << sum << std::endl;
    return htons(sum);
}

template<typename I>
u16 read_uint16(I buf)
{
    u16 tmp = 0;
    tmp += *buf++; tmp <<= 8;
    tmp += *buf++;
    return tmp;
}

template<typename I>
u32 read_uint32(I buf)
{
    u32 tmp = 0;
    tmp |= *buf++; tmp <<= 8;
    tmp |= *buf++; tmp <<= 8;
    tmp |= *buf++; tmp <<= 8;
    tmp |= *buf++;
    return tmp;
}

template<typename I>
void write_uint16(u16 v, I buf)
{
    u8 b = v & 0xff; v >>= 8;
    u8 a = v & 0xff;
    *buf++ = a; *buf++ = b;
}

template<typename I>
void write_uint32(u32 v, I buf)
{
    u8 a = v & 0xff; v >>= 8;
    u8 b = v & 0xff; v >>= 8;
    u8 c = v & 0xff; v >>= 8;
    u8 d = v & 0xff;
    *buf++ = d; *buf++ = c; *buf++ = b; *buf++ = a;
}

struct IPv4_t
{
    IPv4_t() { }
    explicit IPv4_t(u32 i) : data_(i) { }
    IPv4_t(const IPv4_t& i) : data_(i.as_int()) { }
    u32 as_int() const { return data_; }
    void as_int(u32 i) { data_ = i; }
private:
    u32 data_;
};

#ifdef __linux
std::ostream& operator<<(std::ostream& o, const IPv4_t& i)
{
    u32 ip = i.as_int();
    u32 d = ip & 0xff; ip >>= 8;
    u32 c = ip & 0xff; ip >>= 8;
    u32 b = ip & 0xff; ip >>= 8;
    u32 a = ip & 0xff;
    o << std::dec << a << "." << b << "." << c << "." << d;
    return o;
}
#endif

template<typename Type>
class SafePointerIter
{
public:
    SafePointerIter() : ptr_(0), begin_(0), end_(0) { }
    SafePointerIter(Type * p, Type * b, Type * e) :
        ptr_(p), begin_(b), end_(e) { }
    SafePointerIter(const SafePointerIter& i) :
        ptr_(i.ptr_), begin_(i.begin_), end_(i.end_) { }

    SafePointerIter operator++(int) {
        Type * old = ptr_;
        ++ptr_;
        return SafePointerIter(old, begin_, end_);
    }
    SafePointerIter operator--(int) {
        Type * old = ptr_;
        --ptr_;
        return SafePointerIter(old, begin_, end_);
    }
    Type& operator*() { VTSS_ASSERT(ptr_ >= begin_ && ptr_ < end_); return *ptr_; }
    SafePointerIter operator+(int i) { return SafePointerIter(ptr_+i, begin_, end_); }
    SafePointerIter operator-(int i) { return SafePointerIter(ptr_-i, begin_, end_); }
    SafePointerIter& operator++() { ++ptr_; return *this; }
    SafePointerIter& operator--() { --ptr_; return *this; }
    SafePointerIter& operator+=(ptrdiff_t d) { ++ptr_ += d; return *this; }
    SafePointerIter& operator-=(ptrdiff_t d) { ++ptr_ -= d; return *this; }
    bool operator==(const SafePointerIter& rhs) { return ptr_ == rhs.ptr_; }
    bool operator!=(const SafePointerIter& rhs) { return ptr_ != rhs.ptr_; }
    bool operator<=(const SafePointerIter& rhs) { return ptr_ <= rhs.ptr_; }
    bool operator>=(const SafePointerIter& rhs) { return ptr_ >= rhs.ptr_; }
    bool operator<(const SafePointerIter& rhs) { return ptr_ < rhs.ptr_; }
    bool operator>(const SafePointerIter& rhs) { return ptr_ > rhs.ptr_; }

private:
    Type * ptr_; Type * begin_; Type * end_;
};

struct FrameRef
{
    typedef SafePointerIter<const u8> const_iterator;
    FrameRef(const u8 * data, unsigned length) : valid(length), buf(data) { }
    unsigned header_size() const { return 0; }
    unsigned max_size() const { return valid; }
    unsigned payload_length() const { return valid; }
    const_iterator payload() const { return payload_begin(); }
    const_iterator payload_begin() const { return const_iterator(buf, buf, buf+valid); }
    const_iterator payload_end() const { return const_iterator(buf+valid, buf, buf+valid); }
    const_iterator payload_valid_end() const { return const_iterator(buf+valid, buf, buf+valid); }

private:
    const unsigned valid;
    const u8 * buf;
};

template<unsigned MaxSize = 1500>
struct Frame
{
    typedef SafePointerIter<u8> iterator;
    typedef SafePointerIter<const u8> const_iterator;

    Frame() : valid(0) {
        clear();
    }

    Frame(const Frame& f) {
        memcpy(buf, f.buf, MaxSize);
        valid = f.payload_length();
    }

    Frame& operator=(const Frame& f) {
        memcpy(buf, f.buf, MaxSize);
        valid = f.payload_length();
        return *this;
    }

    bool operator!=(const Frame<MaxSize>& rhs) {
        if (valid != rhs.payload_length()) {
            return true;
        }

        if (memcmp(buf, rhs.buf, valid) != 0 ) {
            return true;
        }

        return false;
    }

    unsigned max_size() const {
        return MaxSize;
    }

    unsigned payload_length() const {
        return valid;
    }

    void payload_length(unsigned l) {
        VTSS_ASSERT(l <= MaxSize);
        valid = l;
    }

    iterator payload() {
        return payload_begin();
    }

    iterator payload_begin() {
        return iterator(buf, buf, buf+MaxSize);
    }

    iterator payload_end() {
        return iterator(buf+MaxSize, buf, buf+MaxSize);
    }

    iterator payload_valid_end() {
        return iterator(buf+valid, buf, buf+valid);
    }

    const_iterator payload() const {
        return payload_begin();
    }

    const_iterator payload_begin() const {
        return const_iterator(buf, buf, buf+MaxSize);
    }

    const_iterator payload_end() const {
        return const_iterator(buf+MaxSize, buf, buf+MaxSize);
    }

    const_iterator payload_valid_end() const {
        return const_iterator(buf+valid, buf, buf+valid);
    }

    void clear() {
        memset(buf, 0, MaxSize);
        valid = 0;
    }

    void update_deep() {
        // hack
    }

    void etype(u16 t) {
        // hack
    }

    unsigned header_size() const {
        return 0;
    }

private:
    unsigned valid;
    u8 buf[MaxSize];
};

#ifdef __linux
template<unsigned MaxSize>
std::ostream& operator<<(std::ostream& o, const Frame<MaxSize>& f) {
    typedef typename Frame<MaxSize>::const_iterator I;
    I it;
    ptrdiff_t i;

    for (it = f.payload_begin(), i = 0; it != f.payload_valid_end();) {
        o << std::hex << std::setw(6) << std::setfill('0') << i <<  ": ";
        for (unsigned j = 0; j < 16 && it != f.payload_valid_end(); ++i, ++j, ++it) {
            if (j == 8) {
                o << " ";
            }
            o << std::hex << std::setw(2) << std::setfill('0') << (u32) *it <<  " ";
        }
        o << std::endl;
    }
    return o;
}
#endif

template <typename T>
struct StackableFrameAdaptor
{
    typedef SafePointerIter<u8> iterator;
    typedef SafePointerIter<const u8> const_iterator;

    unsigned max_size() const {
        const T* _this = static_cast<const T*>(this);
        return _this->lower_layer().max_size() - _this->lower_layer().header_size();
    }

    iterator payload() {
        return payload_begin();
    }

    iterator payload_begin() {
        T* _this = static_cast<T*>(this);
        return (_this->lower_layer().payload_begin()) + _this->header_size();
    }

    iterator payload_end() {
        T* _this = static_cast<T*>(this);
        return (_this->lower_layer().payload_end());
    }

    iterator payload_valid_end() {
        T* _this = static_cast<T*>(this);
        return (_this->payload_begin() +
                _this->header_size() +
                _this->payload_length());
    }

    const_iterator payload() const {
        return payload_begin();
    }

    const_iterator payload_begin() const {
        const T* _this = static_cast<const T*>(this);
        return _this->lower_layer().payload_begin() + _this->header_size();
    }

    const_iterator payload_end() const {
        const T* _this = static_cast<const T*>(this);
        return (_this->lower_layer().payload_end());
    }

    const_iterator payload_valid_end() const {
        const T* _this = static_cast<const T*>(this);
        return (_this->payload_begin() +
                _this->header_size() +
                _this->payload_length());
    }
};


//template<class LL> struct Ethernet;

template<class LL>
struct EthernetFrame : public StackableFrameAdaptor<EthernetFrame<LL> >
{
    typedef typename Meta::add_const<LL>::type const_ll_t;
    EthernetFrame(LL & ll) : lower_layer_(ll) { }

    void update_deep() { }
    unsigned header_size() const { return 14; }
    unsigned payload_length() const {return lower_layer_.payload_length() - header_size(); }
    void payload_length(unsigned l) { lower_layer_.payload_length(l + header_size()); }
    unsigned total_length() const { return lower_layer_.payload_length(); }

    Mac_t dst() const { return Mac_t(lower_layer_.payload()); }
    Mac_t src() const { return Mac_t(lower_layer_.payload() + 6); }
    u16 etype() const { return read_uint16(lower_layer_.payload() + 12); }

    void dst(const Mac_t & m) { m.copy_to(lower_layer_.payload()); }
    void src(const Mac_t & m) { m.copy_to(lower_layer_.payload() + 6); }
    void etype(u16 t) { write_uint16(t, lower_layer_.payload() + 12); }

    LL & lower_layer() { return lower_layer_; }
    const_ll_t & lower_layer() const { return lower_layer_; }

private:
    LL & lower_layer_;
};

#ifdef __linux
template<typename T>
std::ostream& operator<<(std::ostream& o, const EthernetFrame<T>& e) {
    using namespace std;
    o << "dst:" << e.dst() << " src:" << e.src() << " type:0x" <<
        hex << setw(4) << setfill('0') << e.etype() << " lenght:" <<
        dec << e.payload_length();
    return o;
}
#endif

template<class LL>
struct IpFrame : public StackableFrameAdaptor<IpFrame<LL> >
{
    typedef typename Meta::add_const<LL>::type const_ll_t;
    typedef StackableFrameAdaptor<IpFrame<LL> > BASE;
    typedef typename StackableFrameAdaptor<IpFrame<LL> >::iterator iterator;
    typedef typename StackableFrameAdaptor<IpFrame<LL> >::const_iterator const_iterator;

    IpFrame(LL & ll) : lower_layer_(ll) { }
    unsigned header_size() const {
        unsigned len = ihl() * 4;
        if (len < 20) {
            return 20;
        }
        return len;
    }

    unsigned payload_length() const { return total_length() - header_size(); }
    void payload_length(unsigned l) { write_uint16(l + header_size(), lower_layer_.payload() + 2); }
    unsigned total_length() const {
        unsigned len = read_uint16(lower_layer_.payload() + 2);
        if (len < header_size()) {
            return header_size();
        }
        return len;
    }

    // no fragmenting, flags or options
    bool is_simple() const {
        return ((*(lower_layer_.payload() + 6)) == 0 &&
                (*(lower_layer_.payload() + 7)) == 0 &&
                ihl() == 5);
    }

    bool check() const {
        if (BASE::max_size() < 20) {
            return false;
        }

        if (BASE::max_size() < header_size()) {
            return false;
        }

        // we might need to relax this one...
        if (total_length() != lower_layer_.payload_length()) {
            return false;
        }

        if (calc_chksum() != 0) {
            return false;
        }

        return true;
    }

    u8 version() const { return ((*lower_layer_.payload()) & 0xf0) >> 4; }
    void version(u8 v) { (*lower_layer_.payload()) &= 0x0f; (*lower_layer_.payload()) |= ((v & 0x0f) << 4); }

    u8 ihl() const { return ((*lower_layer_.payload()) & 0x0f); }
    void ihl(u8 l) { (*lower_layer_.payload()) &= 0xf0; (*lower_layer_.payload()) |= (l & 0xf); }

    u8 ttl() const { return *(lower_layer_.payload() + 8); }
    void ttl(u8 p) { *(lower_layer_.payload() + 8) = p; }

    u8 protocol() const { return *(lower_layer_.payload() + 9); }
    void protocol(u8 p) { *(lower_layer_.payload() + 9) = p; }

    void checksum(u16 c) { write_uint16(c, lower_layer_.payload() + 10); }
    u16 checksum() const { return read_uint16(lower_layer_.payload() + 10); }

    IPv4_t src() const { return IPv4_t(read_uint32(lower_layer_.payload() + 12)); }
    void src(IPv4_t i) { write_uint32(i.as_int(), lower_layer_.payload() + 12); }

    IPv4_t dst() const { return IPv4_t(read_uint32(lower_layer_.payload() + 16)); }
    void dst(IPv4_t i) { write_uint32(i.as_int(), lower_layer_.payload() + 16); }

    u16 calc_chksum() const {
        VTSS_ASSERT(this->max_size() >= header_size());
        return inet_chksum(0, (const u16*)(&(*lower_layer_.payload())), header_size());
    }
    u32 udp_checksum_contribution() const {
        VTSS_ASSERT(this->max_size() >= header_size());
        u32 sum = 0;
        const u16 * p = ((u16*)(&(*lower_layer_.payload()))) + 6;
        sum += htons(total_length() - header_size());
        sum += *p++; sum += *p++; // src
        sum += *p++; sum += *p++; // dst
        sum += (protocol() << 8);
        return sum;
    }

    void set_defaults() {
        ihl(5);
        ttl(64);
        version(4);
        checksum(0);
        checksum(calc_chksum());
    }

    void update_deep() {
        //std::cout << "update deep" <<std::endl;
        lower_layer_.etype(0x0800);
        lower_layer_.payload_length(total_length());
        lower_layer_.update_deep();
    }

    LL & lower_layer() { return lower_layer_; }
    const_ll_t & lower_layer() const { return lower_layer_; }

private:
    LL & lower_layer_;
};

#ifdef __linux
template<typename T>
std::ostream& operator<<(std::ostream& o, const IpFrame<T>& i) {
    using namespace std;
    o << "IPv4> dst:" << i.dst() << " src:" << i.src() << " proto:" <<
        (int) i.protocol() << " lenght:" << i.payload_length();
    return o;
}
#endif

template<class LL>
struct UdpFrame : public StackableFrameAdaptor<UdpFrame<LL> >
{
    typedef typename Meta::add_const<LL>::type const_ll_t;
    typedef StackableFrameAdaptor<UdpFrame<LL> > BASE;
    typedef typename StackableFrameAdaptor<UdpFrame<LL> >::iterator iterator;
    typedef typename StackableFrameAdaptor<UdpFrame<LL> >::const_iterator const_iterator;

    UdpFrame(LL & ll) : lower_layer_(ll) { }

    unsigned header_size() const { return 8; }

    bool check() const {
        if (BASE::max_size() < header_size()) {
            return false;
        }

        // we might need to relax this one...
        if (total_length() != lower_layer_.payload_length()) {
            return false;
        }

        if (calc_chksum() != 0) {
            return false;
        }

        return true;
    }

    u16 payload_length() const { return total_length() - header_size(); }
    void payload_length(unsigned l) { write_uint16(l + header_size(), lower_layer_.payload() + 4); }
    u16 total_length() const {
        unsigned len = read_uint16(lower_layer_.payload() + 4);
        if (len < header_size()) {
            return header_size();
        }
        return len;
    }

    void src(u16 c) { write_uint16(c, lower_layer_.payload()); }
    unsigned src() const { return read_uint16(lower_layer_.payload()); }

    void dst(u16 c) { write_uint16(c, lower_layer_.payload() + 2); }
    unsigned dst() const { return read_uint16(lower_layer_.payload() + 2); }

    void checksum(u16 c) { write_uint16(c, lower_layer_.payload() + 6); }
    unsigned checksum() const { return read_uint16(lower_layer_.payload() + 6); }
    u16 calc_chksum() const {
        VTSS_ASSERT(BASE::max_size() >= total_length());
        return inet_chksum(lower_layer_.udp_checksum_contribution(),
                           (u16*)(&(*lower_layer_.payload())),
                           total_length());
    }

    void set_defaults() {
        checksum(calc_chksum());
    }

    void update_deep() {
        lower_layer_.protocol(17);
        lower_layer_.payload_length(total_length());
        lower_layer_.update_deep();
    }

    LL & lower_layer() { return lower_layer_; }
    const_ll_t & lower_layer() const { return lower_layer_; }

private:
    LL & lower_layer_;

};

#ifdef __linux
template<typename T>
std::ostream& operator<<(std::ostream& o, const UdpFrame<T>& i) {
    using namespace std;
    o << "UDP> src:" << i.src() << " dst:" << i.dst() << " lenght:" <<
        i.payload_length();
    return o;
}
#endif

template<typename L0,
         template<class LL1> class L1>
struct Stack1
{
    typedef L0 _L0;      _L0 buf;
    typedef L1<_L0> _L1; _L1 l1;

    Stack1() : buf(), l1(buf) { buf.clear(); }
    Stack1(const Stack1& s) : buf(s.buf), l1(buf) { }
    Stack1& operator=(const Stack1& s) { buf = s.buf; return *this; }
    bool operator!=(const Stack1& rhs) {
        return buf != rhs.buf;
    }

    template<typename LL>
    Stack1(const L1<LL>& s) : buf(), l1(buf) {
        // TODO, make a compile time error if we try to copy a larger
        // frame to a smaler
        typedef typename L1<LL>::const_iterator I;
        typedef typename _L1::iterator J;
        I i = s.lower_layer().payload_begin();
        J j = l1.lower_layer().payload_begin();

        for (; i != s.lower_layer().payload_end(); ++i) {
            *j++ = *i;
        }

        buf.payload_length(s.lower_layer().payload_length());
    }

    template<typename LL>
    Stack1& operator=(const L1<LL>& s) {
        // TODO, make a compile time error if we try to copy a larger
        // frame to a smaler
        typedef typename L1<LL>::const_iterator I;
        typedef typename _L1::iterator J;
        I i = s.lower_layer().payload_begin();
        J j = l1.lower_layer().payload_begin();

        for (; i != s.lower_layer().payload_end(); ++i) {
            *j++ = *i;
        }

        buf.payload_length(s.lower_layer().payload_length());
        return *this;
    }

    _L1 * operator->() { return &l1; }
};

template<typename L0,
         template<class LL1> class L1,
         template<class LL2> class L2>
struct Stack2
{
    Stack2() : buf(), l1(buf), l2(l1) { buf.clear(); }
    Stack2(const Stack2& s) : buf(s.buf) { }
    Stack2& operator=(const Stack2& s) { buf = s.buf; return *this; }

    typedef L0 _L0;      _L0 buf;
    typedef L1<_L0> _L1; _L1 l1;
    typedef L2<_L1> _L2; _L2 l2;
    _L2 * operator->() { return &l2; }
};

template<typename L0,
         template<class LL1> class L1,
         template<class LL2> class L2,
         template<class LL3> class L3>
struct Stack3
{
    typedef L0 _L0;      _L0 buf;
    typedef L1<_L0> _L1; _L1 l1;
    typedef L2<_L1> _L2; _L2 l2;
    typedef L3<_L2> _L3; _L3 l3;

    Stack3() : buf(), l1(buf), l2(l1), l3(l2) { buf.clear(); }
    Stack3(const Stack3& s) : buf(s.buf) { }
    Stack3& operator=(const Stack3& s) { buf = s.buf; return *this; }

    _L3 * operator->() { return &l3; }

};

template<typename L0,
         template<class LL1> class L1,
         template<class LL2> class L2,
         template<class LL3> class L3,
         template<class LL4> class L4>
struct Stack4
{
    Stack4() : buf(), l1(buf), l2(l1), l3(l2), l4(l3) { buf.clear(); }
    Stack4(const Stack4& s) : buf(s.buf) { }
    Stack4& operator=(const Stack4& s) { buf = s.buf; return *this; }

    typedef L0 _L0;      _L0 buf;
    typedef L1<_L0> _L1; _L1 l1;
    typedef L2<_L1> _L2; _L2 l2;
    typedef L3<_L2> _L3; _L3 l3;
    typedef L4<_L3> _L4; _L4 l4;
    _L4 * operator->() { return &l4; }
};
} /* VTSS */
#endif // __FRAME_UTILS_HXX__
