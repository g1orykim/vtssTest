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

#include <stdlib.h>  // for malloc, free
#include "string.hxx"

namespace VTSS {

void copy_c_str(const char *str, Buf &b) {
    // can't do anything with a null-buf
    if (b.begin() == b.end()) return;

    // output iterator
    char *i = b.begin();

    if (!str) { // null ptr -> empty string
        *i = 0;
        return;
    }

    // Copy input ptr, with range protection
    while (*str && i != b.end()) *i++ = *str++;

    // Make sure we have room for a null-terminator
    if (i == b.end()) --i;

    // write null terminator
    *i = 0;
}

FixedBuffer::FixedBuffer(size_t s) : size_(0), buf_(0){
    buf_ = static_cast<char *>(malloc(s));
    if (buf_) size_ = s;
}

FixedBuffer::~FixedBuffer() { if (buf_) free(buf_); }

bool operator==(const CBuf& lhs, const CBuf& rhs) {
    return equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

bool operator!=(const CBuf& lhs, const CBuf& rhs) {
    return !equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

str trim(const str& s) {
    const char * begin = find_first_if(s.begin(), s.end(), &isgraph);
    const char * end = find_last_if(begin, s.end(), &isgraph);

    if (end != s.end()) {
        ++ end;
    }

    return str(begin, end);
}

bool split(const str& in, char split_char, str& head, str& tail) {
    const char * split_point = find(in.begin(), in.end(), split_char);
    head = str(in.begin(), split_point);

    if (split_point == in.end()) {
        tail = str(in.end(), in.end());
        return false;
    } else {
        tail = str(split_point + 1, in.end());
        return true;
    }
}

}  // namespace VTSS

