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

#ifndef _TUPLE_H_
#define _TUPLE_H_

#include "meta.hxx"

namespace VTSS {
template<typename T1, typename T2> struct Pair {
    typedef typename Meta::add_const<T1>::type CT1;
    typedef typename Meta::add_const<T2>::type CT2;

    Pair(CT1& t1, CT2& t2) : first(t1), second(t2) { }
    Pair(const Pair<T1, T2>& rhs) : first(rhs.first), second(rhs.second) { }

    T1 first;
    T2 second;
};
} /* VTSS */

#endif /* _TUPLE_H_ */
