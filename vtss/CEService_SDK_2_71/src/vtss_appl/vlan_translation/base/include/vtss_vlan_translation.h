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

#ifndef _VTSS_VLAN_TRANSLATION_H_
#define _VTSS_VLAN_TRANSLATION_H_

#include <main.h>           /* VTSS common definitions */

/*********************** Macros to define errors ***************************************************************************/
/*********************** ----------------------- ***************************************************************************/
#define     VTSS_VT_RC_OK                              0               /**< No Error                                       */
#define     VTSS_VT_ERROR_PARM                         1               /**< Illegal parameter                              */
#define     VTSS_VT_ERROR_CONFIG_NOT_OPEN              2               /**< Configuration open error                       */
#define     VTSS_VT_ERROR_ENTRY_NOT_FOUND              3               /**< Entry not found                                */
#define     VTSS_VT_ERROR_TABLE_EMPTY                  4               /**< Table empty                                    */
#define     VTSS_VT_ERROR_TABLE_FULL                   5               /**< Table full                                     */
#define     VTSS_VT_ERROR_REG_TABLE_FULL               6               /**< Registration table full                        */
#define     VTSS_VT_ERROR_USER_PREVIOUSLY_CONFIGURED   8               /**< Previously configured                          */
#define     VTSS_VT_ERROR_ENTRY_PREVIOUSLY_CONFIGURED  10              /**< Entry previously configured                    */
/********************End****************************************************************************************************/

/********************** Macros to define maximum values and ranges ***************************************************/
/********************** ------------------------------------------ ***************************************************/
#define     VT_MAX_TRANSLATION_CNT                     256             /**< Maximum VLAN Translations count                */
#define     VT_MAX_GROUP_CNT                           VTSS_PORTS      /**< Maximum Group count                            */
#define     VT_NULL_GROUP_ID                           0               /**< Special value for group ID                     */
#define     VT_FIRST_GROUP_ID                          1               /**< First Group ID                                 */
#define     VT_VID_START                               1               /**< First valid VLAN ID                            */
#define     VT_MAX_VLAN_ID                             4095            /**< Last valid VLAN ID                             */
#define     VT_LAST_GROUP_ID                           (VT_FIRST_GROUP_ID + VT_MAX_GROUP_CNT - 1) /**< Last valid Group ID */
/********************End****************************************************************************************************/

/********************** Macros to check ranges *****************************************************************************/
/********************** ---------------------- *****************************************************************************/
/**< Macro to check valid group */
#define     VT_VALID_GROUP_CHECK(grp_id)               (((grp_id < VT_FIRST_GROUP_ID) || (grp_id > VT_LAST_GROUP_ID)) ? FALSE : TRUE)
/**< Macro to check valid VLAN ID  */
#define     VT_VALID_VLAN_CHECK(vid)                   (((vid < VT_VID_START) || (vid > VT_MAX_VLAN_ID)) ? FALSE : TRUE)
/**< Macro to check NULL Pointer */
#define     VT_NULL_CHECK(ptr)                         ((ptr == NULL) ? FALSE : TRUE)
/********************End****************************************************************************************************/

/******************* Structures related to port to group mapping ****************************************************************/
/******************* ------------------------------------------- ****************************************************************/
/**
 * This Structure contains a port to a group mapping
 **/
typedef struct vlan_trans_port2grp_conf_s {
    u16                                       group_id;                 /**< Group ID                        */
    u8                                        ports[VTSS_PORT_BF_SIZE]; /**< Ports Bitfield                  */
} vlan_trans_port2grp_conf_t;
/**
 * This structure stores port to group mapping and next pointer to store it in list
 **/
typedef struct vlan_trans_port2grp_list_entry_s {
    struct vlan_trans_port2grp_list_entry_s   *next;                    /**< Next in list                    */
    vlan_trans_port2grp_conf_t                conf;                     /**< Port to Group Configuration     */
} vlan_trans_port2grp_list_entry_t;
/**
 * This stucture contains two list - used and free which are used to store used entries
 * and free entries respectively
 **/
typedef struct vlan_trans_port2grp_list_s {
    vlan_trans_port2grp_list_entry_t          *free;                    /**< Free list                       */
    vlan_trans_port2grp_list_entry_t          *used;                    /**< Used list                       */
} vlan_trans_port2grp_list_t;
/********************End*********************************************************************************************************/

/******************* Structures related to group to VLAN mapping ****************************************************************/
/******************* ------------------------------------------- ****************************************************************/
/**
 * This Structure contains a group to a VLAN Translation mapping
 **/
typedef struct vlan_trans_grp2vlan_conf_s {
    u16                                       group_id;                 /**< Group ID                        */
    vtss_vid_t                                vid;                      /**< VLAN ID                         */
    vtss_vid_t                                trans_vid;                /**< Translated VLAN ID              */
} vlan_trans_grp2vlan_conf_t;
/**
 * This structure stores group to VLAN mapping and next pointer to store it in list
 **/
typedef struct vlan_trans_grp2vlan_list_entry_s {
    struct vlan_trans_grp2vlan_list_entry_s   *next;                    /**< Next in list                    */
    vlan_trans_grp2vlan_conf_t                conf;                     /**< Group to VLAN Configuration     */
} vlan_trans_grp2vlan_list_entry_t;
/**
 * This stucture contains two list - used and free which are used to store used entries
 * and free entries respectively
 **/
typedef struct vlan_trans_grp2vlan_list_s {
    vlan_trans_grp2vlan_list_entry_t          *free;                    /**< Free list                       */
    vlan_trans_grp2vlan_list_entry_t          *used;                    /**< Used list                       */
} vlan_trans_grp2vlan_list_t;
/********************End*********************************************************************************************************/
/**
 * VLAN Translation Global database
 **/
typedef struct {
    /* Lists to maintain Ports to Group mapping */
    vlan_trans_port2grp_list_t                port2grp_list;                        /**< List to Ports to Group mapping         */
    vlan_trans_port2grp_list_entry_t          port2grp_db[VT_MAX_GROUP_CNT];        /**< Port to Group mapping array            */
    /* Lists to maintain Group to VLAN mapping */
    vlan_trans_grp2vlan_list_t                grp2vlan_list;                        /**< List to maintain Group to VLAN mapping */
    vlan_trans_grp2vlan_list_entry_t          grp2vlan_db[VT_MAX_TRANSLATION_CNT];  /**< Group to VLAN mapping array            */
} vlan_trans_global_data_t;

void vlan_trans_ports2_port_bitfield(BOOL *ports, u8 *ports_bf);
void vlan_trans_port_bitfield2_ports(u8 *ports_bf, BOOL *ports);
void vlan_trans_default_set(void);
void vlan_trans_vlan_list_default_set(void);
void vlan_trans_port_list_default_set(void);
u32 vlan_trans_grp2vlan_entry_add(const u16 group_id, const vtss_vid_t vid, const vtss_vid_t trans_vid);
u32 vlan_trans_grp2vlan_entry_delete(const u16 group_id, const vtss_vid_t vid);
u32 vlan_trans_grp2vlan_entry_get(vlan_trans_grp2vlan_conf_t *conf, BOOL next);
u32 vlan_trans_port2grp_entry_add(const u16 group_id, u8 *port_bf);
u32 vlan_trans_port2grp_entry_get(vlan_trans_port2grp_conf_t *conf, BOOL next);

/*************************************************************************************/
/*  VLAN TRANSLATION platform call out interface                                                  */
/*************************************************************************************/
/**
 *  \brief  VLAN TRANSLATION database lock function.
 *  This function locks the crititical section.
 *
 *  \return
 *  Nothing.
 */
void vtss_vlan_trans_crit_data_lock(void);

/**
 *  \brief  VLAN TRANSLATION database unlock function.
 *  This function unlocks the crititical section.
 *
 *  \return
 *  Nothing.
 */
void vtss_vlan_trans_crit_data_unlock(void);

/**
 *  \brief  VLAN TRANSLATION database lock assert function.
 *  This function checks for crititical section lock. If it is not already taken by the function
 *  calling this function, it asserts.
 *
 *  \return
 *  Nothing.
 */
void vtss_vlan_trans_crit_data_assert_locked(void);
#endif /* _VTSS_VLAN_TRANSLATION_H_ */
