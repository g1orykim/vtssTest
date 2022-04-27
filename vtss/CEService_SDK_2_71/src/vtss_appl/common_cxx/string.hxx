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

#ifndef _STRING_H_
#define _STRING_H_

#include "common.hxx"
#include "tuple.hxx"
#include "algorithm.hxx"

namespace VTSS {

struct CBuf {
    virtual const char * begin() const = 0;
    virtual const char * end() const = 0;
    size_t size() const { return end() - begin(); }
    virtual ~CBuf() { }
};

struct Buf : public CBuf {
    // Just to avoid warnings on older compilers
    virtual const char * begin() const = 0;
    virtual const char * end() const = 0;

    virtual char * begin() = 0;
    virtual char * end() = 0;
    virtual ~Buf() { }
};

bool operator==(const CBuf& lhs, const CBuf& rhs);
bool operator!=(const CBuf& lhs, const CBuf& rhs);

struct str : public CBuf {
    str() : b_(0), e_(0) { }
    explicit str(const char * b) : b_(b) { e_ = find_end(b); }
    str(const char * b, size_t l) : b_(b), e_(b+l) { if (!b) e_ = 0; }
    str(const char * b, const char * e) : b_(b), e_(e) { if (!b) e_ = 0; }
    str(const CBuf& rhs) : b_(rhs.begin()), e_(rhs.end()) { }
    str& operator=(const CBuf& rhs) {
        b_ = rhs.begin();
        e_ = rhs.end();
        return *this;
    }

    const char * begin() const { return b_; }
    const char * end() const { return e_; }

    virtual ~str() { }

  private:
    const char * b_;
    const char * e_;
};

void copy_c_str(const char *str, Buf &b);

struct BufPtr : public Buf {
    BufPtr() : b_(0), e_(0) { }
    explicit BufPtr(Buf& rhs) : b_(rhs.begin()), e_(rhs.end()) { }
    BufPtr(char *b, char *e) : b_(b), e_(e) { if (!b) e_ = 0; }
    BufPtr(char *b, size_t l) : b_(b), e_(b+l) { if (!b) e_ = 0; }

    BufPtr& operator=(Buf& rhs) {
        b_ = rhs.begin();
        e_ = rhs.end();
        return *this;
    }

    virtual const char * begin() const { return b_; }
    virtual const char * end() const { return e_; }
    virtual char * begin() { return b_; }
    virtual char * end() { return e_; }

    virtual ~BufPtr() { }

  private:
    char *b_, *e_;
};


template<size_t SIZE>
struct StaticBuffer : public Buf {
    StaticBuffer() { }
    StaticBuffer(const char *str) { copy_c_str(str, *this); }

    StaticBuffer<SIZE> operator=(const char *str) {
        copy_c_str(str, *this);
        return *this;
    }

    StaticBuffer<SIZE> operator=(const CBuf &buf) {
        size_t s = min(buf.size(), SIZE);
        copy(buf.begin(), buf.begin() + s, buf);
        return *this;
    }

    virtual ~StaticBuffer() { }

    const char * begin() const { return buf; }
    const char * end() const { return buf+SIZE; }
    char * begin() { return buf; }
    char * end() { return buf+SIZE; }
    unsigned size() const { return SIZE; }

  private:
    char buf[SIZE];
};
typedef StaticBuffer<4>    SBuf4;
typedef StaticBuffer<8>    SBuf8;
typedef StaticBuffer<16>   SBuf16;
typedef StaticBuffer<32>   SBuf32;
typedef StaticBuffer<64>   SBuf64;
typedef StaticBuffer<128>  SBuf128;
typedef StaticBuffer<256>  SBuf256;
typedef StaticBuffer<512>  SBuf512;

struct FixedBuffer : public Buf {
    FixedBuffer(size_t size);

    FixedBuffer operator=(const char *str) {
        copy_c_str(str, *this);
        return *this;
    }

    FixedBuffer operator=(const CBuf &buf) {
        size_t s = min(buf.size(), size_);
        copy(buf.begin(), buf.begin() + s, buf_);
        return *this;
    }

    virtual ~FixedBuffer();

    const char * begin() const { return buf_; }
    const char * end() const { return buf_ + size_; }
    char * begin() { return buf_; }
    char * end() { return buf_ + size_; }
    unsigned size() const { return size_; }

  private:
    size_t size_;
    char * buf_;
};

inline bool isgraph(const char c) { return (c >= 0x21 && c <= 0x7E); }

str trim(const str& s);
bool split(const str& in, char split_char, str& head, str& tail);


}  // namespace VTSS

#endif /* _STRING_H_ */
