/*

 Vitesse Switch Application software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
/*
******************************************************************************

    Revision history
    > CP.Wang, 2012/08/09 13:36
        - create

******************************************************************************
*/
#ifndef __VTSS_FREE_LIST_API_H__
#define __VTSS_FREE_LIST_API_H__
/*
******************************************************************************

    Include

******************************************************************************
*/
#include "vtss_types.h"

/*
******************************************************************************

    Constant and Macro

******************************************************************************
*/
#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*
******************************************************************************

    Structure Type

******************************************************************************
*/
typedef struct vtss_free_node_s     vtss_free_node_t;
struct vtss_free_node_s {
    void                *user_data;     /* user data */
    vtss_free_node_t    *next;          /* next node */
};

typedef struct {
    void                *user_data;     /* all memory for user data */
    u32                 user_data_size; /* size of each user data */
    u32                 max_node_cnt;   /* max number of free nodes */
    vtss_free_node_t    *free_nodes;    /* free nodes */

    /* list control */
    vtss_free_node_t    *free_list;     /* list of free nodes */
    u32                 free_count;     /* number of free nodes in free list */
} vtss_free_list_t;

/*
    this Macro will create tree nodes and the data struct for tree head.
    so, you can use &_struct_name_ to be tree handler for all APIs.
*/
#define VTSS_FREE_LIST(_struct_name_, _user_data_type_, _max_count_) \
    static _user_data_type_ _struct_name_##_user_data[_max_count_]; \
    static vtss_free_node_t _struct_name_##_free_nodes[_max_count_]; \
    static vtss_free_list_t _struct_name_ = {           \
        .user_data      = _struct_name_##_user_data,    \
        .user_data_size = sizeof(_user_data_type_),     \
        .max_node_cnt   = _max_count_,                  \
        .free_nodes     = _struct_name_##_free_nodes,   \
    };

/*
******************************************************************************

    Public Function

******************************************************************************
*/
/*
    initialize free list

    INPUT
        flist - free list

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_free_list_init(
    IN vtss_free_list_t     *flist
);

/*
    allocate a free memory from user data

    INPUT
        flist - free list

    OUTPUT
        n/a

    RETURN
        not NULL - available user_data
        NULL     - memory insufficient
*/
void *vtss_free_list_malloc(
    IN vtss_free_list_t     *flist
);

/*
    initialize free list

    INPUT
        flist - free list
        user_data - the memory for free

    OUTPUT
        n/a

    RETURN
        n/a
*/
void vtss_free_list_free(
    IN vtss_free_list_t     *flist,
    IN void                 *user_data
);

//****************************************************************************
#endif //__VTSS_FREE_LIST_API_H__
