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
    > CP.Wang, 2012/08/08 11:35
        - create

******************************************************************************
*/
#ifndef __VTSS_AVL_TREE_API_H__
#define __VTSS_AVL_TREE_API_H__
/*
******************************************************************************

    Include

******************************************************************************
*/
/**
 * \file vtss_avl_tree_api.h
 * \brief This file defines the APIs for the AVL tree utility
 */
#include "vtss_types.h"
#include "vtss_module_id.h"

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
/**
 * \brief Data structure for the node of AVL tree
 */
typedef struct vtss_avl_tree_node_s     vtss_avl_tree_node_t;
struct vtss_avl_tree_node_s {
    u8                      height;     /**<  Height of the node in tree */
    u8                      nodetype;   /**<  To indicate the node is root, left or right child in vtss_avl_tree_node_type_t */
    vtss_avl_tree_node_t    *pparent;   /**<  Parent node */
    vtss_avl_tree_node_t    *pchild[2]; /**<  Left child(0) and right child(1) */
    void                    *user_data; /**<  User data */
};

/**
 * \brief Result value returned by compare function, vtss_avl_tree_cmp_func_t
 */
typedef enum {
    VTSS_AVL_TREE_CMP_RESULT_A_LARGER  =  1, /**< A is larger than B */
    VTSS_AVL_TREE_CMP_RESULT_A_B_SAME  =  0, /**< A is equal to B */
    VTSS_AVL_TREE_CMP_RESULT_A_SMALLER = -1, /**< A is smaller than B */
} vtss_avl_tree_cmp_result_t;

/**
  * \brief Compare function to compare data A and B.
  *
  * \param data_a [IN] Data A
  * \param data_b [IN] Data B
  *
  * \return vtss_avl_tree_cmp_result_t\n
  */
typedef i32 (*vtss_avl_tree_cmp_func_t)(void *data_a, void *data_b);

/**
 * \brief Data structure for the control header of AVL tree
 */
typedef struct {
    char                            *name;          /**< Friendly name of tree */
    vtss_module_id_t                module_id;      /**< Module id */
    vtss_avl_tree_cmp_func_t        cmp_func;       /**< Compare function */
    u32                             max_node_cnt;   /**< Max number of nodes of this AVL tree */
    vtss_avl_tree_node_t            *free_nodes;    /**< Free nodes in array for AVL tree */

    vtss_avl_tree_node_t            *free_list;     /**< List of free nodes */
    vtss_avl_tree_node_t            *proot;         /**< Root node of AVL tree */
    vtss_avl_tree_node_t            *prev_get_node; /**< Previous result of get operation to accelate the subsequent get operations */
} vtss_avl_tree_t;

/**
  * \brief Macro to create AVL tree control header and required nodes.
  *        So, (&_struct_name_) can be used to be tree handler for all APIs.
  *
  * \param     _struct_name_    [IN] Name of control header that can be used in APIs
  * \param     _tree_name_      [IN] Friendly name of tree
  * \param     _module_id_      [IN] Module id
  * \param     _cmp_            [IN] Compare function
  * \param     _max_node_cnt_   [IN] Max number of nodes of this AVL tree
  *
  * \return    no return value.\n
  */
#define VTSS_AVL_TREE(_struct_name_, _tree_name_, _module_id_, _cmp_, _max_node_cnt_) \
    static vtss_avl_tree_node_t _struct_name_##_avl_tree_nodes[_max_node_cnt_];       \
    static vtss_avl_tree_t _struct_name_ = {                                          \
        .name         = _tree_name_,                                                  \
        .module_id    = _module_id_,                                                  \
        .cmp_func     = _cmp_,                                                        \
        .max_node_cnt = _max_node_cnt_,                                               \
        .free_nodes   = _struct_name_##_avl_tree_nodes,                               \
    };

/**
  * \brief This macro is used when the AVL tree is created by component itself, but not by VTSS_AVL_TREE()
  *        it is to give init values for AVL tree.
  *
  * \param     _struct_name_    [IN] Name of control header that can be used in API's
  * \param     _tree_name_      [IN] Friendly name of tree
  * \param     _module_id_      [IN] Module id
  * \param     _cmp_            [IN] Compare function
  * \param     _max_node_cnt_   [IN] Max number of nodes of this AVL tree
  * \param     _free_nodes_     [IN] Free nodes in array for AVL tree
  *
  * \return    no return value.\n
  */
#define VTSS_AVL_TREE_INIT(_struct_name_, _tree_name_, _module_id_, _cmp_, _max_node_cnt_, _free_nodes_) \
{ \
    (_struct_name_)->name         = _tree_name_; \
    (_struct_name_)->module_id    = _module_id_; \
    (_struct_name_)->cmp_func     = _cmp_; \
    (_struct_name_)->max_node_cnt = _max_node_cnt_; \
    (_struct_name_)->free_nodes   = _free_nodes_; \
}

/**
 * \brief Operation codes in vtss_avl_tree_get()
 */
typedef enum {
    VTSS_AVL_TREE_GET,          /**< get          */
    VTSS_AVL_TREE_GET_FIRST,    /**< get first    */
    VTSS_AVL_TREE_GET_LAST,     /**< get last     */
    VTSS_AVL_TREE_GET_PREV,     /**< get previous */
    VTSS_AVL_TREE_GET_NEXT,     /**< get next     */
} vtss_avl_tree_get_t;

/*
******************************************************************************

    Public Function

******************************************************************************
*/
/**
  * \brief Initialize tree header
  *
  * \param ptree [IN] AVL tree control header
  *
  * \return TRUE  - successful.\n
  *         FALSE - failed.\n
  */
BOOL vtss_avl_tree_init(
    IN vtss_avl_tree_t  *ptree
);

/**
  * \brief Destory all tree nodes in AVL tree.
  *        Currently, this API does nothing.
  *        The AVL tree must be vtss_avl_tree_init() again before reusing it.
  *
  * \param ptree [IN] AVL tree control header
  *
  * \return no return value.\n
  */
void vtss_avl_tree_destroy(
    IN vtss_avl_tree_t  *ptree
);

/**
  * \brief Add user data into the AVL tree.
  *        If the user data is new, then it is added.
  *        If it is duplicated, then it is not added and return FALSE.
  *
  * \param ptree [IN] AVL tree control header
  * \param pdata [IN] User data
  *
  * \return TRUE  - successful.\n
  *         FALSE - failed.\n
  */
BOOL vtss_avl_tree_add(
    IN vtss_avl_tree_t  *ptree,
    IN void             *pdata
);

/**
  * \brief Delete user data from the AVL tree.
  *
  * \param ptree [IN]     AVL tree control header
  * \param pdata [INOUT]  When IN, pdata points to the data with index to be deleted.
  *                       When OUT, pdata points to the original user data deleted from the AVL tree.
  *
  * \return TRUE  - successful.\n
  *         FALSE - failed, the data does not exist.\n
  */
BOOL vtss_avl_tree_delete(
    IN    vtss_avl_tree_t   *ptree,
    INOUT void              **pdata
);

/**
  * \brief Get user data from the AVL tree.
  *
  * \param ptree [IN]     AVL tree control header
  * \param pdata [INOUT]  When IN, pdata points to the data with index to be looked up.
  *                       When OUT, pdata points to the original user data found in the AVL tree accordg to the get operation.
  * \param get   [IN]     The operation code tells which get operation should be done.
  *
  * \return TRUE  - successful.\n
  *         FALSE - failed, the data not found.\n
  */
BOOL vtss_avl_tree_get(
    IN    vtss_avl_tree_t       *ptree,
    INOUT void                  **pdata,
    IN    vtss_avl_tree_get_t   get
);

//****************************************************************************
#endif //__VTSS_AVL_TREE_API_H__
