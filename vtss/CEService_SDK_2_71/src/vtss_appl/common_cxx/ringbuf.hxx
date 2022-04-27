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

#ifndef __RINGBUF_HXX__
#define __RINGBUF_HXX__

#include "vtss_mutex.hxx"

namespace VTSS {
template<typename T, unsigned SIZE>
class RingBuf {
  public:
    RingBuf() {
        clear();
    }

    // Push will overwrite if needed! returns true if not overwrite was needed,
    // and false if tail was overwrited
    template <typename TT>
    bool push(const TT& t) {
        bool res;

        // make sure there is room for one more
        if (full_) {
            head_increment();
            res = false;
        } else {
            res = true;
        }

        // Store data at tails current possision.
        data_[tail_] = t;
        tail_increment();

        // can not be empty after we have pushed an element
        if (tail_ == head_) // full
            full_ = true;

        return res;
    }

    // Returns true if a value was pop'ed
    bool pop(T& t) {
        if (empty())
            return false;

        t = data_[head_];
        head_increment();
        full_ = false; // can't be full after a pop

        return true;
    }

    bool empty() const {
        return head_ == tail_ && !full_;
    }

    bool full() const {
        return full_;
    }

    unsigned size() const {
        if (full_)
            return SIZE;

        if (head_ <  tail_)
            return tail_ - head_;

        return (SIZE - head_) + tail_;
    }

    void clear() {
        full_ = false;
        head_ = 0;
        tail_ = 0;
    }

  private:
    void head_increment() {
        head_ ++;
        if (head_ >= SIZE) // wrap
            head_ = 0;
    }

    void tail_increment() {
        tail_ ++;
        if (tail_ >= SIZE) // wrap
            tail_ = 0;
    }

    bool full_;
    unsigned head_, tail_;
    T data_[SIZE];
};

template<typename T, unsigned SIZE>
class RingBufConcurrent {
  public:
    RingBufConcurrent () {
        lock_guard<mutex> lock(m);
        impl.clear();
    }

    template <typename TT>
    bool push(const TT& t) {
        lock_guard<mutex> lock(m);
        return impl.push<TT>(t);
    }

    bool pop(T& t) {
        lock_guard<mutex> lock(m);
        return impl.pop(t);
    }

    bool empty() const {
        lock_guard<mutex> lock(m);
        return impl.empty();
    }

    bool full() const {
        lock_guard<mutex> lock(m);
        return impl.full();
    }

    unsigned size() const {
        lock_guard<mutex> lock(m);
        return impl.size();
    }

    void clear() {
        lock_guard<mutex> lock(m);
        impl.clear();
    }

  private:
    mutable mutex m;
    RingBuf<T, SIZE> impl;
};

}  // namespace VTSS
#endif  // __RINGBUF_HXX__
