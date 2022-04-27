/*

 Vitesse Switch API software.

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
#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "ip_source_guard_api.h"
#include "ip_source_guard.h"

#ifdef VTSS_LIB_DATA_STRUCT
#include "vtss_lib_data_struct_api.h"
#else
#include "vtss_avl_tree_api.h"
#endif

#include "port_api.h"
#include "acl_api.h"
#include "dhcp_snooping_api.h"
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"   // For topo_usid2isid(), topo_isid2usid()
#endif
#include <network.h>

#ifdef VTSS_SW_OPTION_VCLI
#include "ip_source_guard_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "ip_source_guard_icfg.h"
#endif


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "ip_guard",
    .descr     = "IP_SOURCE_GUARD"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

#define IP_SOURCE_GUARD_CRIT_ENTER() critd_enter(&ip_source_guard_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IP_SOURCE_GUARD_CRIT_EXIT()  critd_exit( &ip_source_guard_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IP_SOURCE_GUARD_CRIT_ENTER() critd_enter(&ip_source_guard_global.crit)
#define IP_SOURCE_GUARD_CRIT_EXIT()  critd_exit( &ip_source_guard_global.crit)
#endif /* VTSS_TRACE_ENABLED */

#define IP_SOURCE_GUARD_MAC_LENGTH  6
#define IP_SOURCE_GUARD_BUF_LENGTH  40

/* Global structure */
static ip_source_guard_global_t ip_source_guard_global;

static i32 _ip_source_guard_entry_compare_func(void *elm1, void *elm2);
static i32 _ip_source_guard_dynamic_entry_compare_func(void *elm1, void *elm2);
static vtss_rc _ip_source_guard_mgmt_conf_get_static_entry(ip_source_guard_entry_t *entry);
static vtss_rc _ip_source_guard_mgmt_conf_update_static_entry(ip_source_guard_entry_t *entry);

#ifdef VTSS_LIB_DATA_STRUCT
static  vtss_lib_data_struct_ctrl_header_t      static_ip_source_entry_list;
#else
VTSS_AVL_TREE(static_ip_source_entry_list_avlt, "IP_Source_Guard_static_avlt", VTSS_MODULE_ID_IP_SOURCE_GUARD, _ip_source_guard_entry_compare_func, IP_SOURCE_GUARD_MAX_ENTRY_CNT)
#endif
static  int     static_ip_source_entry_list_created_done = FALSE;

#ifdef VTSS_LIB_DATA_STRUCT
static  vtss_lib_data_struct_ctrl_header_t      dynamic_ip_source_entry_list;
#else
VTSS_AVL_TREE(dynamic_ip_source_entry_list_avlt, "IP_Source_Guard_dynamic_avlt", VTSS_MODULE_ID_IP_SOURCE_GUARD, _ip_source_guard_dynamic_entry_compare_func, IP_SOURCE_GUARD_MAX_ENTRY_CNT)
#endif
static  int     dynamic_ip_source_entry_list_created_done = FALSE;

/****************************************************************************/
/*  Various local functions with no semaphore protection                    */
/****************************************************************************/

static i32 _ip_source_guard_entry_compare_func(void *elm1, void *elm2)
{
    ip_source_guard_entry_t *element1, *element2;

    /* BODY
     */
    element1 = (ip_source_guard_entry_t *)elm1;
    element2 = (ip_source_guard_entry_t *)elm2;
    if (element1->isid > element2->isid) {
        return 1;
    } else if (element1->isid < element2->isid) {
        return -1;
    } else if (element1->port_no > element2->port_no) {
        return 1;
    } else if (element1->port_no < element2->port_no) {
        return -1;
    } else if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
        return -1;
    } else if (element1->assigned_ip > element2->assigned_ip) {
        return 1;
    } else if (element1->assigned_ip < element2->assigned_ip) {
        return -1;
    } else if (element1->ip_mask > element2->ip_mask) {
        return 1;
    } else if (element1->ip_mask < element2->ip_mask) {
        return -1;
    } else {
        return 0;
    }
}

static i32 _ip_source_guard_dynamic_entry_compare_func(void *elm1, void *elm2)
{
    ip_source_guard_entry_t *element1, *element2;

    /* BODY
     */
    element1 = (ip_source_guard_entry_t *)elm1;
    element2 = (ip_source_guard_entry_t *)elm2;
    if (element1->isid > element2->isid) {
        return 1;
    } else if (element1->isid < element2->isid) {
        return -1;
    } else if (element1->port_no > element2->port_no) {
        return 1;
    } else if (element1->port_no < element2->port_no) {
        return -1;
    } else if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
        return -1;
    } else if (element1->assigned_ip > element2->assigned_ip) {
        return 1;
    } else if (element1->assigned_ip < element2->assigned_ip) {
        return -1;
    } else {
        return 0;
    }
}

/* Add IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_add_static_entry(ip_source_guard_entry_t *entry, BOOL allocated)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    /* allocated memory */
    if (allocated) {
        if (_ip_source_guard_mgmt_conf_get_static_entry(entry) == VTSS_OK) {
            if (_ip_source_guard_mgmt_conf_update_static_entry(entry) != VTSS_OK) {
                T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
            }
        } else {
            /* Find an unused entry */
            for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
                if (ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i].valid) {
                    continue;
                }
                /* insert the entry on global memory for saving configuration */
                memcpy(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], entry, sizeof(ip_source_guard_entry_t));
                entry_p = &(ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i]);

                /* add the entry into static DB */
#ifdef VTSS_LIB_DATA_STRUCT
                if (vtss_lib_data_struct_set_entry(&static_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                    memset(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                    T_D("add the entry into static DB failed");
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
                }
#else
                if (vtss_avl_tree_add(&static_ip_source_entry_list_avlt, entry_p) != TRUE) {
                    memset(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                    T_D("add the entry into static DB failed");
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
                }
#endif
                break;
            }
        }
    } else {
        /* only insert the entry into link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_set_entry(&static_ip_source_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
        }
#else
        if (vtss_avl_tree_add(&static_ip_source_entry_list_avlt, entry) != TRUE) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
        }
#endif
    }

    return rc;
}

/* Update IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_update_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    ip_source_guard_entry_t *entry_p;
    int                     i;

    /* Find the exist entry */
    for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
        if (!ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i].valid) {
            continue;
        }

        entry_p = &(ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i]);
        T_N("i=%d, isid=%u , port_no=%u, vid=%u, assigned_ip=%u, ip_mask=%u", i, entry_p->isid, entry_p->port_no, entry_p->vid, entry_p->assigned_ip, entry_p->ip_mask);
        T_N("isid=%u , port_no=%u, vid=%u, assigned_ip=%u, ip_mask=%u", entry->isid, entry->port_no, entry->vid, entry->assigned_ip, entry->ip_mask);

        if (_ip_source_guard_entry_compare_func(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], entry) == 0) {
            /* update the entry on global memory */
            memcpy(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], entry, sizeof(ip_source_guard_entry_t));
            T_N("_ip_source_guard_mgmt_conf_update_static_entry success");
            break;
        }
    }

    return rc;
}

/* Delete IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_del_static_entry(ip_source_guard_entry_t *entry, BOOL free_node)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    if (free_node) {
        /* Find the exist entry */
        for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
            if (!ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i].valid) {
                continue;
            }

            if (_ip_source_guard_entry_compare_func(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], entry) == 0) {
                entry_p = &(ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i]);

#ifdef VTSS_LIB_DATA_STRUCT
                /* delete the entry on static DB */
                if (vtss_lib_data_struct_delete_entry(&static_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                }
#else
                /* delete the entry on static DB */
                if (vtss_avl_tree_delete(&static_ip_source_entry_list_avlt, (void **) &entry_p) != TRUE) {
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                }
#endif
                break;
            }
        }
    } else {
        /* only delete the entry on link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_entry(&static_ip_source_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#else
        if (vtss_avl_tree_delete(&static_ip_source_entry_list_avlt, (void **) &entry_p) != TRUE) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#endif
    }

    return rc;
}

/* Delete All IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_del_all_static_entry(BOOL free_node)
{
    vtss_rc rc = VTSS_OK;

    if (free_node) {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&static_ip_source_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        } else {
            /* clear global cache memory */
            memset(ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
        }
#else
        vtss_avl_tree_destroy(&static_ip_source_entry_list_avlt);
        if (vtss_avl_tree_init(&static_ip_source_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
        /* clear global cache memory */
        memset(ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
#endif
    } else {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&static_ip_source_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#else
        vtss_avl_tree_destroy(&static_ip_source_entry_list_avlt);
        if (vtss_avl_tree_init(&static_ip_source_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
#endif
    }

    return rc;
}

/* Get IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_static_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&static_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Get First IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_first_static_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&static_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_FIRST) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Get Next IP_SOURCE_GUARD static entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_next_static_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&static_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_NEXT) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Create IP_SOURCE_GUARD static data base */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _ip_source_guard_mgmt_conf_create_static_db(ulong max_entry_cnt, compare_func_cb_t compare_func)
{
    vtss_rc rc = VTSS_OK;

    if (!static_ip_source_entry_list_created_done) {
        /* Create data base for storing static entries */
        if (vtss_lib_data_struct_create_tree(&static_ip_source_entry_list, max_entry_cnt, sizeof(ip_source_guard_entry_t), compare_func, NULL, NULL, VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE)) {
            T_W("vtss_lib_data_struct_create_tree() failed");
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_CREATE;
        }
        static_ip_source_entry_list_created_done = TRUE;
    }

    return rc;
}
#else
static vtss_rc _ip_source_guard_mgmt_conf_create_static_db(void)
{
    vtss_rc rc = VTSS_OK;

    if (!static_ip_source_entry_list_created_done) {
        /* Create data base for storing static entries */
        if (vtss_avl_tree_init(&static_ip_source_entry_list_avlt)) {
            static_ip_source_entry_list_created_done = TRUE;
        } else {
            T_W("vtss_avl_tree_init() failed");
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_CREATE;
        }
    }

    return rc;
}
#endif

/* Add IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_add_dynamic_entry(ip_source_guard_entry_t *entry, BOOL allocated)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    /* allocated memory */
    if (allocated) {
        /* Find an unused entry */
        for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
            if (ip_source_guard_global.ip_source_guard_dynamic_entry[i].valid) {
                continue;
            }
            /* insert the entry on global memory */
            memcpy(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], entry, sizeof(ip_source_guard_entry_t));
            entry_p = &(ip_source_guard_global.ip_source_guard_dynamic_entry[i]);

            /* add the entry into dynamic DB */
#ifdef VTSS_LIB_DATA_STRUCT
            if (vtss_lib_data_struct_set_entry(&dynamic_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                memset(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                T_D("add the entry into dynamic DB failed");
                rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
            }
#else
            if (vtss_avl_tree_add(&dynamic_ip_source_entry_list_avlt, entry_p) != TRUE) {
                memset(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                T_D("add the entry into dynamic DB failed");
                rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
            }
#endif
            break;
        }
    } else {
        /* only insert the entry into link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_set_entry(&dynamic_ip_source_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
        }
#else
        if (vtss_avl_tree_add(&dynamic_ip_source_entry_list_avlt, entry) != TRUE) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_ADD;
        }
#endif
    }

    return rc;
}

/* Delete IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_del_dynamic_entry(ip_source_guard_entry_t *entry, BOOL free_node)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    if (free_node) {
        /* Find the exist entry */
        for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
            if (!ip_source_guard_global.ip_source_guard_dynamic_entry[i].valid) {
                continue;
            }

            if (_ip_source_guard_dynamic_entry_compare_func(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], entry) == 0) {
                entry_p = &(ip_source_guard_global.ip_source_guard_dynamic_entry[i]);

#ifdef VTSS_LIB_DATA_STRUCT
                /* delete the entry on dynamic DB */
                if (vtss_lib_data_struct_delete_entry(&dynamic_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                }
#else
                /* delete the entry on dynamic DB */
                if (vtss_avl_tree_delete(&dynamic_ip_source_entry_list_avlt, (void **) &entry_p) != TRUE) {
                    rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&ip_source_guard_global.ip_source_guard_dynamic_entry[i], 0x0, sizeof(ip_source_guard_entry_t));
                }
#endif
                break;
            }
        }
    } else {
        /* only delete the entry on link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_entry(&dynamic_ip_source_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#else
        if (vtss_avl_tree_delete(&dynamic_ip_source_entry_list_avlt, (void **) &entry_p) != TRUE) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#endif
    }

    return rc;
}

/* Delete All IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_del_all_dynamic_entry(BOOL free_node)
{
    vtss_rc rc = VTSS_OK;

    if (free_node) {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&dynamic_ip_source_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        } else {
            /* clear global cache memory */
            memset(ip_source_guard_global.ip_source_guard_dynamic_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
        }
#else
        vtss_avl_tree_destroy(&dynamic_ip_source_entry_list_avlt);
        if (vtss_avl_tree_init(&dynamic_ip_source_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
        /* clear global cache memory */
        memset(ip_source_guard_global.ip_source_guard_dynamic_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
#endif
    } else {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&dynamic_ip_source_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_DEL;
        }
#else
        vtss_avl_tree_destroy(&dynamic_ip_source_entry_list_avlt);
        if (vtss_avl_tree_init(&dynamic_ip_source_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
#endif
    }

    return rc;
}

/* Get IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_dynamic_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&dynamic_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Get First IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_first_dynamic_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&dynamic_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_FIRST) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Get Next IP_SOURCE_GUARD dynamic entry */
static vtss_rc _ip_source_guard_mgmt_conf_get_next_dynamic_entry(ip_source_guard_entry_t *entry)
{
    ip_source_guard_entry_t *entry_p;
    vtss_rc                 rc = VTSS_OK;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&dynamic_ip_source_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_ip_source_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_NEXT) != TRUE) {
        rc = IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    } else {
        memcpy(entry, entry_p, sizeof(ip_source_guard_entry_t));
    }
#endif

    return rc;
}

/* Create IP_SOURCE_GUARD dynamic data base */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _ip_source_guard_mgmt_conf_create_dynamic_db(ulong max_entry_cnt, compare_func_cb_t compare_func)
{
    vtss_rc rc = VTSS_OK;

    if (!dynamic_ip_source_entry_list_created_done) {
        /* Create data base for storing static entries */
        if (vtss_lib_data_struct_create_tree(&dynamic_ip_source_entry_list, max_entry_cnt, sizeof(ip_source_guard_entry_t), compare_func, NULL, NULL, VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE)) {
            T_W("vtss_lib_data_struct_create_tree() failed");
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_CREATE;
        }
        dynamic_ip_source_entry_list_created_done = TRUE;
    }

    return rc;
}
#else
static vtss_rc _ip_source_guard_mgmt_conf_create_dynamic_db(void)
{
    vtss_rc rc = VTSS_OK;

    if (!dynamic_ip_source_entry_list_created_done) {
        /* Create data base for storing static entries */
        if (vtss_avl_tree_init(&dynamic_ip_source_entry_list_avlt)) {
            dynamic_ip_source_entry_list_created_done = TRUE;
        } else {
            T_W("vtss_avl_tree_init() failed");
            rc = IP_SOURCE_GUARD_ERROR_DATABASE_CREATE;
        }
    }

    return rc;
}
#endif

/* Translate IP_SOURCE_GUARD dynamic entries into static entries */
static vtss_rc _ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* avoid inserting the null ace id on static db */
    if (entry->ace_id == ACE_ID_NONE) {
        T_W("ace id is empty");
        return IP_SOURCE_GUARD_ERROR_INV_PARAM;
    }

    /* add the entry into static DB */
    if ((rc = _ip_source_guard_mgmt_conf_add_static_entry(entry, TRUE)) != VTSS_OK) {
        T_D("_ip_source_guard_mgmt_conf_add_static_entry() failed");
        T_D("exit");
        return rc;
    } else {
        /* delete the entry on dynamic DB */
        if ((rc = _ip_source_guard_mgmt_conf_del_dynamic_entry(entry, TRUE)) != VTSS_OK) {
            T_D("_ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
        }
    }

    T_D("exit");
    return rc;
}

/* Save IP_SOURCE_GUARD configuration */
static vtss_rc _ip_source_guard_mgmt_conf_save_static_configuration(void)
{
    vtss_rc                     rc = VTSS_OK;
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t               blk_id = CONF_BLK_IP_SOURCE_GUARD_CONF;
    ip_source_guard_conf_blk_t  *ip_source_guard_conf_blk_p;

    T_D("enter");

    /* Save changed configuration */
    if ((ip_source_guard_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open IP_SOURCE_GUARD table");
        rc = IP_SOURCE_GUARD_ERROR_LOAD_CONF;
    } else {
        memcpy(ip_source_guard_conf_blk_p->ip_source_guard_conf.ip_source_guard_static_entry, ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* IP_SOURCE_GUARD entry count */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _ip_source_guard_entry_count(void)
{
    vtss_rc rc = VTSS_OK;

    rc = static_ip_source_entry_list.current_cnt + dynamic_ip_source_entry_list.current_cnt;

    return rc;
}
#else
static vtss_rc _ip_source_guard_entry_count(void)
{
    vtss_rc rc = VTSS_OK;
    int     i;

    /* Find the exist entry on static DB */
    for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
        if (!ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i].valid) {
            continue;
        }
        rc++;
    }

    /* Find the exist entry on dynamic DB */
    for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
        if (!ip_source_guard_global.ip_source_guard_dynamic_entry[i].valid) {
            continue;
        }
        rc++;
    }

    return rc;
}
#endif

/* Get configuration from per port dynamic entry count */
static vtss_rc _ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(vtss_isid_t isid, ip_source_guard_port_dynamic_entry_conf_t *port_dynamic_entry_conf)
{
    vtss_rc         rc = VTSS_OK;
    port_iter_t     pit;

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        port_dynamic_entry_conf->entry_cnt[pit.iport] = ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[pit.iport];
    }

    return rc;
}

/* Get current port dynamic entry count */
static vtss_rc _ip_source_guard_mgmt_conf_get_current_port_dynamic_entry_cnt(vtss_isid_t isid, vtss_port_no_t port_no, ulong *current_port_dynamic_entry_cnt_p)
{
    ip_source_guard_entry_t entry;
    ulong                   current_port_entry_cnt = 0;

    if (_ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if ((entry.isid == isid) && (entry.port_no == port_no)) {
            current_port_entry_cnt++;
        }
        while (_ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if ((entry.isid == isid) && (entry.port_no == port_no)) {
                current_port_entry_cnt++;
            }
        }
    }
    *current_port_dynamic_entry_cnt_p = current_port_entry_cnt;

    return VTSS_OK;
}

/* check is reach IP_SOURCE_GUARD max port dynamic entry count */
static BOOL _ip_source_guard_is_reach_max_port_dynamic_entry_cnt(ip_source_guard_entry_t *entry)
{
    ip_source_guard_port_dynamic_entry_conf_t   port_dynamic_entry_conf;
    ulong                                       max_dynamic_entry_cnt, current_port_dynamic_entry_cnt = 0;
#ifdef VTSS_SW_OPTION_SYSLOG
    char ip_txt[IP_SOURCE_GUARD_BUF_LENGTH], mac_txt[IP_SOURCE_GUARD_BUF_LENGTH];
    char syslog_txt[256], *syslog_txt_p = &syslog_txt[0];
#endif /* VTSS_SW_OPTION_SYSLOG */

    max_dynamic_entry_cnt = _ip_source_guard_entry_count();

    if (max_dynamic_entry_cnt >= IP_SOURCE_GUARD_MAX_ENTRY_CNT) {
#ifdef VTSS_SW_OPTION_SYSLOG
        syslog_txt_p += sprintf(syslog_txt_p, "Dynamic IP Source Guard Table full. Could not add entry <VLAN ID = %d, ", entry->vid);
#if VTSS_SWITCH_STACKABLE
        syslog_txt_p += sprintf(syslog_txt_p, ", Switch ID = %d", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
        syslog_txt_p += sprintf(syslog_txt_p, " Port = %d", iport2uport(entry->port_no));
        syslog_txt_p += sprintf(syslog_txt_p, ", IP Address = %s, MAC Address = %s> to table",
                                misc_ipv4_txt(entry->assigned_ip, ip_txt),
                                misc_mac_txt(entry->assigned_mac, mac_txt));
        S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
        return TRUE;
    }

    if (_ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(entry->isid, &port_dynamic_entry_conf) != VTSS_OK ||
        _ip_source_guard_mgmt_conf_get_current_port_dynamic_entry_cnt(entry->isid, entry->port_no, &current_port_dynamic_entry_cnt) != VTSS_OK) {
        return FALSE;
    }
    if (current_port_dynamic_entry_cnt == port_dynamic_entry_conf.entry_cnt[entry->port_no]) {
#ifdef VTSS_SW_OPTION_SYSLOG
        syslog_txt_p += sprintf(syslog_txt_p, "Dynamic IP Source Guard Table reach port limitation(%d). Could not add entry <VLAN ID = %d, ", current_port_dynamic_entry_cnt, entry->vid);
#if VTSS_SWITCH_STACKABLE
        syslog_txt_p += sprintf(syslog_txt_p, ", Switch ID = %d", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
        syslog_txt_p += sprintf(syslog_txt_p, " Port = %d", iport2uport(entry->port_no));
        syslog_txt_p += sprintf(syslog_txt_p, ", IP Address = %s, MAC Address = %s> to table",
                                misc_ipv4_txt(entry->assigned_ip, ip_txt),
                                misc_mac_txt(entry->assigned_mac, mac_txt));
        S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Set IP_SOURCE_GUARD defaults */
static void _ip_source_guard_default_set(ip_source_guard_conf_t *conf)
{
    int isid, j;

    memset(conf, 0x0, sizeof(*conf));
    conf->mode = IP_SOURCE_GUARD_DEFAULT_MODE;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        for (j = VTSS_PORT_NO_START; j < VTSS_PORT_NO_END; j++) {
            if (port_isid_port_no_is_stack(isid, j)) {
                conf->port_mode_conf[isid - VTSS_ISID_START].mode[j] = IP_SOURCE_GUARD_MGMT_DISABLED;
                conf->port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[j] = IP_SOURCE_GUARD_DYNAMIC_UNLIMITED;
            } else {
                conf->port_mode_conf[isid - VTSS_ISID_START].mode[j] = IP_SOURCE_GUARD_DEFAULT_PORT_MODE;
                conf->port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[j] = IP_SOURCE_GUARD_DEFAULT_DYNAMIC_ENTRY_CNT;
            }
        }
    }

    /* clear global cache memory */
    memset(conf->ip_source_guard_static_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
    T_D("clear static entries on global cache memory");

    return;
}

/* Set IP_SOURCE_GUARD defaults for dynamic entries */
static void _ip_source_guard_default_set_dynamic_entries(void)
{
    /* clear global cache memory */
    memset(ip_source_guard_global.ip_source_guard_dynamic_entry, 0x0, IP_SOURCE_GUARD_MAX_ENTRY_CNT * sizeof(ip_source_guard_entry_t));
    T_D("clear dynamic entries on global cache memory");

    return;
}

/****************************************************************************/
/*  Reserved ACEs functions                                                 */
/****************************************************************************/
/* Add port default ACE for IP source guard.
   Add a port rule for deny all frames to the end of ACL */
static vtss_rc _ip_source_guard_default_ace_add(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_rc             rc;
    acl_entry_conf_t    conf;

#if defined(VTSS_FEATURE_ACL_V2)
    /* We use the new parameter of portlist on ACL V2 */
    if (acl_mgmt_ace_get(ACL_USER_IP_SOURCE_GUARD, VTSS_ISID_GLOBAL, IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, VTSS_PORT_NO_START), &conf, NULL, FALSE) == VTSS_OK) {
        conf.port_list[port_no] = TRUE;
        return acl_mgmt_ace_add(ACL_USER_IP_SOURCE_GUARD, ACE_ID_NONE, &conf);
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV4, &conf)) != VTSS_OK) {
        return rc;
    }
    conf.isid                       = isid;
#if defined(VTSS_FEATURE_ACL_V2)
    conf.id                         = IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, VTSS_PORT_NO_START);
    conf.action.port_action         = VTSS_ACL_PORT_ACTION_FILTER;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
    memset(conf.port_list, 0, sizeof(conf.port_list));
    conf.port_list[port_no]         = TRUE;
#else
    conf.id                         = IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, port_no);
    conf.action.permit              = FALSE;
    conf.action.port_no             = VTSS_PORT_NO_NONE;
    conf.port_no                    = port_no;
#endif /* VTSS_FEATURE_ACL_V2 */
    return acl_mgmt_ace_add(ACL_USER_IP_SOURCE_GUARD, ACE_ID_NONE, &conf);
}

static vtss_rc _ip_source_guard_default_ace_del(vtss_isid_t isid, vtss_port_no_t port_no)
{
#if defined(VTSS_FEATURE_ACL_V2)
    acl_entry_conf_t    conf;
    port_iter_t         pit;

    if (acl_mgmt_ace_get(ACL_USER_IP_SOURCE_GUARD, VTSS_ISID_GLOBAL, IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, VTSS_PORT_NO_START), &conf, NULL, FALSE) == VTSS_OK) {
        int            port_cnt = 0;

        conf.port_list[port_no] = FALSE;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (conf.port_list[pit.iport] == TRUE) {
                port_cnt++;
            }
        }
        if (port_cnt) {
            return acl_mgmt_ace_add(ACL_USER_IP_SOURCE_GUARD, ACE_ID_NONE, &conf);
        } else {
            return (acl_mgmt_ace_del(ACL_USER_IP_SOURCE_GUARD, IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, VTSS_PORT_NO_START)));
        }
    }
    return VTSS_OK;
#else
    return (acl_mgmt_ace_del(ACL_USER_IP_SOURCE_GUARD, IP_SOURCE_GUARD_DEFAULT_ACE_ID(isid, port_no)));
#endif /* VTSS_FEATURE_ACL_V2 */
}

/* Alloc IP source guard ACE ID */
static BOOL _ip_source_guard_ace_alloc(vtss_ace_id_t *id)
{
    int i, found = FALSE;

    /* Get next available ID */
    for (i = VTSS_ISID_CNT * VTSS_PORTS + 1; i <= ACE_ID_END; i++) {
        if (!VTSS_BF_GET(ip_source_guard_global.id_used, i - ACE_ID_START)) {
            found = TRUE;
            *id = (vtss_ace_id_t) i;
            VTSS_BF_SET(ip_source_guard_global.id_used, i - ACE_ID_START, 1);
            break;
        }
    }

    if (found) {
        return TRUE;
    } else {
        T_W("ACE Auto-assigned fail");
        return FALSE;
    }
}

/* Free IP source guard ACE ID
   Free all if id = ACE_ID_NONE */
static void _ip_source_guard_ace_free(vtss_ace_id_t id)
{
    if (id == ACE_ID_NONE) {
        memset(ip_source_guard_global.id_used, 0x0, sizeof(ip_source_guard_global.id_used));
    } else {
        VTSS_BF_SET(ip_source_guard_global.id_used, id - ACE_ID_START, 0);
    }
}

/* Add reserved ACE */
static vtss_rc _ip_source_guard_ace_add(ip_source_guard_entry_t *entry)
{
    acl_entry_conf_t    conf;
    vtss_rc             rc;
    int                 alloc_flag = FALSE;

    /* Check switch ID */
    if (!msg_switch_configurable(entry->isid)) {
        T_D("isid: %d not exist", entry->isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID;
    }

    if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV4, &conf)) != VTSS_OK) {
        return rc;
    }
    if (entry->ace_id == ACE_ID_NONE) {
        if (!_ip_source_guard_ace_alloc(&conf.id)) {
            return IP_SOURCE_GUARD_ERROR_ACE_AUTO_ASSIGNED_FAIL;
        }
        alloc_flag = TRUE;
    }
    conf.isid                       = entry->isid;
#if defined(VTSS_FEATURE_ACL_V2)
    memset(conf.port_list, 0, sizeof(conf.port_list));
    conf.port_list[entry->port_no]  = TRUE;
#else
    conf.action.permit              = TRUE;
    conf.action.port_no             = VTSS_PORT_NO_NONE;
    conf.port_no                    = entry->port_no;
#endif /* VTSS_FEATURE_ACL_V2 */
    conf.vid.mask                   = 0xFFF;
    conf.vid.value                  = entry->vid;
#if defined(VTSS_FEATURE_ACL_V1)
    conf.frame.ipv4.sip.value       = entry->assigned_ip;
    conf.frame.ipv4.sip.mask        = entry->ip_mask;
#endif /* VTSS_FEATURE_ACL_V1 */
#if defined(VTSS_FEATURE_ACL_V2)
    conf.frame.ipv4.sip_smac.enable = TRUE;
    conf.frame.ipv4.sip_smac.sip = entry->assigned_ip;
    memcpy(conf.frame.ipv4.sip_smac.smac.addr, entry->assigned_mac, IP_SOURCE_GUARD_MAC_LENGTH);
    /* We always use parameter of 'VTSS_PORT_NO_START' to get the port's default ACE */
    if ((rc = acl_mgmt_ace_add(ACL_USER_IP_SOURCE_GUARD, IP_SOURCE_GUARD_DEFAULT_ACE_ID(entry->isid, VTSS_PORT_NO_START), &conf)) == VTSS_OK) {
#else
    if ((rc = acl_mgmt_ace_add(ACL_USER_IP_SOURCE_GUARD, IP_SOURCE_GUARD_DEFAULT_ACE_ID(entry->isid, entry->port_no), &conf)) == VTSS_OK) {
#endif /* VTSS_FEATURE_ACL_V2 */
        entry->ace_id = conf.id;
    } else {
        entry->ace_id = ACE_ID_NONE;
        if (alloc_flag) {
            _ip_source_guard_ace_free(conf.id);
        }
    }
    return rc;
}

/* Delete reserved ACE */
static vtss_rc _ip_source_guard_ace_del(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = acl_mgmt_ace_del(ACL_USER_IP_SOURCE_GUARD, entry->ace_id);

    T_N("ace_id = %u", entry->ace_id);

    if (rc == VTSS_OK) {
        _ip_source_guard_ace_free(entry->ace_id);
        entry->ace_id = ACE_ID_NONE;
        T_N("ACE_ID_NONE");
    }

    return rc;
}

/* Clear all ACEs of IP source guard */
static void _ip_source_guard_ace_clear(void)
{
    acl_entry_conf_t ace_conf;

    _ip_source_guard_ace_free(ACE_ID_NONE);

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        return;
    }

    while (acl_mgmt_ace_get(ACL_USER_IP_SOURCE_GUARD, VTSS_ISID_GLOBAL, ACE_ID_NONE, &ace_conf, NULL, 1) == VTSS_OK) {
        if (acl_mgmt_ace_del(ACL_USER_IP_SOURCE_GUARD, ace_conf.id)) {
            T_W("acl_mgmt_ace_del(ACL_USER_IP_SOURCE_GUARD, %d) failed", ace_conf.id);
        }
    }
}

/****************************************************************************/
/*  Dynamic entry functions                                                 */
/****************************************************************************/
/* Get first IP_SOURCE_GUARD dynamic entry */
vtss_rc ip_source_guard_mgmt_conf_get_first_dynamic_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    rc = _ip_source_guard_mgmt_conf_get_first_dynamic_entry(entry);
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Get Next IP_SOURCE_GUARD dynamic entry */
vtss_rc ip_source_guard_mgmt_conf_get_next_dynamic_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    rc = _ip_source_guard_mgmt_conf_get_next_dynamic_entry(entry);
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Set IP_SOURCE_GUARD dynamic entry */
static vtss_rc ip_source_guard_mgmt_conf_set_dynamic_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_exists(entry->isid)) {
        T_D("isid: %d not exist", entry->isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();

    /* Check system mode is enabled */
    if (ip_source_guard_global.ip_source_guard_conf.mode == IP_SOURCE_GUARD_MGMT_DISABLED) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* Check port mode is enabled */
    if (ip_source_guard_global.ip_source_guard_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == IP_SOURCE_GUARD_MGMT_DISABLED) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* Check the entry exist or not ? */
    if ((rc = _ip_source_guard_mgmt_conf_get_dynamic_entry(entry)) != IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if existing, return the event
        T_D("the entry existing on dynamic db, exit, rc=%d", rc);
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ENTRY_EXIST_ON_DB;
    }
    if ((rc = _ip_source_guard_mgmt_conf_get_static_entry(entry)) != IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if existing, return the event
        T_D("the entry existing on static db, exit, rc=%d", rc);
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ENTRY_EXIST_ON_DB;
    }

    /* Check total count reach the max value or not? */
    if (_ip_source_guard_is_reach_max_port_dynamic_entry_cnt(entry)) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        T_W("ip source guard: port entry full or table full.");
        return IP_SOURCE_GUARD_ERROR_DYNAMIC_TABLE_FULL;
    }

    entry->valid = TRUE;
    entry->ace_id = ACE_ID_NONE;
    entry->type = IP_SOURCE_GUARD_DYNAMIC_TYPE;

    /* add the entry on ACL */
    if ((rc = _ip_source_guard_ace_add(entry)) != VTSS_OK) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_W("_ip_source_guard_ace_add() failed");
        T_D("exit");
        return rc;
    } else {
        /* add the entry into dynamic DB */
        if ((rc = _ip_source_guard_mgmt_conf_add_dynamic_entry(entry, TRUE)) != VTSS_OK) {

            /* check ACL inserted or not  */
            if (entry->ace_id != ACE_ID_NONE) {
                /* delete the ace entry */
                if (_ip_source_guard_ace_del(entry) != VTSS_OK) {
                    T_D("_ip_source_guard_ace_del() failed");
                }
            }
        }
    }

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Set IP_SOURCE_GUARD dynamic entry */
static vtss_rc ip_source_guard_mgmt_conf_del_dynamic_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_exists(entry->isid)) {
        T_D("isid: %d not exist", entry->isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();

    /* Check system mode is enabled */
    if (ip_source_guard_global.ip_source_guard_conf.mode == IP_SOURCE_GUARD_MGMT_DISABLED) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        return rc;
    }

    /* Check port mode is enabled */
    if (ip_source_guard_global.ip_source_guard_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == IP_SOURCE_GUARD_MGMT_DISABLED) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        return rc;
    }

    /* Check the entry exist or not ? */
    if ((rc = _ip_source_guard_mgmt_conf_get_dynamic_entry(entry)) == IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if not existing, return
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* check ACL inserted or not */
    if (entry->valid && entry->ace_id != ACE_ID_NONE) {
        /* delete the entry on ACL */
        if (_ip_source_guard_ace_del(entry)) {
            T_D("_ip_source_guard_ace_del() failed");
        }
    }

    /* delete the entry on dynamic DB */
    if ((rc = _ip_source_guard_mgmt_conf_del_dynamic_entry(entry, TRUE)) != VTSS_OK) {
        T_D("_ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
    }

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* flush IP_SOURCE_GUARD dynamic entry by port */
static vtss_rc ip_source_guard_mgmt_conf_flush_dynamic_entry_by_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    vtss_rc                 rc = VTSS_OK;
    ip_source_guard_entry_t entry;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    if (_ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if (entry.isid == isid && entry.port_no == port_no) {
            if (entry.valid && entry.ace_id != ACE_ID_NONE) {
                if (_ip_source_guard_ace_del(&entry)) {
                    T_D("_ip_source_guard_ace_del() failed");
                }
            }

            /* delete the entry on dynamic DB */
            if ((rc = _ip_source_guard_mgmt_conf_del_dynamic_entry(&entry, TRUE)) != VTSS_OK) {
                T_D("_ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
            }
        }
        while (_ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if (entry.isid == isid && entry.port_no == port_no) {
                if (entry.valid && entry.ace_id != ACE_ID_NONE) {
                    if (_ip_source_guard_ace_del(&entry)) {
                        T_D("_ip_source_guard_ace_del() failed");
                    }
                }

                /* delete the entry on dynamic DB */
                if ((rc = _ip_source_guard_mgmt_conf_del_dynamic_entry(&entry, TRUE)) != VTSS_OK) {
                    T_D("_ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
                }
            }
        }
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* del first dynamic entry with port id */
static vtss_rc ip_source_guard_del_first_dynamic_entry_with_specific_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    ip_source_guard_entry_t     entry;
    int                         del_done = FALSE;

#ifdef VTSS_SW_OPTION_SYSLOG
    char ip_txt[IP_SOURCE_GUARD_BUF_LENGTH], mac_txt[IP_SOURCE_GUARD_BUF_LENGTH];
    char syslog_txt[256], *syslog_txt_p = &syslog_txt[0];
#endif

    if (ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if ((entry.isid == isid) && (entry.port_no == port_no)) {
            if (ip_source_guard_mgmt_conf_del_dynamic_entry(&entry)) {
                T_W("ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
            }
            /* Fill system log which entry had been eliminated */
#ifdef VTSS_SW_OPTION_SYSLOG
            syslog_txt_p += sprintf(syslog_txt_p, "Dynamic IP Source Guard Table port limitation changed. should delete entry <VLAN ID = %d,", entry.vid);
#if VTSS_SWITCH_STACKABLE
            syslog_txt_p += sprintf(syslog_txt_p, ", Switch ID = %d", topo_isid2usid(entry.isid));
#endif /* VTSS_SWITCH_STACKABLE */
            syslog_txt_p += sprintf(syslog_txt_p, " Port = %d", iport2uport(entry.port_no));
            syslog_txt_p += sprintf(syslog_txt_p, ", IP Address = %s, MAC Address = %s> to table",
                                    misc_ipv4_txt(entry.assigned_ip, ip_txt),
                                    misc_mac_txt(entry.assigned_mac, mac_txt));
            S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
            del_done = TRUE;
        }
        while (ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if ((entry.isid == isid) && (entry.port_no == port_no)) {
                if (ip_source_guard_mgmt_conf_del_dynamic_entry(&entry)) {
                    T_W("ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
                }
                /* Fill system log which entry had been eliminated */
#ifdef VTSS_SW_OPTION_SYSLOG
                syslog_txt_p += sprintf(syslog_txt_p, "Dynamic IP Source Guard Table port limitation changed. should delete entry <VLAN ID = %d,", entry.vid);
#if VTSS_SWITCH_STACKABLE
                syslog_txt_p += sprintf(syslog_txt_p, ", Switch ID = %d", topo_isid2usid(entry.isid));
#endif /* VTSS_SWITCH_STACKABLE */
                syslog_txt_p += sprintf(syslog_txt_p, " Port = %d", iport2uport(entry.port_no));
                syslog_txt_p += sprintf(syslog_txt_p, ", IP Address = %s, MAC Address = %s> to table",
                                        misc_ipv4_txt(entry.assigned_ip, ip_txt),
                                        misc_mac_txt(entry.assigned_mac, mac_txt));
                S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
                del_done = TRUE;
            }
        }
    }
    if (del_done) {
        return VTSS_OK;
    } else {
        return IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS;
    }
}

/* Get current port dynamic entry count */
static vtss_rc ip_source_guard_mgmt_conf_get_current_port_dynamic_entry_cnt(vtss_isid_t isid, vtss_port_no_t port_no, ulong *current_port_dynamic_entry_cnt_p)
{
    ip_source_guard_entry_t entry;
    ulong                   current_port_entry_cnt = 0;

    if (ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if ((entry.isid == isid) && (entry.port_no == port_no)) {
            current_port_entry_cnt++;
        }
        while (ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if ((entry.isid == isid) && (entry.port_no == port_no)) {
                current_port_entry_cnt++;
            }
        }
    }
    *current_port_dynamic_entry_cnt_p = current_port_entry_cnt;

    return VTSS_OK;
}

/* Eliminated dynamic entry */
static void ip_source_guard_eliminated_dynamic_entry(vtss_isid_t isid, vtss_port_no_t port_no, ulong entry_cnt)
{
    //1. get current entry cnt
    //2. entry_cnt is user config vlaue
    //3. we should del (current entry cnt - user config vlaue) entriese from list

    ulong   current_port_entry_cnt;
    int     extra_entry_cnt;

    if (ip_source_guard_mgmt_conf_get_current_port_dynamic_entry_cnt(isid, port_no, &current_port_entry_cnt)) {
        T_W("ip_source_guard_mgmt_conf_get_current_port_dynamic_entry_cnt() failed");
    }

    extra_entry_cnt = current_port_entry_cnt - entry_cnt;
    if (extra_entry_cnt > 0) {
        while (extra_entry_cnt) {
            if (ip_source_guard_del_first_dynamic_entry_with_specific_port(isid, port_no)) {
                T_W("ip_source_guard_del_first_dynamic_entry_with_specific_port() failed");
            }
            extra_entry_cnt--;
        }
    }

    return;
}

/* set port dynamic entry cnt */
vtss_rc ip_source_guard_mgmt_conf_set_port_dynamic_entry_cnt(vtss_isid_t isid, ip_source_guard_port_dynamic_entry_conf_t *port_dynamic_entry_conf)
{
    vtss_rc     rc = VTSS_OK;
    ulong       port_conf_change[VTSS_PORTS];
    ulong       global_mode, port_mode;
    int         changed = FALSE;
    port_iter_t pit;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    memset(port_conf_change, 0x0, sizeof(port_conf_change));

    IP_SOURCE_GUARD_CRIT_ENTER();
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[pit.iport] != port_dynamic_entry_conf->entry_cnt[pit.iport] &&
            ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[pit.iport] > port_dynamic_entry_conf->entry_cnt[pit.iport]) {
            port_conf_change[pit.iport] = TRUE;
        }
        ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[pit.iport] = port_dynamic_entry_conf->entry_cnt[pit.iport];
    }
    changed = TRUE;
    IP_SOURCE_GUARD_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t              blk_id  = CONF_BLK_IP_SOURCE_GUARD_CONF;
        ip_source_guard_conf_blk_t *ip_source_guard_conf_blk_p;
        if ((ip_source_guard_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open IP source guard table");
        } else {
            IP_SOURCE_GUARD_CRIT_ENTER();
            memcpy(ip_source_guard_conf_blk_p->ip_source_guard_conf.port_dynamic_entry_conf, ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf, VTSS_ISID_CNT * sizeof(ip_source_guard_port_dynamic_entry_conf_t));
            IP_SOURCE_GUARD_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        IP_SOURCE_GUARD_CRIT_ENTER();
        global_mode = ip_source_guard_global.ip_source_guard_conf.mode;
        IP_SOURCE_GUARD_CRIT_EXIT();

        if (global_mode == IP_SOURCE_GUARD_MGMT_ENABLED) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (port_conf_change[pit.iport] == FALSE) {
                    continue;
                }
                IP_SOURCE_GUARD_CRIT_ENTER();
                port_mode = ip_source_guard_global.ip_source_guard_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport];
                IP_SOURCE_GUARD_CRIT_EXIT();
                if (port_mode == IP_SOURCE_GUARD_MGMT_ENABLED) {
                    ip_source_guard_eliminated_dynamic_entry(isid, pit.iport, port_dynamic_entry_conf->entry_cnt[pit.iport]);
                }
            }
        }
    }

    T_D("exit");
    return rc;
}

vtss_rc ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(vtss_isid_t isid, ip_source_guard_port_dynamic_entry_conf_t *port_dynamic_entry_conf)
{
    vtss_rc         rc = VTSS_OK;
    port_iter_t     pit;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    //(void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        port_dynamic_entry_conf->entry_cnt[pit.iport] = ip_source_guard_global.ip_source_guard_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[pit.iport];
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/****************************************************************************/
/*  Register functions                                                      */
/****************************************************************************/

/* Register DHCP receive */
static void ip_source_guard_dhcp_pkt_receive(dhcp_snooping_ip_assigned_info_t *info, dhcp_snooping_info_reason_t reason)
{
    ip_source_guard_entry_t entry;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        return;
    }

    memset(&entry, 0x0, sizeof(entry));
    entry.vid = info->vid;
    memcpy(entry.assigned_mac, info->mac, IP_SOURCE_GUARD_MAC_LENGTH);
    entry.assigned_ip = info->assigned_ip;
    entry.ip_mask = 0xFFFFFFFF;
    entry.isid = info->isid;
    entry.port_no = info->port_no;
    entry.type = IP_SOURCE_GUARD_DYNAMIC_TYPE;
    if (reason == DHCP_SNOOPING_INFO_REASON_ASSIGN_COMPLETED) {
        entry.valid = TRUE;
        if (ip_source_guard_mgmt_conf_set_dynamic_entry(&entry)) {
            T_D("ip_source_guard_mgmt_conf_set_dynamic_entry() failed");
        }
    } else {
        entry.valid = FALSE;
        if (ip_source_guard_mgmt_conf_del_dynamic_entry(&entry)) {
            T_D("ip_source_guard_mgmt_conf_del_dynamic_entry() failed");
        }
    }

    return;
}

/****************************************************************************/
/*  IP source guard configuration                                           */
/****************************************************************************/

/* Determine if IP_SOURCE_GUARD configuration has changed */
static int ip_source_guard_conf_changed(ip_source_guard_conf_t *old, ip_source_guard_conf_t *new)
{
    return (memcmp(old, new, sizeof(*new)));
}

/* Apply IP source guard configuration */
static void ip_source_guard_conf_apply(void)
{
    ip_source_guard_entry_t             entry;
    uchar                               mac[IP_SOURCE_GUARD_MAC_LENGTH], null_mac[IP_SOURCE_GUARD_MAC_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    vtss_vid_t                          vid;
    dhcp_snooping_ip_assigned_info_t    info;
    ip_source_guard_port_mode_conf_t    *port_mode_conf;
    switch_iter_t                       sit;
    port_iter_t                         pit;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_D("not master");
        return;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    if (ip_source_guard_global.ip_source_guard_conf.mode == IP_SOURCE_GUARD_MGMT_ENABLED) {
        port_mode_conf = ip_source_guard_global.ip_source_guard_conf.port_mode_conf;

        /* Set default ACE */
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            if (!msg_switch_exists(sit.isid)) {
                continue;
            }

            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (port_mode_conf[sit.isid - VTSS_ISID_START].mode[pit.iport] == IP_SOURCE_GUARD_MGMT_DISABLED) {
                    continue;
                }
                if (_ip_source_guard_default_ace_add(sit.isid, pit.iport)) {
                    T_W("_ip_source_guard_default_ace_add() failed");
                }
            }
        }

        /* Get information from DHCP snooping */
        memset(&entry, 0x0 , sizeof(entry));
        memcpy(mac, null_mac, sizeof(mac));
        vid = 0;
        while (dhcp_snooping_ip_assigned_info_getnext(mac, vid, &info)) {
            memcpy(mac, info.mac, IP_SOURCE_GUARD_MAC_LENGTH);
            vid = info.vid;
            if (!msg_switch_exists(info.isid) ||
                port_mode_conf[info.isid - VTSS_ISID_START].mode[info.port_no] == IP_SOURCE_GUARD_MGMT_DISABLED) {
                continue;
            }
            entry.vid = info.vid;
            memcpy(entry.assigned_mac, info.mac, IP_SOURCE_GUARD_MAC_LENGTH);
            entry.assigned_ip = info.assigned_ip;
            entry.ip_mask = 0xFFFFFFFF;
            entry.isid = info.isid;
            entry.port_no = info.port_no;
            entry.type = IP_SOURCE_GUARD_DYNAMIC_TYPE;
            entry.ace_id = ACE_ID_NONE;
            entry.valid = TRUE;

            /* Check if reach max count */
            if (_ip_source_guard_is_reach_max_port_dynamic_entry_cnt(&entry)) {
                break;
            }

            /* Add ACE for dynamic entry */
            if (_ip_source_guard_mgmt_conf_get_static_entry(&entry) != VTSS_OK) {
                if (_ip_source_guard_ace_add(&entry)) {
                    T_W("_ip_source_guard_ace_add() failed");
                } else {
                    if (_ip_source_guard_mgmt_conf_add_dynamic_entry(&entry, TRUE) != VTSS_OK) {
                        T_W("_ip_source_guard_mgmt_conf_add_dynamic_entry() failed");
                    }
                }
            }
        };

        /* Add ACEs for static entries */
        if (_ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
            if (port_mode_conf[entry.isid - VTSS_ISID_START].mode[entry.port_no] == IP_SOURCE_GUARD_MGMT_ENABLED) {
                if (entry.valid /* && entry.ace_id == ACE_ID_NONE */ && msg_switch_exists(entry.isid)) {
                    entry.ace_id = ACE_ID_NONE;
                    if (_ip_source_guard_ace_add(&entry)) {
                        T_W("_ip_source_guard_ace_add() failed");
                    } else {
                        T_N("ace_id = %u", entry.ace_id);
                        if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                            T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                        }
                    }
                }
            }
            while (_ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                if (port_mode_conf[entry.isid - VTSS_ISID_START].mode[entry.port_no] == IP_SOURCE_GUARD_MGMT_DISABLED) {
                    continue;
                }
                if (entry.valid /* && entry.ace_id == ACE_ID_NONE */ && msg_switch_exists(entry.isid)) {
                    entry.ace_id = ACE_ID_NONE;
                    if (_ip_source_guard_ace_add(&entry)) {
                        T_W("_ip_source_guard_ace_add() failed");
                    } else {
                        T_N("ace_id = %u", entry.ace_id);
                        if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                            T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                        }
                    }
                }
            }
        }

        /* Register DHCP snooping */
        dhcp_snooping_ip_assigned_info_register(ip_source_guard_dhcp_pkt_receive);
    } else {
        /* Unregister DHCP snooping */
        dhcp_snooping_ip_assigned_info_unregister(ip_source_guard_dhcp_pkt_receive);

        /* Delete ACEs from static entries */
        if (_ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
            if (entry.valid && entry.ace_id != ACE_ID_NONE) {
                entry.ace_id = ACE_ID_NONE;
                if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                    T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                }
            }
            while (_ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                if (entry.valid && entry.ace_id != ACE_ID_NONE) {
                    entry.ace_id = ACE_ID_NONE;
                    if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                        T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                    }
                }
            }
        }

        /* Clear all dynamic entries */
        if (_ip_source_guard_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_all_dynamic_entry() failed");
        }

        /* Clear all IP source guard ACEs */
        _ip_source_guard_ace_clear();
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    return;
}

/* Apply IP source guard port configuration change */
static void ip_source_guard_port_conf_changed_apply(vtss_isid_t isid, vtss_port_no_t port_no, ulong mode)
{
    ip_source_guard_entry_t             entry;
    dhcp_snooping_ip_assigned_info_t    info;
    uchar                               mac[IP_SOURCE_GUARD_MAC_LENGTH], null_mac[IP_SOURCE_GUARD_MAC_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    vtss_vid_t                          vid;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        return;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    if (ip_source_guard_global.ip_source_guard_conf.mode == IP_SOURCE_GUARD_MGMT_DISABLED) {
        IP_SOURCE_GUARD_CRIT_EXIT();
        return;
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    if (mode == IP_SOURCE_GUARD_MGMT_ENABLED) {
        IP_SOURCE_GUARD_CRIT_ENTER();

        /* Set default ACE */
        if (_ip_source_guard_default_ace_add(isid, port_no)) {
            T_W("_ip_source_guard_default_ace_add() failed");
        }

        /* Get information from DHCP snooping */
        memset(&entry, 0x0 , sizeof(entry));
        memcpy(mac, null_mac, sizeof(mac));
        vid = 0;
        while (dhcp_snooping_ip_assigned_info_getnext(mac, vid, &info)) {
            memcpy(mac, info.mac, IP_SOURCE_GUARD_MAC_LENGTH);
            vid = info.vid;
            if (info.isid == isid && info.port_no == port_no) {
                entry.vid = info.vid;
                memcpy(entry.assigned_mac, info.mac, IP_SOURCE_GUARD_MAC_LENGTH);
                entry.assigned_ip = info.assigned_ip;
                entry.ip_mask = 0xFFFFFFFF;
                entry.isid = info.isid;
                entry.port_no = info.port_no;
                entry.type = IP_SOURCE_GUARD_DYNAMIC_TYPE;
                entry.ace_id = ACE_ID_NONE;
                entry.valid = TRUE;

                /* Check if reach max count */
                if (_ip_source_guard_is_reach_max_port_dynamic_entry_cnt(&entry)) {
                    break;
                }

                /* Add ACE for dynamic entry */
                if (_ip_source_guard_mgmt_conf_get_static_entry(&entry) != VTSS_OK) {
                    if (_ip_source_guard_ace_add(&entry)) {
                        T_W("_ip_source_guard_ace_add() failed");
                    } else {
                        if (_ip_source_guard_mgmt_conf_add_dynamic_entry(&entry, TRUE) != VTSS_OK) {
                            T_W("_ip_source_guard_mgmt_conf_add_dynamic_entry() failed");
                        }
                    }
                }
            }
        };

        /* Add ACEs for static entries */
        if (_ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
            if (entry.isid == isid && entry.port_no == port_no && entry.valid) {
                entry.ace_id = ACE_ID_NONE;
                if (_ip_source_guard_ace_add(&entry)) {
                    T_W("_ip_source_guard_ace_add() failed");
                } else {
                    T_N("ace_id = %u", entry.ace_id);
                    if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                        T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                    }
                }
            }
            while (_ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                if (entry.isid == isid && entry.port_no == port_no && entry.valid) {
                    entry.ace_id = ACE_ID_NONE;
                    if (_ip_source_guard_ace_add(&entry)) {
                        T_W("_ip_source_guard_ace_add() failed");
                    } else {
                        T_N("ace_id = %u", entry.ace_id);
                        if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                            T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                        }
                    }
                }
            }
        }
        IP_SOURCE_GUARD_CRIT_EXIT();
    } else {
        /* Delete ACEs from static entries */
        IP_SOURCE_GUARD_CRIT_ENTER();
        if (_ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
            if (entry.isid == isid && entry.port_no == port_no && entry.valid) {
                if (_ip_source_guard_ace_del(&entry)) {
                    T_W("_ip_source_guard_ace_del() failed");
                } else {
                    T_N("ace_id = %u", entry.ace_id);
                    if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                        T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                    }
                }
            }
            while (_ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
                if (entry.isid == isid && entry.port_no == port_no && entry.valid) {
                    if (_ip_source_guard_ace_del(&entry)) {
                        T_W("_ip_source_guard_ace_del() failed");
                    } else {
                        T_N("ace_id = %u", entry.ace_id);
                        if (_ip_source_guard_mgmt_conf_update_static_entry(&entry) != VTSS_OK) {
                            T_W("_ip_source_guard_mgmt_conf_update_static_entry() failed");
                        }
                    }
                }
            }
        }
        IP_SOURCE_GUARD_CRIT_EXIT();

        /* Clear all dynamic entry by port */
        (void) ip_source_guard_mgmt_conf_flush_dynamic_entry_by_port(isid, port_no);

        /* Delete default ACE */
        IP_SOURCE_GUARD_CRIT_ENTER();
        if (_ip_source_guard_default_ace_del(isid, port_no)) {
            T_D("_ip_source_guard_default_ace_del() failed");
        }
        IP_SOURCE_GUARD_CRIT_EXIT();
    }

    return;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* Set IP_SOURCE_GUARD defaults */
void ip_source_guard_default_set(ip_source_guard_conf_t *conf)
{
    _ip_source_guard_default_set(conf);
    return;
}

/* IP_SOURCE_GUARD error text */
char *ip_source_guard_error_txt(vtss_rc rc)
{
    switch (rc) {
    case IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER:
        return "IP source guard: operation only valid on master switch.";

    case IP_SOURCE_GUARD_ERROR_ISID:
        return "IP source guard: invalid Switch ID.";

    case IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING:
        return "Switch ID is non-existing";

    case IP_SOURCE_GUARD_ERROR_INV_PARAM:
        return "IP source guard: invalid parameter supplied to function.";

    case IP_SOURCE_GUARD_ERROR_STATIC_TABLE_FULL:
        return "IP source guard: static table is full.";

    case IP_SOURCE_GUARD_ERROR_DYNAMIC_TABLE_FULL:
        return "IP source guard: dynamic table is full.";

    case IP_SOURCE_GUARD_ERROR_ACE_AUTO_ASSIGNED_FAIL:
        return "IP source guard: ACE auto-assigned fail.";

    case IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS:
        return "IP source guard: ACE databse access error.";

    case IP_SOURCE_GUARD_ERROR_ENTRY_EXIST_ON_DB:
        return "IP source guard: the entry maybe exists on the DB.";

    case IP_SOURCE_GUARD_ERROR_DATABASE_ADD:
        return "IP source guard: add the entry on DB fail.";

    case IP_SOURCE_GUARD_ERROR_DATABASE_DEL:
        return "IP source guard: delete fthe entry on DB fail.";

    case IP_SOURCE_GUARD_ERROR_LOAD_CONF:
        return "IP source guard: open configuration fail.";

    default:
        return "IP source guard: Unknown error code.";
    }
}

/* Get IP_SOURCE_GUARD configuration */
vtss_rc ip_source_guard_mgmt_conf_get_mode(ulong *mode)
{
    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    *mode = ip_source_guard_global.ip_source_guard_conf.mode;
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set IP_SOURCE_GUARD configuration */
vtss_rc ip_source_guard_mgmt_conf_set_mode(ulong mode)
{
    vtss_rc rc = VTSS_OK;
    int     changed = FALSE;

    T_D("enter, mode: %d", mode);

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check illegal parameter */
    if (mode != IP_SOURCE_GUARD_MGMT_ENABLED &&
        mode != IP_SOURCE_GUARD_MGMT_DISABLED) {
        return IP_SOURCE_GUARD_ERROR_INV_PARAM;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    if (ip_source_guard_global.ip_source_guard_conf.mode != mode) {
        ip_source_guard_global.ip_source_guard_conf.mode = mode;
        changed = TRUE;
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t              blk_id  = CONF_BLK_IP_SOURCE_GUARD_CONF;
        ip_source_guard_conf_blk_t *ip_source_guard_conf_blk_p;
        if ((ip_source_guard_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open IP_SOURCE_GUARD table");
        } else {
            ip_source_guard_conf_blk_p->ip_source_guard_conf.mode = mode;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Activate changed configuration */
        ip_source_guard_conf_apply();
    }

    T_D("exit");
    return rc;
}

/* Get first IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_get_first_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    rc = _ip_source_guard_mgmt_conf_get_first_static_entry(entry);
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Get Next IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_get_next_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    rc = _ip_source_guard_mgmt_conf_get_next_static_entry(entry);
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Set IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_set_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(entry->isid)) {
        T_W("isid: %d isn't configurable switch", entry->isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
#if defined(VTSS_FEATURE_ACL_V2)
    if (entry->assigned_ip == 0) {
#else
    if (entry->assigned_ip == 0 || entry->ip_mask == 0) {
#endif /* VTSS_FEATURE_ACL_V2 */
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_INV_PARAM;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();

    /* Check the entry exist or not ? */
    if ((rc = _ip_source_guard_mgmt_conf_get_dynamic_entry(entry)) != IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if existing, transfer the dynamic entry into static entry
        T_D("the entry existing on dynamic db, transfer, rc=%d", rc);
        if ((rc = _ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry(entry)) == VTSS_OK) {
            /* save configuration */
            rc = _ip_source_guard_mgmt_conf_save_static_configuration();
        }
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return rc;
    }
    if ((rc = _ip_source_guard_mgmt_conf_get_static_entry(entry)) != IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if existing, return the event
        T_D("the entry existing on static db, exit, rc=%d", rc);
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ENTRY_EXIST_ON_DB;
    }

    /* Check total count reach the max value or not? */
    if (_ip_source_guard_entry_count() >= IP_SOURCE_GUARD_MAX_ENTRY_CNT) {
        T_D("total count, rc=%d", _ip_source_guard_entry_count());
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        T_W("ip source guard: table full.");
        return IP_SOURCE_GUARD_ERROR_STATIC_TABLE_FULL;
    }

    entry->valid = TRUE;
    entry->ace_id = ACE_ID_NONE;
    entry->type = IP_SOURCE_GUARD_STATIC_TYPE;

    /* check the system mode and port mode */
    if (ip_source_guard_global.ip_source_guard_conf.mode == IP_SOURCE_GUARD_MGMT_ENABLED &&
        ip_source_guard_global.ip_source_guard_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == IP_SOURCE_GUARD_MGMT_ENABLED) {
        /* add the entry into ACL */
        if ((rc = _ip_source_guard_ace_add(entry)) != VTSS_OK) {
            T_W("_ip_source_guard_ace_add() failed");
        }
    }

    /* add the entry into static DB */
    if ((rc = _ip_source_guard_mgmt_conf_add_static_entry(entry, TRUE)) == VTSS_OK) {
        /* save configuration */
        rc = _ip_source_guard_mgmt_conf_save_static_configuration();
    } else {
        /* check ACL inserted or not  */
        if (entry->ace_id != ACE_ID_NONE) {
            /* delete the ace entry */
            if (_ip_source_guard_ace_del(entry) != VTSS_OK) {
                T_D("_ip_source_guard_ace_del() failed");
            }
        }
    }

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Set IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_del_static_entry(ip_source_guard_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(entry->isid)) {
        T_W("isid: %d isn't configurable switch", entry->isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();

    /* Check if entry exist? */
    if ((rc = _ip_source_guard_mgmt_conf_get_static_entry(entry)) == IP_SOURCE_GUARD_ERROR_DATABASE_ACCESS) {
        //if not existing, return
        IP_SOURCE_GUARD_CRIT_EXIT();
        T_D("exit");
        return rc;
    } else {
        /* check ACL inserted or not */
        if (entry->valid && entry->ace_id != ACE_ID_NONE) {
            /* delete the entry on ACL */
            if (_ip_source_guard_ace_del(entry) != VTSS_OK) {
                T_D("_ip_source_guard_ace_del() failed");
            }
        }
    }

    /* delete the entry on static DB */
    if ((rc = _ip_source_guard_mgmt_conf_del_static_entry(entry, TRUE)) != VTSS_OK) {
        T_W("_ip_source_guard_mgmt_conf_del_static_entry failed");
    }

    /* save configuration */
    rc = _ip_source_guard_mgmt_conf_save_static_configuration();

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* del all IP_SOURCE_GUARD static entry */
vtss_rc ip_source_guard_mgmt_conf_del_all_static_entry(void)
{
    vtss_rc                         rc = VTSS_OK;
    ip_source_guard_entry_t         entry;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    /* Delete ACEs from static entries */
    while (_ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {
        if (entry.valid && entry.ace_id != ACE_ID_NONE) {
            if (_ip_source_guard_ace_del(&entry)) {
                T_D("_ip_source_guard_ace_del() failed");
            }
        }

        /* delete the entry on static DB */
        if ((rc = _ip_source_guard_mgmt_conf_del_static_entry(&entry, TRUE)) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_static_entry failed");
        }
    }

    /* save configuration */
    rc = _ip_source_guard_mgmt_conf_save_static_configuration();

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return rc;
}

vtss_rc ip_source_guard_mgmt_conf_set_port_mode(vtss_isid_t isid, ip_source_guard_port_mode_conf_t *port_mode_conf)
{
    int         changed = FALSE;
    ulong       port_conf_change[VTSS_PORTS];
    port_iter_t pit;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
    if (port_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_INV_PARAM;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (port_mode_conf->mode[pit.iport] != IP_SOURCE_GUARD_MGMT_ENABLED &&
            port_mode_conf->mode[pit.iport] != IP_SOURCE_GUARD_MGMT_DISABLED) {
            return IP_SOURCE_GUARD_ERROR_INV_PARAM;
        }
    }

    memset(port_conf_change, 0x0, sizeof(port_conf_change));

    /* Check port mode change */
    IP_SOURCE_GUARD_CRIT_ENTER();
    if (memcmp(&ip_source_guard_global.ip_source_guard_conf.port_mode_conf[isid - VTSS_ISID_START], port_mode_conf, sizeof(ip_source_guard_port_mode_conf_t))) {
        changed = TRUE;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (ip_source_guard_global.ip_source_guard_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport] != port_mode_conf->mode[pit.iport]) {
                port_conf_change[pit.iport] = TRUE;
                ip_source_guard_global.ip_source_guard_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport] = port_mode_conf->mode[pit.iport];
            }
        }
    }
    IP_SOURCE_GUARD_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t              blk_id  = CONF_BLK_IP_SOURCE_GUARD_CONF;
        ip_source_guard_conf_blk_t *ip_source_guard_conf_blk_p;
        if ((ip_source_guard_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open IP source guard table");
        } else {
            IP_SOURCE_GUARD_CRIT_ENTER();
            memcpy(ip_source_guard_conf_blk_p->ip_source_guard_conf.port_mode_conf, ip_source_guard_global.ip_source_guard_conf.port_mode_conf, VTSS_ISID_CNT * sizeof(ip_source_guard_port_mode_conf_t));
            IP_SOURCE_GUARD_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Activate changed configuration */
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (port_conf_change[pit.iport]) {
                ip_source_guard_port_conf_changed_apply(isid, pit.iport, port_mode_conf->mode[pit.iport]);
            }
        }
    }

    T_D("exit");
    return VTSS_OK;
}

vtss_rc ip_source_guard_mgmt_conf_get_port_mode(vtss_isid_t isid, ip_source_guard_port_mode_conf_t *port_mode_conf)
{
    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
    if (port_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_INV_PARAM;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();
    memcpy(port_mode_conf, &ip_source_guard_global.ip_source_guard_conf.port_mode_conf[isid - VTSS_ISID_START], sizeof(ip_source_guard_port_mode_conf_t));
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Translate IP_SOURCE_GUARD dynamic entries into static entries */
vtss_rc ip_source_guard_mgmt_conf_translate_dynamic_into_static(void)
{
    vtss_rc                     rc = VTSS_OK;
    ip_source_guard_entry_t     entry;
    int                         count = 0;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return IP_SOURCE_GUARD_ERROR_MUST_BE_MASTER;
    }

    IP_SOURCE_GUARD_CRIT_ENTER();

    /* translate dynamic entries into static entries */
    if (_ip_source_guard_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if ((rc = _ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry)) != VTSS_OK) {
            T_D("_ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry() failed");
        } else {
            count++;
        }

        while (_ip_source_guard_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if ((rc = _ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry)) != VTSS_OK) {
                T_D("_ip_source_guard_mgmt_conf_translate_dynamic_entry_into_static_entry() failed");
            } else {
                count++;
            }
        }
    }

    /* save configuration */
    rc = _ip_source_guard_mgmt_conf_save_static_configuration();

    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    if (rc < VTSS_OK) {
        return rc;
    } else {
        return count;
    }
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create IP_SOURCE_GUARD stack configuration */
static void ip_source_guard_conf_read_stack(BOOL create)
{
    int                             changed;
    BOOL                            do_create;
    ulong                           size;
    static ip_source_guard_conf_t   new_ip_source_guard_conf;
    ip_source_guard_conf_t          *old_ip_source_guard_conf_p;
    ip_source_guard_conf_blk_t      *conf_blk_p;
    conf_blk_id_t                   blk_id;
    ulong                           blk_version;

    T_D("enter, create: %d", create);

    /* Read/create IP_SOURCE_GUARD configuration */
    blk_id = CONF_BLK_IP_SOURCE_GUARD_CONF;
    blk_version = IP_SOURCE_GUARD_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = FALSE;
    IP_SOURCE_GUARD_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        _ip_source_guard_default_set(&new_ip_source_guard_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->ip_source_guard_conf = new_ip_source_guard_conf;
        }

        /* Delete all static entries */
        if (_ip_source_guard_mgmt_conf_del_all_static_entry(TRUE) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_all_static_entry() failed");
        }

        /* Delete all dynamic entries */
        if (_ip_source_guard_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_all_dynamic_entry() failed");
        }

        /* Clear all IP source guard ACEs */
        _ip_source_guard_ace_clear();
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {
            new_ip_source_guard_conf = conf_blk_p->ip_source_guard_conf;
        }
    }
    old_ip_source_guard_conf_p = &ip_source_guard_global.ip_source_guard_conf;
    if (ip_source_guard_conf_changed(old_ip_source_guard_conf_p, &new_ip_source_guard_conf)) {
        changed = TRUE;
    }
    ip_source_guard_global.ip_source_guard_conf = new_ip_source_guard_conf;

    IP_SOURCE_GUARD_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open IP_SOURCE_GUARD table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        /* Apply all configuration to switch */
        ip_source_guard_conf_apply();
    }

    T_D("exit");
    return;
}

/* Module start */
static void ip_source_guard_start(void)
{
    ip_source_guard_conf_t *conf_p;

    T_D("enter");

    /* Initialize IP_SOURCE_GUARD configuration */
    conf_p = &ip_source_guard_global.ip_source_guard_conf;
    _ip_source_guard_default_set(conf_p);
    _ip_source_guard_default_set_dynamic_entries();

    /* Initialize IP_SOURCE_GUARD ACE ID */
    memset(ip_source_guard_global.id_used, 0x0, sizeof(ip_source_guard_global.id_used));

    /* Create semaphore for critical regions */
    critd_init(&ip_source_guard_global.crit, "ip_source_guard_global.crit", VTSS_MODULE_ID_IP_SOURCE_GUARD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IP_SOURCE_GUARD_CRIT_EXIT();

    T_D("exit");
    return;
}

/* Initialize module */
vtss_rc ip_source_guard_init(vtss_init_data_t *data)
{
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc = VTSS_OK;
#endif
    vtss_isid_t isid = data->isid;
    int         i;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        ip_source_guard_start();
#ifdef VTSS_SW_OPTION_VCLI
        ips_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = ip_source_guard_icfg_init()) != VTSS_OK) {
            T_D("Calling ip_source_guard_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            ip_source_guard_conf_read_stack(TRUE);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");
        IP_SOURCE_GUARD_CRIT_ENTER();

        /* create data base for storing static entries */
#ifdef VTSS_LIB_DATA_STRUCT
        if (_ip_source_guard_mgmt_conf_create_static_db(IP_SOURCE_GUARD_MAX_ENTRY_CNT, _ip_source_guard_entry_compare_func) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_create_static_db() failed");
        }
#else
        if (_ip_source_guard_mgmt_conf_create_static_db() != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_create_static_db() failed");
        }
#endif

        /* create data base for storing dynamic entries */
#ifdef VTSS_LIB_DATA_STRUCT
        if (_ip_source_guard_mgmt_conf_create_dynamic_db(IP_SOURCE_GUARD_MAX_ENTRY_CNT, _ip_source_guard_dynamic_entry_compare_func) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_create_dynamic_db() failed");
        }
#else
        if (_ip_source_guard_mgmt_conf_create_dynamic_db() != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_create_dynamic_db() failed");
        }
#endif

        IP_SOURCE_GUARD_CRIT_EXIT();

        /* Read stack and switch configuration */
        ip_source_guard_conf_read_stack(FALSE);

        IP_SOURCE_GUARD_CRIT_ENTER();
        /* Sync static entries to the efficient data base */
        for (i = 0; i < IP_SOURCE_GUARD_MAX_ENTRY_CNT; i++) {
            if (ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i].valid == TRUE) {
                if (_ip_source_guard_mgmt_conf_add_static_entry(&ip_source_guard_global.ip_source_guard_conf.ip_source_guard_static_entry[i], FALSE) != VTSS_OK) {
                    T_W("_ip_source_guard_mgmt_conf_add_static_entry() failed");
                }
            }
        }
        IP_SOURCE_GUARD_CRIT_EXIT();
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        /* clear all entry */
        IP_SOURCE_GUARD_CRIT_ENTER();

        /* Delete all static entries */
        if (_ip_source_guard_mgmt_conf_del_all_static_entry(TRUE) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_all_static_entry() failed");
        }

        /* Delete all dynamic entries */
        if (_ip_source_guard_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
            T_W("_ip_source_guard_mgmt_conf_del_all_dynamic_entry() failed");
        }

        _ip_source_guard_ace_free(ACE_ID_NONE);
        IP_SOURCE_GUARD_CRIT_EXIT();
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        ip_source_guard_conf_apply();
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
