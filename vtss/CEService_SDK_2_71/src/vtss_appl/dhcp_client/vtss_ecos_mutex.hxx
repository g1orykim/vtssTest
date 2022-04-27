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
}  // namespace VTSS
#endif /* _VTSS_ECOS_MUTEX_H_ */
