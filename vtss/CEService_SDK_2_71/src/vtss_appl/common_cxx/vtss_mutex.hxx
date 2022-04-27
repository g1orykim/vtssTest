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
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams

// This files contains parts which are copied from boost/thread/locks.hpp

#ifndef _VTSS_MUTEX_H_
#define _VTSS_MUTEX_H_

namespace VTSS {
template<typename Mutex>
class lock_guard;

template<typename Mutex>
class unique_lock;
} // namespace VTSS

#if defined(VTSS_OPSYS_ECOS)
#  include "vtss_ecos_mutex.hxx"
#else
#  error "Operating system not supported".
#endif

namespace VTSS {

struct defer_lock_t {};
struct try_to_lock_t {};
struct adopt_lock_t {};

template<typename Mutex = mutex>
class lock_guard {
  private:
    Mutex& m;
    explicit lock_guard(lock_guard&);
    lock_guard& operator=(lock_guard&);
  public:
    explicit lock_guard(Mutex& m_): m(m_) { m.lock(); }
    lock_guard(Mutex& m_,adopt_lock_t): m(m_) {}
    ~lock_guard() { m.unlock(); }
};

template<typename Mutex>
class unique_lock
{
private:
    Mutex* m;
    bool is_locked;
    unique_lock(unique_lock&);
    unique_lock& operator=(unique_lock&);
public:
    unique_lock():
        m(0),is_locked(false)
    {}

    explicit unique_lock(Mutex& m_): m(&m_), is_locked(false) { lock(); }
    unique_lock(Mutex& m_,adopt_lock_t): m(&m_), is_locked(true) {}
    unique_lock(Mutex& m_,defer_lock_t): m(&m_), is_locked(false) {}
    unique_lock(Mutex& m_,try_to_lock_t): m(&m_), is_locked(false) {
        try_lock();
    }

    ~unique_lock() {
        if(owns_lock())
        {
            m->unlock();
        }
    }

    void lock() {
        if(owns_lock()) {
            // TODO, assert
        }
        m->lock();
        is_locked=true;
    }

    bool try_lock() {
        if(owns_lock()) {
            // TODO, assert
        }
        is_locked=m->try_lock();
        return is_locked;
    }

    void unlock() {
        if(!owns_lock()) {
            // TODO, assert
        }
        m->unlock();
        is_locked=false;
    }
    typedef void (unique_lock::*bool_type)();

    operator bool_type() const { return is_locked ? &unique_lock::lock : 0; }
    bool operator!() const { return !owns_lock(); }
    bool owns_lock() const { return is_locked; }
    Mutex* mutex() const { return m; }

    Mutex* release() {
        Mutex* const res=m;
        m=0;
        is_locked=false;
        return res;
    }
};

} // namespace VTSS
#endif /* _VTSS_MUTEX_H_ */
