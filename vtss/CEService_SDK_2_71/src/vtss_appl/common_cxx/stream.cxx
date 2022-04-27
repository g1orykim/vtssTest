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

#include "stream.hxx"

namespace VTSS {

ostream& operator<<(ostream& o, const str& s) {
    obuffer_iterator<char> out(o);
    copy(s.begin(), s.end(), out);
    return o;
}

ostream& operator<<(ostream& o, const char * s) {
    const char *i = s;
    while(*i) o.push(*i++);
    return o;
}

ostream& operator<<(ostream& o, const size_t s) {
    obuffer_iterator<char> out(o);
    BufStream<SBuf16> buf;
    unsigned_to_dec_rbuf(s, buf);
    copy_backward(buf.begin(), buf.end(), out);
    return o;
}

}  // namespace VTSS

