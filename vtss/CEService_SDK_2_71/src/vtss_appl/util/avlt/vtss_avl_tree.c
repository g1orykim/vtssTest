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
        - revise

******************************************************************************
*/
/*******************************************************
*
* Project: avl-tree
*
* Description: Generic implementation of AVL tree
*
*******************************************************/

/*
******************************************************************************

    Include File

******************************************************************************
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vtss_avl_tree_api.h"
#include "vtss_avl_tree.h"

/*
******************************************************************************

    Constant and Macro

******************************************************************************
*/
/* Macros
***********/
#define MAX_INT(a,b)                ((a) > (b) ? (a) : (b))

/*
******************************************************************************

    Type Definition

******************************************************************************
*/

/*
******************************************************************************

    Static Variable

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
static u8 _height_get(vtss_avl_tree_node_t *node)
{
    return( MAX_INT(LCHILD_HEIGHT(node), RCHILD_HEIGHT(node)) + 1 );
}

static void _tree_init(
    IN vtss_avl_tree_t  *ptree
)
{
    u32                     i;
    vtss_avl_tree_node_t    *node;

    /* reset free nodes */
    memset(ptree->free_nodes, 0, ptree->max_node_cnt * sizeof(vtss_avl_tree_node_t));

    /* link free_nodes to be a list */
    i = ptree->max_node_cnt - 1;
    node = ptree->free_nodes;
    for ( ; i > 0; i--, node++ ) {
        NODE_NEXT(node) = node + 1;
    }
    NODE_NEXT(node) = NULL;

    /* set default value */
    ptree->free_list     = ptree->free_nodes;
    ptree->proot         = NULL;
    ptree->prev_get_node = NULL;
}

static vtss_avl_tree_node_t *_tree_node_alloc(
    IN vtss_avl_tree_t *ptree
)
{
    vtss_avl_tree_node_t    *node;

    if ( ptree->free_list == NULL ) {
        return NULL;
    }

    // take node
    node = ptree->free_list;
    ptree->free_list = NODE_NEXT(node);
    NODE_NEXT(node) = NULL;

    return node;
}

static void _tree_node_free(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *node
)
{
    if ( node == NULL ) {
        return;
    }

    // clean node
    memset(node, 0, sizeof(vtss_avl_tree_node_t));

    // mark as free node
    node->nodetype = VTSS_AVL_TREE_NODE_UNKNOWN;

    // link back to free list
    NODE_NEXT(node) = ptree->free_list;
    ptree->free_list = node;
}

/*
    rotate tree right

    INPUT
        *root - root of tree to rotate

    OUTPUT
        *root - new root of tree after rotate

    RETURN
        TRUE  - successful
        FALSE - failed
*/
static BOOL _tree_rotate_right(
    INOUT vtss_avl_tree_node_t  **root
)
{
    vtss_avl_tree_node_t    *old_root;
    vtss_avl_tree_node_t    *new_root;
    vtss_avl_tree_node_t    *new_left;

    // get old root
    old_root = *root;

    // get new root
    new_root = old_root->pchild[LCHILD];
    if ( new_root == NULL ) {
        VTSS_AVL_TREE_T_E("new_root == NULL\n");
        return FALSE;
    }

    // get new left of old root
    new_left = new_root->pchild[RCHILD];

    /* rotate right */

    // set right child of new root
    new_root->pchild[RCHILD] = old_root;
    old_root->pparent = new_root;
    SET_RCHILD( old_root );

    // set left child of old root
    old_root->pchild[LCHILD] = new_left;
    if ( new_left ) {
        new_left->pparent = old_root;
        SET_LCHILD( new_left );
    }

    // get height of old root
    old_root->height = _height_get( old_root );

    // get height of new root
    new_root->height = _height_get( new_root );

    // Return new root
    *root = new_root;
    return TRUE;
}

/*
    rotate tree left

    INPUT
        *root - root of tree to rotate

    OUTPUT
        *root - new root of tree after rotate

    RETURN
        TRUE  - successful
        FALSE - failed
*/
static BOOL _tree_rotate_left(
    INOUT vtss_avl_tree_node_t  **root
)
{
    vtss_avl_tree_node_t    *old_root;
    vtss_avl_tree_node_t    *new_root;
    vtss_avl_tree_node_t    *new_right;

    // get old root
    old_root = *root;

    // get new root
    new_root = old_root->pchild[RCHILD];
    if ( new_root == NULL ) {
        VTSS_AVL_TREE_T_E("new_root == NULL\n");
        return FALSE;
    }

    // get new left of old root
    new_right = new_root->pchild[LCHILD];

    /* rotate left */

    // set right child of new root
    new_root->pchild[LCHILD] = old_root;
    old_root->pparent = new_root;
    SET_LCHILD( old_root );

    // set left child of old root
    old_root->pchild[RCHILD] = new_right;
    if ( new_right ) {
        new_right->pparent = old_root;
        SET_RCHILD( new_right );
    }

    // get height of old root
    old_root->height = _height_get( old_root );

    // get height of new root
    new_root->height = _height_get( new_root );

    // Return new root
    *root = new_root;
    return TRUE;
}

/*
    get balance number of the tree of root

    INPUT
        root - tree root

    OUTPUT
        none

    RETURN
        the height difference between left and right subtree
*/
static i32 _tree_balance_get(
    IN vtss_avl_tree_node_t     *root
)
{
    if ( root ) {
        return( LCHILD_HEIGHT(root) - RCHILD_HEIGHT(root) );
    } else {
        return 0;
    }
}

/*
    get tree node for the data

    INPUT
        root   - root node
        *pdata - data to get
        cmp_func - compare function

    OUTPUT
        n/a

    RETURN
        not NULL - found
        NULL     - not found
*/
static vtss_avl_tree_node_t *_tree_node_get(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *root,
    IN void                 *pdata
)
{
    vtss_avl_tree_node_t        *walk_node;
    vtss_avl_tree_cmp_result_t  r;

    walk_node = root;
    while ( walk_node ) {
        r = (*(ptree->cmp_func))(pdata, walk_node->user_data);
        switch ( r ) {
        case VTSS_AVL_TREE_CMP_RESULT_A_B_SAME:
            return walk_node;

        case VTSS_AVL_TREE_CMP_RESULT_A_LARGER:
            // find Right child
            if ( NO_RCHILD(walk_node)) {
                return NULL;
            }
            // go right
            walk_node = GET_RCHILD(walk_node);
            break;

        case VTSS_AVL_TREE_CMP_RESULT_A_SMALLER:
            // find Left child
            if ( NO_LCHILD(walk_node)) {
                return NULL;
            }
            // go left
            walk_node = GET_LCHILD(walk_node);
            break;

        default:
            VTSS_AVL_TREE_T_E("invalid compare result: %d\n", r);
            return NULL;
        } // switch
    } // while
    return NULL;
}

/*
    get previous tree node for the data

    INPUT
        ptree  - the tree
        root   - root node
        *pdata - data to get previous

    OUTPUT
        n/a

    RETURN
        not NULL - found
        NULL     - not found
*/
static vtss_avl_tree_node_t *_tree_node_get_prev(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *root,
    IN void                 *pdata
)
{
    vtss_avl_tree_node_t        *walk_node;
    vtss_avl_tree_node_t        *result_node;
    vtss_avl_tree_cmp_result_t  r;

    result_node = NULL;
    walk_node = root;
    while ( walk_node ) {
        r = (*(ptree->cmp_func))(pdata, walk_node->user_data);
        switch ( r ) {
        case VTSS_AVL_TREE_CMP_RESULT_A_LARGER:
            // find Right child
            if ( NO_RCHILD(walk_node)) {
                return walk_node;
            }
            // store temp result
            result_node = walk_node;
            // go right
            walk_node = GET_RCHILD(walk_node);
            break;

        case VTSS_AVL_TREE_CMP_RESULT_A_SMALLER:
        case VTSS_AVL_TREE_CMP_RESULT_A_B_SAME:
            // find Left child
            if ( NO_LCHILD(walk_node)) {
                return result_node;
            }
            // go left
            walk_node = GET_LCHILD(walk_node);
            break;

        default:
            VTSS_AVL_TREE_T_E("invalid compare result: %d\n", r);
            return NULL;
        } // switch
    } // while
    return NULL;
}

/*
    get next tree node for the data

    INPUT
        ptree  - the tree
        root   - root node
        *pdata - data to get next

    OUTPUT
        n/a

    RETURN
        not NULL - found
        NULL     - not found
*/
static vtss_avl_tree_node_t *_tree_node_get_next(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *root,
    IN void                 *pdata
)
{
    vtss_avl_tree_node_t        *walk_node;
    vtss_avl_tree_node_t        *result_node;
    vtss_avl_tree_cmp_result_t  r;

    result_node = NULL;
    walk_node = root;
    while ( walk_node ) {
        r = (*(ptree->cmp_func))(pdata, walk_node->user_data);
        switch ( r ) {
        case VTSS_AVL_TREE_CMP_RESULT_A_LARGER:
        case VTSS_AVL_TREE_CMP_RESULT_A_B_SAME:
            // find Right child
            if ( NO_RCHILD(walk_node)) {
                return result_node;
            }
            // go right
            walk_node = GET_RCHILD(walk_node);
            break;

        case VTSS_AVL_TREE_CMP_RESULT_A_SMALLER:
            // find Left child
            if ( NO_LCHILD(walk_node)) {
                return walk_node;
            }
            // store temp result
            result_node = walk_node;
            // go left
            walk_node = GET_LCHILD(walk_node);
            break;

        default:
            VTSS_AVL_TREE_T_E("invalid compare result: %d\n", r);
            return NULL;
        } // switch
    } // while
    return NULL;
}

static vtss_avl_tree_node_t *_min_node_get(
    IN vtss_avl_tree_node_t *root
)
{
    vtss_avl_tree_node_t    *min_node;

    if ( root->pchild[LCHILD] == NULL ) {
        return root;
    }

    for ( min_node = root->pchild[LCHILD];
          min_node->pchild[LCHILD];
          min_node = min_node->pchild[LCHILD]) {
        ;
    }

    return min_node;
}

static vtss_avl_tree_node_t *_max_node_get(
    IN vtss_avl_tree_node_t *root
)
{
    vtss_avl_tree_node_t    *max_node;

    if ( root->pchild[RCHILD] == NULL ) {
        return root;
    }

    for ( max_node = root->pchild[RCHILD];
          max_node->pchild[RCHILD];
          max_node = max_node->pchild[RCHILD]) {
        ;
    }

    return max_node;
}

static BOOL _tree_node_rotate(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *rotate_root
)
{
    i32                     rotate_balance;
    i32                     left_balance;
    i32                     right_balance;
    vtss_avl_tree_node_t    *new_root;
    vtss_avl_tree_node_t    *parent;
    vtss_avl_tree_node_t    *rotate_parent;
    BOOL                    b_left_child;

    // get balance of root
    rotate_parent = rotate_root;
    while ( rotate_parent ) {
        rotate_root    = rotate_parent;
        rotate_parent  = rotate_root->pparent;
        rotate_balance = _tree_balance_get( rotate_root );
        if ( rotate_balance <= 1 && rotate_balance >= -1 ) {
            // balance, next parent
            continue;
        }

        // left higher
        if ( rotate_balance > 1 ) {
            // get balance of left subtree
            left_balance = _tree_balance_get( rotate_root->pchild[LCHILD] );
            if ( left_balance < 0 ) {
                /* Left Right */
                // rotate left first
                new_root = rotate_root->pchild[LCHILD];
                if ( _tree_rotate_left(&new_root) == FALSE ) {
                    VTSS_AVL_TREE_T_E("_tree_rotate_left()\n");
                    return FALSE;
                }
                // update new root to root
                rotate_root->pchild[LCHILD] = new_root;
                new_root->pparent = rotate_root;
                SET_LCHILD( new_root );
                rotate_root->height = _height_get( rotate_root );
            } else {
                /* Left Left */
                // rotate right only
            }
            // rotate right
            new_root = rotate_root;
            parent = rotate_root->pparent;
            b_left_child = IS_LCHILD( rotate_root );
            if ( _tree_rotate_right(&new_root) == FALSE ) {
                VTSS_AVL_TREE_T_E("_tree_rotate_right()\n");
                return FALSE;
            }
            if ( parent ) {
                // tree node
                if ( b_left_child ) {
                    parent->pchild[LCHILD] = new_root;
                    new_root->pparent = parent;
                    SET_LCHILD( new_root );
                } else {
                    parent->pchild[RCHILD] = new_root;
                    new_root->pparent = parent;
                    SET_RCHILD( new_root );
                }
                parent->height = _height_get( parent );
            } else {
                // tree root
                ptree->proot = new_root;
                SET_ROOT( new_root );
                new_root->pparent = NULL;
            }
            continue;
        }

        // right higher
        if ( rotate_balance < -1 ) {
            // get balance of left subtree
            right_balance = _tree_balance_get( rotate_root->pchild[RCHILD] );
            if ( right_balance > 0 ) {
                /* Right Left */
                // rotate right first
                new_root = rotate_root->pchild[RCHILD];
                if ( _tree_rotate_right(&new_root) == FALSE ) {
                    VTSS_AVL_TREE_T_E("_tree_rotate_right()\n");
                    return FALSE;
                }
                // update new root to root
                rotate_root->pchild[RCHILD] = new_root;
                new_root->pparent = rotate_root;
                SET_RCHILD( new_root );
                rotate_root->height = _height_get( rotate_root );
            } else {
                /* Right Right */
                // rotate left only
            }
            // rotate left
            new_root = rotate_root;
            parent = rotate_root->pparent;
            b_left_child = IS_LCHILD( rotate_root );
            if ( _tree_rotate_left(&new_root) == FALSE ) {
                VTSS_AVL_TREE_T_E("_tree_rotate_left()\n");
                return FALSE;
            }
            if ( parent ) {
                // tree node
                if ( b_left_child ) {
                    parent->pchild[LCHILD] = new_root;
                    new_root->pparent = parent;
                    SET_LCHILD( new_root );
                } else {
                    parent->pchild[RCHILD] = new_root;
                    new_root->pparent = parent;
                    SET_RCHILD( new_root );
                }
                parent->height = _height_get( parent );
            } else {
                // tree root
                ptree->proot = new_root;
                SET_ROOT( new_root );
                new_root->pparent = NULL;
            }
            continue;
        }

    } // while
    return TRUE;
}

/*
    if new, then add
    if duplcate,
        if ( b_add_only ) then FAILED
        else set, replace the old one and output the old one

    INPUT
        ptree      - AVL tree
        *pdata     - data set into tree
        b_add_only - add or set data

    OUTPUT
        *pdata - NULL     : add
                 not NULL : set, the old one replaced

    RETURN
        TRUE  - successful
        FALSE - failed
            if memory insufficient, *pdata = NULL;
            if add, duplicate index, *pdata = duplicate one;
            if set, the same memory node, *pdata = original same one;
*/
static BOOL _tree_node_set(
    IN    vtss_avl_tree_t   *ptree,
    INOUT void              **pdata,
    IN    BOOL              b_add_only
)
{
    vtss_avl_tree_node_t        *pwalknode = NULL;
    vtss_avl_tree_node_t        **insert_point = NULL;
    vtss_avl_tree_cmp_result_t  r;
    vtss_avl_tree_node_t        *pnewnode;
    vtss_avl_tree_node_t        *parent;
    void                        *pnewdata;
    BOOL                        b;
    u8                          new_height;

    pnewdata = *pdata;
    *pdata = NULL;

    pnewnode = _tree_node_alloc(ptree);
    if ( pnewnode == NULL ) {
        return FALSE;
    }

    /* Initialize the new node's metadata */
    pnewnode->user_data      = pnewdata;
    pnewnode->height         = 1;
    pnewnode->pparent        = NULL;
    pnewnode->nodetype       = VTSS_AVL_TREE_NODE_UNKNOWN;
    pnewnode->pchild[LCHILD] = NULL;
    pnewnode->pchild[RCHILD] = NULL;

    /* Check if the new node is the root */
    if ( ptree->proot == NULL ) {
        SET_ROOT( pnewnode );
        ptree->proot      = pnewnode;
        pnewnode->height  = 1;
        return TRUE;
    }

    /* Traverse and find the position of the new node */
    insert_point = &ptree->proot;
    while ( *insert_point ) {
        pwalknode = *insert_point;
        r = (*ptree->cmp_func)(pwalknode->user_data, pnewdata);
        switch ( r ) {
        case VTSS_AVL_TREE_CMP_RESULT_A_B_SAME:
            // recover height
            for ( parent = pwalknode->pparent; parent != NULL; parent = parent->pparent) {
                parent->height = _height_get( parent );
            }
            // get duplicate one
            *pdata = pwalknode->user_data;
            // free new node
            _tree_node_free(ptree, pnewnode);
            // add or set
            if ( b_add_only ) {
                return FALSE;
            } else {
                if ( pwalknode->user_data == pnewdata ) {
                    return FALSE;
                } else {
                    // replace
                    pwalknode->user_data = pnewdata;
                    return TRUE;
                }
            }

        case VTSS_AVL_TREE_CMP_RESULT_A_LARGER:
            insert_point = &pwalknode->pchild[LCHILD];
            SET_LCHILD(pnewnode);
            break;

        case VTSS_AVL_TREE_CMP_RESULT_A_SMALLER:
            insert_point = &pwalknode->pchild[RCHILD];
            SET_RCHILD(pnewnode);
            break;

        default:
            VTSS_AVL_TREE_T_E("invalid compare result: %d\n", r);
            return FALSE;
        }
    } /* End: while() loop - Traversal ends */

    /* Found the position of the newnode in the tree.
       Insert the new node and take note of it's parent */
    *insert_point = pnewnode;
    pnewnode->pparent = pwalknode;

    // update height
    parent = pwalknode;
    while ( parent ) {
        new_height = _height_get( parent );
        if ( parent->height != new_height ) {
            parent->height = new_height;
            parent = parent->pparent;
        } else {
            // if same height, then stop update
            // because the later also will be the same
            break;
        }
    }

    // rotate
    b = _tree_node_rotate(ptree, pwalknode);
    return b;
}

/*
    delete a tree node
    ASSUME: the deleted node without any child, or
            its children are not used anymore

    INPUT
        ptree       - tree
        root        - root for rotate
        delete_node - node to delete

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed, due to with children
*/
static BOOL _tree_node_delete(
    IN vtss_avl_tree_t      *ptree,
    IN vtss_avl_tree_node_t *delete_node
)
{
    vtss_avl_tree_node_t    *replace_node;
    vtss_avl_tree_node_t    *parent;
    vtss_avl_tree_node_t    *rotate_root;
    u8                      new_height;
    BOOL                    b;

    // get root for rotate
    rotate_root = delete_node;

    // delete and replace
    if ( delete_node->pchild[LCHILD] ) {
        // left child
        replace_node = _max_node_get( delete_node->pchild[LCHILD] );
        delete_node->user_data = replace_node->user_data;
        b = _tree_node_delete(ptree, replace_node);
        if ( b == FALSE ) {
            VTSS_AVL_TREE_T_E("delete left replace node\n");
            return FALSE;
        }
    } else if ( delete_node->pchild[RCHILD] ) {
        // right child
        replace_node = _min_node_get( delete_node->pchild[RCHILD] );
        delete_node->user_data = replace_node->user_data;
        b = _tree_node_delete(ptree, replace_node);
        if ( b == FALSE ) {
            VTSS_AVL_TREE_T_E("delete right replace node\n");
            return FALSE;
        }
    } else {
        // no child, just delete it
        if ( delete_node->pparent ) {
            // parent
            parent = delete_node->pparent;
            if ( delete_node == parent->pchild[RCHILD] ) {
                parent->pchild[RCHILD] = NULL;
            } else if ( delete_node == parent->pchild[LCHILD] ) {
                parent->pchild[LCHILD] = NULL;
            } else {
                VTSS_AVL_TREE_T_E("delete_node is not parent's child\n");
                return FALSE;
            }
            // update height
            while ( parent ) {
                new_height = _height_get( parent );
                if ( parent->height != new_height ) {
                    parent->height = new_height;
                    parent = parent->pparent;
                } else {
                    // if same height, then stop update
                    // because the later also will be the same
                    break;
                }
            }
            // get root for rotate
            rotate_root = delete_node->pparent;
        } else {
            // no parent, tree with only one node
            // free node and empty tree
            ptree->proot = NULL;
            // get root for rotate
            rotate_root = NULL;
        }

        // free node
        _tree_node_free(ptree, delete_node);
    }

    // rotate
    b = _tree_node_rotate(ptree, rotate_root);
    return b;
}

/*
******************************************************************************

    Public Function

******************************************************************************
*/
/*
    initialize tree header

    INPUT
        ptree - AVL tree head

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_avl_tree_init(
    IN vtss_avl_tree_t  *ptree
)
{
    if ( ptree == NULL ) {
        VTSS_AVL_TREE_T_E("ptree == NULL\n");
        return FALSE;
    }

    if ( ptree->cmp_func == NULL ) {
        VTSS_AVL_TREE_T_E("ptree->cmp_func == NULL\n");
        return FALSE;
    }

    if ( ptree->max_node_cnt == 0 ) {
        VTSS_AVL_TREE_T_E("ptree->max_node_cnt == 0\n");
        return FALSE;
    }

    if ( ptree->free_nodes == NULL ) {
        VTSS_AVL_TREE_T_E("ptree->free_nodes == NULL\n");
        return FALSE;
    }

    _tree_init( ptree );
    return TRUE;
}

/*
    free all tree nodes and tree head is go back to initial state

    INPUT
        ptree - AVL tree head

    OUTPUT
        n/a

    RETURN
        n/a
*/
void vtss_avl_tree_destroy(
    IN vtss_avl_tree_t  *ptree
)
{
    return;
}

/*
    if new, then add
    if duplcate or no free node, return FALSE

    INPUT
        ptree - AVL tree
        pdata - data set into tree

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_avl_tree_add(
    IN vtss_avl_tree_t  *ptree,
    IN void             *pdata
)
{
    BOOL    b;
    void    *user_data;

    if ( ptree == NULL ) {
        VTSS_AVL_TREE_T_E("ptree == NULL\n");
        return FALSE;
    }

    if ( pdata == NULL ) {
        VTSS_AVL_TREE_T_E("pdata == NULL\n");
        return FALSE;
    }

    // add
    user_data = pdata;
    b = _tree_node_set(ptree, &user_data, TRUE);
    return b;
}

/*
    delete data from tree

    INPUT
        ptree  - AVL tree
        *pdata - data with index

    OUTPUT
        *pdata - the real user data removed from tree

    RETURN
        TRUE  - successful
        FALSE - failed, doesn't existe
*/
BOOL vtss_avl_tree_delete(
    IN    vtss_avl_tree_t   *ptree,
    INOUT void              **pdata
)
{
    vtss_avl_tree_node_t    *delete_node;
    void                    *output;

    if ( ptree == NULL ) {
        VTSS_AVL_TREE_T_E("ptree == NULL\n");
        return FALSE;
    }

    if ( pdata == NULL ) {
        VTSS_AVL_TREE_T_E("pdata == NULL\n");
        return FALSE;
    }

    if ( *pdata == NULL ) {
        VTSS_AVL_TREE_T_E("*pdata == NULL\n");
        return FALSE;
    }

    if ( ptree->proot == NULL ) {
        return FALSE;
    }

    delete_node = ptree->prev_get_node;
    if ( (delete_node == NULL) || IS_UNKNOWN(delete_node) || (delete_node->user_data != *pdata) ) {
        // find node
        delete_node = _tree_node_get(ptree, ptree->proot, *pdata);
        if ( delete_node == NULL ) {
            return FALSE;
        }
    } else {
        // use prev_get_node and reset it
        ptree->prev_get_node = NULL;
    }

    // get output
    output = delete_node->user_data;

    // delete node
    if ( _tree_node_delete(ptree, delete_node) == FALSE ) {
        return FALSE;
    }

    *pdata = output;
    return TRUE;
}

/*
    get function

    INPUT
        ptree  - AVL tree
        *pdata - data to get
        get    - get, get first, get next, get prev

    OUTPUT
        *pdata - data got

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_avl_tree_get(
    IN    vtss_avl_tree_t       *ptree,
    INOUT void                  **pdata,
    IN    vtss_avl_tree_get_t   get
)
{
    vtss_avl_tree_node_t    *data_node;
    vtss_avl_tree_node_t    *parent;

    /* parameter check */
    if ( ptree == NULL ) {
        VTSS_AVL_TREE_T_E("ptree == NULL\n");
        return FALSE;
    }

    if ( pdata == NULL ) {
        VTSS_AVL_TREE_T_E("pdata == NULL\n");
        return FALSE;
    }

    // empty tree
    if ( ptree->proot == NULL ) {
        return FALSE;
    }

    switch ( get ) {
    case VTSS_AVL_TREE_GET:
    case VTSS_AVL_TREE_GET_PREV:
    case VTSS_AVL_TREE_GET_NEXT:
        if ( *pdata == NULL ) {
            VTSS_AVL_TREE_T_E("*pdata == NULL\n");
            return FALSE;
        }
        break;

    case VTSS_AVL_TREE_GET_FIRST:
    case VTSS_AVL_TREE_GET_LAST:
        break;

    default:
        VTSS_AVL_TREE_T_E("invalid get = %d\n", get);
        return FALSE;
    }

    // use previous result
    data_node = ptree->prev_get_node;

    /* operation */
    switch ( get ) {
    case VTSS_AVL_TREE_GET:
        if ( (data_node == NULL) || IS_UNKNOWN(data_node) || (data_node->user_data != *pdata) ) {
            // find node
            data_node = _tree_node_get(ptree, ptree->proot, *pdata);
        }
        break;

    case VTSS_AVL_TREE_GET_FIRST:
        data_node = _min_node_get( ptree->proot );
        break;

    case VTSS_AVL_TREE_GET_LAST:
        data_node = _max_node_get( ptree->proot );
        break;

    case VTSS_AVL_TREE_GET_PREV:
        if ( (data_node == NULL) || IS_UNKNOWN(data_node) || (data_node->user_data != *pdata) ) {
            // find node
            data_node = _tree_node_get_prev(ptree, ptree->proot, *pdata);
        } else {
            // quick search from previous result
            if ( HAS_LCHILD(data_node) ) {
                // prev node at child
                data_node = _max_node_get( GET_LCHILD(data_node) );
            } else {
                // prev node at parent
                if ( IS_ROOT(data_node) ) {
                    data_node = NULL;
                } else if ( IS_RCHILD(data_node) ) {
                    data_node = GET_PARENT( data_node );
                } else {
                    for ( parent = GET_PARENT(data_node); parent; parent = GET_PARENT(parent) ) {
                        if ( IS_RCHILD(parent) ) {
                            parent = GET_PARENT( parent );
                            break;
                        }
                    }
                    data_node = parent;
                }
            }
        }
        break;

    case VTSS_AVL_TREE_GET_NEXT:
        if ( (data_node == NULL) || IS_UNKNOWN(data_node) || (data_node->user_data != *pdata) ) {
            // find node
            data_node = _tree_node_get_next(ptree, ptree->proot, *pdata);
        } else {
            // quick search from previous result
            if ( HAS_RCHILD(data_node) ) {
                // next node at child
                data_node = _min_node_get( GET_RCHILD(data_node) );
            } else {
                // next node at parent
                if ( IS_ROOT(data_node) ) {
                    data_node = NULL;
                } else if ( IS_LCHILD(data_node) ) {
                    data_node = GET_PARENT( data_node );
                } else {
                    for ( parent = GET_PARENT(data_node); parent; parent = GET_PARENT(parent) ) {
                        if ( IS_LCHILD(parent) ) {
                            parent = GET_PARENT( parent );
                            break;
                        }
                    }
                    data_node = parent;
                }
            }
        }
        break;

    default:
        return FALSE;
    }

    if ( data_node == NULL ) {
        return FALSE;
    }

    // keep result for quick operation
    ptree->prev_get_node = data_node;

    // output
    *pdata = data_node->user_data;
    return TRUE;
}
