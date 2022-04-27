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

#include "pvlan.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "pvlan_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "pvlan_icfg.h"
#endif
#include "misc_api.h"


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static pvlan_global_t pvlan;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "pvlan",
    .descr     = "PVLAN table"
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
#define PVLAN_CRIT_ENTER() critd_enter(&pvlan.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define PVLAN_CRIT_EXIT()  critd_exit( &pvlan.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#else
#define PVLAN_CRIT_ENTER() critd_enter(&pvlan.crit)
#define PVLAN_CRIT_EXIT()  critd_exit( &pvlan.crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  NAMING CONVENTION FOR INTERNAL FUNCTIONS:                               */
/*    SM_<function_name>   : Functions related to SrcMask-based VLAN        */
/*                           (standalone, only).                            */
/*    PI_<function_name>   : Functions related to Port Isolation            */
/*                           (standalone and stacking).                     */
/*    PVLAN_<function_name>: Functions common to SM_xxx and PI_xxx.         */
/*                                                                          */
/*  NAMING CONVENTION FOR EXTERNAL (API) FUNCTIONS:                         */
/*    pvlan_mgmt_pvlan_XXX  : Functions related to SrcMask-based VLAN.      */
/*    pvlan_mgmt_isolate_XXX: Functions related to Port Isolation.          */
/*                                                                          */
/* Notice, that in the Datasheet, Port Isolation is known as Private VLANs. */
/****************************************************************************/

/****************************************************************************/
//
//  SHARED PRIVATE FUNCTIONS, PART 1.
//
/****************************************************************************/

/****************************************************************************/
// PVLAN_msg_id_txt()
/****************************************************************************/
static char *PVLAN_msg_id_txt(PVLAN_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case PVLAN_MSG_ID_CONF_SET_REQ:
        txt = "PVLAN_CONF_SET_REQ";
        break;
    case PVLAN_MSG_ID_CONF_ADD_REQ:
        txt = "PVLAN_CONF_ADD_REQ";
        break;
    case PVLAN_MSG_ID_CONF_DEL_REQ:
        txt = "PVLAN_CONF_DEL_REQ";
        break;
    case PVLAN_MSG_ID_ISOLATE_CONF_SET_REQ:
        txt = "PVLAN_ISOLATE_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}

/****************************************************************************/
/* Allocate request/reply buffer                                            */
/****************************************************************************/
static void PVLAN_msg_alloc(PVLAN_msg_buf_t *buf, BOOL request)
{
    PVLAN_CRIT_ENTER();
    buf->sem = &pvlan.request.sem;
    buf->msg = &pvlan.request.msg[0];
    PVLAN_CRIT_EXIT();
    (void)VTSS_OS_SEM_WAIT(buf->sem);
}

/****************************************************************************/
/* Free request/reply buffer */
/****************************************************************************/
static inline void PVLAN_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

/****************************************************************************/
/****************************************************************************/
static void PVLAN_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    PVLAN_msg_id_t msg_id = *(PVLAN_msg_id_t *)msg;

    T_D("msg_id: %d, %s", msg_id, PVLAN_msg_id_txt(msg_id));
    PVLAN_msg_free(contxt);
}

/****************************************************************************/
/****************************************************************************/
static void PVLAN_msg_tx(PVLAN_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    PVLAN_msg_id_t msg_id = *(PVLAN_msg_id_t *)buf->msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, PVLAN_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, PVLAN_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_PVLAN, isid, buf->msg, len);
}

/****************************************************************************/
//
//  SM_xxx PRIVATE FUNCTIONS
//
/****************************************************************************/

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// SM_entry_add()
// Description:Add Private VLAN entry via switch API
// - Calling API function vtss_pvlan_port_members_set()
// Input:
// Output: Return VTSS_OK if success
/****************************************************************************/
static vtss_rc SM_entry_add(BOOL privatevid_delete, SM_entry_conf_t *conf)
{
    BOOL            member[VTSS_PORT_ARRAY_SIZE];
    port_iter_t     pit;

    memset(member, 0, sizeof(member));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        member[pit.iport] = (VTSS_BF_GET(conf->ports[VTSS_ISID_LOCAL], pit.iport) && (!privatevid_delete)) ? 1 : 0;
    }

    T_I("Private VLAN Add: private privatevid %u, privatevid_delete %d, ports 0x%x",
        conf->privatevid,
        privatevid_delete,
        conf->ports[VTSS_ISID_LOCAL][0]);

    return vtss_pvlan_port_members_set(NULL, conf->privatevid, member);
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// SM_list_pvlan_del()
// If @local is TRUE, modifies pvlan.switch_pvlan, i.e. this switch's
// configuration. Otherwise modifies pvlan.stack_pvlan.
// Deleting a PVLAN on the stack works globally. All ISIDs get it removed.
/****************************************************************************/
static vtss_rc SM_list_pvlan_del(BOOL local, vtss_pvlan_no_t privatevid)
{
    SM_pvlan_t      *pvlan_p, *prev;
    SM_pvlan_list_t *list;
    vtss_rc         rc = VTSS_OK;

    T_I("Delete old PVLAN privatevid: %u", privatevid);

    PVLAN_CRIT_ENTER();

    list = (local ? &pvlan.switch_pvlan : &pvlan.stack_pvlan);

    if (list->used == NULL) {
        rc = PVLAN_ERROR_PVLAN_TABLE_EMPTY;
        goto exit_func;
    }

    /* Delete old PVLAN */
    pvlan_p = list->used;
    prev = NULL;
    while (pvlan_p) {
        if (pvlan_p->conf.privatevid == privatevid) {
            break;
        }
        prev = pvlan_p;
        pvlan_p = pvlan_p->next;
    }

    if (!pvlan_p) {
        rc = PVLAN_ERROR_ENTRY_NOT_FOUND;
        goto exit_func;
    }

    // Concatenate the used list.
    if (prev) {
        prev->next = pvlan_p->next;
    } else {
        list->used = pvlan_p->next;
    }

    // Move it to free list
    pvlan_p->next = list->free;
    list->free = pvlan_p;

exit_func:
    PVLAN_CRIT_EXIT();
    return rc;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// SM_list_pvlan_add()
// Adds or overwrites a private vlan to @isid_add.
// @isid_add may be either
//   VTSS_ISID_LOCAL: Called from msg_rx, i.e. we modify the switch-specific list.
//   VTSS_ISID_LEGAL: Any legal switch ID, i.e. we modify the stack-specific list.
//   VTSS_ISID_GLOBAL: All legal switch IDs, i.e. we modify the stack-specific list.
/****************************************************************************/
static vtss_rc SM_list_pvlan_add(SM_entry_conf_t *pvlan_entry, vtss_isid_t isid_add, BOOL *privatevid_is_new)
{
    SM_pvlan_t      *temp_pvlan, *prev_pvlan, *new_pvlan;
    SM_pvlan_list_t *list;
    vtss_rc         rc = VTSS_OK;
    int             j;

    T_I("enter: privatevid %u isid_add %d", pvlan_entry->privatevid, isid_add);

    PVLAN_CRIT_ENTER();

    list = (isid_add == VTSS_ISID_LOCAL ? &pvlan.switch_pvlan : &pvlan.stack_pvlan);

    // Find insertion/overwrite point
    temp_pvlan = list->used;
    prev_pvlan = NULL;
    while (temp_pvlan) {
        // The Private PRIVATEVIDs are inserted in numeric, inceasing order.
        if (temp_pvlan->conf.privatevid > pvlan_entry->privatevid) {
            break;
        }
        prev_pvlan = temp_pvlan;
        temp_pvlan = temp_pvlan->next;
    }

    // Now, prev_pvlan points to the entry to which to append the new entry, or
    // in case of an overwrite, the entry to overwrite. The remainder of the
    // list is given by temp_pvlan.

    // Better tell the caller whether we're overwriting an existing or not.
    *privatevid_is_new = (prev_pvlan == NULL || prev_pvlan->conf.privatevid != pvlan_entry->privatevid);

    if (*privatevid_is_new) {
        T_D("Adding PRIVATEVID=%u to used list", pvlan_entry->privatevid);

        // Attempt to add a new entry.
        if (list->free == NULL) {
            rc = PVLAN_ERROR_PVLAN_TABLE_FULL;
            goto exit_func; // goto statements are quite alright when we always need to do something special before exiting a function.
        }

        // Pick the first item from the free list.
        new_pvlan = list->free;
        list->free = list->free->next;

        // Insert the new entry in the appropriate position in list->used.
        new_pvlan->next = temp_pvlan; // Attach remainder of list.

        if (prev_pvlan) {
            // Entries before this exist.
            prev_pvlan->next = new_pvlan;
        } else {
            // This goes first in the list.
            list->used = new_pvlan;
        }
    } else {
        // Overwrite existing
        T_D("Replacing PRIVATEVID=%u in used list for isid = %u", pvlan_entry->privatevid, isid_add);
        new_pvlan = prev_pvlan;
    }

    /* This check is to make lint happy */
    if (new_pvlan == NULL) {
        rc = PVLAN_ERROR_PVLAN_TABLE_FULL;
        goto exit_func;
    }
    // Populate new_pvlan entry.
    if (*privatevid_is_new || isid_add == VTSS_ISID_GLOBAL) {
        // Overwrite everything of our current configuration, assuming
        // that pvlan_entry is properly initialized.
        new_pvlan->conf = *pvlan_entry;
    } else {
        // Only overwrite the portion that has changed.
        for (j = 0; j < VTSS_PORT_BF_SIZE; j++) {
            new_pvlan->conf.ports[isid_add][j] = pvlan_entry->ports[isid_add][j];
        }
    }

#if VTSS_SWITCH_STANDALONE
    BOOL no_ports_in_pvlan = 1;
    /* If the user specifies a PVLAN with no ports this shall be treated as if the PVLAN was deleted.
    */
    for (j = 0; j < VTSS_PORT_BF_SIZE; j++) {
        T_I("Checking if all  ports are dis-selected");
        // Check if we have reached the port vector that includes the stack ports.
        if (new_pvlan->conf.ports[isid_add][j] != 0) {
            no_ports_in_pvlan = 0;
        }
    }

    PVLAN_CRIT_EXIT();

    // If no ports were added for this VLAN then delete the VLAN.
    if (no_ports_in_pvlan) {
        T_I("No ports in vlan - deleting");
        (void)SM_list_pvlan_del(FALSE, new_pvlan->conf.privatevid);
        rc = PVLAN_ERROR_DEL_INSTEAD_OF_ADD;
    }
    return rc;
#endif /* VTSS_SWITCH_STANDALONE */

exit_func:
    PVLAN_CRIT_EXIT();
    return rc;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Set PVLAN configuration to switch
/****************************************************************************/
static vtss_rc SM_stack_conf_set(vtss_isid_t isid_add)
{
    PVLAN_msg_buf_t       buf;
    SM_msg_conf_set_req_t *msg;
    vtss_isid_t           isid;
    int                   i, j;
    SM_pvlan_t            *pvlan_stack;

    T_D("enter, isid_add: %d", isid_add);

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }

        PVLAN_msg_alloc(&buf, 1);
        msg = (SM_msg_conf_set_req_t *)buf.msg;
        msg->msg_id = PVLAN_MSG_ID_CONF_SET_REQ;
        PVLAN_CRIT_ENTER();
        for (pvlan_stack = pvlan.stack_pvlan.used, i = 0; pvlan_stack != NULL; pvlan_stack = pvlan_stack->next, i++) {
            msg->table[i].privatevid = pvlan_stack->conf.privatevid;
            for (j = 0; j < VTSS_PORT_BF_SIZE; j++) {
                msg->table[i].ports[VTSS_ISID_LOCAL][j] = pvlan_stack->conf.ports[isid][j];
            }
        }
        msg->count = i;
        PVLAN_CRIT_EXIT();
        PVLAN_msg_tx(&buf, isid, sizeof(*msg) - (VTSS_PVLANS - i)*sizeof(SM_entry_conf_t));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// SM_stack_conf_add()
// Transmit the Private VLAN configuration to @isid_add for privatevid = @privatevid.
// @isid may be VTSS_ISID_GLOBAL if the configuration for @privatevid
// should be sent to all existing switches in the stack. Otherwise
// @isid is just a legal SID for which the PRIVATEVID's port configuration
// has changed (i.e. the PRIVATEVID already existed).
/****************************************************************************/
static void SM_stack_conf_add(vtss_isid_t isid_add, vtss_pvlan_no_t privatevid)
{
    PVLAN_msg_buf_t       buf;
    SM_msg_conf_add_req_t *msg;
    vtss_isid_t           isid_min, isid_max, isid;
    SM_pvlan_t            *pvlan_stack;
    SM_entry_conf_t       conf;
    BOOL                  found;
    int                   j;

    T_D("enter, isid_add: %d, privatevid: %u", isid_add, privatevid);
    memset(&conf, 0, sizeof(conf));

    PVLAN_CRIT_ENTER();

    // Find the PRIVATEVID entry that matches @privatevid
    for (found = FALSE, pvlan_stack = pvlan.stack_pvlan.used; pvlan_stack != NULL; pvlan_stack = pvlan_stack->next) {
        if (pvlan_stack->conf.privatevid == privatevid) {
            // Make a copy of the configuration, so that we can release PVLAN_CRIT
            // before calling PVLAN_msg_alloc(). If we had PVLAN_CRIT when calling
            // PVLAN_msg_alloc(), a deadlock could occur in the following way:
            // Another thread calls PVLAN_msg_alloc() before attempting to acquire
            // PVLAN_CRIT, which we have. When we call PVLAN_msg_alloc(), the buffer
            // is already acquired by the other thread => DEADLOCK.
            conf = pvlan_stack->conf;
            found = TRUE;
            break;
        }
    }

    PVLAN_CRIT_EXIT();

    if (!found) {
        T_W("PRIVATEVID %u not found, isid_add: %d", privatevid, isid_add);
        return;
    }

    // Find boundaries
    if (isid_add == VTSS_ISID_GLOBAL) {
        isid_min = VTSS_ISID_START;
        isid_max = VTSS_ISID_END;
    } else {
        isid_min = isid_add;
        isid_max = isid_add + 1;
    }

    for (isid = isid_min; isid < isid_max; isid++) {
        if (!msg_switch_exists(isid)) {
            continue;
        }
        PVLAN_msg_alloc(&buf, 1);
        msg = (SM_msg_conf_add_req_t *)buf.msg;
        msg->msg_id = PVLAN_MSG_ID_CONF_ADD_REQ;
        msg->conf.privatevid = conf.privatevid;
        for (j = 0; j < VTSS_PORT_BF_SIZE; j++) {
            msg->conf.ports[VTSS_ISID_LOCAL][j] = conf.ports[isid][j];
        }
        PVLAN_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d, privatevid: %u", isid_add, privatevid);
    return;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Delete PVLAN from switch
/****************************************************************************/
static vtss_rc SM_stack_conf_del(vtss_pvlan_no_t privatevid)
{
    PVLAN_msg_buf_t       buf;
    SM_msg_conf_del_req_t *msg;
    vtss_isid_t           isid, isid_del = VTSS_ISID_GLOBAL;

    T_D("enter, isid: %d, privatevid: %u", isid_del, privatevid);

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists(isid)) {
            continue;
        }

        PVLAN_msg_alloc(&buf, 1);
        msg = (SM_msg_conf_del_req_t *)buf.msg;
        memset(&msg->conf, 0, sizeof(SM_entry_conf_t));
        msg->msg_id = PVLAN_MSG_ID_CONF_DEL_REQ;
        msg->conf.privatevid = privatevid;
        PVLAN_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d, privatevid: %u", isid_del, privatevid);
    return VTSS_OK;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
/****************************************************************************/
static BOOL SM_isid_invalid(vtss_isid_t isid)
{
    if (isid > VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return 1;
    }

    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        T_W("not master");
        return 1;
    }

    if (VTSS_ISID_LEGAL(isid) && !msg_switch_configurable(isid)) {
        T_W("isid %d not active", isid);
        return 1;
    }

    return 0;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Store srcmask-based PVLAN configuration
/****************************************************************************/
static vtss_rc SM_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t blk_id;
    SM_blk_t      *pvlan_blk;
    SM_pvlan_t    *pvlan_stack;
    ulong         i;

    T_D("enter");

    blk_id = CONF_BLK_PVLAN_TABLE;
    if ((pvlan_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open PVLAN member table");
        return PVLAN_ERROR_GEN;
    }

    PVLAN_CRIT_ENTER();
    for (pvlan_stack = pvlan.stack_pvlan.used, i = 0; pvlan_stack != NULL; pvlan_stack = pvlan_stack->next, i++) {
        pvlan_blk->table[i] = pvlan_stack->conf;
    }
    pvlan_blk->count = i;
    PVLAN_CRIT_EXIT();

    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    T_D("exit");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return VTSS_OK;
}
#endif /* PVLAN_SRC_MASK_ENA */

/****************************************************************************/
//
//  PI_xxx PRIVATE FUNCTIONS
//
/****************************************************************************/

/*****************************************************************/
/* Description: Setup default pvlan port configuration           */
/*                                                               */
/* Input:  Pointer to pvlan port entry                           */
/* Output:                                                       */
/*****************************************************************/
static inline void PI_default_set(BOOL *PI_conf)
{
    /* Default to no port being isolated */
    memset(PI_conf, 0, sizeof(BOOL) * VTSS_PORT_ARRAY_SIZE);
}

/****************************************************************************/
// Set port configuration
/****************************************************************************/
static vtss_rc PI_stack_conf_set(vtss_isid_t isid)
{
    PVLAN_msg_buf_t       buf;
    PI_msg_conf_set_req_t *msg;

    T_D("enter, isid: %d", isid);

    PVLAN_msg_alloc(&buf, 1);
    msg = (PI_msg_conf_set_req_t *)buf.msg;
    msg->msg_id = PVLAN_MSG_ID_ISOLATE_CONF_SET_REQ;
    PVLAN_CRIT_ENTER();
    memcpy(&msg->conf[0], pvlan.PI_conf[isid], sizeof(pvlan.PI_conf[isid]));
    PVLAN_CRIT_EXIT();
    PVLAN_msg_tx(&buf, isid, sizeof(*msg));
    T_D("exit, isid: %d", isid);

    return VTSS_OK;
}

/****************************************************************************/
//
//  SHARED PRIVATE FUNCTIONS, PART 2.
//
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
static BOOL PVLAN_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    vtss_port_no_t        port_no;
    PVLAN_msg_id_t msg_id = *(PVLAN_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, PVLAN_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
#if defined(PVLAN_SRC_MASK_ENA)
    case PVLAN_MSG_ID_CONF_SET_REQ: {
        // Sets the whole configuration - not just one PVLAN entry.
        // We need to delete all currently configured PVLANs and
        // possibly create some new ones. This will shortly give
        // rise to no configured PVLANs, but that's ignorable.
        SM_msg_conf_set_req_t *msg;
        SM_pvlan_t            *pvlan_switch;
        SM_pvlan_list_t       *switch_list;
        SM_entry_conf_t       delete_default_conf;
        u32                   i;
        BOOL                  privatevid_is_new;

        PVLAN_CRIT_ENTER();
        switch_list = &pvlan.switch_pvlan; // Select the actual configuration

        // Always delete default PVLAN, since it is initialized by the Switch API to contain
        // all ports upon boot, and if it's not included in the list of PVLANs that are loaded
        // from flash, all ports will always be members of all PVLANs, independent of how
        // the remaining PVLANs are configured.
        delete_default_conf.privatevid = VTSS_PVLAN_NO_START;
        (void)SM_entry_add(PVLAN_DELETE, &delete_default_conf);

        /* Delete current PVLAN */
        for (pvlan_switch = switch_list->used; pvlan_switch != NULL; pvlan_switch = pvlan_switch->next) {
            T_D("Calling SM_entry_add from PVLAN_MSG_ID_CONF_SET_REQ - PVLAN_DELETE privatevid %u", pvlan_switch->conf.privatevid);
            (void)SM_entry_add(PVLAN_DELETE, &pvlan_switch->conf);
            if (pvlan_switch->next == NULL) {
                /* Last entry found, move used list to free list */
                pvlan_switch->next = switch_list->free;
                switch_list->free = switch_list->used;
                switch_list->used = NULL;
                break;
            }
        }

        PVLAN_CRIT_EXIT();

        /* Add new PVLANs */
        msg = (SM_msg_conf_set_req_t *)rx_msg;
        for (i = 0; i < msg->count; i++) {
            if (SM_list_pvlan_add(&msg->table[i], VTSS_ISID_LOCAL, &privatevid_is_new) == VTSS_OK) {
                (void)SM_entry_add(PVLAN_ADD, &msg->table[i]);
            }
        }

        break;
    }

    case PVLAN_MSG_ID_CONF_ADD_REQ: {
        SM_msg_conf_add_req_t *msg;
        BOOL                  privatevid_is_new;

        msg = (SM_msg_conf_add_req_t *)rx_msg;

        /* Add/modify entry through API */
        T_D("Calling SM_entry_add - PVLAN_ADD from PVLAN_MSG_ID_CONF_ADD_REQ rx_msg, isid %u, privatevid=%u", isid, msg->conf.privatevid);
        if (SM_list_pvlan_add(&msg->conf, VTSS_ISID_LOCAL, &privatevid_is_new) == VTSS_OK) {
            (void)SM_entry_add(PVLAN_ADD, &msg->conf);
        }
        break;
    }

    case PVLAN_MSG_ID_CONF_DEL_REQ: {
        SM_msg_conf_del_req_t *msg;

        msg = (SM_msg_conf_del_req_t *)rx_msg;
        // Delete the switch from this switch's configuration (first param = TRUE = it's the local conf).
        T_D("Calling SM_entry_add - PVLAN_DELETE from PVLAN_MSG_ID_CONF_DEL_REQ rx_msg, isid %u, privatevid=%u", isid, msg->conf.privatevid);
        if (SM_list_pvlan_del(TRUE, msg->conf.privatevid) == VTSS_OK) {
            (void)SM_entry_add(PVLAN_DELETE, &msg->conf);
        }
        break;
    }
#endif /* PVLAN_SRC_MASK_ENA */

    case PVLAN_MSG_ID_ISOLATE_CONF_SET_REQ: {
        PI_msg_conf_set_req_t *msg;
        BOOL                  *conf;
        int                   changed;
#if VTSS_SWITCH_STACKABLE
        port_isid_info_t      pinfo;
#endif

        msg = (PI_msg_conf_set_req_t *)rx_msg;
        conf = &msg->conf[0];

#if VTSS_SWITCH_STACKABLE
        // Never isolate stack ports.
        if (port_isid_info_get(VTSS_ISID_LOCAL, &pinfo) == VTSS_RC_OK) {
            conf[pinfo.stack_port_0] = 0;
            conf[pinfo.stack_port_1] = 0;
        }
#endif

        PVLAN_CRIT_ENTER();
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            T_D("Port=%d, old_conf=%d, new_conf=%d", port_no, pvlan.PI_conf[VTSS_ISID_LOCAL][port_no], conf[port_no]);
        }

        changed = memcmp(&pvlan.PI_conf[VTSS_ISID_LOCAL][0], &conf[0], sizeof(pvlan.PI_conf[VTSS_ISID_LOCAL]));
        if (changed) {
            memcpy(&pvlan.PI_conf[VTSS_ISID_LOCAL][0], &conf[0], sizeof(pvlan.PI_conf[VTSS_ISID_LOCAL]));
            T_D("Calling vtss_isolated_port_members_set() from PVLAN_MSG_ID_ISOLATE_CONF_SET_REQ with isid %u", isid);
            (void)vtss_isolated_port_members_set(NULL, conf);
        }
        PVLAN_CRIT_EXIT();
        break;
    }

    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

/****************************************************************************/
/****************************************************************************/
static inline vtss_rc PVLAN_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = PVLAN_msg_rx;
    filter.modid = VTSS_MODULE_ID_PVLAN;
    return msg_rx_filter_register(&filter);
}

/****************************************************************************/
// PVLAN_conf_read_switch()
// Read/create per-switch configuration. For the PVLAN module, this only
// concerns the port-isolation PVLANs.
/****************************************************************************/
static void PVLAN_conf_read_switch(vtss_isid_t isid_add)
{
    conf_blk_id_t  blk_id;
    PI_blk_t       *isolate_blk;
    BOOL           new_conf[VTSS_PORT_ARRAY_SIZE], *isolate_conf;
    int            i, changed;
    BOOL           do_create;
    ulong          size;
    vtss_isid_t    isid;

    T_D("enter, isid_add: %d", isid_add);

    if (misc_conf_read_use()) {
        /* Read/create port isolation configuration */
        blk_id = CONF_BLK_PVLAN_ISOLATE_TABLE;
        if ((isolate_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*isolate_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            isolate_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*isolate_blk));
            do_create = 1;
            T_I("done conf_sec_create");
        } else if (isolate_blk->version != PVLAN_ISOLATE_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        isolate_blk = NULL;
        do_create   = 1;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        changed = 0;
        PVLAN_CRIT_ENTER();
        i = (isid - VTSS_ISID_START);

        if (do_create) {
            /* Use default values */
            PI_default_set(&new_conf[0]);
            if (isolate_blk != NULL) {
                T_I("Using default configuration");
                memcpy(isolate_blk->conf[i], &new_conf[0], sizeof(new_conf));
            }
        } else {
            /* Use new configuration */
            T_I("Using flash configuration");
            if (isolate_blk != NULL) {  // Quiet lint
                memcpy(&new_conf[0], isolate_blk->conf[i], sizeof(new_conf));
            }
        }
        isolate_conf = &pvlan.PI_conf[isid][0];
        changed = memcmp(&new_conf[0], isolate_conf, sizeof(new_conf));
        memcpy(isolate_conf, &new_conf[0], sizeof(new_conf));
        PVLAN_CRIT_EXIT();
        if (isid_add != VTSS_ISID_GLOBAL && changed && msg_switch_exists(isid)) {
            (void)PI_stack_conf_set(isid);
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (isolate_blk == NULL) {
        T_W("failed to open port isolation table");
    } else {
        isolate_blk->version = PVLAN_ISOLATE_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    T_I("exit");
    return;

} /* End - Read/create PVLAN port isolation Switch configuration */

/****************************************************************************/
// PVLAN_conf_read_stack()
// Read/create stack-wide configuration. For the PVLAN module, this only
// concerns the srcmask-based PVLANs (which are not enabled for the stack,
// which sounds like a contradiction, but we could enable it in the future).
/****************************************************************************/
#if defined(PVLAN_SRC_MASK_ENA)
static void PVLAN_conf_read_stack(BOOL create)
{
    conf_blk_id_t   blk_id;
    SM_blk_t        *pvlan_blk;
    SM_pvlan_list_t *list;
    SM_pvlan_t      *pvlan_stack;
    int             i, changed;
    BOOL            do_create;
    vtss_isid_t     isid;
    ulong           size;

    T_D("enter, create: %d", create);

    /* Read/create PVLAN table configuration */
    blk_id = CONF_BLK_PVLAN_TABLE;

    if (misc_conf_read_use()) {
        if ((pvlan_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL || size != sizeof(*pvlan_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            pvlan_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*pvlan_blk));
            do_create = 1;
        } else if (pvlan_blk->version != PVLAN_MEMBERSHIP_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        PVLAN_CRIT_ENTER();

        list = &pvlan.stack_pvlan;
        changed = (list->used != NULL);

        if (do_create && pvlan_blk != NULL) {
            T_D("do_create && pvlan_blk != NULL");
            pvlan_blk->count = 1;
            pvlan_blk->size = sizeof(SM_entry_conf_t);
            pvlan_blk->table[0].privatevid = VTSS_PVLAN_NO_START;
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    pvlan_blk->table[0].ports[isid][i] = 0xff;
                }
            }
            changed = TRUE;
        }

        /* Free old PVLANs */
        for (pvlan_stack = list->used; pvlan_stack != NULL; pvlan_stack = pvlan_stack->next) {
            if (pvlan_stack->next == NULL) {
                /* Last entry found, move used list to free list */
                pvlan_stack->next = list->free;
                list->free = list->used;
                list->used = NULL;
                break;
            }
        }

        if (pvlan_blk == NULL) {
            T_D("failed to open PVLAN table");
        } else {
            /* Add new PVLANs */
            T_D("Add new PVLANs");
            for (i = pvlan_blk->count; i != 0; i--) {
                /* Move entry from free list to used list */
                if ((pvlan_stack = list->free) == NULL) {
                    break;
                }
                list->free = pvlan_stack->next;
                pvlan_stack->next = list->used;
                list->used = pvlan_stack;
                pvlan_stack->conf = pvlan_blk->table[i - 1];
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            pvlan_blk->version = PVLAN_MEMBERSHIP_BLK_VERSION;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        }
        PVLAN_CRIT_EXIT();

        if (changed && create) {
            (void)SM_stack_conf_set(VTSS_ISID_GLOBAL);
        }
    } else {
        // Not using conf

        PVLAN_CRIT_ENTER();

        list = &pvlan.stack_pvlan;

        /* Free old PVLANs */
        for (pvlan_stack = list->used; pvlan_stack != NULL; pvlan_stack = pvlan_stack->next) {
            if (pvlan_stack->next == NULL) {
                /* Last entry found, move used list to free list */
                pvlan_stack->next = list->free;
                list->free = list->used;
                list->used = NULL;
                break;
            }
        }

        /* Move entry from free list to used list */
        if ((pvlan_stack = list->free) == NULL) {
            T_W("This really wasn't expected. Bug.");
        } else {
            list->free = pvlan_stack->next;
            pvlan_stack->next = list->used;
            list->used = pvlan_stack;
            pvlan_stack->conf.privatevid = VTSS_PVLAN_NO_START;
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    pvlan_stack->conf.ports[isid][i] = 0xff;
                }
            }
        }

        PVLAN_CRIT_EXIT();

        (void)SM_stack_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
} /* End - Read/create PVLAN configuration */
#endif /* PVLAN_SRC_MASK_ENA */

/****************************************************************************/
// Module start
/****************************************************************************/
static void PVLAN_start(BOOL init)
{
    T_D("enter init %d", init);

    if (init) {
        vtss_isid_t     isid;
#if defined(PVLAN_SRC_MASK_ENA)
        int             i;
        SM_pvlan_t      *pvlan_p;
        SM_pvlan_list_t *list;

        /* Initialize Private VLAN for stack: All free */
        list = &pvlan.stack_pvlan;
        list->free = NULL;
        list->used = NULL;
        for (i = 0; i < VTSS_PVLANS; i++) {
            pvlan_p = &pvlan.pvlan_stack_table[i];
            pvlan_p->next = list->free;
            list->free = pvlan_p;
        }

        /* Initialize Private VLAN for this switch: All free */
        list = &pvlan.switch_pvlan;
        list->free = NULL;
        list->used = NULL;
        for (i = 0; i < VTSS_PVLANS; i++) {
            pvlan_p = &pvlan.pvlan_switch_table[i];
            pvlan_p->next = list->free;
            list->free = pvlan_p;
        }
#endif /* PVLAN_SRC_MASK_ENA */

        /* Initialize port isolation configuration */
        for (isid = VTSS_ISID_LOCAL; isid < VTSS_ISID_END; isid++) {
            PI_default_set(&pvlan.PI_conf[isid][0]);
        }

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&pvlan.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&pvlan.crit, "pvlan.crit", VTSS_MODULE_ID_PVLAN, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        PVLAN_CRIT_EXIT();
    } else {
        /* Register for stack messages */
        (void)PVLAN_stack_register();
    }

    T_D("exit");
}

/****************************************************************************/
// Determine if port and ISID are valid
/****************************************************************************/
static BOOL PI_isid_invalid(vtss_isid_t isid, BOOL set)
{
    /* Check ISID */
    if (isid >= VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return TRUE;
    }

    if (set && isid == VTSS_ISID_LOCAL) {
        T_W("SET not allowed, isid: %d", isid);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************/
// Store port isolation PVLAN configuration
/****************************************************************************/
static vtss_rc PI_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t blk_id;
    PI_blk_t      *PI_blk = NULL;
    vtss_isid_t   isid;
    u32           port;

    T_D("enter");
    blk_id = CONF_BLK_PVLAN_ISOLATE_TABLE;
    if ((PI_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_W("failed to open port-isolation table");
        return PVLAN_ERROR_GEN;
    }

    PVLAN_CRIT_ENTER();
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        // Copy from global setting into the flash block.
        // The flash is 0-based, whereas the global setting is VTSS_ISID_START based.
        for (port = 0; port < VTSS_PORT_ARRAY_SIZE; port++) {
            PI_blk->conf[isid - VTSS_ISID_START][port] = pvlan.PI_conf[isid][port];
        }
    }
    PVLAN_CRIT_EXIT();

    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    T_D("exit");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return VTSS_OK;
}

/****************************************************************************/
/*  Public functions                                                        */
/****************************************************************************/

/****************************************************************************/
// PVLAN error text
/****************************************************************************/
char *pvlan_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case PVLAN_ERROR_GEN:
        txt = "PVLAN generic error";
        break;
    case PVLAN_ERROR_PARM:
        txt = "PVLAN parameter error";
        break;
    case PVLAN_ERROR_ENTRY_NOT_FOUND:
        txt = "Entry not found";
        break;
    case PVLAN_ERROR_PVLAN_TABLE_FULL:
        txt = "PVLAN table full";
        break;
    case PVLAN_ERROR_PVLAN_TABLE_EMPTY:
        txt = "PVLAN table empty";
        break;
    case PVLAN_ERROR_STACK_STATE:
        txt = "Invalid stack state";
        break;
    case PVLAN_ERROR_UNSUPPORTED:
        txt = "Unsupported feature";
        break;
    default:
        txt = "PVLAN unknown error";
        break;
    }
    return txt;
}

/****************************************************************************/
// Description:Management PVLAN isolate get function
// - Only accessing switch module linked list
//
// Input: isid and port isolation configuration
// Output: Return VTSS_OK if success
/****************************************************************************/
vtss_rc pvlan_mgmt_isolate_conf_get(vtss_isid_t isid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    port_iter_t     pit;

    T_D("enter, isid=%d", isid);

    if (PI_isid_invalid(isid, FALSE)) {
        return PVLAN_ERROR_PARM;
    }

    PVLAN_CRIT_ENTER();
    (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        member[pit.iport] = pvlan.PI_conf[isid][pit.iport];
    }
    PVLAN_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/****************************************************************************/
// pvlan_mgmt_isolate_conf_set()
// Management PVLAN port isolation set function
// - Both configuration update and chip update through message module and
// switch API
// Input: isid, port number and port isolation struct. Must be LEGAL.
// Output: Return VTSS_OK if success
/****************************************************************************/
vtss_rc pvlan_mgmt_isolate_conf_set(vtss_isid_t isid, const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    vtss_rc             rc = VTSS_OK;
    int                 changed = 0;
    BOOL                local_member[VTSS_PORT_ARRAY_SIZE];

    T_D("enter, isid: %d", isid);
    memcpy(&local_member[0], &member[0], sizeof(local_member));

    // We allow configuration where stack ports (in stackable builds) are
    // marked for isolation. Only when applying the config to real
    // hardware, will possible stack ports get unmarked.

    if (PI_isid_invalid(isid, TRUE)) {
        return PVLAN_ERROR_PARM;
    }

    if (msg_switch_configurable(isid)) {
        PVLAN_CRIT_ENTER();
        changed = memcmp(&local_member[0], &pvlan.PI_conf[isid][0], sizeof(local_member));
        memcpy(&pvlan.PI_conf[isid][0], &member[0], sizeof(pvlan.PI_conf[isid]));
        PVLAN_CRIT_EXIT();
    } else {
        T_W("isid %d not active", isid);
        return PVLAN_ERROR_STACK_STATE;
    }

    if (changed) {
        /* Save changed configuration */
        rc = PI_conf_commit();
        if (rc == VTSS_OK) {
            /* Activate changed configuration in stack */
            (void)PI_stack_conf_set(isid);
        }
    }
    T_D("exit, isid: %d", isid);
    return rc;
}

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Description:Management PVLAN list get function
// - Only accessing internal linked lists
//
// Input:Note pvlan members as boolean port list.
//       If next and privatevid = 0 then will return first PVLAN entry
// Output: Return VTSS_OK if success
/****************************************************************************/
vtss_rc pvlan_mgmt_pvlan_get(vtss_isid_t isid, vtss_pvlan_no_t privatevid, pvlan_mgmt_entry_t *pvlan_mgmt_entry, BOOL next)
{
    vtss_rc         rc = VTSS_OK;
    SM_pvlan_t      *pvlan_p;
    BOOL            found = FALSE;
    port_iter_t     pit;
    SM_pvlan_list_t *list;

    T_D("isid=%d, privatevid=%u, next=%d", isid, privatevid, next);
    PVLAN_CRIT_ENTER();
    list = isid == VTSS_ISID_LOCAL ? &pvlan.switch_pvlan : &pvlan.stack_pvlan;
    if (SM_isid_invalid(isid)) {
        PVLAN_CRIT_EXIT();
        return PVLAN_ERROR_STACK_STATE;
    }
    memset(pvlan_mgmt_entry->ports, 0, sizeof(pvlan_mgmt_entry->ports));
    for (pvlan_p = list->used; pvlan_p != NULL; pvlan_p = pvlan_p->next) {
        if ((next && pvlan_p->conf.privatevid > privatevid) ||
            (!next && pvlan_p->conf.privatevid == privatevid)) {
            found = TRUE;
            pvlan_mgmt_entry->privatevid = pvlan_p->conf.privatevid;
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                pvlan_mgmt_entry->ports[pit.iport] = (BOOL)VTSS_BF_GET(pvlan_p->conf.ports[isid], pit.iport);
            }
            break;
        }
    }
    PVLAN_CRIT_EXIT();

    if (!found) {
        rc = PVLAN_ERROR_ENTRY_NOT_FOUND;
        T_D("PVLAN_ERROR_ENTRY_NOT_FOUND - Called with privatevid = %u, next=%d",
            privatevid, next);
    }
    T_D("return %d \n", rc);

    return rc;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Description:Management PVLAN list add function
// - Both configuration update and chip update through message
// - module and API
//
// Input: Note pvlan members as boolean port list
//        @isid must be LEGAL or GLOBAL
// Output: Return VTSS_OK if success
/****************************************************************************/
vtss_rc pvlan_mgmt_pvlan_add(vtss_isid_t isid_add, pvlan_mgmt_entry_t *pvlan_mgmt_entry)
{
    vtss_rc             rc = VTSS_OK;
    BOOL                privatevid_is_new;
    SM_entry_conf_t     pvlan_entry;
    vtss_isid_t         isid, isid_min, isid_max;
    int                 i;
#if VTSS_SWITCH_STACKABLE
    port_isid_info_t    pinfo;
#endif

    T_D("enter, isid: %u, privatevid: %u", isid_add, pvlan_mgmt_entry->privatevid);

    if (!msg_switch_is_master()) {
        T_W("Not master");
        return PVLAN_ERROR_STACK_STATE;
    }

    if (isid_add != VTSS_ISID_GLOBAL && !VTSS_ISID_LEGAL(isid_add)) {
        T_E("Invalid ISID (%u). LEGAL or GLOBAL expected", isid_add);
        return PVLAN_ERROR_PARM;
    }

    // Convert the pvlan_mgmt_entry_t to an SM_entry_conf_t
    if (isid_add == VTSS_ISID_GLOBAL) {
        isid_min = VTSS_ISID_START;
        isid_max = VTSS_ISID_END;
    } else {
        isid_min = isid_add;
        isid_max = isid_add + 1;
    }

    pvlan_entry.privatevid = pvlan_mgmt_entry->privatevid;
    memset(pvlan_entry.ports, 0, sizeof(pvlan_entry.ports));

    // First set the front ports for selected ISIDs.
    for (isid = isid_min; isid < isid_max; isid++) {
        for (i = 0; i < VTSS_PORTS; i++) {
            VTSS_BF_SET(pvlan_entry.ports[isid], i, pvlan_mgmt_entry->ports[i + VTSS_PORT_NO_START]);
        }
    }

#if VTSS_SWITCH_STACKABLE
    // Then set the stack ports to members for all ISIDs, if the switch is stackable.
    // By default srcmask-based PVLANs are not stackable, but if we at some time
    // choose to offer it in our stacked solution, the stack ports must be included.
    // It will have the side effect that a frame arriving on private port p1 on switch s1,
    // will be forwardable to switch s2 where it will be able to hit any front port,
    // so the VLAN is not that private anymore.
    for (isid = VTSS_ISID_LOCAL; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists(isid + 1)) {
            continue;
        }
        if (port_isid_info_get(isid + 1, &pinfo) == VTSS_RC_OK) {
            VTSS_BF_SET(pvlan_entry.ports[isid], pinfo.stack_port_0 - VTSS_PORT_NO_START, 0x1);
            VTSS_BF_SET(pvlan_entry.ports[isid], pinfo.stack_port_1 - VTSS_PORT_NO_START, 0x1);
        }
    }
#endif

    // Add/modify PRIVATEVID. It may return rc != VTSS_OK if table is full.
    rc = SM_list_pvlan_add(&pvlan_entry, isid_add, &privatevid_is_new);

    // Save it to flash
    if (rc == VTSS_OK) {
        rc = SM_conf_commit();
    }

    // And transmit it to @isid_add. If @privatevid_is_new, transmit it to all switches,
    // since all switches must include the PVLAN on their stack ports.
    if (rc == VTSS_OK) {
        SM_stack_conf_add(privatevid_is_new ? VTSS_ISID_GLOBAL : isid_add, pvlan_mgmt_entry->privatevid);
    }

    return rc;
}
#endif /* PVLAN_SRC_MASK_ENA */

#if defined(PVLAN_SRC_MASK_ENA)
/****************************************************************************/
// Delete Private VLAN
// Description:Management PVLAN list delete function
// - Both configuration update and chip update through API
// Deletes globally. Use "cli: pvlan add <privatevid> <stack_port>" to
// delete frontports locally on a switch.
//
// Input:
// Output: Return VTSS_OK if success
/****************************************************************************/
vtss_rc pvlan_mgmt_pvlan_del(vtss_pvlan_no_t privatevid)
{
    vtss_rc     rc;

    T_D("enter, privatevid: %u", privatevid);

    // Delete the entry from the stack configuration
    rc = SM_list_pvlan_del(FALSE, privatevid);

    // Save it to flash
    if (rc == VTSS_OK) {
        rc = SM_conf_commit();
    }

    // Tell it to all switches in the stack.
    if (rc == VTSS_OK) {
        rc = SM_stack_conf_del(privatevid);
    }

    T_D("exit, id: %u", privatevid);

    return rc;
}
#endif /* PVLAN_SRC_MASK_ENA */

/****************************************************************************/
// Initialize module
/****************************************************************************/
vtss_rc pvlan_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        PVLAN_start(1);
#ifdef VTSS_SW_OPTION_VCLI
        pvlan_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = PVLAN_icfg_init()) != VTSS_OK) {
            T_D("Calling pvlan_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        PVLAN_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
#if defined(PVLAN_SRC_MASK_ENA)
            /* Reset stack configuration */
            PVLAN_conf_read_stack(TRUE);
#endif
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            PVLAN_conf_read_switch(isid);
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
#if defined(PVLAN_SRC_MASK_ENA)
        /* Read stack and switch configuration */
        PVLAN_conf_read_stack(FALSE);
#endif
        PVLAN_conf_read_switch(VTSS_ISID_GLOBAL);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        (void)PI_stack_conf_set(isid);
#if defined(PVLAN_SRC_MASK_ENA)
        (void)SM_stack_conf_set(isid);
#endif /* PVLAN_SRC_MASK_ENA */
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
