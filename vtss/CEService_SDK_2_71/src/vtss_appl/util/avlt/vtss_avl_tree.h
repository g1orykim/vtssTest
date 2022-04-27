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
    > CP.Wang, 2012/07/20 12:54
        - Create

******************************************************************************
*/
/*******************************************************
*
* Project: avl-tree
*
* Description: Generic implementation of AVL tree
*
*******************************************************/
#ifndef __VTSS_AVL_TREE_H__
#define __VTSS_AVL_TREE_H__

/*
******************************************************************************

    Include File

******************************************************************************
*/

/*
******************************************************************************

    Constant and Macro

******************************************************************************
*/
#define LCHILD              0
#define RCHILD              1

/*
    for debug only
*/
#ifdef WIN32
#define DBG_PRINTF          printf
#define _AVLT_SNPRINTF      _snprintf
#else
#define DBG_PRINTF          (void)diag_printf
#define _AVLT_SNPRINTF      (void)snprintf
#endif

/* CP, 2012/08/08 16:17, for debugging only */
#if 1
#ifndef WIN32
#include <cyg/infra/diag.h>
#endif
#define _AVLT_DEBUG(args)   { DBG_PRINTF args; }
#else
#define _AVLT_DEBUG(args)
#endif

#define VTSS_AVL_TREE_T_E(...)  _AVLT_DEBUG(("Error: %s, %s, %d, ", __FILE__, __FUNCTION__, __LINE__)); \
                                _AVLT_DEBUG((__VA_ARGS__));

/*
******************************************************************************

    Type Definition

******************************************************************************
*/
typedef enum {
    VTSS_AVL_TREE_NODE_UNKNOWN,
    VTSS_AVL_TREE_NODE_ROOT,
    VTSS_AVL_TREE_NODE_LCHILD,
    VTSS_AVL_TREE_NODE_RCHILD
} vtss_avl_tree_node_type_t;

#define SET_ROOT(ptreenode) { \
    ptreenode->nodetype = VTSS_AVL_TREE_NODE_ROOT; \
}

#define SET_LCHILD(ptreenode) { \
    ptreenode->nodetype = VTSS_AVL_TREE_NODE_LCHILD; \
}

#define SET_RCHILD(ptreenode) { \
    ptreenode->nodetype = VTSS_AVL_TREE_NODE_RCHILD; \
}

#define IS_UNKNOWN(ptreenode)       (ptreenode->nodetype == VTSS_AVL_TREE_NODE_UNKNOWN)
#define IS_ROOT(ptreenode)          (ptreenode->nodetype == VTSS_AVL_TREE_NODE_ROOT)
#define IS_LCHILD(ptreenode)        (ptreenode->nodetype == VTSS_AVL_TREE_NODE_LCHILD)
#define IS_RCHILD(ptreenode)        (ptreenode->nodetype == VTSS_AVL_TREE_NODE_RCHILD)
#define NO_LCHILD(ptreenode)        (ptreenode->pchild[LCHILD] == NULL)
#define NO_RCHILD(ptreenode)        (ptreenode->pchild[RCHILD] == NULL)
#define HAS_LCHILD(ptreenode)       (ptreenode->pchild[LCHILD] != NULL)
#define HAS_RCHILD(ptreenode)       (ptreenode->pchild[RCHILD] != NULL)
#define GET_LCHILD(ptreenode)       (ptreenode->pchild[LCHILD])
#define GET_RCHILD(ptreenode)       (ptreenode->pchild[RCHILD])
#define GET_PARENT(ptreenode)       (ptreenode->pparent)

#define LCHILD_HEIGHT(ptreenode)    (ptreenode->pchild[LCHILD] ? ptreenode->pchild[LCHILD]->height : 0)
#define RCHILD_HEIGHT(ptreenode)    (ptreenode->pchild[RCHILD] ? ptreenode->pchild[RCHILD]->height : 0)
#define NO_CHILD(ptreenode)         (NO_LCHILD(ptreenode) && NO_RCHILD(ptreenode))

#define NODE_NEXT(n)                ((n)->pparent)

//****************************************************************************
#endif /* __VTSS_AVL_TREE_H__ */
