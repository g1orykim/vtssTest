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

#ifndef _META_H_
#define _META_H_

#include "common.hxx"

namespace VTSS {
namespace Meta {

struct _true { };
struct _false { };

template<typename C, typename A, typename B> struct _if;
template<typename A, typename B> struct _if<_true, A, B> { typedef A type; };
template<typename A, typename B> struct _if<_false, A, B> { typedef B type; };

template<typename T1, typename T2> struct AssertTypeEqual;
template<typename T1> struct AssertTypeEqual<T1, T1> { };

template<typename T1, typename T2> struct equal_type { typedef _false type; };
template<typename _T> struct equal_type<_T, _T> { typedef _true type; };

template<typename _T> struct remove_const { typedef _T type; };
template<typename _T> struct remove_const<const _T> { typedef _T type; };

template<typename _T> struct add_const { typedef const _T type; };
template<typename _T> struct add_const<const _T> { typedef const _T type; };

template<typename _T> struct IntTraits;

struct Signed { };
struct Unsigned { };

template<> struct IntTraits<int8_t> {
    typedef Signed SignType;
    typedef int8_t WithOutUnsigned;
    typedef uint8_t WithUnsigned;
};

template<> struct IntTraits<uint8_t> {
    typedef Unsigned SignType;
    typedef int8_t WithOutUnsigned;
    typedef uint8_t WithUnsigned;
};

template<> struct IntTraits<int16_t> {
    typedef Signed SignType;
    typedef int16_t WithOutUnsigned;
    typedef uint16_t WithUnsigned;
};

template<> struct IntTraits<uint16_t> {
    typedef Unsigned SignType;
    typedef int16_t WithOutUnsigned;
    typedef uint16_t WithUnsigned;
};

template<> struct IntTraits<int32_t> {
    typedef Signed SignType;
    typedef int32_t WithOutUnsigned;
    typedef uint32_t WithUnsigned;
};

template<> struct IntTraits<uint32_t> {
    typedef Unsigned SignType;
    typedef int32_t WithOutUnsigned;
    typedef uint32_t WithUnsigned;
};

template<> struct IntTraits<int64_t> {
    typedef Signed SignType;
    typedef int64_t WithOutUnsigned;
    typedef uint64_t WithUnsigned;
};

template<> struct IntTraits<uint64_t> {
    typedef Unsigned SignType;
    typedef int64_t WithOutUnsigned;
    typedef uint64_t WithUnsigned;
};

}  // namespace Meta
}  // namespace VTSS

#endif /* _META_H_ */
