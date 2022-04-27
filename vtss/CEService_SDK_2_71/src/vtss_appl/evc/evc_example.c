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

#include "main.h"

#if defined(VTSS_ARCH_SERVAL)

/* Include management APIs */
#include "sysutil_api.h"
#if defined(VTSS_SW_OPTION_MSTP)
#include "mstp_api.h"
#endif /* VTSS_SW_OPTION_MSTP */
#if defined(VTSS_SW_OPTION_LLDP)
#include "lldp_api.h"
#endif /* VTSS_SW_OPTION_LLDP */
#include "vlan_api.h"
#if defined(VTSS_SW_OPTION_QOS)
#include "qos_api.h"
#endif /* VTSS_SW_OPTION_QOS */
#if defined(VTSS_SW_OPTION_ACL)
#include "acl_api.h"
#endif /* VTSS_SW_OPTION_ACL */
#include "evc_api.h"
#if defined(VTSS_SW_OPTION_MEP)
#include "mep_api.h"
#endif /* VTSS_SW_OPTION_MEP */

/* EVC test command flags */
#define EVC_TEST_CMD_DEF_CONF   0x0001 /* Create defaults */
#define EVC_TEST_CMD_EVPL       0x0002 /* EPL/EVPL */
#define EVC_TEST_CMD_PRESERVE   0x0004 /* Tag preservation (for EVPL) */
#define EVC_TEST_CMD_MEP_SWAP   0x0008 /* Swap MEP IDs */
#define EVC_TEST_CMD_SIMPLE_NNI 0x0010 /* Simple NNI */

/* Create defaults, preserving system configuration */
static void evc_default_conf(u32 cmd)
{
    system_conf_t conf;
    
    if ((cmd & EVC_TEST_CMD_DEF_CONF) && system_get_config(&conf) == VTSS_RC_OK) {
        control_config_reset(VTSS_USID_ALL, 0);
        (void)system_set_config(&conf);
    }
}

#if defined(VTSS_SW_OPTION_MSTP)
/* MSTP configuration */
static void evc_mstp_conf(vtss_port_no_t uni, vtss_port_no_t nni)
{
    vtss_isid_t       isid = VTSS_ISID_START;  /* Switch ID */
    mstp_port_param_t conf;                    /* Port configuration */
    BOOL              enable;                  /* Mode */
    vtss_port_no_t    port_no;
    u32               i;

    /* On UNI/NNI, disable MSTP */
    for (i = 0; i < 2; i++) {
        port_no = (i ? nni : uni);
        if (mstp_get_port_config(isid, port_no, &enable, &conf)) {
            enable = 0;
            (void)mstp_set_port_config(isid, port_no, enable, &conf);
        }
    }
}
#endif /* VTSS_SW_OPTION_MSTP */

#if defined(VTSS_SW_OPTION_LLDP)
/* LLDP configuration */
static void evc_lldp_conf(vtss_port_no_t uni, vtss_port_no_t nni)
{
    vtss_isid_t    isid = VTSS_ISID_START;  /* Switch ID */
    lldp_struc_0_t conf;                    /* Port configuration for all ports */
    
    /* On UNI/NNI, disable LLDP */
    lldp_mgmt_get_config(&conf, isid);
    conf.admin_state[uni] = LLDP_DISABLED;
    conf.admin_state[nni] = LLDP_DISABLED;
    (void)lldp_mgmt_set_admin_state(conf.admin_state, isid);
}
#endif /* VTSS_SW_OPTION_LLDP */

/* VLAN configuration */
static void evc_vlan_conf(vtss_port_no_t uni, vtss_port_no_t nni)
{
    vtss_isid_t       isid = VTSS_ISID_START;  /* Switch ID */
    vlan_user_t       user = VLAN_USER_STATIC; /* VLAN user ID */
    vlan_port_conf_t  conf;                    /* Port configuration */
    vlan_mgmt_entry_t entry;                   /* VLAN membership */
    vtss_vid_t        vid = VLAN_ID_DEFAULT;   /* Default VLAN */
    vtss_port_no_t    port_no;
    u32               i;

    /* Exclude UNI and NNI port from default VLAN */
    if (vlan_mgmt_vlan_get(isid, vid, &entry, 0, user) == VTSS_RC_OK) {
        entry.ports[uni] = 0;
        entry.ports[nni] = 0;
        (void)vlan_mgmt_vlan_add(isid, &entry, user);
    }
    
    /* UNI is C-port, NNI is S-port */
    for (i = 0; i < 2; i++) {
        port_no = (i ? nni : uni);
        if (vlan_mgmt_port_conf_get(isid, port_no, &conf, user) == VTSS_RC_OK) {
            conf.port_type = (i ? VLAN_PORT_TYPE_S : VLAN_PORT_TYPE_C);
            conf.pvid = VTSS_VID_RESERVED;
            conf.flags = VLAN_PORT_FLAGS_ALL;
            (void)vlan_mgmt_port_conf_set(isid, port_no, &conf, user);
        }
    }
}

#if defined(VTSS_SW_OPTION_QOS)
/* QoS configuration */
static void evc_qos_conf(vtss_port_no_t uni, vtss_port_no_t nni)
{
    vtss_isid_t     isid = VTSS_ISID_START;  /* Switch ID */
    qos_port_conf_t conf;
    vtss_port_no_t  port_no;
    u32             i, pcp, dei;

    /* On UNI/NNI, enable 1:1 mapping from PCP to QoS class */
    for (i = 0; i < 2; i++) {
        port_no = (i ? nni : uni);
        if (qos_port_conf_get(isid, port_no, &conf) == VTSS_RC_OK) {
            conf.tag_class_enable = 1;
            conf.tag_remark_mode = VTSS_TAG_REMARK_MODE_MAPPED;
            for (pcp = 0; pcp < 2; pcp++) {
                for (dei = 0; dei < 2; dei++) {
                    conf.qos_class_map[pcp][dei] = pcp;
                    conf.tag_pcp_map[pcp][dei] = pcp;
                }
            }
            (void)qos_port_conf_set(isid, port_no, &conf);
        }
    }
}
#endif /* VTSS_SW_OPTION_QOS */

#if defined(VTSS_SW_OPTION_ACL)
/* ACL configuration */
static void evc_acl_conf(vtss_port_no_t uni, u8 l2cp_id, vtss_acl_policy_no_t policy, u32 cmd)
{
    acl_entry_conf_t ace;
    vtss_port_no_t   port_no;
    u32              i;
    
    if ((cmd & EVC_TEST_CMD_EVPL) && acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &ace) == VTSS_RC_OK) {
        ace.id = 1;
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            ace.port_list[port_no] = (port_no == uni ? 1 : 0);
            ace.action.port_list[port_no] = 0;
        }
        ace.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
        ace.policy.value = policy;
        ace.policy.mask = 0x3f;
        for (i = 0; i < 6; i++) {
            ace.frame.etype.dmac.value[i] = (i == 0 ? 0x01 : i == 1 ? 0x80 : i == 2 ? 0xc2 :
                                             i == 5 ? l2cp_id : 0);
            ace.frame.etype.dmac.mask[i] = 0xff;
        }
        (void)acl_mgmt_ace_add(ACL_USER_STATIC, ACE_ID_NONE, &ace);
    }
}
#endif /* VTSS_SW_OPTION_ACL */

/* EVC port configuration */
static void evc_port_conf(vtss_port_no_t uni, vtss_port_no_t nni, u8 l2cp_id, u32 cmd)
{
    evc_mgmt_port_conf_t conf;
    vtss_port_no_t       port_no;
    u32                  i,j;

    /* On UNI/NNI, use quarter key and setup L2CP forwarding */
    for (i = 0; i < 2; i++) {
        port_no = (i ? nni : uni);
        if (evc_mgmt_port_conf_get(port_no, &conf) == VTSS_RC_OK) {
            /* Quarter rules */
            conf.conf.key_type = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG;
            
            /* L2CP forwarding */
            for (j = 0; j < 16; j++) {
                conf.reg.bpdu_reg[j] = VTSS_PACKET_REG_FORWARD;
                conf.reg.garp_reg[j] = VTSS_PACKET_REG_FORWARD;
            }
            if ((cmd & EVC_TEST_CMD_EVPL) == 0 && i == 0) {
                /* EP-Line, discard L2CP for UNI port */
                conf.reg.bpdu_reg[l2cp_id] = VTSS_PACKET_REG_DISCARD;
            }
            (void)evc_mgmt_port_conf_set(port_no, &conf);
        }
    }
}

/* EVC configuration */
static void evc_conf(vtss_evc_id_t evc_id, vtss_port_no_t nni, vtss_vid_t s_vid)
{
    evc_mgmt_conf_t    conf;
    vtss_evc_pb_conf_t *pb = &conf.conf.network.pb;
    
    memset(&conf, 0, sizeof(conf));
    pb->nni[nni] = 1; /* NNI port */
    pb->ivid = s_vid; /* Internal/classified VID */
    pb->vid = s_vid;  /* S-VID used in outer tag */
    (void)evc_mgmt_add(evc_id, &conf);
}

/* ECE configuration for two service classes */
static void evc_ece_conf(vtss_evc_id_t evc_id, vtss_port_no_t uni, vtss_vid_t ce_vid,
                         vtss_evc_policer_id_t policer_low, vtss_evc_policer_id_t policer_high,
                         vtss_acl_policy_no_t policy, u32 cmd)
{
    vtss_ece_id_t       ece_id = 1; /* First ECE ID */
    evc_mgmt_ece_conf_t conf;       /* ECE configuration */
    vtss_ece_key_t      *key = &conf.conf.key;
    vtss_ece_action_t   *action = &conf.conf.action;
    u32                 i;

    for (i = 0; i < 2; i++) {
        memset(&conf, 0, sizeof(conf));
        conf.conf.id = ece_id++;
        key->port_list[uni] = VTSS_ECE_PORT_ROOT;
        if (cmd & EVC_TEST_CMD_EVPL) {
            /* EVP-Line: Match specific CE-VID */
            key->tag.vid.vr.v.value = ce_vid;
            key->tag.vid.vr.v.mask = 0xfff;
            action->policy_no = policy;
            if (!(cmd & EVC_TEST_CMD_PRESERVE)) {
                /* Pop tag if tag preservation is disabled */
                action->pop_tag = VTSS_ECE_POP_TAG_1;
            }
        }
        if (i == 0) {
            /* First ECE: Match PCP 4-7 in C-tag, put PCP 4 in S-tag */
            key->tag.tagged = VTSS_VCAP_BIT_1;
            key->tag.s_tagged = VTSS_VCAP_BIT_0;
            key->tag.pcp.value = 0x4;
            key->tag.pcp.mask = 0x4;
            action->outer_tag.pcp = 4;
            action->prio = 4;
            action->policer_id = policer_high;
        } else {
            /* Last ECE: Match all other frames, put PCP 0 in S-tag */
            action->outer_tag.pcp = 0;
            action->prio = 0;
            action->policer_id = policer_low;
        }
        action->dir = VTSS_ECE_DIR_BOTH; /* Bidirectional rule */
        if (cmd & EVC_TEST_CMD_SIMPLE_NNI) {
            /* Simple NNI, reduced resource consumption */
            action->rule = (i ? VTSS_ECE_RULE_RX : VTSS_ECE_RULE_BOTH); /* Only Rx for last ECE */
            action->tx_lookup = VTSS_ECE_TX_LOOKUP_VID;                 /* Tx lookup with VID */
            action->outer_tag.pcp_mode = VTSS_ECE_PCP_MODE_MAPPED;      /* Mapped outer tag PCP */
        } else {
            /* Advanced NNI */
            action->rule = VTSS_ECE_RULE_BOTH;                    /* Ingress and egress rules */
            action->tx_lookup = VTSS_ECE_TX_LOOKUP_VID_PCP;       /* Tx lookup with (VID, PCP) */
            action->outer_tag.pcp_mode = VTSS_ECE_PCP_MODE_FIXED; /* Fixed outer tag PCP */
        }
        action->outer_tag.dei_mode = VTSS_ECE_DEI_MODE_FIXED;
        action->evc_id = evc_id;
        action->prio_enable = 1;
        (void)evc_mgmt_ece_add(VTSS_ECE_ID_LAST, &conf);
    }
}

/* Policer configuration for low and high priority */
void evc_policer_conf(vtss_evc_policer_id_t policer_low, vtss_evc_policer_id_t policer_high)
{
    evc_mgmt_policer_conf_t conf; /* Policer configuration */
    vtss_evc_policer_conf_t *pol = &conf.conf;
    vtss_evc_policer_id_t   policer;
    u32                     i;

    for (i = 0; i < 2; i++) {
        policer = (i ? policer_high : policer_low);
        if (evc_mgmt_policer_conf_get(policer, &conf) == VTSS_RC_OK) {
            pol->enable = 1;
            pol->cf = 0;     /* Coupling flag             : Disabled */
            pol->cir = 3000; /* Committed Information Rate: 3 Mbps */
            pol->cbs = 4096; /* Committed Burst Size      : 4096 bytes */
            pol->eir = 1000; /* Excess Information Rate   : 1 Mpbs */
            pol->ebs = 4096; /* Excess Burst Size         : 4096 bytes */
            (void)evc_mgmt_policer_conf_set(policer, &conf);
        }
    }
}

#if defined(VTSS_SW_OPTION_MEP)
/* OAM configuration */
static void evc_oam_conf(vtss_evc_id_t evc_id, vtss_port_no_t uni, u32 cmd)
{
    vtss_mep_mgmt_conf_t    conf;
    vtss_mep_mgmt_cc_conf_t cc_conf;
    u8                      mac[VTSS_MEP_MAC_LENGTH];
    u32                     eps_count;
    u16                     eps_inst[MEP_EPS_MAX];
    u32                     inst_evc_up = 19;

    /* Create EVC up MEP */
    if (mep_mgmt_conf_get(inst_evc_up, mac, &eps_count, eps_inst, &conf) == VTSS_RC_OK) {
        conf.enable = 1;
        conf.mode = VTSS_MEP_MGMT_MEP;
        conf.direction = VTSS_MEP_MGMT_UP;
        conf.domain = VTSS_MEP_MGMT_EVC;
        conf.flow = evc_id;
        conf.port = uni;
        conf.level = 2;
        conf.vid = 0;
        conf.voe = 1;
        conf.format = VTSS_MEP_MGMT_ITU_ICC;
        strcpy(conf.name, "VITESS");
        strcpy(conf.meg, "meg002");
        conf.mep = ((cmd & EVC_TEST_CMD_MEP_SWAP) ? 2 : 1);
        conf.peer_count = 1;
        conf.peer_mep[0] = (conf.mep == 1 ? 2 : 1);
        if (mep_mgmt_conf_set(inst_evc_up, &conf) == VTSS_RC_OK &&
            mep_mgmt_cc_conf_get(inst_evc_up, &cc_conf) == VTSS_RC_OK) {
            /* Enable CCM with 1 second interval */
            cc_conf.enable = 1;
            cc_conf.prio = 0;
            cc_conf.period = VTSS_MEP_MGMT_PERIOD_1S;
            (void)mep_mgmt_cc_conf_set(inst_evc_up, &cc_conf);
        }
    }
}
#endif /* VTSS_SW_OPTION_MEP */

/* EP-Line/EVC-Line test configuration */
void evc_example(u32 cmd)
{
    /* Fixed EVC parameters */
    vtss_port_no_t        uni = 0;      /* UNI port */
    vtss_port_no_t        nni = 2;      /* NNI port */
    vtss_vid_t            s_vid = 1000; /* S-VID, also used as internal/classified VID */
    vtss_vid_t            ce_vid = 17;  /* CE-VID for EVPL */
    u8                    l2cp_id = 14; /* Discarded L2CP, 14 means 01-80-C2-00-00-0E (LLDP) */

    /* Allocated EVC management API parameters */
    vtss_evc_id_t         evc_id = 9;       /* EVC ID */
    vtss_acl_policy_no_t  policy = 42;      /* ACL policy used for L2CP control */
    vtss_evc_policer_id_t policer_high = 1; /* High priority policer */
    vtss_evc_policer_id_t policer_low = 2;  /* Low priority policer */

    /* Configuration functions */
    evc_default_conf(cmd);
#if defined(VTSS_SW_OPTION_MSTP)
    evc_mstp_conf(uni, nni);
#endif /* VTSS_SW_OPTION_MSTP */
#if defined(VTSS_SW_OPTION_LLDP)
    evc_lldp_conf(uni, nni);
#endif /* VTSS_SW_OPTION_LLDP */
    evc_vlan_conf(uni, nni);
#if defined(VTSS_SW_OPTION_QOS)
    evc_qos_conf(uni, nni);
#endif /* VTSS_SW_OPTION_QOS */
#if defined(VTSS_SW_OPTION_ACL)
    evc_acl_conf(uni, l2cp_id, policy, cmd);
#endif /* VTSS_SW_OPTION_ACL */
    evc_port_conf(uni, nni, l2cp_id, cmd);
    evc_conf(evc_id, nni, s_vid);
    evc_ece_conf(evc_id, uni, ce_vid, policer_low, policer_high, policy, cmd);
    evc_policer_conf(policer_low, policer_high);
#if defined(VTSS_SW_OPTION_MEP)
    evc_oam_conf(evc_id, uni, cmd);
#endif /* VTSS_SW_OPTION_MEP */
}
#endif /* VTSS_ARCH_SERVAL */
