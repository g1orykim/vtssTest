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

#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_
namespace VTSS {

template <class T0> const T0& max(const T0& a, const T0& b) {
    return (a < b) ? b : a;
}

template <class T0> const T0& min(const T0& a, const T0& b) {
    return (a < b)?a:b;
}

template <typename I, typename O>
inline O copy(I first, I last, O res) {
    for (; first != last; ++res, ++first)
        *res = *first;
    return res;
}

template <typename I, typename O>
inline O copy_backward(I first, I last, O res) {
    while (first != last)
        *--res = *--last;
    return res;
}

inline const char *find_end(const char * s) {
    if (s== 0)  return 0;
    if (*s== 0) return s;
    while (*(++s) != 0) continue;
    return s;
}

template<class I, class T0>
I find(I first, I last, const T0& value) {
    for (; first != last; ++first)
        if (*first == value) break;
    return first;
}

template<class I, class T0>
I find_not(I first, I last, const T0& value) {
    for (; first != last; ++first)
        if ( *first != value ) break;
    return first;
}

template<typename T0, typename P>
T0 find_first_if(T0 b, T0 e, P p) {
    for (; b != e; b++)
        if (p(*b)) break;
    return b;
}

template<typename T0, typename P>
T0 find_last_if(T0 b, T0 e, P p) {
    T0 i = b;
    for (; b != e; b++)
        if (p(*b)) i = b;
    return i;
}


template <typename I>
inline bool equal(I begin1, I end1, I begin2, I end2) {
    while (begin1 != end1 && begin2 != end2 && *begin1 == *begin2)
        ++begin1, ++begin2;
    return begin1 == end1 && begin2 == end2;
}

template<typename T0, typename B>
void signed_to_dec_rbuf(T0 val, B& buf, bool force_sign = false) {
    bool sign = false;

    if (val < 0)  // do not multiply by -1 here, it will cause overflow!
        sign = true;

    // First iteration is unroled to print 0, and to avoid overflow
    if (sign) {
        buf.push((static_cast<char>(val % 10) * -1) + '0');
        val /= 10;
        val *= -1;
    } else {
        buf.push(static_cast<char>(val % 10) + '0');
        val /= 10;
    }

    while (val) {
        buf.push(static_cast<char>(val % 10) + '0');
        val /= 10;
    }

    if (sign)            buf.push('-');
    else if (force_sign) buf.push('+');
}

template<typename T0, typename B>
void unsigned_to_dec_rbuf(T0 val, B& buf) {
    if (val == 0)
        buf.push('0');

    while (val) {
        buf.push(static_cast<char>(val % 10) + '0');
        val /= 10;
    }
}

template<typename T0, typename B>
void unsigned_to_hex_rbuf(T0 val, B& buf, char base) {
    if (val == 0)
        buf.push('0');

    while (val) {
        char v = val & 0xf;
        if (v > 9) buf.push((v - 10) + base);
        else       buf.push(v + '0');
        val >>= 4;
    }
}

template<class I, class S, class O>
O& join(O& o, const S& s, I begin, I end) {
    if (begin == end)
        return o;

    o << *begin;
    ++begin;

    for (; begin != end; ++begin)
        o << s << *begin;

    return o;
}

template<class I, class S, class O, class FMT>
O& join(O& o, const S& s, I begin, I end, FMT fmt) {
    if (begin == end)
        return o;

    o << fmt(*begin);
    ++begin;

    for (; begin != end; ++begin)
        o << s << fmt(*begin);

    return o;
}


}  // namespace VTSS
#endif /* _ALGORITHM_H_ */
