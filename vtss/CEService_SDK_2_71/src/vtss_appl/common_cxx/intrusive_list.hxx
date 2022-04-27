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

#ifndef __INTRUSIVE_LIST_HXX__
#define __INTRUSIVE_LIST_HXX__

extern "C" {
#include "main.h" // for VTSS_ASSERT
}
#include "meta.hxx"

namespace VTSS {
template<typename T> class List;
template<typename Type,
         typename ListNode_type>
class list_iterator;

template< typename T >
struct ListNode_impl
{
    friend class List<T>;
    friend class list_iterator<T, ListNode_impl<T> >;
    friend class list_iterator<const T, const ListNode_impl<T> >;

    ListNode_impl() :
        next(0), prev(0)
    { }

    bool is_linked() const {
        return next != 0 && prev != 0;
    }

    ~ListNode_impl() {
        VTSS_ASSERT(next == 0);
        VTSS_ASSERT(prev == 0);
    }

    void unlink() { // Warning.... not thread safe
        prev->next = next;
        next->prev = prev;
        next = 0;
        prev = 0;
    }

private:
    ListNode_impl<T> * next;
    ListNode_impl<T> * prev;
};


template<typename Super, typename T>
struct IterTypes
{
    typedef ListNode_impl<T> * member;
    typedef ListNode_impl<T> const * const_member;
};

template<typename Super, typename T>
struct IterTypes<Super, const T>
{
    typedef ListNode_impl<T> const * member;
    typedef ListNode_impl<T> const * const_member;
};


template<typename Type,
         typename ListNode_type>
class list_iterator
{
public:
    typedef typename Meta::remove_const<Type>::type Type_no_const;
    friend class List<Type_no_const>;

    explicit list_iterator(ListNode_type * _n) : n(_n) { }
    list_iterator(const list_iterator& rhs) : n(rhs.n) { }
    Type* operator->() { return static_cast<Type*>(n); }
    Type& operator*() { return *static_cast<Type*>(n); }

    list_iterator& operator++() {
        n = n->next;
        return *this;
    }

    list_iterator operator++(int) {
        n = n->next;
        return list_iterator(n->prev);
    }

    list_iterator& operator--() {
        n = n->prev;
        return *this;
    }

    list_iterator operator--(int) {
        n = n->prev;
        return list_iterator(n->next);
    }

    bool operator==(const list_iterator& rhs) {
        return n == rhs.n;
    }

    bool operator!=(const list_iterator& rhs) {
        return n != rhs.n;
    }

private:
    ListNode_type * n;
};

template<typename T>
class List : ListNode_impl<T>
{
public:
    typedef T                                      value_type;
    typedef value_type*                            pointer;
    typedef const value_type*                      const_pointer;
    typedef value_type&                            reference;
    typedef const value_type&                      const_reference;
    typedef list_iterator<T, ListNode_impl<T> > iterator;
    typedef list_iterator<const T, const ListNode_impl<T> > const_iterator;

    List() {
        ListNode_impl<T>::next = static_cast<ListNode_impl<T>*>(this);
        ListNode_impl<T>::prev = static_cast<ListNode_impl<T>*>(this);
    }

    ~List() {
        VTSS_ASSERT(ListNode_impl<T>::next == static_cast<ListNode_impl<T>*>(this));
        VTSS_ASSERT(ListNode_impl<T>::prev == static_cast<ListNode_impl<T>*>(this));
        ListNode_impl<T>::next = 0;
        ListNode_impl<T>::prev = 0;
    }

    inline iterator begin () {
        return iterator(
                static_cast<ListNode_impl<T>*>(
                    ListNode_impl<T>::next));
    }

    inline const_iterator begin ( ) const {
        return const_iterator(
                static_cast<const ListNode_impl<T>*>(
                    ListNode_impl<T>::next));
    }

    inline iterator end ( ) {
        return iterator(static_cast<ListNode_impl<T>*>(this));
    }

    inline const_iterator end () const {
        return const_iterator(static_cast<const ListNode_impl<T>*>(this));
    }

    inline iterator insert(iterator it, T& _v) {
        VTSS_ASSERT(!_v.is_linked());

        ListNode_impl<T>* v = static_cast<ListNode_impl<T>*>(&_v);
        VTSS_ASSERT(it.n != v);

        v->prev = it.n->prev;
        v->next = it.n;

        it.n->prev->next = v;
        it.n->prev = v;

        return iterator(v);
    }

    // TODO, make an external function
    inline void insert_sorted(T& _v) {
        VTSS_ASSERT(!_v.is_linked());
        iterator i = begin();
        while (i != end() && *i < _v ) {
            ++i;
        }
        insert(i, _v);
    }

    inline void unlink (T& t) {
        // TODO, check that t is actually a member of this list
        if (t.is_linked())
            t.unlink();
    }

    inline iterator erase (iterator it) {
        iterator i = it;
        ++i;
        it->unlink();
        return i;
    }

    inline void push_front (T& v) { insert (begin(), v); }
    inline void push_back (T& v) { insert (end(), v); }
    inline void pop_front () { erase (begin()); }
    inline void pop_back () { erase (--end()); }
    inline const_reference front (void) const { return (*begin()); }
    inline reference front () { return (*begin()); }
    inline const_reference back (void) const { return (*(--end())); }
    inline reference back () { return (*(--end())); }
    bool empty() const {
        const ListNode_impl<T>* this_node =
            static_cast<const ListNode_impl<T>*>(this);

        return ListNode_impl<T>::next == this_node &&
            ListNode_impl<T>::prev == this_node;
    }
};
} /* VTSS */
#endif /* __INTRUSIVE_LIST_HXX__ */
