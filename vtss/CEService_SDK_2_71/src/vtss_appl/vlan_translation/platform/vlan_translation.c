/*
   Vitesse VLAN Translation software.

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

#include <vlan_translation_api.h>
#include <conf_api.h>               /* definition of conf_blk_id_t */
#include <vtss_module_id.h>         /* For VCL module ID */
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>
#include <misc_api.h>
#ifdef VTSS_SW_OPTION_VCLI
#include <vlan_translation_cli.h>   /* definition of vlan_translation_cli_req_init */
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "vlan_translation_icli_functions.h" // For vlan_translation_icfg_init
#endif

#define     VTSS_TRACE_MODULE_ID                        VTSS_MODULE_ID_VLAN_TRANSLATION
#define     VTSS_TRACE_GRP_DEFAULT                      0
#define     TRACE_GRP_CRIT                              1
#define     TRACE_GRP_CNT                               2
/*lint -sem( vtss_vlan_trans_crit_data_lock, thread_lock ) */
/*lint -sem( vtss_vlan_trans_crit_data_unlock, thread_unlock ) */
#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "vlan_trans",
    .descr     = "VLAT Translation table"
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
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};
/* Global module/API Lock for Critical VT data protection */
critd_t                         vlan_trans_data_lock;
#define VT_CRIT_ENTER()         critd_enter(&vlan_trans_data_lock, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VT_CRIT_EXIT()          critd_exit(&vlan_trans_data_lock, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VT_CRIT_ASSERT_LOCKED() critd_assert_locked(&vlan_trans_data_lock, TRACE_GRP_CRIT,  __FILE__, __LINE__)
#else  /* VTSS_TRACE_ENABLED */
#define VT_CRIT_ENTER()         critd_enter(&vlan_trans_data_lock)
#define VT_CRIT_EXIT()          critd_exit(&vlan_trans_data_lock, 0)
#define VT_CRIT_ASSERT_LOCKED() critd_assert_locked(&vlan_trans_data_lock)
#endif /* VTSS_TRACE_ENABLED */

static vtss_rc rc_conv(int vt_rc)
{
    vtss_rc rc = VTSS_RC_ERROR;

    switch (vt_rc) {
    case VTSS_VT_RC_OK:
        rc = VTSS_RC_OK;
        break;
    case VTSS_VT_ERROR_PARM:
        rc = VT_ERROR_PARM;
        break;
    case VTSS_VT_ERROR_CONFIG_NOT_OPEN:
        rc = VT_ERROR_CONFIG_NOT_OPEN;
        break;
    case VTSS_VT_ERROR_ENTRY_NOT_FOUND:
        rc = VT_ERROR_ENTRY_NOT_FOUND;
        break;
    case VTSS_VT_ERROR_TABLE_EMPTY:
        rc = VT_ERROR_TABLE_EMPTY;
        break;
    case VTSS_VT_ERROR_TABLE_FULL:
        rc = VT_ERROR_TABLE_FULL;
        break;
    case VTSS_VT_ERROR_REG_TABLE_FULL:
        rc = VT_ERROR_REG_TABLE_FULL;
        break;
    case VTSS_VT_ERROR_USER_PREVIOUSLY_CONFIGURED:
        rc = VT_ERROR_USER_PREVIOUSLY_CONFIGURED;
        break;
    case VTSS_VT_ERROR_ENTRY_PREVIOUSLY_CONFIGURED:
        rc = VT_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
        break;
    default:
        T_E("Invalid VT module error is noticed:- %d\n", vt_rc);
        break;
    }
    return rc;
}
char *vlan_trans_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case VT_ERROR_GEN:
        txt = "VLAN Translation generic error";
        break;
    case VT_ERROR_PARM:
        txt = "VLAN Translation parameter error";
        break;
    case VT_ERROR_CONFIG_NOT_OPEN:
        txt = "VLAN Translation configuration error";
        break;
    case VT_ERROR_ENTRY_NOT_FOUND:
        txt = "Entry not found";
        break;
    case VT_ERROR_TABLE_FULL:
        txt = "VLAN Translation table full";
        break;
    case VT_ERROR_TABLE_EMPTY:
        txt = "VLAN Translation table empty";
        break;
    case VT_ERROR_USER_PREVIOUSLY_CONFIGURED:
        txt = "VLAN Translation Error Previously Configured";
        break;
    case VT_ERROR_VLAN_SAME_AS_TRANSLATE_VLAN:
        txt = "VLAN ID and Translated VLAN ID cannot be same";
        break;
    default:
        txt = "VLAN Translation unknown error";
        break;
    }
    return txt;
}

static void vlan_trans_vlan_conf_read(BOOL create)
{
    conf_blk_id_t               blk_id;
    vlan_trans_vlan_table_blk_t *vt_blk;
    u32                         i;
    BOOL                        do_create;
    ulong                       size;
    vtss_rc                     rc = VTSS_OK;
    vlan_trans_grp2vlan_conf_t  *vlan_entry;
    vlan_trans_grp2vlan_conf_t  entry;
    BOOL                        next = FALSE;
    T_D("%s: enter, create: %d\n", __FUNCTION__, create);
    /* Read/create VLAN Translation VLAN table configuration */
    blk_id = CONF_BLK_VLAN_TRANS_VLAN_TABLE;

    if (misc_conf_read_use()) {
        if (((vt_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vt_blk))) {
            T_W("%s: conf_sec_open failed or size mismatch, creating defaults\n", __FUNCTION__);
            vt_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vt_blk));
            do_create = 1;
        } else if (vt_blk->version != VT_VLAN_BLK_VERSION) {
            T_W("%s: version mismatch, creating defaults\n", __FUNCTION__);
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        vt_blk    = NULL;
        do_create = 1;
    }

    entry.group_id = VT_NULL_GROUP_ID;
    while ((rc_conv(vlan_trans_grp2vlan_entry_get(&entry, next))) == VTSS_RC_OK) {
        next = TRUE;
        /* Call the switch API */
        if ((rc = vtss_vlan_trans_group_del(NULL, entry.group_id, entry.vid)) != VTSS_RC_OK) {
            T_D("API function failed\n");
        }
    }
    vlan_trans_vlan_list_default_set();

    if (do_create) {
        if (vt_blk != NULL) {
            vt_blk->version = VT_VLAN_BLK_VERSION;
            vt_blk->count = 0;
        }
    } else {
        if (vt_blk == NULL) {
            T_W("failed to open VLAN Translation VLAN table");
        } else {
            /* Add new VLAN Translations */
            for (i = 0; i < vt_blk->count; i++) {
                vlan_entry = &vt_blk->table[i];
                if ((rc = rc_conv(vlan_trans_grp2vlan_entry_add(vlan_entry->group_id, vlan_entry->vid,
                                                                vlan_entry->trans_vid))) == VTSS_RC_OK) {
                    /* Call the switch API */
                    if ((rc = vtss_vlan_trans_group_add(NULL, vlan_entry->group_id, vlan_entry->vid,
                                                        vlan_entry->trans_vid)) != VTSS_RC_OK) {
                        T_E("%s: API function failed\n", __FUNCTION__);
                    }
                } else {
                    T_D("%s: vlan_trans_grp2vlan_entry_add Failed, rc = %d\n", __FUNCTION__, rc);
                } /* if ((rc = */
            } /* for (i = 0; */
        } /* if (vt_blk == */
    } /* if (do_create */

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
}

// Function for initializing an entry with it's default configuration
// In - group_id - The id number for the entry to set to default.
// Out - entry - Pointer to the entry to initialize.
void entry_default_set(vlan_trans_port2grp_conf_t *entry, u8 group_id)
{
    memset(entry, 0, sizeof(vlan_trans_port2grp_conf_t));
    entry->group_id = group_id + 1;
    /* Set the corresponding bit for this port */
    entry->ports[group_id / 8] = (1 << (group_id % 8));
}

static void vlan_trans_port_conf_read(BOOL create)
{
    vtss_rc                             rc = VTSS_OK;
    vtss_vlan_trans_port2grp_conf_t     conf;
    u32                                 i;

    T_D("%s: enter, create: %d\n", __FUNCTION__, create);

    /* Read/create VLAN Translation Port table configuration */

    if (misc_conf_read_use()) {
        conf_blk_id_t                       blk_id;
        vlan_trans_port_table_blk_t         *vt_blk;
        BOOL                                do_create;
        ulong                               size;
        vlan_trans_port2grp_conf_t          *port_entry;

        blk_id = CONF_BLK_VLAN_TRANS_PORT_TABLE;
        if (((vt_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vt_blk))) {
            T_W("%s: conf_sec_open failed or size mismatch, creating defaults\n", __FUNCTION__);
            vt_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vt_blk));
            do_create = 1;
        } else if (vt_blk->version != VT_PORT_BLK_VERSION) {
            T_W("%s: version mismatch, creating defaults\n", __FUNCTION__);
            do_create = 1;
        } else {
            do_create = create;
        }
        vlan_trans_port_list_default_set();
        if (do_create) {
            if (vt_blk != NULL) {
                vt_blk->version = VT_PORT_BLK_VERSION;
                vt_blk->count = VT_MAX_GROUP_CNT;
                /* Set all the default entries */
                for (i = 0; i < VT_MAX_GROUP_CNT; i++) {
                    entry_default_set(&vt_blk->table[i], i);
                }
            }
        }
        if (vt_blk == NULL) {
            T_W("failed to open VLAN Translation Port table");
        } else {
            /* Add new Ports to Group mappings */
            for (i = 0; i < vt_blk->count; i++) {
                port_entry = &vt_blk->table[i];
                if ((rc = rc_conv(vlan_trans_port2grp_entry_add(port_entry->group_id, port_entry->ports)))
                    == VTSS_RC_OK) {
                    conf.group_id = port_entry->group_id;
                    memcpy(conf.ports, port_entry->ports, VTSS_PORT_BF_SIZE);
                    if ((rc = vtss_vlan_trans_group_to_port_set(NULL, &conf)) != VTSS_RC_OK) {
                        T_E("%s: Switch API call failed\n", __FUNCTION__);
                    }
                } else {
                    T_D("%s: vlan_trans_port2grp_entry_add Failed, rc = %d\n", __FUNCTION__, rc);
                } /* if ((rc = */
            } /* for (i = 0; */
        } /* if (vt_blk == */
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        // ICFG, not conf: Load defaults
        vlan_trans_port2grp_conf_t tmp_port_entry;

        vlan_trans_port_list_default_set();
        for (i = 0; i < VT_MAX_GROUP_CNT; i++) {
            entry_default_set(&tmp_port_entry, i);
            if ((rc = rc_conv(vlan_trans_port2grp_entry_add(tmp_port_entry.group_id, tmp_port_entry.ports))) == VTSS_RC_OK) {
                conf.group_id = tmp_port_entry.group_id;
                memcpy(conf.ports, tmp_port_entry.ports, VTSS_PORT_BF_SIZE);
                if ((rc = vtss_vlan_trans_group_to_port_set(NULL, &conf)) != VTSS_RC_OK) {
                    T_E("%s: Switch API call failed\n", __FUNCTION__);
                }
            } else {
                T_D("%s: vlan_trans_port2grp_entry_add Failed, rc = %d\n", __FUNCTION__, rc);
            } /* if ((rc = */
        } /* for (i = 0; */
    }
}
vtss_rc vlan_trans_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    vtss_rc  rc = VTSS_RC_OK;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* PPP1004 : Initialize trace, local data structures; Create and initialize
           OS objects(threads, mutexes, event flags etc). Resume threads if they should
           be running. This command is executed before scheduler is started, so don't
           perform any blocking operation such as critd_enter() */
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        T_D("vlan_trans_init : INIT_CMD_INIT\n");
        /* Initializing the local data structures */
        vlan_trans_default_set();
        critd_init(&vlan_trans_data_lock, "vlan_trans_data_lock", VTSS_MODULE_ID_VLAN_TRANSLATION,
                   VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        vtss_vlan_trans_crit_data_unlock();
#ifdef VTSS_SW_OPTION_VCLI
        (void) vlan_trans_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        // Initialize ICLI "show running" configuration
        VTSS_RC(vlan_translation_icfg_init());
#endif
        break;
    case INIT_CMD_START:
        T_D("vlan_trans_init : INIT_CMD_START\n");
        /* PPP1004 : Initialize the things that might perform blocking opearations as
           scheduler has been started. Also, register callbacks from other modules */
        break;
    case INIT_CMD_CONF_DEF:
        /* As stacking is not supported by VLAN Translation module, it is ok to handle one of the
           VTSS_ISID_LOCAL, VTSS_ISID_GLOBAL and VTSS_ISID_LEGAL */
        T_D("vlan_trans_init : INIT_CMD_CONF_DEF\n");
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            vlan_trans_vlan_conf_read(TRUE);
            vlan_trans_port_conf_read(TRUE);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("vlan_trans_init : INIT_CMD_MASTER_UP\n");
        /* Read switch configuration */
        vlan_trans_vlan_conf_read(FALSE);
        vlan_trans_port_conf_read(FALSE);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("vlan_trans_init : INIT_CMD_MASTER_DOWN\n");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("vlan_trans_init : INIT_CMD_SWITCH_ADD\n");
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("vlan_trans_init : INIT_CMD_SWITCH_DEL\n");
        break;
    default:
        break;
    }
    return rc;
}
vtss_rc vlan_trans_grp2vlan_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vlan_trans_grp2vlan_conf_t  entry;
    BOOL                        next = FALSE;
    conf_blk_id_t               blk_id;
    vlan_trans_vlan_table_blk_t *vt_blk;
    ulong                       size;
    T_N("%s: Enter\n", __FUNCTION__);
    blk_id = CONF_BLK_VLAN_TRANS_VLAN_TABLE;
    if ((vt_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
        T_D("%s: conf_sec_open Failed\n", __FUNCTION__);
        return VT_ERROR_CONFIG_NOT_OPEN;
    }
    vt_blk->count = 0;
    entry.group_id = VT_NULL_GROUP_ID;
    while ((rc_conv(vlan_trans_grp2vlan_entry_get(&entry, next))) == VTSS_RC_OK) {
        next = TRUE;
        vt_blk->table[vt_blk->count] = entry;
        vt_blk->count++;
    }
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    T_N("%s: Exit\n", __FUNCTION__);
#endif
    return VTSS_RC_OK;
}
vtss_rc vlan_trans_port2grp_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vlan_trans_port2grp_conf_t  entry;
    BOOL                        next = FALSE;
    conf_blk_id_t               blk_id;
    vlan_trans_port_table_blk_t *vt_blk;
    ulong                       size;
    T_N("%s: Enter\n", __FUNCTION__);
    blk_id = CONF_BLK_VLAN_TRANS_PORT_TABLE;
    if ((vt_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
        T_D("%s: conf_sec_open Failed\n", __FUNCTION__);
        return VT_ERROR_CONFIG_NOT_OPEN;
    }
    vt_blk->count = 0;
    entry.group_id = VT_NULL_GROUP_ID;
    while ((rc_conv(vlan_trans_port2grp_entry_get(&entry, next))) == VTSS_RC_OK) {
        next = TRUE;
        vt_blk->table[vt_blk->count] = entry;
        vt_blk->count++;
    }
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    T_N("%s: Exit\n", __FUNCTION__);
#endif
    return VTSS_RC_OK;
}

/* Management functions */
vtss_rc vlan_trans_mgmt_grp2vlan_entry_add(const u16 group_id, const vtss_vid_t vid, const vtss_vid_t trans_vid)
{
    vtss_rc                     rc = VTSS_RC_OK;
    T_D("%s: enter\n", __FUNCTION__);
    do {
        /* Check for valid group_id */
        if (VT_VALID_GROUP_CHECK(group_id) == FALSE) {
            rc = VT_ERROR_PARM;
            break;
        }
        /* Check for valid vid */
        if (VT_VALID_VLAN_CHECK(vid) == FALSE) {
            rc = VT_ERROR_PARM;
            break;
        }
        /* Check for valid trans_vid */
        if (VT_VALID_VLAN_CHECK(trans_vid) == FALSE) {
            rc = VT_ERROR_PARM;
            break;
        }
        if ((rc = rc_conv(vlan_trans_grp2vlan_entry_add(group_id, vid, trans_vid))) != VTSS_RC_OK) {
            T_D("%s: vlan_trans_entry_add Failed; rc = %d\n", __FUNCTION__, rc);
            break;
        }
        /* Commit the configuration to the flash */
        if ((rc = rc_conv(vlan_trans_grp2vlan_conf_commit())) != VTSS_RC_OK) {
            T_W("%s: Flash commit failed\n", __FUNCTION__);
            break;
        }
        /* Call the switch API */
        if ((rc = vtss_vlan_trans_group_add(NULL, group_id, vid, trans_vid)) != VTSS_RC_OK) {
            T_D("%s: API function failed\n", __FUNCTION__);
            /* TODO: Clean up the control structures */
            break;
        }
    } while (0);
    T_D("%s: exit rc %d\n", __FUNCTION__, rc);
    return rc;
}
vtss_rc vlan_trans_mgmt_grp2vlan_entry_delete(const u16 group_id, const vtss_vid_t vid)
{
    vtss_rc                     rc = VTSS_RC_OK;
    T_D("%s: enter\n", __FUNCTION__);
    do {
        /* Check for valid group_id */
        if (VT_VALID_GROUP_CHECK(group_id) == FALSE) {
            rc = VT_ERROR_PARM;
            break;
        }
        /* Check for valid vid */
        if (VT_VALID_VLAN_CHECK(vid) == FALSE) {
            rc = VT_ERROR_PARM;
            break;
        }
        if ((rc = rc_conv(vlan_trans_grp2vlan_entry_delete(group_id, vid))) != VTSS_RC_OK) {
            T_D("%s: vlan_trans_entry_delete Failed; rc = %d\n", __FUNCTION__, rc);
            break;
        }
        /* Commit the configuration to the flash */
        if ((rc = rc_conv(vlan_trans_grp2vlan_conf_commit())) != VTSS_RC_OK) {
            T_W("%s: Flash commit failed\n", __FUNCTION__);
            break;
        }
        /* Call the switch API */
        if ((rc = vtss_vlan_trans_group_del(NULL, group_id, vid)) != VTSS_RC_OK) {
            T_D("%s: API function failed\n", __FUNCTION__);
            /* TODO: Clean up the control structures */
            break;
        }
    } while (0);
    T_D("%s: exit rc %d\n", __FUNCTION__, rc);
    return rc;

}
/**
 * If conf->group_id is zero => get first group; If first is TRUE, get first VLAN Translation entry
 **/
vtss_rc vlan_trans_mgmt_grp2vlan_entry_get(vlan_trans_mgmt_grp2vlan_conf_t *const conf, BOOL next)
{
    vlan_trans_grp2vlan_conf_t  entry;
    vtss_rc                     rc = VTSS_RC_OK;
    T_D("%s: enter\n", __FUNCTION__);
    /* Check for valid group_id. If group_id is VT_NULL_GROUP_ID, get the first group */
    if ((VT_VALID_GROUP_CHECK(conf->group_id) == FALSE) && (conf->group_id != VT_NULL_GROUP_ID)) {
        T_D("%s: invalid group ID\n", __FUNCTION__);
        return VT_ERROR_PARM;
    }
    if (VT_NULL_CHECK(conf) == FALSE) {
        T_D("%s: conf-NULL pointer\n", __FUNCTION__);
        return VT_ERROR_PARM;
    }
    entry.group_id = conf->group_id;
    entry.vid = conf->vid;
    if ((rc = rc_conv(vlan_trans_grp2vlan_entry_get(&entry, next))) != VTSS_RC_OK) {
        T_D("%s: vlan_trans_entry_get Failed; rc = %d\n", __FUNCTION__, rc);
    } else {
        conf->group_id = entry.group_id;
        conf->vid = entry.vid;
        conf->trans_vid = entry.trans_vid;
    }
    T_D("%s: exit\n", __FUNCTION__);
    return rc;
}
vtss_rc vlan_trans_mgmt_port2grp_entry_add(const u16 group_id, BOOL *ports)
{
    vtss_rc                             rc = VTSS_RC_OK;
    u8                                  ports_bf[VTSS_PORT_BF_SIZE];
    vtss_vlan_trans_port2grp_conf_t     conf;
    T_D("%s: enter\n", __FUNCTION__);
    /* Check for valid group_id. */
    do {
        if (VT_VALID_GROUP_CHECK(group_id) == FALSE) {
            T_D("%s: invalid group ID\n", __FUNCTION__);
            rc = VT_ERROR_PARM;
            break;
        }
        /* Convert ports to port bitfield */
        vlan_trans_ports2_port_bitfield(ports, ports_bf);
        if ((rc = rc_conv(vlan_trans_port2grp_entry_add(group_id, ports_bf))) != VTSS_RC_OK) {
            T_D("%s: vlan_trans_port2grp_entry_add Failed; rc = %d\n", __FUNCTION__, rc);
            break;
        }
        /* Commit the configuration to the flash */
        if ((rc = rc_conv(vlan_trans_port2grp_conf_commit())) != VTSS_RC_OK) {
            T_D("%s: Flash commit failed\n", __FUNCTION__);
            break;
        }
        conf.group_id = group_id;
        memcpy(conf.ports, ports_bf, VTSS_PORT_BF_SIZE);
        if ((rc = vtss_vlan_trans_group_to_port_set(NULL, &conf)) != VTSS_RC_OK) {
            T_D("%s: Switch API call failed\n", __FUNCTION__);
            break;
        }
    } while (0);
    T_D("%s: exit\n", __FUNCTION__);
    return rc;
}
/* If conf->group_id == 0; return the first group */
vtss_rc vlan_trans_mgmt_port2grp_entry_get(vlan_trans_mgmt_port2grp_conf_t *const conf, BOOL next)
{
    vtss_rc                     rc = VTSS_RC_OK;
    vlan_trans_port2grp_conf_t  entry;
    T_D("%s: enter\n", __FUNCTION__);
    do {
        /* Check for valid group_id. If group_id is VT_NULL_GROUP_ID, get the first group */
        if ((VT_VALID_GROUP_CHECK(conf->group_id) == FALSE) && (conf->group_id != VT_NULL_GROUP_ID)) {
            T_D("%s: invalid group ID\n", __FUNCTION__);
            rc = VT_ERROR_PARM;
            break;
        }
        /* Check for NULL pointer */
        if (VT_NULL_CHECK(conf) == FALSE) {
            T_D("%s: conf-NULL pointer\n", __FUNCTION__);
            rc = VT_ERROR_PARM;
            break;
        }
        entry.group_id = conf->group_id;
        if ((rc = rc_conv(vlan_trans_port2grp_entry_get(&entry, next))) != VTSS_RC_OK) {
            T_D("%s: vlan_trans_mgmt_port2grp_entry_get Failed; rc = %d\n", __FUNCTION__, rc);
        } else {
            conf->group_id = entry.group_id;
            /* Convert port bitfield to ports */
            vlan_trans_port_bitfield2_ports(entry.ports, conf->ports);
        }
    } while (0);
    T_D("%s: exit\n", __FUNCTION__);
    return rc;
}

/**
 * CALLOUT Function Definitions
 */
void vtss_vlan_trans_crit_data_lock(void)
{
    VT_CRIT_ENTER();
}
void vtss_vlan_trans_crit_data_unlock(void)
{
    VT_CRIT_EXIT();
}
void vtss_vlan_trans_crit_data_assert_locked(void)
{
    VT_CRIT_ASSERT_LOCKED();
}
