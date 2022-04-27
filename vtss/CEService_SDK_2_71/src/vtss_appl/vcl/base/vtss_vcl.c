/*
   Vitesse VCL software.

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
#include <vtss_vcl.h>
#include <msg_api.h>        /* Message API definitions */

#if defined(VTSS_SW_OPTION_DOT1X)
#define VOLATILE_VLAN_USER_PRESENT  1
#endif
/*lint -sem( vtss_vcl_crit_data_lock, thread_lock ) */
/*lint -sem( vtss_vcl_crit_data_unlock, thread_unlock ) */
/**
 *  VCL Global Data
 **/
vcl_global_data_t   vcl_glb_data;

void vcl_ports2_port_bitfield(BOOL *ports, u8 *ports_bf)
{
    u32 port_num;

    memset(ports_bf, 0, VTSS_PORT_BF_SIZE);
    for (port_num = 0; port_num < VTSS_PORT_ARRAY_SIZE; port_num++) {
        if (ports[port_num] == TRUE) {
            ports_bf[port_num / 8] |= (1 << (port_num % 8));
        }
    }
}

/**
 * Set the data structures to default values
 **/
void vcl_default_set(void)
{
    vcl_proto_list_t                        *list;
    vcl_proto_t                             *proto_p;
    vcl_proto_hw_conf_list_t                *hw_list;
    vcl_proto_hw_conf_t                     *tmp;
    vcl_proto_local_list_t                  *local_list;
    vcl_proto_local_conf_t                  *conf;
    vcl_proto_grp_list_t                    *grp_list;
    vcl_proto_grp_t                         *grp;
    vcl_ip_local_list_t                     *ip_local_list;
    vcl_ip_local_conf_t                     *iconf;
    vcl_ip_t                                *ip_p;
    vcl_ip_list_t                           *ip_list;
    u32                                     i;

    memset(&vcl_glb_data, 0, sizeof(vcl_global_data_t));
    /* Initialize Protocol list */
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    list->used = NULL;
    list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_MAX_PROTOCOLS; i++) {
        proto_p = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_db[i];
        proto_p->next = list->free;
        list->free = proto_p;
    }
    /* Initialize VLAN list */
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    hw_list->used = NULL;
    hw_list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_MAX_HW_ENTRIES; i++) {
        tmp = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_db[i];
        tmp->next = hw_list->free;
        hw_list->free = tmp;
    }
    /* Initialize Local switch list */
    local_list = &vcl_glb_data.vcl_proto_vlan_glb_data.local_list;
    local_list->used = NULL;
    local_list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_MAX_PROTOCOLS; i++) {
        conf = &vcl_glb_data.vcl_proto_vlan_glb_data.local_db[i];
        conf->next = local_list->free;
        local_list->free = conf;
    }
    /* Initialize Group list */
    grp_list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    grp_list->used = NULL;
    grp_list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_TOTAL_GROUPS; i++) {
        grp = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_db[i];
        grp->next = grp_list->free;
        grp_list->free = grp;
    }
    /* Initialize IP Local switch list */
    ip_local_list = &vcl_glb_data.vcl_ip_vlan_glb_data.local_list;
    ip_local_list->used = NULL;
    ip_local_list->free = NULL;
    for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) {
        iconf = &vcl_glb_data.vcl_ip_vlan_glb_data.local_db[i];
        iconf->next = ip_local_list->free;
        ip_local_list->free = iconf;
    }
    /* Initialize IP subnet-based VLAN list */
    ip_list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    ip_list->used = NULL;
    ip_list->free = NULL;
    for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) {
        ip_p = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_db[i];
        ip_p->next = ip_list->free;
        ip_list->free = ip_p;
    }
}

/**
 * Set the MAC-based VLAN data structures to default values
 **/
void vcl_mac_vlan_default_set(void)
{
    vtss_vcl_crit_data_lock();
    memset(&vcl_glb_data.vcl_mac_vlan_glb_data, 0, sizeof(vcl_mac_vlan_global_t));
    vtss_vcl_crit_data_unlock();
}

/**
 * Set the Proto-based VLAN protocol list to default values
 **/
void vcl_proto_vlan_proto_default_set(void)
{
    vcl_proto_list_t *list;
    vcl_proto_t      *proto_p;
    u32              i;

    vtss_vcl_crit_data_lock();
    /* Initialize Protocol list */
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    list->used = NULL;
    list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_MAX_PROTOCOLS; i++) {
        proto_p = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_db[i];
        proto_p->next = list->free;
        list->free = proto_p;
    }
    vtss_vcl_crit_data_unlock();
}

/**
 * Set the Proto-based VLAN Local list and VLAN list to default values
 **/
void vcl_proto_vlan_vlan_default_set(void)
{
    vcl_proto_hw_conf_list_t                *hw_list;
    vcl_proto_hw_conf_t                     *tmp;
    vcl_proto_grp_list_t                    *grp_list;
    vcl_proto_grp_t                         *grp;
    u32                                     i;

    vtss_vcl_crit_data_lock();
    /* Initialize VLAN list */
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    hw_list->used = NULL;
    hw_list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_MAX_HW_ENTRIES; i++) {
        tmp = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_db[i];
        tmp->next = hw_list->free;
        hw_list->free = tmp;
    }
    /* Initialize Group list */
    grp_list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    grp_list->used = NULL;
    grp_list->free = NULL;
    for (i = 0; i < VCL_PROTO_VLAN_TOTAL_GROUPS; i++) {
        grp = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_db[i];
        grp->next = grp_list->free;
        grp_list->free = grp;
    }
    vtss_vcl_crit_data_unlock();
}

/**
 * Set the IP subnet-based VLAN data structures to default values
 **/
void vcl_ip_vlan_default_set(void)
{
    vcl_ip_t             *ip_p;
    vcl_ip_list_t        *list;
    u32                  i;

    vtss_vcl_crit_data_lock();
    /* Initialize IP subnet-based VLAN list */
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    list->used = NULL;
    list->free = NULL;
    for (i = 0; i < VCL_IP_VLAN_MAX_ENTRIES; i++) {
        ip_p = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_db[i];
        ip_p->next = list->free;
        list->free = ip_p;
    }
    vtss_vcl_crit_data_unlock();
}


/**
 * This function gets the conflicting users configured. This function should also be called while adding or deleting
 * MAC-based VLAN entry to resolve conflicts
 **/
u32 vcl_mac_vlan_conflict_resolver(u32 indx,
                                   vcl_mac_vlan_conflicting_users *conflicting_users,
                                   BOOL get_conflicts)
{
    vcl_mac_vlan_user_t     user = VCL_MAC_VLAN_USER_STATIC;
    vcl_mac_vlan_entry_t    *entry;
#ifdef  VOLATILE_VLAN_USER_PRESENT
    vcl_mac_vlan_user_t     user_temp;
    BOOL                    volatile_user_present = FALSE;
#endif

    if (conflicting_users == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (indx >= VCL_MAC_VLAN_MAX_ENTRIES) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* The call to this function should have taken the semaphore already */
    vtss_vcl_crit_data_assert_locked();
    entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[indx];
    /* If there are no volatile users, no conflicts */
    if (user == (VCL_MAC_VLAN_USER_ALL - 1)) {
        *conflicting_users = 0;
        if (entry->conf[VCL_MAC_VLAN_USER_STATIC].valid == TRUE) {
            entry->conf[VCL_MAC_VLAN_USER_ALL] = entry->conf[VCL_MAC_VLAN_USER_STATIC];
        } else { /* No VLAN user is configured for this entry */
            entry->conf[VCL_MAC_VLAN_USER_ALL].valid = FALSE;
        }
        return VTSS_VCL_RC_OK;
    }
#ifdef  VOLATILE_VLAN_USER_PRESENT
    for (user = (VCL_MAC_VLAN_USER_STATIC + 1); user < VCL_MAC_VLAN_USER_ALL; user++) {
        if (entry->conf[user].valid == TRUE) {
            volatile_user_present = TRUE;
            if (get_conflicts == FALSE) {
                entry->conf[VCL_MAC_VLAN_USER_ALL] = entry->conf[user];
            }
            break;
        }
    }
    *conflicting_users = 0;
    if (volatile_user_present == TRUE) {
        for (user_temp = user; (user_temp < VCL_MAC_VLAN_USER_ALL); user_temp++) {
            *conflicting_users |= (entry->conf[user_temp].valid == TRUE) ? (1 << user_temp) : 0;
        }
        *conflicting_users = (entry->conf[VCL_MAC_VLAN_USER_STATIC].valid == TRUE) ? (1 << VCL_MAC_VLAN_USER_STATIC) : 0;
    }

    /**
     * If not volatile VLAN user is configured, apply the static configuration if present.
     **/
    if ((volatile_user_present == FALSE) && (get_conflicts == FALSE)) {
        if (entry->conf[VCL_MAC_VLAN_USER_STATIC].valid == TRUE) {
            entry->conf[VCL_MAC_VLAN_USER_ALL] = entry->conf[VCL_MAC_VLAN_USER_STATIC];
        } else { /* No VLAN user is configured for this entry */
            entry->conf[VCL_MAC_VLAN_USER_ALL].valid = FALSE;
        }
    }
#endif
    return VTSS_VCL_RC_OK;
}

u32 vcl_mac_vlan_entry_add(vcl_mac_vlan_conf_entry_t *mac_vlan_entry,
                           vtss_vce_id_t *id,
                           vcl_mac_vlan_user_t user)
{
    vcl_mac_vlan_conflicting_users  users;
    vcl_mac_vlan_entry_t            *entry;
    BOOL                            found = FALSE;
    u32                             i, indx, first_free_index = VCL_MAC_VLAN_INVALID_INDEX;
    u32                             rc = VTSS_VCL_RC_OK;

    do {
        /**
         * Check the pointer for NULL
         **/
        if (mac_vlan_entry == NULL) {
            rc = VTSS_VCL_ERROR_PARM;
            break;  /* break from do-while loop */
        }
        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            rc = VTSS_VCL_ERROR_PARM;    /* break from do-while loop */
            break;
        }

        vtss_vcl_crit_data_lock();
        /**
         *  Loop through all MAC-based VLAN entries in the array to see if this entry already exists
         **/
        for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
            entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[indx];
            if (entry->conf[VCL_MAC_VLAN_USER_ALL].valid == TRUE) {
                /**
                 *  Check to see if the entry already exists
                 **/
                if (!memcmp(&mac_vlan_entry->smac, &entry->smac, sizeof(mac_vlan_entry->smac))) {
                    first_free_index = indx;
                    break;  /* break from for loop */
                }
            } else if (found == FALSE) { /* Get the first index that is free */
                first_free_index = indx;
                found = TRUE;
            }
        }
        /**
         *  Copy the Mgmt entry to the first valid index
         **/
        if (first_free_index != VCL_MAC_VLAN_INVALID_INDEX) {
            *id = first_free_index + 1;
            entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[first_free_index];
            if (entry->conf[user].valid == FALSE) {
                /**
                 * Clear the memory before initializing the entry
                 **/
                memset(&entry->conf[user], 0, sizeof(vcl_mac_vlan_conf_t));
                entry->smac = mac_vlan_entry->smac;
                entry->conf[user].vid = mac_vlan_entry->vid;
            } else if (entry->conf[user].vid != mac_vlan_entry->vid) {
                rc = VTSS_VCL_ERROR_ENTRY_WITH_DIFF_VLAN;    /* break from do-while loop */
                vtss_vcl_crit_data_unlock();
                break;
            }
            /**
             * If different vid is configured from previous vid, it is not allowed. Only allow different ports
             * to be configured.
             **/
            for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
                entry->conf[user].l2ports[i] |= mac_vlan_entry->l2ports[i];
            }
            entry->conf[user].valid = TRUE;
            /**
             * MAC-based VLAN Conflict resolver to copy the user information to combined info
             * after conflicts are resolved.
             **/
            rc = vcl_mac_vlan_conflict_resolver(first_free_index, &users, FALSE);
            /**
             * Copy the effective configuration back to set in the hardware. Here we don't have to check valid
             * flag as at least one user configured this entry.
             **/
            mac_vlan_entry->vid = entry->conf[VCL_MAC_VLAN_USER_ALL].vid;
            memcpy(mac_vlan_entry->l2ports, entry->conf[VCL_MAC_VLAN_USER_ALL].l2ports, VTSS_L2PORT_BF_SIZE);
        } else {
            /**
             *  Control reaches here if the table is full
             **/
            rc = VTSS_VCL_ERROR_TABLE_FULL;
        }
        vtss_vcl_crit_data_unlock();
    } while (0); /* End of do-while loop */

    return rc;
}

u32 vcl_mac_vlan_entry_del(vcl_mac_vlan_conf_entry_t *mac_vlan_entry,
                           vtss_vce_id_t *id,
                           vcl_mac_vlan_user_t user)
{
    vcl_mac_vlan_conflicting_users  users;
    u32                             indx, i;
    BOOL                            entry_found = FALSE, ports_exist = FALSE;
    u32                             rc = VTSS_VCL_RC_OK;
    vcl_mac_vlan_entry_t            *entry;

    do {
        /**
         * Check the pointer for NULL
         **/
        if (mac_vlan_entry == NULL) {
            rc = VTSS_VCL_ERROR_PARM;
            break;  /* break from the do-while loop */
        }
        /**
         * Check for the valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            rc = VTSS_VCL_ERROR_PARM;
            break;                       /* break from the do-while loop */
        }
        vtss_vcl_crit_data_lock();
        for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
            entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[indx];
            if (entry->conf[user].valid == TRUE) {
                if (!memcmp(&mac_vlan_entry->smac, &entry->smac, sizeof(vtss_mac_t))) {
                    /* TODO: May not be required to delete always as user's conf may only need to be deleted */
                    *id = indx + 1;
                    /**
                     * Entry exists
                     **/
                    entry_found = TRUE;
                    for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
                        entry->conf[user].l2ports[i] &= (~mac_vlan_entry->l2ports[i]);
                        if (entry->conf[user].l2ports[i]) {
                            ports_exist = TRUE;
                        }
                    }
                    if (ports_exist == FALSE) {
                        entry->conf[user].valid = FALSE;
                    }
                    rc = vcl_mac_vlan_conflict_resolver(indx, &users, FALSE);
                    break;  /* break from the for loop */
                }
            }
        }
        vtss_vcl_crit_data_unlock();

        if (entry_found == FALSE) {
            rc = VTSS_VCL_ERROR_ENTRY_NOT_FOUND;
        }
    } while (0);  /* End of do-while loop */

    return rc;
}

u32 vcl_mac_vlan_entry_get(vtss_mac_t *smac,
                           vcl_mac_vlan_user_t user,
                           BOOL next,
                           BOOL first,
                           vcl_mac_vlan_conf_entry_t *mac_vlan_entry)
{
    vcl_mac_vlan_entry_t            *entry;
    BOOL                            found = FALSE;
    u32                             indx, count = 0;
    /**
     * Check for NULL pointer
     **/
    if (smac == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /**
     * Check for valid MAC-based VLAN User
     **/
    if ((user < VCL_MAC_VLAN_USER_STATIC) || (user > VCL_MAC_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    /**
     *  Loop through all MAC-based VLAN entries in the array to see if this entry exists
     **/
    for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
        entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[indx];
        if (entry->conf[user].valid == TRUE) {
            /**
             *  Check to see if the entry exists
             **/
            if ((!memcmp(smac, &entry->smac, sizeof(mac_vlan_entry->smac))) || (count == 1) || (first == 1)) {
                count++;
                if ((next == FALSE) || ((next == TRUE) && (count == 2))) {
                    found = TRUE;
                    mac_vlan_entry->smac = entry->smac;
                    mac_vlan_entry->vid = entry->conf[user].vid;
                    memcpy(mac_vlan_entry->l2ports, entry->conf[user].l2ports, VTSS_L2PORT_BF_SIZE);
                    break;
                } /* if ((next == FALSE) */
            } /* if ((!memcmp(smac,  */
        } /* if (entry->conf[user].valid == TRUE */
    } /* for (indx = 0 */
    vtss_vcl_crit_data_unlock();
    if (found == FALSE) {
        return VTSS_VCL_ERROR_ENTRY_NOT_FOUND;
    }
    return VTSS_VCL_RC_OK;
}

u32 vcl_mac_vlan_entry_get_by_key(u32 indx, vcl_mac_vlan_user_t user,
                                  vcl_mac_vlan_conf_entry_t *mac_vlan_entry)
{
    vcl_mac_vlan_entry_t   *entry;
    u32                    rc = VTSS_VCL_RC_OK;

    do {
        /**
         * Check for NULL pointer
         **/
        if (mac_vlan_entry == NULL) {
            return VTSS_VCL_ERROR_PARM;
        }
        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            rc = VTSS_VCL_ERROR_PARM;
            break;  /* break from do-while */
        }
        /**
         * validate index
         **/
        if (indx >= VCL_MAC_VLAN_MAX_ENTRIES) {
            rc = VTSS_VCL_ERROR_PARM;
            break;  /* break from do-while */
        }
        vtss_vcl_crit_data_lock();
        entry = &vcl_glb_data.vcl_mac_vlan_glb_data.mac_vlan_db[indx];
        if (entry->conf[user].valid == TRUE) {
            mac_vlan_entry->smac = entry->smac;
            mac_vlan_entry->vid = entry->conf[user].vid;
            memcpy(mac_vlan_entry->l2ports, entry->conf[user].l2ports, VTSS_L2PORT_BF_SIZE);
        } else {
            rc = VTSS_VCL_ERROR_ENTRY_NOT_FOUND;
        }
        vtss_vcl_crit_data_unlock();
    } while (0);    /* end of do-while */

    return rc;
}

u32 vcl_mac_vlan_local_entry_add(vcl_mac_vlan_local_sid_conf_entry_t *entry)
{
    vcl_mac_vlan_local_sid_conf_t       *loc_entry;
    BOOL                                found = FALSE;
    u32                                 indx, first_free_index = VCL_MAC_VLAN_INVALID_INDEX;
    u32                                 rc = VTSS_VCL_RC_OK;

    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }

    vtss_vcl_crit_data_lock();
    /**
     *  Loop through all MAC-based VLAN entries in the array to see if this entry already exists
     **/
    for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
        loc_entry = &vcl_glb_data.vcl_mac_vlan_loc_data[indx];
        if (loc_entry->valid == TRUE) {
            /**
             *  Check to see if the entry already exists
             **/
            if (!memcmp(&entry->mac, &loc_entry->entry.mac, sizeof(vtss_mac_t))) {
                first_free_index = indx;
                break;  /* break from for loop */
            }
        } else if (found == FALSE) { /* Get the first index that is free */
            first_free_index = indx;
            found = TRUE;
        }
    }
    /**
     *  Copy the entry to the first valid index
     **/
    if (first_free_index != VCL_MAC_VLAN_INVALID_INDEX) {
        loc_entry = &vcl_glb_data.vcl_mac_vlan_loc_data[first_free_index];
        if (loc_entry->valid == FALSE) {
            /**
             * Clear the memory before initializing the entry
             **/
            memset(loc_entry, 0, sizeof(vcl_mac_vlan_local_sid_conf_entry_t));
            loc_entry->entry.mac = entry->mac;
        }
        /**
         * If different vid is configured from previous vid, it will be overwritten
         **/
        loc_entry->entry.vid = entry->vid;
        memcpy(loc_entry->entry.ports, entry->ports, VTSS_PORT_BF_SIZE);
        loc_entry->entry.id = entry->id;
        loc_entry->valid = TRUE;
    } else {
        /**
         *  Control reaches here if the table is full
         **/
        rc = VTSS_VCL_ERROR_TABLE_FULL;
    }
    vtss_vcl_crit_data_unlock();

    return rc;

}

u32 vcl_mac_vlan_local_entry_del(vtss_mac_t *mac)
{
    u32                             indx;
    BOOL                            entry_found = FALSE;
    u32                             rc = VTSS_VCL_RC_OK;
    vcl_mac_vlan_local_sid_conf_t   *loc_entry;

    /** Check for NULL pointer */
    if (mac == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }

    vtss_vcl_crit_data_lock();
    for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
        loc_entry = &vcl_glb_data.vcl_mac_vlan_loc_data[indx];
        if (loc_entry->valid == TRUE) {
            if (!memcmp(&loc_entry->entry.mac, mac, sizeof(vtss_mac_t))) {
                /**
                 * Entry exists
                 **/
                loc_entry->valid = FALSE;
                entry_found = TRUE;
                break;  /* break from the for loop */
            }
        }
    }
    vtss_vcl_crit_data_unlock();

    if (entry_found == FALSE) {
        rc = VTSS_VCL_ERROR_ENTRY_NOT_FOUND;
    }

    return rc;
}

u32 vcl_mac_vlan_local_entry_get(vcl_mac_vlan_local_sid_conf_entry_t *entry,
                                 BOOL next,
                                 BOOL first)
{
    vcl_mac_vlan_local_sid_conf_t   *loc_entry;
    BOOL                            found = FALSE;
    u32                             indx, count = 0;
    /**
     * Check for NULL pointer
     **/
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    /**
     *  Loop through all MAC-based VLAN entries in the array to see if this entry exists
     **/
    for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
        loc_entry = &vcl_glb_data.vcl_mac_vlan_loc_data[indx];
        if (loc_entry->valid == TRUE) {
            /**
             *  Check to see if the entry exists
             **/
            if ((!memcmp(&entry->mac, &loc_entry->entry.mac, sizeof(vtss_mac_t))) || (count == 1) || (first == 1)) {
                count++;
                if ((next == FALSE) || ((next == TRUE) && (count == 2))) {
                    found = TRUE;
                    memcpy(entry, &loc_entry->entry, sizeof(vcl_mac_vlan_local_sid_conf_entry_t));
                    break;
                } /* if ((next == FALSE) */
            } /* if ((!memcmp(  */
        } /* if (entry->valid == TRUE */
    } /* for (indx = 0 */
    vtss_vcl_crit_data_unlock();
    if (found == FALSE) {
        return VTSS_VCL_ERROR_ENTRY_NOT_FOUND;
    }
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_local_entry_add(vcl_proto_vlan_local_sid_conf_t *entry)
{
    vcl_proto_local_list_t      *list;
    vcl_proto_local_conf_t      *new = NULL, *tmp;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.local_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        if (entry->id == tmp->conf.id) {
            /* Overwrite the port list */
            memcpy(tmp->conf.ports, entry->ports, VTSS_PORT_BF_SIZE);
            break;
        }
    }
    if (tmp == NULL) {
        /* Get free node from free list */
        new = list->free;
        /* 'new' can not be NULL as free never be NULL */
        if (new == NULL) {
            vtss_vcl_crit_data_unlock();
            return VTSS_VCL_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        new->conf = *entry;
        /* Update the used list */
        new->next = list->used;
        list->used = new;
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_local_entry_delete(u32 id)
{
    vcl_proto_local_list_t      *list;
    vcl_proto_local_conf_t      *tmp = NULL, *prev;

    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.local_list;
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        if (id == tmp->conf.id) {
            break;
        }
    }
    if (tmp != NULL) {
        /* Move entry from used list to free list */
        if (prev == NULL) {
            list->used = tmp->next;
        } else {
            prev->next = tmp->next;
        }
        /* Move entry from used list to free list */
        tmp->next = list->free;
        list->free = tmp;
    }
    vtss_vcl_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VCL_RC_OK : VTSS_VCL_ERROR_ENTRY_NOT_FOUND);
}

u32 vcl_proto_vlan_local_entry_get(vcl_proto_vlan_local_sid_conf_t *entry, u32 id, BOOL next)
{
    vcl_proto_local_list_t      *list;
    vcl_proto_local_conf_t      *tmp = NULL;
    BOOL                        use_next = FALSE;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.local_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        if (id == 0) {
            break;
        } else {
            if (use_next) {
                break;
            }
            if (id == tmp->conf.id) {
                if (next) {
                    use_next = 1;
                } else {
                    break;
                }
            }
        }
    }
    if (tmp != NULL) {
        *entry = tmp->conf;
    }
    vtss_vcl_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VCL_RC_OK : VTSS_VCL_ERROR_ENTRY_NOT_FOUND);
}

/* Last local entry is the first entry that was added. */
void vcl_proto_vlan_first_vce_id_get(vtss_vce_id_t *id)
{
    vcl_proto_local_list_t      *list;
    vcl_proto_local_conf_t      *tmp = NULL;
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.local_list;
    /* If no entry present return VCE_ID_NONE */
    if (list->used == NULL) {
        *id = VCE_ID_NONE;
    } else {
        tmp = list->used;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        *id = tmp->conf.id;
    }
    vtss_vcl_crit_data_unlock();
    return;
}

#if 0
static void vcl_user_first_vce_id_get(vtss_vce_id_t *id, vcl_user_t user)
{
    if (user == VCL_TYPE_PROTO) {
        vcl_proto_vlan_first_vce_id_get(id);
    }
}
#endif

void vcl_proto_vlan_vce_id_get(u32 *vce_id)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *tmp;
    u16                         id_used[VCE_ID_END];
    u32                         i;

    memset(id_used, 0, sizeof(id_used));
    vtss_vcl_crit_data_assert_locked();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    for (tmp = hw_list->used; tmp != NULL; tmp = tmp->next) {
        id_used[tmp->conf.vce_id - VCE_ID_START] = 1;
    }
    for (i = VCE_ID_START; i <= VCE_ID_END; i++) {
        if (!id_used[i - VCE_ID_START]) {
            *vce_id =  i;
            break;
        }
    }
}

static u32 vcl_proto_vlan_hw_entry_delete(vcl_proto_vlan_entry_t *entry)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *tmp, *prev;
    BOOL                        proto_exists = FALSE, modified_entry = FALSE, node_deleted = FALSE;
    u32                         i, j;
    u8                          ports[VTSS_PORT_BF_SIZE];

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_assert_locked();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    for (tmp = hw_list->used, prev = NULL; tmp != NULL;) {
        node_deleted = FALSE;
        if ((entry->user == tmp->conf.user) && (entry->isid == tmp->conf.isid)) {
            if (tmp->conf.proto_encap_type == entry->proto_encap_type) {
                if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                    (entry->proto.eth2_proto.eth_type == tmp->conf.proto.eth2_proto.eth_type)) {
                    proto_exists = TRUE;
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                    if (!memcmp(entry->proto.llc_snap_proto.oui, tmp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                        && (entry->proto.llc_snap_proto.pid == tmp->conf.proto.llc_snap_proto.pid)) {
                        proto_exists = TRUE;
                    }
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                    if ((entry->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap)
                        && (entry->proto.llc_other_proto.ssap == tmp->conf.proto.llc_other_proto.ssap)) {
                        proto_exists = TRUE;
                    } /* if ((entry->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap) */
                } /* if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) */
            } /* if (tmp->conf.proto_encap_type == entry->proto_encap_type) */
        } /* if ((entry->user == tmp->conf.user) && (entry->isid == tmp->conf.isid))  */
        if (proto_exists == TRUE) {
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                if (entry->ports[i] & tmp->conf.ports[i]) {
                    modified_entry = TRUE;
                    /* Remove the ports */
                    tmp->conf.ports[i] &= ~(entry->ports[i]);
                } /* if (ports[i] & tmp->conf.ports[i]) */
            } /* for (i = 0; i < VTSS_PORT_BF_SIZE; i++) */
            if (modified_entry == TRUE) {
                for (i = 0, j = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    if (tmp->conf.ports[i] == 0) {
                        j++;
                    }
                }
                entry->vce_id = tmp->conf.vce_id;
                entry->vid = tmp->conf.vid;
                memcpy(ports, entry->ports, VTSS_PORT_BF_SIZE);
                memcpy(entry->ports, tmp->conf.ports, VTSS_PORT_BF_SIZE);
                if (j == VTSS_PORT_BF_SIZE) {
                    if (vcl_stack_vcl_proto_vlan_conf_add_del(entry->isid, entry, FALSE) != VTSS_RC_OK) {
                        return VTSS_VCL_ERROR_STACK_STATE;
                    }
                    /* Move entry from used list to free list */
                    if (prev == NULL) {
                        hw_list->used = tmp->next;
                    } else {
                        prev->next = tmp->next;
                    }
                    /* Move entry from used list to free list */
                    tmp->next = hw_list->free;
                    hw_list->free = tmp;
                    /* Point to previous node to continue the list traversal */
                    tmp = prev;
                    node_deleted = TRUE;
                } else {
                    /* Some of the ports are modified, hence add the HW entry with modified ports */
                    if (vcl_stack_vcl_proto_vlan_conf_add_del(entry->isid, entry, TRUE) != VTSS_RC_OK) {
                        return VTSS_VCL_ERROR_STACK_STATE;
                    }
                } /* if (j == VTSS_PORT_BF_SIZE) */
                memcpy(entry->ports, ports, VTSS_PORT_BF_SIZE);
            } /* if (modified_entry == TRUE) */
        } /* if (proto_exists == TRUE) */
        /* First node deletion case */
        if ((tmp == NULL) && (node_deleted == TRUE)) {
            tmp = hw_list->used;
            prev = NULL;
        } else {
            prev = tmp;
            /* Warning -- Possible use of null pointer 'tmp' in left argument to operator '->'.
               This is not possible as tmp will never be NULL when node_deleted = FALSE */
            /*lint -e{613} */
            tmp = tmp->next;
        }
    } /* for (tmp = hw_list->used, prev = NULL; tmp != NULL;) */
    return VTSS_VCL_RC_OK;
}

static u32 vcl_proto_vlan_group_name_check(u8 *grp_name)
{
    uint    idx;
    BOOL    error = FALSE;

    for (idx = 0; idx < strlen((char *)grp_name); idx++) {
        if ((grp_name[idx] < 48) || (grp_name[idx] > 122)) {
            error = TRUE;
        } else {
            if ((grp_name[idx] > 57) && (grp_name[idx] < 65)) {
                error = TRUE;
            } else if ((grp_name[idx] > 90) && (grp_name[idx] < 97)) {
                error = TRUE;
            }
        }
    }
    if (error == TRUE) {
        return VTSS_VCL_ERROR_PARM;
    }
    return VTSS_VCL_RC_OK;
}

/* Description: Adds protocol to protocol list. If the protocol is already mapped to different group,
                return error. If not, add the entry to the protocol list. Check the group to see if
                present in the group_vlan list. If present, add corresponding entries in the HW list
                and send the stack message.
 */
u32 vcl_proto_vlan_proto_entry_add(vcl_proto_vlan_proto_entry_t *entry,
                                   vcl_proto_vlan_user_t        user)
{
    vcl_proto_list_t            *list;
    vcl_proto_t                 *grp, *new = NULL;
    vcl_proto_grp_list_t        *vlan_list;
    vcl_proto_grp_t             *grp_vlan;
    vcl_proto_vlan_entry_t      conf;
    u32                         vce_id;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid MAC-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check group name - it should only contain alphabets or digits */
    if ((vcl_proto_vlan_group_name_check(entry->group_id)) != VTSS_VCL_RC_OK) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    /* Search for existing entry */
    for (grp = list->used; grp != NULL; grp = grp->next) {
        if (grp->conf.proto_encap_type == entry->proto_encap_type) {
            if ((grp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                (entry->proto.eth2_proto.eth_type == grp->conf.proto.eth2_proto.eth_type)) {
                vtss_vcl_crit_data_unlock();
                return VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                if (!memcmp(entry->proto.llc_snap_proto.oui, grp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                    && (entry->proto.llc_snap_proto.pid == grp->conf.proto.llc_snap_proto.pid)) {
                    vtss_vcl_crit_data_unlock();
                    return VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
                }
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                if ((entry->proto.llc_other_proto.dsap == grp->conf.proto.llc_other_proto.dsap)
                    && (entry->proto.llc_other_proto.ssap == grp->conf.proto.llc_other_proto.ssap)) {
                    vtss_vcl_crit_data_unlock();
                    return VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
                }
            }
        }
    }
    /* Get free node from free list */
    new = list->free;
    /* 'new' can not be NULL as free never be NULL */
    if (new == NULL) {
        vtss_vcl_crit_data_unlock();
        return VTSS_VCL_ERROR_TABLE_FULL;
    }
    /* Update the free list */
    list->free = new->next;
    /* Copy the configuration */
    new->conf = *entry;
    /* Update the used list */
    new->next = list->used;
    list->used = new;
    /* Check in the Group-VLAN list to see if there is any entry already configured for this group */
    vlan_list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    /* Search for existing entry */
    for (grp_vlan = vlan_list->used; grp_vlan != NULL; grp_vlan = grp_vlan->next) {
        if (!memcmp(grp_vlan->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) {
            conf.proto_encap_type = entry->proto_encap_type;
            conf.proto = entry->proto;
            conf.vid = grp_vlan->conf.vid;
            memcpy(conf.ports, grp_vlan->conf.ports, VTSS_PORT_BF_SIZE);
            conf.user = user;
            conf.isid = grp_vlan->conf.isid;
            /* VCL_TODO : This can be optimized */
            vcl_proto_vlan_vce_id_get(&vce_id);
            conf.vce_id = vce_id;
            if (vcl_proto_vlan_hw_entry_add(&conf) != VTSS_VCL_RC_OK) {
            }
        }
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_proto_entry_delete(vcl_proto_encap_type_t  proto_encap_type,
                                      vcl_proto_conf_t        *proto,
                                      vcl_proto_vlan_user_t   user)
{
    vcl_proto_list_t            *list;
    vcl_proto_t                 *grp, *prev;

    /* Check for NULL pointer */
    if (proto == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid MAC-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid encap type */
    if (!((proto_encap_type == VCL_PROTO_ENCAP_ETH2) || (proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) ||
          (proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER))) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    /* Search for existing entry */
    for (grp = list->used, prev = NULL; grp != NULL; prev = grp, grp = grp->next) {
        if (grp->conf.proto_encap_type == proto_encap_type) {
            if ((grp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                (proto->eth2_proto.eth_type == grp->conf.proto.eth2_proto.eth_type)) {
                break;
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                if (!memcmp(proto->llc_snap_proto.oui, grp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                    && (proto->llc_snap_proto.pid == grp->conf.proto.llc_snap_proto.pid)) {
                    break;
                }
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                if ((proto->llc_other_proto.dsap == grp->conf.proto.llc_other_proto.dsap)
                    && (proto->llc_other_proto.ssap == grp->conf.proto.llc_other_proto.ssap)) {
                    break;
                } /* if ((proto->llc_other_proto.dsap == grp->conf.proto.llc_other_proto.dsap) */
            } /* if ((grp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) && */
        } /* if (grp->conf.proto_encap_type == proto_encap_type) */
    } /* for (grp = list->used; grp != NULL; grp = grp->next) */
    /* Delete all the entries corresponding to this protocol in the hw list */
    if (grp != NULL) {
        /* Move entry from used list to free list */
        if (prev == NULL) {
            list->used = grp->next;
        } else {
            prev->next = grp->next;
        }
        /* Move entry from used list to free list */
        grp->next = list->free;
        list->free = grp;
    }
    if (vcl_proto_vlan_hw_entry_delete_by_protocol(proto_encap_type, proto) != VTSS_VCL_RC_OK) {
    }
    vtss_vcl_crit_data_unlock();
    return ((grp != NULL) ? VTSS_VCL_RC_OK : VTSS_VCL_ERROR_ENTRY_NOT_FOUND);
}

u32 vcl_proto_vlan_proto_entry_get(vcl_proto_vlan_proto_entry_t *entry,
                                   vcl_proto_vlan_user_t        user,
                                   BOOL                         next,
                                   BOOL                         first)
{
    vcl_proto_list_t            *list;
    vcl_proto_t                 *grp;
    BOOL                        use_next = FALSE;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid MAC-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    /* Search for existing entry */
    for (grp = list->used; grp != NULL; grp = grp->next) {
        if (use_next) {
            break;
        }
        /* If first flag is set, protocol is not compared */
        if (first && !next) {
            break;
        }
        if (first && next) {
            use_next = TRUE;
            continue;
        }
        if (grp->conf.proto_encap_type == entry->proto_encap_type) {
            if ((grp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                (entry->proto.eth2_proto.eth_type == grp->conf.proto.eth2_proto.eth_type)) {
                if (next) {
                    use_next = TRUE;
                } else {
                    break;
                }
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                if (!memcmp(entry->proto.llc_snap_proto.oui, grp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                    && (entry->proto.llc_snap_proto.pid == grp->conf.proto.llc_snap_proto.pid)) {
                    if (next) {
                        use_next = TRUE;
                    } else {
                        break;
                    }
                }
            } else if (grp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                if ((entry->proto.llc_other_proto.dsap == grp->conf.proto.llc_other_proto.dsap)
                    && (entry->proto.llc_other_proto.ssap == grp->conf.proto.llc_other_proto.ssap)) {
                    if (next) {
                        use_next = TRUE;
                    } else {
                        break;
                    }
                } /* if ((entry->proto.llc_other_proto.dsap == grp->conf.proto.llc_other_proto.dsap) */
            } /* if ((grp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) && */
        } /* if (grp->conf.proto_encap_type == proto_encap_type) */
    } /* for (grp = list->used; grp != NULL; grp = grp->next) */
    if (grp != NULL) {
        *entry = grp->conf;
    }
    vtss_vcl_crit_data_unlock();
    return (grp == NULL ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}

u32 vcl_proto_vlan_group_entry_add(vtss_isid_t                       isid_add,
                                   vcl_proto_vlan_vlan_entry_t       *entry,
                                   vcl_proto_vlan_user_t             user)
{
    vcl_proto_grp_list_t            *list;
    vcl_proto_grp_t                 *grp, *new;
    vcl_proto_list_t                *pr_list;
    vcl_proto_t                     *pr;
    u8                              ports[VTSS_PORT_BF_SIZE];
    u32                             i, vce_id;
    BOOL                            update = FALSE;
    vcl_proto_vlan_entry_t          conf;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid Protocol-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_add) || (isid_add == VTSS_ISID_GLOBAL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check group name - it should only contain alphabets or digits */
    if ((vcl_proto_vlan_group_name_check(entry->group_id)) != VTSS_VCL_RC_OK) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    /* If a group is already configured for a port, returns error */
    for (grp = list->used; grp != NULL; grp = grp->next) {
        if ((user == grp->conf.user) && (isid_add == grp->conf.isid)) {
            /* Check for Group match */
            if (!memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) {
                /* Check whether any port is configured for different VLAN */
                vcl_ports2_port_bitfield(entry->ports, ports);
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    if (ports[i] & grp->conf.ports[i]) {
                        vtss_vcl_crit_data_unlock();
                        return VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
                    } /* if (ports[i] & grp->conf.ports[i]) */
                } /* for (i = 0; i < VTSS_PORT_BF_SIZE; i++) */
                if (grp->conf.vid == entry->vid) {
                    /* Updating entry is sufficient */
                    for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                        grp->conf.ports[i] |= ports[i];
                    }
                    update = TRUE;
                    break;
                    /* VCL_TODO : On update ports will have all the ports that will be used later */
                    //memcpy(ports, grp->conf.ports, VTSS_PORT_BF_SIZE);
                }
            } /* !memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) */
        } /* if ((user == grp->conf.user) && (isid_add == grp->conf.isid)) */
    } /* for (grp = list->used; grp != NULL; grp = grp->next) */
    if (update == FALSE) {
        /* Get free node from free list */
        new = list->free;
        /* 'new' can not be NULL as free never be NULL */
        if (new == NULL) {
            vtss_vcl_crit_data_unlock();
            return VTSS_VCL_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        memcpy(new->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN);
        new->conf.isid = isid_add;
        new->conf.vid = entry->vid;
        new->conf.user = user;
        vcl_ports2_port_bitfield(entry->ports, new->conf.ports);
        /* Update the used list */
        new->next = list->used;
        list->used = new;
    }
    pr_list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    /* Check if any protocol is already mapped to this group. If so, add the hw entry */
    for (pr = pr_list->used; pr != NULL; pr = pr->next) {
        /* Check for Group match */
        if (!memcmp(pr->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) {
            conf.proto_encap_type = pr->conf.proto_encap_type;
            conf.proto = pr->conf.proto;
            conf.vid = entry->vid;
            /* VCL_TODO : For updated ports, this will change */
            vcl_ports2_port_bitfield(entry->ports, ports);
            memcpy(conf.ports, ports, VTSS_PORT_BF_SIZE);
            conf.user = user;
            conf.isid = isid_add;
            /* Updated entries don't require VCE_ID */
            if (update == FALSE) {
                vcl_proto_vlan_vce_id_get(&vce_id);
            }
            /* vce_id is not required for updation case; Hence suppressing the LINT warning */
            /*lint -e{644} */
            conf.vce_id = vce_id;
            if (vcl_proto_vlan_hw_entry_add(&conf) != VTSS_VCL_RC_OK) {
            }
        }
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_group_entry_delete(vtss_isid_t                       isid_del,
                                      vcl_proto_vlan_vlan_entry_t       *entry,
                                      vcl_proto_vlan_user_t             user)
{
    vcl_proto_grp_list_t            *list;
    vcl_proto_grp_t                 *grp, *prev;
    vcl_proto_list_t                *pr_list;
    vcl_proto_t                     *pr;
    u8                              ports[VTSS_PORT_BF_SIZE];
    u32                             i, j;
    BOOL                            modified_entry = FALSE, update = TRUE;
    vcl_proto_vlan_entry_t          conf;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid MAC-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_del) || (isid_del == VTSS_ISID_GLOBAL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    for (grp = list->used, prev = NULL; grp != NULL; /* prev = grp, grp = grp->next */) {
        update = TRUE;
        if (!memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) {
            if ((user == grp->conf.user) && (isid_del == grp->conf.isid)) {
                /* Check whether any port is configured for different VLAN */
                vcl_ports2_port_bitfield(entry->ports, ports);
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    if (ports[i] & grp->conf.ports[i]) {
                        modified_entry = TRUE;
                        /* Remove the ports */
                        grp->conf.ports[i] &= ~ports[i];
                    } /* if (ports[i] & tmp->conf.ports[i]) */
                } /* for (i = 0; i < VTSS_PORT_BF_SIZE; i++) */
                if (modified_entry == TRUE) {
                    for (i = 0, j = 0; i < VTSS_PORT_BF_SIZE; i++) {
                        if (grp->conf.ports[i] == 0) {
                            j++;
                        } /* if (tmp->conf.ports[i] == 0) */
                    }
                    if (j == VTSS_PORT_BF_SIZE) {
                        update = FALSE;
                        /* All the ports got deleted, delete the entry */
                        /* Move entry from used list to free list */
                        if (prev == NULL) {
                            list->used = grp->next;
                        } else {
                            prev->next = grp->next;
                        }
                        /* Move entry from used list to free list */
                        grp->next = list->free;
                        list->free = grp;
                        /* VCL_TODO : It may not be sufficient to delete one entry's port list */
                        grp = prev;
                    } /* if (j == VTSS_PORT_BF_SIZE) */
                } /* if (modified_entry == TRUE) */
            } /* if ((user == grp->conf.user) && (isid_del == grp->conf.isid)) */
        } /* if (!memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) */
        /* First node deletion case */
        /* Warning -- Possible use of null pointer 'grp' in left argument to operator '->'.
                      This is not possible as grp will never be NULL when update = FALSE */
        if ((grp == NULL) && (update == FALSE)) {
            grp = list->used;
            prev = NULL;
        } else {
            prev = grp;
            /* Warning -- Possible use of null pointer 'tmp' in left argument to operator '->'.
               This is not possible as tmp will never be NULL when node_deleted = FALSE */
            /*lint -e{613} */
            grp = grp->next;
        }
    } /* for (grp = list->used; grp != NULL; grp = grp->next) */
    /* Get group Id to protocol mapping and delete hw entries with matching proto, vid and ports */
    pr_list = &vcl_glb_data.vcl_proto_vlan_glb_data.proto_list;
    for (pr = pr_list->used; pr != NULL; pr = pr->next) {
        if (!memcmp(pr->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN)) {
            conf.proto_encap_type = pr->conf.proto_encap_type;
            conf.proto = pr->conf.proto;
            /* Check whether any port is configured for different VLAN */
            vcl_ports2_port_bitfield(entry->ports, ports);
            memcpy(conf.ports, ports, VTSS_PORT_BF_SIZE);
            conf.user = user;
            conf.isid = isid_del;
            if (vcl_proto_vlan_hw_entry_delete(&conf) != VTSS_VCL_RC_OK) {
            }
        }
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_group_entry_get(vtss_isid_t                       isid_get,
                                   vcl_proto_vlan_vlan_entry_t       *entry,
                                   vcl_proto_vlan_user_t             user,
                                   BOOL                              next,
                                   BOOL                              first)
{
    vcl_proto_grp_list_t            *list;
    vcl_proto_grp_t                 *grp;
    u8                              ports[VTSS_PORT_BF_SIZE];
    BOOL                            use_next = FALSE;
    u32                             i;
    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid Proto-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    for (grp = list->used; grp != NULL; grp = grp->next) {
        if ((user == grp->conf.user) && (isid_get == grp->conf.isid)) {
            if ((first == TRUE) || (use_next == TRUE)) {
                break;
            }
            vcl_ports2_port_bitfield(entry->ports, ports);
            if (!memcmp(grp->conf.ports, ports, VTSS_PORT_BF_SIZE) &&
                (!memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN))) {
                if (next) {
                    use_next = TRUE;
                } else {
                    break;
                } /* if (next) */
            } /* if (!memcmp */
        } /* if ((user == grp->conf.user) && (isid_del == grp->conf.isid)) */
    } /* for (grp = list->used, */
    if (grp != NULL) {
        memcpy(entry->group_id, grp->conf.group_id, MAX_GROUP_NAME_LEN);
        entry->vid = grp->conf.vid;
        /* Convert port bitfield to port list */
        for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
            entry->ports[i] = ((grp->conf.ports[i / 8]) & (1 << (i % 8))) ? TRUE : FALSE;
        }
    }
    vtss_vcl_crit_data_unlock();
    return ((grp == NULL) ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}

u32 vcl_proto_vlan_group_entry_get_by_vlan(vtss_isid_t                       isid_get,
                                           vcl_proto_vlan_vlan_entry_t       *entry,
                                           vcl_proto_vlan_user_t             user,
                                           BOOL                              next,
                                           BOOL                              first)
{
    vcl_proto_grp_list_t            *list;
    vcl_proto_grp_t                 *grp;
    BOOL                            use_next = FALSE;
    u32                             i;
    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid Proto-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_proto_vlan_glb_data.grp_list;
    for (grp = list->used; grp != NULL; grp = grp->next) {
        if ((user == grp->conf.user) && (isid_get == grp->conf.isid)) {
            if ((first == TRUE) || (use_next == TRUE)) {
                break;
            }
            if ((grp->conf.vid == entry->vid) && (!memcmp(grp->conf.group_id, entry->group_id, MAX_GROUP_NAME_LEN))) {
                if (next) {
                    use_next = TRUE;
                } else {
                    break;
                } /* if (next) */
            } /* if (!memcmp */
        } /* if ((user == grp->conf.user) && (isid_del == grp->conf.isid)) */
    } /* for (grp = list->used, */
    if (grp != NULL) {
        memcpy(entry->group_id, grp->conf.group_id, MAX_GROUP_NAME_LEN);
        entry->vid = grp->conf.vid;
        /* Convert port bitfield to port list */
        for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
            entry->ports[i] = ((grp->conf.ports[i / 8]) & (1 << (i % 8))) ? TRUE : FALSE;
        }
    }
    vtss_vcl_crit_data_unlock();
    return ((grp == NULL) ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}

u32 vcl_proto_vlan_hw_entry_add(vcl_proto_vlan_entry_t *entry)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *new, *tmp;
    BOOL                        update = FALSE, proto_exists = FALSE;
    u32                         i;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_assert_locked();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    /* Search for existing entry; If found update the entry */
    for (tmp = hw_list->used; tmp != NULL; tmp = tmp->next) {
        if ((entry->user == tmp->conf.user) && (entry->isid == tmp->conf.isid)) {
            if (tmp->conf.proto_encap_type == entry->proto_encap_type) {
                if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                    (entry->proto.eth2_proto.eth_type == tmp->conf.proto.eth2_proto.eth_type)) {
                    proto_exists = TRUE;
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                    if (!memcmp(entry->proto.llc_snap_proto.oui, tmp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                        && (entry->proto.llc_snap_proto.pid == tmp->conf.proto.llc_snap_proto.pid)) {
                        proto_exists = TRUE;
                    }
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                    if ((entry->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap)
                        && (entry->proto.llc_other_proto.ssap == tmp->conf.proto.llc_other_proto.ssap)) {
                        proto_exists = TRUE;
                    } /* if ((entry->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap) */
                } /* if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) && */
            } /* if (tmp->conf.proto_encap_type == entry->proto_encap_type) */
            if ((proto_exists == TRUE) && (entry->vid == tmp->conf.vid)) {
                update = TRUE;
                /* Updating entry is sufficient */
                for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                    tmp->conf.ports[i] |= entry->ports[i];
                    entry->ports[i] = tmp->conf.ports[i];
                }
                entry->vce_id = tmp->conf.vce_id;
            }
        } /* if ((entry->user == tmp->conf.user) && (entry->isid == tmp->conf.isid)) */
    }
    if (update == FALSE) {
        /* Get free node from free list */
        new = hw_list->free;
        /* 'new' can not be NULL as free never be NULL */
        if (new == NULL) {
            return VTSS_VCL_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        hw_list->free = new->next;
        /* Copy the configuration */
        new->conf = *entry;
        /* Update the used list */
        new->next = hw_list->used;
        hw_list->used = new;
    }

    if (vcl_stack_vcl_proto_vlan_conf_add_del(entry->isid, entry, TRUE) != VTSS_RC_OK) {
        return VTSS_VCL_ERROR_STACK_STATE;
    }
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_hw_entry_delete_by_protocol(vcl_proto_encap_type_t proto_encap_type, vcl_proto_conf_t *proto)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *tmp, *prev;
    BOOL                        found = FALSE;
    vcl_proto_vlan_entry_t      conf;

    /* Check for NULL pointer */
    if (proto == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_assert_locked();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    for (tmp = hw_list->used, prev = NULL; tmp != NULL;) {
        found = FALSE;
        if (tmp->conf.proto_encap_type == proto_encap_type) {
            if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                (proto->eth2_proto.eth_type == tmp->conf.proto.eth2_proto.eth_type)) {
                found = TRUE;
            } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                if (!memcmp(proto->llc_snap_proto.oui, tmp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                    && (proto->llc_snap_proto.pid == tmp->conf.proto.llc_snap_proto.pid)) {
                    found = TRUE;
                }
            } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                if ((proto->llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap)
                    && (proto->llc_other_proto.ssap == tmp->conf.proto.llc_other_proto.ssap)) {
                    found = TRUE;
                }
            }
        }
        if (found == TRUE) {
            conf.proto_encap_type = proto_encap_type;
            conf.proto = *proto;
            conf.isid = tmp->conf.isid;
            conf.vid = tmp->conf.vid;
            conf.vce_id = tmp->conf.vce_id;
            conf.user = tmp->conf.user;
            memcpy(conf.ports, tmp->conf.ports, VTSS_PORT_BF_SIZE);
            /* Send the stack message to update slaves to delete this entry */
            if (vcl_stack_vcl_proto_vlan_conf_add_del(tmp->conf.isid, &conf, FALSE) != VTSS_RC_OK) {
                return VTSS_VCL_ERROR_STACK_STATE;
            }
            /* Move entry from used list to free list */
            if (prev == NULL) {
                hw_list->used = tmp->next;
            } else {
                prev->next = tmp->next;
            }
            /* Move entry from used list to free list */
            tmp->next = hw_list->free;
            hw_list->free = tmp;
            /* Point to previous node to continue the list traversal */
            tmp = prev;
        }
        /* First node deletion case */
        if ((tmp == NULL) && (found == TRUE)) {
            tmp = hw_list->used;
            prev = NULL;
        } else {
            prev = tmp;
            /* Warning -- Possible use of null pointer 'tmp' in left argument to operator '->'.
               This is not possible as tmp will never be NULL when node_deleted = FALSE */
            /*lint -e{613} */
            tmp = tmp->next;
        }
    }
    return VTSS_VCL_RC_OK;
}

u32 vcl_proto_vlan_hw_entry_get(vtss_isid_t                       isid_get,
                                vcl_proto_vlan_entry_t            *entry,
                                u32                               id,
                                vcl_proto_vlan_user_t             user,
                                BOOL                              next)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *tmp;
    BOOL                        use_next = FALSE;
    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid Proto-based VLAN User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user > VCL_PROTO_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL)) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    /* Search for existing entry */
    for (tmp = hw_list->used; tmp != NULL; tmp = tmp->next) {
        if ((user == tmp->conf.user) && (isid_get == tmp->conf.isid)) {
            if (id == 0) {
                break;
            } else {
                if (use_next) {
                    break;
                }
                if (id == tmp->conf.vce_id) {
                    if (next) {
                        use_next = 1;
                    } else {
                        break;
                    } /* if (next) */
                } /* if (id == tmp->conf.vce_id) */
            } /* if (id == 0) */
        } /* if ((user == tmp->conf.user) && (isid_get == tmp->conf.isid)) */
    } /* for (tmp = hw_list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) */
    if (tmp != NULL) {
        *entry = tmp->conf;
    }
    vtss_vcl_crit_data_unlock();
    return (tmp == NULL) ?  VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK;
}

/**
 * INPUT: conf->vce_id, conf->proto_encap_type, conf->proto.
 *        If conf->vce_id == NULL=>fetch first entry for this protocol and user
 * OUTPUT: conf
 **/
u32 vcl_proto_vlan_hw_entry_get_by_protocol(vcl_proto_vlan_entry_t  *conf,
                                            vcl_proto_vlan_user_t   user,
                                            BOOL                    next)
{
    vcl_proto_hw_conf_list_t    *hw_list;
    vcl_proto_hw_conf_t         *tmp;
    BOOL                        use_next = FALSE;

    vtss_vcl_crit_data_lock();
    hw_list = &vcl_glb_data.vcl_proto_vlan_glb_data.hw_list;
    /* Search for existing entry */
    for (tmp = hw_list->used; tmp != NULL; tmp = tmp->next) {
        if (user == tmp->conf.user) {
            if (tmp->conf.proto_encap_type == conf->proto_encap_type) {
                if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) &&
                    (conf->proto.eth2_proto.eth_type == tmp->conf.proto.eth2_proto.eth_type)) {
                    if (conf->vce_id == VCE_ID_NONE) {
                        break;
                    }
                    if (use_next) {
                        break;
                    }
                    if (conf->vce_id == tmp->conf.vce_id) {
                        if (next) {
                            use_next = 1;
                        } else {
                            break;
                        }
                    }
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                    if (!memcmp(conf->proto.llc_snap_proto.oui, tmp->conf.proto.llc_snap_proto.oui, OUI_SIZE)
                        && (conf->proto.llc_snap_proto.pid == tmp->conf.proto.llc_snap_proto.pid)) {
                        if (conf->vce_id == VCE_ID_NONE) {
                            break;
                        }
                        if (use_next) {
                            break;
                        }
                        if (conf->vce_id == tmp->conf.vce_id) {
                            if (next) {
                                use_next = 1;
                            } else {
                                break;
                            }
                        }

                    }
                } else if (tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                    if ((conf->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap)
                        && (conf->proto.llc_other_proto.ssap == tmp->conf.proto.llc_other_proto.ssap)) {
                        if (conf->vce_id == VCE_ID_NONE) {
                            break;
                        }
                        if (use_next) {
                            break;
                        }
                        if (conf->vce_id == tmp->conf.vce_id) {
                            if (next) {
                                use_next = 1;
                            } else {
                                break;
                            }
                        }
                    } /* if ((conf->proto.llc_other_proto.dsap == tmp->conf.proto.llc_other_proto.dsap) */
                } /* if ((tmp->conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) && */
            } /* if (tmp->conf.proto_encap_type == conf->proto_encap_type) */
        } /* if (user == tmp->conf.user) */
    } /* for (tmp = list->used; tmp != NULL; tmp = tmp->next) */
    if (tmp != NULL) {
        *conf = tmp->conf;
    }
    vtss_vcl_crit_data_unlock();
    return (tmp == NULL ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}

void ip_mask_len_2_mask(u8 mask_len, ulong *ip_mask)
{
    char kk;
    *ip_mask = 0x0;
    for (kk = 31; kk >= (32 - mask_len); kk--) {
        *ip_mask |= (1 << kk);
    }
}

/* Last local entry is the first entry that was added. */
void vcl_ip_vlan_first_vce_id_get(vtss_vce_id_t *id)
{
    vcl_ip_local_list_t      *list;
    vcl_ip_local_conf_t      *tmp = NULL;
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.local_list;
    /* If no entry present return VCE_ID_NONE */
    if (list->used == NULL) {
        *id = VCE_ID_NONE;
    } else {
        tmp = list->used;
        /* while (tmp->next != NULL) { */
        /*     tmp = tmp->next; */
        /* } */
        *id = tmp->conf.id;
    }
    vtss_vcl_crit_data_unlock();
    return;
}

u32 vcl_ip_vlan_vce_id_get(u32 *vce_id)
{
    vcl_ip_list_t               *list;
    vcl_ip_t                    *tmp;
    u16                         id_used[VCL_IP_VLAN_MAX_ENTRIES];
    u32                         i, rc = VTSS_VCL_ERROR_TABLE_FULL;

    memset(id_used, 0, sizeof(id_used));
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        id_used[tmp->conf.vce_id - VCE_ID_START] = 1;
    }
    for (i = VCE_ID_START; i <= VCL_IP_VLAN_MAX_ENTRIES; i++) {
        if (!id_used[i - VCE_ID_START]) {
            *vce_id =  i;
            rc = VTSS_VCL_RC_OK;
            break;
        }
    }
    vtss_vcl_crit_data_unlock();
    return rc;
}

/* IP subnet-based VLAN base function definitions */
u32 vcl_ip_vlan_local_entry_add(vcl_ip_vlan_msg_cfg_t *entry, vtss_vce_id_t *next_id)
{
    vcl_ip_local_list_t      *list;
    vcl_ip_local_conf_t      *new = NULL, *tmp, *prev, *ins = NULL , *ins_prev = NULL;
    u32                      i;
    BOOL                     update = FALSE;

    *next_id = 0;
    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.local_list;
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        if (tmp->conf.mask_len < entry->mask_len) { /* Insertion point */
            ins = tmp;
            ins_prev = prev;
        }
        /* If IP subnet and VLAN matches, update the ports list */
        if (entry->id == tmp->conf.id) {
            /* Update the entry */
            update = TRUE;
            ins = tmp;
            break;
        }
    }
    if (update == FALSE) {
        /* Get free node from free list */
        new = list->free;
        if (new == NULL) {
            vtss_vcl_crit_data_unlock();
            return VTSS_VCL_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        new->conf = *entry;
        /* Update the used list */
        new->next = NULL;
        if (ins == NULL) { /* Add the entry to the end of list */
            if (list->used == NULL) {
                /* Adding first entry to the empty list */
                list->used = new;
            } else {
                /* Adding the entry after last entry in the list */
                prev->next = new;
            }
        } else { /* Add the entry to either head or middle of the list */
            if (ins_prev != NULL) { /* Add the entry to the middle of the list */
                ins_prev->next = new;
                new->next = ins;
                *next_id = ins->conf.id;
            } else { /* Add the entry before first entry */
                new->next = list->used;
                *next_id = list->used->conf.id;
                list->used = new;
            }
        }
    } else { /* Update the entry */
        if (ins != NULL) { /* This case always happens. But, this check is for satisfying LINT */
            ins->conf.vid = entry->vid;
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                ins->conf.ports[i] |= entry->ports[i];
            }
        }
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}

u32 vcl_ip_vlan_local_entry_delete(vcl_ip_vlan_msg_cfg_t *entry)
{
    vcl_ip_local_list_t      *list;
    vcl_ip_local_conf_t      *tmp = NULL, *prev;
    BOOL                     ports_exist = FALSE;
    u32                      i;

    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.local_list;
    for (tmp = list->used, prev = NULL; tmp != NULL; prev = tmp, tmp = tmp->next) {
        /* If IP subnet and VLAN matches, delete the ports from the ports list */
        if (entry->id == tmp->conf.id) {
            /* Update the port list */
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                tmp->conf.ports[i] &= ~entry->ports[i];
            }
            /* Check if all the ports are deleted. If so, we need to delete the entry */
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                if (tmp->conf.ports[i] != 0) {
                    ports_exist = TRUE;
                }
            }
            break;
        }
    }
    if ((ports_exist == FALSE) && (tmp != NULL)) {
        /* Move entry from used list to free list */
        if (prev == NULL) {
            list->used = tmp->next;
        } else {
            prev->next = tmp->next;
        }
        tmp->next = list->free;
        list->free = tmp;
    }
    vtss_vcl_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VCL_RC_OK : VTSS_VCL_ERROR_ENTRY_NOT_FOUND);
}

u32 vcl_ip_vlan_local_entry_get(vcl_ip_vlan_msg_cfg_t *entry, BOOL first, BOOL next)
{
    vcl_ip_local_list_t      *list;
    vcl_ip_local_conf_t      *tmp = NULL;
    BOOL                     use_next = FALSE;

    /* Check for NULL pointer */
    if (entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.local_list;
    for (tmp = list->used; tmp != NULL; tmp = tmp->next) {
        if (first == TRUE) {
            break;
        } else {
            if (use_next) {
                break;
            }
            if ((entry->ip_addr == tmp->conf.ip_addr) && (entry->mask_len == tmp->conf.mask_len)
                && (entry->vid == tmp->conf.vid)) {
                if (next) {
                    use_next = 1;
                } else {
                    break;
                } /* if (next) */
            } /* if ((entry->ip_addr == tmp->conf.ip_addr) && (entry->mask_len == tmp->conf.mask_len) */
        } /* if (first == TRUE) */
    } /* for (tmp = list->used; tmp != NULL; tmp = tmp->next) */
    if (tmp != NULL) {
        *entry = tmp->conf;
    }
    vtss_vcl_crit_data_unlock();
    return ((tmp != NULL) ? VTSS_VCL_RC_OK : VTSS_VCL_ERROR_ENTRY_NOT_FOUND);
}

u32 vcl_ip_vlan_entry_add(vcl_ip_vlan_entry_t *ip_vlan_entry,
                          vcl_ip_vlan_user_t  user)
{
    vcl_ip_list_t *list;
    vcl_ip_t      *entry, *new, *prev, *ins = NULL , *ins_prev = NULL;
    BOOL          update = FALSE;
    u32           i;
    ulong         mask1, mask2;

    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid IP subnet-based VLAN User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user > VCL_IP_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }

    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    /* Calculate subnet mask */
    ip_mask_len_2_mask(ip_vlan_entry->mask_len, &mask2);
    /* VCE ID and IP subnet must be unique for each entry. One IP subnet can only be mapped to one VLAN.
       Entries are sorted in descending order of mask length. We need to loop through all entries to check
       the uniqueness of VCE ID
     */
    for (entry = list->used, prev = NULL; entry != NULL; prev = entry, entry = entry->next) {
        if (entry->conf.mask_len < ip_vlan_entry->mask_len) { /* Insertion point */
            ins = entry;
            ins_prev = prev;
        }
        if (ip_vlan_entry->vce_id == entry->conf.vce_id) { /* This means either update the entry or return error */
            if (entry->conf.mask_len == ip_vlan_entry->mask_len) {
                /* Calculate subnet mask */
                ip_mask_len_2_mask(entry->conf.mask_len, &mask1);
                /* If ip subnet and vid matches, then ports updation is allowed */
                if ((entry->conf.ip_addr & mask1) == (ip_vlan_entry->ip_addr & mask2)) {
                    /* Update the entry */
                    update = TRUE;
                    ins = entry;
                    break;
                } else { /* Subnets didn't match */
                    /* Return the error as subnet didn't match for same VCE ID */
                    vtss_vcl_crit_data_unlock();
                    return VTSS_VCL_ERROR_ENTRY_WITH_DIFF_SUBNET;
                }
            } else { /* Mask lengths are different. Subnet cannot match */
                /* Return the error as subnet didn't match for same VCE ID */
                vtss_vcl_crit_data_unlock();
                return VTSS_VCL_ERROR_ENTRY_WITH_DIFF_SUBNET;
            } /* if (ip_vlan_entry->vce_id == entry->conf.vce_id) */
        } else { /* Check for duplicate entries */
            if ((entry->conf.mask_len == ip_vlan_entry->mask_len) && (entry->conf.ip_addr == ip_vlan_entry->ip_addr) &&
                (entry->conf.vid == ip_vlan_entry->vid)) {
                vtss_vcl_crit_data_unlock();
                return VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
            }
        } /* if (ip_vlan_entry->vce_id == entry->conf.vce_id) */
    } /* for (entry = list->used; entry != NULL; entry = entry->next) */
    if (update == FALSE) {
        /* Get free node from free list */
        new = list->free;
        if (new == NULL) {
            vtss_vcl_crit_data_unlock();
            return VTSS_VCL_ERROR_TABLE_FULL;
        }
        /* Update the free list */
        list->free = new->next;
        /* Copy the configuration */
        new->conf = *ip_vlan_entry;
        /* Update the used list */
        new->next = NULL;
        if (ins == NULL) { /* Add the entry to the end of list */
            if (list->used == NULL) {
                /* Adding first entry to the empty list */
                list->used = new;
            } else {
                /* Adding the entry after last entry in the list */
                prev->next = new;
            }
        } else { /* Add the entry to either head or middle of the list */
            if (ins_prev != NULL) { /* Add the entry to the middle of the list */
                ins_prev->next = new;
                new->next = ins;
            } else { /* Add the entry before first entry */
                new->next = list->used;
                list->used = new;
            }
        }
    } else { /* Update the entry */
        if (ins != NULL) { /* This case always happens. But, this check is for satisfying LINT */
            ins->conf.vid = ip_vlan_entry->vid;
            for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
                ins->conf.l2ports[i] |= ip_vlan_entry->l2ports[i];
            }
        }
    }
    vtss_vcl_crit_data_unlock();
    return VTSS_VCL_RC_OK;
}
u32 vcl_ip_vlan_entry_del(vcl_ip_vlan_entry_t *ip_vlan_entry,
                          vcl_ip_vlan_user_t  user)
{
    vcl_ip_list_t *list;
    vcl_ip_t      *entry, *prev;
    BOOL          found_entry = FALSE, ports_exist = FALSE;
    u32           i;

    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid IP subnet-based VLAN User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user > VCL_IP_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }

    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    /* Search for existing entry */
    for (entry = list->used, prev = NULL; entry != NULL; prev = entry, entry = entry->next) {
        if (ip_vlan_entry->vce_id == entry->conf.vce_id) {
            found_entry = TRUE;
            for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
                entry->conf.l2ports[i] &= (~ip_vlan_entry->l2ports[i]);
                if (entry->conf.l2ports[i]) {
                    ports_exist = TRUE;
                } /* if (entry->conf.l2ports[i]) */
            } /* for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) */
            if (ports_exist == FALSE) { /* delete the entry */
                /* Move entry from used list to free list */
                if (prev == NULL) {
                    list->used = entry->next;
                } else {
                    prev->next = entry->next;
                }
                /* Move entry from used list to free list */
                entry->next = list->free;
                list->free = entry;
            } /* if (ports_exist == FALSE) */
            break;
        } /* if (ip_vlan_entry->vce_id == entry->conf.vce_id) */
    } /* for (entry = list->used; entry != NULL; entry = entry->next) */
    vtss_vcl_crit_data_unlock();
    return ((found_entry == FALSE) ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}
u32 vcl_ip_vlan_entry_get(vcl_ip_vlan_entry_t  *ip_vlan_entry,
                          vcl_ip_vlan_user_t   user,
                          BOOL                 first,
                          BOOL                 next)
{
    vcl_ip_list_t *list;
    vcl_ip_t      *entry;
    BOOL          use_next = FALSE;

    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        return VTSS_VCL_ERROR_PARM;
    }
    /* Check for valid IP subnet-based VLAN User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user > VCL_IP_VLAN_USER_ALL)) {
        return VTSS_VCL_ERROR_PARM;
    }

    vtss_vcl_crit_data_lock();
    list = &vcl_glb_data.vcl_ip_vlan_glb_data.ip_vlan_list;
    /* Search for existing entry */
    for (entry = list->used; entry != NULL; entry = entry->next) {
        if (use_next) {
            break;
        }
        /* If first flag is set and next is FALSE then return the first entry */
        if (first && !next) {
            break;
        }
        /* If first flag is set and next flag is also set, return the second entry */
        if (first && next) {
            use_next = TRUE;
            continue;
        }
        /* If VCE ID matches */
        if (ip_vlan_entry->vce_id == entry->conf.vce_id) {
            /* If next flag is set, return the first entry after the matching entry else return the matching entry */
            if (next) {
                use_next = TRUE;
            } else {
                break;
            }
        }
    }
    if (entry != NULL) {
        *ip_vlan_entry = entry->conf;
    }
    vtss_vcl_crit_data_unlock();
    return (entry == NULL ? VTSS_VCL_ERROR_ENTRY_NOT_FOUND : VTSS_VCL_RC_OK);
}
