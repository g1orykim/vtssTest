/*

   Vitesse VCL software.

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

#ifndef _VTSS_VCL_H_
#define _VTSS_VCL_H_

/**
 * \file vtss_vcl.h
 * \brief VCL base header file
 *
 * This file contains the definitions of API functions and associated types.
 *
 */

#include "main.h"                                                       /**< For VTSS common defintions                     */
#include "l2proto_api.h"                                                /**< Definition for L2_MAX_PORTS                    */

#define     VTSS_VCL_RC_OK                              0
#define     VTSS_VCL_ERROR_PARM                         1               /**< Illegal parameter                              */
#define     VTSS_VCL_ERROR_CONFIG_NOT_OPEN              2               /**< Configuration open error                       */
#define     VTSS_VCL_ERROR_ENTRY_NOT_FOUND              3               /**< Entry not found                                */
#define     VTSS_VCL_ERROR_TABLE_EMPTY                  4               /**< Table empty                                    */
#define     VTSS_VCL_ERROR_TABLE_FULL                   5               /**< Table full                                     */
#define     VTSS_VCL_ERROR_REG_TABLE_FULL               6               /**< Registration table full                        */
#define     VTSS_VCL_ERROR_STACK_STATE                  7               /**< Illegal MASTER/SLAVE state                     */
#define     VTSS_VCL_ERROR_USER_PREVIOUSLY_CONFIGURED   8               /**< Previously configured                          */
#define     VTSS_VCL_ERROR_VCE_ID_PREVIOUSLY_CONFIGURED 9               /**< VCE ID previously configured                   */
#define     VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED  10              /**< Entry previously configured                    */
#define     VTSS_VCL_ERROR_ENTRY_WITH_DIFF_NAME         11              /**< Entry previously configured with different name */
#define     VTSS_VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN    12              /**< Entry previously configured with different name and vlan */
#define     VTSS_VCL_ERROR_ENTRY_WITH_DIFF_SUBNET       13              /**< Entry previously configured with different subnet */
#define     VTSS_VCL_ERROR_ENTRY_WITH_DIFF_VLAN         14              /**< Entry previously configured with different VLAN */

#define     VCL_BLK_VERSION                             0               /**< VCL flash table version                        */
#define     VTSS_L2PORT_BF_SIZE                         ((L2_MAX_PORTS+7)/8)  /* Bitfield reqd to store all L2 ports        */
/* MAC-based VLAN macros */
#define     VCL_MAC_VLAN_MAX_ENTRIES                    256             /**< Maximum number of MAC-based VCL entries        */

/* Protocol-based VLAN macros */
#define     MAX_GROUP_NAME_LEN                          16
#define     MAX_PROTO_LEN                               5
#define     VCL_PROTO_VLAN_MAX_GROUPS                   64
#define     VCL_PROTO_VLAN_TOTAL_GROUPS                 (VTSS_ISID_CNT * VCL_PROTO_VLAN_MAX_GROUPS)
#define     VCL_PROTO_VLAN_MAX_PROTOCOLS                128
#define     VCL_PROTO_VLAN_MAX_HW_ENTRIES               (VTSS_ISID_CNT * VCL_PROTO_VLAN_MAX_PROTOCOLS)
#define     VCL_MAC_VLAN_INVALID_INDEX                  0xFFFF
#define     OUI_SIZE                                    3

/* IP Subnet-based VLAN macros */
#define     VCL_NAME_MAX_LEN                            17
#define     VCL_IP_VLAN_MAX_ENTRIES                     128             /**< Maximum number of IP Subnet-based VCL entries  */

#define     VCE_ID_NONE                                 0               /**< Reserved VCE ID                                */
#define     VCE_ID_START                                1               /**< First VCE ID                                   */
#define     VCE_ID_END                                  VCL_PROTO_VLAN_MAX_HW_ENTRIES  /**< Last VCE ID                     */
/* ======================================================================
 *  VCE ID definitions
 *  we will use a 8-bit users ID and a 16-bit VCE ID to identify VCE.
 *  The VTSS API used a 32-bits VCE ID. These three values could
 *  be combined when accessing the switch API. This would give each VCL
 *  user a separate ID space used for its own entries.
 * ====================================================================== */
#define VCL_USER_ID_GET(vce_id)               (((vce_id) >> 16) & 0xFF)
#define VCL_VCE_ID_GET(vce_id)                ((vce_id) & 0xFFFF)
#define VCL_COMBINED_ID_SET(user_id, vce_id)  ((((user_id) & 0xFF)<< 16) + ((vce_id) & 0xFFFF))

/**
 *  \brief This enum defines various VCL types like MAC and PROTO VCLs. Do Not Change the position.
 *         This will be used to get vce_id_next for the entries without vce_id_next.
 */
typedef enum {
    VCL_TYPE_PROTO = 0,
    VCL_TYPE_IP,
    VCL_TYPE_MAC,
    VCL_TYPE_LAST
} vcl_user_t;

/**
 *  \brief This enum defines various MAC-based VLAN Users. Do Not Change the position
 *         of static VLAN User. Except static user, the MAC VLAN User that comes first
 *         in the enum has more priority for resolving conflicts.
 */
typedef enum {
    VCL_MAC_VLAN_USER_STATIC = 0,                                       /**< VCL MAC VLAN User is Static                    */
#ifdef VTSS_SW_OPTION_DOT1X
    VCL_MAC_VLAN_USER_NAS,                                              /**< VCL MAC VLAN User is NAS                       */
#endif
    VCL_MAC_VLAN_USER_ALL                                               /**< VCL MAC VLAN User combined                     */
} vcl_mac_vlan_user_t;

/**
 *  \brief This enum defines various Protocol-based VLAN Users. Do Not Change the position
 *         of static VLAN User. Currently there is only one user but more users may be added
 *         in future as and when required.
 */
typedef enum {
    VCL_PROTO_VLAN_USER_STATIC = 0,                                     /**< VCL Protocol VLAN User is Static               */
    VCL_PROTO_VLAN_USER_ALL                                             /**< VCL Protocol VLAN User combined                */
} vcl_proto_vlan_user_t;

/**
 *  \brief This enum defines various IP-based VLAN Users. Do Not Change the position
 *         of static VLAN User. Currently there is only one user but more users may be added
 *         in future as and when required.
 */
typedef enum {
    VCL_IP_VLAN_USER_STATIC = 0,                                     /**< VCL IP VLAN User is Static               */
    VCL_IP_VLAN_USER_ALL                                             /**< VCL IP VLAN User combined                */
} vcl_ip_vlan_user_t;

/**
 * \brief MAC VLAN conflicting users are indicated by setting corresponding bit.
 */
typedef unsigned long vcl_mac_vlan_conflicting_users;

/**
 *  \brief This structure is used by MAC-based VLAN platform code to convey SMAC to VID and
 *         port mapping to the base code.
 */
typedef struct {
    vtss_mac_t  smac;                                                   /**< Source MAC Address                                 */
    vtss_vid_t  vid;                                                    /**< VLAN ID                                            */
    u8          l2ports[VTSS_L2PORT_BF_SIZE];                           /**< Ports bitfield on which SMAC based VLAN is enabled */
} vcl_mac_vlan_conf_entry_t;

/**
 *  \brief This structure is used by MAC-based VLAN platform code to convey SMAC to VID and
 *         port mapping to base code. This structure is only used to store the above mappings
 *         for single SID (Local SID).
 */
typedef struct {
    vtss_mac_t          mac;                                                /**< Source MAC Address                                 */
    vtss_vid_t          vid;                                                /**< VLAN ID                                            */
    u8                  ports[VTSS_PORT_BF_SIZE];                           /**< Ports bitfield on which SMAC based VLAN is enabled */
    vtss_vce_id_t       id;                                                 /**< VCE ID                                             */
    vcl_mac_vlan_user_t user;                                               /**< User                                               */
} vcl_mac_vlan_local_sid_conf_entry_t;

/**
 *  \brief This structure is used local switch's MAC-based VLAN configuration
 */
typedef struct {
    BOOL                                    valid;                      /**< Flag to indicate whether the entry is valid        */
    vcl_mac_vlan_local_sid_conf_entry_t     entry;                      /**< Local entry                                        */
} vcl_mac_vlan_local_sid_conf_t;

/**
 *  \brief This structure defines the MAC-based VLAN fields other than MAC address.
 */
typedef struct {
    BOOL            valid;                                              /**< Flag to indicate whether the entry is valid        */
    vtss_vid_t      vid;                                                /**< VLAN ID                                            */
    u8              l2ports[VTSS_L2PORT_BF_SIZE];                       /**< L2 ports bit field                                 */
} vcl_mac_vlan_conf_t;

/**
 *  \brief This structure is used by VCL module to store the SMAC to VID and port association for
 *         various MAC VLAN Users.
 */
typedef struct {
    vtss_mac_t            smac;                                         /**< Source MAC Address                                 */
    vcl_mac_vlan_conf_t   conf[VCL_MAC_VLAN_USER_ALL + 1];              /**< VID and port_list information for all VLAN users   */
} vcl_mac_vlan_entry_t;

/**
 *  \brief MAC-based VLAN Database
 */
typedef struct {
    vcl_mac_vlan_entry_t  mac_vlan_db[VCL_MAC_VLAN_MAX_ENTRIES];        /**< array of vcl_mac_vlan_entry_t structures           */
} vcl_mac_vlan_global_t;

/**
 *  \brief This structure forms an entry in a group database. It contains group name. Group ID
 *         is the index into the array.
 */
typedef struct {
    BOOL                    valid;                                      /**< Flag to indicate whether entry is valid            */
    u8                      group_name[MAX_GROUP_NAME_LEN];             /**< Group Name                                         */
} vcl_proto_vlan_group_db_entry_t;

/**
 * \brief The following three structures define protocol fields for different frame encapsulations
 */
typedef struct {
    u16 eth_type;                                                       /**< Ether Protocol Type                                */
} eth2_proto_t;

/**
 * \brief SNAP header.
 */
typedef struct {
    u8  oui[OUI_SIZE];                                                  /**< Organizationally Unique ID                         */
    u16 pid;                                                            /**< Protocol ID                                        */
} llc_snap_proto_t;

/**
 * \brief LLC header.
 */
typedef struct {
    u8 dsap;                                                            /**< Destionation SAP                                   */
    u8 ssap;                                                            /**< Source SAP                                         */
} llc_other_proto_t;

/**
 *  \brief This enum identifies Frame encapsulation Type.
 */
typedef enum {
    VCL_PROTO_ENCAP_ETH2 = 1,                                           /**< ETHERNET II encapsulation                          */
    VCL_PROTO_ENCAP_LLC_SNAP,                                           /**< LLC SNAP encapsulation                             */
    VCL_PROTO_ENCAP_LLC_OTHER                                           /**< LLC Other encapsulation that is not SNAP           */
} vcl_proto_encap_type_t;

typedef union {
    eth2_proto_t        eth2_proto;                                 /**< Protocol fields for Eth II encap                   */
    llc_snap_proto_t    llc_snap_proto;                             /**< Protocol fields for LLC SNAP encap                 */
    llc_other_proto_t   llc_other_proto;                            /**<Protocol fields for LLC other encap                 */
} vcl_proto_conf_t;

/**
 * \brief This structure is required to store the Protocol to Group mapping.
 */
typedef struct {
    vcl_proto_encap_type_t  proto_encap_type;                           /**< Encapsulation type                                 */
    vcl_proto_conf_t        proto;                                      /**< Protocol                                           */
    u8                      group_id[MAX_GROUP_NAME_LEN];               /**< Group ID                                           */
} vcl_proto_vlan_proto_entry_t;

typedef struct {
    u8                      group_id[MAX_GROUP_NAME_LEN];               /**< Group ID                                           */
    vtss_vid_t              vid;                                        /**< VLAN ID                                            */
    BOOL                    ports[VTSS_PORT_ARRAY_SIZE];                /**< Ports                                              */
} vcl_proto_vlan_vlan_entry_t;

typedef struct {
    vtss_isid_t             isid;                                       /**< ISID                                               */
    u8                      group_id[MAX_GROUP_NAME_LEN];               /**< Group ID                                           */
    vtss_vid_t              vid;                                        /**< VLAN ID                                            */
    BOOL                    ports[VTSS_PORT_BF_SIZE];                   /**< Ports                                              */
    vcl_proto_vlan_user_t   user;                                       /**< User                                               */
} vcl_proto_vlan_grp_entry_t;

typedef struct {
    vcl_proto_encap_type_t  proto_encap_type;                           /**< Encapsulation type                                 */
    vcl_proto_conf_t        proto;                                      /**< Protocol                                           */
    vtss_isid_t             isid;                                       /**< Mask for ISIDs                                     */
    u8                      ports[VTSS_PORT_BF_SIZE];                   /**< Ports BitField                                     */
    vtss_vid_t              vid;                                        /**< VLAN ID                                            */
    vtss_vce_id_t           vce_id;                                     /**< VCE ID                                             */
    vcl_proto_vlan_user_t   user;                                       /**< User                                               */
} vcl_proto_vlan_entry_t;

typedef struct {
    vtss_isid_t                     isid;                               /**< Mask for ISIDs                                     */
    vcl_proto_vlan_vlan_entry_t     conf;                               /**< Protocol group to VLAN conf                        */
} vcl_proto_vlan_flash_entry_t;

/**
 *  \brief This structure is used by Protocol-based VLAN platform code to convey protocol to VID and
 *         port mapping to base code. This structure is only used to store the above mappings
 *         for single SID (Local SID).
 */
typedef struct {
    vcl_proto_encap_type_t  proto_encap_type;                           /**< Encapsulation type                                 */
    vcl_proto_conf_t        proto;                                      /**< Protocol                                           */
    vtss_vid_t              vid;                                        /**< VLAN ID                                            */
    u8                      ports[VTSS_PORT_BF_SIZE];                   /**< Ports bitfield on which SMAC based VLAN is enabled */
    vtss_vce_id_t           id;                                         /**< VCE ID                                             */
    vcl_proto_vlan_user_t   user;                                       /**< User                                               */
} vcl_proto_vlan_local_sid_conf_t;


typedef struct vcl_proto_local_conf_t {
    struct vcl_proto_local_conf_t       *next;                          /**< Next in list                                       */
    vcl_proto_vlan_local_sid_conf_t     conf;                           /**< Local Protocol Configuration                       */
} vcl_proto_local_conf_t;

/* Protocol-based VLAN protocol entry */
typedef struct vcl_proto_grp_t {
    struct vcl_proto_grp_t              *next;                          /**< Next in list                                       */
    vcl_proto_vlan_grp_entry_t          conf;                           /**< Protocol Group Configuration                       */
} vcl_proto_grp_t;

/* Protocol-based VLAN protocol entry */
typedef struct vcl_proto_t {
    struct vcl_proto_t                  *next;                          /**< Next in list                                       */
    vcl_proto_vlan_proto_entry_t        conf;                           /**< Protocol Group Configuration                       */
} vcl_proto_t;

/* Protocol-based VLAN vlan entry */
typedef struct vcl_proto_hw_conf_t {
    struct vcl_proto_hw_conf_t         *next;                          /**< Next in list                                       */
    vcl_proto_vlan_entry_t             conf;                           /**< Full protocol Configuration                        */
} vcl_proto_hw_conf_t;

typedef struct {
    vcl_proto_grp_t          *free;                              /**< Free list                                          */
    vcl_proto_grp_t          *used;                              /**< Used list                                          */
} vcl_proto_grp_list_t;

typedef struct {
    vcl_proto_local_conf_t          *free;                              /**< Free list                                          */
    vcl_proto_local_conf_t          *used;                              /**< Used list                                          */
} vcl_proto_local_list_t;

/* Protocol-based VLAN lists */
typedef struct {
    vcl_proto_t                     *free;                              /**< Free list                                          */
    vcl_proto_t                     *used;                              /**< Used list                                          */
} vcl_proto_list_t;

typedef struct {
    vcl_proto_hw_conf_t                     *free;                              /**< Free list                                          */
    vcl_proto_hw_conf_t                     *used;                              /**< Used list                                          */
} vcl_proto_hw_conf_list_t;

/**
 *  \brief This structure contains the protocol-based VLAN db.
 */
typedef struct {
    /* Lists to maintain Protocol to Group mapping */
    vcl_proto_list_t                        proto_list;
    vcl_proto_t                             proto_db[VCL_PROTO_VLAN_MAX_PROTOCOLS];
    /* Lists to maintain Group to VID+ports mapping */
    vcl_proto_grp_list_t                    grp_list;
    vcl_proto_grp_t                         grp_db[VCL_PROTO_VLAN_TOTAL_GROUPS];
    /* List to maintain Protocol to VID, Ports mapping */
    vcl_proto_hw_conf_list_t                hw_list;
    vcl_proto_hw_conf_t                     hw_db[VCL_PROTO_VLAN_MAX_HW_ENTRIES];
    /* List to maintain Local switch configuration */
    vcl_proto_local_list_t                  local_list;
    vcl_proto_local_conf_t                  local_db[VCL_PROTO_VLAN_MAX_PROTOCOLS];
} vcl_proto_vlan_global_t;

/**
 *  \brief This structure is used by VCL module to store the IP Subnet to VID
 *         and port association in Flash and to store it in base data structures.
 */
typedef struct {
    vtss_vce_id_t   vce_id;                                         /**< VCE ID                                     */
    ulong           ip_addr;                                        /**< Source IP Address                          */
    u8              mask_len;                                       /**< Mask length                                */
    vtss_vid_t      vid;                                            /**< VLAN ID                                    */
    u8              l2ports[VTSS_L2PORT_BF_SIZE];                   /**< Ports bit map                              */
} vcl_ip_vlan_entry_t;

/**
 *  \brief IP Subnet-based VLAN list entry
 **/
typedef struct vcl_ip_t {
    struct vcl_ip_t         *next;                            /**< Next in list                              */
    vcl_ip_vlan_entry_t     conf;                             /**< IP Subnet-based VLAN  Configuration       */
} vcl_ip_t;

/**
 *  \brief IP Subnet-based VLAN lists
 **/
typedef struct {
    vcl_ip_t                *free;                            /**< Free list                                 */
    vcl_ip_t                *used;                            /**< Used list                                 */
} vcl_ip_list_t;

#if 0
/**
 *  \brief This structure is used by units in a stack to store IP subnet to VID and port mappings.
 */
typedef struct {
    ulong                   ip_addr;                          /**< IP Address                                         */
    u8                      mask_len;                         /**< Number of mask bits                                */
    vtss_vid_t              vid;                              /**< VLAN ID                                            */
    u8                      ports[VTSS_PORT_BF_SIZE];         /**< Ports bitfield on which SMAC based VLAN is enabled */
    /* vtss_vce_id_t           id;  */                        /**< VCE ID                                             */
} vcl_ip_vlan_local_sid_conf_t;
#endif
/**
 *  \brief This structure is used by VCL module to send the IP Subnet to VID and port association to slaves.
 */
typedef struct {
    vtss_vce_id_t            id;                               /**< VCE ID                                             */
    ulong                    ip_addr;                          /**< IP Address                                  */
    u8                       mask_len;                         /**< Mask length                                 */
    vtss_vid_t               vid;                              /**< VLAN ID                                     */
    BOOL                     ports[VTSS_PORT_BF_SIZE];         /**< Ports on which IP-based VLAN is enabled     */
} vcl_ip_vlan_msg_cfg_t;

/**
 * \brief This structure is used to store the local sid's subnet-based VLAN entry's configuration per ISID.
 */
typedef struct vcl_ip_local_conf_t {
    struct vcl_ip_local_conf_t       *next;                   /**< Next in list                                          */
    vcl_ip_vlan_msg_cfg_t            conf;                    /**< Local Subnet Configuration                            */
} vcl_ip_local_conf_t;

/**
 *  \brief IP Subnet-based VLAN local lists.
 */
typedef struct {
    vcl_ip_local_conf_t          *free;                       /**< Free list                                          */
    vcl_ip_local_conf_t          *used;                       /**< Used list                                          */
} vcl_ip_local_list_t;

/**
 *  \brief IP Subnet-based VLAN Database
 */
typedef struct {
    vcl_ip_list_t         ip_vlan_list;                        /**< used and free lists to maintain db        */
    vcl_ip_t              ip_vlan_db[VCL_IP_VLAN_MAX_ENTRIES]; /**< array of vcl_ip_vlan_entry_t structures   */
    vcl_ip_local_list_t   local_list;                          /**< used and free lists to maintain locl db   */
    vcl_ip_local_conf_t   local_db[VCL_IP_VLAN_MAX_ENTRIES];   /**< Local switch configuration                */
} vcl_ip_vlan_global_t;

/**
 *  \brief Structure to hold VCL global data.
 */
typedef struct {
    vcl_mac_vlan_global_t             vcl_mac_vlan_glb_data;                             /**< MAC VLAN Global data              */
    vcl_mac_vlan_local_sid_conf_t     vcl_mac_vlan_loc_data[VCL_MAC_VLAN_MAX_ENTRIES];   /**< MAC VLAN Local SID data           */
    vcl_proto_vlan_global_t           vcl_proto_vlan_glb_data;                           /**< PROTO VLAN Global data            */
    vcl_ip_vlan_global_t              vcl_ip_vlan_glb_data;                              /**< IP Subnet VLAN Global data        */
} vcl_global_data_t;

/*************************************************************************************/
/*  VCL base function prototypes                                                     */
/*************************************************************************************/
/**
 *  \brief  VCL MAC-based VLAN entry add base function.
 *  This function adds an entry into MAC-based VLAN database.
 *
 *  \param mac_vlan_entry [IN]:     Pointer to the vcl_mac_vlan_conf_entry_t structure. It contains
 *                                  MAC address, VID and L2 ports.
 *
 *  \param id             [OUT]:    VCE ID to pass to the Switch API for adding in the HW table.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_VCL_RC_OK on success.
 */
u32 vcl_mac_vlan_entry_add(vcl_mac_vlan_conf_entry_t *mac_vlan_entry,
                           vtss_vce_id_t *id,
                           vcl_mac_vlan_user_t user);

/**
 *  \brief  VCL MAC-based VLAN entry delete base function.
 *  This function deletes MAC-based VLAN entry in the database.
 *
 *  \param mac_vlan_entry [INOUT]:  Pointer to the vcl_mac_vlan_conf_entry_t structure. It contains
 *                                  MAC address, L2 ports.
 *
 *  \param id             [OUT]:    VCE ID to pass to the Switch API for deleting in the HW table.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_VCL_RC_OK on success.
 */
u32 vcl_mac_vlan_entry_del(vcl_mac_vlan_conf_entry_t *mac_vlan_entry,
                           vtss_vce_id_t *id,
                           vcl_mac_vlan_user_t user);

/**
 *  \brief  VCL MAC-based VLAN entry lookup base function.
 *  This function looks up an entry in the MAC-based VLAN database for the user.
 *
 *  \param smac           [IN]:     Pointer to the MAC address for which entry needs to be looked up.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the MAC
 *                                  address above.
 *
 *  \param first          [IN]:     first entry or not.
 *
 *  \param mac_vlan_entry [OUT]:    Pointer to the vcl_mac_vlan_conf_entry_t structure. It contains
 *                                  MAC address, VID and L2 ports.
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_mac_vlan_entry_get(vtss_mac_t *smac,
                           vcl_mac_vlan_user_t user,
                           BOOL next,
                           BOOL first,
                           vcl_mac_vlan_conf_entry_t *mac_vlan_entry);

/**
 *  \brief  VCL MAC-based VLAN entry lookup base function.
 *  This function looks up an entry in the MAC-based VLAN database for the user.
 *
 *  \param index          [IN]:     MAC-based VLAN table index.
 *
 *  \param user           [IN]:     MAC-based VLAN user (either static or volatile).
 *
 *  \param mac_vlan_entry [OUT]:    Pointer to the vcl_mac_vlan_conf_entry_t structure. It contains
 *                                  MAC address, VID and L2 ports.
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_mac_vlan_entry_get_by_key(u32 index,
                                  vcl_mac_vlan_user_t user,
                                  vcl_mac_vlan_conf_entry_t *mac_vlan_entry);

/**
 *  \brief  VCL Protocol-based VLAN entry add Protocol base function.
 *  This function adds a protocol entry into Protocol-based VLAN database.
 *
 *  \return
 *  VTSS_VCL_RC_OK on success.
 */
u32 vcl_proto_vlan_proto_entry_add(vcl_proto_vlan_proto_entry_t     *proto_entry,
                                   vcl_proto_vlan_user_t            user);

u32 vcl_proto_vlan_proto_entry_delete(vcl_proto_encap_type_t        proto_encap_type,
                                      vcl_proto_conf_t              *proto,
                                      vcl_proto_vlan_user_t         user);

u32 vcl_proto_vlan_proto_entry_get(vcl_proto_vlan_proto_entry_t     *proto_entry,
                                   vcl_proto_vlan_user_t            user,
                                   BOOL                             next,
                                   BOOL                             first);
u32 vcl_proto_vlan_group_entry_add(vtss_isid_t                       isid_add,
                                   vcl_proto_vlan_vlan_entry_t       *entry,
                                   vcl_proto_vlan_user_t             user);
u32 vcl_proto_vlan_group_entry_delete(vtss_isid_t                       isid_del,
                                      vcl_proto_vlan_vlan_entry_t       *entry,
                                      vcl_proto_vlan_user_t             user);
u32 vcl_proto_vlan_group_entry_get(vtss_isid_t                       isid_get,
                                   vcl_proto_vlan_vlan_entry_t       *entry,
                                   vcl_proto_vlan_user_t             user,
                                   BOOL                              next,
                                   BOOL                              first);
u32 vcl_proto_vlan_group_entry_get_by_vlan(vtss_isid_t                       isid_get,
                                           vcl_proto_vlan_vlan_entry_t       *entry,
                                           vcl_proto_vlan_user_t             user,
                                           BOOL                              next,
                                           BOOL                              first);
u32 vcl_proto_vlan_local_entry_add(vcl_proto_vlan_local_sid_conf_t *entry);
u32 vcl_proto_vlan_local_entry_delete(u32 id);
u32 vcl_proto_vlan_local_entry_get(vcl_proto_vlan_local_sid_conf_t *entry, u32 id, BOOL next);
u32 vcl_proto_vlan_hw_entry_add(vcl_proto_vlan_entry_t *entry);
u32 vcl_proto_vlan_hw_entry_delete_by_protocol(vcl_proto_encap_type_t proto_encap_type, vcl_proto_conf_t *proto);
u32 vcl_proto_vlan_hw_entry_get(vtss_isid_t                       isid_get,
                                vcl_proto_vlan_entry_t            *entry,
                                u32                               id,
                                vcl_proto_vlan_user_t             user,
                                BOOL                              next);
void vcl_proto_vlan_proto_default_set(void);
void vcl_proto_vlan_vlan_default_set(void);
void vcl_proto_vlan_first_vce_id_get(vtss_vce_id_t *id);
void vcl_ports2_port_bitfield(BOOL *ports, u8 *ports_bf);

/**
 *  \brief  VCL database init base function.
 *  This function sets memory used by the VCL database to zeros which is default value.
 *
 *  \return
 *  Nothing.
 */
void vcl_default_set(void);

/**
 *  \brief  VCL MAC-based VLAN database init base function.
 *  This function sets memory used by the VCL MAC-based VLAN database to zeros which is default
 *  value.
 *
 *  \return
 *  Nothing.
 */
void vcl_mac_vlan_default_set(void);

/**
 *  \brief  This function adds an entry into the local switch database.
 *  VCL MAC-based VLAN entry add function to add entry into local switch database.
 *
 *  \param entry          [IN]:    Pointer to the vcl_mac_vlan_local_sid_conf_entry_t structure.
 *                                 It contains MAC address, VID, L2 ports and VCE ID.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_mac_vlan_local_entry_add(vcl_mac_vlan_local_sid_conf_entry_t *entry);

/**
 *  \brief  This function deletes an entry from local switch database.
 *  VCL MAC-based VLAN entry delete function to delete the entry in the local switch database.
 *
 *  \param mac            [IN]:     MAC address.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_mac_vlan_local_entry_del(vtss_mac_t *mac);

/**
 *  \brief  This function looksup an entry in the local switch database.
 *  VCL MAC-based VLAN entry lookup function to lookup entry in the local switch database.
 *
 *  \param entry          [INOUT]:  Pointer to the vcl_mac_vlan_local_sid_conf_entry_t structure.
 *                                  It contains valid MAC address on input and valid MAC address,
 *                                  VID, L2 ports and VCE ID on output.
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the MAC
 *                                  address above.
 *
 *  \param first          [IN]:     first entry or not.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_mac_vlan_local_entry_get(vcl_mac_vlan_local_sid_conf_entry_t *entry,
                                 BOOL next,
                                 BOOL first);
/*IP subnet-VLAN base functions */
/**
 *  \brief  VCL IP subnet-based VLAN entry add base function.
 *  This function adds an entry into IP subnet-based VLAN database.
 *
 *  \param ip_vlan_entry [IN]:     Pointer to the vcl_ip_vlan_entry_t structure.
 *
 *  \param user          [IN]:     IP subnet-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_VCL_RC_OK on success.
 */
u32 vcl_ip_vlan_entry_add(vcl_ip_vlan_entry_t *ip_vlan_entry,
                          vcl_ip_vlan_user_t  user);

/**
 *  \brief  VCL IP subnet-based VLAN entry delete base function.
 *  This function deletes IP subnet-based VLAN entry in the database.
 *
 *  \param ip_vlan_entry [INOUT]:  Pointer to the vcl_ip_vlan_entry_t structure. It contains
 *                                 name and L2 ports as valid parameters.
 *
 *  \param id             [OUT]:   VCE ID to pass to the Switch API for deleting in the HW table.
 *
 *  \param user           [IN]:    IP subnet-based VLAN user (either static or volatile).
 *
 *  \return
 *  VTSS_VCL_RC_OK on success.
 */
u32 vcl_ip_vlan_entry_del(vcl_ip_vlan_entry_t *ip_vlan_entry,
                          vcl_ip_vlan_user_t  user);

/**
 *  \brief  VCL IP subnet-based VLAN entry lookup base function.
 *  This function looks up an entry in the IP subnet-based VLAN database for the user.
 *
 *  \param ip_vlan_entry  [INOUT]:  Pointer to the vcl_ip_vlan_entry_t structure. "name" is the input.
 *
 *  \param user           [IN]:     IP subnet-based VLAN user (either static or volatile).
 *
 *  \param first          [IN]:     first entry or not.
 *
 *  \param next           [IN]:     Next entry or not (Next valid entry after the entry with the IP
 *                                  subnet above.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_ip_vlan_entry_get(vcl_ip_vlan_entry_t  *ip_vlan_entry,
                          vcl_ip_vlan_user_t   user,
                          BOOL                 first,
                          BOOL                 next);

/**
 *  \brief  VCL IP VLAN function to set the IP-based VLAN data structures to default state.
 */
void vcl_ip_vlan_default_set(void);

/**
 *  \brief  VCL IP VLAN local list add function.
 *
 *  \param  entry         [IN]: pointer to the vcl_ip_vlan_msg_cfg_t structure that contains IP subnet,
 *                              VID and ports.
 *  \param  next_id      [OUT]: pointer to the next VCE ID.
 *
 */
u32 vcl_ip_vlan_local_entry_add(vcl_ip_vlan_msg_cfg_t *entry, vtss_vce_id_t *next_id);

/**
 *  \brief  VCL IP VLAN local list delete function.
 *
 *  \param  entry         [IN]: pointer to the vcl_ip_vlan_msg_cfg_t structure that contains IP subnet,
 *                              VID and ports.
 */
u32 vcl_ip_vlan_local_entry_delete(vcl_ip_vlan_msg_cfg_t *entry);

/**
 *  \brief  VCL IP VLAN local list delete function.
 *
 *  \param  entry         [OUT]: pointer to the vcl_ip_vlan_msg_cfg_t structure.
 *
 *  \param  first         [IN]:  first entry or not.
 *
 *  \param  next          [IN]:  next entry or not.
 *
 */
u32 vcl_ip_vlan_local_entry_get(vcl_ip_vlan_msg_cfg_t *entry, BOOL first, BOOL next);

/**
 *  \brief  Get unique VCE ID across platform.
 *
 *  \param  vce_id        [OUT]: pointer to VCE ID.
 *
 *  \return
 *  VTSS_RC_OK on success.
 */
u32 vcl_ip_vlan_vce_id_get(u32 *vce_id);

/**
 *  \brief  Get first VCE ID configured.
 *
 *  \param  id            [OUT]: pointer to VCE ID.
 *
 */
void vcl_ip_vlan_first_vce_id_get(vtss_vce_id_t *id);

/**
 *  \brief  Convert Mask length to mask.
 *
 *  \param  mask_len      [IN]:  mask length.
 *
 *  \param  ip_mask       [OUT]: mask.
 *
 */
void ip_mask_len_2_mask(u8 mask_len, ulong *ip_mask);
/*************************************************************************************/
/*  VCL platform call out interface                                                  */
/*************************************************************************************/

/**
 *  \brief  VCL database lock function.
 *  This function locks the crititical section.
 *
 *  \return
 *  Nothing.
 */
void vtss_vcl_crit_data_lock(void);

/**
 *  \brief  VCL database unlock function.
 *  This function unlocks the crititical section.
 *
 *  \return
 *  Nothing.
 */
void vtss_vcl_crit_data_unlock(void);

/**
 *  \brief  VCL database lock assert function.
 *  This function checks for crititical section lock. If it is not already taken by the function
 *  calling this function, it asserts.
 *
 *  \return
 *  Nothing.
 */
void vtss_vcl_crit_data_assert_locked(void);

/**
 * \brief  Send a stack message to add the HW entry just created
 */
u32 vcl_stack_vcl_proto_vlan_conf_add_del(vtss_isid_t isid, vcl_proto_vlan_entry_t *conf, BOOL add);
#endif /* _VTSS_VCL_H_ */
