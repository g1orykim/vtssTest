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
#include "gtest/gtest.h"
#include "string.hxx"

namespace VTSS {

TEST(str, constructors) {
    { str s; } // default constructable

    {  // null ptr
        str s(0);
        EXPECT_EQ(0, s.size());
    }

    {  // null ptr
        str s(static_cast<const char *>(0), 1024);
        EXPECT_EQ(0, s.size());
    }

    {  // null ptr + length
        const char * b = 0;
        str s(b, b + 1024);
    }

    {
        const char * b = 0;
        StaticBuffer<16> sbuf = b;
        EXPECT_EQ(16, sbuf.size());
    }

    {  // overflow
        const char * b = "asdfsadfasdfadsf";
        StaticBuffer<8> sbuf = b;
        EXPECT_EQ(8, sbuf.size());
    }

    {  // overflow by one
        const char * b = "asdfsadf";
        StaticBuffer<8> sbuf = b;
        EXPECT_EQ(8, sbuf.size());

        const char * b2 = "asdfsad\0";
        str ss(b2, b2 + 8);

        EXPECT_EQ(sbuf, ss);
    }

    {  // empty_buf
        const char * b = "asdfsadf";
        StaticBuffer<0> sbuf = b;
        EXPECT_EQ(0, sbuf.size());
    }

    { // constructable from c-string
        str s("hello");
        EXPECT_EQ(5, s.size());
        EXPECT_EQ(s, str("hello"));
    }

    { // ptr, length
        str s("hellohello", 5);
        EXPECT_EQ(5, s.size());
        EXPECT_EQ(s, str("hello"));
    }

    { // begin, end
        const char *cs = "hello";
        str s(cs, cs + 5);
        EXPECT_EQ(5, s.size());
        EXPECT_EQ(s, str("hello"));
    }

    { // from str
        str a("asdf");
        str b(a);

        EXPECT_EQ(4, a.size());
        EXPECT_EQ(4, b.size());

        EXPECT_EQ(a, str("asdf"));
        EXPECT_EQ(b, str("asdf"));
    }

    { // from buf
        BufPtr a;
        str b(a);

        EXPECT_EQ(0, a.size());
        EXPECT_EQ(0, b.size());
        EXPECT_EQ(a, str(""));
        EXPECT_EQ(b, str(""));
    }


    {
        StaticBuffer<16> sbuf = "asdf1234qwernnn";
        BufPtr a(sbuf);

        EXPECT_EQ(16, sbuf.size());
        EXPECT_EQ(16, a.size());

        a = sbuf;

        BufPtr b(a);
        BufPtr c = a;
        BufPtr d;
        d = a;
        BufPtr e(a.begin(), a.end());

        EXPECT_EQ(16, b.size());
        EXPECT_EQ(16, c.size());
        EXPECT_EQ(16, d.size());
        EXPECT_EQ(16, e.size());

        EXPECT_EQ(sbuf, a);
        EXPECT_EQ(sbuf, b);
        EXPECT_EQ(sbuf, c);
        EXPECT_EQ(sbuf, d);
        EXPECT_EQ(sbuf, e);
    }


}



}

