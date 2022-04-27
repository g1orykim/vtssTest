/*
   Vitesse VCL software.

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

#include <vcl_trace.h>
#include <vcl_api.h>
#ifdef VTSS_SW_OPTION_VCLI
#include <vcl_cli.h>
#endif
#include <vlan_api.h>
#include <msg_api.h>
#include <conf_api.h>               /* definition of conf_blk_id_t */
#include <vtss_module_id.h>         /* For VCL module ID */
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>
#include <misc_api.h>
#ifdef VTSS_SW_OPTION_ICFG
#include "vcl_icfg.h"
#endif


/*lint -sem( vtss_vcl_crit_data_lock, thread_lock ) */
/*lint -sem( vtss_vcl_crit_data_unlock, thread_unlock ) */

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "vcl",
    .descr     = "VCL table"
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
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CLI] = {
        .name      = "cli",
        .descr     = "CLI",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};

#define VCL_CRIT_ENTER()         critd_enter(&vcl_data_lock, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VCL_CRIT_EXIT()          critd_exit(&vcl_data_lock, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FUNCTION__, __LINE__)
#define VCL_CRIT_ASSERT_LOCKED() critd_assert_locked(&vcl_data_lock, TRACE_GRP_CRIT,  __FILE__, __LINE__)
#else  /* VTSS_TRACE_ENABLED */
#define VCL_CRIT_ENTER()         critd_enter(&vcl_data_lock)
#define VCL_CRIT_EXIT()          critd_exit(&vcl_data_lock, 0)
#define VCL_CRIT_ASSERT_LOCKED() critd_assert_locked(&vcl_data_lock)
#endif /* VTSS_TRACE_ENABLED */

/* Request structure required by the message module */
static void *vcl_request_pool;

/* Debug policy number */
static vtss_acl_policy_no_t vcl_debug_policy_no = VTSS_ACL_POLICY_NO_NONE;

/* Global module/API Lock for Critical VCL data protection */
static critd_t vcl_data_lock;

static vtss_rc rc_conv(int vcl_rc)
{
    vtss_rc rc = VTSS_RC_ERROR;

    switch (vcl_rc) {
    case VTSS_VCL_RC_OK:
        rc = VTSS_RC_OK;
        break;
    case VTSS_VCL_ERROR_PARM:
        rc = VCL_ERROR_PARM;
        break;
    case VTSS_VCL_ERROR_CONFIG_NOT_OPEN:
        rc = VCL_ERROR_CONFIG_NOT_OPEN;
        break;
    case VTSS_VCL_ERROR_ENTRY_NOT_FOUND:
        rc = VCL_ERROR_ENTRY_NOT_FOUND;
        break;
    case VTSS_VCL_ERROR_TABLE_EMPTY:
        rc = VCL_ERROR_TABLE_EMPTY;
        break;
    case VTSS_VCL_ERROR_TABLE_FULL:
        rc = VCL_ERROR_TABLE_FULL;
        break;
    case VTSS_VCL_ERROR_REG_TABLE_FULL:
        rc = VCL_ERROR_REG_TABLE_FULL;
        break;
    case VTSS_VCL_ERROR_STACK_STATE:
        rc = VCL_ERROR_STACK_STATE;
        break;
    case VTSS_VCL_ERROR_USER_PREVIOUSLY_CONFIGURED:
        rc = VCL_ERROR_USER_PREVIOUSLY_CONFIGURED;
        break;
    case VTSS_VCL_ERROR_VCE_ID_PREVIOUSLY_CONFIGURED:
        rc = VCL_ERROR_VCE_ID_PREVIOUSLY_CONFIGURED;
        break;
    case VTSS_VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED:
        rc = VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED;
        break;
    case VTSS_VCL_ERROR_ENTRY_WITH_DIFF_NAME:
        rc = VCL_ERROR_ENTRY_WITH_DIFF_NAME;
        break;
    case VTSS_VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN:
        rc = VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN;
        break;
    case VTSS_VCL_ERROR_ENTRY_WITH_DIFF_SUBNET:
        rc = VCL_ERROR_ENTRY_WITH_DIFF_SUBNET;
        break;
    case VTSS_VCL_ERROR_ENTRY_WITH_DIFF_VLAN:
        rc = VCL_ERROR_ENTRY_WITH_DIFF_VLAN;
        break;
    default:
        T_E("Invalid VCL module error is noticed:- %d", vcl_rc);
        break;
    }
    return rc;
}

static char *vcl_msg_id_txt(vcl_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case VCL_MAC_VLAN_MSG_ID_CONF_SET_REQ:
        txt = "VCL_MAC_VLAN_CONF_SET_REQ";
        break;
    case VCL_MAC_VLAN_MSG_ID_CONF_ADD_REQ:
        txt = "VCL_MAC_VLAN_CONF_ADD_REQ";
        break;
    case VCL_MAC_VLAN_MSG_ID_CONF_DEL_REQ:
        txt = "VCL_MAC_VLAN_CONF_DEL_REQ";
        break;
    case VCL_PROTO_VLAN_MSG_ID_CONF_SET_REQ:
        txt = "VCL_PROTO_VLAN_CONF_SET_REQ";
        break;
    case VCL_PROTO_VLAN_MSG_ID_CONF_ADD_REQ:
        txt = "VCL_PROTO_VLAN_CONF_ADD_REQ";
        break;
    case VCL_PROTO_VLAN_MSG_ID_CONF_DEL_REQ:
        txt = "VCL_PROTO_VLAN_CONF_DEL_REQ";
        break;
    case VCL_IP_VLAN_MSG_ID_CONF_SET_REQ:
        txt = "VCL_IP_VLAN_CONF_SET_REQ";
        break;
    case VCL_IP_VLAN_MSG_ID_CONF_ADD_REQ:
        txt = "VCL_IP_VLAN_CONF_ADD_REQ";
        break;
    case VCL_IP_VLAN_MSG_ID_CONF_DEL_REQ:
        txt = "VCL_IP_VLAN_CONF_DEL_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}

/* Allocate request/reply buffer */
static void *vcl_msg_alloc(u32 ref_cnt)
{
    void *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(vcl_request_pool);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }
    return msg;
}

static void vcl_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    vcl_msg_id_t msg_id = *(vcl_msg_id_t *)msg;

    T_D("msg_id: %d, %s", msg_id, vcl_msg_id_txt(msg_id));
    (void)msg_buf_pool_put(msg);
}

/* Send the VCL Message */
static void vcl_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    vcl_msg_id_t msg_id = *(vcl_msg_id_t *)msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, vcl_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(NULL, vcl_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_VCL, isid, msg, len);
}

static vtss_rc vcl_mac_vlan_vce_entry_add(vcl_mac_vlan_msg_cfg_t *conf)
{
    vtss_vce_t           vce;
    u32                  i;
    vtss_rc              rc = VTSS_RC_OK;
    vtss_vce_id_t        vce_id = conf->id, vce_id_next = VTSS_VCE_ID_LAST;
    port_iter_t          pit;

    /* Check for NULL pointer */
    if (conf == NULL) {
        T_I("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid VCE ID */
    if (vce_id >= VTSS_VCL_ID_END) {
        T_I("Invalid VCE ID");
        return VCL_ERROR_PARM;
    }

    if ((rc = vtss_vce_init(NULL, VTSS_VCE_TYPE_ANY,  &vce)) != VTSS_RC_OK) {
        return rc;
    }

    /* Prepare key */
    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        vce.key.port_list[pit.iport] = (conf->ports[pit.iport / 8] & (1 << (pit.iport % 8))) ? TRUE : FALSE;
    }
    for (i = 0; i < sizeof(vtss_mac_t); i++) {
        vce.key.mac.smac.value[i] = conf->smac.addr[i];
        vce.key.mac.smac.mask[i] = 0xFF;
    }
    vce.key.mac.dmac_mc = VTSS_VCAP_BIT_ANY;
    vce.key.mac.dmac_bc = VTSS_VCAP_BIT_ANY;

    /* Prepare action - Only action is to classify to VLAN specified */
    vce.action.vid = conf->vid;
    VCL_CRIT_ENTER();
    vce.action.policy_no = vcl_debug_policy_no;
    VCL_CRIT_EXIT();

    /* Populate VCE ID: First 16 bits (15:0) -> real VCE_ID; next 4-bits (23:16) -> VLAN_User;
       next 4-bits (31:24) -> VCL Type (MAC or Protocol)  */
    vce.id = ((vce_id & 0xFFFF) | (((conf->user) & 0xF) << 16) | ((VCL_TYPE_MAC & 0xF) << 20));
    vcl_ip_vlan_first_vce_id_get(&vce_id_next);
    if (vce_id_next == VCE_ID_NONE) {
        vcl_proto_vlan_first_vce_id_get(&vce_id_next);
        if (vce_id_next == VCE_ID_NONE) {
            vce_id_next = VTSS_VCE_ID_LAST;
        } else {
            vce_id_next = ((vce_id_next & 0xFFFF) | (((conf->user) & 0xF) << 16) | ((VCL_TYPE_PROTO & 0xF) << 20));
        }
    } else {
        vce_id_next = ((vce_id_next & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20));
    }
    /* Call the switch API for setting the configuration in ASIC. MAC-based VLAN has more priority than
       Protocol-based VLAN. Hence add any MAC-based VLAN entries before first protocol-based VLAN entry and IP Subnet VLAN*/
    rc = vtss_vce_add(NULL, vce_id_next,  &vce);
    return rc;
}

static vtss_rc vcl_proto_vlan_vce_entry_add(vcl_proto_vlan_local_sid_conf_t *conf)
{
    vtss_vce_t           vce;
    vtss_rc              rc = VTSS_RC_OK;
    vtss_vce_id_t        vce_id = conf->id;
    u8                   tmp;
    port_iter_t          pit;

    /* Check for NULL pointer */
    if (conf == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid VCE ID */
    if (vce_id >= VTSS_VCL_ID_END) {
        T_E("Invalid VCE ID");
        return VCL_ERROR_PARM;
    }

    if ((rc = vtss_vce_init(NULL, VTSS_VCE_TYPE_ANY,  &vce)) != VTSS_RC_OK) {
        return rc;
    }

    /* Prepare key */
    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        vce.key.port_list[pit.iport] = (conf->ports[pit.iport / 8] & (1 << (pit.iport % 8))) ? TRUE : FALSE;
    }
    if (conf->proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
        if (conf->proto.eth2_proto.eth_type == ETHERTYPE_IP) {
            vce.key.type = VTSS_VCE_TYPE_IPV4;
        } else if (conf->proto.eth2_proto.eth_type == ETHERTYPE_IP6) {
            vce.key.type = VTSS_VCE_TYPE_IPV6;
        } else {
            vce.key.type = VTSS_VCE_TYPE_ETYPE;
            /* Copy etype field */
            memcpy(vce.key.frame.etype.etype.value, &conf->proto.eth2_proto.eth_type,
                   sizeof(vce.key.frame.etype.etype.value));
            /* Swapping bytes - probably need to check ENDIANNESS*/
            tmp = vce.key.frame.etype.etype.value[0];
            vce.key.frame.etype.etype.value[0] = vce.key.frame.etype.etype.value[1];
            vce.key.frame.etype.etype.value[1] = tmp;
            vce.key.frame.etype.etype.mask[0] = 0xFF;
            vce.key.frame.etype.etype.mask[1] = 0xFF;
            vce.key.frame.etype.data.mask[0] = 0x0;
            vce.key.frame.etype.data.mask[1] = 0x0;
            vce.key.frame.etype.data.mask[2] = 0x0;
            vce.key.frame.etype.data.mask[3] = 0x0;
        }
    } else if (conf->proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
        vce.key.type = VTSS_VCE_TYPE_SNAP;
        memcpy(vce.key.frame.snap.data.value, conf->proto.llc_snap_proto.oui, OUI_SIZE);
        /* Copy OUI field */
        vce.key.frame.snap.data.value[0] = conf->proto.llc_snap_proto.oui[0];
        vce.key.frame.snap.data.value[1] = conf->proto.llc_snap_proto.oui[1];
        vce.key.frame.snap.data.value[2] = conf->proto.llc_snap_proto.oui[2];
        vce.key.frame.snap.data.mask[0] = 0xFF;
        vce.key.frame.snap.data.mask[1] = 0xFF;
        vce.key.frame.snap.data.mask[2] = 0xFF;
        /* Copy PID field */
        memcpy(&vce.key.frame.snap.data.value[3], &conf->proto.llc_snap_proto.pid, 2);
        tmp = vce.key.frame.snap.data.value[3];
        vce.key.frame.snap.data.value[3] = vce.key.frame.snap.data.value[4];
        vce.key.frame.snap.data.value[4] = tmp;
        vce.key.frame.snap.data.mask[3] = 0xFF;
        vce.key.frame.snap.data.mask[4] = 0xFF;
    } else if (conf->proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
        vce.key.type = VTSS_VCE_TYPE_LLC;
        /* Copy DSAP and SSAP fields */
        vce.key.frame.llc.data.value[0] = conf->proto.llc_other_proto.dsap;
        vce.key.frame.llc.data.value[1] = conf->proto.llc_other_proto.ssap;
        vce.key.frame.llc.data.mask[0] = 0xFF;
        vce.key.frame.llc.data.mask[1] = 0xFF;
    }
    /* Allow only for untagged Frames */
    vce.key.tag.tagged = VTSS_VCAP_BIT_0;

    /* Prepare action - Only action is to classify to VLAN specified */
    vce.action.vid = conf->vid;
    VCL_CRIT_ENTER();
    vce.action.policy_no = vcl_debug_policy_no;
    VCL_CRIT_EXIT();

    /* Populate VCE ID: First 16 bits (15:0) -> real VCE_ID; next 4-bits (19:16) -> VLAN_User;
       next 4-bits (23:20) -> VCL Type (MAC or Protocol)  */
    vce.id = ((vce_id & 0xFFFF) | (((conf->user) & 0xF) << 16) | ((VCL_TYPE_PROTO & 0xF) << 20));
    /* Call the switch API for setting the configuration in ASIC. It is OK to add the entry at the end
       for now. But once another VCL user functionality is added, depending on the priority of the user,
       we may have to provide vce_id_next */
    rc = vtss_vce_add(NULL, VTSS_VCE_ID_LAST,  &vce);
    /* Add VCE rule for priority-tagged frames - All fields remain same except tagged and vid */
    vce.key.tag.tagged = VTSS_VCAP_BIT_1;
    vce.key.tag.vid.value = 0x0;
    vce.key.tag.vid.mask = 0xFFFF;
    /* Priority-tagged VCE entry is identified by setting 24th bit of vce_id */
    vce.id = ((vce_id & 0xFFFF) | (((conf->user) & 0xF) << 16) | ((VCL_TYPE_PROTO & 0xF) << 20) | (0x1 << 24));
    rc = vtss_vce_add(NULL, VTSS_VCE_ID_LAST,  &vce);
    return rc;
}

static vtss_rc vcl_ip_vlan_vce_entry_add(vcl_ip_vlan_msg_cfg_t *conf, vtss_vce_id_t next_id)
{
    vtss_vce_t           vce;
    vtss_port_no_t       port;
    vtss_rc              rc = VTSS_RC_OK;
    vtss_vce_id_t        vce_id = conf->id, vce_id_next = VTSS_VCE_ID_LAST;

    /* Check for NULL pointer */
    if (conf == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    if ((rc = vtss_vce_init(NULL, VTSS_VCE_TYPE_IPV4,  &vce)) != VTSS_RC_OK) {
        return rc;
    }

    /* Prepare key */
    for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
        vce.key.port_list[port] = (conf->ports[port / 8] & (1 << (port % 8))) ? TRUE : FALSE;
    }
    vce.key.frame.ipv4.sip.value = conf->ip_addr;
    ip_mask_len_2_mask(conf->mask_len, &vce.key.frame.ipv4.sip.mask);

    /* Allow only for untagged Frames */
    vce.key.tag.tagged = VTSS_VCAP_BIT_0;

    /* Prepare action - Only action is to classify to VLAN specified */
    vce.action.vid = conf->vid;
    VCL_CRIT_ENTER();
    vce.action.policy_no = vcl_debug_policy_no;
    VCL_CRIT_EXIT();

    /* Populate VCE ID: First 16 bits (15:0) -> real VCE_ID; next 4-bits (23:20) -> VCL Type (MAC, Protocol or IP);
       Only user possible is STATIC */
    vce.id = ((vce_id & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20));
    if (next_id == 0) {
        vcl_proto_vlan_first_vce_id_get(&vce_id_next);
        if (vce_id_next == VCE_ID_NONE) {
            vce_id_next = VTSS_VCE_ID_LAST;
        }
    } else {
        vce_id_next = ((next_id & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20));
    }
    /* Call the switch API for setting the configuration in ASIC. */
    rc = vtss_vce_add(NULL, vce_id_next,  &vce);
    /* Add VCE rule for priority-tagged frames - All fields remain same except tagged and vid */
    vce.key.tag.tagged = VTSS_VCAP_BIT_1;
    vce.key.tag.vid.value = 0x0;
    vce.key.tag.vid.mask = 0xFFFF;
    /* Priority-tagged VCE entry is identified by setting 24th bit of vce_id */
    vce.id = ((vce_id & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20) | (0x1 << 24));
    rc = vtss_vce_add(NULL, vce_id_next,  &vce);
    return rc;
}

/* VCL Message Receive Handler */
static BOOL vcl_msg_rx(void *contxt, const void *const rx_msg, const size_t len,
                       const vtss_module_id_t modid, const ulong isid)
{
    vcl_msg_id_t msg_id = *(vcl_msg_id_t *)rx_msg;
    vtss_rc rc = VTSS_RC_OK;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, vcl_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case VCL_MAC_VLAN_MSG_ID_CONF_SET_REQ: {
        u32                                 cnt;
        BOOL                                first = TRUE, next = FALSE, entered = FALSE;
        vcl_msg_conf_mac_vlan_set_req_t     *msg;
        vcl_mac_vlan_local_sid_conf_entry_t entry;
        vtss_mac_t                          mac;
        vtss_vce_id_t                       vce_id;
        T_D("VCL_MAC_VLAN_MSG_ID_CONF_SET_REQ");
        /* Delete all the existing entries */
        while ((rc_conv(vcl_mac_vlan_local_entry_get(&entry, next, first))) == VTSS_RC_OK) {
            entered = TRUE;
            if (first == TRUE) {
                first = FALSE;
                next = TRUE;
            } else {
                rc = rc_conv(vcl_mac_vlan_local_entry_del(&mac));
            }
            if (rc == VTSS_RC_OK) {
                mac = entry.mac;
                vce_id = entry.id;
                vce_id = ((vce_id & 0xFFFF) | (((entry.user) & 0xF) << 16) | ((VCL_TYPE_MAC & 0xF) << 20));
                /* Call the switch API */
                rc = vtss_vce_del(NULL, vce_id);
            }
        }
        /* Delete the last entry */
        if (entered == TRUE) {
            rc = rc_conv(vcl_mac_vlan_local_entry_del(&mac));
        }
        msg = (vcl_msg_conf_mac_vlan_set_req_t *)rx_msg;
        /* Add all the new entries */
        for (cnt = 0; cnt < msg->count; cnt++) {
            memcpy(&entry.mac, &msg->conf[cnt].smac, sizeof(vtss_mac_t));
            entry.vid = msg->conf[cnt].vid;
            memcpy(entry.ports, msg->conf[cnt].ports, VTSS_PORT_BF_SIZE);
            entry.id = msg->conf[cnt].id;
            entry.user = msg->conf[cnt].user;
            rc = rc_conv(vcl_mac_vlan_local_entry_add(&entry));
            if (rc == VTSS_RC_OK) {
                /* Call the switch API */
                rc = vcl_mac_vlan_vce_entry_add(&msg->conf[cnt]);
            }
        }
        break;
    }
    case VCL_MAC_VLAN_MSG_ID_CONF_ADD_REQ: {
        vcl_mac_vlan_local_sid_conf_entry_t entry;
        vcl_msg_conf_mac_vlan_add_del_req_t *msg;
        msg = (vcl_msg_conf_mac_vlan_add_del_req_t *)rx_msg;
        memcpy(&entry.mac, &(msg->conf.smac), sizeof(vtss_mac_t));
        entry.vid = msg->conf.vid;
        entry.id = msg->conf.id;
        entry.user = msg->conf.user;
        memcpy(entry.ports, msg->conf.ports, VTSS_PORT_BF_SIZE);
        T_D("VCL_MAC_VLAN_MSG_ID_CONF_ADD_REQ mac is %02x:%02x:%02x:%02x:%02x:%02x, entry.vid = %d, vce_id = %u",
            entry.mac.addr[0], entry.mac.addr[1], entry.mac.addr[2], entry.mac.addr[3],
            entry.mac.addr[4], entry.mac.addr[5], entry.vid, msg->conf.id);
        rc = rc_conv(vcl_mac_vlan_local_entry_add(&entry));
        if (rc == VTSS_RC_OK) {
            /* Call the switch API */
            rc = vcl_mac_vlan_vce_entry_add(&msg->conf);
            if (rc != VTSS_RC_OK) {
                T_I("VCE Add failed on the hardware. Please delete the entry from control database. To delete, use the command VCL Mac Del command");
                /* TODO: We need to clean up control data structures and return error */
#if 0
                if ((rc = rc_conv(vcl_mac_vlan_entry_del(&entry, &indx, user))) == VTSS_RC_OK) {
                    /* Currently this case is handled only on standalone swithc, stacking support will be added later */
                    if (msg_switch_is_master()) {
                        if ((user == VCL_MAC_VLAN_USER_STATIC)) {
                            vcl_mac_vlan_mgmt_mac_vlan_get(VTSS_ISID_LOCAL, &entry, VCL_MAC_VLAN_USER_STATIC, FALSE, FALSE);
                            /* Write the configuration to flash */
                            rc = vcl_mac_vlan_save_config();
                        }
                    }
                }
#endif
            }
        }
        break;
    }
    case VCL_MAC_VLAN_MSG_ID_CONF_DEL_REQ: {
        vcl_msg_conf_mac_vlan_add_del_req_t *msg;
        vtss_vce_id_t                       vce_id;
        T_D("VCL_MAC_VLAN_MSG_ID_CONF_DEL_REQ");
        msg = (vcl_msg_conf_mac_vlan_add_del_req_t *)rx_msg;
        rc = rc_conv(vcl_mac_vlan_local_entry_del(&msg->conf.smac));
        /* Call the switch API */
        if (rc == VTSS_RC_OK) {
            vce_id = msg->conf.id;
            vce_id = ((vce_id & 0xFFFF) | (((msg->conf.user) & 0xF) << 16) | ((VCL_TYPE_MAC & 0xF) << 20));
            /* Call the switch API */
            rc = vtss_vce_del(NULL, vce_id);
        }
        break;
    }
    case VCL_PROTO_VLAN_MSG_ID_CONF_SET_REQ: {
        u32                                     cnt;
        BOOL                                    next = FALSE, first = TRUE, entered = FALSE;
        vcl_msg_conf_proto_vlan_set_req_t       *msg;
        vcl_proto_vlan_local_sid_conf_t         entry;
        u32                                     id = 0;
        vtss_vce_id_t                           vce_id;
        T_D("VCL_PROTO_VLAN_MSG_ID_CONF_SET_REQ");
        /* Delete all the entries */
        while ((rc_conv(vcl_proto_vlan_local_entry_get(&entry, id, next))) == VTSS_RC_OK) {
            entered = TRUE;
            if (first != TRUE) {
                rc = rc_conv(vcl_proto_vlan_local_entry_delete(id));
            }
            id = entry.id;
            next = TRUE;
            first = FALSE;
            vce_id = entry.id;
            vce_id = ((vce_id & 0xFFFF) | (((entry.user) & 0xF) << 16) | ((VCL_TYPE_PROTO & 0xF) << 20));
            /* Call the switch API to delete vce entry of untagged frames*/
            rc = vtss_vce_del(NULL, vce_id);
            /* Call the switch API to delete vce entry of priority-tagged frames*/
            vce_id = (vce_id | (0x1 << 24));
            rc = vtss_vce_del(NULL, vce_id);
        }
        if (entered) {
            rc = rc_conv(vcl_proto_vlan_local_entry_delete(id));
        }
        msg = (vcl_msg_conf_proto_vlan_set_req_t *)rx_msg;
        /* Add all the new entries */
        for (cnt = 0; cnt < msg->count; cnt++) {
            entry.proto_encap_type = msg->conf[cnt].proto_encap_type;
            entry.proto = msg->conf[cnt].proto;
            entry.vid = msg->conf[cnt].vid;
            memcpy(entry.ports, msg->conf[cnt].ports, VTSS_PORT_BF_SIZE);
            entry.id = msg->conf[cnt].id;
            entry.user = msg->conf[cnt].user;
            rc = rc_conv(vcl_proto_vlan_local_entry_add(&entry));
            if (rc == VTSS_RC_OK) {
                /* Call the switch API */
                rc = vcl_proto_vlan_vce_entry_add(&entry);
            }
        }
        break;
    }
    case VCL_PROTO_VLAN_MSG_ID_CONF_ADD_REQ: {
        vcl_proto_vlan_local_sid_conf_t         entry;
        vcl_msg_conf_proto_vlan_add_del_req_t   *msg;
        T_D("VCL_PROTO_VLAN_MSG_ID_CONF_ADD_REQ");
        msg = (vcl_msg_conf_proto_vlan_add_del_req_t *)rx_msg;
        entry.proto_encap_type = msg->conf.proto_encap_type;
        entry.proto = msg->conf.proto;
        entry.vid = msg->conf.vid;
        entry.id = msg->conf.id;
        memcpy(entry.ports, msg->conf.ports, VTSS_PORT_BF_SIZE);
        entry.user = msg->conf.user;
        rc = rc_conv(vcl_proto_vlan_local_entry_add(&entry));
        if (rc == VTSS_RC_OK) {
            /* Call the switch API */
            rc = vcl_proto_vlan_vce_entry_add(&entry);
            if (rc != VTSS_RC_OK) {
                T_W("Protocol-based VLAN entry: Adding entry to the TCAM failed");
            }
        }
        break;
    }
    case VCL_PROTO_VLAN_MSG_ID_CONF_DEL_REQ: {
        vcl_msg_conf_proto_vlan_add_del_req_t   *msg;
        vtss_vce_id_t                           vce_id;
        T_D("VCL_PROTO_VLAN_MSG_ID_CONF_DEL_REQ");
        msg = (vcl_msg_conf_proto_vlan_add_del_req_t *)rx_msg;
        rc = rc_conv(vcl_proto_vlan_local_entry_delete(msg->conf.id));
        if (rc == VTSS_RC_OK) {
            vce_id = msg->conf.id;
            vce_id = ((vce_id & 0xFFFF) | (((msg->conf.user) & 0xF) << 16) | ((VCL_TYPE_PROTO & 0xF) << 20));
            /* Call the switch API to delete vce entry of untagged frame */
            rc = vtss_vce_del(NULL, vce_id);
            /* Call the switch API to delete vce entry of priority-tagged frames*/
            vce_id = (vce_id | (0x1 << 24));
            rc = vtss_vce_del(NULL, vce_id);
        }
        break;
    }
    case VCL_IP_VLAN_MSG_ID_CONF_SET_REQ: {
        vcl_msg_conf_ip_vlan_set_req_t   *msg;
        vtss_vce_id_t                    vce_id, next_id = 0;
        vcl_ip_vlan_msg_cfg_t            entry;
        u32                              cnt;

        T_D("VCL_IP_VLAN_MSG_ID_CONF_SET_REQ");
        msg = (vcl_msg_conf_ip_vlan_set_req_t *)rx_msg;
        /* Delete all the entries. Get first entry will always get first entry. Deletion of first entry results in
           next entry moving to first entry.
         */
        while (vcl_ip_vlan_local_entry_get(&entry, TRUE, FALSE) == VTSS_RC_OK) {
            /* Delete the HW entry */
            vce_id = entry.id;
            vce_id = ((vce_id & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20));
            /* Call the switch API to delete vce entry of untagged frames*/
            rc = vtss_vce_del(NULL, vce_id);
            /* Call the switch API to delete vce entry of priority-tagged frames*/
            vce_id = (vce_id | (0x1 << 24));
            rc = vtss_vce_del(NULL, vce_id);
            /* Delete the software entry */
            rc = rc_conv(vcl_ip_vlan_local_entry_delete(&entry));
        }
        /* Add all the new entries */
        for (cnt = 0; cnt < msg->count; cnt++) {
            rc = rc_conv(vcl_ip_vlan_local_entry_add(&(msg->conf[cnt]), &next_id));
            if (rc == VTSS_RC_OK) {
                /* Call the switch API */
                rc = vcl_ip_vlan_vce_entry_add(&(msg->conf[cnt]), next_id);
            }
        }
        break;
    }
    case VCL_IP_VLAN_MSG_ID_CONF_ADD_REQ: {
        vcl_msg_conf_ip_vlan_add_del_req_t   *msg;
        vtss_vce_id_t                        next_id = 0;
        T_D("VCL_IP_VLAN_MSG_ID_CONF_ADD_REQ");
        msg = (vcl_msg_conf_ip_vlan_add_del_req_t *)rx_msg;
        rc = rc_conv(vcl_ip_vlan_local_entry_add(&(msg->conf), &next_id));
        if (rc == VTSS_RC_OK) {
            rc = vcl_ip_vlan_vce_entry_add(&(msg->conf), next_id);
            if (rc != VTSS_RC_OK) {
                T_W("IP subnet-based VLAN entry: Adding entry to the hardware failed");
            }
        }
        break;
    }
    case VCL_IP_VLAN_MSG_ID_CONF_DEL_REQ: {
        vcl_msg_conf_ip_vlan_add_del_req_t   *msg;
        vtss_vce_id_t                        vce_id;
        T_D("VCL_IP_VLAN_MSG_ID_CONF_DEL_REQ");
        msg = (vcl_msg_conf_ip_vlan_add_del_req_t *)rx_msg;
        rc = rc_conv(vcl_ip_vlan_local_entry_delete(&(msg->conf)));
        if (rc == VTSS_RC_OK) {
            vce_id = msg->conf.id;
            vce_id = ((vce_id & 0xFFFF) | ((VCL_TYPE_IP & 0xF) << 20));
            /* Call the switch API to delete vce entry of untagged frame */
            rc = vtss_vce_del(NULL, vce_id);
            /* Call the switch API to delete vce entry of priority-tagged frames*/
            vce_id = (vce_id | (0x1 << 24));
            rc = vtss_vce_del(NULL, vce_id);
        }
        break;
    }
    default: {
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    }
    if (rc == VTSS_RC_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Register to the stack call back */
vtss_rc vcl_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = vcl_msg_rx;
    filter.modid = VTSS_MODULE_ID_VCL;
    return msg_rx_filter_register(&filter);
}

void vcl_l2ports2_ports(BOOL ports[][VTSS_PORT_ARRAY_SIZE], u8 *l2ports)
{
    u8          port_num;
    u32         num_ports, k;
    vtss_isid_t isid;

    num_ports = VTSS_ISID_CNT * VTSS_PORT_ARRAY_SIZE;
    memset(ports, 0, num_ports);
    for (k = 0; k < num_ports; k++) {
        isid = (k / VTSS_PORT_ARRAY_SIZE);
        port_num = k % VTSS_PORT_ARRAY_SIZE;
        ports[isid][port_num] = ((l2ports[k / 8] & (1 << (k % 8)))) ? TRUE : FALSE;
    }
}

/* Get ports bf from l2ports for given isid */
void vcl_l2ports2_portbf(u8 isid, u8 *ports_bf, u8 *l2ports, BOOL *ports_exist)
{
    u16   temp = (isid - VTSS_ISID_START) * VTSS_PORT_ARRAY_SIZE;
    u8    k;

    *ports_exist = FALSE;
    memset (ports_bf, 0, VTSS_PORT_BF_SIZE);
    for (k = 0; k < VTSS_PORT_ARRAY_SIZE; k++, temp++) {
        if ((l2ports[temp / 8] & (1 << (temp % 8)))) {
            ports_bf[k / 8] |= (1 << (k % 8));
            *ports_exist = TRUE;
        }
    }
}

static vtss_rc vcl_mac_vlan_set_vlan_tx_tag(u8 *l2ports, BOOL tx_tag_set)
{
    vlan_port_conf_t            vlan_cfg;
    BOOL                        ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
    vtss_rc                     rc = VTSS_RC_OK, rc1;
    switch_iter_t               sit;
    port_iter_t                 pit;

    vcl_l2ports2_ports(ports, l2ports);
    memset(&vlan_cfg, 0, sizeof(vlan_cfg));
    if (tx_tag_set == TRUE) {
        vlan_cfg.flags = VLAN_PORT_FLAGS_TX_TAG_TYPE;
        vlan_cfg.tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_ALL;
    }
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (ports[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START] == 1) {
                if ((rc1 = vlan_mgmt_port_conf_set(sit.isid, pit.iport, &vlan_cfg, VLAN_USER_VCL)) != VTSS_RC_OK) {
                    T_E("%u:%d: Unable to change VLAN tx tag type. Error = %s", sit.isid, pit.uport, error_txt(rc1));
                    rc += rc1;
                }
            }
        }
    }
    return rc;
}

static vtss_rc vcl_stack_vcl_mac_vlan_conf_set(vtss_isid_t isid)
{
    vcl_msg_conf_mac_vlan_set_req_t     *msg;
    vcl_mac_vlan_conf_entry_t           entry;
    u32                                 cnt, temp, indx;
    BOOL                                found_sid;
    vtss_rc                             rc = VTSS_RC_OK;
    vtss_port_no_t                      port;
    switch_iter_t                       sit;

    T_D("Enter, isid: %d", isid);
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        /* Initiate counter to count number of messages to be sent to sit.isid */
        cnt = 0;
        msg = vcl_msg_alloc(1);
        msg->msg_id = VCL_MAC_VLAN_MSG_ID_CONF_SET_REQ;
        /* Loop through all the entries in the db */
        for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
            /* Alternatively, vcl_mac_vlan_entry_get() also can be used. But, the below function is faster */
            rc = rc_conv(vcl_mac_vlan_entry_get_by_key(indx, VCL_MAC_VLAN_USER_STATIC, &entry));
            if (rc == VTSS_RC_OK) {
                found_sid = FALSE;
                for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
                    /* Calculate the position of the port bit */
                    temp = (sit.isid - 1) * VTSS_PORT_ARRAY_SIZE + port;
                    /* If the bit for that port is set in L2 ports array, set the corresponding bit in ports[] */
                    if (entry.l2ports[temp / 8] & (1 << (temp % 8))) {
                        msg->conf[cnt].ports[port / 8] |= (1 << (port % 8));
                        found_sid = TRUE;
                    }
                }
                /* If at least one port is part of this isid's configuration, then send the message to the isid */
                if (found_sid == FALSE) {
                    continue;
                }
                /* Set the VLAN tx_tag */
                if (vcl_mac_vlan_set_vlan_tx_tag(entry.l2ports, TRUE) != VTSS_RC_OK) {
                    T_W("Failed while applying configuration");
                }
                msg->conf[cnt].smac = entry.smac;
                msg->conf[cnt].vid = entry.vid;
                msg->conf[cnt].id = indx + 1;
                msg->conf[cnt].user = VCL_MAC_VLAN_USER_STATIC; /* Saved entry is always static (and not volatile)*/
                cnt++;
            }
        }
        T_D("Sending message to isid %d", sit.isid);
        msg->count = cnt;
        /* The below function also frees the msg after tx */
        vcl_msg_tx(msg, sit.isid, sizeof(*msg));
    }
    T_D("Exit, isid: %d", isid);
    return VTSS_RC_OK;
}

static vtss_rc vcl_stack_vcl_proto_vlan_conf_set(vtss_isid_t isid)
{
    vcl_msg_conf_proto_vlan_set_req_t *msg;
    vcl_proto_vlan_entry_t            entry;
    u32                               cnt, id = 0;
    BOOL                              next = FALSE;
    switch_iter_t                     sit;

    T_D("Enter, isid: %d", isid);
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        /* Initiate counter to count number of messages to be sent to isid */
        cnt = 0;
        msg = vcl_msg_alloc(1);
        msg->msg_id = VCL_PROTO_VLAN_MSG_ID_CONF_SET_REQ;

        /* Loop through all the entries in the db */
        while ((rc_conv(vcl_proto_vlan_hw_entry_get(sit.isid, &entry, id, VCL_PROTO_VLAN_USER_STATIC, next))) == VTSS_RC_OK) {
            msg->conf[cnt].proto_encap_type = entry.proto_encap_type;
            msg->conf[cnt].proto = entry.proto;
            msg->conf[cnt].vid = entry.vid;
            msg->conf[cnt].id = entry.vce_id;
            msg->conf[cnt].user = entry.user;
            memcpy(msg->conf[cnt].ports, entry.ports, VTSS_PORT_BF_SIZE);
            next = TRUE;
            id = entry.vce_id;
            cnt++;
        }

        T_D("Sending message to isid %d", sit.isid);
        msg->count = cnt;
        /* The below function also frees the msg after tx */
        vcl_msg_tx(msg, sit.isid, sizeof(*msg));
    }
    T_D("Exit");
    return VTSS_RC_OK;
}

static vtss_rc vcl_stack_vcl_ip_vlan_conf_set(vtss_isid_t isid)
{
    vcl_msg_conf_ip_vlan_set_req_t      *msg;
    vtss_isid_t                         isid_temp, isid_start, isid_end;
    vcl_ip_vlan_entry_t                 entry;
    u32                                 cnt, port;
    BOOL                                found_sid, first = TRUE, next = FALSE;
    BOOL                                ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];

    T_D("Enter, isid: %d", isid);
    if (isid == VTSS_ISID_GLOBAL) {
        isid_start = VTSS_ISID_START;
        isid_end = VTSS_ISID_END;
    } else {
        isid_start = isid;
        isid_end = isid + 1;
    }
    for (isid_temp = isid_start; isid_temp < isid_end; isid_temp++) {
        if (!msg_switch_exists(isid_temp)) {
            continue;
        }
        /* Initiate counter to count number of messages to be sent to isid_temp */
        cnt = 0;
        msg = vcl_msg_alloc(1);
        msg->msg_id = VCL_IP_VLAN_MSG_ID_CONF_SET_REQ;
        /* Loop through all the entries in the db */
        while (vcl_ip_vlan_entry_get(&entry, VCL_IP_VLAN_USER_STATIC, first, next) == VTSS_RC_OK) {
            found_sid = FALSE;
            memset(ports, 0, sizeof(ports));
            vcl_l2ports2_ports(ports, entry.l2ports);
            for (port = 0; port < VTSS_PORT_ARRAY_SIZE; port++) {
                VTSS_BF_SET(msg->conf[cnt].ports, port, ports[isid_temp - 1][port]);
                if (ports[isid_temp - 1][port]) {
                    found_sid = TRUE;
                }
            }
            //vcl_l2ports2_portbf(isid, msg->conf[cnt].ports, entry.l2ports, &found_sid);
            /* If at least one port is part of this isid's configuration, then send the message to the isid */
            if (found_sid == FALSE) {
                first = FALSE;
                next = TRUE;
                continue;
            }
            msg->conf[cnt].ip_addr = entry.ip_addr;
            msg->conf[cnt].mask_len = entry.mask_len;
            msg->conf[cnt].vid = entry.vid;
            msg->conf[cnt].id = entry.vce_id;
            cnt++;
            next = TRUE;
            first = FALSE;
        }
        T_D("Sending message to isid %d", isid_temp);
        msg->count = cnt;
        /* The below function also frees the msg after tx */
        vcl_msg_tx(msg, isid_temp, sizeof(*msg));
    }
    T_D("Exit, isid: %d", isid);
    return VTSS_RC_OK;
}

/* Read/create VCL stack configuration */
static void vcl_mac_conf_read_stack(BOOL create)
{
    conf_blk_id_t               blk_id;
    vcl_mac_vlan_table_blk_t    *vcl_blk;
    u32                         i, changed = FALSE;
    BOOL                        do_create;
    ulong                       size;
    vcl_mac_vlan_conf_entry_t   mac_vlan_entry;
    vtss_vce_id_t               id;
    u8                          l2ports[VTSS_L2PORT_BF_SIZE];

    T_D("Enter, create: %d", create);

    /* Read/create VCL MAC table configuration */
    blk_id = CONF_BLK_VCL_MAC_VLAN_TABLE;
    if (misc_conf_read_use()) {
        if (((vcl_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vcl_blk))) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            vcl_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vcl_blk));
            do_create = 1;
        } else if (vcl_blk->version != VCL_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if ((do_create == 1) && (vcl_blk != NULL)) {
            vcl_blk->version = VCL_BLK_VERSION;
            vcl_blk->count = 0;
            vcl_mac_vlan_default_set();
            /* Delete the volatile VLAN settings for tx_tag */
            for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
                /* Setting all ports */
                l2ports[i] = 0xFF;
            }
            if (vcl_mac_vlan_set_vlan_tx_tag(l2ports, FALSE) != VTSS_RC_OK) {
                T_W("Failed while setting defaults");
            }
            changed = TRUE;
        }
        if (do_create == 0) {
            if (vcl_blk == NULL) {
                T_W("Failed to open VCL table");
            } else {
                /* Add new VCLs */
                for (i = 0; i < vcl_blk->count; i++) {
                    mac_vlan_entry.smac = vcl_blk->table[i].smac;
                    mac_vlan_entry.vid = vcl_blk->table[i].vid;
                    memcpy(mac_vlan_entry.l2ports, vcl_blk->table[i].l2ports, VTSS_L2PORT_BF_SIZE);
                    (void)rc_conv(vcl_mac_vlan_entry_add(&mac_vlan_entry, &id, VCL_MAC_VLAN_USER_STATIC));
                }
            }
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    } else {
        // Not silent upgrade; ICFG: Load defaults
        vcl_mac_vlan_default_set();
        /* Delete the volatile VLAN settings for tx_tag */
        for (i = 0; i < VTSS_L2PORT_BF_SIZE; i++) {
            /* Setting all ports */
            l2ports[i] = 0xFF;
        }
        if (vcl_mac_vlan_set_vlan_tx_tag(l2ports, FALSE) != VTSS_RC_OK) {
            T_W("Failed while setting defaults");
        }
        changed = TRUE;
    }

    if (changed) {
        (void)vcl_stack_vcl_mac_vlan_conf_set(VTSS_ISID_GLOBAL);
    }
}
/* Read/create VCL Protocol stack configuration */
static void vcl_proto_conf_read_stack(BOOL create)
{
    conf_blk_id_t                           blk_id;
    vcl_proto_vlan_proto_table_blk_t        *vcl_blk;
    u32                                     i;
    BOOL                                    do_create;
    ulong                                   size;
    vcl_proto_vlan_proto_entry_t            proto_vlan_entry;

    T_D("Enter, create: %d", create);

    /* Read/create VCL Proto table configuration */
    blk_id = CONF_BLK_VCL_PROTO_VLAN_PRO_TABLE;
    if (misc_conf_read_use()) {
        if (((vcl_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vcl_blk))) {
            T_W("conf_sec_open() failed or size mismatch, creating defaults");
            vcl_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vcl_blk));
            do_create = 1;
        } else if (vcl_blk->version != VCL_BLK_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
        if ((do_create == 1) && (vcl_blk != NULL)) {
            vcl_blk->version = VCL_BLK_VERSION;
            vcl_blk->count = 0;
        }
        vcl_proto_vlan_proto_default_set();
        if (do_create == 0) {
            if (vcl_blk == NULL) {
                T_W("Failed to open VCL proto table");
            } else {
                /* Add new VCLs */
                for (i = 0; i < vcl_blk->count; i++) {
                    proto_vlan_entry = vcl_blk->table[i];
                    if (rc_conv(vcl_proto_vlan_proto_entry_add(&proto_vlan_entry, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                        T_N("Failed");
                    }
                }
            }
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    } else {
        // Not silent upgrade; ICFG: Load defaults
        vcl_proto_vlan_proto_default_set();
    }
}

/* Read/create VCL VLAN stack configuration */
static void vcl_proto_vlan_conf_read_stack(BOOL create)
{
    conf_blk_id_t                           blk_id;
    vcl_proto_vlan_vlan_table_blk_t         *vcl_blk;
    u32                                     i, changed = FALSE;
    BOOL                                    do_create;
    ulong                                   size;
    vcl_proto_vlan_flash_entry_t            *vlan_entry;
    vcl_proto_vlan_vlan_entry_t             entry;

    T_D("Enter, create: %d", create);

    /* Read/create VCL Proto table configuration */
    blk_id = CONF_BLK_VCL_PROTO_VLAN_VID_TABLE;
    if (misc_conf_read_use()) {
        if (((vcl_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vcl_blk))) {
            T_W("Conf_sec_open() failed or size mismatch, creating defaults");
            vcl_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vcl_blk));
            do_create = 1;
        } else if (vcl_blk->version != VCL_BLK_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
        if ((do_create == 1) && (vcl_blk != NULL)) {
            vcl_blk->version = VCL_BLK_VERSION;
            vcl_blk->count = 0;
            vcl_proto_vlan_vlan_default_set();
            changed = TRUE;
        }
        if (do_create == 0) {
            if (vcl_blk == NULL) {
                T_W("Failed to open VCL proto VLAN table");
            } else {
                /* Add new entries */
                for (i = 0; i < vcl_blk->count; i++) {
                    vlan_entry = &vcl_blk->table[i];
                    entry = vlan_entry->conf;
                    if (rc_conv(vcl_proto_vlan_group_entry_add(vlan_entry->isid, &entry, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                        T_N("Failed");
                    }
                }
            }
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    } else {
        // Not silent upgrade; ICFG: Load defaults
        vcl_proto_vlan_vlan_default_set();
        changed = TRUE;
    }

    if (changed == TRUE) {
        /* This will cleanup all the HW entries on stack units. This is only applicable to "sys res def".
           For "sys reb" case, cleaning up of h/w entries and adding entries from the flash is handled
           through INIT_CMD_SWITCH_ADD event */
        if (vcl_stack_vcl_proto_vlan_conf_set(VTSS_ISID_GLOBAL) != VTSS_RC_OK) {
            T_D("Returned error");
        }
    }
}

/* Read/create VCL stack configuration */
static void vcl_ip_conf_read_stack(BOOL create)
{
    conf_blk_id_t               blk_id;
    vcl_ip_vlan_table_blk_t     *vcl_blk;
    u32                         i, changed = FALSE;
    BOOL                        do_create;
    ulong                       size, allowed_size_2;
    vcl_ip_vlan_entry_t         ip_vlan_entry;

    T_D("enter, create: %d", create);

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    // At some point in time between 2.80e and 3.30, the VCL_IP_VLAN_MAX_ENTRIES
    // changed from 256 to 128 (because it turned out that the H/W only
    // supported 128 entries) causing the vcl_ip_vlan_table_blk_t structure to
    // be somewhat smaller. Since we need to silently be able to upgrade
    // from the old vconf-way of saving config to the new icfg-way, we need to
    // support the 256 entry format (and ignore entries above 128).
    // Luckily, the #table[] comes last in the vcl_ip_vlan_table_blk_t, so
    // we can do that by just finding the old size.
    allowed_size_2 = sizeof(struct {u32 version; u32 count; u32 size; vcl_ip_vlan_entry_t table[256];});
#else
    // Pretend that we only allow this size.
    allowed_size_2 = sizeof(*vcl_blk);
#endif

    /* Read/create VCL IP Subnet table configuration */
    blk_id = CONF_BLK_VCL_IP_VLAN_TABLE;
    if (misc_conf_read_use()) {
        if (((vcl_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) || (size != sizeof(*vcl_blk) && size != allowed_size_2)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            vcl_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*vcl_blk));
            do_create = 1;
        } else if (vcl_blk->version != VCL_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
        if ((do_create == 1) && (vcl_blk != NULL)) {
            vcl_blk->version = VCL_BLK_VERSION;
            vcl_blk->count = 0;
            vcl_ip_vlan_default_set();
            changed = TRUE;
        }
        if (do_create == 0) {
            if (vcl_blk == NULL) {
                T_W("Failed to open VCL table");
            } else {
                /* Add new VCLs */
                for (i = 0; i < vcl_blk->count; i++) {
                    ip_vlan_entry.ip_addr = vcl_blk->table[i].ip_addr;
                    ip_vlan_entry.mask_len = vcl_blk->table[i].mask_len;
                    ip_vlan_entry.vid = vcl_blk->table[i].vid;
                    memcpy(ip_vlan_entry.l2ports, vcl_blk->table[i].l2ports, VTSS_L2PORT_BF_SIZE);
                    ip_vlan_entry.vce_id = vcl_blk->table[i].vce_id;
                    (void)rc_conv(vcl_ip_vlan_entry_add(&ip_vlan_entry, VCL_IP_VLAN_USER_STATIC));
                }
            }
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    } else {
        // Not silent upgrade; ICFG: Load defaults
        vcl_ip_vlan_default_set();
        changed = TRUE;
    }

    if (changed) {
        (void)vcl_stack_vcl_ip_vlan_conf_set(VTSS_ISID_GLOBAL);
    }
}

void vcl_ports2_l2ports(vtss_isid_t isid, BOOL *ports, u8 *l2ports)
{
    u32                 temp;
    switch_iter_t       sit;
    port_iter_t         pit;

    memset(l2ports, 0, VTSS_L2PORT_BF_SIZE);
    /**
     * isid is converted to zero-based-isid to index into the array
     **/
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            temp = ((sit.isid - VTSS_ISID_START) * VTSS_PORT_ARRAY_SIZE) + pit.iport;
            l2ports[temp / 8] |= ports[pit.iport] << (temp % 8);
        }
    }
}

char *vcl_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case VCL_ERROR_GEN:
        txt = "VCL generic error";
        break;
    case VCL_ERROR_PARM:
        txt = "VCL parameter error";
        break;
    case VCL_ERROR_ENTRY_NOT_FOUND:
        txt = "Entry not found";
        break;
    case VCL_ERROR_TABLE_FULL:
        txt = "VCL table full";
        break;
    case VCL_ERROR_TABLE_EMPTY:
        txt = "VCL table empty";
        break;
    case VCL_ERROR_USER_PREVIOUSLY_CONFIGURED:
        txt = "VCL Error - Previously Configured";
        break;
    case VCL_ERROR_VCE_ID_PREVIOUSLY_CONFIGURED:
        txt = "VCL Error - VCE ID Previously Configured";
        break;
    case VCL_ERROR_ENTRY_PREVIOUSLY_CONFIGURED:
        txt = "VCL Error - entry previously configured";
        break;
    case VCL_ERROR_ENTRY_WITH_DIFF_NAME:
        txt = "VCL Error - entry exists with different VCE ID";
        break;
    case VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN:
        txt = "VCL Error - entry exists with different VCE ID and VLAN";
        break;
    case VCL_ERROR_ENTRY_WITH_DIFF_SUBNET:
        txt = "VCL Error - entry exists with different subnet";
        break;
    case VCL_ERROR_ENTRY_WITH_DIFF_VLAN:
        txt = "VCL Error - entry exists with different VLAN";
        break;
    default:
        txt = "VCL unknown error";
        break;
    }
    return txt;
}

/**
 * CALLOUT Function Definitions
 */
void vtss_vcl_crit_data_lock(void)
{
    VCL_CRIT_ENTER();
}

void vtss_vcl_crit_data_unlock(void)
{
    VCL_CRIT_EXIT();
}

void vtss_vcl_crit_data_assert_locked(void)
{
    VCL_CRIT_ASSERT_LOCKED();
}

u32 vcl_stack_vcl_proto_vlan_conf_add_del(vtss_isid_t isid, vcl_proto_vlan_entry_t *conf, BOOL add)
{
    vcl_msg_conf_proto_vlan_add_del_req_t   *msg;

    T_D("Enter, isid: %d", isid);
    VCL_CRIT_ASSERT_LOCKED();
    msg = vcl_msg_alloc(1);
    if (add) {
        msg->msg_id = VCL_PROTO_VLAN_MSG_ID_CONF_ADD_REQ;
    } else {
        msg->msg_id = VCL_PROTO_VLAN_MSG_ID_CONF_DEL_REQ;
    }
    msg->conf.id = conf->vce_id;
    msg->conf.proto_encap_type = conf->proto_encap_type;
    msg->conf.proto = conf->proto;
    msg->conf.vid = conf->vid;
    msg->conf.user = conf->user;
    memcpy(msg->conf.ports, conf->ports, VTSS_PORT_BF_SIZE);
    vcl_msg_tx(msg, isid, sizeof(*msg));
    T_D("Exit, isid: %d", isid);
    return VTSS_RC_OK;
}
/**
 * END of CALLOUT Function Definitions
 */

static vtss_rc vcl_stack_vcl_mac_vlan_conf_add_del(vtss_isid_t isid, vcl_mac_vlan_msg_cfg_t *cfg, BOOL add)
{
    vcl_msg_conf_mac_vlan_add_del_req_t *msg;
    switch_iter_t                       sit;

    T_D("Enter, isid: %d", isid);

    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = vcl_msg_alloc(sit.remaining)) != NULL) {
        if (add) {
            msg->msg_id = VCL_MAC_VLAN_MSG_ID_CONF_ADD_REQ;
        } else {
            msg->msg_id = VCL_MAC_VLAN_MSG_ID_CONF_DEL_REQ;
        }
        VCL_CRIT_ENTER();
        memcpy(&msg->conf, cfg, sizeof(msg->conf));
        VCL_CRIT_EXIT();
        while (switch_iter_getnext(&sit)) {
            vcl_msg_tx(msg, sit.isid, sizeof(*msg));
        }
    }

    T_D("Exit, isid: %d", isid);

    return VTSS_RC_OK;
}

vtss_rc vcl_mac_vlan_save_config(void)
{
    vtss_rc                     rc = VTSS_RC_OK;
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t               blk_id;
    vcl_mac_vlan_table_blk_t    *mac_vlan_blk;
    vcl_mac_vlan_conf_entry_t   entry;
    BOOL                        next = FALSE, first = TRUE;

    blk_id = CONF_BLK_VCL_MAC_VLAN_TABLE;
    if ((mac_vlan_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        return VTSS_VCL_ERROR_CONFIG_NOT_OPEN;
    }
    mac_vlan_blk->count = 0;
    while ((rc_conv(vcl_mac_vlan_entry_get(&entry.smac, VCL_MAC_VLAN_USER_STATIC, next, first, &entry)))
           == VTSS_RC_OK) {
        T_D("mac is %02x:%02x:%02x:%02x:%02x:%02x, vid = %d", entry.smac.addr[0], entry.smac.addr[1], entry.smac.addr[2],
            entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5], entry.vid);
        //memcpy(&mac_vlan_blk->table[mac_vlan_blk->count].smac, &entry.smac, sizeof(entry.smac));
        mac_vlan_blk->table[mac_vlan_blk->count].smac = entry.smac;
        mac_vlan_blk->table[mac_vlan_blk->count].vid = entry.vid;
        memcpy(mac_vlan_blk->table[mac_vlan_blk->count].l2ports, entry.l2ports, VTSS_L2PORT_BF_SIZE);
        mac_vlan_blk->count++;
        first = FALSE;
        next = TRUE;
    }
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    return rc;
}

vtss_rc vcl_mac_vlan_mgmt_mac_vlan_add(vtss_isid_t isid_add, vcl_mac_vlan_mgmt_entry_t *mac_vlan_entry,
                                       vcl_mac_vlan_user_t user)
{
    vcl_mac_vlan_conf_entry_t      entry;
    vtss_rc                        rc = VTSS_RC_OK;
    vcl_mac_vlan_msg_cfg_t         conf;
    vtss_vce_id_t                  indx;
    u8                             system_mac[6];
    u32                            i = 0;
    BOOL                           ports_temp[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
    switch_iter_t                  sit;

    do {
        if (!msg_switch_is_master()) {
            T_W("Not master");
            rc = VCL_ERROR_STACK_STATE;
            break;
        }
        /**
         * Check for ISID validity
         **/
        if (!(VTSS_ISID_LEGAL(isid_add) || (isid_add == VTSS_ISID_GLOBAL))) {
            T_E("Invalid ISID (%u). LEGAL expected", isid_add);
            rc = VCL_ERROR_PARM;
            break;
        }
        /**
         * Check the pointer for NULL
         **/
        if (mac_vlan_entry == NULL) {
            T_E("NULL pointer");
            rc = VCL_ERROR_PARM;
            break;
        }
        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            T_E("Invalid User");
            rc = VCL_ERROR_PARM;
            break;
        }
        /* System address and broadcast/multicast address cannot be used */
        if (conf_mgmt_mac_addr_get(system_mac, 0) == 0) {
            for (i = 0; i < 6; i++) {
                if (system_mac[i] != mac_vlan_entry->smac.addr[i]) {
                    break;
                }
            }
            /* i will be 6 only when system MAC matches entered MAC address */
            if (i == 6) {
                T_D("System MAC address cannot be used for MAC-based VLAN");
                rc = VCL_ERROR_PARM;
                break;
            }
        }

        /* Multicast or Broadcast MAC is identified by Least Significant Bit set in MSB of MAC */
        if ((mac_vlan_entry->smac.addr[0] & 0x1) == 1) {
            T_D("Multicast or Broadcast MAC address cannot be used for MAC-based VLAN");
            rc = VCL_ERROR_PARM;
            break;
        }

        T_D("Enter: mac: %02x:%02x:%02x:%02x:%02x:%02x, vid: %u, isid_add: %u, user %d",
            mac_vlan_entry->smac.addr[0], mac_vlan_entry->smac.addr[1],
            mac_vlan_entry->smac.addr[2], mac_vlan_entry->smac.addr[3], mac_vlan_entry->smac.addr[4],
            mac_vlan_entry->smac.addr[5], mac_vlan_entry->vid, isid_add, user);
        /**
         * Convert the vcl_mac_vlan_mgmt_entry_t to a vcl_mac_vlan_conf_entry_t.
         **/
        entry.smac = mac_vlan_entry->smac;
        entry.vid = mac_vlan_entry->vid;
        memset(entry.l2ports, 0, VTSS_L2PORT_BF_SIZE);
        /**
         *  Convert the ports to the l2ports bitfield.
         **/
        vcl_ports2_l2ports(isid_add, mac_vlan_entry->ports, entry.l2ports);

        /**
         * Call the base function to add the Mac-based VLAN entry.
         **/
        if ((rc = rc_conv(vcl_mac_vlan_entry_add(&entry, &indx, user))) == VTSS_RC_OK) {
            if (user == VCL_MAC_VLAN_USER_STATIC) {
                /* Write the configuration to flash */
                rc = vcl_mac_vlan_save_config();
            }
            if (rc == VTSS_RC_OK) {
                /* Set VLAN tx_tag */
                if ((rc = vcl_mac_vlan_set_vlan_tx_tag(entry.l2ports, TRUE)) != VTSS_RC_OK) {
                    T_W("Failed");
                }
                memcpy(&conf.smac, &mac_vlan_entry->smac, sizeof(vtss_mac_t));
                conf.vid = entry.vid;
                conf.id = indx;
                conf.user = user;
                /**
                 * Convert L2 ports to ports
                 **/
                vcl_l2ports2_ports(ports_temp, entry.l2ports);
                (void)switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);
                while (switch_iter_getnext(&sit)) {
                    /**
                     * Convert ports to port bit field.
                     **/
                    vcl_ports2_port_bitfield(ports_temp[sit.isid - 1], conf.ports);
                    rc = vcl_stack_vcl_mac_vlan_conf_add_del(sit.isid, &conf, TRUE);
                }
            }
        } else if (rc == VCL_ERROR_TABLE_FULL) {
            T_I("MAC-based VLAN Table full");
            break;
        }
    } while (0);

    T_D("Exit");
    return rc;
}

vtss_rc vcl_mac_vlan_mgmt_mac_vlan_del(vtss_isid_t isid_del, vtss_mac_t *mac_addr,
                                       vcl_mac_vlan_user_t user)
{
    vcl_mac_vlan_conf_entry_t      entry, entry_temp;
    vtss_rc                        rc = VTSS_RC_OK;
    BOOL                           ports[VTSS_PORT_ARRAY_SIZE];
    BOOL                           ports_temp[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
    u32                            port_num;
    vcl_mac_vlan_msg_cfg_t         conf;
    vtss_vce_id_t                  indx;
    switch_iter_t                  sit;

    do {
        if (!msg_switch_is_master()) {
            T_W("Not master");
            rc = VCL_ERROR_STACK_STATE;
            break;
        }

        /**
         * Check for ISID validity
         **/
        if (!(VTSS_ISID_LEGAL(isid_del) || (isid_del == VTSS_ISID_GLOBAL))) {
            T_E("Invalid ISID (%u). LEGAL expected", isid_del);
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check the pointer for NULL
         **/
        if (mac_addr == NULL) {
            T_E("NULL pointer");
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            T_E("Invalid User");
            rc = VCL_ERROR_PARM;
            break;
        }

        T_D("Enter: mac: %02x:%02x:%02x:%02x:%02x:%02x, isid_del: %u, user %d",
            mac_addr->addr[0], mac_addr->addr[1], mac_addr->addr[2], mac_addr->addr[3],
            mac_addr->addr[4], mac_addr->addr[5], isid_del, user);
        entry.smac = *mac_addr;

        for (port_num = 0; port_num < VTSS_PORT_ARRAY_SIZE; port_num++) {
            ports[port_num] = 1;
        }

        vcl_ports2_l2ports(isid_del, ports, entry.l2ports);
        if ((rc = rc_conv(vcl_mac_vlan_entry_get(mac_addr, user,
                                                 FALSE, FALSE, &entry_temp))) != VTSS_RC_OK) {
            T_D("Entry doesn't exist");
            rc = VCL_ERROR_ENTRY_NOT_FOUND;
            break;
        }

        /**
         * Call the base function to delete the Mac-based VLAN entry.
         **/
        if ((rc = rc_conv(vcl_mac_vlan_entry_del(&entry, &indx, user))) == VTSS_RC_OK) {
            if ((user == VCL_MAC_VLAN_USER_STATIC)) {
                /* Write the configuration to flash */
                rc = vcl_mac_vlan_save_config();
            }

            if (rc == VTSS_RC_OK) {
                /* Delete VLAN tx_tag setting for all ports */
                if ((rc = vcl_mac_vlan_set_vlan_tx_tag(entry_temp.l2ports, FALSE)) != VTSS_RC_OK) {
                    T_W("Failed");
                }
                memcpy(&conf.smac, mac_addr, sizeof(vtss_mac_t));
                conf.id = indx;
                conf.user = user;

                if (rc_conv(vcl_mac_vlan_entry_get(mac_addr, VCL_MAC_VLAN_USER_ALL, FALSE, FALSE, &entry))
                    != VTSS_RC_OK) {
                    /* Delete this entry as no user is configured */
                    rc = vcl_stack_vcl_mac_vlan_conf_add_del(isid_del, &conf, FALSE);
                } else {
                    conf.vid = entry.vid;
                    vcl_l2ports2_ports(ports_temp, entry.l2ports);
                    (void)switch_iter_init(&sit, isid_del, SWITCH_ITER_SORT_ORDER_ISID);
                    while (switch_iter_getnext(&sit)) {
                        vcl_ports2_port_bitfield(ports_temp[sit.isid - 1], conf.ports);
                        /* Modify this entry as another user is configured */
                        rc = vcl_stack_vcl_mac_vlan_conf_add_del(sit.isid, &conf, TRUE);
                    }
                }
            }
        }
    } while (0);

    T_D("Exit");
    return rc;
}

vtss_rc vcl_mac_vlan_mgmt_mac_vlan_get(vtss_isid_t isid_get, vcl_mac_vlan_mgmt_entry_get_cfg_t *mac_vlan_entry, vcl_mac_vlan_user_t user, BOOL next, BOOL first)
{
    vcl_mac_vlan_conf_entry_t      entry;
    vtss_rc                        rc = VTSS_RC_OK;
    BOOL                           found_sid = FALSE;
    port_iter_t                    pit;

    do {
        if (!msg_switch_is_master()) {
            T_W("Not master");
            rc = VCL_ERROR_STACK_STATE;
            break;
        }

        /**
         * Check for ISID validity
         **/
        if (!VTSS_ISID_LEGAL(isid_get)) {
            T_E("Invalid ISID (%u). LEGAL expected", isid_get);
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check the pointer for NULL
         **/
        if (mac_vlan_entry == NULL) {
            T_E("NULL pointer");
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user > VCL_MAC_VLAN_USER_ALL)) {
            T_E("Invalid User");
            rc = VCL_ERROR_PARM;
            break;
        }

        //TODO - Display MAC address, VID and ports as well.
        T_D("Enter: isid_get: %u, user %d", isid_get, user);
        memset(mac_vlan_entry->ports, 0, sizeof(mac_vlan_entry->ports));
        memset(&entry, 0, sizeof(entry));

        /**
         * Call the base function to fetch the Mac-based VLAN entry.
         **/
        while ((rc = rc_conv(vcl_mac_vlan_entry_get(&mac_vlan_entry->smac, user, next, first, &entry)))
               == VTSS_RC_OK) {
            T_D("mac is %02x:%02x:%02x:%02x:%02x:%02x, vid = %d", entry.smac.addr[0], entry.smac.addr[1], entry.smac.addr[2],
                entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5], entry.vid);
            vcl_l2ports2_ports(mac_vlan_entry->ports, entry.l2ports);
            (void)port_iter_init(&pit, NULL, isid_get, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (mac_vlan_entry->ports[isid_get - 1][pit.iport] == 1) {
                    found_sid = TRUE;
                    break;
                }
            }
            if (found_sid == FALSE) {
                next = TRUE;
                first = FALSE;
                memcpy(&mac_vlan_entry->smac, &entry.smac, sizeof(entry.smac));
                continue;
            }
            memcpy(&mac_vlan_entry->smac, &entry.smac, sizeof(entry.smac));
            mac_vlan_entry->vid = entry.vid;
            break;
        }
    } while (0);

    T_D("Exit");
    return rc;
}

vtss_rc vcl_mac_vlan_mgmt_local_isid_mac_vlan_get(vcl_mac_vlan_mgmt_entry_t *mac_vlan_entry, BOOL next, BOOL first)
{
    vtss_rc                             rc;
    vcl_mac_vlan_local_sid_conf_entry_t entry;
    port_iter_t                         pit;
    /**
     * Check the pointer for NULL
     **/
    if (mac_vlan_entry == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    memset(&entry, 0, sizeof(vcl_mac_vlan_local_sid_conf_entry_t));
    entry.mac = mac_vlan_entry->smac;
    T_D("mac is %02x:%02x:%02x:%02x:%02x:%02x, next = %d, first = %d", entry.mac.addr[0], entry.mac.addr[1], entry.mac.addr[2],
        entry.mac.addr[3], entry.mac.addr[4], entry.mac.addr[5], next, first);
    rc = rc_conv(vcl_mac_vlan_local_entry_get(&entry, next, first));
    if (rc == VTSS_RC_OK) {
        mac_vlan_entry->smac = entry.mac;
        mac_vlan_entry->vid = entry.vid;
        memset(mac_vlan_entry->ports, 0, VTSS_PORT_ARRAY_SIZE);
        (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            mac_vlan_entry->ports[pit.iport] = (entry.ports[pit.iport / 8] & (1 << (pit.iport % 8))) ? TRUE : FALSE;
        }
    }
    return rc;
}


vtss_rc vcl_mac_vlan_mgmt_mac_vlan_get_by_key(vtss_isid_t isid_get, u32 indx, vcl_mac_vlan_mgmt_entry_get_cfg_t *mac_vlan_entry, vcl_mac_vlan_user_t user)
{
    vcl_mac_vlan_conf_entry_t      entry;
    vtss_rc                        rc = VTSS_RC_OK;

    do {
        if (!msg_switch_is_master()) {
            T_W("Not master");
            rc = VCL_ERROR_STACK_STATE;
            break;
        }

        /**
         * Check for ISID validity
         **/
        if (!(VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL))) {
            T_E("Invalid ISID (%u). LEGAL expected", isid_get);
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check the pointer for NULL
         **/
        if (mac_vlan_entry == NULL) {
            T_E("NULL pointer");
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * Check for valid MAC-based VLAN User
         **/
        if ((user < VCL_MAC_VLAN_USER_STATIC) || (user >= VCL_MAC_VLAN_USER_ALL)) {
            T_E("Invalid User");
            rc = VCL_ERROR_PARM;
            break;
        }

        /**
         * validate index
         **/
        if (indx >= VCL_MAC_VLAN_MAX_ENTRIES) {
            T_D("Invalid Index");
            rc = VTSS_VCL_ERROR_PARM;
            break;  /* break from do-while */
        }

        //TODO - Display MAC address, VID and ports as well.
        T_D("Enter: isid_get: %u, user %d", isid_get, user);

        /**
         * Call the base function to fetch the Mac-based VLAN entry.
         **/
        if ((rc = rc_conv(vcl_mac_vlan_entry_get_by_key(indx, user, &entry))) == VTSS_RC_OK) {
            memcpy(&mac_vlan_entry->smac, &entry.smac, sizeof(entry.smac));
            mac_vlan_entry->vid = entry.vid;
            vcl_l2ports2_ports(mac_vlan_entry->ports, entry.l2ports);
        }
    } while (0);

    T_D("Exit");
    return rc;
}

char *vcl_mac_vlan_mgmt_vcl_user_to_txt(vcl_mac_vlan_user_t usr)
{
    switch (usr) {
    case VCL_MAC_VLAN_USER_STATIC:
        return "Static";
#ifdef VTSS_SW_OPTION_DOT1X
    case VCL_MAC_VLAN_USER_NAS:
        return "NAS";
#endif
    case VCL_MAC_VLAN_USER_ALL:
        return "Combined";
    }
    return "";
}

char *vcl_proto_vlan_mgmt_proto_type_to_txt(vcl_proto_encap_type_t encap)
{
    switch (encap) {
    case VCL_PROTO_ENCAP_ETH2:
        return "EthernetII";
    case VCL_PROTO_ENCAP_LLC_SNAP:
        return "LLC_SNAP";
    case VCL_PROTO_ENCAP_LLC_OTHER:
        return "LLC_Other";
    }
    return "";
}

static vtss_rc vcl_proto_vlan_proto_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t                        blk_id;
    vcl_proto_vlan_proto_table_blk_t     *vce_blk;
    vcl_proto_vlan_proto_entry_t        grp;
    ulong                               i = 0;
    BOOL                                first = TRUE, next = FALSE;

    blk_id = CONF_BLK_VCL_PROTO_VLAN_PRO_TABLE;
    if ((vce_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Failed to open VCL Protocol table");
        return VCL_ERROR_CONFIG_NOT_OPEN;
    }
    while ((rc_conv(vcl_proto_vlan_proto_entry_get(&grp, VCL_PROTO_VLAN_USER_STATIC, next, first))) == VTSS_RC_OK) {
        vce_blk->table[i++] = grp;
        first = FALSE;
        next = TRUE;
    }
    vce_blk->count = i;
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    return VTSS_RC_OK;
}

static vtss_rc vcl_proto_vlan_vlan_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t                       blk_id;
    vcl_proto_vlan_vlan_table_blk_t     *vce_blk;
    vcl_proto_vlan_vlan_entry_t         entry;
    ulong                               i = 0;
    BOOL                                first, next;
    switch_iter_t                       sit;

    blk_id = CONF_BLK_VCL_PROTO_VLAN_VID_TABLE;
    if ((vce_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Failed to open VCL Protocol table");
        return VCL_ERROR_CONFIG_NOT_OPEN;
    }
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        first = TRUE;
        next = FALSE;
        while ((rc_conv(vcl_proto_vlan_group_entry_get(sit.isid, &entry, VCL_PROTO_VLAN_USER_STATIC, next, first)))
               == VTSS_RC_OK) {
            vce_blk->table[i].conf = entry;
            vce_blk->table[i].isid = sit.isid;
            i++;
            first = FALSE;
            next = TRUE;
        }
    }
    vce_blk->count = i;
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    return VTSS_RC_OK;
}

vtss_rc vcl_proto_vlan_mgmt_proto_add(vcl_proto_vlan_proto_entry_t *proto_grp, vcl_proto_vlan_user_t user)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }

    /* Check for NULL pointer */
    if (proto_grp == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }

    T_D("Enter");
    if ((rc = rc_conv(vcl_proto_vlan_proto_entry_add(proto_grp, user))) == VTSS_RC_OK) {
        //VCL_TODO: Commit the configuration to flash and send the stack messages.
        if (rc == VTSS_OK && user == VCL_PROTO_VLAN_USER_STATIC) {
            rc = vcl_proto_vlan_proto_conf_commit();
        }
        /* Need not send any stack message, as the protocol to group configuration is global,
           not dependent on isid */
    }

    T_D("Exit");
    return rc;
}

vtss_rc vcl_proto_vlan_mgmt_proto_delete(vcl_proto_encap_type_t proto_encap_type, vcl_proto_conf_t *proto, vcl_proto_vlan_user_t user)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }

    /* Check for NULL pointer */
    if (proto == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }

    if ((rc = rc_conv(vcl_proto_vlan_proto_entry_delete(proto_encap_type, proto, user))) == VTSS_RC_OK) {
        /* VCL_TODO : flash commit and send a stack mesage */
        if (rc == VTSS_OK && user == VCL_PROTO_VLAN_USER_STATIC) {
            rc = vcl_proto_vlan_proto_conf_commit();
        }
    }

    return rc;
}

vtss_rc vcl_proto_vlan_mgmt_proto_get(vcl_proto_vlan_proto_entry_t *proto_grp, vcl_proto_vlan_user_t user, BOOL next, BOOL first)
{
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }

    /* Check for NULL pointer */
    if (proto_grp == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }

    return rc_conv(vcl_proto_vlan_proto_entry_get(proto_grp, user, next, first));
}
vtss_rc vcl_proto_vlan_mgmt_group_entry_add(vtss_isid_t isid_add, vcl_proto_vlan_vlan_entry_t *grp_vlan, vcl_proto_vlan_user_t user)
{
    vtss_rc                     rc = VTSS_RC_OK;
    switch_iter_t               sit;
    port_iter_t                 pit;
    vcl_proto_vlan_vlan_entry_t tmp_grp_vlan;

    /* Check for NULL pointer */
    if (grp_vlan == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }

    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }

    if (!(VTSS_ISID_LEGAL(isid_add) || (isid_add == VTSS_ISID_GLOBAL))) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_add);
        return VCL_ERROR_PARM;
    }

    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }

    /* If isid is VTSS_ISID_GLOBAL, loop through all the isids. */
    (void)switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        memset(&tmp_grp_vlan, 0, sizeof(tmp_grp_vlan));
        memcpy(tmp_grp_vlan.group_id, grp_vlan->group_id, sizeof(grp_vlan->group_id));
        tmp_grp_vlan.vid = grp_vlan->vid;
        (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            tmp_grp_vlan.ports[pit.iport] = grp_vlan->ports[pit.iport];
        }
        if ((rc = rc_conv(vcl_proto_vlan_group_entry_add(sit.isid, &tmp_grp_vlan, user))) == VTSS_RC_OK) {
            if (rc == VTSS_OK && user == VCL_PROTO_VLAN_USER_STATIC) {
                rc = vcl_proto_vlan_vlan_conf_commit();
            }
        }
    }
    return rc;
}

vtss_rc vcl_proto_vlan_mgmt_group_entry_delete(vtss_isid_t                       isid_del,
                                               vcl_proto_vlan_vlan_entry_t       *grp_vlan,
                                               vcl_proto_vlan_user_t             user)
{
    vtss_rc           rc = VTSS_RC_OK;
    switch_iter_t     sit;

    /* Check for NULL pointer */
    if (grp_vlan == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    if (!(VTSS_ISID_LEGAL(isid_del) || (isid_del == VTSS_ISID_GLOBAL))) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_del);
        return VCL_ERROR_PARM;
    }
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    /* If isid is VTSS_ISID_GLOBAL, loop through all the isids. */
    (void)switch_iter_init(&sit, isid_del, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        if ((rc = rc_conv(vcl_proto_vlan_group_entry_delete(sit.isid, grp_vlan, user)))
            == VTSS_RC_OK) {
            if (rc == VTSS_OK && user == VCL_PROTO_VLAN_USER_STATIC) {
                rc = vcl_proto_vlan_vlan_conf_commit();
            }
        }
    }
    return rc;
}

vtss_rc vcl_proto_vlan_mgmt_group_entry_get(vtss_isid_t                       isid_get,
                                            vcl_proto_vlan_vlan_entry_t       *grp_vlan,
                                            vcl_proto_vlan_user_t             user,
                                            BOOL                              next,
                                            BOOL                              first)
{
    vtss_rc           rc = VTSS_RC_OK;

    /* Check for NULL pointer */
    if (grp_vlan == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL)) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_get);
        return VCL_ERROR_PARM;
    }
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    if ((rc = rc_conv(vcl_proto_vlan_group_entry_get(isid_get, grp_vlan, user, next, first)))
        == VTSS_RC_OK) {
    }
    return rc;
}

vtss_rc vcl_proto_vlan_mgmt_group_entry_get_by_vlan(vtss_isid_t                       isid_get,
                                                    vcl_proto_vlan_vlan_entry_t       *grp_vlan,
                                                    vcl_proto_vlan_user_t             user,
                                                    BOOL                              next,
                                                    BOOL                              first)
{
    vtss_rc           rc = VTSS_RC_OK;

    /* Check for NULL pointer */
    if (grp_vlan == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_PROTO_VLAN_USER_STATIC) || (user >= VCL_PROTO_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    if (!VTSS_ISID_LEGAL(isid_get) || (isid_get == VTSS_ISID_GLOBAL)) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_get);
        return VCL_ERROR_PARM;
    }
    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    if ((rc = rc_conv(vcl_proto_vlan_group_entry_get_by_vlan(isid_get, grp_vlan, user, next, first)))
        == VTSS_RC_OK) {
    }
    return rc;
}

/* Debug function */
vtss_rc vcl_proto_vlan_mgmt_local_entry_get(vcl_proto_vlan_local_sid_conf_t *entry, u32 id, BOOL next)
{
    vtss_rc rc;
    if ((rc = rc_conv(vcl_proto_vlan_local_entry_get(entry, id, next))) != VTSS_RC_OK) {
        T_N("Failed");
    }
    return rc;
}
/* Debug function */
vtss_rc vcl_proto_vlan_mgmt_hw_entry_get(vtss_isid_t                       isid_get,
                                         vcl_proto_vlan_entry_t            *entry,
                                         u32                               id,
                                         vcl_proto_vlan_user_t             user,
                                         BOOL                              next)
{
    vtss_rc rc;
    if ((rc = rc_conv(vcl_proto_vlan_hw_entry_get(isid_get, entry, id, user, next))) != VTSS_RC_OK) {
        T_N("Failed");
    }
    return rc;
}

vtss_rc vcl_set_conf_to_default(void)
{
    u32                               indx;
    vcl_mac_vlan_mgmt_entry_get_cfg_t cfg;
    vtss_rc                           rc = VTSS_RC_OK;

    for (indx = 0; indx < VCL_MAC_VLAN_MAX_ENTRIES; indx++) {
        if ((rc = vcl_mac_vlan_mgmt_mac_vlan_get_by_key(VTSS_ISID_GLOBAL, indx, &cfg, VCL_MAC_VLAN_USER_STATIC)) == VTSS_RC_OK) {
            if ((vcl_mac_vlan_mgmt_mac_vlan_del(VTSS_ISID_GLOBAL, &cfg.smac, VCL_MAC_VLAN_USER_STATIC)) != VTSS_RC_OK) {
                T_D("Failed");
            }
        }
    }
    return rc;
}

static vtss_rc vcl_stack_vcl_ip_vlan_conf_add_del(vtss_isid_t isid, vcl_ip_vlan_msg_cfg_t *cfg, BOOL add)
{
    vcl_msg_conf_ip_vlan_add_del_req_t  *msg;
    switch_iter_t                       sit;

    T_D("Enter, isid: %u", isid);

    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = vcl_msg_alloc(sit.remaining)) != NULL) {
        if (add) {
            msg->msg_id = VCL_IP_VLAN_MSG_ID_CONF_ADD_REQ;
        } else {
            msg->msg_id = VCL_IP_VLAN_MSG_ID_CONF_DEL_REQ;
        }
        VCL_CRIT_ENTER();
        memcpy(&msg->conf, cfg, sizeof(msg->conf));
        VCL_CRIT_EXIT();

        while (switch_iter_getnext(&sit)) {
            vcl_msg_tx(msg, sit.isid, sizeof(*msg));
        }
    }
    T_D("Exit, isid: %u", isid);

    return VTSS_RC_OK;
}

static vtss_rc vcl_ip_vlan_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t                       blk_id;
    vcl_ip_vlan_table_blk_t             *vce_blk;
    vcl_ip_vlan_entry_t                 entry;
    ulong                               cnt = 0;
    BOOL                                first = TRUE, next = FALSE;

    T_D("Enter");
    blk_id = CONF_BLK_VCL_IP_VLAN_TABLE;
    if ((vce_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Failed to open VCL IP VLAN table");
        return VCL_ERROR_CONFIG_NOT_OPEN;
    }
    while ((rc_conv(vcl_ip_vlan_entry_get(&entry, VCL_IP_VLAN_USER_STATIC, first, next))) == VTSS_RC_OK) {
        vce_blk->table[cnt++] = entry;
        first = FALSE;
        next = TRUE;
    }
    vce_blk->count = cnt;
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
    return VTSS_RC_OK;
}

vtss_rc vcl_ip_vlan_mgmt_ip_vlan_add(vtss_isid_t                  isid_add,
                                     vcl_ip_vlan_mgmt_entry_t     *ip_vlan_entry,
                                     vcl_ip_vlan_user_t           user)
{
    vtss_rc                 rc = VTSS_RC_OK;
    vcl_ip_vlan_entry_t     entry;
    vtss_vce_id_t           vce_id = ip_vlan_entry->vce_id;
    vcl_ip_vlan_msg_cfg_t   cfg;
    BOOL                    ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
    switch_iter_t           sit;
    port_iter_t             pit;

    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user >= VCL_IP_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    /* Check for valid ISID */
    if (!(VTSS_ISID_LEGAL(isid_add) || (isid_add == VTSS_ISID_GLOBAL))) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_add);
        return VCL_ERROR_PARM;
    }
    T_D("isid = %u, user = %d, vce_id = %u", isid_add, user, ip_vlan_entry->vce_id);
    /* Check for valid VCE ID */
    if (ip_vlan_entry->vce_id > VCL_IP_VLAN_MAX_ENTRIES) {
        T_E("Invalid VCE ID");
        return VCL_ERROR_PARM;
    }
    if ((ip_vlan_entry->mask_len > 32) || (ip_vlan_entry->mask_len == 0)) {
        T_E("Mask length cannot be either greater than 32 or less than 1");
        return VCL_ERROR_PARM;
    }
    if (ip_vlan_entry->vid < VLAN_ID_MIN || ip_vlan_entry->vid > VLAN_ID_MAX) {
        T_E("Invalid VLAN ID");
        return VCL_ERROR_PARM;
    }
    T_D("ip = %u, mask length = %u, vid = %u vce id = %u",
        ip_vlan_entry->ip_addr, ip_vlan_entry->mask_len, ip_vlan_entry->vid, ip_vlan_entry->vce_id);
    memset(&entry, 0, sizeof(vcl_ip_vlan_entry_t));
    /* Update the IP subnet-based VLAN list entry */
    entry.ip_addr = ip_vlan_entry->ip_addr;
    entry.mask_len = ip_vlan_entry->mask_len;
    entry.vid = ip_vlan_entry->vid;
    vcl_ports2_l2ports(isid_add, ip_vlan_entry->ports, entry.l2ports);
    if (ip_vlan_entry->vce_id == 0) {
        if ((rc = rc_conv(vcl_ip_vlan_vce_id_get(&vce_id))) != VTSS_RC_OK) {
            T_D("Failed");
            return rc;
        }
        T_D("Auto-generated vce_id = %u", vce_id);
    }
    entry.vce_id = vce_id;
    if ((rc = rc_conv(vcl_ip_vlan_entry_add(&entry, user))) == VTSS_RC_OK) { /* add or update */
        if (rc == VTSS_RC_OK && user == VCL_IP_VLAN_USER_STATIC) {
            if ((rc = vcl_ip_vlan_conf_commit()) != VTSS_RC_OK) {
                rc = rc_conv(vcl_ip_vlan_entry_del(&entry, user));
                T_D("Failed");
                return rc;
            }
        }
        cfg.ip_addr = ip_vlan_entry->ip_addr;
        cfg.mask_len = ip_vlan_entry->mask_len;
        cfg.vid = ip_vlan_entry->vid;
        cfg.id = vce_id;
        memset(&entry, 0, sizeof(entry));
        entry.vce_id = vce_id;
        if ((rc = rc_conv(vcl_ip_vlan_entry_get(&entry, user, FALSE, FALSE))) == VTSS_RC_OK) {
            vcl_l2ports2_ports(ports, entry.l2ports);
            (void)switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                T_D("isid = %u", sit.isid);
                memset(cfg.ports, 0, sizeof(cfg.ports));
                (void)port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    /* First set the front ports for selected ISIDs. */
                    VTSS_BF_SET(cfg.ports, pit.iport, ports[sit.isid - 1][pit.iport]);
                }
                /* Send a stack message to update the slaves */
                if ((rc = vcl_stack_vcl_ip_vlan_conf_add_del(sit.isid, &cfg, TRUE)) != VTSS_RC_OK) {
                    T_D("stack message sent failed for isid = %d", sit.isid);
                }
            }
        }
    }
    T_D("Exit");
    return rc;
}

vtss_rc vcl_ip_vlan_mgmt_ip_vlan_del(vtss_isid_t                isid_del,
                                     vcl_ip_vlan_mgmt_entry_t   *ip_vlan_entry,
                                     vcl_ip_vlan_user_t         user)
{
    vtss_rc                 rc = VTSS_RC_OK;
    vcl_ip_vlan_entry_t     entry;
    vcl_ip_vlan_msg_cfg_t   cfg;
    switch_iter_t           sit;

    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user >= VCL_IP_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    if (!(VTSS_ISID_LEGAL(isid_del) || (isid_del == VTSS_ISID_GLOBAL))) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_del);
        return VCL_ERROR_PARM;
    }
    /* Check for valid VCE ID */
    if (ip_vlan_entry->vce_id > VCL_IP_VLAN_MAX_ENTRIES) {
        T_E("Invalid VCE ID");
        return VCL_ERROR_PARM;
    }
    T_D("Enter");
    entry.vce_id = ip_vlan_entry->vce_id;
    T_D("isid_del = %u, user = %u, vce_id = %u", isid_del, user, ip_vlan_entry->vce_id);
    // vcl_ports2_l2ports(isid_del, ip_vlan_entry->ports, entry.l2ports);
    memset(entry.l2ports, 0XFF, sizeof(entry.l2ports));
    if ((rc = rc_conv(vcl_ip_vlan_entry_del(&entry, user))) == VTSS_RC_OK) {
        if (rc == VTSS_OK && user == VCL_IP_VLAN_USER_STATIC) {
            if ((rc = vcl_ip_vlan_conf_commit()) != VTSS_RC_OK) {
                T_D("Failed");
                return rc;
            }
        }
        cfg.ip_addr = ip_vlan_entry->ip_addr;
        cfg.mask_len = ip_vlan_entry->mask_len;
        cfg.vid = ip_vlan_entry->vid;
        cfg.id = ip_vlan_entry->vce_id;
        /* Delete all ports */
        memset(cfg.ports, 0xFF, sizeof(cfg.ports));
        (void)switch_iter_init(&sit, isid_del, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            /* Send a stack message to update the slaves */
            if ((rc = vcl_stack_vcl_ip_vlan_conf_add_del(sit.isid, &cfg, FALSE)) != VTSS_RC_OK) {
                T_D("stack message sent failed for isid = %d", sit.isid);
            }
        }
    }
    T_D("Exit");
    return rc;
}

// See vcl-api.h
vtss_rc vcl_ip_vlan_mgmt_ip_vce_id_get(vtss_isid_t                          isid_get,
                                       vcl_ip_vlan_mgmt_entry_t             *ip_vlan_entry,
                                       vcl_ip_vlan_user_t                   user,
                                       u16                                  vce_id)
{

    BOOL next = FALSE;
    BOOL first = TRUE;

    // Loop through all entries and find the specific entry
    while (vcl_ip_vlan_mgmt_ip_vlan_get(isid_get, ip_vlan_entry, user, first, next) == VTSS_RC_OK) {
        next = TRUE;
        first = FALSE;
        if (ip_vlan_entry->vce_id == vce_id) {
            return VTSS_RC_OK;
        }
    }

    // No entry with the specific vce_id found
    return VCL_ERROR_ENTRY_NOT_FOUND;
}


vtss_rc vcl_ip_vlan_mgmt_ip_vlan_get(vtss_isid_t                          isid_get,
                                     vcl_ip_vlan_mgmt_entry_t             *ip_vlan_entry,
                                     vcl_ip_vlan_user_t                   user,
                                     BOOL                                 first,
                                     BOOL                                 next)
{
    vtss_rc             rc = VTSS_RC_OK;
    vcl_ip_vlan_entry_t entry;
    BOOL                ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t      port;
    BOOL                found_sid = FALSE;

    if (!msg_switch_is_master()) {
        T_W("Not master");
        return VCL_ERROR_STACK_STATE;
    }
    /* Check for NULL pointer */
    if (ip_vlan_entry == NULL) {
        T_E("NULL pointer");
        return VCL_ERROR_PARM;
    }
    /* Check for valid User */
    if ((user < VCL_IP_VLAN_USER_STATIC) || (user >= VCL_IP_VLAN_USER_ALL)) {
        T_E("Invalid User");
        return VCL_ERROR_PARM;
    }
    if (!(VTSS_ISID_LEGAL(isid_get))) {
        T_E("Invalid ISID (%u). LEGAL expected", isid_get);
        return VCL_ERROR_PARM;
    }
    T_D("isid_get = %u, user = %d, first = %d, next = %d, vce_id = %u",
        isid_get, user, first, next, ip_vlan_entry->vce_id);
    entry.vce_id = ip_vlan_entry->vce_id;
    while ((rc = rc_conv(vcl_ip_vlan_entry_get(&entry, user, first, next))) == VTSS_RC_OK) {
        vcl_l2ports2_ports(ports, entry.l2ports);
        for (port = VTSS_PORT_NO_START; port < VTSS_PORT_NO_END; port++) {
            if (ports[isid_get - 1][port] == 1) {
                found_sid = TRUE;
                break;
            }
        }
        if (found_sid == FALSE) {
            next = TRUE;
            first = FALSE;
            continue;
        }
        ip_vlan_entry->vce_id = entry.vce_id;
        ip_vlan_entry->vid = entry.vid;
        ip_vlan_entry->ip_addr = entry.ip_addr;
        ip_vlan_entry->mask_len = entry.mask_len;
        memcpy(ip_vlan_entry->ports, ports[isid_get - 1], sizeof(ip_vlan_entry->ports));
        break;
    }
    T_D("Exit");

    return rc;
}

vtss_rc vcl_debug_policy_no_set(vtss_acl_policy_no_t policy_no)
{
    if ((policy_no < VTSS_ACL_POLICIES) || (policy_no == VTSS_ACL_POLICY_NO_NONE)) {
        VCL_CRIT_ENTER();
        vcl_debug_policy_no = policy_no;
        VCL_CRIT_EXIT();
        return VTSS_RC_OK;
    }
    return VTSS_RC_ERROR;
}

vtss_rc vcl_debug_policy_no_get(vtss_acl_policy_no_t *policy_no)
{
    if (policy_no) {
        VCL_CRIT_ENTER();
        *policy_no = vcl_debug_policy_no;
        VCL_CRIT_EXIT();
        return VTSS_RC_OK;
    }
    return VTSS_RC_ERROR;
}

/* Initialize module */
vtss_rc vcl_init(vtss_init_data_t *data)
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
        T_D("INIT_CMD_INIT");
        /* Initializing the local data structures */
        vcl_default_set();
        /* Create message pool. In standalone, we need three buffers because of three requests
         * in the SWITCH_ADD event, and in stacking, we need one per switch. */
        // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of VTSS_ISID_CNT below
        /*lint -e{506} */
        vcl_request_pool = msg_buf_pool_create(VTSS_MODULE_ID_VCL, "Request", VTSS_ISID_CNT > 3 ? VTSS_ISID_CNT : 3, sizeof(vcl_msg_req_t));
        /* Create semaphore for critical regions */
        critd_init(&vcl_data_lock, "vcl_data_lock", VTSS_MODULE_ID_VCL, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        VCL_CRIT_EXIT();
#ifdef VTSS_SW_OPTION_VCLI
        (void)vcl_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = VCL_icfg_init()) != VTSS_OK) {
            T_D("Failed (rc = %s)", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("INIT_CMD_START");
        /* PPP1004 : Initialize the things that might perform blocking opearations as
           scheduler has been started. Also, register callbacks from other modules */
        rc = vcl_stack_register();
        break;
    case INIT_CMD_CONF_DEF:
        T_D("INIT_CMD_CONF_DEF");
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            vcl_mac_conf_read_stack(1);
            vcl_proto_conf_read_stack(1);
            vcl_proto_vlan_conf_read_stack(1);
            vcl_ip_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("INIT_CMD_MASTER_UP");
        /* Read stack and switch configuration */
        vcl_mac_conf_read_stack(0);
        vcl_proto_conf_read_stack(0);
        vcl_proto_vlan_conf_read_stack(0);
        vcl_ip_conf_read_stack(0);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("INIT_CMD_MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("INIT_CMD_SWITCH_ADD");
        /* Apply all configuration to switch */
        rc = vcl_stack_vcl_proto_vlan_conf_set(isid);
        if (rc != VTSS_RC_OK) {
            T_D("rc = %s", error_txt(rc));
            return rc;
        }
        rc = vcl_stack_vcl_ip_vlan_conf_set(isid);
        if (rc != VTSS_RC_OK) {
            T_D("rc = %s", error_txt(rc));
            return rc;
        }
        rc = vcl_stack_vcl_mac_vlan_conf_set(isid);
        if (rc != VTSS_RC_OK) {
            T_D("rc = %s", error_txt(rc));
            return rc;
        }
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("INIT_CMD_SWITCH_DEL");
        break;
    default:
        break;
    }
    return rc;
}

