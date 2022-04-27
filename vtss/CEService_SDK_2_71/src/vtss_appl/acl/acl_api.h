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

 $Id$
 $Revision$

*/

#ifndef _VTSS_ACL_API_H_
#define _VTSS_ACL_API_H_

/**
 * \file acl_api.h
 * \brief This file defines the APIs for the ACL module
 */

#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */
#include "main.h"     /* For MODULE_ERROR_START()      */
#if defined(VTSS_SW_OPTION_EVC)
#include "evc_api.h"
#endif /* VTSS_SW_OPTION_EVC */

#define ACL_IPV6_SUPPORTED // if support IPv6 ACE configuration

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    ACL_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_ACL), /**< Generic error code              */
    ACL_ERROR_ISID_NON_EXISTING,                            /**< isid parameter is non-existing. */
    ACL_ERROR_PARM,                                         /**< Illegal parameter               */
    ACL_ERROR_REQ_TIMEOUT,                                  /**< Timeout on message request      */
    ACL_ERROR_STACK_STATE,                                  /**< Illegal MASTER/SLAVE state      */
    ACL_ERROR_ACE_NOT_FOUND,                                /**< ACE not found                   */
    ACL_ERROR_ACE_TABLE_FULL,                               /**< ACE table full                  */
    ACL_ERROR_USER_NOT_FOUND,                               /**< ACL user ID not found           */
    ACL_ERROR_MEM_ALLOC_FAIL,                               /**< Allocate memory fail            */
    ACL_ERROR_ACE_AUTO_ASSIGNED_FAIL,                       /**< ACE auto-assigned fail          */
    ACL_ERROR_UNKNOWN_ACE_TYPE                              /**< Unknown ACE type                */
};

/**
 * \brief ACL Action
 */
typedef struct {
    vtss_acl_policer_no_t   policer;                            /**< Policer number or VTSS_ACL_POLICY_NO_NONE */
#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
    BOOL                    evc_police;                         /**< Enable EVC policer */
    vtss_evc_policer_id_t   evc_policer_id;                     /**< EVC policer ID */
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_acl_port_action_t  port_action;                        /**< Port action */
    BOOL                    port_list[VTSS_PORT_ARRAY_SIZE];    /**< Egress port list */
    BOOL                    mirror;                             /**< Enable mirroring */
    vtss_acl_ptp_action_t   ptp_action;                         /**< PTP action */
#else
    vtss_port_no_t          port_no;                            /**< Port copy number or VTSS_PORT_NO_NONE */
    BOOL                    permit;                             /**< Permit/deny */
#endif /* VTSS_FEATURE_ACL_V2 */
    BOOL                    logging;                            /**< Logging */
    BOOL                    shutdown;                           /**< Port shut down */
    BOOL                    force_cpu;                          /**< Forward to CPU */
    BOOL                    cpu_once;                           /**< Only first frame forwarded to CPU */
    vtss_packet_rx_queue_t  cpu_queue;                          /**< CPU queue (if copied) */
#if defined(VTSS_ARCH_JAGUAR_1)
    BOOL                    irq_trigger;                        /**< Trigger interrupt against CPU */
#endif
#if defined(VTSS_ARCH_SERVAL)
    BOOL                    lm_cnt_disable;                     /**< Disable OAM LM Tx counting */
#endif /* VTSS_ARCH_SERVAL */
} acl_action_t;

/**
 * \brief ACL port configuration
 */
typedef struct {
    vtss_acl_policy_no_t    policy_no;          /**< Policy number */
    acl_action_t            action;             /**< Default action */
    BOOL                    sip_overloading;    /**< Source IP address overloading */
    BOOL                    smac_overloading;   /**< Source MAC address overloading */
} acl_port_conf_t;

/**
 * \brief ACL policer configuration
 */
typedef struct {
#if defined(VTSS_FEATURE_ACL_V2)
    BOOL               bit_rate_enable; /**< Use bit rate policing instead of packet rate */
    vtss_bitrate_t     bit_rate;        /**< Bit rate */
#endif /* VTSS_FEATURE_ACL_V2 */
    vtss_packet_rate_t packet_rate;     /**< Packet rate */
} acl_policer_conf_t;


/**
 * \brief ACE bit flags
 */
enum {
    ACE_FLAG_DMAC_BC = 0,   /**> DMAC with broadcast address */
    ACE_FLAG_DMAC_MC,       /**> DMAC with multicast address */
    ACE_FLAG_VLAN_CFI,      /**< VLAN CFI */
    ACE_FLAG_ARP_ARP,       /**< ARP/RARP */
    ACE_FLAG_ARP_REQ,       /**< Request/Reply ARP frame */
    ACE_FLAG_ARP_UNKNOWN,   /**< Unknown ARP frame */
    ACE_FLAG_ARP_SMAC,      /**< ARP flag: Sender hardware address (SHA) */
    ACE_FLAG_ARP_DMAC,      /**< ARP flag: Target hardware address (THA) */
    ACE_FLAG_ARP_LEN,       /**< ARP flag: Hardware address length (HLN) */
    ACE_FLAG_ARP_IP,        /**< ARP flag: Hardware address space (HRD) */
    ACE_FLAG_ARP_ETHER,     /**< ARP flag: Protocol address space (PRO) */
    ACE_FLAG_IP_TTL,        /**< IPv4 frames with a Time-to-Live field */
    ACE_FLAG_IP_FRAGMENT,   /**< More Fragments (MF) bit and the Fragment Offset (FRAG OFFSET) field for an IPv4 frame */
    ACE_FLAG_IP_OPTIONS,    /**< IP options */
    ACE_FLAG_TCP_FIN,       /**< TCP flag: No more data from sender (FIN) */
    ACE_FLAG_TCP_SYN,       /**< TCP flag: Synchronize sequence numbers (SYN) */
    ACE_FLAG_TCP_RST,       /**< TCP flag: Reset the connection (RST) */
    ACE_FLAG_TCP_PSH,       /**< TCP flag: Push Function (PSH) */
    ACE_FLAG_TCP_ACK,       /**< TCP flag: Acknowledgment field significant (ACK) */
    ACE_FLAG_TCP_URG,       /**< TCP flag: Urgent Pointer field significan (URG) */
    ACE_FLAG_COUNT          /**< Last entry */
};

typedef int acl_flag_t;

#define ACE_FLAG_SIZE VTSS_BF_SIZE(ACE_FLAG_COUNT)

/* ACL policy numbers */
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
#define ACL_POLICIES            255        /**< Number of ACL policies */
#elif defined(VTSS_ARCH_SERVAL)
#define ACL_POLICIES            63         /**< Number of ACL policies */
#define ACL_POLICY_CPU_REDIR    62         /**< EVC: Peer */
#define ACL_POLICY_DISCARD      63         /**< EVC: Discard */
#else
#define ACL_POLICIES            7          /**< Number of ACL policies */
#endif /* VTSS_ARCH_LUTON26 || VTSS_ARCH_JAGUAR_1 */

/* ACL policer numbers */
#define ACL_POLICER_NONE        0xFFFFFFFF
#define ACL_POLICER_NO_START    VTSS_ACL_POLICER_NO_START
#if defined(VTSS_ARCH_LUTON28)
#define ACL_POLICER_NO_END      (VTSS_ACL_POLICER_NO_END - 1)
#define ACL_POLICER_NO_RESV     ACL_POLICER_NO_END /* Reserved for CPU policing */
#define ACL_POLICIES_BITMASK    0x3
#else
#define ACL_POLICER_NO_END      VTSS_ACL_POLICER_NO_END
#if defined(VTSS_ARCH_SERVAL)
#define ACL_POLICIES_BITMASK    0x3F
#else
#define ACL_POLICIES_BITMASK    0xFF
#endif /* VTSS_ARCH_SERVAL */
#endif /* VTSS_ARCH_LUTON28 */
#define ACL_EVC_POLICER_NO_START    1
#if defined(VTSS_SW_OPTION_EVC)
#define ACL_EVC_POLICER_NO_END      EVC_POL_COUNT
#else
#define ACL_EVC_POLICER_NO_END      256
#endif /* VTSS_SW_OPTION_EVC */

/* ACE ID numbers */
#if defined(VTSS_ARCH_JAGUAR_1) ||  defined(VTSS_ARCH_SERVAL)
#define ACE_MAX         512
#elif defined(VTSS_ARCH_LUTON26)
#define ACE_MAX         256
#else
#define ACE_MAX         128
#endif

#define ACE_ID_NONE     0                       /* Reserved */
#define ACE_ID_START    1                       /* First ACE ID */
#if VTSS_SWITCH_STACKABLE
#define ACE_ID_END   (VTSS_ISID_CNT * ACE_MAX)  /* Last ACE ID */
#else
#define ACE_ID_END   (ACE_MAX)                  /* Last ACE ID */
#endif /* VTSS_SWITCH_STACKABLE */


#if defined(VTSS_ARCH_JAGUAR_1)
#define ACL_PACKET_RATE_IN_RANGE
#define ACL_PACKET_RATE_MAX         131071  /* pps */
#elif defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#define ACL_BIT_RATE_MAX            1000000 /* kbps */
#define ACL_BIT_RATE_GRANULARITY    100
#if defined(VTSS_ARCH_SERVAL)
#define ACL_PKT_RATE_GRANULARITY    100
#endif
#define ACL_PACKET_RATE_IN_RANGE
#define ACL_PACKET_RATE_MAX         3276700 /* pps */
#else //VTSS_ARCH_LUTON28
#define ACL_PACKET_RATE_MAX         1024    /* kpps */
#endif

/**
 * \brief ACL users declaration.
 */
typedef enum {
    ACL_USER_STATIC = 0,

    /*
     * Add your new ACL user below.
     * Be careful the location of your new ACL user.
     * The enum value also used to decide the order in ACL.
     */

    /*
     * The rule of IP management must be the last.
     */
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_SW_OPTION_IP_MGMT_ACL)
    ACL_USER_IP_MGMT,
#endif

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    ACL_USER_IP_SOURCE_GUARD,
#endif

#if defined(VTSS_SW_OPTION_IPMC) || defined(VTSS_SW_OPTION_IGMPS) || defined(VTSS_SW_OPTION_MLDSNP)
    ACL_USER_IPMC,
#endif

#ifdef VTSS_SW_OPTION_EVC
    ACL_USER_EVC,
#endif

#ifdef VTSS_SW_OPTION_MEP
    ACL_USER_MEP,
#endif

#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    ACL_USER_ARP_INSPECTION,
#endif

#ifdef VTSS_SW_OPTION_UPNP
    ACL_USER_UPNP,
#endif

#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    ACL_USER_PTP,
#endif

#ifdef VTSS_SW_OPTION_DHCP_HELPER
    ACL_USER_DHCP,
#endif

#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    ACL_USER_LOOP_PROTECT,
#endif

    /*
     * The rule of link OAM must be the first.
     */
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    ACL_USER_LINK_OAM,
#endif

    ACL_USER_CNT
} acl_user_t;

/**
 * \brief ACL users registered mode declaration.
 * If the ACL user uses local registration, it will fully control the
 * specific ACEs. And the ACL module won't delete these ACEs when
 * INIT_CMD_SWITCH_ADD state even if INIT_CMD_CONF_DEF state (restore default).
 * If the ACL user uses global registration, the ACL module will set the
 * related ACEs to each switch.
 */
enum {
    ACL_USER_REG_MODE_GLOBAL,   /* The ACL user use global registration */
    ACL_USER_REG_MODE_LOCAL     /* The ACL user use local registration */
};

extern const int acl_user_reg_modes[ACL_USER_CNT];
extern const char *const acl_user_names[ACL_USER_CNT];

/*
 * Currently, only ACL_USER_STATIC and ACL_USER_IP_SOURCE_GUARD uses the global
 * mode now. If there are new module use the global registration, We need change
 * the defined value of ACL_USER_GLOBAL_MODE_CNT.
 */
#define ACL_USER_GLOBAL_MODE_CNT    2

#ifdef _ACL_USER_NAME_C_
/**
 * \brief ACL user registered mode.
 * If a module need use both global and local registration, this module
 * should be create two ACL users.
 */
const int acl_user_reg_modes[ACL_USER_CNT] = {
    [ACL_USER_STATIC] = ACL_USER_REG_MODE_GLOBAL,

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_SW_OPTION_IP_MGMT_ACL)
    [ACL_USER_IP_MGMT] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    [ACL_USER_IP_SOURCE_GUARD] = ACL_USER_REG_MODE_GLOBAL,
#endif

#if defined(VTSS_SW_OPTION_IPMC) || defined(VTSS_SW_OPTION_IGMPS) || defined(VTSS_SW_OPTION_MLDSNP)
    [ACL_USER_IPMC] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_EVC
    [ACL_USER_EVC] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_MEP
    [ACL_USER_MEP] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    [ACL_USER_ARP_INSPECTION] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_UPNP
    [ACL_USER_UPNP] = ACL_USER_REG_MODE_LOCAL,
#endif

#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    [ACL_USER_PTP] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_DHCP_HELPER
    [ACL_USER_DHCP] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    [ACL_USER_LOOP_PROTECT] = ACL_USER_REG_MODE_LOCAL,
#endif

#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    [ACL_USER_LINK_OAM] = ACL_USER_REG_MODE_LOCAL,
#endif
};

/**
 * \brief ACL user names.
 * Each ACL user is associated with a text string,
 * which can be used when showing volatile ACL status
 * in the CLI/Web.
 */
const char *const acl_user_names[ACL_USER_CNT] = {
    [ACL_USER_STATIC] = "Static",

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_SW_OPTION_IP_MGMT_ACL)
    [ACL_USER_IP_MGMT] = "IP Management",
#endif

#ifdef VTSS_SW_OPTION_IP_SOURCE_GUARD
    [ACL_USER_IP_SOURCE_GUARD] = "IP Source Guard",
#endif

#if defined(VTSS_SW_OPTION_IPMC) || defined(VTSS_SW_OPTION_IGMPS) || defined(VTSS_SW_OPTION_MLDSNP)
    [ACL_USER_IPMC] = "IPMC",
#endif

#ifdef VTSS_SW_OPTION_EVC
    [ACL_USER_EVC] = "EVC",
#endif

#ifdef VTSS_SW_OPTION_MEP
    [ACL_USER_MEP] = "MEP",
#endif

#ifdef VTSS_SW_OPTION_ARP_INSPECTION
    [ACL_USER_ARP_INSPECTION] = "ARP Inspection",
#endif

#ifdef VTSS_SW_OPTION_UPNP
    [ACL_USER_UPNP] = "UPnP",
#endif

#if defined(VTSS_SW_OPTION_PTP) || defined(VTSS_SW_OPTION_PHY_1588_SIM)
    [ACL_USER_PTP] = "PTP",
#endif

#ifdef VTSS_SW_OPTION_DHCP_HELPER
    [ACL_USER_DHCP] = "DHCP",
#endif

#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    [ACL_USER_LOOP_PROTECT] = "Loop Protect",
#endif

#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
    [ACL_USER_LINK_OAM] = "Link OAM",
#endif
};
#endif /* _ACL_USER_NAME_C_ */

/** \brief ACL entry configuration */
typedef struct {
    vtss_ace_id_t           id;                                 /**< ACE ID */
#if defined(VTSS_ARCH_SERVAL)
    u8                      lookup;                             /**< Lookup, any non-zero value means second lookup */
    BOOL                    isdx_enable;                        /**< Use VID field for ISDX value */
    BOOL                    isdx_disable;                       /**< Match only frames with ISDX zero */
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_ACL_V2)
    BOOL                    port_list[VTSS_PORT_ARRAY_SIZE];    /**< Port list: VTSS_ACL_RULE_PORT */
#else
    vtss_port_no_t          port_no;                            /**< Port number: VTSS_ACL_RULE_PORT */
#endif /* VTSS_FEATURE_ACL_V2 */
    vtss_ace_u8_t           policy;                             /**< Policy number */
    vtss_ace_type_t         type;                               /**< ACE frame type */
    acl_action_t            action;                             /**< ACE action */
    vtss_isid_t             isid;                               /**< Switch ID, VTSS_ISID_GLOBAL means any */
    BOOL                    conflict;                           /**< Volatile ACE conflict flag */
    BOOL                    new_allocated;                      /**< This ACE entry is new allocated. (Only for internal used) */

    struct {
        uchar value[ACE_FLAG_SIZE];     /* ACE flag value */
        uchar mask[ACE_FLAG_SIZE];      /**< ACE flag mask */
    } flags;

    vtss_ace_vid_t  vid;                /**< VLAN ID (12 bit) */
    vtss_ace_u8_t   usr_prio;           /**< User priority (3 bit) */
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_ace_bit_t  tagged;             /**< Tagged/untagged frame */
#endif /* VTSS_FEATURE_ACL_V2 */

    /* Frame type specific data */
    union {
        /* VTSS_ACE_TYPE_ANY: No specific fields */

        /**< VTSS_ACE_TYPE_ETYPE */
        struct {
            vtss_ace_u48_t  dmac;   /**< DMAC */
            vtss_ace_u48_t  smac;   /**< SMAC */
            vtss_ace_u16_t  etype;  /**< Ethernet Type value */
            vtss_ace_u16_t  data;   /**< MAC data */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_ptp_t  ptp;    /**< PTP header filtering (overrides smac byte 0-1 and data fields) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } etype;

        /**< VTSS_ACE_TYPE_LLC */
        struct {
            vtss_ace_u48_t dmac; /**< DMAC */
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_u32_t llc;  /**< LLC */
        } llc;

        /**< VTSS_ACE_TYPE_SNAP */
        struct {
            vtss_ace_u48_t dmac; /**< DMAC */
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_u40_t snap; /**< SNAP */
        } snap;

        /** VTSS_ACE_TYPE_ARP */
        struct {
            vtss_ace_u48_t smac; /**< SMAC */
            vtss_ace_ip_t  sip;  /**< Sender IP address */
            vtss_ace_ip_t  dip;  /**< Target IP address */
        } arp;

        /**< VTSS_ACE_TYPE_IPV4 */
        struct {
            vtss_ace_u8_t       ds;         /* DS field */
            vtss_ace_u8_t       proto;      /**< Protocol */
            vtss_ace_ip_t       sip;        /**< Source IP address */
            vtss_ace_ip_t       dip;        /**< Destination IP address */
            vtss_ace_u48_t      data;       /**< Not UDP/TCP: IP data */
            vtss_ace_udp_tcp_t  sport;      /**< UDP/TCP: Source port */
            vtss_ace_udp_tcp_t  dport;      /**< UDP/TCP: Destination port */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_sip_smac_t sip_smac;   /**< SIP/SMAC matching (overrides sip field) */
            vtss_ace_ptp_t      ptp;        /**< PTP header filtering (overrides sip field) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } ipv4;

        /**< VTSS_ACE_TYPE_IPV6 */
        struct {
            vtss_ace_u8_t      proto;     /**< IPv6 protocol */
            vtss_ace_u128_t    sip;       /**< IPv6 source address */
            vtss_ace_bit_t     ttl;       /**< TTL zero */
            vtss_ace_u8_t      ds;        /**< DS field */
            vtss_ace_u48_t     data;      /**< Not UDP/TCP: IP data */
            vtss_ace_udp_tcp_t sport;     /**< UDP/TCP: Source port */
            vtss_ace_udp_tcp_t dport;     /**< UDP/TCP: Destination port */
            vtss_ace_bit_t     tcp_fin;   /**< TCP FIN */
            vtss_ace_bit_t     tcp_syn;   /**< TCP SYN */
            vtss_ace_bit_t     tcp_rst;   /**< TCP RST */
            vtss_ace_bit_t     tcp_psh;   /**< TCP PSH */
            vtss_ace_bit_t     tcp_ack;   /**< TCP ACK */
            vtss_ace_bit_t     tcp_urg;   /**< TCP URG */
#if defined(VTSS_FEATURE_ACL_V2)
            vtss_ace_ptp_t     ptp;       /**< PTP header filtering (overrides sip byte 0-3) */
#endif /* VTSS_FEATURE_ACL_V2 */
        } ipv6;
    } frame;
} acl_entry_conf_t;

/**
  * \brief Retrieve an error string based on a return code
  *        from one of the ACL API functions.
  *
  * \param rc [IN]: Error code that must be in the ACL_ERROR_xxx range.
  */
char *acl_error_txt(vtss_rc rc);


/**
  * \brief Determine if ACL port configuration has changed.
  *
  * \param old [IN]: Pointer to structure that contains the
  *                  old configuration.
  * \param new [IN]: Pointer to structure that contains the
  *                  new configuration.
  *
  * \return
  *   0: No change.\n
  *   none zero: Configuration changed.\n
  */
int acl_mgmt_port_conf_changed(acl_port_conf_t *old, acl_port_conf_t *new);


/**
  * \brief Get the ACL default port configuration.
  *
  * \param conf [IN_OUT]: Pointer to structure that contains the
  *                       configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void acl_mgmt_port_conf_get_default(acl_port_conf_t *conf);

/**
  * \brief Get a switch's per-port configuration.
  *
  * \param isid        [IN]: The Switch ID for which to retrieve the
  *                          configuration.
  * \param port        [IN]: The port ID for which to retrieve the
  *                          configuration.
  * \param conf        [OUT]: Pointer to structure that receives
  *                          the switch's per-port configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_PARM if Switch ID or port ID is invalid.\n
  */
vtss_rc acl_mgmt_port_conf_get(vtss_isid_t isid,
                               vtss_port_no_t port_no, acl_port_conf_t *conf);

/**
  * \brief Set a switch's per-port configuration.
  *
  * \param isid       [IN]: The switch ID for which to set the configuration.
  * \param port_no    [IN]: The port ID for which to set the configuration.
  * \param conf       [IN]: Pointer to structure that contains
  *                         the switch's per-port configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_PARM if switch ID or port ID is invalid.\n
  *    ACL_ERROR_STACK_STATE if called on a slave switch.\n
  */
vtss_rc acl_mgmt_port_conf_set(vtss_isid_t isid,
                               vtss_port_no_t port_no, acl_port_conf_t *conf);

/**
  * \brief Get a switch's port counter.
  *
  * \param isid       [IN]: The switch ID for which to get the counter.
  * \param port_no    [IN]: The port ID for which to get the counter.
  * \param counter    [OUT]: Pointer to structure that receives
  *                          the switch's port counter.
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_PARM if switch ID or port ID is invalid.\n
  */
vtss_rc acl_mgmt_port_counter_get(vtss_isid_t isid,
                                  vtss_port_no_t port_no, vtss_acl_port_counter_t *counter);


/**
  * \brief Determine if ACL policer configuration has changed.
  *
  * \param old [IN]: Pointer to structure that contains the
  *                  old configuration.
  * \param new [IN]: Pointer to structure that contains the
  *                  new configuration.
  *
  * \return
  *   0: No change.\n
  *   none zero: Configuration changed.\n
  */
int acl_mgmt_policer_conf_changed(acl_policer_conf_t *old, acl_policer_conf_t *new);

/**
  * \brief Get the global ACL policer default configuration.
  *
  * \param conf [IN_OUT]: Pointer to structure that contains the
  *                       configuration to get the default setting.
  *
  * \return
  *   Nothing.
  */
void acl_mgmt_policer_conf_get_default(acl_policer_conf_t *conf);

/**
  * \brief Get a switch's port policer configuration.
  *
  * \param policer     [IN]: The policer ID for which to retrieve the
  *                          configuration.
  * \param conf        [OUT]: Pointer to structure that receives
  *                          the switch's port policer configuration.
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_STACK_STATE if called on a slave switch.\n
  */
vtss_rc acl_mgmt_policer_conf_get(vtss_acl_policer_no_t policer_no,
                                  acl_policer_conf_t *conf);

/**
  * \brief Set a switch's port policer configuration.
  *
  * \param policer_no [IN]: The policer ID for which to set the configuration.
  * \param conf       [IN]: Pointer to structure that contains
  *                         the switch's port policer configuration to be applied.
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_STACK_STATE if called on a slave switch.\n
  */
vtss_rc acl_mgmt_policer_conf_set(vtss_acl_policer_no_t policer_no,
                                  acl_policer_conf_t *conf);

/* Get ACE or next ACE (use ACE_ID_NONE to get first) */
/**
 * \brief Get/Getnext an ACE by user ID and ACE ID.
 *
 * \param user_id [IN]    The ACL user ID.
 *
 * \param isid    [IN]    The switch ID. This parameter must equal
 *                        VTSS_ISID_LOCAL when the role is slave.
 *
 *                        isid = VTSS_ISID_LOCAL:
 *                        Can be called from both a slave and the master.
 *                        It will return the entry that stored in the local
 *                        switch.
 *
 *                        isid = [VTSS_ISID_START - VTSS_ISID_END]:
 *                        isid = VTSS_ISID_GLOBAL:
 *                        Can only be called on the master.
 *
 * \param id      [IN]    Indentify the ACE ID. Each ACL user can use the
 *                        independent ACE ID for its own entries.
 *
 *                        id = ACE_ID_NONE:
 *                        It will return the first entry.
 *
 * \param conf    [OUT]   Pointer to structure that contains
 *                        the ACE configuration.
 *
 * \param counter [OUT]   The counter of the specific ACE. It can be equal
 *                        NULL if you don’t need the information.
 *
 * \param next    [IN]    Indentify if it is getnext operation.
 *
 * \return
 *    VTSS_OK on success.\n
 *    ACL_ERROR_STACK_STATE if the illegal MASTER/SLAVE state.\n
 *    ACL_ERROR_USER_NOT_FOUND if the user ID not found.\n
 *    ACL_ERROR_ACE_NOT_FOUND if the ACE ID not found.\n
 *    ACL_ERROR_REQ_TIMEOUT if the requirement timeout.\n
 **/
vtss_rc acl_mgmt_ace_get(acl_user_t user_id, vtss_isid_t isid,
                         vtss_ace_id_t id, acl_entry_conf_t *conf,
                         vtss_ace_counter_t *counter, BOOL next);

/* Add ACE entry before given ACE or last (ACE_ID_NONE) */
/**
 * \brief Add/Edit an ACE by user ID and ACE ID.
 *
 * Except for static configured ACEs, if the maximum number of entries
 * supported by the hardware is exceeded, the conflict flag of this entry
 * will be set and this entry will not be applied to the hardware.
 *
 * The order of the ACEs refers to the ACL user ID value. An ACE with
 * higher value will be placed the front of ACL.
 *
 * Only static configured ACEs will be stored in Flash.
 *
 * \param user_id [IN]    The ACL user ID.
 *
 * \param next_id [IN]    The next ACE ID that this ACE entry want
 *                        to insert before it. Each ACL user can use the
 *                        independent ACE ID for its own entries.
 *
 *                        next_id = ACE_ID_NONE:
 *                        This entry will be placed at last in the given user ID.
 *
 * \param conf    [IN]    Pointer to structure that contains the ACE configuration.
 *
 *                        conf->isid = VTSS_ISID_LOCAL:
 *                        Can be called from both a slave and the master.
 *                        It will add/edit the specific entry that stored
 *                        in the local switch. This parameter must equal
 *                        VTSS_ISID_LOCAL when the role is slave.
 *
 *                        conf->isid = [VTSS_ISID_START - VTSS_ISID_END]:
 *                        Can only be called on the master. This entry will only
 *                        apply to specific switch.
 *
 *                        conf->isid = VTSS_ISID_GLOBAL:
 *                        Can only be called on the master. This entry will apply
 *                        to all switches.
 *
 *                        conf->id = ACE_ID_NONE:
 *                        The ACE ID will be auto-assigned.
 *
 *                        conf->action.force_cpu = TRUE:
 *                        If the parameter is set, the specific packet will forward
 *                        to local switch.
 *                        The parameter of "conf->action.force_cpu" and
 *                        "conf->action.cpu_once" is insignificance when
 *                        user_id = ACL_USER_STATIC.
 *
 * \param conf    [OUT]   The parameter of "conf->id" will be update if using
 *                        auto-assigned.
 *                        The parameter of "conf->conflict" will be set if
 *                        it is not applied to the hardware due to hardware
 *                        limitations.
 *
 * \return
 *    VTSS_OK on success.\n
 *    ACL_ERROR_STACK_STATE if the illegal MASTER/SLAVE state.\n
 *    ACL_ERROR_USER_NOT_FOUND if the user ID not found.\n
 *    ACL_ERROR_MEM_ALLOC_FAIL if memory allocated fail.\n
 *    ACL_ERROR_PARM if input parameters error.\n
 *    ACL_ERROR_REQ_TIMEOUT if the requirement timeout.\n
 **/
vtss_rc acl_mgmt_ace_add(acl_user_t user_id, vtss_ace_id_t next_id, acl_entry_conf_t *conf);

/**
 * \brief Delete an ACE by user ID and ACE ID.
 *
 * \param user_id [IN]    The ACL user ID.
 *
 * \param id      [IN]    The ACE ID that we want to delete it.
 *                        Each ACL user can use the independent
 *                        ACE ID for its own entries.
 *
 * \return
 *    VTSS_OK on success.\n
 *    ACL_ERROR_STACK_STATE if the illegal MASTER/SLAVE state.\n
 *    ACL_ERROR_USER_NOT_FOUND if the user ID not found.\n
 *    ACL_ERROR_ACE_NOT_FOUND if the ACE ID not found.\n
 **/
vtss_rc acl_mgmt_ace_del(acl_user_t user_id, vtss_ace_id_t id);

/**
  * \brief Clear all ACE counter
  *
  * \return
  *    VTSS_OK on success.\n
  *    ACL_ERROR_STACK_STATE if called on a slave switch.\n
  */
vtss_rc acl_mgmt_counters_clear(void);

/**
  * \brief Initialize the ACL module
  *
  * \param data [IN]: Initial data point.
  *
  * \return
  *    VTSS_OK.
  */
vtss_rc acl_init(vtss_init_data_t *data);

/**
 * \brief Initialize ACE to default values.(permit on all front ports)
 *
 * \param type [IN]  ACE type.
 *
 * \param ace [OUT]  ACE structure.
 *
 * \return
 *    VTSS_OK on success.\n
 *    ACL_ERROR_UNKNOWN_ACE_TYPE if the ACE type is unknown.\n
 **/
vtss_rc acl_mgmt_ace_init(vtss_ace_type_t type, acl_entry_conf_t *ace);

#endif /* _VTSS_ACL_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
