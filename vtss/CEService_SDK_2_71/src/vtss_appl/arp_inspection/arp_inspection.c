/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "arp_inspection_api.h"
#include "arp_inspection.h"

#ifdef VTSS_LIB_DATA_STRUCT
#include "vtss_lib_data_struct_api.h"
#else
#include "vtss_avl_tree_api.h"
#endif

#include "ip2_api.h"
#include "packet_api.h"
#include "port_api.h"

#include "acl_api.h"
#include "dhcp_snooping_api.h"

#include <network.h>

#ifdef VTSS_SW_OPTION_VCLI
#include "arp_inspection_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "arp_inspection_icfg.h"
#endif

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"   // For topo_usid2isid(), topo_isid2usid()
#endif

#include "vtss_bip_buffer_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ARP_INSPECTION


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "arp_insp",
    .descr     = "ARP_INSPECTION"
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

#define ARP_INSPECTION_CRIT_ENTER() critd_enter(&arp_inspection_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ARP_INSPECTION_CRIT_EXIT()  critd_exit( &arp_inspection_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ARP_INSPECTION_BIP_CRIT_ENTER()         critd_enter(&arp_inspection_global.bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ARP_INSPECTION_BIP_CRIT_EXIT()          critd_exit(&arp_inspection_global.bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)

#else
#define ARP_INSPECTION_CRIT_ENTER() critd_enter(&arp_inspection_global.crit)
#define ARP_INSPECTION_CRIT_EXIT()  critd_exit( &arp_inspection_global.crit)
#define ARP_INSPECTION_BIP_CRIT_ENTER()         critd_enter(&arp_inspection_global.bip_crit)
#define ARP_INSPECTION_BIP_CRIT_EXIT()          critd_exit(&arp_inspection_global.bip_crit)
#endif /* VTSS_TRACE_ENABLED */

#define ARP_INSPECTION_CERT_THREAD_STACK_SIZE       8192
#define ARP_INSPECTION_MAC_LENGTH       6
#define ARP_INSPECTION_BUF_LENGTH       40

/* Global structure */

/* BIP buffer event declaration */
#define ARP_INSPECTION_EVENT_PKT_RECV      0x00000001
#define ARP_INSPECTION_EVENT_ANY           ARP_INSPECTION_EVENT_PKT_RECV  /* Any possible bit */
static cyg_flag_t   arp_inspection_bip_buffer_thread_events;

/* BIP buffer Thread variables */
static cyg_handle_t arp_inspection_bip_buffer_thread_handle;
static cyg_thread   arp_inspection_bip_buffer_thread_block;
static char         arp_inspection_bip_buffer_thread_stack[ARP_INSPECTION_CERT_THREAD_STACK_SIZE];

/* BIP buffer data declaration */
#define ARP_INSPECTION_BIP_BUF_PKT_SIZE    1520  /* 4 bytes alignment */
#define ARP_INSPECTION_BIP_BUF_CNT         ARP_INSPECTION_FRAME_INFO_MAX_CNT

typedef struct {
    char            pkt[ARP_INSPECTION_BIP_BUF_PKT_SIZE];
    size_t          len;
    vtss_vid_t      vid;
    u16             dummy;
    vtss_isid_t     isid;
    vtss_port_no_t  port_no;
} arp_inspection_bip_buf_t;

#define ARP_INSPECTION_BIP_BUF_TOTAL_SIZE          (ARP_INSPECTION_BIP_BUF_CNT * sizeof(arp_inspection_bip_buf_t))
static vtss_bip_buffer_t arp_inspection_bip_buf;
static arp_inspection_global_t arp_inspection_global;

static i32 _arp_inspection_entry_compare_func(void *elm1, void *elm2);

/* create library variables for data struct */
#ifdef VTSS_LIB_DATA_STRUCT
static  vtss_lib_data_struct_ctrl_header_t      static_arp_entry_list;
#else
VTSS_AVL_TREE(static_arp_entry_list_avlt, "ARP_Inspection_static_avlt", VTSS_MODULE_ID_ARP_INSPECTION, _arp_inspection_entry_compare_func, ARP_INSPECTION_MAX_ENTRY_CNT)
#endif
static  int     static_arp_entry_list_created_done = 0;

#ifdef VTSS_LIB_DATA_STRUCT
static  vtss_lib_data_struct_ctrl_header_t      dynamic_arp_entry_list;
#else
VTSS_AVL_TREE(dynamic_arp_entry_list_avlt, "ARP_Inspection_dynamic_avlt", VTSS_MODULE_ID_ARP_INSPECTION, _arp_inspection_entry_compare_func, ARP_INSPECTION_MAX_ENTRY_CNT)
#endif
static  int     dynamic_arp_entry_list_created_done = 0;


/* packet rx filter */
packet_rx_filter_t      arp_inspection_rx_filter;
static void             *arp_inspection_filter_id = NULL; // Filter id for subscribing arp_inspection packet.

/* RX loopback on master */
static vtss_isid_t      master_isid = VTSS_ISID_LOCAL;

/****************************************************************************/
/*  Various local functions with no semaphore protection                    */
/****************************************************************************/

/* ARP_INSPECTION compare function */
static i32 _arp_inspection_entry_compare_func(void *elm1, void *elm2)
{
    arp_inspection_entry_t *element1, *element2;

    /* BODY
     */
    element1 = (arp_inspection_entry_t *)elm1;
    element2 = (arp_inspection_entry_t *)elm2;
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
    } else if (memcmp(element1->mac, element2->mac, ARP_INSPECTION_MAC_LENGTH * sizeof(u8)) > 0) {
        return 1;
    } else if (memcmp(element1->mac, element2->mac, ARP_INSPECTION_MAC_LENGTH * sizeof(u8)) < 0) {
        return -1;
    } else if (element1->assigned_ip > element2->assigned_ip) {
        return 1;
    } else if (element1->assigned_ip < element2->assigned_ip) {
        return -1;
    } else {
        return 0;
    }
}

/* Add ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_add_static_entry(arp_inspection_entry_t *entry, BOOL allocated)
{
    arp_inspection_entry_t  *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    /* allocated memory */
    if (allocated) {
        /* Find an unused entry */
        for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
            if (arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i].valid) {
                continue;
            }
            /* insert the entry on global memory for saving configuration */
            memcpy(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], entry, sizeof(arp_inspection_entry_t));
            entry_p = &(arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i]);

            /* add the entry into static DB */
#ifdef VTSS_LIB_DATA_STRUCT
            if (vtss_lib_data_struct_set_entry(&static_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                memset(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                T_D("add the entry into static DB failed");
                rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
            }
#else
            if (vtss_avl_tree_add(&static_arp_entry_list_avlt, entry_p) != TRUE) {
                memset(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                T_D("add the entry into static DB failed");
                rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
            }
#endif
            break;
        }
    } else {
        /* only insert the entry into link */

#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_set_entry(&static_arp_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
        }
#else
        if (vtss_avl_tree_add(&static_arp_entry_list_avlt, entry) != TRUE) {
            rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
        }
#endif
    }

    return rc;
}

/* Delete ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_del_static_entry(arp_inspection_entry_t *entry, BOOL free_node)
{
    arp_inspection_entry_t  *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    if (free_node) {
        /* Find the exist entry */
        for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
            if (!arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i].valid) {
                continue;
            }

            if (_arp_inspection_entry_compare_func(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], entry) == 0) {
                entry_p = &(arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i]);
#ifdef VTSS_LIB_DATA_STRUCT
                /* delete the entry on static DB */
                if (vtss_lib_data_struct_delete_entry(&static_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                    rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                }
#else
                /* delete the entry on static DB */
                if (vtss_avl_tree_delete(&static_arp_entry_list_avlt, (void **) &entry_p) != TRUE) {
                    rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                }
#endif
                break;
            }
        }
    } else {
        /* only delete the entry on link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_entry(&static_arp_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#else
        if (vtss_avl_tree_delete(&static_arp_entry_list_avlt, (void **) &entry_p) != TRUE) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#endif
    }

    return rc;
}

/* Delete All ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_del_all_static_entry(BOOL free_node)
{
    vtss_rc rc = VTSS_OK;

    if (free_node) {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&static_arp_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        } else {
            /* clear global cache memory */
            memset(arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
        }
#else
        vtss_avl_tree_destroy(&static_arp_entry_list_avlt);
        if (vtss_avl_tree_init(&static_arp_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
        /* clear global cache memory */
        memset(arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
#endif
    } else {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&static_arp_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#else
        vtss_avl_tree_destroy(&static_arp_entry_list_avlt);
        if (vtss_avl_tree_init(&static_arp_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
#endif
    }

    return rc;
}

/* Get ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_get_static_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&static_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif

    return rc;
}

/* Get First ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_get_first_static_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_first_entry(&static_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_FIRST) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif

    return rc;
}

/* Get Next ARP_INSPECTION static entry */
static vtss_rc _arp_inspection_mgmt_conf_get_next_static_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_next_entry(&static_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&static_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_NEXT) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif

    return rc;
}

/* Create ARP_INSPECTION static data base */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _arp_inspection_mgmt_conf_create_static_db(ulong max_entry_cnt, compare_func_cb_t compare_func)
{
    vtss_rc rc = VTSS_OK;

    if (!static_arp_entry_list_created_done) {
        /* create data base for storing static arp entry */
        if (vtss_lib_data_struct_create_tree(&static_arp_entry_list, max_entry_cnt, sizeof(arp_inspection_entry_t), compare_func, NULL, NULL, VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE)) {
            T_W("vtss_lib_data_struct_create_tree() failed");
            rc = ARP_INSPECTION_ERROR_DATABASE_CREATE;
        }
        static_arp_entry_list_created_done = TRUE;
    }

    return rc;
}
#else
static vtss_rc _arp_inspection_mgmt_conf_create_static_db(void)
{
    vtss_rc rc = VTSS_OK;

    if (!static_arp_entry_list_created_done) {
        /* create data base for storing static arp entry */
        if (vtss_avl_tree_init(&static_arp_entry_list_avlt)) {
            static_arp_entry_list_created_done = TRUE;
        } else {
            T_W("vtss_avl_tree_init() failed");
            rc = ARP_INSPECTION_ERROR_DATABASE_CREATE;
        }
    }

    return rc;
}
#endif


/* Add ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_add_dynamic_entry(arp_inspection_entry_t *entry, BOOL allocated)
{
    arp_inspection_entry_t  *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    /* allocated memory */
    if (allocated) {
        /* Find an unused entry */
        for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
            if (arp_inspection_global.arp_inspection_dynamic_entry[i].valid) {
                continue;
            }
            /* insert the entry on global memory */
            memcpy(&arp_inspection_global.arp_inspection_dynamic_entry[i], entry, sizeof(arp_inspection_entry_t));
            entry_p = &(arp_inspection_global.arp_inspection_dynamic_entry[i]);

            /* add the entry into dynamic DB */
#ifdef VTSS_LIB_DATA_STRUCT
            if (vtss_lib_data_struct_set_entry(&dynamic_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                memset(&arp_inspection_global.arp_inspection_dynamic_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                T_D("add the entry into dynamic DB failed");
                rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
            }
#else
            if (vtss_avl_tree_add(&dynamic_arp_entry_list_avlt, entry_p) != TRUE) {
                memset(&arp_inspection_global.arp_inspection_dynamic_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                T_D("add the entry into dynamic DB failed");
                rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
            }
#endif
            break;
        }
    } else {
        /* only insert the entry into link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_set_entry(&dynamic_arp_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
        }
#else
        if (vtss_avl_tree_add(&dynamic_arp_entry_list_avlt, entry) != TRUE) {
            rc = ARP_INSPECTION_ERROR_DATABASE_ADD;
        }
#endif
    }

    return rc;
}

/* Delete ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_del_dynamic_entry(arp_inspection_entry_t *entry, BOOL free_node)
{
    arp_inspection_entry_t  *entry_p;
    vtss_rc                 rc = VTSS_OK;
    int                     i;

    if (free_node) {
        /* Find the exist entry */
        for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
            if (!arp_inspection_global.arp_inspection_dynamic_entry[i].valid) {
                continue;
            }

            if (_arp_inspection_entry_compare_func(&arp_inspection_global.arp_inspection_dynamic_entry[i], entry) == 0) {
                entry_p = &(arp_inspection_global.arp_inspection_dynamic_entry[i]);

#ifdef VTSS_LIB_DATA_STRUCT
                /* delete the entry on dynamic DB */
                if (vtss_lib_data_struct_delete_entry(&dynamic_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
                    rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&arp_inspection_global.arp_inspection_dynamic_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                }
#else
                /* delete the entry on dynamic DB */
                if (vtss_avl_tree_delete(&dynamic_arp_entry_list_avlt, (void **) &entry_p) != TRUE) {
                    rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
                } else {
                    /* clear global cache memory */
                    memset(&arp_inspection_global.arp_inspection_dynamic_entry[i], 0x0, sizeof(arp_inspection_entry_t));
                }
#endif
                break;
            }
        }
    } else {
        /* only delete the entry on link */
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_entry(&dynamic_arp_entry_list, &entry) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#else
        if (vtss_avl_tree_delete(&dynamic_arp_entry_list_avlt, (void **) &entry_p) != TRUE) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#endif
    }

    return rc;
}

/* Delete All ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_del_all_dynamic_entry(BOOL free_node)
{
    vtss_rc rc = VTSS_OK;

    if (free_node) {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&dynamic_arp_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        } else {
            /* clear global cache memory */
            memset(arp_inspection_global.arp_inspection_dynamic_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
        }
#else
        vtss_avl_tree_destroy(&dynamic_arp_entry_list_avlt);
        if (vtss_avl_tree_init(&dynamic_arp_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
        /* clear global cache memory */
        memset(arp_inspection_global.arp_inspection_dynamic_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
#endif
    } else {
#ifdef VTSS_LIB_DATA_STRUCT
        if (vtss_lib_data_struct_delete_all_entry(&dynamic_arp_entry_list) != VTSS_LIB_DATA_STRUCT_RC_OK) {
            rc = ARP_INSPECTION_ERROR_DATABASE_DEL;
        }
#else
        vtss_avl_tree_destroy(&dynamic_arp_entry_list_avlt);
        if (vtss_avl_tree_init(&dynamic_arp_entry_list_avlt) != TRUE) {
            T_W("vtss_avl_tree_init() failed");
        }
#endif
    }

    return rc;
}

/* Get ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_get_dynamic_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_entry(&dynamic_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif

    return rc;
}

/* Get First ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_get_first_dynamic_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_first_entry(&dynamic_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_FIRST) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif


    return rc;
}

/* Get Next ARP_INSPECTION dynamic entry */
static vtss_rc _arp_inspection_mgmt_conf_get_next_dynamic_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                 rc = VTSS_OK;
    arp_inspection_entry_t  *entry_p;

    entry_p = entry;
#ifdef VTSS_LIB_DATA_STRUCT
    if (vtss_lib_data_struct_get_next_entry(&dynamic_arp_entry_list, &entry_p) != VTSS_LIB_DATA_STRUCT_RC_OK) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#else
    if (vtss_avl_tree_get(&dynamic_arp_entry_list_avlt, (void **) &entry_p, VTSS_AVL_TREE_GET_NEXT) != TRUE) {
        rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
    } else {
        memcpy(entry, entry_p, sizeof(arp_inspection_entry_t));
    }
#endif

    return rc;
}

/* Create ARP_INSPECTION dynamic data base */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _arp_inspection_mgmt_conf_create_dynamic_db(ulong max_entry_cnt, compare_func_cb_t compare_func)
{
    vtss_rc rc = VTSS_OK;

    if (!dynamic_arp_entry_list_created_done) {
        /* create data base for storing dynamic arp entry */
        if (vtss_lib_data_struct_create_tree(&dynamic_arp_entry_list, max_entry_cnt, sizeof(arp_inspection_entry_t), compare_func, NULL, NULL, VTSS_LIB_DATA_STRUCT_TYPE_AVL_TREE)) {
            T_W("vtss_lib_data_struct_create_tree() failed");
            rc = ARP_INSPECTION_ERROR_DATABASE_CREATE;
        }
        dynamic_arp_entry_list_created_done = TRUE;
    }

    return rc;
}
#else
static vtss_rc _arp_inspection_mgmt_conf_create_dynamic_db(void)
{
    vtss_rc rc = VTSS_OK;

    if (!dynamic_arp_entry_list_created_done) {
        /* create data base for storing dynamic arp entry */
        if (vtss_avl_tree_init(&dynamic_arp_entry_list_avlt)) {
            dynamic_arp_entry_list_created_done = TRUE;
        } else {
            T_W("vtss_avl_tree_init() failed");
            rc = ARP_INSPECTION_ERROR_DATABASE_CREATE;
        }
    }

    return rc;
}
#endif


/* Translate ARP_INSPECTION dynamic entries into static entries */
static vtss_rc _arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(arp_inspection_entry_t *entry)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* add the entry into static DB */
    if ((rc = _arp_inspection_mgmt_conf_add_static_entry(entry, TRUE)) != VTSS_OK) {
        T_D("_arp_inspection_mgmt_conf_add_static_entry() failed");
        T_D("exit");
        return rc;
    } else {

        /* delete the entry on dynamic DB */
        if ((rc = _arp_inspection_mgmt_conf_del_dynamic_entry(entry, TRUE)) != VTSS_OK) {
            T_D("_arp_inspection_mgmt_conf_del_dynamic_entry() failed");
        }
    }

    T_D("exit");
    return rc;
}

/* Save ARP_INSPECTION configuration */
static vtss_rc _arp_inspection_mgmt_conf_save_static_configuration(void)
{
    vtss_rc                     rc = VTSS_OK;
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t               blk_id = CONF_BLK_ARP_INSPECTION_CONF;
    arp_inspection_conf_blk_t  *arp_inspection_conf_blk_p;

    T_D("enter");

    /* Save changed configuration */
    if ((arp_inspection_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open ARP_INSPECTION table");
        rc = ARP_INSPECTION_ERROR_LOAD_CONF;
    } else {
        memcpy(arp_inspection_conf_blk_p->arp_inspection_conf.arp_inspection_static_entry, arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* ARP_INSPECTION entry count */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _arp_inspection_entry_count(void)
{
    vtss_rc rc = VTSS_OK;

    rc = static_arp_entry_list.current_cnt + dynamic_arp_entry_list.current_cnt;

    return rc;
}
#else
static vtss_rc _arp_inspection_entry_count(void)
{
    vtss_rc rc = VTSS_OK;
    int     i;

    /* Find the exist entry on static DB */
    for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
        if (!arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i].valid) {
            continue;
        }
        rc++;
    }

    /* Find the exist entry on dynamic DB */
    for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
        if (!arp_inspection_global.arp_inspection_dynamic_entry[i].valid) {
            continue;
        }
        rc++;
    }

    return rc;
}
#endif

/* Get ARP_INSPECTION static count */
#ifdef VTSS_LIB_DATA_STRUCT
static vtss_rc _arp_inspection_get_static_count(void)
{
    vtss_rc rc = VTSS_OK;

    rc = static_arp_entry_list.current_cnt;

    return rc;
}
#else
static vtss_rc _arp_inspection_get_static_count(void)
{
    vtss_rc rc = VTSS_OK;
    int     i;

    /* Find the exist entry on static DB */
    for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
        if (!arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i].valid) {
            continue;
        }
        rc++;
    }

    return rc;
}
#endif

#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
/* Pack Utility */
static void pack16(u16 v, u8 *buf)
{
    buf[0] = (v >> 8) & 0xff;
    buf[1] = v & 0xff;
}
#endif

/****************************************************************************/
/*  Configuration silent upgrade                                            */
/****************************************************************************/

typedef struct {
    uchar                       mac[6];
    ulong                       vid;
    vtss_ipv4_t                 assigned_ip;
    ulong                       isid;
    ulong                       port_no;
    arp_inspection_entry_type_t type;
    ulong                       valid;
} arp_inspection_entry_v1_t;

typedef struct {
    ulong   mode[VTSS_PORTS];
} arp_inspection_port_mode_conf_v1_t;

/* ARP_INSPECTION configuration */
typedef struct {
    ulong                               mode;                                                       /* ARP_INSPECTION Mode */
    arp_inspection_port_mode_conf_v1_t  port_mode_conf[VTSS_ISID_CNT];
    arp_inspection_entry_v1_t           arp_inspection_static_entry[ARP_INSPECTION_MAX_ENTRY_CNT];
} arp_inspection_conf_v1_t;

/* ARP_INSPECTION configuration block */
typedef struct {
    unsigned long               version;                /* Block version */
    arp_inspection_conf_v1_t    arp_inspection_conf;    /* ARP_INSPECTION configuration */
} arp_inspection_conf_blk_v1_t;

/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static arp_inspection_conf_blk_t *arp_inspection_conf_flash_silent_upgrade(const void *blk, u32 old_ver)
{
    arp_inspection_conf_blk_t *new_blk = NULL;

    if (old_ver == ARP_INSPECTION_CONF_BLK_VERSION1) {
        if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
            arp_inspection_conf_blk_v1_t    *old_blk = (arp_inspection_conf_blk_v1_t *)blk;
            int                             i, j;

            /* upgrade configuration from v1 to v2 */
            arp_inspection_default_set(&new_blk->arp_inspection_conf); // Initial with default values
            new_blk->version    = ARP_INSPECTION_CONF_BLK_VERSION2;
            new_blk->arp_inspection_conf.mode = old_blk->arp_inspection_conf.mode;
            for (i = 0; i < VTSS_ISID_CNT; ++i) {
                for (j = 0; j < VTSS_PORTS; ++j) {
                    new_blk->arp_inspection_conf.port_mode_conf[i].mode[j] = old_blk->arp_inspection_conf.port_mode_conf[i].mode[j];
                }
            }
            for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; ++i) {
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].isid = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].isid;
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].port_no = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].port_no;
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].assigned_ip = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].assigned_ip;
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].vid = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].vid;
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].type = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].type;
                new_blk->arp_inspection_conf.arp_inspection_static_entry[i].valid = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].valid;
                for (j = 0; j < 6; ++j) {
                    new_blk->arp_inspection_conf.arp_inspection_static_entry[i].mac[j] = old_blk->arp_inspection_conf.arp_inspection_static_entry[i].mac[j];
                }
            }
        }
    }

    return new_blk;
}

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Set ARP_INSPECTION defaults */
void arp_inspection_default_set(arp_inspection_conf_t *conf)
{
    int isid, j;

    memset(conf, 0x0, sizeof(*conf));
    conf->mode = ARP_INSPECTION_DEFAULT_MODE;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        for (j = VTSS_PORT_NO_START; j < VTSS_PORT_NO_END; j++) {
            if (port_isid_port_no_is_stack(isid, j)) {
                conf->port_mode_conf[isid - VTSS_ISID_START].mode[j] = ARP_INSPECTION_MGMT_DISABLED;
            } else {
                conf->port_mode_conf[isid - VTSS_ISID_START].mode[j] = ARP_INSPECTION_DEFAULT_PORT_MODE;
            }
            conf->port_mode_conf[isid - VTSS_ISID_START].check_VLAN[j] = ARP_INSPECTION_DEFAULT_PORT_VLAN_MODE;
            conf->port_mode_conf[isid - VTSS_ISID_START].log_type[j] = ARP_INSPECTION_DEFAULT_LOG_TYPE;
        }
    }

    /* clear global cache memory for static entries */
    memset(conf->arp_inspection_static_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
    T_D("clear static entries on global cache memory");

    return;
}

/* Set ARP_INSPECTION defaults dynamic entry */
static void arp_inspection_default_set_dynamic_entry(void)
{
    /* clear global cache memory for dynamic entries */
    memset(arp_inspection_global.arp_inspection_dynamic_entry, 0x0, ARP_INSPECTION_MAX_ENTRY_CNT * sizeof(arp_inspection_entry_t));
    T_D("clear dynamic entries on global cache memory");

    return;
}

/****************************************************************************/
/*  Compare Function                                                        */
/****************************************************************************/
/* ARP_INSPECTION compare function */
static int arp_inspection_entry_compare_func(void *elm1, void *elm2)
{
    arp_inspection_entry_t *element1, *element2;

    /* BODY
     */
    element1 = (arp_inspection_entry_t *)elm1;
    element2 = (arp_inspection_entry_t *)elm2;
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
    } else if (memcmp(element1->mac, element2->mac, ARP_INSPECTION_MAC_LENGTH * sizeof(u8)) > 0) {
        return 1;
    } else if (memcmp(element1->mac, element2->mac, ARP_INSPECTION_MAC_LENGTH * sizeof(u8)) < 0) {
        return -1;
    } else if (element1->assigned_ip > element2->assigned_ip) {
        return 1;
    } else if (element1->assigned_ip < element2->assigned_ip) {
        return -1;
    } else {
        return 0;
    }
}

/* ARP_INSPECTION entry checking if sender/target IP matched */
static vtss_rc arp_inspection_entry_checking_matched_ipv4(arp_inspection_entry_t *entry, vtss_ipv4_t target_ip)
{
    vtss_rc                     rc = VTSS_OK;
    arp_inspection_entry_t      temp_entry;
    BOOL                        found = FALSE;
    //char                        buf1[ARP_INSPECTION_BUF_LENGTH], buf2[ARP_INSPECTION_BUF_LENGTH], buf3[ARP_INSPECTION_BUF_LENGTH];

    T_D("enter");

    ARP_INSPECTION_CRIT_ENTER();
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("mode disabled, bypass checking");
        return VTSS_OK;
    }

    /* Check the VLAN mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].check_VLAN[entry->port_no] == ARP_INSPECTION_MGMT_VLAN_ENABLED) {
        /* Check the VLAN is in DB */
        if (arp_inspection_global.arp_inspection_conf.vlan_mode_conf[entry->vid].flags & ARP_INSPECTION_VLAN_MODE) {
            // VLAN in DB, continue to checking source address
        } else {
            // no checking, forward frame
            ARP_INSPECTION_CRIT_EXIT();
            return VTSS_OK;
        }
    } else {
        // VLAN mode disable, continue to checking source address
    }

    memset(&temp_entry, 0x0, sizeof(temp_entry));
    while (1) {
        if ((rc = _arp_inspection_mgmt_conf_get_next_static_entry(&temp_entry)) == ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
            break;
        }
        //T_N("[static]isid: %lu,%lu, port: %lu,%lu, vid %lu,%lu", temp_entry.isid, entry->isid, temp_entry.port_no, entry->port_no, temp_entry.vid, entry->vid);
        //T_N("[static]db= %s, s= %s, t=%s", misc_ipv4_txt( (temp_entry.assigned_ip) , buf1), misc_ipv4_txt( (entry->assigned_ip) , buf2), misc_ipv4_txt( target_ip , buf3));
        if (temp_entry.vid == entry->vid && temp_entry.isid == entry->isid && temp_entry.port_no == entry->port_no && (temp_entry.assigned_ip == entry->assigned_ip || temp_entry.assigned_ip == target_ip)) {
            found = TRUE;
            break;
        }
    };

    if (!found) {
        memset(&temp_entry, 0x0, sizeof(temp_entry));
        while (1) {
            if ((rc = _arp_inspection_mgmt_conf_get_next_dynamic_entry(&temp_entry)) == ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
                break;
            }
            //T_N("[dynamic]isid: %lu,%lu, port: %lu,%lu, vid %lu,%lu", temp_entry.isid, entry->isid, temp_entry.port_no, entry->port_no, temp_entry.vid, entry->vid);
            //T_N("[dynamic]db= %s, s= %s, t=%s", misc_ipv4_txt( (temp_entry.assigned_ip) , buf1), misc_ipv4_txt( (entry->assigned_ip) , buf2), misc_ipv4_txt( target_ip , buf3));
            if (temp_entry.vid == entry->vid && temp_entry.isid == entry->isid && temp_entry.port_no == entry->port_no && (temp_entry.assigned_ip == entry->assigned_ip || temp_entry.assigned_ip == target_ip)) {
                break;
            }
        };
    }
    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit rc=%d", rc);
    return rc;
}

/* ARP_INSPECTION entry checking */
static vtss_rc arp_inspection_checking(arp_inspection_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    ARP_INSPECTION_CRIT_ENTER();
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        return VTSS_OK;
    }

    /* Check the VLAN mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].check_VLAN[entry->port_no] == ARP_INSPECTION_MGMT_VLAN_ENABLED) {
        /* Check the VLAN is in DB */
        if (arp_inspection_global.arp_inspection_conf.vlan_mode_conf[entry->vid].flags & ARP_INSPECTION_VLAN_MODE) {
            // VLAN in DB, continue to checking source address
        } else {
            // no checking, forward frame
            ARP_INSPECTION_CRIT_EXIT();
            return VTSS_OK;
        }
    } else {
        // VLAN mode disable, continue to checking source address
    }

    /* Check the entry exist or not ? */
    if ((rc = _arp_inspection_mgmt_conf_get_dynamic_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, return the event
        T_D("the entry existing on dynamic db, exit, rc=%d", rc);
        ARP_INSPECTION_CRIT_EXIT();
        return VTSS_OK;
    }
    if ((rc = _arp_inspection_mgmt_conf_get_static_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, return the event
        T_D("the entry existing on static db, exit, rc=%d", rc);
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return VTSS_OK;
    }

    ARP_INSPECTION_CRIT_EXIT();

    return rc;
}

/* ARP_INSPECTION log entry */
static void arp_inspection_log_entry(arp_inspection_entry_t *entry, arp_inspection_log_type_t type)
{
#ifdef VTSS_SW_OPTION_SYSLOG
    char ip_txt[ARP_INSPECTION_BUF_LENGTH], mac_txt[ARP_INSPECTION_BUF_LENGTH];
    char syslog_txt[256], *syslog_txt_p = &syslog_txt[0];
#endif /* VTSS_SW_OPTION_SYSLOG */

    /* Check the port mode is enabled */
    ARP_INSPECTION_CRIT_ENTER();
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        return;
    }

    /* Check the VLAN mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].check_VLAN[entry->port_no] == ARP_INSPECTION_MGMT_VLAN_ENABLED) {
        /* Check the VLAN is in DB */
        if (arp_inspection_global.arp_inspection_conf.vlan_mode_conf[entry->vid].flags & ARP_INSPECTION_VLAN_MODE) {
            // VLAN in DB, use VLAN log type setting
            switch (type) {
            case ARP_INSPECTION_LOG_DENY:
                if (arp_inspection_global.arp_inspection_conf.vlan_mode_conf[entry->vid].flags & ARP_INSPECTION_VLAN_LOG_DENY) {
#ifdef VTSS_SW_OPTION_SYSLOG
                    syslog_txt_p += sprintf(syslog_txt_p, "ARP packet is denied on");
#if VTSS_SWITCH_STACKABLE
                    syslog_txt_p += sprintf(syslog_txt_p, " switch %d,", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
                    syslog_txt_p += sprintf(syslog_txt_p, " port %d", iport2uport(entry->port_no));
                    syslog_txt_p += sprintf(syslog_txt_p, ", vlan %u, mac %s, sip %s.",
                                            entry->vid,
                                            misc_mac_txt(entry->mac, mac_txt),
                                            misc_ipv4_txt(entry->assigned_ip, ip_txt));
                    S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
                }
                break;
            case ARP_INSPECTION_LOG_PERMIT:
                if (arp_inspection_global.arp_inspection_conf.vlan_mode_conf[entry->vid].flags & ARP_INSPECTION_VLAN_LOG_PERMIT) {
#ifdef VTSS_SW_OPTION_SYSLOG
                    syslog_txt_p += sprintf(syslog_txt_p, "ARP packet is permitted on");
#if VTSS_SWITCH_STACKABLE
                    syslog_txt_p += sprintf(syslog_txt_p, " switch %d,", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
                    syslog_txt_p += sprintf(syslog_txt_p, " port %d", iport2uport(entry->port_no));
                    syslog_txt_p += sprintf(syslog_txt_p, ", vlan %u, mac %s, sip %s.",
                                            entry->vid,
                                            misc_mac_txt(entry->mac, mac_txt),
                                            misc_ipv4_txt(entry->assigned_ip, ip_txt));
                    S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
                }
                break;
            default:
                break;
            }

        }
    } else {
        // VLAN mode disable, use port log type setting
        switch (type) {
        case ARP_INSPECTION_LOG_DENY:
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].log_type[entry->port_no] == ARP_INSPECTION_LOG_DENY ||
                arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].log_type[entry->port_no] == ARP_INSPECTION_LOG_ALL) {
#ifdef VTSS_SW_OPTION_SYSLOG
                syslog_txt_p += sprintf(syslog_txt_p, "ARP packet is denied on");
#if VTSS_SWITCH_STACKABLE
                syslog_txt_p += sprintf(syslog_txt_p, " switch %d,", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
                syslog_txt_p += sprintf(syslog_txt_p, " port %d", iport2uport(entry->port_no));
                syslog_txt_p += sprintf(syslog_txt_p, ", vlan %u, mac %s, sip %s.",
                                        entry->vid,
                                        misc_mac_txt(entry->mac, mac_txt),
                                        misc_ipv4_txt(entry->assigned_ip, ip_txt));
                S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
            }
            break;
        case ARP_INSPECTION_LOG_PERMIT:
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].log_type[entry->port_no] == ARP_INSPECTION_LOG_PERMIT ||
                arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].log_type[entry->port_no] == ARP_INSPECTION_LOG_ALL) {
#ifdef VTSS_SW_OPTION_SYSLOG
                syslog_txt_p += sprintf(syslog_txt_p, "ARP packet is permitted on");
#if VTSS_SWITCH_STACKABLE
                syslog_txt_p += sprintf(syslog_txt_p, " switch %d,", topo_isid2usid(entry->isid));
#endif /* VTSS_SWITCH_STACKABLE */
                syslog_txt_p += sprintf(syslog_txt_p, " port %d", iport2uport(entry->port_no));
                syslog_txt_p += sprintf(syslog_txt_p, ", vlan %u, mac %s, sip %s.",
                                        entry->vid,
                                        misc_mac_txt(entry->mac, mac_txt),
                                        misc_ipv4_txt(entry->assigned_ip, ip_txt));
                S_I(syslog_txt);
#endif /* VTSS_SW_OPTION_SYSLOG */
            }
            break;
        default:
            break;
        }
    }

    ARP_INSPECTION_CRIT_EXIT();
    return;
}

/****************************************************************************/
/*  Reserved ACEs functions                                                 */
/****************************************************************************/

/* Add reserved ACE */
static vtss_rc arp_inspection_ace_add(void)
{
    vtss_rc             rc;
    acl_entry_conf_t    conf;
    ulong               arp_flag;

    if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_ARP, &conf)) != VTSS_OK) {
        return rc;
    }
    conf.id = ARP_INSPECTION_ACE_ID;

#if defined(VTSS_ARCH_SERVAL)
    conf.isdx_disable = TRUE;
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_FEATURE_ACL_V2)
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
#else
    conf.action.permit = FALSE;
#endif /* VTSS_FEATURE_ACL_V2 */
    conf.action.force_cpu = TRUE;
    conf.action.cpu_once = FALSE;
    conf.isid = VTSS_ISID_LOCAL;
    arp_flag = (ulong) ACE_FLAG_ARP_ARP;
    VTSS_BF_SET(conf.flags.mask, arp_flag, 1);
    arp_flag = (ulong) ACE_FLAG_ARP_ARP;
    VTSS_BF_SET(conf.flags.value, arp_flag, 1);
    arp_flag = (ulong) ACE_FLAG_ARP_UNKNOWN;
    VTSS_BF_SET(conf.flags.mask, arp_flag, 1);
    arp_flag = (ulong) ACE_FLAG_ARP_UNKNOWN;
    VTSS_BF_SET(conf.flags.value, arp_flag, 0);
    arp_flag = (ulong) ACE_FLAG_ARP_REQ;
    VTSS_BF_SET(conf.flags.mask, arp_flag, 1);
    arp_flag = (ulong) ACE_FLAG_ARP_REQ;
    VTSS_BF_SET(conf.flags.value, arp_flag, 1);

    return (acl_mgmt_ace_add(ACL_USER_ARP_INSPECTION, ACE_ID_NONE, &conf));
}

/* Delete reserved ACE */
static vtss_rc arp_inspection_ace_del(void)
{
    return (acl_mgmt_ace_del(ACL_USER_ARP_INSPECTION, ARP_INSPECTION_ACE_ID));
}

/****************************************************************************/
/*  ARP inspection Allocate functions                                       */
/****************************************************************************/

/* Allocate request buffer */
static arp_inspection_msg_req_t *arp_inspection_alloc_pkt_message(size_t size, arp_inspection_msg_id_t msg_id)
{
    arp_inspection_msg_req_t *msg = VTSS_MALLOC(size);

    if (msg) {
        msg->msg_id = msg_id;
    }
    T_D("msg len %zd, type %d => %p", size, msg_id, msg);

    return msg;
}

/* Allocate request buffer for sending packets */
static arp_inspection_msg_req_t *arp_inspection_alloc_message(size_t size, arp_inspection_msg_id_t msg_id)
{
    arp_inspection_msg_req_t *msg = VTSS_MALLOC(size);

    if (msg) {
        msg->msg_id = msg_id;
    }
    T_D("msg len %zd, type %d => %p", size, msg_id, msg);

    return msg;
}

/* Alloc memory for transmit ARP frame */
static void *arp_inspection_alloc_xmit(size_t len,
                                       unsigned long vid,
                                       unsigned long isid,
                                       BOOL *members,
                                       void **pbufref)
{
    void *p = NULL;

    if (msg_switch_is_local(isid)) {
        p = packet_tx_alloc(len);
        *pbufref = NULL;    /* Local operation */
    } else {                /* Remote */
        arp_inspection_msg_req_t *msg = arp_inspection_alloc_message(sizeof(arp_inspection_msg_req_t) + len, ARP_INSPECTION_MSG_ID_FRAME_TX_REQ);

        if (msg) {
            msg->req.tx_req.len = len;
            msg->req.tx_req.vid = vid;
            msg->req.tx_req.isid = isid;
            memcpy(msg->req.tx_req.port_list, members, VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));
            *pbufref = (void *) msg; /* Remote op */
            p = ((unsigned char *) msg) + sizeof(*msg);
        } else {
            T_E("Allocation failure, TX length %zd", len);
        }
    }

    T_D("%s(%zd) ret %p", __FUNCTION__, len, p);

    return p;
}

/****************************************************************************/
/*  ARP inspection transmit functions                                       */
/****************************************************************************/

/* Transmit ARP frame
   Return 0  : Success
   Return -1 : Fail */
static int arp_inspection_xmit(void *frame,
                               size_t len,
                               unsigned long vid,
                               unsigned long isid,
                               BOOL *members,
                               void *bufref,
                               int is_relay)
{
    arp_inspection_msg_req_t    *msg = (arp_inspection_msg_req_t *)bufref;
    port_iter_t                 pit;
#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
    vtss_packet_port_info_t     info;
    vtss_packet_port_filter_t   filter[VTSS_PORT_ARRAY_SIZE];
#endif

    T_D("%s(%p, %zd, %ld, %ld)", __FUNCTION__, frame, len, isid, vid);

    if (msg) {
        if (msg_switch_is_local(msg->req.tx_req.isid)) {
            T_E("ISID became local (%ld)?", msg->req.tx_req.isid);
            VTSS_FREE(msg);
            return -1;
        } else {
            msg_tx(VTSS_MODULE_ID_ARP_INSPECTION,
                   msg->req.tx_req.isid, msg, len + sizeof(*msg));
        }
    } else {

        if (!frame) {
            T_W("no packet need to send");
            return 0;
        }

#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
        // get port information by vid
        (void) vtss_packet_port_info_init(&info);
        info.vid = vid;
        (void) vtss_packet_port_filter_get(NULL, &info, filter);
#endif

        // transmit frame
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (members[pit.iport]) {
                uchar *buffer;

#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
                T_D("VID %lu, filter %d, tpid %x", vid, filter[pit.iport].filter, filter[pit.iport].tpid);

                switch (filter[pit.iport].filter) {
                case VTSS_PACKET_FILTER_TAGGED:
                    buffer = packet_tx_alloc(len + 4);
                    if (buffer) {
                        packet_tx_props_t   tx_props;
                        uchar               *frame_ptr = frame;
                        uchar               vlan_tag[4];

                        /* Fill out VLAN tag */
                        memset(vlan_tag, 0x0, sizeof(vlan_tag));
                        pack16(filter[pit.iport].tpid, vlan_tag);
                        pack16(vid, vlan_tag + 2);

                        /* process VLAN tagging issue */
                        memcpy(buffer, frame_ptr, 12); // DMAC & SMAC
                        memcpy(buffer + 12, vlan_tag, 4); // VLAN Header
                        memcpy(buffer + 12 + 4, frame_ptr + 12, len - 12); // Remainder of frame

                        packet_tx_props_init(&tx_props);
                        tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                        tx_props.packet_info.frm[0]    = buffer;
                        tx_props.packet_info.len[0]    = len + 4;
                        tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                        if (packet_tx(&tx_props) != VTSS_RC_OK) {
                            T_E("Frame transmit on port %d failed", pit.iport);
                            return -1;
                        }
                    } else {
                        T_W("allocation failure, length %zd", len + 4);
                    }
                    break;
                case VTSS_PACKET_FILTER_UNTAGGED:
                    buffer = packet_tx_alloc(len);
                    if (buffer) {
                        packet_tx_props_t tx_props;
                        memcpy(buffer, frame, len);
                        packet_tx_props_init(&tx_props);
                        tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                        tx_props.packet_info.frm[0]    = buffer;
                        tx_props.packet_info.len[0]    = len;
                        tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                        if (packet_tx(&tx_props) != VTSS_RC_OK) {
                            T_E("Frame transmit on port %d failed", pit.iport);
                            return -1;
                        }
                    } else {
                        T_W("allocation failure, length %zd", len);
                    }
                    break;
                case VTSS_PACKET_FILTER_DISCARD:
                    T_D("VTSS_PACKET_FILTER_DISCARD");
                    break;
                default:
                    T_E("unknown ID: %d", filter[pit.iport].filter);
                    break;
                }
#else
                buffer = packet_tx_alloc(len);
                if (buffer) {
                    packet_tx_props_t tx_props;
                    memcpy(buffer, frame, len);
                    packet_tx_props_init(&tx_props);
                    tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                    tx_props.packet_info.frm[0]    = buffer;
                    tx_props.packet_info.len[0]    = len;
                    tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                    if (packet_tx(&tx_props) != VTSS_RC_OK) {
                        T_E("Frame transmit on port %ld failed", pit.iport);
                        return -1;
                    }
                } else {
                    T_W("allocation failure, length %zd", len);
                }
#endif
            }
        }

        // release packet
        if (frame) {
            packet_tx_free(frame);
        }
    }

    return 0;
}

/****************************************************************************/
/*  MSG/Debug Function                                                      */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
/* ARP_INSPECTION msg text */
static char *arp_inspection_msg_id_txt(arp_inspection_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ:
        txt = "ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ";
        break;
    case ARP_INSPECTION_MSG_ID_FRAME_RX_IND:
        txt = "ARP_INSPECTION_MSG_ID_FRAME_RX_IND";
        break;
    case ARP_INSPECTION_MSG_ID_FRAME_TX_REQ:
        txt = "ARP_INSPECTION_MSG_ID_FRAME_TX_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* ARP_INSPECTION error text */
char *arp_inspection_error_txt(vtss_rc rc)
{
    switch (rc) {
    case ARP_INSPECTION_ERROR_MUST_BE_MASTER:
        return "ARP Inspection: operation only valid on master switch.";

    case ARP_INSPECTION_ERROR_ISID:
        return "ARP Inspection: invalid Switch ID.";

    case ARP_INSPECTION_ERROR_ISID_NON_EXISTING:
        return "Switch ID is non-existing";

    case ARP_INSPECTION_ERROR_INV_PARAM:
        return "ARP Inspection: invalid parameter supplied to function.";

    case ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND:
        return "ARP Inspection: databse access error.";

    case ARP_INSPECTION_ERROR_ENTRY_EXIST_ON_DB:
        return "ARP Inspection: the entry already exists in the database.";

    case ARP_INSPECTION_ERROR_TABLE_FULL:
        return "ARP Inspection: table is full.";

    default:
        return "ARP Inspection: unknown error code.";
    }
}

/****************************************************************************/
/*  Static Function                                                         */
/****************************************************************************/

/* Get ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_get(arp_inspection_entry_t *entry, BOOL next)
{
    vtss_rc rc = VTSS_OK;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();
    if (next) {
        rc = _arp_inspection_mgmt_conf_get_next_static_entry(entry);
    } else {
        rc = _arp_inspection_mgmt_conf_get_first_static_entry(entry);
    }
    ARP_INSPECTION_CRIT_EXIT();

    return rc;
}

/* Set ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_set(arp_inspection_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(entry->isid)) {
        T_W("isid: %d isn't configurable switch", entry->isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
    if (entry->assigned_ip == 0) {
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* Check the entry exist or not ? */
    if ((rc = _arp_inspection_mgmt_conf_get_dynamic_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, transfer the dynamic entry into static entry
        T_D("the entry existing on dynamic db, transfer, rc=%d", rc);
        if ((rc = _arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(entry)) == VTSS_OK) {
            // save configuration
            rc = _arp_inspection_mgmt_conf_save_static_configuration();
        }
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }
    if ((rc = _arp_inspection_mgmt_conf_get_static_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, return the event
        T_D("the entry existing on static db, exit, rc=%d", rc);
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return ARP_INSPECTION_ERROR_ENTRY_EXIST_ON_DB;
    }

    /* Check total count reach the max value or not? */
    if (_arp_inspection_entry_count() >= ARP_INSPECTION_MAX_ENTRY_CNT) {
        T_D("total count, rc=%d", _arp_inspection_entry_count());
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        T_W("arp inspection: table full.");
        return ARP_INSPECTION_ERROR_TABLE_FULL;
    }

    /* add the entry into static DB */
    if ((rc = _arp_inspection_mgmt_conf_add_static_entry(entry, TRUE)) == VTSS_OK) {
        /* save configuration */
        rc = _arp_inspection_mgmt_conf_save_static_configuration();
    }

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Del ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_static_entry_del(arp_inspection_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(entry->isid)) {
        T_W("isid: %d isn't configurable switch", entry->isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID_NON_EXISTING;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* Check if entry exist? */
    rc = _arp_inspection_mgmt_conf_get_static_entry(entry);
    if (rc == ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if not existing, return
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* delete the entry on static DB */
    if ((rc = _arp_inspection_mgmt_conf_del_static_entry(entry, TRUE)) != VTSS_OK) {
        T_W("_arp_inspection_mgmt_conf_del_static_entry failed");
    } else {
        /* save configuration */
        rc = _arp_inspection_mgmt_conf_save_static_configuration();
    }

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Delete all ARP_INSPECTION static entry */
vtss_rc arp_inspection_mgmt_conf_all_static_entry_del(void)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* Check current entry count */
    if (!_arp_inspection_get_static_count()) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return VTSS_OK;
    }

    /* Delete all static entries */
    if (_arp_inspection_mgmt_conf_del_all_static_entry(TRUE) != VTSS_OK) {
        T_W("_arp_inspection_mgmt_conf_del_all_static_entry() failed");
    }

    /* save configuration */
    rc = _arp_inspection_mgmt_conf_save_static_configuration();

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Reset ARP_INSPECTION VLAN database */
vtss_rc arp_inspection_mgmt_conf_vlan_entry_del(void)
{
    vtss_rc     rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* clear global cache memory */
    memset(arp_inspection_global.arp_inspection_conf.vlan_mode_conf, 0x0, VTSS_VIDS * sizeof(arp_inspection_vlan_mode_conf_t));

    ARP_INSPECTION_CRIT_EXIT();

    /* save configuration */
    rc = arp_inspection_mgmt_conf_vlan_mode_save();

    T_D("exit");
    return rc;
}

/****************************************************************************/
/*  Dynamic Function                                                        */
/****************************************************************************/

/* Get ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_get(arp_inspection_entry_t *entry, BOOL next)
{
    vtss_rc rc = VTSS_OK;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();
    if (next) {
        rc = _arp_inspection_mgmt_conf_get_next_dynamic_entry(entry);
    } else {
        rc = _arp_inspection_mgmt_conf_get_first_dynamic_entry(entry);
    }
    ARP_INSPECTION_CRIT_EXIT();

    return rc;
}

/* Set ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_set(arp_inspection_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_exists(entry->isid)) {
        T_D("isid: %d not exist", entry->isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* Check system mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.mode == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* Check port mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* Check the entry exist or not ? */
    if ((rc = _arp_inspection_mgmt_conf_get_dynamic_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, return the event
        T_D("the entry existing on dynamic db, exit, rc=%d", rc);
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return ARP_INSPECTION_ERROR_ENTRY_EXIST_ON_DB;
    }
    if ((rc = _arp_inspection_mgmt_conf_get_static_entry(entry)) != ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
        //if existing, return the event
        T_D("the entry existing on static db, exit, rc=%d", rc);
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return ARP_INSPECTION_ERROR_ENTRY_EXIST_ON_DB;
    }

    /* Check total count reach the max value or not? */
    if (_arp_inspection_entry_count() >= ARP_INSPECTION_MAX_ENTRY_CNT) {
        T_D("dynamic count, rc=%d", _arp_inspection_entry_count());
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        T_W("arp inspection: table full.");
        return ARP_INSPECTION_ERROR_TABLE_FULL;
    }

    /* add the entry into dynamic DB */
    if ((rc = _arp_inspection_mgmt_conf_add_dynamic_entry(entry, TRUE)) != VTSS_OK) {
        T_D("_arp_inspection_mgmt_conf_add_dynamic_entry() failed");
    }

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Delete ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_del(arp_inspection_entry_t *entry)
{
    vtss_rc     rc = VTSS_OK;

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_exists(entry->isid)) {
        T_D("isid: %d not exist", entry->isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* Check system mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.mode == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* Check port mode is enabled */
    if (arp_inspection_global.arp_inspection_conf.port_mode_conf[entry->isid - VTSS_ISID_START].mode[entry->port_no] == ARP_INSPECTION_MGMT_DISABLED) {
        ARP_INSPECTION_CRIT_EXIT();
        T_D("exit");
        return rc;
    }

    /* delete the entry on dynamic DB */
    if ((rc = _arp_inspection_mgmt_conf_del_dynamic_entry(entry, TRUE)) != VTSS_OK) {
        T_D("_arp_inspection_mgmt_conf_del_dynamic_entry() failed");
    }

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Check ARP_INSPECTION dynamic entry */
vtss_rc arp_inspection_mgmt_conf_dynamic_entry_check(arp_inspection_entry_t *check_entry)
{
    arp_inspection_entry_t      entry;

    if (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, FALSE) == VTSS_OK) {

        if (arp_inspection_entry_compare_func(&entry, check_entry) == 0) {
            return VTSS_OK;
        }
        while (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, TRUE) == VTSS_OK) {
            if (arp_inspection_entry_compare_func(&entry, check_entry) == 0) {
                return VTSS_OK;
            }
        }
    }

    return VTSS_INCOMPLETE;
}

/* del all ARP_INSPECTION dynamic entry */
static void arp_inspection_mgmt_conf_all_dynamic_entry_del(void)
{
    ARP_INSPECTION_CRIT_ENTER();

    /* Delete all dynamic entries */
    if (_arp_inspection_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
        T_W("_arp_inspection_mgmt_conf_del_all_dynamic_entry() failed");
    }

    ARP_INSPECTION_CRIT_EXIT();

    return;
}

/* flush ARP_INSPECTION dynamic entry by port */
static vtss_rc arp_inspection_mgmt_conf_flush_dynamic_entry_by_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    arp_inspection_entry_t      entry;

    if (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, FALSE) == VTSS_OK) {
        if ((entry.isid == isid) && (entry.port_no == port_no)) {
            if (arp_inspection_mgmt_conf_dynamic_entry_del(&entry)) {
                T_W("arp_inspection_mgmt_conf_dynamic_entry_del() failed");
            }
        }
        while (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, TRUE) == VTSS_OK) {
            if ((entry.isid == isid) && (entry.port_no == port_no)) {
                if (arp_inspection_mgmt_conf_dynamic_entry_del(&entry)) {
                    T_W("arp_inspection_mgmt_conf_dynamic_entry_del() failed");
                }
            }
        }
    }

    return VTSS_OK;
}

/* Translate ARP_INSPECTION dynamic entries into static entries */
vtss_rc arp_inspection_mgmt_conf_translate_dynamic_into_static(void)
{
    vtss_rc                     rc = VTSS_OK;
    arp_inspection_entry_t      entry;
    int                         count = 0;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* translate dynamic entries into static entries */
    if (_arp_inspection_mgmt_conf_get_first_dynamic_entry(&entry) == VTSS_OK) {
        if ((rc = _arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry)) != VTSS_OK) {
            T_D("_arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry() failed");
        } else {
            count++;
        }

        while (_arp_inspection_mgmt_conf_get_next_dynamic_entry(&entry) == VTSS_OK) {
            if ((rc = _arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry)) != VTSS_OK) {
                T_D("_arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry() failed");
            } else {
                count++;
            }
        }
    }

    /* save configuration */
    rc = _arp_inspection_mgmt_conf_save_static_configuration();

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    if (rc < VTSS_OK) {
        return rc;
    } else {
        return count;
    }
}

/* Translate ARP_INSPECTION dynamic entry into static entry */
vtss_rc arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(arp_inspection_entry_t *entry)
{
    vtss_rc                     rc = VTSS_OK;
    int                         count = 0;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }

    ARP_INSPECTION_CRIT_ENTER();

    /* translate dynamic entry into static entry */
    if ((rc = _arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(entry)) != VTSS_OK) {
        T_D("_arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry() failed");
    } else {
        count++;
    }

    /* save configuration */
    rc = _arp_inspection_mgmt_conf_save_static_configuration();

    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    if (rc < VTSS_OK) {
        return rc;
    } else {
        return count;
    }
}

/****************************************************************************/
/*  Callback Function                                                       */
/****************************************************************************/

/* ARP_INSPECTION callback function for master */
static void
arp_inspection_do_rx_callback(const void *packet,
                              size_t len,
                              ulong vid,
                              ulong isid,
                              ulong port_no)
{
    vtss_rc                     rc = VTSS_OK;
    vtss_arp_header             *ar;
    arp_inspection_entry_t      entry, temp_entry;
    void                        *bufref = NULL;
    uchar                       *pkt_buf;
    port_info_t                 port_info;
    vtss_packet_frame_info_t    info;
    vtss_packet_filter_t        filter = 0;
    uchar                       *ptr = (uchar *)(packet);
    port_status_t               port_module_status;
    char                        buf[ARP_INSPECTION_BUF_LENGTH];
    //char                        buf1[ARP_INSPECTION_BUF_LENGTH];
    //char                        buf2[ARP_INSPECTION_BUF_LENGTH];
    BOOL                        port_list[VTSS_PORT_ARRAY_SIZE];
    BOOL                        is_gratuitous_arp = FALSE;
    BOOL                        is_myself_arp = FALSE;
    BOOL                        send_packet = FALSE;
    switch_iter_t               sit;
    port_iter_t                 pit;
    vtss_ip_addr_t              ipv4_addr;

    /* Fill out frame information for filtering */
    vtss_packet_frame_info_init(&info);
    info.port_no = port_no;                         /* Ingress port number or zero */
    info.vid = vid;

    if (VTSS_ISID_LEGAL(isid)) {   /* Bypass message module! */
        T_D("enter, port_no: %u ,len %u ,vid %u ,isid %u", port_no, len, vid, isid);
        memset(&entry, 0, sizeof(entry));
//        T_E_HEX(frm, rx_info->length);
        ar = (vtss_arp_header *)(ptr + ARP_INSPECTION_MAC_LENGTH + ARP_INSPECTION_MAC_LENGTH + 2);  //DA, SA, and ether type
//        T_E("src_port = %d, vid = %d, arp sa = %s, aip = %s", rx_info->port_no, rx_info->tag.vid, misc_mac_txt(ar->ar_sha,buf1), misc_ipv4_txt(htonl(ar->ar_spa),buf2));
        // checking database
        memcpy(entry.mac, ar->ar_sha, ARP_INSPECTION_MAC_LENGTH * sizeof(uchar));
        entry.vid = vid;
        entry.assigned_ip = htonl(ar->ar_spa);
        entry.isid = isid;
        entry.port_no = port_no;

        if (vtss_ip2_ip_by_vlan(vid, VTSS_IP_TYPE_IPV4, &ipv4_addr) != VTSS_OK) {
            T_D("Get Current IP Address failed !!!!");
        } else {
            if ((htonl(ar->ar_tpa) == ipv4_addr.addr.ipv4)) {
                is_myself_arp = TRUE;
            }
        }

        /* Pass gratuitous ARP to IP stack first */
        is_gratuitous_arp = (ar->ar_spa == ar->ar_tpa);
        if (is_gratuitous_arp) {
            if (vtss_ip2_if_inject((vtss_if_id_vlan_t) vid, len, packet)) {
                T_W("Calling vtss_ip2_if_inject() failed.\n");
            }
        }

        temp_entry = entry;
        if ((rc = arp_inspection_checking(&entry)) == VTSS_OK) {
            //if source address is existing, forwarding frames
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                if (!msg_switch_exists(sit.isid)) {
                    continue;
                }
                memset(port_list, 0x0, sizeof(port_list));
                send_packet = FALSE;

                (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                while (port_iter_getnext(&pit)) {
                    info.port_tx = pit.iport;

                    if (msg_switch_is_local(sit.isid)) {
                        /* check link status on master */
                        if (port_info_get(pit.iport, &port_info) != VTSS_OK ||
                            port_info.link == 0 ) {
                            //T_R("port_no %lu, link %d", pit.iport, port_info.link);
                            continue;
                        }
                        /* avoid packet loop */
                        if ( sit.isid == isid ) {
                            if (vtss_packet_frame_filter(NULL, &info, &filter) != VTSS_OK ||
                                filter == VTSS_PACKET_FILTER_DISCARD) {
                                //T_R("port_no %lu, isid %u, filter %d", pit.iport, sit.isid, filter);
                                continue;
                            }
                        }
                    } else {
                        /* check link status on slave */
                        if (port_mgmt_status_get(sit.isid, pit.iport, &port_module_status) == VTSS_OK && port_module_status.status.link == 0 ) {
                            //T_R("isid %u, port_no %lu", sit.isid, pit.iport);
                            continue;
                        }
                        /* avoid packet loop */
                        if (sit.isid == isid) {
                            if ( pit.iport == port_no ) {
                                //T_R("isid %u, port_no %lu", sit.isid, pit.iport);
                                continue;
                            }
                        }
                    }

                    T_D("isid: %u, isid_idx: %u, port: %u", isid, sit.isid, pit.iport );
                    /* Only forward ARP packet to enabled ports
                       when sender/target IP is matched ARP inspection entries or gratuitous ARP */
                    if (!is_gratuitous_arp) {
                        temp_entry.isid = sit.isid;
                        temp_entry.port_no = pit.iport;
                        if (arp_inspection_entry_checking_matched_ipv4(&temp_entry, htonl(ar->ar_tpa)) != VTSS_OK) {
                            continue;
                        }
                    }

                    T_D("target protocol address= %s, iport= %d", misc_ipv4_txt(htonl(ar->ar_tpa), buf), pit.iport);
                    T_D("sender protocol address= %s, iport= %d", misc_ipv4_txt(htonl(ar->ar_spa), buf), pit.iport);

                    T_D("ARP transmit, isid %u, vid %u, port_no %u", sit.isid, entry.vid, pit.iport);
                    port_list[pit.iport] = TRUE;
                    send_packet = TRUE;
                }
                if (send_packet) {
                    /* Alloc memory for transmit ARP frame */
                    if ((pkt_buf = arp_inspection_alloc_xmit(len, entry.vid, sit.isid, port_list, &bufref)) != NULL) {
                        memcpy(pkt_buf, ptr, len);
                        if (arp_inspection_xmit(pkt_buf, len, entry.vid, sit.isid, port_list, bufref, TRUE)) {
                            T_W("arp_inspection_xmit() transmit failed");
                        }
                    }

                    // log here, permit log
                    //T_E("1 ARP packet is permitted, port %lu, vlan %u, mac %s, sip %s, tip %s.", iport2uport(entry.port_no), entry.vid, misc_mac_txt(ar->ar_sha, buf), misc_ipv4_txt(htonl(ar->ar_spa), buf1), misc_ipv4_txt(htonl(ar->ar_tpa), buf2));
                    arp_inspection_log_entry(&entry, ARP_INSPECTION_LOG_PERMIT);
                } else {
                    // log here, deny log
                    //T_E("2 ARP packet is denied, port %lu, vlan %u, mac %s, sip %s, tip %s.", iport2uport(entry.port_no), entry.vid, misc_mac_txt(ar->ar_sha, buf), misc_ipv4_txt(htonl(ar->ar_spa), buf1), misc_ipv4_txt(htonl(ar->ar_tpa), buf2));
                    arp_inspection_log_entry(&entry, ARP_INSPECTION_LOG_DENY);
                }

                /* Pass myself ARP to IP stack */
                if (msg_switch_is_local(sit.isid)) {
                    if (is_myself_arp && !is_gratuitous_arp) {
                        if (vtss_ip2_if_inject((vtss_if_id_vlan_t) vid, len, packet)) {
                            T_W("Calling vtss_ip2_if_inject() failed.\n");
                        }
                    }
                }

            }
        }


        if (rc == ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND) {
            // log here, deny log
            //T_E("3 ARP packet is denied, port %lu, vlan %u, mac %s, sip %s, tip %s.", iport2uport(entry.port_no), entry.vid, misc_mac_txt(ar->ar_sha, buf), misc_ipv4_txt(htonl(ar->ar_spa), buf1), misc_ipv4_txt(htonl(ar->ar_tpa), buf2));
            arp_inspection_log_entry(&entry, ARP_INSPECTION_LOG_DENY);
        }

        return;
    }
    return;
}

static void arp_inspection_bip_buffer_enqueue(const u8 *const packet,
                                              size_t len,
                                              vtss_vid_t vid,
                                              vtss_isid_t isid,
                                              vtss_port_no_t port_no)
{
    arp_inspection_bip_buf_t *bip_buf;

    /* Check input parameters */
    if (packet == NULL || len == 0) {
        return;
    }

    ARP_INSPECTION_BIP_CRIT_ENTER();
    bip_buf = (arp_inspection_bip_buf_t *) vtss_bip_buffer_reserve(&arp_inspection_bip_buf, sizeof(*bip_buf));
    ARP_INSPECTION_BIP_CRIT_EXIT();
    if (bip_buf == NULL) {
        T_D("Failure in reserving DHCP Helper BIP buffer");
        return;
    }

    memcpy(bip_buf->pkt, packet, len);
    bip_buf->len     = len;
    bip_buf->vid     = vid;
    bip_buf->isid    = isid;
    bip_buf->port_no = port_no;

    ARP_INSPECTION_BIP_CRIT_ENTER();
    vtss_bip_buffer_commit(&arp_inspection_bip_buf);
    cyg_flag_setbits(&arp_inspection_bip_buffer_thread_events, ARP_INSPECTION_EVENT_PKT_RECV);
    ARP_INSPECTION_BIP_CRIT_EXIT();
}

static void arp_inspection_bip_buffer_dequeue(void)
{
    arp_inspection_bip_buf_t   *bip_buf;
    int                        buf_size;
    static u32                 cnt = 0;

    do {
        ARP_INSPECTION_BIP_CRIT_ENTER();
        bip_buf = (arp_inspection_bip_buf_t *) vtss_bip_buffer_get_contiguous_block(&arp_inspection_bip_buf, &buf_size);
        ARP_INSPECTION_BIP_CRIT_EXIT();
        if (bip_buf) {
            arp_inspection_do_rx_callback((u8 *)bip_buf->pkt, bip_buf->len, bip_buf->vid, bip_buf->isid, bip_buf->port_no);
            ARP_INSPECTION_BIP_CRIT_ENTER();
            vtss_bip_buffer_decommit_block(&arp_inspection_bip_buf, sizeof(arp_inspection_bip_buf_t));
            ARP_INSPECTION_BIP_CRIT_EXIT();
        }

        // To avoid the busy-loop process that cannot access DUT
        if ((++cnt % 100) == 0) {
            cnt = 0;
            VTSS_OS_MSLEEP(10);
        }
    } while (bip_buf);
}

/****************************************************************************/
/*  ARP_INSPECTION receive functions                                        */
/****************************************************************************/

/* ARP_INSPECTION receive function for slave */
static void arp_inspection_receive_indication(const void *packet,
                                              size_t len,
                                              vtss_port_no_t switchport,
                                              vtss_vid_t vid,
                                              vtss_glag_no_t glag_no)
{
    T_D("len %zd port %u vid %d glag %u", len, switchport, vid, glag_no);

    if (msg_switch_is_master() && VTSS_ISID_LEGAL(master_isid)) {   /* Bypass message module! */
        arp_inspection_bip_buffer_enqueue(packet, len, vid, master_isid, switchport);
    } else {
        size_t msg_len = sizeof(arp_inspection_msg_req_t) + len;
        arp_inspection_msg_req_t *msg = arp_inspection_alloc_pkt_message(msg_len, ARP_INSPECTION_MSG_ID_FRAME_RX_IND);

        if (msg) {
            msg->req.rx_ind.len = len;
            msg->req.rx_ind.vid = vid;
            msg->req.rx_ind.port_no = switchport;
            memcpy(&msg[1], packet, len); /* Copy frame */
            // These frames are subject to shaping.
            msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK | MSG_TX_OPT_SHAPE, VTSS_MODULE_ID_ARP_INSPECTION, 0, msg, msg_len);
        } else {
            T_W("Unable to allocate %zd bytes, tossing frame on port %u", msg_len, switchport);
        }
    }

    return;
}

/****************************************************************************/
/*  Rx filter register functions                                            */
/****************************************************************************/

/* Local port packet receive indication - forward through arp_inspection */
static BOOL arp_inspection_rx_packet_callback(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    /* If this a slave, use the message to pack the packet and then transmit to the master */
    T_D("enter, port_no: %u len %d vid %d glag %u", rx_info->port_no, rx_info->length, rx_info->tag.vid, rx_info->glag_no);

    // NB: Null out the GLAG (port is 1st in aggr)
    arp_inspection_receive_indication(frm, rx_info->length, rx_info->port_no, rx_info->tag.vid,
                                      (vtss_glag_no_t)(VTSS_GLAG_NO_START - 1));

    T_D("exit");
    return TRUE; // Do not allow other subscribers to receive the packet
}

/* ARP_INSPECTION rx register function */
static void arp_inspection_rx_filter_register(BOOL registerd)
{
    ARP_INSPECTION_CRIT_ENTER();

    if (!arp_inspection_filter_id) {
        memset(&arp_inspection_rx_filter, 0x0, sizeof(arp_inspection_rx_filter));
    }

    arp_inspection_rx_filter.modid = VTSS_MODULE_ID_ARP_INSPECTION;
    arp_inspection_rx_filter.match = PACKET_RX_FILTER_MATCH_ACL | PACKET_RX_FILTER_MATCH_ETYPE;
    arp_inspection_rx_filter.etype = 0x0806; // ARP
    arp_inspection_rx_filter.prio = PACKET_RX_FILTER_PRIO_NORMAL;
    arp_inspection_rx_filter.cb = arp_inspection_rx_packet_callback;

    if (registerd && !arp_inspection_filter_id) {
        if (packet_rx_filter_register(&arp_inspection_rx_filter, &arp_inspection_filter_id)) {
            T_W("packet_rx_filter_register() failed");
        }
    } else if (!registerd && arp_inspection_filter_id) {
        if (packet_rx_filter_unregister(arp_inspection_filter_id) == VTSS_OK) {
            arp_inspection_filter_id = NULL;
        }
    }

    ARP_INSPECTION_CRIT_EXIT();

    return;
}

/****************************************************************************/
/*  Receive register functions                                              */
/****************************************************************************/
/* Register link status change callback */
static void arp_inspection_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    if (msg_switch_is_master() && !info->stack) {
        T_D("port_no: [%d,%u] link %s", isid, port_no, info->link ? "up" : "down");
        if (!msg_switch_exists(isid)) { /* IP interface maybe change, don't send trap */
            return;
        }
        if (!info->link) {
            if (arp_inspection_mgmt_conf_flush_dynamic_entry_by_port(isid, port_no)) {
                T_W("arp_inspection_mgmt_conf_flush_dynamic_entry_by_port() failed");
            }
        }
    }

    return;
}

/* Register DHCP receive */
static void arp_inspection_dhcp_pkt_receive(dhcp_snooping_ip_assigned_info_t *info, dhcp_snooping_info_reason_t reason)
{
    arp_inspection_entry_t entry;

    memcpy(entry.mac, info->mac, ARP_INSPECTION_MAC_LENGTH);
    entry.vid = info->vid;
    entry.assigned_ip = info->assigned_ip;
    entry.isid = info->isid;
    entry.port_no = info->port_no;
    entry.type = ARP_INSPECTION_DYNAMIC_TYPE;

    if (reason == DHCP_SNOOPING_INFO_REASON_ASSIGN_COMPLETED) {
        entry.valid = TRUE;
        if (arp_inspection_mgmt_conf_dynamic_entry_set(&entry)) {
            T_D("arp_inspection_mgmt_conf_dynamic_entry_set() failed");
        }
    } else {
        entry.valid = FALSE;
        if (arp_inspection_mgmt_conf_dynamic_entry_del(&entry)) {
            T_D("arp_inspection_mgmt_conf_dynamic_entry_del() failed");
        }
    }

    return;
}

/* Register ARP Inspection receive */
static void arp_inspection_receive_register(void)
{
    uchar                               mac[ARP_INSPECTION_MAC_LENGTH], null_mac[ARP_INSPECTION_MAC_LENGTH] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    vtss_vid_t                          vid;
    dhcp_snooping_ip_assigned_info_t    info;
    arp_inspection_entry_t              entry;

    if (msg_switch_is_master()) {

        memcpy(mac, null_mac, sizeof(mac));
        vid = 0;
        while (dhcp_snooping_ip_assigned_info_getnext(mac, vid, &info)) {
            memcpy(entry.mac, info.mac, ARP_INSPECTION_MAC_LENGTH);
            entry.vid = info.vid;
            entry.assigned_ip = info.assigned_ip;
            entry.isid = info.isid;
            entry.port_no = info.port_no;
            entry.type = ARP_INSPECTION_DYNAMIC_TYPE;
            entry.valid = TRUE;

            if (arp_inspection_mgmt_conf_dynamic_entry_set(&entry)) {
                T_D("arp_inspection_mgmt_conf_dynamic_entry_set() failed");
            }
            memcpy(mac, info.mac, ARP_INSPECTION_MAC_LENGTH);
            vid = info.vid;
        };

        dhcp_snooping_ip_assigned_info_register(arp_inspection_dhcp_pkt_receive);
    }

    arp_inspection_rx_filter_register(TRUE);
    if (arp_inspection_ace_add()) {
        T_W("arp_inspection_ace_add() failed");
    }

    return;
}

/* Unregister arp_inspection receive */
static void arp_inspection_receive_unregister(void)
{
    if (arp_inspection_ace_del()) {
        T_D("arp_inspection_ace_del() failed");
    }
    arp_inspection_rx_filter_register(FALSE);

    if (msg_switch_is_master()) {
        dhcp_snooping_ip_assigned_info_unregister(arp_inspection_dhcp_pkt_receive);

        /* Clear all dynamic entries */
        arp_inspection_mgmt_conf_all_dynamic_entry_del();
    }

    return;
}

/****************************************************************************/
/*  MSG Function                                                            */
/****************************************************************************/

/* Free request/reply buffer */
static void arp_inspection_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);

    return;
}

/* ARP_INSPECTION msg done */
static void arp_inspection_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    arp_inspection_msg_id_t msg_id = *(arp_inspection_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, arp_inspection_msg_id_txt(msg_id));
    arp_inspection_msg_free(contxt);

    return;
}

/* ARP_INSPECTION msg tx */
static void arp_inspection_msg_tx(arp_inspection_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    arp_inspection_msg_id_t msg_id = *(arp_inspection_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, arp_inspection_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, arp_inspection_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_ARP_INSPECTION, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(arp_inspection_msg_req_t, req));

    return;
}

/* ARP_INSPECTION msg rx */
static BOOL arp_inspection_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    arp_inspection_msg_id_t     msg_id = *(arp_inspection_msg_id_t *)rx_msg;
    arp_inspection_msg_req_t    *msg = (void *)rx_msg;
    port_info_t                 port_info;
    port_iter_t                 pit;
#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
    vtss_packet_port_info_t     info;
    vtss_packet_port_filter_t   filter[VTSS_PORT_ARRAY_SIZE];
#endif

    switch (msg_id) {
    case ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ: {
        T_D("SET msg_id: %d, %s, len: %zd, isid: %u", msg_id, arp_inspection_msg_id_txt(msg_id), len, isid);
        if (msg->req.conf_set.conf.mode == ARP_INSPECTION_MGMT_ENABLED) {
            arp_inspection_receive_register();
        } else {
            arp_inspection_receive_unregister();
        }
        break;
    }
    case ARP_INSPECTION_MSG_ID_FRAME_RX_IND: {
        T_D("RX msg_id: %d, %s, len: %zd, isid: %u", msg_id, arp_inspection_msg_id_txt(msg_id), len, isid);
        arp_inspection_bip_buffer_enqueue((u8 *)&msg[1], msg->req.rx_ind.len, msg->req.rx_ind.vid, isid, msg->req.rx_ind.port_no);
        break;
    }
    case ARP_INSPECTION_MSG_ID_FRAME_TX_REQ: {
        T_D("TX msg_id: %d, %s, len: %zd, isid: %u, msg_isid: %lu", msg_id, arp_inspection_msg_id_txt(msg_id), msg->req.tx_req.len, isid, msg->req.tx_req.isid );

#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
        // get port information by vid
        (void) vtss_packet_port_info_init(&info);
        info.vid = msg->req.tx_req.vid;
        (void) vtss_packet_port_filter_get(NULL, &info, filter);
#endif

        /* check which port needs to send the packet */
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (msg->req.tx_req.port_list[pit.iport]) {
                /* check the port is link or not */
                if (port_info_get(pit.iport, &port_info) != VTSS_OK ||
                    port_info.link == 0) {
                    continue;
                }

                /* discard un-wanted vlan packet */
                if (filter[pit.iport].filter == VTSS_PACKET_FILTER_DISCARD) {
                    continue;
                }

#if ARP_INSPECTION_PROCESS_VLAN_TAGGING_ISSUE
                T_D("Slave TX, port %u, VID %lu, filter %d, tpid %x", pit.iport, msg->req.tx_req.vid, filter[pit.iport].filter, filter[pit.iport].tpid);

                switch (filter[pit.iport].filter) {
                case VTSS_PACKET_FILTER_TAGGED: {
                    void *frame = packet_tx_alloc(msg->req.tx_req.len + 4);
                    if (frame) {
                        packet_tx_props_t   tx_props;
                        void                *tr_msg = &msg[1];
                        uchar               *frame_ptr = tr_msg;
                        uchar               *buffer_ptr = frame;
                        uchar               vlan_tag[4];

                        /* Fill out VLAN tag */
                        memset(vlan_tag, 0x0, sizeof(vlan_tag));
                        pack16(filter[pit.iport].tpid, vlan_tag);
                        pack16(msg->req.tx_req.vid, vlan_tag + 2);

                        /* process VLAN tagging issue */
                        memcpy(buffer_ptr, frame_ptr, 12); // DMAC & SMAC
                        memcpy(buffer_ptr + 12, vlan_tag, 4); // VLAN Header
                        memcpy(buffer_ptr + 12 + 4, frame_ptr + 12, msg->req.tx_req.len - 12); // Remainder of frame

                        //memcpy(frame, &msg[1], msg->req.tx_req.len);
                        packet_tx_props_init(&tx_props);
                        tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                        tx_props.packet_info.frm[0]    = frame;
                        tx_props.packet_info.len[0]    = msg->req.tx_req.len + 4;
                        tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                        if (packet_tx(&tx_props) != VTSS_RC_OK) {
                            T_W("packet_tx() failed");
                        }
                    } else {
                        T_W("allocation failure, length %zd", msg->req.tx_req.len + 4);
                    }
                }
                break;
                case VTSS_PACKET_FILTER_UNTAGGED: {
                    void *frame = packet_tx_alloc(msg->req.tx_req.len);
                    if (frame) {
                        packet_tx_props_t tx_props;
                        memcpy(frame, &msg[1], msg->req.tx_req.len);
                        packet_tx_props_init(&tx_props);
                        tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                        tx_props.packet_info.frm[0]    = frame;
                        tx_props.packet_info.len[0]    = msg->req.tx_req.len;
                        tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                        if (packet_tx(&tx_props) != VTSS_RC_OK) {
                            T_W("packet_tx() failed");
                        }
                    } else {
                        T_W("allocation failure, length %zd", msg->req.tx_req.len);
                    }
                }
                break;
                case VTSS_PACKET_FILTER_DISCARD:
                    T_E("VTSS_PACKET_FILTER_DISCARD");
                    break;
                default:
                    T_E("unknown ID: %d", filter[pit.iport].filter);
                    break;
                }
#else
                T_D("Slave TX, port %lu, VID %lu", pit.iport, msg->req.tx_req.vid);

                void *frame = packet_tx_alloc(msg->req.tx_req.len);
                if (frame) {
                    packet_tx_props_t tx_props;
                    memcpy(frame, &msg[1], msg->req.tx_req.len);
                    packet_tx_props_init(&tx_props);
                    tx_props.packet_info.modid     = VTSS_MODULE_ID_ARP_INSPECTION;
                    tx_props.packet_info.frm[0]    = frame;
                    tx_props.packet_info.len[0]    = msg->req.tx_req.len;
                    tx_props.tx_info.dst_port_mask = VTSS_BIT64(pit.iport);
                    if (packet_tx(&tx_props) != VTSS_RC_OK) {
                        T_W("packet_tx() failed");
                    }
                } else {
                    T_W("allocation failure, length %zd", msg->req.tx_req.len);
                }
#endif
            }
        }

        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}

/* Allocate request buffer */
static arp_inspection_msg_req_t *arp_inspection_msg_req_alloc(arp_inspection_msg_buf_t *buf, arp_inspection_msg_id_t msg_id)
{
    arp_inspection_msg_req_t *msg = &arp_inspection_global.request.msg;

    buf->sem = &arp_inspection_global.request.sem;
    buf->msg = msg;
    (void) VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;

    return msg;
}

/****************************************************************************/
/*  Stack Register Function                                                 */
/****************************************************************************/

/* ARP_INSPECTION stack register */
static vtss_rc arp_inspection_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0x0, sizeof(filter));
    filter.cb = arp_inspection_msg_rx;
    filter.modid = VTSS_MODULE_ID_ARP_INSPECTION;

    return msg_rx_filter_register(&filter);
}

/* Set stack ARP_INSPECTION configuration */
static void arp_inspection_stack_arp_inspection_conf_set(vtss_isid_t isid_add)
{
    arp_inspection_msg_req_t    *msg;
    arp_inspection_msg_buf_t    buf;
    switch_iter_t               sit;

    T_D("enter, isid_add: %d", isid_add);

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        ARP_INSPECTION_CRIT_ENTER();
        msg = arp_inspection_msg_req_alloc(&buf, ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ);

        /* copy all configurations to stacking msg */
        //msg->req.conf_set.conf = arp_inspection_global.arp_inspection_conf;
        msg->req.conf_set.conf.mode = arp_inspection_global.arp_inspection_conf.mode;
        memcpy(msg->req.conf_set.conf.port_mode_conf, arp_inspection_global.arp_inspection_conf.port_mode_conf, VTSS_ISID_CNT * sizeof(arp_inspection_port_mode_conf_t));

        ARP_INSPECTION_CRIT_EXIT();

        arp_inspection_msg_tx(&buf, sit.isid, sizeof(msg->req.conf_set.conf));
    }

    T_D("exit, isid_add: %d", isid_add);
    return;
}

/****************************************************************************/
/*  Configuration Function                                                  */
/****************************************************************************/

/* Set ARP_INSPECTION configuration */
vtss_rc arp_inspection_mgmt_conf_mode_set(u32 *mode)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = FALSE;

    T_D("enter, mode: %d", *mode);

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check illegal parameter */
    if (*mode != ARP_INSPECTION_MGMT_ENABLED && *mode != ARP_INSPECTION_MGMT_DISABLED) {
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    ARP_INSPECTION_CRIT_ENTER();
    if (arp_inspection_global.arp_inspection_conf.mode != *mode) {
        arp_inspection_global.arp_inspection_conf.mode = *mode;
        changed = TRUE;
    }
    ARP_INSPECTION_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t             blk_id  = CONF_BLK_ARP_INSPECTION_CONF;
        arp_inspection_conf_blk_t *arp_inspection_conf_blk_p;
        if ((arp_inspection_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open ARP_INSPECTION table");
        } else {
            arp_inspection_conf_blk_p->arp_inspection_conf.mode = *mode;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        /* Activate changed configuration */
        arp_inspection_stack_arp_inspection_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
    return rc;
}

/* Get ARP_INSPECTION configuration */
vtss_rc arp_inspection_mgmt_conf_mode_get(u32 *mode)
{
    T_D("enter");

    ARP_INSPECTION_CRIT_ENTER();
    *mode = arp_inspection_global.arp_inspection_conf.mode;
    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set ARP_INSPECTION configuration for port mode */
vtss_rc arp_inspection_mgmt_conf_port_mode_set(vtss_isid_t isid, arp_inspection_port_mode_conf_t *port_mode_conf)
{
    vtss_rc     rc = VTSS_OK;
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    int         changed = FALSE;
#endif
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];
    port_iter_t pit;


    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
    if (port_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (port_mode_conf->mode[pit.iport] != ARP_INSPECTION_MGMT_ENABLED &&
            port_mode_conf->mode[pit.iport] != ARP_INSPECTION_MGMT_DISABLED) {
            return ARP_INSPECTION_ERROR_INV_PARAM;
        }
    }

    memset(ports, 0x0, sizeof(ports));

    // find which port needs to flush
    ARP_INSPECTION_CRIT_ENTER();
    if (memcmp(&arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START], port_mode_conf, sizeof(arp_inspection_port_mode_conf_t))) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        changed = TRUE;
#endif

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport]  != port_mode_conf->mode[pit.iport]) {
                if (port_mode_conf->mode[pit.iport] == ARP_INSPECTION_MGMT_DISABLED) {
                    ports[pit.iport] = TRUE;
                }
                //arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].mode[i] = port_mode_conf->mode[i];
            }
        }
    }
    ARP_INSPECTION_CRIT_EXIT();

    /* clear all dynamic entries with port disabled */
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (ports[pit.iport]) {
            if (arp_inspection_mgmt_conf_flush_dynamic_entry_by_port(isid, pit.iport)) {
                T_W("arp_inspection_mgmt_conf_flush_dynamic_entry_by_port() failed");
            }
        }
    }

    ARP_INSPECTION_CRIT_ENTER();
    if (memcmp(&arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START], port_mode_conf, sizeof(arp_inspection_port_mode_conf_t))) {

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            // set port mode
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport] != port_mode_conf->mode[pit.iport]) {
                arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].mode[pit.iport] = port_mode_conf->mode[pit.iport];
            }
            // set VLAN mode
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].check_VLAN[pit.iport] != port_mode_conf->check_VLAN[pit.iport]) {
                arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].check_VLAN[pit.iport] = port_mode_conf->check_VLAN[pit.iport];
            }
            // set port log type
            if (arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].log_type[pit.iport] != port_mode_conf->log_type[pit.iport]) {
                arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START].log_type[pit.iport] = port_mode_conf->log_type[pit.iport];
            }
        }
    }
    ARP_INSPECTION_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (changed) {
        /* Save changed configuration */
        conf_blk_id_t             blk_id  = CONF_BLK_ARP_INSPECTION_CONF;
        arp_inspection_conf_blk_t *arp_inspection_conf_blk_p;
        if ((arp_inspection_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open ARP_INSPECTION table");
        } else {
            ARP_INSPECTION_CRIT_ENTER();
            memcpy(arp_inspection_conf_blk_p->arp_inspection_conf.port_mode_conf, arp_inspection_global.arp_inspection_conf.port_mode_conf, VTSS_ISID_CNT * sizeof(arp_inspection_port_mode_conf_t));
            ARP_INSPECTION_CRIT_EXIT();
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
    return rc;
}

/* Get ARP_INSPECTION configuration for port mode */
vtss_rc arp_inspection_mgmt_conf_port_mode_get(vtss_isid_t isid, arp_inspection_port_mode_conf_t *port_mode_conf)
{
    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check switch ID */
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_ISID_NON_EXISTING;
    }
    /* Check illegal parameter */
    if (port_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    ARP_INSPECTION_CRIT_ENTER();
    memcpy(port_mode_conf, &arp_inspection_global.arp_inspection_conf.port_mode_conf[isid - VTSS_ISID_START], sizeof(arp_inspection_port_mode_conf_t));
    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_set(vtss_vid_t vid, arp_inspection_vlan_mode_conf_t *vlan_mode_conf)
{
    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check vid */
    if (vid >= VTSS_VIDS) {
        T_W("illegal vid: %u", vid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }
    /* Check illegal parameter */
    if (vlan_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    ARP_INSPECTION_CRIT_ENTER();
    memcpy(&arp_inspection_global.arp_inspection_conf.vlan_mode_conf[vid], vlan_mode_conf, sizeof(arp_inspection_vlan_mode_conf_t));
    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Get ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_get(vtss_vid_t vid, arp_inspection_vlan_mode_conf_t *vlan_mode_conf, BOOL next)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter");

    /* Check stack role */
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ARP_INSPECTION_ERROR_MUST_BE_MASTER;
    }
    /* Check vid */
    if (vid >= VTSS_VIDS) {
        T_W("illegal vid: %u", vid);
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }
    /* Check illegal parameter */
    if (vlan_mode_conf == NULL) {
        T_W("illegal parameter");
        T_D("exit");
        return ARP_INSPECTION_ERROR_INV_PARAM;
    }

    ARP_INSPECTION_CRIT_ENTER();
    if (next) {
        vid++;
        if (vid >= VTSS_VIDS) {
            rc = ARP_INSPECTION_ERROR_DATABASE_NOT_FOUND;
        } else {
            memcpy(vlan_mode_conf, &arp_inspection_global.arp_inspection_conf.vlan_mode_conf[vid], sizeof(arp_inspection_vlan_mode_conf_t));
        }
    } else {
        memcpy(vlan_mode_conf, &arp_inspection_global.arp_inspection_conf.vlan_mode_conf[vid], sizeof(arp_inspection_vlan_mode_conf_t));
    }
    ARP_INSPECTION_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Save ARP_INSPECTION configuration for VLAN mode */
vtss_rc arp_inspection_mgmt_conf_vlan_mode_save(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t                   blk_id  = CONF_BLK_ARP_INSPECTION_CONF;
    arp_inspection_conf_blk_t       *arp_inspection_conf_blk_p;

    /* Save changed configuration */
    if ((arp_inspection_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open ARP_INSPECTION table");
    } else {
        ARP_INSPECTION_CRIT_ENTER();
        memcpy(arp_inspection_conf_blk_p->arp_inspection_conf.vlan_mode_conf, arp_inspection_global.arp_inspection_conf.vlan_mode_conf, VTSS_VIDS * sizeof(arp_inspection_vlan_mode_conf_t));
        ARP_INSPECTION_CRIT_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return VTSS_OK;
}

/* Determine if ARP_INSPECTION configuration has changed */
static int arp_inspection_conf_changed(arp_inspection_conf_t *old, arp_inspection_conf_t *new)
{
    return (memcmp(old, new, sizeof(*new)));
}

/* Read/create ARP_INSPECTION stack configuration */
static void arp_inspection_conf_read_stack(BOOL create)
{
    int                             changed;
    ulong                           size;
    static arp_inspection_conf_t    new_arp_inspection_conf;
    arp_inspection_conf_t           *old_arp_inspection_conf_p;
    arp_inspection_conf_blk_t       *conf_blk_p;
    conf_blk_id_t                   blk_id;
    ulong                           blk_version;
    BOOL                            do_create = FALSE;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_ARP_INSPECTION_CONF;
    blk_version = ARP_INSPECTION_CONF_BLK_VERSION2;

    if (misc_conf_read_use()) {
        /* Read/create ARP_INSPECTION configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
            T_W("conf_sec_open failed, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version < blk_version) {
            arp_inspection_conf_blk_t *new_blk;

            T_I("version upgrade, run silent upgrade");
            new_blk = arp_inspection_conf_flash_silent_upgrade(conf_blk_p, conf_blk_p->version);
            if (size != sizeof(*conf_blk_p)) {
                conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            }
            if (new_blk && conf_blk_p) {
                T_I("upgrade ok");
                *conf_blk_p = *new_blk;
                VTSS_FREE(new_blk);
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
        } else if (size != sizeof(*conf_blk_p)) {
            T_W("size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = FALSE;

    ARP_INSPECTION_CRIT_ENTER();

    if (do_create) {
        /* Use default values */
        arp_inspection_default_set(&new_arp_inspection_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->arp_inspection_conf = new_arp_inspection_conf;
        }
        /* Delete all static entries */
        if (_arp_inspection_mgmt_conf_del_all_static_entry(TRUE) != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_del_all_static_entry() failed");
        }

    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {
            new_arp_inspection_conf = conf_blk_p->arp_inspection_conf;
        }
    }
    old_arp_inspection_conf_p = &arp_inspection_global.arp_inspection_conf;
    if (arp_inspection_conf_changed(old_arp_inspection_conf_p, &new_arp_inspection_conf)) {
        changed = TRUE;
    }
    arp_inspection_global.arp_inspection_conf = new_arp_inspection_conf;

    /* Delete all dynamic entries */
    if (_arp_inspection_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
        T_W("_arp_inspection_mgmt_conf_del_all_dynamic_entry() failed");
    }

    ARP_INSPECTION_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open ARP_INSPECTION table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        /* Apply all configuration to switch */
        arp_inspection_stack_arp_inspection_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
    return;
}

static void arp_inspection_bip_buffer_thread(cyg_addrword_t data)
{
    cyg_flag_value_t events;

    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {
                events = cyg_flag_wait(&arp_inspection_bip_buffer_thread_events,
                                       ARP_INSPECTION_EVENT_ANY,
                                       CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
                if (events & ARP_INSPECTION_EVENT_PKT_RECV) {
                    arp_inspection_bip_buffer_dequeue();
                }
            } //while(msg_switch_is_master())
        } //if(msg_switch_is_master())

        //No reason for using CPU ressources when we're a slave
        T_D("Suspending DHCP helper bip buffer thread");
        ARP_INSPECTION_BIP_CRIT_ENTER();
        vtss_bip_buffer_clear(&arp_inspection_bip_buf);
        ARP_INSPECTION_BIP_CRIT_EXIT();
        cyg_thread_suspend(arp_inspection_bip_buffer_thread_handle);
        T_D("Resumed DHCP helper bip buffer thread");
    } //while(1)
}

/****************************************************************************/
/*  Start functions                                                         */
/****************************************************************************/

/* Module start */
static void arp_inspection_start(BOOL init)
{
    arp_inspection_conf_t *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize ARP_INSPECTION configuration */
        conf_p = &arp_inspection_global.arp_inspection_conf;
        arp_inspection_default_set(conf_p);
        arp_inspection_default_set_dynamic_entry();

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&arp_inspection_global.request.sem, 1);

        /* Initialize BIP buffer */
        if (!vtss_bip_buffer_init(&arp_inspection_bip_buf, ARP_INSPECTION_BIP_BUF_TOTAL_SIZE)) {
            T_E("vtss_bip_buffer_init failed!");
        }

        /* Create semaphore for critical regions */
        critd_init(&arp_inspection_global.crit, "arp_inspection_global.crit", VTSS_MODULE_ID_ARP_INSPECTION, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        ARP_INSPECTION_CRIT_EXIT();
        critd_init(&arp_inspection_global.bip_crit, "dhcp_helper_global.bip_crit", VTSS_MODULE_ID_ARP_INSPECTION, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        ARP_INSPECTION_BIP_CRIT_EXIT();

        /* Create ARP Inspection bip buffer thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          arp_inspection_bip_buffer_thread,
                          0,
                          "ARP inspection bip buffer",
                          arp_inspection_bip_buffer_thread_stack,
                          sizeof(arp_inspection_bip_buffer_thread_stack),
                          &arp_inspection_bip_buffer_thread_handle,
                          &arp_inspection_bip_buffer_thread_block);

    } else {
        /* Register for stack messages */
        if (arp_inspection_stack_register()) {
            T_W("arp_inspection_stack_register() failed");
        }
        if (port_global_change_register(VTSS_MODULE_ID_ARP_INSPECTION, arp_inspection_state_change_callback)) {
            T_W("port_global_change_register() failed");
        }
    }

    T_D("exit");
    return;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Initialize module */
vtss_rc arp_inspection_init(vtss_init_data_t *data)
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
        arp_inspection_start(TRUE);
#ifdef VTSS_SW_OPTION_VCLI
        arp_insp_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = arp_inspection_icfg_init()) != VTSS_OK) {
            T_D("Calling arp_inspection_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        arp_inspection_start(FALSE);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            arp_inspection_conf_read_stack(TRUE);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");
        ARP_INSPECTION_CRIT_ENTER();

        /* create data base for storing static arp entry */
#ifdef VTSS_LIB_DATA_STRUCT
        if (_arp_inspection_mgmt_conf_create_static_db(ARP_INSPECTION_MAX_ENTRY_CNT, _arp_inspection_entry_compare_func) != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_create_static_db() failed");
        }
#else
        if (_arp_inspection_mgmt_conf_create_static_db() != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_create_static_db() failed");
        }
#endif

        /* create data base for storing dynamic arp entry */
#ifdef VTSS_LIB_DATA_STRUCT
        if (_arp_inspection_mgmt_conf_create_dynamic_db(ARP_INSPECTION_MAX_ENTRY_CNT, _arp_inspection_entry_compare_func) != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_create_dynamic_db() failed");
        }
#else
        if (_arp_inspection_mgmt_conf_create_dynamic_db() != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_create_dynamic_db() failed");
        }
#endif

        ARP_INSPECTION_CRIT_EXIT();

        /* Read stack and switch configuration */
        arp_inspection_conf_read_stack(FALSE);

        ARP_INSPECTION_CRIT_ENTER();
        /* sync static arp entries to the efficient data base */
        for (i = 0; i < ARP_INSPECTION_MAX_ENTRY_CNT; i++) {
            if (arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i].valid == TRUE) {
                if (_arp_inspection_mgmt_conf_add_static_entry(&(arp_inspection_global.arp_inspection_conf.arp_inspection_static_entry[i]), FALSE) != VTSS_OK) {
                    T_W("_arp_inspection_mgmt_conf_add_static_entry() failed");
                }
            }
        }
        ARP_INSPECTION_CRIT_EXIT();

        /* Starting ARP Inspection BIP buffer thread (became master) */
        cyg_thread_resume(arp_inspection_bip_buffer_thread_handle);
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        /* clean data base for storing static arp entry */
        ARP_INSPECTION_CRIT_ENTER();
        /* Delete all static entries */
        if (_arp_inspection_mgmt_conf_del_all_static_entry(TRUE) != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_del_all_static_entry() failed");
        }
        /* Delete all dynamic entries */
        if (_arp_inspection_mgmt_conf_del_all_dynamic_entry(TRUE) != VTSS_OK) {
            T_W("_arp_inspection_mgmt_conf_del_all_dynamic_entry() failed");
        }
        ARP_INSPECTION_CRIT_EXIT();
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        if (msg_switch_is_master()) {
            if (msg_switch_is_local(isid)) {
                master_isid = isid;
            }
        }
        /* Apply all configuration to switch */
        arp_inspection_stack_arp_inspection_conf_set(isid);

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
