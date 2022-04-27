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
#ifndef _VLAN_TRANSLATION_API_H_
#define _VLAN_TRANSLATION_API_H_
#include <vtss_vlan_translation.h>
#include <main.h>                       /* VTSS common definitions                      */
#include <critd_api.h>                  /**< For critical section related code          */

#define         VT_VLAN_BLK_VERSION      0       /* VLAN Translation VLAN table flash version */
#define         VT_PORT_BLK_VERSION      0       /* VLAN Translation Port table flash version */
/**
 * \brief VLAN Translation error codes.
 * This enum identifies different error types used in the VLAN Translation module.
 */
enum {
    VT_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_VLAN_TRANSLATION), /**< Generic error code                        */
    VT_ERROR_PARM,                                                      /**< Illegal parameter                         */
    VT_ERROR_CONFIG_NOT_OPEN,                                           /**< Configuration open error                  */
    VT_ERROR_ENTRY_NOT_FOUND,                                           /**< Entry not found                           */
    VT_ERROR_TABLE_EMPTY,                                               /**< Table empty                               */
    VT_ERROR_TABLE_FULL,                                                /**< Table full                                */
    VT_ERROR_REG_TABLE_FULL,                                            /**< Registration table full                   */
    VT_ERROR_USER_PREVIOUSLY_CONFIGURED,                                /**< Already configured                        */
    VT_ERROR_ENTRY_PREVIOUSLY_CONFIGURED,                               /**< Entry already configured                  */
    VT_ERROR_VLAN_SAME_AS_TRANSLATE_VLAN                                /**< VLAN and translate VLAN is the same       */
};
/**
 * \brief This Structure contains a group to a VLAN Translation mapping configuration
 **/
typedef struct vlan_trans_mgmt_grp2vlan_conf_s {
    u16         group_id;                                       /**< Group ID                                  */
    vtss_vid_t  vid;                                            /**< VLAN ID                                   */
    vtss_vid_t  trans_vid;                                      /**< Translated VLAN ID                        */
} vlan_trans_mgmt_grp2vlan_conf_t;
/**
 * \brief This Structure contains port to group mapping configuration
 **/
typedef struct vlan_trans_mgmt_port2grp_conf_s {
    u16         group_id;                                       /**< Group ID                                  */
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];                    /**< Ports                                     */
} vlan_trans_mgmt_port2grp_conf_t;

/**
 * \brief VLAN Translation VLAN configuration table
 */
typedef struct {
    u32                        version;                         /**< Block version                              */
    u32                        count;                           /**< Number of entries                          */
    u32                        size;                            /**< Size of each entry                         */
    vlan_trans_grp2vlan_conf_t table[VT_MAX_TRANSLATION_CNT];   /**< Entries                                    */
} vlan_trans_vlan_table_blk_t;

/**
 * \brief VLAN Translation Port configuration table
 */
typedef struct {
    u32                        version;                         /**< Block version                              */
    u32                        count;                           /**< Number of entries                          */
    u32                        size;                            /**< Size of each entry                         */
    vlan_trans_port2grp_conf_t table[VT_MAX_GROUP_CNT];         /**< Entries                                    */
} vlan_trans_port_table_blk_t;

/**
 * \brief Function for initializing an entry with it's default configuration
 * \param group_id [IN]  The id number for the entry (group) to set to default.
 * \param entry [OUT] Pointer to the entry to initialize.
 */
void entry_default_set(vlan_trans_port2grp_conf_t *entry, u8 group_id);


vtss_rc vlan_trans_init(vtss_init_data_t *data);
char *vlan_trans_error_txt(vtss_rc rc);
vtss_rc vlan_trans_mgmt_grp2vlan_entry_add(const u16 group_id, const vtss_vid_t vid, const vtss_vid_t trans_vid);
vtss_rc vlan_trans_mgmt_grp2vlan_entry_delete(const u16 group_id, const vtss_vid_t vid);
vtss_rc vlan_trans_mgmt_grp2vlan_entry_get(vlan_trans_mgmt_grp2vlan_conf_t *const conf, BOOL next);
vtss_rc vlan_trans_mgmt_port2grp_entry_add(const u16 group_id, BOOL *ports);
vtss_rc vlan_trans_mgmt_port2grp_entry_get(vlan_trans_mgmt_port2grp_conf_t *const conf, BOOL next);
#endif /* _VLAN_TRANSLATION_API_H_ */
