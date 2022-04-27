/*
   Vitesse VLAN Translation software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <vtss_vlan_translation.h>

/*lint -sem( vtss_vlan_trans_crit_data_lock, thread_lock ) */
/*lint -sem( vtss_vlan_trans_crit_data_unlock, thread_unlock ) */
/**
 *  VLAN Translation Global Data
 **/
vlan_trans_global_data_t   vt_glb_data;

void vlan_trans_ports2_port_bitfield(BOOL *ports, u8 *ports_bf)
{
    u32 port_num;
    memset(ports_bf, 0, VTSS_PORT_BF_SIZE);
    for (port_num = 0; port_num < VTSS_PORT_ARRAY_SIZE; port_num++) {
        if (ports[port_num] == TRUE) {
            ports_bf[port_num / 8] |= (1 << (port_num % 8));
        }
    }
}
void vlan_trans_port_bitfield2_ports(u8 *ports_bf, BOOL *ports)
{
    u32   i;
    for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
        ports[i] = ((ports_bf[i / 8]) & (1 << (i % 8))) ? TRUE : FALSE;
    }
}
void vlan_trans_default_set(void)
{
    vlan_trans_grp2vlan_list_t          *vlan_list;
    vlan_trans_grp2vlan_list_entry_t    *vlan_conf;
    vlan_trans_port2grp_list_t          *port_list;
    vlan_trans_port2grp_list_entry_t    *port_conf;
    u32                                 i;
    memset(&vt_glb_data, 0, sizeof(vlan_trans_global_data_t));
    /* Initialize list of VLAN Translations */
    vlan_list = &vt_glb_data.grp2vlan_list;
    vlan_list->used = NULL;
    vlan_list->free = NULL;
    for (i = 0; i < VT_MAX_TRANSLATION_CNT; i++) {
        vlan_conf = &vt_glb_data.grp2vlan_db[i];
        vlan_conf->next = vlan_list->free;
        vlan_list->free = vlan_conf;
    }
    /* Initialize list of port to group mappings */
    port_list = &vt_glb_data.port2grp_list;
    port_list->used = NULL;
    port_list->free = NULL;
    /* Each port is by default mapped to group with id = port_num */
    for (i = 0; i < VT_MAX_GROUP_CNT; i++) {
        port_conf = &vt_glb_data.port2grp_db[i];
        port_conf->next = port_list->free;
        port_list->free = port_conf;
    }
}
void vlan_trans_vlan_list_default_set(void)
{
    vlan_trans_grp2vlan_list_t          *vlan_list;
    vlan_trans_grp2vlan_list_entry_t    *vlan_conf;
    u32                                 i;
    vtss_vlan_trans_crit_data_lock();
    /* Initialize list of VLAN Translations */
    vlan_list = &vt_glb_data.grp2vlan_list;
    vlan_list->used = NULL;
    vlan_list->free = NULL;
    for (i = 0; i < VT_MAX_TRANSLATION_CNT; i++) {
        vlan_conf = &vt_glb_data.grp2vlan_db[i];
        vlan_conf->next = vlan_list->free;
        vlan_list->free = vlan_conf;
    }
    vtss_vlan_trans_crit_data_unlock();
}
void vlan_trans_port_list_default_set(void)
{
    vlan_trans_port2grp_list_t          *port_list;
    vlan_trans_port2grp_list_entry_t    *port_conf;
    u32                                 i;
    /* Initialize list of port to group mappings */
    port_list = &vt_glb_data.port2grp_list;
    port_list->used = NULL;
    port_list->free = NULL;
    /* Each port is by default mapped to group with id = port_num */
    for (i = 0; i < VT_MAX_GROUP_CNT; i++) {
        port_conf = &vt_glb_data.port2grp_db[i];
        port_conf->next = port_list->free;
        port_list->free = port_conf;
    }
}
/************** Functions to manipulate Group-VLAN mappings *********************************************************/
/************** ------------------------------------------- *********************************************************/
u32 vlan_trans_grp2vlan_entry_add(const u16 group_id, const vtss_vid_t vid, const vtss_vid_t trans_vid)
{
    vlan_trans_grp2vlan_list_t          *list;
    vlan_trans_grp2vlan_list_entry_t    *tmp, *prev, *new = NULL;
    BOOL                                update_entry = FALSE;
    vtss_vlan_trans_crit_data_lock();
    list = &vt_glb_data.grp2vlan_list;
    /* Insert the new node into the used list */
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        /* Check to see if the vid is already configured for this group */
        if ((group_id == tmp->conf.group_id) && (vid == tmp->conf.vid)) {
            /* Overwrite the trans_vid */
            tmp->conf.trans_vid = trans_vid;
            update_entry = TRUE;
            break;
        }
        /* List needs to be sorted based on group_id */
        if (group_id > tmp->conf.group_id) {
            break;
        }
    }
    if (update_entry == FALSE) {
        /* Get free node from free list */
        new = list->free;
        if (new == NULL) {  /* No free entry exists */
            vtss_vlan_trans_crit_data_unlock();
            return VTSS_VT_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        new->conf.group_id = group_id;
        new->conf.vid = vid;
        new->conf.trans_vid = trans_vid;
        new->next = NULL;
        /* Group to VLAN mapping can be added in to the used list in three ways:
           1. Add at the beginning of the list;
           2. Add somewhere in the middle of the list;
           3. Add at the end of the list.
         */
        if (tmp != NULL) { /* This means insertion point found */
            if (prev == NULL) {
                /* Add at the beginning of the list */
                new->next = list->used;
                list->used = new;
            } else {
                /* Add somewhere in the middle of the list */
                prev->next = new;
                new->next = tmp;
            }
        } else { /* insertion point not found; add at the end of the list */
            if (prev == NULL) { /* used list is empty */
                list->used = new;
            } else {
                prev->next = new;
            }
        }
    }
    /* VT-TODO: At this point, if there is already a port to group mapping for this group,
       we need to add h/w entries */
    vtss_vlan_trans_crit_data_unlock();
    return VTSS_VT_RC_OK;
}
u32 vlan_trans_grp2vlan_entry_delete(const u16 group_id, const vtss_vid_t vid)
{
    vlan_trans_grp2vlan_list_t          *list;
    vlan_trans_grp2vlan_list_entry_t    *tmp, *prev;
    vtss_vlan_trans_crit_data_lock();
    list = &vt_glb_data.grp2vlan_list;
    /* Search used list to find out matching entry */
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        if ((group_id == tmp->conf.group_id) && (vid == tmp->conf.vid)) {
            break;
        }
    }
    if (tmp != NULL) { /* This means deletion point found */
        if (prev == NULL) { /* Delete the first node */
            /* Remove from the used list */
            list->used = tmp->next;
        } else {
            /* Remove from the used list */
            prev->next = tmp->next;
        }
        /* Add to the free list */
        tmp->next = list->free;
        list->free = tmp;
    }
    /*  VT-TODO: We may need to delete the h/w entry if there is a port to group mapping
        for this group */
    vtss_vlan_trans_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VT_RC_OK : VTSS_VT_ERROR_ENTRY_NOT_FOUND);
}
u32 vlan_trans_grp2vlan_entry_get(vlan_trans_grp2vlan_conf_t *conf, BOOL next)
{
    vlan_trans_grp2vlan_list_t          *list;
    vlan_trans_grp2vlan_list_entry_t    *tmp;
    BOOL                                use_next = FALSE;
    /* Check for valid group_id */
    if ((VT_VALID_GROUP_CHECK(conf->group_id) == FALSE) && (conf->group_id != VT_NULL_GROUP_ID)) {
        return VTSS_VT_ERROR_PARM;
    }
    vtss_vlan_trans_crit_data_lock();
    list = &vt_glb_data.grp2vlan_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        /* If group_id is zero, get first entry */
        if (conf->group_id == VT_NULL_GROUP_ID) {
            break;
        } else {
            if (use_next) {
                break;
            }
            /* Group Id and VID match check */
            if ((conf->group_id == tmp->conf.group_id) && (conf->vid == tmp->conf.vid)) {
                if (next) { /* Return the next entry */
                    use_next = TRUE;
                } else {
                    break;
                } /* if (next) */
            } /* if ((conf->group_id == tmp->conf.group_id */
        } /* if (conf->group_id == VT_NULL_GROUP_ID */
    } /* for (tmp = list->used; */
    if (tmp != NULL) { /* Entry exists, return the configuration */
        *conf = tmp->conf;
    }
    vtss_vlan_trans_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VT_RC_OK : VTSS_VT_ERROR_ENTRY_NOT_FOUND);
}
/************** End *************************************************************************************************/
/************** Functions to manipulate Port to Group mappings ******************************************************/
/************** ---------------------------------------------- ******************************************************/
u32 vlan_trans_port2grp_entry_add(const u16 group_id, u8 *port_bf)
{
    vlan_trans_port2grp_list_t          *list;
    vlan_trans_port2grp_list_entry_t    *tmp, *prev, *new = NULL;
    u32                                 i, j;
    BOOL                                modified_entry, update_entry = FALSE;
    vtss_vlan_trans_crit_data_lock();
    list = &vt_glb_data.port2grp_list;
    /* Delete all the previous port to group mappings before adding this new entry */
    for (tmp = list->used, prev = NULL; tmp != NULL;) {
        modified_entry = FALSE;
        for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
            if (port_bf[i] & tmp->conf.ports[i]) {
                modified_entry = TRUE;
                /* Remove the ports */
                tmp->conf.ports[i] &= ~(port_bf[i]);
            } /* if (port_bf[i] & tmp->conf.ports[i]) */
        } /* for (i = 0; i < VTSS_PORT_BF_SIZE; i++) */
        /* As a result of port deletes, if an entry exists with no ports, delete the entry */
        if (modified_entry == TRUE) {
            for (i = 0, j = 0; i < VTSS_PORT_BF_SIZE; i++) {
                if (tmp->conf.ports[i] == 0) {
                    j++;
                }
            }
            if (j == VTSS_PORT_BF_SIZE) { /* None of the ports is valid, so delete the entry */
                if (prev == NULL) { /* Delete the first node */
                    /* Remove from the used list */
                    list->used = tmp->next;
                } else {
                    /* Remove from the used list */
                    prev->next = tmp->next;
                }
                /* Add to the free list */
                tmp->next = list->free;
                list->free = tmp;
                /* Update tmp to continue the loop */
                if (prev == NULL) { /* This is the first node in used list */
                    tmp = list->used;
                    continue;
                } else {
                    tmp = prev;
                } /* if (prev == NULL) */
            } /* if (j == VTSS_PORT_BF_SIZE */
        } /* if (modified_entry == TRUE */
        prev = tmp;
        tmp = tmp->next;
    } /* for (tmp = list->used, */
    /* Insert the new node into the used list */
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        if (group_id == tmp->conf.group_id) {
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                tmp->conf.ports[i] |= port_bf[i];
            }
            update_entry = TRUE;
            break;
        }
        /* List needs to be sorted based on group_id */
        if (group_id > tmp->conf.group_id) {
            break;
        }
    }
    if (update_entry == FALSE) {
        /* Get free node from free list */
        new = list->free;
        if (new == NULL) {  /* No free entry exists */
            vtss_vlan_trans_crit_data_unlock();
            return VTSS_VT_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        new->conf.group_id = group_id;
        memcpy(new->conf.ports, port_bf, VTSS_PORT_BF_SIZE);
        new->next = NULL;
        /* Port to Group mapping can be added in to the used list in three ways:
           1. Add at the beginning of the list;
           2. Add somewhere in the middle of the list;
           3. Add at the end of the list.
         */
        if (tmp != NULL) { /* This means insertion point found */
            if (prev == NULL) {
                /* Add at the beginning of the list */
                new->next = list->used;
                list->used = new;
            } else {
                /* Add somewhere in the middle of the list */
                prev->next = new;
                new->next = tmp;
            }
        } else { /* insertion point not found; add at the end of the list */
            if (prev == NULL) { /* used list is empty */
                list->used = new;
            } else {
                prev->next = new;
            }
        }
    }
    /* VT-TODO: At this point, if there is already a group to VLAN mapping for this group,
       we need to add h/w entries */
    vtss_vlan_trans_crit_data_unlock();
    return VTSS_VT_RC_OK;
}
u32 vlan_trans_port2grp_entry_get(vlan_trans_port2grp_conf_t *conf, BOOL next)
{
    vlan_trans_port2grp_list_t          *list;
    vlan_trans_port2grp_list_entry_t    *tmp;
    BOOL                                use_next = FALSE;
    /* Check for valid group_id */
    if ((VT_VALID_GROUP_CHECK(conf->group_id) == FALSE) && (conf->group_id != VT_NULL_GROUP_ID)) {
        return VTSS_VT_ERROR_PARM;
    }
    vtss_vlan_trans_crit_data_lock();
    list = &vt_glb_data.port2grp_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        /* If group_id is zero, get first entry */
        if (conf->group_id == VT_NULL_GROUP_ID) {
            break;
        } else {
            if (use_next) {
                break;
            }
            /* Group Id match check */
            if (conf->group_id == tmp->conf.group_id) {
                if (next) { /* Return the next entry */
                    use_next = TRUE;
                } else {
                    break;
                } /* if (next) */
            } /* if ((conf->group_id == tmp->conf.group_id */
        } /* if (conf->group_id == VT_NULL_GROUP_ID */
    } /* for (tmp = list->used; */
    if (tmp != NULL) { /* Entry exists, return the configuration */
        *conf = tmp->conf;
    }
    vtss_vlan_trans_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VT_RC_OK : VTSS_VT_ERROR_ENTRY_NOT_FOUND);
}
/************** End *************************************************************************************************/
