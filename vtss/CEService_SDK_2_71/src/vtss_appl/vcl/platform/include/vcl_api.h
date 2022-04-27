/*

   Vitesse VCL software.

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

#ifndef _VCL_API_H_
#define _VCL_API_H_

/**
 * \file vcl_api.h
 * \brief VCL platform header file
 *
 * This file contains the definitions of management API functions and associated types.
 * This file is exposed to other modules.
 */

#include <vtss_types.h>                                         /**< For uXXX, iXXX, and BOOL                   */
#include <vtss_vcl.h>                                           /**< For all base definitions code              */
#include <critd_api.h>                                          /**< For critical section related code          */

/**
 *  ARP Ether Type
 */
#define     ETHERTYPE_ARP                               0x806
/**
 *  IPv4 Ether Type
 */
#define     ETHERTYPE_IP                                0x800
/**
 *  IPv6 Ether Type
 */
#define     ETHERTYPE_IP6                               0x86DD
/**
 *  IPX Ether Type
 */
#define     ETHERTYPE_IPX                               0x8137
/**
 *  AppleTalk Ether Type
 */
#define     ETHERTYPE_AT                                0x809B
/**
 * Max length of the group Id
 **/
//#define     MAX_GROUP_NAME_LEN                          16

/**
 * \brief VCL error codes.
 * This enum identifies different error types used in the VCL module.
 */
enum {
    VCL_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_VCL),     /**< Generic error code                        */
    VCL_ERROR_PARM,                                             /**< Illegal parameter                         */
    VCL_ERROR_CONFIG_NOT_OPEN,                                  /**< Configuration open error                  */
    VCL_ERROR_ENTRY_NOT_FOUND,                                  /**< Entry not found                           */
    VCL_ERROR_TABLE_EMPTY,                                      /**< Table empty                               */
    VCL_ERROR_TABLE_FULL,                                       /**< Table full                                */
    VCL_ERROR_REG_TABLE_FULL,                                   /**< Registration table full                   */
    VCL_ERROR_STACK_STATE,                                      /**< Illegal MASTER/SLAVE state                */
    VCL_ERROR_USER_PREVIOUSLY_CONFIGURED,                       /**< Already configured                        */
    VCL_ERROR_VCE_ID_PREVIOUSLY_CONFIGURED,                     /**< VCE ID previously configured              */
    VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED,                      /**< Entry previously configured               */
    VCL_ERROR_ENTRY_WITH_DIFF_NAME,                             /**< Entry exists with different name          */
    VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN,                        /**< Entry exists with different name and VLAN */
    VCL_ERROR_ENTRY_WITH_DIFF_SUBNET,                           /**< Entry exists with different subnet        */
    VCL_ERROR_ENTRY_WITH_DIFF_VLAN                              /**< Entry exists with different VLAN          */
};

/**
 *  \brief This structure is used by MAC-based VLAN Users such as CLI/Web to convey SMAC to VID and port mapping to VCL module.
 */
typedef struct {
    vtss_mac_t  smac;                                           /**< Source MAC Address                         */
    vtss_vid_t  vid;                                            /**< VLAN ID                                    */
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];                    /**< Ports on which SMAC based VLAN is enabled  */
} vcl_mac_vlan_mgmt_entry_t;

/**
 *  \brief This structure is used by VCL module to pass the MAC-based configuration to VLAN Users.
 */
typedef struct {
    vtss_mac_t  smac;                                           /**< Source MAC Address                         */
    vtss_vid_t  vid;                                            /**< VLAN ID                                    */
    BOOL        ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /**< Ports on which SMAC based VLAN is enabled  */
} vcl_mac_vlan_mgmt_entry_get_cfg_t;

/**
 * \brief This structure is required to store the Group to VLAN and ports mapping
 */
typedef struct {
    u8                      group_id[MAX_GROUP_NAME_LEN];       /**< Group ID                                   */
    vtss_vid_t              vid;                                /**< VLAN ID                                    */
    u8                      ports[VTSS_PORT_ARRAY_SIZE];        /**< Port list on which this mapping is valid   */
} vcl_proto_vlan_mgmt_vlan_entry_t;

/**
 *  \brief This structure is used by IP-based VLAN Users such as CLI/Web to convey IP Subnet to VID and port
 *         mapping to VCL module.
 */
typedef struct {
    u16         vce_id;                                         /**< Entry identifier                           */
    vtss_ipv4_t ip_addr;                                        /**< IP Address                                 */
    u8          mask_len;                                       /**< Mask length                                */
    vtss_vid_t  vid;                                            /**< VLAN ID                                    */
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];                    /**< Ports on which IP-based VLAN is enabled    */
} vcl_ip_vlan_mgmt_entry_t;

/**
 *  \brief This structure is used by VCL module to store the SMAC to VID
 *         and port association in Flash.
 */
typedef struct {
    vtss_mac_t  smac;                                           /**< Source MAC Address                         */
    vtss_vid_t  vid;                                            /**< VLAN ID                                    */
    u8          l2ports[VTSS_L2PORT_BF_SIZE];                   /**< Ports bit map                              */
} vcl_mac_vlan_flash_entry_t;

/**
 *  \brief This structure is used by VCL module to send the SMAC to VID and port association to slaves.
 */
typedef struct {
    vtss_vce_id_t           id;                                         /**< VCE ID                                     */
    vtss_mac_t              smac;                                       /**< Source MAC Address                         */
    vtss_vid_t              vid;                                        /**< VLAN ID                                    */
    vcl_mac_vlan_user_t     user;                                       /**< User                                       */
    u8                      ports[VTSS_PORT_BF_SIZE];                   /**< Port list bit field                        */
} vcl_mac_vlan_msg_cfg_t;

/**
 *  \brief This structure is used by VCL module to send the Protocol to VID and port association to slaves.
 */
typedef struct {
    vtss_vce_id_t           id;                                 /**< VCE ID                                     */
    vcl_proto_encap_type_t  proto_encap_type;                   /**< Encapsulation type                         */
    vcl_proto_conf_t        proto;                              /**< Protocol                                   */
    vtss_vid_t              vid;                                /**< VLAN ID                                    */
    u8                      ports[VTSS_PORT_BF_SIZE];           /**< Port list bit field                        */
    vcl_proto_vlan_user_t   user;                               /**< Protocol-based VLAN User                   */
} vcl_proto_vlan_msg_cfg_t;

/**
 * \brief VCL MAC VLAN configuration table
 */
typedef struct {
    u32                        version;                         /**< Block version                              */
    u32                        count;                           /**< Number of entries                          */
    u32                        size;                            /**< Size of each entry                         */
    vcl_mac_vlan_flash_entry_t table[VCL_MAC_VLAN_MAX_ENTRIES]; /**< Entries                                    */
} vcl_mac_vlan_table_blk_t;

/**
 * \brief VCL Protocol VLAN protocol configuration table
 */
typedef struct {
    u32                             version;                                /**< Block version                              */
    u32                             count;                                  /**< Number of entries                          */
    u32                             size;                                   /**< Size of each entry                         */
    vcl_proto_vlan_proto_entry_t    table[VCL_PROTO_VLAN_MAX_PROTOCOLS];    /**< Entries                                    */
} vcl_proto_vlan_proto_table_blk_t;

/**
 * \brief VCL Protocol_VLAN VLAN configuration table
 */
typedef struct {
    u32                             version;                                /**< Block version                              */
    u32                             count;                                  /**< Number of entries                          */
    u32                             size;                                   /**< Size of each entry                         */
    vcl_proto_vlan_flash_entry_t    table[VCL_PROTO_VLAN_MAX_HW_ENTRIES];   /**< Entries                                    */
} vcl_proto_vlan_vlan_table_blk_t;

/**
 * \brief VCL IP Subnet-based VLAN configuration table
 */
typedef struct {
    u32                             version;                                /**< Block version                              */
    u32                             count;                                  /**< Number of entries                          */
    u32                             size;                                   /**< Size of each entry                         */
    vcl_ip_vlan_entry_t             table[VCL_IP_VLAN_MAX_ENTRIES];         /**< Entries                                    */
} vcl_ip_vlan_table_blk_t;

/* ================================================================= *
 *  VCL stack messages
 * ================================================================= */
/**
 * \brief VCL messages IDs
 */
typedef enum {
    VCL_MAC_VLAN_MSG_ID_CONF_SET_REQ,                           /**< VCL MAC-based VLAN configuration set request          */
    VCL_MAC_VLAN_MSG_ID_CONF_ADD_REQ,                           /**< VCL MAC-based VLAN configuration add request          */
    VCL_MAC_VLAN_MSG_ID_CONF_DEL_REQ,                           /**< VCL MAC-based VLAN configuration delete request       */
    VCL_PROTO_VLAN_MSG_ID_CONF_SET_REQ,                         /**< VCL protocol-based VLAN configuration set request     */
    VCL_PROTO_VLAN_MSG_ID_CONF_ADD_REQ,                         /**< VCL protocol-based VLAN configuration add request     */
    VCL_PROTO_VLAN_MSG_ID_CONF_DEL_REQ,                         /**< VCL protocol-based VLAN configuration delete request  */
    VCL_IP_VLAN_MSG_ID_CONF_SET_REQ,                            /**< VCL IP-based VLAN configuration set request           */
    VCL_IP_VLAN_MSG_ID_CONF_ADD_REQ,                            /**< VCL IP-based VLAN configuration add request           */
    VCL_IP_VLAN_MSG_ID_CONF_DEL_REQ,                            /**< VCL IP-based VLAN configuration delete request        */
} vcl_msg_id_t;

/**
 * \brief Message structure to set the MAC-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t              msg_id;                           /**< Message ID                             */
    u32                       count;                            /**< Number of entries                      */
    vcl_mac_vlan_msg_cfg_t    conf[VCL_MAC_VLAN_MAX_ENTRIES];   /**< Configuration                          */
} vcl_msg_conf_mac_vlan_set_req_t;

/**
 * \brief Message structure to add or delete the MAC-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t              msg_id;                           /**< Message ID                                 */
    vcl_mac_vlan_msg_cfg_t    conf;                             /**< Configuration                              */
} vcl_msg_conf_mac_vlan_add_del_req_t;

/**
 * \brief Message structure to set the Protocol-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t              msg_id;                               /**< Message ID                             */
    u32                       count;                                /**< Number of entries                      */
    vcl_proto_vlan_msg_cfg_t  conf[VCL_PROTO_VLAN_MAX_PROTOCOLS];   /**< Configuration                          */
} vcl_msg_conf_proto_vlan_set_req_t;

/**
 * \brief Message structure to add or delete the Protocol-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t              msg_id;                           /**< Message ID                                 */
    vcl_proto_vlan_msg_cfg_t  conf;                             /**< Configuration                              */
} vcl_msg_conf_proto_vlan_add_del_req_t;

/**
 * \brief Message structure to set the IP Subnet-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t              msg_id;                           /**< Message ID                             */
    u32                       count;                            /**< Number of entries                      */
    vcl_ip_vlan_msg_cfg_t     conf[VCL_IP_VLAN_MAX_ENTRIES];    /**< Configuration                          */
} vcl_msg_conf_ip_vlan_set_req_t;

/**
 * \brief Message structure to add or delete the IP Subnet-based VLAN entries in a stack.
 */
typedef struct {
    vcl_msg_id_t             msg_id;                           /**< Message ID                                  */
    vcl_ip_vlan_msg_cfg_t    conf;                             /**< Configuration                               */
} vcl_msg_conf_ip_vlan_add_del_req_t;

/**
 * \brief Request message used in stack
 */
typedef struct {
    union {
        vcl_msg_conf_mac_vlan_set_req_t          a;             /**< MAC VCL SET request message                       */
        vcl_msg_conf_mac_vlan_add_del_req_t      b;             /**< MAC VCL ADD/DELETE request message                */
        vcl_msg_conf_proto_vlan_set_req_t        c;             /**< PROTO VCL SET request message                     */
        vcl_msg_conf_proto_vlan_add_del_req_t    d;             /**< PROTO VCL ADD/DELETE request message              */
        vcl_msg_conf_ip_vlan_set_req_t           e;             /**< IP VCL SET request message                        */
        vcl_msg_conf_ip_vlan_add_del_req_t       f;             /**< IP VCL ADD/DELETE request message                 */
    } data;
} vcl_msg_req_t;

/**
 *  \brief  VCL module init.
 *  This function initializes the VCL module during startup.
 *
 *  \param data [IN]:      Pointer to vtss_init_data_t structure that contains command, isid
 *                         and other information.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_init(vtss_init_data_t *data);

/**
 *  \brief  VCL error type to string function.
 *  This function converts MAC-based VLAN error type to string form.
 *
 *  \param rc             [IN]: VCL error type.
 *
 *  \return
 *  pointer to MAC-based VLAN error string.
 */
char *vcl_error_txt(vtss_rc rc);

/**
 *  \brief  VCL MAC-based VLAN entry add function.
 *  This function adds an entry into MAC-based VLAN table. It resolves conflicts if multiple
 *  users configure the MAC-based VLAN entry with same MAC address and different VID or ports.
 *  It also adds the entry into flash if the STATIC user configures the entry.
 *
 *  \param isid           [IN]: Internal Swith ID.
 *
 *  \param mac_vlan_entry [IN]: Pointer to the vcl_mac_vlan_mgmt_entry_t structure. It contains
 *                              MAC address, VID and ports.
 *
 *  \param user           [IN]: MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_mac_vlan_mgmt_mac_vlan_add(vtss_isid_t                  isid,
                                       vcl_mac_vlan_mgmt_entry_t    *mac_vlan_entry,
                                       vcl_mac_vlan_user_t          user);

/**
 *  \brief  VCL MAC-based VLAN entry delete function.
 *  This function deletes user's MAC-based VLAN configuration in the MAC-based VLAN table. If
 *  there is only one user configured, it deletes the entry or if multiple users are configured,
 *  it resolves conflicts and modifies the entry.
 *
 *  \param isid           [IN]: Internal Swith ID.
 *
 *  \param mac_addr       [IN]: Pointer to the MAC address.
 *
 *  \param user           [IN]: MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_mac_vlan_mgmt_mac_vlan_del(vtss_isid_t          isid,
                                       vtss_mac_t           *mac_addr,
                                       vcl_mac_vlan_user_t  user);

/**
 *  \brief  VCL MAC-based VLAN entry lookup function.
 *  This function looks up an entry in the MAC-based VLAN table for the user.
 *
 *  \param isid           [IN]:     Internal Swith ID.
 *
 *  \param mac_vlan_entry [INOUT]:  Pointer to the vcl_mac_vlan_mgmt_entry_get_cfg_t. This function
 *                                  should have MAC address as input that is part of the mac_vlan_entry
 *                                  structure. This structure is filled with MAC-based VLAN entry
 *                                  information on output.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the MAC
 *                                  address above.
 *
 *  \param first          [IN]:     first entry or not.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_mac_vlan_mgmt_mac_vlan_get(vtss_isid_t                          isid,
                                       vcl_mac_vlan_mgmt_entry_get_cfg_t    *mac_vlan_entry,
                                       vcl_mac_vlan_user_t user,
                                       BOOL next,
                                       BOOL first);

/**
 *  \brief  VCL MAC-based VLAN entry lookup function.
 *  This function looks up an entry in the MAC-based VLAN table for the user.
 *
 *  \param isid           [IN]:     Internal Swith ID.
 *
 *  \param index          [IN]:     MAC-based table index.
 *
 *  \param mac_vlan_entry [OUT]:    Pointer to the vcl_mac_vlan_mgmt_entry_get_cfg_t. This structure is
 *                                  filled with MAC-based VLAN entry information on output.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_mac_vlan_mgmt_mac_vlan_get_by_key(vtss_isid_t isid,
                                              u32 index,
                                              vcl_mac_vlan_mgmt_entry_get_cfg_t *mac_vlan_entry,
                                              vcl_mac_vlan_user_t user);

/**
 *  \brief  Debug function to get Local Mac-based VLAN entries.
 *  This is a debug function to fetch MAC-based VLAN entries.
 *
 *  \param mac_vlan_entry [OUT]:    Pointer to the vcl_mac_vlan_mgmt_entry_t. This structure is
 *                                  filled with MAC-based VLAN entry information on output.
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the MAC
 *                                  address above.
 *
 *  \param first          [IN]:     first entry or not.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */

vtss_rc vcl_mac_vlan_mgmt_local_isid_mac_vlan_get(vcl_mac_vlan_mgmt_entry_t *mac_vlan_entry,
                                                  BOOL next,
                                                  BOOL first);

/**
 *  \brief  VCL MAC-based user to string function.
 *  This function converts MAC-based VLAN user to string form.
 *
 *  \param user           [IN]: MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  pointer to MAC-based VLAN user string.
 */
char *vcl_mac_vlan_mgmt_vcl_user_to_txt(vcl_mac_vlan_user_t usr);

/**
 *  \brief  VCL Protocol-based VLAN entry Protocol to Group add function.
 *  This function adds Protocol to Group mapping. It also adds the entry
 *  into flash for the STATIC user.
 *
 *  \param proto_grp      [IN]: Pointer to the vcl_proto_vlan_proto_entry_t structure.
 *
 *  \param user           [IN]: Protocol-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_proto_vlan_mgmt_proto_add(vcl_proto_vlan_proto_entry_t     *proto_grp,
                                      vcl_proto_vlan_user_t            user);

/**
 *  \brief  VCL Protocol-based VLAN entry Protocol to Group delete function.
 *  This function deletes Protocol to Group mapping. It also deletes the entry
 *  the flash for the STATIC user.
 *
 *  \param proto_grp      [IN]: Pointer to the vcl_proto_vlan_proto_entry_t structure.
 *
 *  \param user           [IN]: Protocol-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_proto_vlan_mgmt_proto_delete(vcl_proto_encap_type_t  proto_encap_type,
                                         vcl_proto_conf_t        *proto,
                                         vcl_proto_vlan_user_t   user);

/**
 *  \brief  VCL Protocol-based VLAN entry Protocol to Group Lookup function.
 *  This function fetches Protocol to Group mapping.
 *
 *  \param proto_grp      [IN]: Pointer to the vcl_proto_vlan_proto_entry_t structure.
 *
 *  \param user           [IN]: Protocol-based VLAN user (either static or volatile).
 *
 *  \param next           [IN]: Next entry flag
 *
 *  \param first          [IN]: first entry flag.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_proto_vlan_mgmt_proto_get(vcl_proto_vlan_proto_entry_t     *proto_grp,
                                      vcl_proto_vlan_user_t                 user,
                                      BOOL                                  next,
                                      BOOL                                  first);

vtss_rc vcl_proto_vlan_mgmt_group_entry_add(vtss_isid_t                       isid_add,
                                            vcl_proto_vlan_vlan_entry_t       *entry,
                                            vcl_proto_vlan_user_t             user);
vtss_rc vcl_proto_vlan_mgmt_group_entry_delete(vtss_isid_t                       isid_del,
                                               vcl_proto_vlan_vlan_entry_t       *entry,
                                               vcl_proto_vlan_user_t             user);
vtss_rc vcl_proto_vlan_mgmt_group_entry_get(vtss_isid_t                       isid_get,
                                            vcl_proto_vlan_vlan_entry_t       *entry,
                                            vcl_proto_vlan_user_t             user,
                                            BOOL                              next,
                                            BOOL                              first);
vtss_rc vcl_proto_vlan_mgmt_group_entry_get_by_vlan(vtss_isid_t                       isid_get,
                                                    vcl_proto_vlan_vlan_entry_t       *entry,
                                                    vcl_proto_vlan_user_t             user,
                                                    BOOL                              next,
                                                    BOOL                              first);
vtss_rc vcl_proto_vlan_mgmt_local_entry_get(vcl_proto_vlan_local_sid_conf_t *entry, u32 id, BOOL next);
vtss_rc vcl_proto_vlan_mgmt_hw_entry_get(vtss_isid_t                       isid_get,
                                         vcl_proto_vlan_entry_t            *entry,
                                         u32                               id,
                                         vcl_proto_vlan_user_t             user,
                                         BOOL                              next);
/**
 *  \brief  Protocol Type to text.
 **/
char *vcl_proto_vlan_mgmt_proto_type_to_txt(vcl_proto_encap_type_t encap);

/**
 *  \brief  VCL IP-based VLAN entry add function.
 *  This function adds an entry into IP-based VLAN table. It also adds the entry into flash
 *  if the STATIC user configures the entry.
 *
 *  \param isid           [IN]: Internal Swith ID.
 *
 *  \param ip_vlan_entry  [IN]: Pointer to the vcl_ip_vlan_mgmt_entry_t structure. It contains
 *                              IP address, mask length, VID and ports.
 *
 *  \param user           [IN]: IP-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_ip_vlan_mgmt_ip_vlan_add(vtss_isid_t                  isid,
                                     vcl_ip_vlan_mgmt_entry_t     *ip_vlan_entry,
                                     vcl_ip_vlan_user_t           user);

/**
 *  \brief  VCL IP-based VLAN entry delete function.
 *  This function deletes user's IP-based VLAN configuration in the IP-based VLAN table.
 *
 *  \param isid           [IN]: Internal Swith ID.
 *
 *  \param ip_vlan_entry  [IN]: Pointer to the vcl_ip_vlan_mgmt_entry_t structure. It contains
 *                              IP address, mask and ports.
 *
 *  \param user           [IN]: IP-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_ip_vlan_mgmt_ip_vlan_del(vtss_isid_t                isid,
                                     vcl_ip_vlan_mgmt_entry_t   *ip_vlan_entry,
                                     vcl_ip_vlan_user_t         user);

/**
 *  \brief  VCL IP-based VLAN entry lookup function.
 *  This function looks up an entry in the IP-based VLAN table for the user.
 *
 *  \param isid           [IN]:     Internal Swith ID.
 *
 *  \param ip_vlan_entry  [INOUT]:  Pointer to the vcl_ip_vlan_mgmt_entry_t. This function
 *                                  should have vce_id as input that is part of the ip_vlan_entry
 *                                  structure. This structure is filled with IP-based VLAN entry
 *                                  information on output.
 *
 *  \param user           [IN]:     IP-based VLAN user (either static or volatile).
 *
 *  \param first          [IN]:     flag to indicate to fetch first entry.
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the IP
 *                                  address above.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_ip_vlan_mgmt_ip_vlan_get(vtss_isid_t                          isid,
                                     vcl_ip_vlan_mgmt_entry_t             *ip_vlan_entry,
                                     vcl_ip_vlan_user_t                   user,
                                     BOOL                                 first,
                                     BOOL                                 next);
/**
 *  \brief  VCL IP-based VLAN entry lookup function.
 *  This function looks up an entry in the IP-based VLAN table for a specific vce_id.
 *
 *  \param isid           [IN]:     Internal Swith ID.
 *
 *  \param ip_vlan_entry  [INOUT]:  Pointer to the vcl_ip_vlan_mgmt_entry_t. This structure is filled with IP-based VLAN entry
 *                                  information on output.
 *
 *  \param user           [IN]:     IP-based VLAN user (either static or volatile).
 *
 *  \vce_id               [IN]:     VCE id for the entry to get.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
vtss_rc vcl_ip_vlan_mgmt_ip_vce_id_get(vtss_isid_t                          isid,
                                       vcl_ip_vlan_mgmt_entry_t             *ip_vlan_entry,
                                       vcl_ip_vlan_user_t                   user,
                                       u16                                  vce_id);

/**
 * \brief VCL debug command for setting the policy number.
 * This function sets the policy number that is used in subsequent VCL add commands.
 * Works on local switch only (no stack support).
 *
 *  \param policy_no      [IN]:     Policy number. 0..( VTSS_ACL_POLICIES - 1) or VTSS_ACL_POLICY_NO_NONE
 *
 *  \return
 *  VTSS_RC_OK on success.
 */

vtss_rc vcl_debug_policy_no_set(vtss_acl_policy_no_t policy_no);

/**
 * \brief VCL debug command for getting the policy number.
 * This function gets the currently configured policy number that is used in subsequent VCL add commands.
 * Works on local switch only (no stack support).
 *
 *  \param policy_no      [OUT]:    Policy number. 0..( VTSS_ACL_POLICIES - 1) or VTSS_ACL_POLICY_NO_NONE
 *
 *  \return
 *  VTSS_RC_OK on success.
 */

vtss_rc vcl_debug_policy_no_get(vtss_acl_policy_no_t *policy_no);

#endif /* _VCL_API_H_ */
