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

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "icli_porting_util.h"
#include "acl_api.h"
#include "acl_icfg.h"
#include "mgmt_api.h"   //mgmt_acl_type_txt()
#include "misc_api.h"   //uport2iport(), iport2uport(), misc_strncpyz()
#include "msg_api.h"    //msg_switch_exists()
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define ACL_ICFG_BUF_SIZE    80

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
static void ACL_ICFG_ace_flag(acl_entry_conf_t *conf_p, char *ace_str_p, acl_flag_t flag, BOOL *first_flag_p, char *flag_str_p)
{
    char str_buf[ACL_ICFG_BUF_SIZE];

    misc_strncpyz(str_buf, mgmt_acl_flag_txt(conf_p, flag, TRUE), ACL_ICFG_BUF_SIZE);
    if (strcmp(str_buf, "any")) {
        if (*first_flag_p) {
            *first_flag_p = FALSE;
            sprintf(ace_str_p + strlen(ace_str_p), " %s", flag_str_p);
        }

        switch (flag) {
        case ACE_FLAG_ARP_REQ:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_REQUEST_TEXT, str_buf);
            break;
        case ACE_FLAG_ARP_SMAC:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_SMAC_TEXT, str_buf);
            break;
        case ACE_FLAG_ARP_DMAC:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_TMAC_TEXT, str_buf);
            break;
        case ACE_FLAG_ARP_LEN:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_LEN_TEXT, str_buf);
            break;
        case ACE_FLAG_ARP_IP:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_IP_TEXT, str_buf);
            break;
        case ACE_FLAG_ARP_ETHER:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_ARP_FLAG_ETHER_TEXT, str_buf);
            break;
        case ACE_FLAG_IP_TTL:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_IP_FLAG_TTL_TEXT, str_buf);
            break;
        case ACE_FLAG_IP_OPTIONS:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_IP_FLAG_OPT_TEXT, str_buf);
            break;
        case ACE_FLAG_IP_FRAGMENT:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_IP_FLAG_FRAG_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_FIN:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_FIN_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_SYN:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_SYN_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_RST:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_RST_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_PSH:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_PSH_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_ACK:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_ACK_TEXT, str_buf);
            break;
        case ACE_FLAG_TCP_URG:
            sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_URG_TEXT, str_buf);
            break;
        default:
            break;
        }
    }
}

static void ACL_ICFG_ace_ipv6_tcp_flag(acl_entry_conf_t *conf_p, char *ace_str_p, acl_flag_t flag, BOOL *first_flag_p)
{
    if (conf_p->frame.ipv6.tcp_fin != VTSS_ACE_BIT_ANY ||
        conf_p->frame.ipv6.tcp_syn != VTSS_ACE_BIT_ANY ||
        conf_p->frame.ipv6.tcp_rst != VTSS_ACE_BIT_ANY ||
        conf_p->frame.ipv6.tcp_psh != VTSS_ACE_BIT_ANY ||
        conf_p->frame.ipv6.tcp_ack != VTSS_ACE_BIT_ANY ||
        conf_p->frame.ipv6.tcp_urg != VTSS_ACE_BIT_ANY) {
        if (*first_flag_p) {
            *first_flag_p = FALSE;
            sprintf(ace_str_p + strlen(ace_str_p), " %s" , ACL_TCP_FLAG_TEXT);
        }

        switch (flag) {
        case ACE_FLAG_TCP_FIN:
            if (conf_p->frame.ipv6.tcp_fin != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_FIN_TEXT, conf_p->frame.ipv6.tcp_fin == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        case ACE_FLAG_TCP_SYN:
            if (conf_p->frame.ipv6.tcp_syn != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_SYN_TEXT, conf_p->frame.ipv6.tcp_syn == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        case ACE_FLAG_TCP_RST:
            if (conf_p->frame.ipv6.tcp_rst != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_RST_TEXT, conf_p->frame.ipv6.tcp_rst == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        case ACE_FLAG_TCP_PSH:
            if (conf_p->frame.ipv6.tcp_psh != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_PSH_TEXT, conf_p->frame.ipv6.tcp_psh == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        case ACE_FLAG_TCP_ACK:
            if (conf_p->frame.ipv6.tcp_ack != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_ACK_TEXT, conf_p->frame.ipv6.tcp_ack == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        case ACE_FLAG_TCP_URG:
            if (conf_p->frame.ipv6.tcp_urg != VTSS_ACE_BIT_ANY) {
                sprintf(ace_str_p + strlen(ace_str_p), " %s %s", ACL_TCP_FLAG_URG_TEXT, conf_p->frame.ipv6.tcp_urg == VTSS_ACE_BIT_0 ? "0" : "1");
            }
            break;
        default:
            break;
        }
    }
}

/* ICFG callback functions */
static vtss_rc ACL_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    vtss_rc                 rc = VTSS_OK;
    vtss_acl_policer_no_t   policer_idx;
    acl_policer_conf_t      policer_conf, def_policer_conf;
    int                     policer_conf_changed = 0, policer_conf_all_the_same = TRUE;
    char                    ace_str[ICLI_STR_MAX_LEN], str_buf[ACL_ICFG_BUF_SIZE];
    vtss_ace_id_t           ace_id[ACE_ID_END + 1];
    int                     ace_id_idx = 0, ace_cnt = 0;
    acl_entry_conf_t        ace_conf, default_ace_conf;
    BOOL                    first_flag = TRUE;
#if defined(ACL_IPV6_SUPPORTED)
    u32                     sip_v6_mask;
#endif /* ACL_IPV6_SUPPORTED */
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t          port_idx;
    u32                     port_list_cnt, port_filter_list_cnt;
#endif /* VTSS_FEATURE_ACL_V2 */

    //check if each sub-configuration is the same
    if ((rc = acl_mgmt_policer_conf_get(ACL_POLICER_NO_START, &def_policer_conf)) != VTSS_OK) {
        return rc;
    }
    for (policer_idx = ACL_POLICER_NO_START + 1; policer_idx < ACL_POLICER_NO_END; policer_idx++) {
        if ((rc = acl_mgmt_policer_conf_get(policer_idx, &policer_conf)) != VTSS_OK) {
            return rc;
        }
        if (acl_mgmt_policer_conf_changed(&def_policer_conf, &policer_conf)) {
            policer_conf_all_the_same = FALSE;
            break;  // found one is different
        }
    }

    str_buf[0] = '\0';
    for (policer_idx = ACL_POLICER_NO_START; policer_idx < ACL_POLICER_NO_END; policer_idx++) {
        if ((rc = acl_mgmt_policer_conf_get(policer_idx, &policer_conf)) != VTSS_OK) {
            return rc;
        }

        acl_mgmt_policer_conf_get_default(&def_policer_conf);
        policer_conf_changed = acl_mgmt_policer_conf_changed(&def_policer_conf, &policer_conf);

        if (!policer_conf_all_the_same) {
            sprintf(str_buf, " %u", ipolicer2upolicer(policer_idx));
        }

        //rate-limiter
        if (req->all_defaults || policer_conf_changed) {
            rc = vtss_icfg_printf(result, "%s %s%s %s %u\n",
                                  ACL_ACCESS_LIST_TEXT,
                                  ACL_RATE_LIMITER_TEXT,
                                  str_buf,
#if defined(ACL_BIT_RATE_MAX)
#if defined(VTSS_ARCH_SERVAL) /* 100pps */
                                  policer_conf.bit_rate_enable == TRUE ? "100kbps" : policer_conf.packet_rate < 100 ? "pps" : "100pps",
                                  policer_conf.bit_rate_enable == TRUE ? policer_conf.bit_rate / 100 : policer_conf.packet_rate < 100 ? policer_conf.packet_rate : policer_conf.packet_rate / 100);
#else /* 100kbps, pps */
                                  policer_conf.bit_rate_enable == TRUE ? "100kbps" : "pps",
                                  policer_conf.bit_rate_enable == TRUE ? policer_conf.bit_rate / 100 : policer_conf.packet_rate);
#endif /* VTSS_ARCH_SERVAL */
#elif defined(VTSS_ARCH_LUTON28)
                                  policer_conf.packet_rate / 1000 ? "kpps" : "pps",
                                  policer_conf.packet_rate / 1000 ? policer_conf.packet_rate / 1000 : policer_conf.packet_rate);
#else
                                  "pps",
                                  policer_conf.packet_rate);
#endif /* VTSS_FEATURE_ACL_V2 */
        }

        // do one time while all configuration is the same
        if (policer_conf_all_the_same) {
            break;
        }
    }

    /* We need to show the ACEs from the bottom to top.
       That the sytem won't occur an error of "cannot find the next ACE ID" when running the "startup-config". */
    ace_id[ace_id_idx] = ACE_ID_NONE;
    while (acl_mgmt_ace_get(ACL_USER_STATIC, VTSS_ISID_GLOBAL, ace_id[ace_id_idx], &ace_conf, NULL, TRUE) == VTSS_OK) {
        ace_id[ace_id_idx] = ace_id[ace_id_idx + 1] = ace_conf.id;
        ace_id_idx++;
        ace_cnt++;
    }
    if (ace_cnt == 0) {
        return rc;
    }

    for (ace_id_idx = ace_cnt - 1; ace_id_idx >= 0; ace_id_idx--) {
        if (acl_mgmt_ace_get(ACL_USER_STATIC, VTSS_ISID_GLOBAL, ace_id[ace_id_idx], &ace_conf, NULL, FALSE) != VTSS_OK) {
            break;
        }

        sprintf(ace_str, "%s %d", ACL_ACE_TEXT, ace_id[ace_id_idx]);

        //next_ace_id
        if (ace_id_idx != ace_cnt - 1) {
            sprintf(ace_str + strlen(ace_str), " next %u", ace_id[ace_id_idx + 1]);
        }

        //ingress
#if defined(VTSS_FEATURE_ACL_V2)
        port_list_cnt = 0;
        port_filter_list_cnt = 0;
        for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
            if (ace_conf.port_list[port_idx]) {
                port_list_cnt++;
            }
            if (ace_conf.action.port_list[port_idx]) {
                port_filter_list_cnt++;
            }
        }

#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
        if (ace_conf.isid != VTSS_ISID_GLOBAL && port_list_cnt != VTSS_PORT_NO_END) {
            //specific switch ID/port No.
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_INGRESS_TEXT,
                    ACL_INTERFACE_TEXT,
                    icli_port_list_info_txt(ace_conf.isid, ace_conf.port_list, str_buf, FALSE));
        } else if (ace_conf.isid == VTSS_ISID_GLOBAL && port_list_cnt != VTSS_PORT_NO_END) {
            //all switchport
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_INGRESS_TEXT,
                    ACL_SWITCHPORT_TEXT,
                    mgmt_iport_list2txt(ace_conf.port_list, str_buf));
        } else if (ace_conf.isid != VTSS_ISID_GLOBAL && port_list_cnt == VTSS_PORT_NO_END) {
            //specific switch ID
            sprintf(ace_str + strlen(ace_str), " %s %s %u",
                    ACL_INGRESS_TEXT,
                    ACL_SWITCH_TEXT,
                    topo_isid2usid(ace_conf.isid));
        }
#else //VTSS_SWITCH_STACKABLE
        if (port_list_cnt != VTSS_PORT_NO_END) { //specific switch port No.
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_INGRESS_TEXT,
                    ACL_INTERFACE_TEXT,
                    icli_port_list_info_txt(VTSS_ISID_START, ace_conf.port_list, str_buf, FALSE));
        }
#endif /* VTSS_SWITCH_STACKABLE */

#else //VTSS_FEATURE_ACL_V2

#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
        if (ace_conf.isid != VTSS_ISID_GLOBAL && ace_conf.port_no != VTSS_PORT_NO_ANY) {
            //specific switch ID/port No.
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_INGRESS_TEXT,
                    ACL_INTERFACE_TEXT,
                    icli_port_info_txt(topo_isid2usid(ace_conf.isid), iport2uport(ace_conf.port_no), str_buf));
        } else if (ace_conf.isid == VTSS_ISID_GLOBAL && ace_conf.port_no != VTSS_PORT_NO_ANY) {
            //all switchport
            sprintf(ace_str + strlen(ace_str), " %s %s %u",
                    ACL_INGRESS_TEXT,
                    ACL_SWITCHPORT_TEXT,
                    iport2uport(ace_conf.port_no));
        } else if (ace_conf.isid != VTSS_ISID_GLOBAL && ace_conf.port_no == VTSS_PORT_NO_ANY) {
            //specific switch ID
            sprintf(ace_str + strlen(ace_str), " %s %s %u",
                    ACL_INGRESS_TEXT,
                    ACL_SWITCH_TEXT,
                    topo_isid2usid(ace_conf.isid));
        }
#else //VTSS_SWITCH_STACKABLE
        if (ace_conf.port_no != VTSS_PORT_NO_ANY) { //specific switch port No.
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_INGRESS_TEXT,
                    ACL_INTERFACE_TEXT,
                    icli_port_info_txt(VTSS_USID_START, iport2uport(ace_conf.port_no), str_buf));
        }
#endif /* VTSS_SWITCH_STACKABLE */
#endif /* VTSS_FEATURE_ACL_V2 */

        //policy
        if (ace_conf.policy.mask) {
            sprintf(ace_str + strlen(ace_str), " %s %u",
                    ACL_POLICY_TEXT,
                    ace_conf.policy.value);

            //policy-bitmask
            if (ace_conf.policy.mask != ACL_POLICIES_BITMASK) {
                sprintf(ace_str + strlen(ace_str), " %s 0x%X",
                        ACL_POLICY_BITMASK_TEXT,
                        ace_conf.policy.mask);
            }
        }

#if defined(VTSS_FEATURE_ACL_V2)
        //tag
        if (ace_conf.tagged != VTSS_ACE_BIT_ANY) {
            sprintf(ace_str + strlen(ace_str), " %s %s",
                    ACL_TAG_TEXT,
                    ace_conf.tagged == VTSS_ACE_BIT_0 ? "untagged" : "tagged");
        }
#endif /*VTSS_FEATURE_ACL_V2 */

        //vid
        if (ace_conf.vid.mask) {
            sprintf(ace_str + strlen(ace_str), " %s %u",
                    ACL_VID_TEXT,
                    ace_conf.vid.value);
        }

        //tag_priority
        if (ace_conf.usr_prio.mask) {
            u8 usr_prio_min, usr_prio_max;
            if (ace_conf.usr_prio.mask == 0x7) {
                sprintf(ace_str + strlen(ace_str), " %s %u",
                        ACL_TAG_PRIORITY_TEXT,
                        ace_conf.usr_prio.value);
            } else {
                if (ace_conf.usr_prio.mask == 0x6) {
                    if (ace_conf.usr_prio.value == 1) { // 0-1
                        usr_prio_min = 0;
                        usr_prio_max = 1;
                    } else if (ace_conf.usr_prio.value == 3) { // 2-3
                        usr_prio_min = 2;
                        usr_prio_max = 3;
                    } else if (ace_conf.usr_prio.value == 5) { // 4-5
                        usr_prio_min = 4;
                        usr_prio_max = 5;
                    } else { // 6-7
                        usr_prio_min = 6;
                        usr_prio_max = 7;
                    }
                } else {
                    if (ace_conf.usr_prio.value == 3) { // 0-3
                        usr_prio_min = 0;
                        usr_prio_max = 3;
                    } else { // 4-7
                        usr_prio_min = 4;
                        usr_prio_max = 7;
                    }
                }

                sprintf(ace_str + strlen(ace_str), " %s %u-%u",
                        ACL_TAG_PRIORITY_TEXT,
                        usr_prio_min,
                        usr_prio_max);
            }
        }

        //dmac_type
        if (mgmt_acl_dmac_txt(&ace_conf, str_buf, TRUE) &&
            strcmp(str_buf, "any")) {
            sprintf(ace_str + strlen(ace_str), " %s %s",
                    ACL_DMAC_TYPE_TEXT,
                    str_buf);
        }

        //frametype
        if (acl_mgmt_ace_init(ace_conf.type, &default_ace_conf) != VTSS_OK) {
            continue;
        }
        switch (ace_conf.type) {
        case VTSS_ACE_TYPE_ETYPE:
            sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_ETYPE_TEXT);
            if (memcmp(&ace_conf.frame, &default_ace_conf.frame, sizeof(ace_conf.frame.etype)) ||
                memcmp(&ace_conf.flags, &default_ace_conf.flags, sizeof(ace_conf.flags))) {
                //etype
                if (mgmt_acl_uchar2_txt(&ace_conf.frame.etype.etype, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_ETYPE_VALUE_TEXT, str_buf);
                }

                //smac
                if (mgmt_acl_uchar6_txt(&ace_conf.frame.etype.smac, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_SMAC_TEXT, str_buf);
                }

                //dmac
                if (mgmt_acl_uchar6_txt(&ace_conf.frame.etype.dmac, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_DMAC_TEXT, str_buf);
                }
            }
            break;
        case VTSS_ACE_TYPE_ARP:
            sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_ARP_TEXT);
            if (memcmp(&ace_conf.frame, &default_ace_conf.frame, sizeof(ace_conf.frame.arp)) ||
                memcmp(&ace_conf.flags, &default_ace_conf.flags, sizeof(ace_conf.flags))) {
                //sip
                if (mgmt_acl_ipv4_txt(&ace_conf.frame.arp.sip, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_SIP_TEXT,
                            str_buf);
                }

                //dip
                if (mgmt_acl_ipv4_txt(&ace_conf.frame.arp.dip, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_DIP_TEXT,
                            str_buf);
                }

                //smac
                if (mgmt_acl_uchar6_txt(&ace_conf.frame.arp.smac, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_SMAC_TEXT,
                            str_buf);
                }

                //arp_opcode
                if (mgmt_acl_opcode_txt(&ace_conf, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_ARP_OPCODE_TEXT,
                            str_buf);
                }

                //arp_flag
                first_flag = TRUE;
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_REQ, &first_flag, "arp-flag");
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_SMAC, &first_flag, "arp-flag");
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_DMAC, &first_flag, "arp-flag");
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_LEN, &first_flag, "arp-flag");
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_IP, &first_flag, "arp-flag");
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_ARP_ETHER, &first_flag, "arp-flag");
            }
            break;
        case VTSS_ACE_TYPE_IPV4:
            if (memcmp(&ace_conf.frame, &default_ace_conf.frame, sizeof(ace_conf.frame.ipv4)) == 0 &&
                memcmp(&ace_conf.flags, &default_ace_conf.flags, sizeof(ace_conf.flags)) == 0) {
                sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_IP_TEXT);
            } else {
                //ipv4_protocol
                switch (ace_conf.frame.ipv4.proto.value) {
                case 1: //icmp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_ICMP_TEXT);
                    break;
                case 6: //tcp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_TCP_TEXT);
                    break;
                case 17: //udp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_UDP_TEXT);
                    break;
                default:
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_FRAMETYPE_TEXT,
                            ACL_IP_TEXT);
                    if (ace_conf.frame.ipv4.proto.mask) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_IP_PROTOCOL_TEXT,
                                ace_conf.frame.ipv4.proto.value);
                    }
                    break;
                }

                //sip
                if (mgmt_acl_ipv4_txt(&ace_conf.frame.ipv4.sip, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_SIP_TEXT,
                            str_buf);
                }

                //dip
                if (mgmt_acl_ipv4_txt(&ace_conf.frame.ipv4.dip, str_buf, TRUE) &&
                    strcmp(str_buf, "any")) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_DIP_TEXT,
                            str_buf);
                }

                if (ace_conf.frame.ipv4.proto.value == 1) {
                    //icmpv4_type
                    if (mgmt_acl_ulong_txt(ace_conf.frame.ipv4.data.value[0], ace_conf.frame.ipv4.data.mask[0], str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %s",
                                ACL_ICMP_TYPE_TEXT,
                                str_buf);
                    }

                    //icmpv4_code
                    if (mgmt_acl_ulong_txt(ace_conf.frame.ipv4.data.value[1], ace_conf.frame.ipv4.data.mask[1], str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %s",
                                ACL_ICMP_CODE_TEXT,
                                str_buf);
                    }
                } else if (ace_conf.frame.ipv4.proto.value == 6 ||
                           ace_conf.frame.ipv4.proto.value == 17) {
                    //sportv4/dportv4
                    if (mgmt_acl_port_txt(&ace_conf.frame.ipv4.sport, str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_SPORT_TEXT,
                                ace_conf.frame.ipv4.sport.low);
                        if (ace_conf.frame.ipv4.sport.low != ace_conf.frame.ipv4.sport.high) {
                            sprintf(ace_str + strlen(ace_str), " to %u",
                                    ace_conf.frame.ipv4.sport.high);
                        }
                    }

                    if (mgmt_acl_port_txt(&ace_conf.frame.ipv4.dport, str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_DPORT_TEXT,
                                ace_conf.frame.ipv4.dport.low);
                        if (ace_conf.frame.ipv4.dport.low != ace_conf.frame.ipv4.dport.high) {
                            sprintf(ace_str + strlen(ace_str), " to %u",
                                    ace_conf.frame.ipv4.dport.high);
                        }
                    }
                }

                //ip_flag
                first_flag = TRUE;
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_IP_TTL, &first_flag, ACL_IP_FLAG_TEXT);
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_IP_OPTIONS, &first_flag, ACL_IP_FLAG_TEXT);
                ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_IP_FRAGMENT, &first_flag, ACL_IP_FLAG_TEXT);

                //tcpv4_flag
                if (ace_conf.frame.ipv4.proto.value == 6) {
                    first_flag = TRUE;
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_FIN, &first_flag, ACL_TCP_FLAG_TEXT);
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_SYN, &first_flag, ACL_TCP_FLAG_TEXT);
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_RST, &first_flag, ACL_TCP_FLAG_TEXT);
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_PSH, &first_flag, ACL_TCP_FLAG_TEXT);
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_ACK, &first_flag, ACL_TCP_FLAG_TEXT);
                    ACL_ICFG_ace_flag(&ace_conf, ace_str, ACE_FLAG_TCP_URG, &first_flag, ACL_TCP_FLAG_TEXT);
                }
            }
            break;
#if defined(ACL_IPV6_SUPPORTED)
        case VTSS_ACE_TYPE_IPV6:
            if (memcmp(&ace_conf.frame, &default_ace_conf.frame, sizeof(ace_conf.frame.ipv6)) == 0 &&
                memcmp(&ace_conf.flags, &default_ace_conf.flags, sizeof(ace_conf.flags)) == 0) {
                sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_IPV6_TEXT);
            } else {
                switch (ace_conf.frame.ipv6.proto.value) {
                case 58: //icmp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_IPV6_ICMP_TEXT);
                    break;
                case 6: //tcp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_IPV6_TCP_TEXT);
                    break;
                case 17: //udp
                    sprintf(ace_str + strlen(ace_str), " %s %s", ACL_FRAMETYPE_TEXT, ACL_IPV6_UDP_TEXT);
                    break;
                default:
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_FRAMETYPE_TEXT,
                            ACL_IPV6_TEXT);
                    if (ace_conf.frame.ipv6.proto.mask) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_NEXT_HEADER_TEXT,
                                ace_conf.frame.ipv6.proto.value);
                    }
                    break;
                }

                //sipv6_bitmask
                sip_v6_mask = (ace_conf.frame.ipv6.sip.mask[12] << 24) +
                              (ace_conf.frame.ipv6.sip.mask[13] << 16) +
                              (ace_conf.frame.ipv6.sip.mask[14] << 8) +
                              ace_conf.frame.ipv6.sip.mask[15];
                if (sip_v6_mask) {
                    //sipv6
                    if (mgmt_acl_ipv6_txt(&ace_conf.frame.ipv6.sip, str_buf, TRUE)) {
                        if (strcmp(str_buf, "any")) {
                            sprintf(ace_str + strlen(ace_str), " %s %s",
                                    ACL_SIP_TEXT,
                                    str_buf);

                            if (sip_v6_mask != 0xFFFFFFFF) {
                                sprintf(ace_str + strlen(ace_str), " %s 0x%X",
                                        ACL_SIP_BITMASK_TEXT,
                                        sip_v6_mask);
                            }
                        }
                    }
                }

                if (ace_conf.frame.ipv6.proto.value == 58) {
                    //icmpv6_type
                    if (mgmt_acl_ulong_txt(ace_conf.frame.ipv6.data.value[0], ace_conf.frame.ipv6.data.mask[0], str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %s",
                                ACL_ICMP_TYPE_TEXT,
                                str_buf);
                    }

                    //icmpv6_code
                    if (mgmt_acl_ulong_txt(ace_conf.frame.ipv6.data.value[1], ace_conf.frame.ipv6.data.mask[1], str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %s",
                                ACL_ICMP_CODE_TEXT,
                                str_buf);
                    }
                } else if (ace_conf.frame.ipv6.proto.value == 6 ||
                           ace_conf.frame.ipv6.proto.value == 17) {
                    //sportv6/dportv6
                    if (mgmt_acl_port_txt(&ace_conf.frame.ipv6.sport, str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_SPORT_TEXT,
                                ace_conf.frame.ipv6.sport.low);
                        if (ace_conf.frame.ipv6.sport.low != ace_conf.frame.ipv6.sport.high) {
                            sprintf(ace_str + strlen(ace_str), " to %u",
                                    ace_conf.frame.ipv6.sport.high);
                        }
                    }

                    if (mgmt_acl_port_txt(&ace_conf.frame.ipv6.dport, str_buf, TRUE) &&
                        strcmp(str_buf, "any")) {
                        sprintf(ace_str + strlen(ace_str), " %s %u",
                                ACL_DPORT_TEXT,
                                ace_conf.frame.ipv6.dport.low);
                        if (ace_conf.frame.ipv6.dport.low != ace_conf.frame.ipv6.dport.high) {
                            sprintf(ace_str + strlen(ace_str), " to %u",
                                    ace_conf.frame.ipv6.dport.high);
                        }
                    }
                }

                //hop_limit
                if (ace_conf.frame.ipv6.ttl != VTSS_ACE_BIT_ANY) {
                    sprintf(ace_str + strlen(ace_str), " %s %s",
                            ACL_HOP_LIMIT_TEXT,
                            ace_conf.frame.ipv6.ttl == VTSS_ACE_BIT_0 ? "0" : "1");
                }

                //tcpv6_flag
                if (ace_conf.frame.ipv6.proto.value == 6) {
                    first_flag = TRUE;
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_FIN, &first_flag);
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_SYN, &first_flag);
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_RST, &first_flag);
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_PSH, &first_flag);
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_ACK, &first_flag);
                    ACL_ICFG_ace_ipv6_tcp_flag(&ace_conf, ace_str, ACE_FLAG_TCP_URG, &first_flag);
                }
            }
            break;
#endif /* ACL_IPV6_SUPPORTED */
        default:
            break;
        }

        //action
#if defined(VTSS_FEATURE_ACL_V2)
        if (ace_conf.action.port_action != VTSS_ACL_PORT_ACTION_NONE) {
            if (ace_conf.action.port_action == VTSS_ACL_PORT_ACTION_FILTER && port_filter_list_cnt != 0) {
#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
                if (ace_conf.isid == VTSS_ISID_GLOBAL) {
                    sprintf(ace_str + strlen(ace_str), " %s %s %s %s",
                            ACL_ACTION_TEXT
                            ACL_FILTER_TEXT,
                            ACL_SWITCHPORT_TEXT,
                            mgmt_iport_list2txt(ace_conf.action.port_list, str_buf));
                } else {
                    sprintf(ace_str + strlen(ace_str), " %s %s %s %s",
                            ACL_ACTION_TEXT,
                            ACL_FILTER_TEXT,
                            ACL_INTERFACE_TEXT,
                            icli_port_list_info_txt(ace_conf.isid, ace_conf.action.port_list, str_buf, FALSE));
                }
#else //VTSS_SWITCH_STACKABLE
                sprintf(ace_str + strlen(ace_str), " %s %s %s %s",
                        ACL_ACTION_TEXT,
                        ACL_FILTER_TEXT,
                        ACL_INTERFACE_TEXT,
                        icli_port_list_info_txt(VTSS_ISID_START, ace_conf.action.port_list, str_buf, FALSE));
#endif /* VTSS_SWITCH_STACKABLE */
            } else {
                sprintf(ace_str + strlen(ace_str), " %s %s", ACL_ACTION_TEXT, ACL_DENY_TEXT);
            }
        }
#else //VTSS_FEATURE_ACL_V2
        if (!ace_conf.action.permit) {
            sprintf(ace_str + strlen(ace_str), " %s %s", ACL_ACTION_TEXT, ACL_DENY_TEXT);
        }
#endif /* VTSS_FEATURE_ACL_V2 */

        //rate_limter_id
        if (ace_conf.action.policer != ACL_POLICER_NONE) {
            sprintf(ace_str + strlen(ace_str), " %s %u",
                    ACL_RATE_LIMITER_TEXT,
                    ipolicer2upolicer(ace_conf.action.policer));
        }

#if defined(VTSS_ARCH_CARACAL) && defined(VTSS_SW_OPTION_EVC)
        //evc_policer
        if (ace_conf.action.evc_police) {
            sprintf(ace_str + strlen(ace_str), " %s %u",
                    ACL_EVC_POLICER_TEXT,
                    ievcpolicer2uevcpolicer(ace_conf.action.evc_policer_id));
        }
#endif /* VTSS_ARCH_CARACAL && VTSS_SW_OPTION_EVC */

#if defined(VTSS_FEATURE_ACL_V2)
        //mirror
        if (ace_conf.action.mirror) {
            sprintf(ace_str + strlen(ace_str), " %s", ACL_MIRROR_TEXT);
        }

        //redirect
        if (ace_conf.action.port_action == VTSS_ACL_PORT_ACTION_REDIR) {
#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
            if (ace_conf.isid == VTSS_ISID_GLOBAL) {
                sprintf(ace_str + strlen(ace_str), " %s %s %u",
                        ACL_REDIRECT_TEXT
                        ACL_SWITCHPORT_TEXT,
                        mgmt_iport_list2txt(ace_conf.action.port_list, str_buf));

            } else {
                sprintf(ace_str + strlen(ace_str), " %s %s %s",
                        ACL_REDIRECT_TEXT,
                        ACL_INTERFACE_TEXT,
                        icli_port_list_info_txt(ace_conf.isid, ace_conf.action.port_list, str_buf, FALSE));
            }
#else //VTSS_SWITCH_STACKABLE
            sprintf(ace_str + strlen(ace_str), " %s %s %s",
                    ACL_REDIRECT_TEXT,
                    ACL_INTERFACE_TEXT,
                    icli_port_list_info_txt(VTSS_ISID_START, ace_conf.action.port_list, str_buf, FALSE));
#endif /* VTSS_SWITCH_STACKABLE */
        }

#else //VTSS_FEATURE_ACL_V2

        //redirect
        if (ace_conf.action.port_no != VTSS_PORT_NO_NONE) {
            sprintf(ace_str + strlen(ace_str), " %s",
#if defined(VTSS_ARCH_LUTON28)
                    ACL_PORT_COPY_TEXT
#else
                    ACL_REDIRECT_TEXT
#endif /* VTSS_ARCH_LUTON28 */
                   );

#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
            if (ace_conf.isid == VTSS_ISID_GLOBAL) {
                sprintf(ace_str + strlen(ace_str), " %s %u",
                        ACL_SWITCHPORT_TEXT,
                        iport2uport(ace_conf.action.port_no));

            } else {
                sprintf(ace_str + strlen(ace_str), " %s %s",
                        ACL_INTERFACE_TEXT,
                        icli_port_info_txt(topo_isid2usid(ace_conf.isid), iport2uport(ace_conf.action.port_no), str_buf));
            }
#else //VTSS_SWITCH_STACKABLE
            sprintf(ace_str + strlen(ace_str), " %s %s",
                    ACL_INTERFACE_TEXT,
                    icli_port_info_txt(VTSS_USID_START, iport2uport(ace_conf.action.port_no), str_buf));
#endif /* VTSS_SWITCH_STACKABLE */
        }
#endif /* VTSS_FEATURE_ACL_V2 */

        //logging
        if (ace_conf.action.logging) {
            sprintf(ace_str + strlen(ace_str), " %s", ACL_LOGGING_TEXT);
        }

        //shutdown
        if (ace_conf.action.shutdown) {
            sprintf(ace_str + strlen(ace_str), " %s", ACL_SHUTDOWN_TEXT);
        }

        if (vtss_icfg_printf(result, "%s\n", ace_str) != VTSS_OK) {
            break;
        }
    }

    return rc;
}

static vtss_rc ACL_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                  vtss_icfg_query_result_t *result)
{
    vtss_rc         rc = VTSS_OK;
    acl_port_conf_t conf, def_conf;
    vtss_isid_t     isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t  iport = uport2iport(req->instance_id.port.begin_uport);
    int             conf_changed = 0;
    char            str_buf[ACL_ICFG_BUF_SIZE];

    if ((rc = acl_mgmt_port_conf_get(isid, iport, &conf)) != VTSS_OK) {
        return rc;
    }

    acl_mgmt_port_conf_get_default(&def_conf);
    conf_changed = acl_mgmt_port_conf_changed(&def_conf, &conf);

#if defined(VTSS_FEATURE_ACL_V2)
    if (req->all_defaults ||
        (conf_changed && conf.action.port_action != def_conf.action.port_action)) {
        rc += vtss_icfg_printf(result, " %s %s %s\n",
                               ACL_ACCESS_LIST_TEXT,
                               ACL_ACTION_TEXT,
                               conf.action.port_action == VTSS_ACL_PORT_ACTION_NONE ? ACL_PERMIT_TEXT : ACL_DENY_TEXT);
    }
#else //VTSS_FEATURE_ACL_V2
    //action
    if (req->all_defaults ||
        (conf_changed && conf.action.permit != def_conf.action.permit)) {
        rc += vtss_icfg_printf(result, " %s %s %s\n",
                               ACL_ACCESS_LIST_TEXT,
                               ACL_ACTION_TEXT,
                               conf.action.permit ? ACL_PERMIT_TEXT : ACL_DENY_TEXT);
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    //rate-limiter
    if (req->all_defaults ||
        (conf_changed && conf.action.policer != def_conf.action.policer)) {
        sprintf(str_buf, "%u", ipolicer2upolicer(conf.action.policer));
        rc += vtss_icfg_printf(result, " %s%s %s %s\n",
                               conf.action.policer != VTSS_ACL_POLICY_NO_NONE ? "" : ACL_NO_FORM_TEXT,
                               ACL_ACCESS_LIST_TEXT,
                               ACL_RATE_LIMITER_TEXT,
                               conf.action.policer == VTSS_ACL_POLICY_NO_NONE ? "" : str_buf);
    }

    //policy
    if (req->all_defaults ||
        (conf_changed && conf.policy_no != def_conf.policy_no)) {
        rc += vtss_icfg_printf(result, " %s %s %u\n",
                               ACL_ACCESS_LIST_TEXT,
                               ACL_POLICY_TEXT,
                               conf.policy_no);
    }

    //redirect
#if defined(VTSS_FEATURE_ACL_V2)
    if (req->all_defaults ||
        (conf_changed && conf.action.port_action != def_conf.action.port_action)) {
        rc += vtss_icfg_printf(result, " %s%s %s%s%s%s%s\n",
                               conf.action.port_action != VTSS_ACL_PORT_ACTION_REDIR ? ACL_NO_FORM_TEXT : "",
                               ACL_ACCESS_LIST_TEXT,
                               ACL_REDIRECT_TEXT,
                               conf.action.port_action != VTSS_ACL_PORT_ACTION_REDIR ? "" : " ",
                               conf.action.port_action != VTSS_ACL_PORT_ACTION_REDIR ? "" : ACL_INTERFACE_TEXT,
                               conf.action.port_action != VTSS_ACL_PORT_ACTION_REDIR ? "" : " ",
                               conf.action.port_action != VTSS_ACL_PORT_ACTION_REDIR ? "" : icli_port_list_info_txt(isid, conf.action.port_list, str_buf, FALSE));
    }
#else //VTSS_FEATURE_ACL_V2
    if (req->all_defaults ||
        (conf_changed && conf.action.port_no != def_conf.action.port_no)) {
        rc += vtss_icfg_printf(result, " %s%s %s%s%s%s%s\n",
                               conf.action.port_no != VTSS_PORT_NO_NONE ? "" : ACL_NO_FORM_TEXT,
                               ACL_ACCESS_LIST_TEXT,
#if defined(VTSS_ARCH_LUTON28)
                               ACL_PORT_COPY_TEXT,
#else
                               ACL_REDIRECT_TEXT,
#endif /* VTSS_ARCH_LUTON28 */
                               conf.action.port_no == VTSS_PORT_NO_NONE ? "" : " ",
                               conf.action.port_no == VTSS_PORT_NO_NONE ? "" : ACL_INTERFACE_TEXT,
                               conf.action.port_no == VTSS_PORT_NO_NONE ? "" : " ",
                               conf.action.port_no == VTSS_PORT_NO_NONE ? "" : icli_port_info_txt(topo_isid2usid(isid), iport2uport(conf.action.port_no), str_buf));
    }
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_FEATURE_ACL_V2)
    //mirror
    if (req->all_defaults ||
        (conf_changed && conf.action.mirror != def_conf.action.mirror)) {
        rc += vtss_icfg_printf(result, " %s%s %s\n",
                               conf.action.mirror ? "" : ACL_NO_FORM_TEXT,
                               ACL_ACCESS_LIST_TEXT,
                               ACL_MIRROR_TEXT);
    }
#endif /* VTSS_FEATURE_ACL_V2 */

    //logging
    if (req->all_defaults ||
        (conf_changed && conf.action.logging != def_conf.action.logging)) {
        rc += vtss_icfg_printf(result, " %s%s %s\n",
                               conf.action.logging ? "" : ACL_NO_FORM_TEXT,
                               ACL_ACCESS_LIST_TEXT,
                               ACL_LOGGING_TEXT);
    }

    //shutdown
    if (req->all_defaults ||
        (conf_changed && conf.action.shutdown != def_conf.action.shutdown)) {
        rc += vtss_icfg_printf(result, " %s%s %s\n",
                               conf.action.shutdown ? "" : ACL_NO_FORM_TEXT,
                               ACL_ACCESS_LIST_TEXT,
                               ACL_SHUTDOWN_TEXT);
    }
    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc acl_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_ACL_GLOBAL_CONF, "access-list", ACL_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    rc = vtss_icfg_query_register(VTSS_ICFG_ACL_PORT_CONF, "access-list", ACL_ICFG_port_conf);

    return rc;
}
