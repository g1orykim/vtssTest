/*
 Vitesse software.

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

#ifndef _OPTIONAL_H_
#define _OPTIONAL_H_

namespace VTSS {
template<typename Type> struct Optional {
    Optional() : valid_(false) { }
    Optional(const Type& t) : valid_(true), data_(t) { }
    Optional(const Optional<Type>& rhs) {
        valid_ = rhs.valid();

        if (valid_ ) {
            data_ = rhs.get();
        }
    }

    Optional<Type>& operator=(const Type& v) {
        valid_ = true;
        data_ = v;
        return *this;
    }

    Optional<Type>& operator=(const Optional<Type>& rhs) {
        valid_ = rhs.valid();

        if (valid_ ) {
            data_ = rhs.get();
        }
        return *this;
    }

    bool operator==(const Optional<Type>& rhs) const {
        if (valid_ != rhs.valid())
            return false;

        if (get() != rhs.get())
            return false;

        return true;
    }

    bool operator!=(const Optional<Type>& rhs) const {
        if (valid_ != rhs.valid())
            return true;
        return false;
    }

    Type const& get() const { return data_; }
    Type&       get() { return data_; }

    Type const* operator ->() const { return &data_; }
    Type*       operator ->() { return &data_; }

    Type const& operator *() const { return data_; }
    Type&       operator *() { return data_; }

    Type const* get_ptr() const { return &data_; }
    Type*       get_ptr() { return &data_; }

    bool        valid() const { return valid_; }
    void        clear() { valid_ = false; }

private:
    bool valid_;
    Type data_;
};
} /* VTSS */

#endif /* _OPTIONAL_H_ */
