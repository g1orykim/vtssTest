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

#ifndef _VTSS_ECOS_MUTEX_H_
#define _VTSS_ECOS_MUTEX_H_

#include <cyg/kernel/kapi.h>

extern "C" {
#include "critd_api.h"
}

namespace VTSS {
class mutex {
private:
    mutex(mutex const&);
    mutex& operator=(mutex const&);
    cyg_mutex_t m;

public:
    mutex() { cyg_mutex_init(&m); }
    ~mutex() { cyg_mutex_destroy(&m); }
    void lock() { cyg_mutex_lock(&m); }
    void unlock() { cyg_mutex_unlock(&m); }
    bool try_lock() { return cyg_mutex_trylock(&m); }
    typedef cyg_mutex_t * native_handle_type;
    native_handle_type native_handle() { return &m; }
    typedef unique_lock<mutex> scoped_lock;
};

class critd {
private:
    ::critd_t critd_;
    int trace_group_;

public:
    // TODO, sample program counter on lock/unlock as a replacement for line
    // number
    void lock() {
        ::critd_enter(
                &critd_
#if VTSS_TRACE_ENABLED
                , trace_group_, VTSS_TRACE_LVL_RACKET, "", 0
#endif
        );
    }

    void unlock() {
        ::critd_exit(
                &critd_
#if VTSS_TRACE_ENABLED
                , trace_group_, VTSS_TRACE_LVL_RACKET, "", 0
#endif
        );
    }

    void init(const char*            const name,
              const vtss_module_id_t module_id,
              const int              trace_module_id,
              const int              trace_group,
              const critd_type_t     type) {
        trace_group_ = trace_group;
        ::critd_init(&critd_, name, module_id, trace_module_id, type);
        unlock();
    }
};

}  // namespace VTSS
#endif /* _VTSS_ECOS_MUTEX_H_ */
