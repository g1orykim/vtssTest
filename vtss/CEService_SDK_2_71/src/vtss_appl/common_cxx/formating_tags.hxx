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
#ifndef __FORMATING_TAGS_HXX__
#define __FORMATING_TAGS_HXX__

#include "common.hxx"

namespace VTSS {

template<typename T0>
struct FormatHex {
    FormatHex(T0& _t, char _b, uint32_t _min = 0, uint32_t _max = 0,
              char _c = '0') : t0(_t), base(_b), width_min(_min),
              width_max(_max), fill_char(_c) { }

    T0& t0;
    char base;
    uint32_t width_min;
    uint32_t width_max;
    char fill_char;
};

struct AsBool {
    AsBool (uint8_t &t_) : t(t_) { }
    uint8_t &t;
};

struct AsIpv4 {
    AsIpv4 (uint32_t &t_) : t(t_) { }
    uint32_t &t;
};

struct BinaryLen {
    BinaryLen (uint8_t *b, uint32_t m, uint32_t &v)
            : buf(b), max_len(m), valid_len(v) { }
    uint8_t *buf;
    const uint32_t max_len;
    uint32_t &valid_len;
};

struct Binary {
    Binary (uint8_t *b, uint32_t m) : buf(b), max_len(m) { }
    uint8_t *buf;
    const uint32_t max_len;
};

template<typename T>
FormatHex<T> hex(T& t, char _c = '0') {
    return FormatHex<T>(t, 'a', 0, 0, _c);
}

template<typename T>
FormatHex<T> HEX(T& t, char _c = '0') {
    return FormatHex<T>(t, 'A', 0, 0, _c);
}

template<uint32_t width, typename T>
FormatHex<T> hex_fixed(T& t, char _c = '0') {
    return FormatHex<T>(t, 'a', width, width, _c);
}

template<uint32_t width, typename T>
FormatHex<T> HEX_fixed(T& t, char _c = '0') {
    return FormatHex<T>(t, 'A', width, width, _c);
}

template<uint32_t min, uint32_t max, typename T>
FormatHex<T> hex_range(T& t, char _c = '0') {
    return FormatHex<T>(t, 'a', min, max, _c);
}

template<uint32_t min, uint32_t max, typename T>
FormatHex<T> HEX_range(T& t, char _c = '0') {
    return FormatHex<T>(t, 'A', min, max, _c);
}

template<unsigned S, typename T>
struct FormatLeft {
    FormatLeft(const T& _t, char f) : t(_t), fill(f) { }
    const T& t;
    const char fill;
};

template<unsigned S, typename T0>
FormatLeft<S, T0> left(const T0& t0) { return FormatLeft<S, T0>(t0, ' '); }

template<unsigned S, typename T0>
struct FormatRight {
    FormatRight(const T0& _t, char f) : t0(_t), fill(f) { }
    const T0& t0;
    const char fill;
};

template<unsigned S, typename T0>
FormatRight<S, T0> right(const T0& t0) {
    return FormatRight<S, T0>(t0, ' ');
}

template<unsigned S, typename T0>
FormatRight<S, T0> fill(const T0& t0, char f = '0') {
    return FormatRight<S, T0>(t0, f);
}

}  // namespace VTSS

#endif
