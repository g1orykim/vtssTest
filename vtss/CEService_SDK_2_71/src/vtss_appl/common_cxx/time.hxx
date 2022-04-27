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

#ifndef __TIME_H__
#define __TIME_H__

#include <time.h>

#ifdef __linux
#include <stdint.h>
#else
extern "C" {
#include "main.h"
#include <cyg/kernel/kapi.h>
}
#endif

namespace VTSS {

template<typename T0>
struct TimeOperators
{
    T0 operator +(const T0& rhs) const {
        return T0(static_cast<const T0 *>(this)->raw() + rhs.raw());
    }

    T0 operator -(const T0& rhs) const {
        return T0(static_cast<const T0 *>(this)->raw() - rhs.raw());
    }

    T0& operator +=(const T0& rhs) {
        static_cast<T0 *>(this)->raw() += rhs.raw();
        return *(static_cast<T0 *>(this));
    }

    T0& operator -=(const T0& rhs) {
        static_cast<T0 *>(this)->raw() -= rhs.raw();
        return *(static_cast<T0 *>(this));
    }

    bool operator<(const T0& rhs) {
        return static_cast<const T0 *>(this)->raw() < rhs.raw();
    }

    bool operator>(const T0& rhs) {
        return static_cast<const T0 *>(this)->raw() > rhs.raw();
    }

    bool operator<=(const T0& rhs) {
        return static_cast<const T0 *>(this)->raw() >= rhs.raw();
    }

    bool operator>=(const T0& rhs) {
        return static_cast<const T0 *>(this)->raw() >= rhs.raw();
    }
};

struct seconds : public TimeOperators<seconds> {
    seconds() : d_(0) { }
    explicit seconds(u64 d) : d_(d) { }
    u64 raw() const { return d_; }
    u32 raw32() const {
        if (d_ > 0xffffffff)
            return 0xffffffff;
        return d_;
    }
    u64& raw() { return d_; }
private:
    u64 d_;
};

struct milliseconds : public TimeOperators<milliseconds> {
    milliseconds() : d_(0) { }
    milliseconds(seconds d) : d_(d.raw() * 1000LLU) { }
    explicit milliseconds(u64 d) : d_(d) { }
    u64 raw() const { return d_; }
    u32 raw32() const {
        if (d_ > 0xffffffff)
            return 0xffffffff;
        return d_;
    }
    u64& raw() { return d_; }
private:
    u64 d_;
};

struct microseconds : public TimeOperators<microseconds> {
    microseconds() : d_(0) { }
    microseconds(milliseconds d) : d_(d.raw() * 1000LLU) { }
    microseconds(seconds d) : d_(d.raw() * 1000000LLU) { }
    explicit microseconds(u64 d) : d_(d) { }
    u64 raw() const { return d_; }
    u32 raw32() const {
        if (d_ > 0xffffffff)
            return 0xffffffff;
        return d_;
    }
    u64& raw() { return d_; }
private:
    u64 d_;
};

struct nanoseconds : public TimeOperators<nanoseconds> {
    nanoseconds() : d_(0) { }
    nanoseconds(microseconds d) : d_(d.raw() * 1000LLU) { }
    nanoseconds(milliseconds d) : d_(d.raw() * 1000000LLU) { }
    nanoseconds(seconds d) : d_(d.raw() * 1000000000LLU) { }
    explicit nanoseconds(u64 d) : d_(d) { }
    u64 raw() const { return d_; }
    u32 raw32() const {
        if (d_ > 0xffffffff)
            return 0xffffffff;
        return d_;
    }
    u64& raw() { return d_; }
private:
    u64 d_;
};

struct eCosClock {
    typedef cyg_tick_count_t time_t;

    static time_t now() {
        return cyg_current_time();
    }

    static nanoseconds to_nanoseconds(time_t t) {
        return milliseconds(VTSS_OS_TICK2MSEC(t));
    }

    static microseconds to_microseconds(time_t t) {
        return microseconds(VTSS_OS_TICK2MSEC(t));
    }

    static milliseconds to_milliseconds(time_t t) {
        return milliseconds(VTSS_OS_TICK2MSEC(t));
    }

    static seconds to_seconds(time_t t) {
        double d = ((double)(VTSS_OS_TICK2MSEC(t))) / 1000.0;
        int i = d;

        if ((d - (double)i) >= 0.5) {
            i ++;
        }

        return seconds(i);
    }

    static time_t to_time_t(nanoseconds n) {
        if (n.raw() == 0llu) {
            return 0;
        }

        u64 v = n.raw() / 1000000llu;
        if (v == 0) {
            v = 1;
        }

        return VTSS_OS_MSEC2TICK(v);
    }

    static void sleep(time_t d) {
        cyg_thread_delay(d);
    }
};

} /* VTSS */

#endif /* __TIME_H__ */

