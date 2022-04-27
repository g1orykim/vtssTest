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
#include "evc_cli.h"
#include "cli.h"
#include "mgmt_api.h"
#include "cli_trace_def.h"
#include "evc_api.h"
#include "vlan_api.h"

/* Number of L2CP IDs */
#define L2CP_ID_COUNT 32

typedef struct {
    BOOL            valid;
    vtss_vcap_bit_t value;
} evc_cli_vcap_t;

typedef struct {
    BOOL            valid;
    vtss_vcap_bit_t tagged;
    vtss_vcap_bit_t s_tagged;
} evc_cli_tag_type_t;

typedef struct {
    BOOL           valid;
    vtss_vcap_u8_t value;
} evc_cli_vcap_u8_t;

typedef struct {
    BOOL            valid;
    vtss_vcap_u48_t value;
} evc_cli_vcap_u48_t;

typedef struct {
    BOOL             valid;
    vtss_vcap_ip_t   ipv4;
    vtss_vcap_u128_t ipv6;
} evc_cli_vcap_ip_t;

typedef struct {
    BOOL           valid;
    vtss_tagprio_t value;
} evc_cli_pcp_t;

typedef struct {
    BOOL       valid;
    vtss_vid_t value;
} evc_cli_vid_t;

typedef struct {
    BOOL           valid;
    vtss_vcap_vr_t value;
} evc_cli_range_t;

typedef struct {
    BOOL valid;
    BOOL value;
} evc_cli_bool_t;

typedef struct {
    BOOL                valid;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ece_pcp_mode_t value;
#else
    BOOL                value;
#endif /* VTSS_ARCH_SERVAL */
} evc_cli_pcp_mode_t;

#if defined(VTSS_ARCH_SERVAL)
typedef struct {
    BOOL                valid;
    vtss_ece_dei_mode_t value;
} evc_cli_dei_mode_t;
#endif /* VTSS_ARCH_SERVAL */

typedef struct {
    vtss_evc_id_t             evc_id;
    BOOL                      evc_id_valid;
    evc_cli_vid_t             evc_vid;
    evc_cli_vid_t             ivid;
    vtss_evc_policer_id_t     policer_id;
    BOOL                      policer_id_valid;
    vtss_packet_reg_type_t    fwd_type;
    BOOL                      l2cp_list[L2CP_ID_COUNT];
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_key_type_t      key_type;
#endif /* VTSS_ARCH_SERVAL */
    vtss_evc_policer_conf_t   pol_conf;
    BOOL                      cir_valid;
    BOOL                      cbs_valid;
    BOOL                      eir_valid;
    BOOL                      ebs_valid;
    vtss_ece_id_t             ece_id;
    BOOL                      ece_id_next_valid;
    vtss_ece_id_t             ece_id_next;
    BOOL                      uni_list_valid;
    BOOL                      nni_list_valid;
    evc_cli_vcap_u48_t        smac;
    evc_cli_vcap_u48_t        dmac;
    BOOL                      dmac_type_valid;
    vtss_vcap_bit_t           dmac_mc;
    vtss_vcap_bit_t           dmac_bc;
    evc_cli_tag_type_t        tag_type;
    evc_cli_range_t           vid;
    evc_cli_vcap_u8_t         pcp;
    evc_cli_vcap_t            dei;
    evc_cli_tag_type_t        in_type;
    evc_cli_range_t           in_vid;
    evc_cli_vcap_u8_t         in_pcp;
    evc_cli_vcap_t            in_dei;
    evc_cli_vcap_u8_t         proto;
    evc_cli_vcap_ip_t         sip;
    evc_cli_vcap_ip_t         dip;
    evc_cli_range_t           dscp;
    evc_cli_vcap_t            fragment;
    evc_cli_range_t           sport;
    evc_cli_range_t           dport;
    BOOL                      policy_valid;
    vtss_acl_policy_no_t      policy_no;
    BOOL                      prio_valid;
    BOOL                      prio_enable;
    vtss_prio_t               prio;
    BOOL                      dp_valid;
    BOOL                      dp_enable;
    vtss_dp_level_t           dp;
    BOOL                      ece_type_valid;
    vtss_ece_type_t           ece_type;
    BOOL                      dir_valid;
    vtss_ece_dir_t            dir;
#if defined(VTSS_ARCH_SERVAL)
    BOOL                      rule_valid;
    vtss_ece_rule_t           rule;
    BOOL                      tx_lookup_valid;
    vtss_ece_tx_lookup_t      tx_lookup;
    vtss_mce_tx_lookup_t      mce_tx_lookup;
    vtss_mce_oam_detect_t     oam_detect;
#endif /* VTSS_ARCH_SERVAL */
    BOOL                      pop_valid;
    vtss_ece_pop_tag_t        pop;
    evc_cli_bool_t            ot_mode;
    evc_cli_vid_t             ot_vid;
    evc_cli_pcp_mode_t        ot_pcp_mode;
    evc_cli_pcp_t             ot_pcp;
#if defined(VTSS_ARCH_SERVAL)
    evc_cli_dei_mode_t        ot_dei_mode;
    vtss_oam_voe_idx_t        voe_idx;
#endif /* VTSS_ARCH_SERVAL */
    evc_cli_bool_t            ot_dei;
    evc_cli_vid_t             it_vid;
    evc_cli_pcp_mode_t        it_pcp_mode;
    evc_cli_pcp_t             it_pcp;
#if defined(VTSS_ARCH_SERVAL)
    evc_cli_dei_mode_t        it_dei_mode;
#endif /* VTSS_ARCH_SERVAL */
    evc_cli_bool_t            it_dei;
    BOOL                      it_vid_mode_valid;
    BOOL                      it_type_valid;
#if defined(VTSS_ARCH_CARACAL)
    vtss_evc_inner_tag_type_t it_type;
    vtss_evc_vid_mode_t       it_vid_mode;
#else
    vtss_ece_inner_tag_type_t it_type;
#endif /* VTSS_ARCH_CARACAL */
    BOOL                      prio_list[VTSS_PRIOS];
#if defined(VTSS_ARCH_SERVAL)
    vtss_mce_id_t             mce_id;
    vtss_mce_id_t             mce_id_next;
    BOOL                      mce_id_next_valid;
    vtss_port_no_t            port_rx;
    vtss_port_no_t            port_tx;
    vtss_mce_pcp_mode_t       pcp_mode;
    vtss_mce_dei_mode_t       dei_mode;
    vtss_isdx_t               isdx;
    evc_cli_pcp_t             mel;
    evc_cli_vcap_t            inject;
    u32                       lookup;
#endif /* VTSS_ARCH_SERVAL */

    /* Keywords */
    BOOL                      aware;
    BOOL                      blind;
    BOOL                      coupled;
    BOOL                      coloured;
    BOOL                      inner;
    BOOL                      dmac_dip;
    BOOL                      clear;
    BOOL                      green;
    BOOL                      yellow;
    BOOL                      red;
    BOOL                      discard;
    BOOL                      bytes;
    BOOL                      frames;
} evc_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

static char uni_list_txt[32];

void evc_cli_init(void)
{
    /* register the size required for EVC req. structure */
    cli_req_size_register(sizeof(evc_cli_req_t));
    
    sprintf(uni_list_txt, "UNI port list (1-%u)", port_isid_port_count(VTSS_ISID_LOCAL));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* Command type, also used for sorting commands */
typedef enum {
    EVC_CMD_CONF,
    EVC_CMD_PORT_KEY,
    EVC_CMD_PORT_DEI,
    EVC_CMD_PORT_TAG,
    EVC_CMD_PORT_ADDR,
    EVC_CMD_PORT_L2CP,
    EVC_CMD_POLICER,
    EVC_CMD_ADD,
    EVC_CMD_DEL,
    EVC_CMD_LOOKUP,
    EVC_CMD_STATUS,
    EVC_CMD_EVC_STATISTICS,
    EVC_CMD_LEARNING,
    EVC_CMD_ECE_ADD,
    EVC_CMD_ECE_DEL,
    EVC_CMD_ECE_LOOKUP,
    EVC_CMD_ECE_STATUS,
    EVC_CMD_ECE_STATISTICS,
    EVC_CMD_DEBUG_EVC_PORT_CHECK = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_EVC_MEP = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_EVC_TEST = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_MCE_ADD = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_MCE_DEL = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_MCE_GET = CLI_CMD_SORT_KEY_DEFAULT,
    EVC_CMD_DEBUG_MCE_KEY = CLI_CMD_SORT_KEY_DEFAULT,
} evc_cmd_t;

#define EVC_PORT_FLAGS_KEY  0x0001
#define EVC_PORT_FLAGS_DEI  0x0002
#define EVC_PORT_FLAGS_TAG  0x0004
#define EVC_PORT_FLAGS_ADDR 0x0008
#define EVC_PORT_FLAGS_L2CP 0x0010
#if defined(VTSS_ARCH_CARACAL)
#define EVC_PORT_FLAGS_ALL  (0xffff - EVC_PORT_FLAGS_L2CP)
#else
#define EVC_PORT_FLAGS_ALL  0xffff
#endif /* VTSS_ARCH_CARACAL */

static char *evc_cmd_pcp_txt(vtss_vcap_u8_t *pcp, char *buf)
{
    char *p = buf;

    if (pcp->mask == 0) {
        strcpy(buf, "Any");
    } else {
        p += sprintf(p, "%u", pcp->value);
        if (pcp->mask != 7) {
            sprintf(p, "-%u", pcp->value + (pcp->mask == 6 ? 1 : 3));
        }
    }
    return buf;
}

static const char *evc_cmd_vcap_bit_txt(vtss_vcap_bit_t value)
{
    return (value == VTSS_VCAP_BIT_ANY ? "Any" :
            value == VTSS_VCAP_BIT_1 ? "1" :
            value == VTSS_VCAP_BIT_0 ? "0" : "?");
}

static const char *evc_cmd_tag_type_txt(vtss_vcap_bit_t tagged, vtss_vcap_bit_t s_tagged)
{
    return (tagged == VTSS_VCAP_BIT_ANY ? "Any" :
            tagged == VTSS_VCAP_BIT_0 ? "Untagged" :
            s_tagged == VTSS_VCAP_BIT_0 ? "C-Tagged" : 
            s_tagged == VTSS_VCAP_BIT_1 ? "S-Tagged" : "Tagged");
}

static const char *evc_cmd_ece_type_txt(vtss_ece_type_t type)
{
    return (type == VTSS_ECE_TYPE_ANY ? "Any" :
            type == VTSS_ECE_TYPE_IPV4 ? "IPv4" :
            type == VTSS_ECE_TYPE_IPV6 ? "IPv6" : "?");
}

static char *evc_cmd_range_txt(vtss_vcap_vr_t *range, char *buf)
{
    vtss_vcap_vr_value_t low = range->vr.r.low;
    vtss_vcap_vr_value_t high = range->vr.r.high;
    
    if (range->type == VTSS_VCAP_VR_TYPE_VALUE_MASK)
        strcpy(buf, "Any");
    else if (low == high)
        sprintf(buf, "%u", low);
    else
        sprintf(buf, "%u-%u", low, high);
    return buf;
}

static char *evc_cmd_evc_id_txt(vtss_evc_id_t evc_id, char *buf)
{
    if (evc_id == VTSS_EVC_ID_NONE) {
        strcpy(buf, "None");
    } else {
        sprintf(buf, "%u", evc_id + 1);
    }
    return buf;
}

static char *evc_cmd_policer_id_txt(vtss_evc_policer_id_t policer_id, char *buf)
{
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    if (policer_id == VTSS_EVC_POLICER_ID_DISCARD) {
        strcpy(buf, "Discard");
    } else if (policer_id == VTSS_EVC_POLICER_ID_NONE) {
        strcpy(buf, "None");
    } else if (policer_id == VTSS_EVC_POLICER_ID_EVC) {
        strcpy(buf, "EVC");
    } else 
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    {
        sprintf(buf, "%u", policer_id + 1);
    }
    return buf;
}

#if defined(VTSS_ARCH_SERVAL)
static const char *evc_key_type_txt(vtss_vcap_key_type_t key_type)
{
    return (key_type == VTSS_VCAP_KEY_TYPE_NORMAL ? "Normal" :
            key_type == VTSS_VCAP_KEY_TYPE_DOUBLE_TAG ? "Dbl. Tag" :
            key_type == VTSS_VCAP_KEY_TYPE_IP_ADDR ? "IP Addr." : "MAC/IP");
}
#endif /* VTSS_ARCH_SERVAL */

static const char *evc_cmd_conflict_txt(BOOL conflict)
{
    return (conflict ? "Yes" : "No");
}

static void evc_cmd_port(cli_req_t *req, u16 flags)
{
    port_iter_t                pit;
    vtss_uport_no_t            uport;
    vtss_port_no_t             iport;
    evc_mgmt_port_conf_t       port_conf;
    vtss_evc_port_conf_t       *conf = &port_conf.conf;
    vtss_packet_rx_port_conf_t *reg = &port_conf.reg;
    BOOL                       header = 1;
    char                       buf[128], *p;
    ulong                      i;
    vtss_packet_reg_type_t     type;
    evc_cli_req_t              *evc_req = req->module_req;
    const char                 *txt;

    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0 ||
            evc_mgmt_port_conf_get(iport, &port_conf) != VTSS_RC_OK) {
            continue;
        }
        
        if (req->set) {
#if defined(VTSS_ARCH_SERVAL)
            if (flags & EVC_PORT_FLAGS_KEY)
                conf->key_type = evc_req->key_type;
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
            if (flags & EVC_PORT_FLAGS_DEI)
                conf->dei_colouring = evc_req->coloured;
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */
#if defined(VTSS_ARCH_CARACAL)
            if (flags & EVC_PORT_FLAGS_TAG)
                conf->inner_tag = evc_req->inner;
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            if (flags & EVC_PORT_FLAGS_ADDR)
                conf->dmac_dip = evc_req->dmac_dip;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
            if (flags & EVC_PORT_FLAGS_L2CP) {
                for (i = 0; i < 32; i++) {
                    if (evc_req->l2cp_list[i] == 0) {
                        continue;
                    }
                    if (i < 16) {
                        reg->bpdu_reg[i] = evc_req->fwd_type;
                    } else {
                        reg->garp_reg[i - 16] = evc_req->fwd_type;
                    }
                }
            }
            if (evc_mgmt_port_conf_set(iport, &port_conf) != VTSS_RC_OK) {
                CPRINTF("EVC port configuration failed for port %u\n", uport);
            }
            continue;
        }

        if (header) {
            header = 0;
            p = &buf[0];
            p += sprintf(p, "Port  ");
#if defined(VTSS_ARCH_SERVAL)
            if (flags & EVC_PORT_FLAGS_KEY)
                p += sprintf(p, "Key Type  ");
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
            if (flags & EVC_PORT_FLAGS_DEI)
                p += sprintf(p, "DEI Mode  ");
#endif /* VTSS_ARCH_JAGUAR_1 */
#if defined(VTSS_ARCH_CARACAL)
            if (flags & EVC_PORT_FLAGS_TAG)
                p += sprintf(p, "Tag Mode  ");
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            if (flags & EVC_PORT_FLAGS_ADDR)
                p += sprintf(p, "Address   ");
#endif /* VTSS_ARCH_CARACAL/SERVAL */
            if (flags & EVC_PORT_FLAGS_L2CP)
                p += sprintf(p, "%-32s%-32s", "BPDU", "GARP");
            cli_table_header(buf);
        }
        CPRINTF("%-6u", uport);
#if defined(VTSS_ARCH_SERVAL)
        if (flags & EVC_PORT_FLAGS_KEY)
            CPRINTF("%-10s", evc_key_type_txt(conf->key_type));
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
        if (flags & EVC_PORT_FLAGS_DEI) {
            CPRINTF("%-10s", conf->dei_colouring ? "Coloured" : "Fixed");
        }
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */
#if defined(VTSS_ARCH_CARACAL)
        if (flags & EVC_PORT_FLAGS_TAG) {
            CPRINTF("%-10s", conf->inner_tag ? "Inner" : "Outer");
        }
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        if (flags & EVC_PORT_FLAGS_ADDR) {
            CPRINTF("%-10s", conf->dmac_dip ? "DMAC/DIP" : "SMAC/SIP");
        }
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        if (flags & EVC_PORT_FLAGS_L2CP) {
            for (i = 0; i < 32; i++) {
                type = (i < 16 ? reg->bpdu_reg[i] : reg->garp_reg[i - 16]);
                txt = (type == VTSS_PACKET_REG_NORMAL ? "N" :
                       type == VTSS_PACKET_REG_FORWARD ? "F" :
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
                       type == VTSS_PACKET_REG_DISCARD ? "D" :
                       type == VTSS_PACKET_REG_CPU_COPY ? "C" :
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
                       type == VTSS_PACKET_REG_CPU_ONLY ? "R" : "?");
                CPRINTF("%s%s", txt, (i % 8) == 7 && i != 31 ? "-" : " ");
            }
        }
        CPRINTF("\n");
    }
}

static void evc_cmd_policer(cli_req_t *req)
{
    vtss_evc_policer_id_t   policer_id;
    evc_mgmt_policer_conf_t pol_conf;
    vtss_evc_policer_conf_t *conf = &pol_conf.conf;
    BOOL                    header = 1;
    evc_cli_req_t           *evc_req = req->module_req;
    vtss_evc_policer_conf_t *new = &evc_req->pol_conf;
    char                    buf[80];
    const char              *txt;

    for (policer_id = 0; policer_id < EVC_POL_COUNT; policer_id++) {
        if ((evc_req->policer_id_valid && evc_req->policer_id != policer_id) ||
            evc_mgmt_policer_conf_get(policer_id, &pol_conf) != VTSS_RC_OK) {
            continue;
        }

        if (req->set) {
            if (req->enable || req->disable) {
                conf->enable = req->enable;
            }

            if (evc_req->coupled) {
#if defined(VTSS_ARCH_JAGUAR_1)
                conf->cm = 1;
#endif /* VTSS_ARCH_JAGUAR_1 */
                conf->cf = 1;
            } else if (evc_req->aware) {
#if defined(VTSS_ARCH_JAGUAR_1)
                conf->cm = 1;
#endif /* VTSS_ARCH_JAGUAR_1 */
                conf->cf = 0;
            } else if (evc_req->blind) {
#if defined(VTSS_ARCH_JAGUAR_1)
                conf->cm = 0;
#endif /* VTSS_ARCH_JAGUAR_1 */
                conf->cf = 0;
            }

            if (evc_req->cir_valid) {
                conf->cir = new->cir;
            }
            if (evc_req->cbs_valid) {
                conf->cbs = new->cbs;
            }
            if (evc_req->eir_valid) {
                conf->eir = new->eir;
            }
            if (evc_req->ebs_valid) {
                conf->ebs = new->ebs;
            }
            if (evc_mgmt_policer_conf_set(policer_id, &pol_conf) != VTSS_RC_OK)
                CPRINTF("EVC profile configuration failed for policer: %s",
                        evc_cmd_policer_id_txt(policer_id, buf));
            continue;
        }
        if (header) {
            header = 0;
            if (req->all) {
                CPRINTF("\n");
            }
            cli_table_header("Policer  State     Mode     CIR      CBS    EIR      EBS ");
        }
        txt = (
#if defined(VTSS_ARCH_JAGUAR_1)
            conf->cm == 0 ? "Blind" : 
#endif /* VTSS_ARCH_JAGUAR_1 */
            conf->cf ? "Coupled" : "Aware");
        CPRINTF("%-9s%-10s%-9s%-9u%-7u%-9u%-7u\n",
                evc_cmd_policer_id_txt(policer_id, buf),
                cli_bool_txt(conf->enable),
                txt,
                conf->cir,
                conf->cbs,
                conf->eir,
                conf->ebs);
    }
}

#if defined(VTSS_ARCH_SERVAL)
static void evc_cmd_port_key(cli_req_t *req)
{
    evc_cmd_port(req, EVC_PORT_FLAGS_KEY);
}
#else
static void evc_cmd_port_dei(cli_req_t *req)
{
    evc_cmd_port(req, EVC_PORT_FLAGS_DEI);
}
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL)
static void evc_cmd_port_tag(cli_req_t *req)
{
    evc_cmd_port(req, EVC_PORT_FLAGS_TAG);
}
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static void evc_cmd_port_addr(cli_req_t *req)
{
    evc_cmd_port(req, EVC_PORT_FLAGS_ADDR);
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

static void evc_cmd_port_l2cp(cli_req_t *req)
{
    evc_cmd_port(req, EVC_PORT_FLAGS_L2CP);
}

static void evc_cmd_evc_add(cli_req_t *req)
{
    evc_mgmt_conf_t    evc_conf;
    vtss_evc_conf_t    *evc = &evc_conf.conf;
    vtss_evc_pb_conf_t *pb = &evc->network.pb;
    evc_port_info_t    info[VTSS_PORT_ARRAY_SIZE];
    port_iter_t        pit;
    vtss_port_no_t     iport;
    vtss_uport_no_t    uport;
    evc_cli_req_t      *evc_req = req->module_req;
    vtss_evc_id_t      evc_id = evc_req->evc_id;
    BOOL               add;

    /* Get current entry or defaults */
    add = (evc_mgmt_get(&evc_id, &evc_conf, 0) == VTSS_RC_OK ? 0 : 1);

    /* VLAN ID */
    if (evc_req->evc_vid.valid)
        pb->vid = evc_req->evc_vid.value;
    else if (add)
        pb->vid = 1;
    
    /* Internal VLAN ID */
    if (evc_req->ivid.valid)
        pb->ivid = evc_req->ivid.value;
    else if (pb->ivid == 0)
        pb->ivid = pb->vid;

    /* NNI list */
    if (evc_req->nni_list_valid) {
        if (evc_mgmt_port_info_get(info) != VTSS_OK) {
            CPRINTF("Port info failed\n");
            return;
        }
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            uport = iport2uport(iport);
            if (req->uport_list[uport] && info[iport].uni_count) {
                CPRINTF("Port %u is a UNI\n", uport);
                return;
            }
            pb->nni[iport] = req->uport_list[uport];
        }
    }

    /* Learning */
    if (req->enable || req->disable)
        evc->learning = req->enable;

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Policer ID */
    if (evc_req->policer_id_valid)
        evc->policer_id = evc_req->policer_id;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */


#if defined(VTSS_ARCH_CARACAL)
    /* Inner tag */
    if (evc_req->it_type_valid)
        pb->inner_tag.type = evc_req->it_type;

    if (evc_req->it_vid_mode_valid)
        pb->inner_tag.vid_mode = evc_req->it_vid_mode;

    if (evc_req->it_vid.valid)
        pb->inner_tag.vid = evc_req->it_vid.value;

    if (evc_req->it_pcp_mode.valid)
        pb->inner_tag.pcp_dei_preserve = evc_req->it_pcp_mode.value;

    if (evc_req->it_pcp.valid)
        pb->inner_tag.pcp = evc_req->it_pcp.value;

    if (evc_req->it_dei.valid)
        pb->inner_tag.dei = evc_req->it_dei.value;

    /* Outer tag */
    if (evc_req->ot_vid.valid)
        pb->uvid = evc_req->ot_vid.value;
#endif /* VTSS_ARCH_CARACAL */

    /* Add/modify entry */
    if (evc_mgmt_add(evc_id, &evc_conf) != VTSS_OK) {
        CPRINTF("EVC configuration failed\n");
    }
}

static void evc_cmd_evc_del(cli_req_t *req)
{
    evc_mgmt_conf_t evc_conf;
    evc_cli_req_t   *evc_req = req->module_req;
    vtss_evc_id_t   evc_id = evc_req->evc_id;
    char            buf[80];

    if (evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_OK) {
        CPRINTF("EVC %s does not exist\n", evc_cmd_evc_id_txt(evc_id, buf));
    } else if (evc_mgmt_del(evc_id) != VTSS_OK) {
        CPRINTF("EVC delete failed\n");
    }
}

static const char *evc_cmd_dir_txt(vtss_ece_dir_t dir)
{
    return (dir == VTSS_ECE_DIR_BOTH ? "Both" : 
            dir == VTSS_ECE_DIR_UNI_TO_NNI ? "UNI-to-NNI" : "NNI-to-UNI");
}

static void evc_ece_txt(const char *name, const char *txt)
{
    CPRINTF("%-16s: %s\n", name, txt);
}

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static char *evc_ipv4_txt(vtss_vcap_ip_t *ip, char *buf)
{
    u32 i, n;

    for (n = 0, i = 0; i < 32; i++)
        if (ip->mask & (1<<i))
            n++;
    if (n == 0)
        strcpy(buf, "Any");
    else {
        i = strlen(misc_ipv4_txt(ip->value, buf));
        sprintf(&buf[i], "/%u", n);
    }
    return buf;
}

static char *evc_ipv6_txt(vtss_vcap_u128_t *ipv6, char *buf)
{
    u32            i, j;
    vtss_vcap_ip_t ipv4;

    ipv4.value = 0;
    ipv4.mask = 0;
    for (i = 0; i < 4; i++) {
        j = ((3 - i) * 8);
        ipv4.value += (ipv6->value[i + 12] << j);
        ipv4.mask += (ipv6->mask[i + 12] << j);
    }
    return evc_ipv4_txt(&ipv4, buf);
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
static void evc_ece_dei_mode_txt(const char *name, vtss_ece_dei_mode_t mode)
{
    evc_ece_txt(name, 
                mode == VTSS_ECE_DEI_MODE_CLASSIFIED ? "Classified" :
                mode == VTSS_ECE_DEI_MODE_FIXED ? "Fixed" : 
                mode == VTSS_ECE_DEI_MODE_DP ? "DP" : "?");
}

static void evc_ece_pcp_mode_txt(const char *name, vtss_ece_pcp_mode_t mode)
{
    evc_ece_txt(name, 
                mode == VTSS_ECE_PCP_MODE_CLASSIFIED ? "Classified" :
                mode == VTSS_ECE_PCP_MODE_FIXED ? "Fixed" : 
                mode == VTSS_ECE_PCP_MODE_MAPPED ? "Mapped" : "?");
}
#else
static void evc_ece_pcp_mode_txt(const char *name, BOOL preserve)
{
    evc_ece_txt(name, preserve ? "Classified" : "Fixed");
}
#endif /* VTSS_ARCH_SERVAL */

static void evc_ece_u32(const char *name, u32 value)
{
    CPRINTF("%-16s: %u\n", name, value);
}

static void evc_cmd_ece_show(evc_mgmt_ece_conf_t *conf)
{
    vtss_ece_t            *ece = &conf->conf;
    vtss_ece_key_t        *key = &ece->key;
    vtss_ece_tag_t        *tag = &key->tag;
    vtss_ece_frame_ipv4_t *ipv4 = &key->frame.ipv4;
    vtss_ece_frame_ipv6_t *ipv6 = &key->frame.ipv6;
    vtss_ece_action_t     *action = &ece->action;
    vtss_ece_outer_tag_t  *ot = &action->outer_tag;
    BOOL                  iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t           pit;
    vtss_port_no_t        iport;
    char                  buf[MGMT_PORT_BUF_SIZE];

    /* Show ECE information in two columns */
    port_iter_init_local_all(&pit);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        iport_list[iport] = (key->port_list[iport] == VTSS_ECE_PORT_NONE ? 0 : 1);
    }
    evc_ece_u32("ECE ID", ece->id);
    CPRINTF("\n");
    cli_table_header("Key Parameters");
    evc_ece_txt("UNI Ports", cli_iport_list_txt(iport_list, buf));
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    evc_ece_txt("SMAC", key->mac.smac.mask[0] ? misc_mac_txt(key->mac.smac.value, buf) : "Any");
    evc_ece_txt("DMAC Type", 
                key->mac.dmac_bc == VTSS_VCAP_BIT_1 ? "Broadcast" :
                key->mac.dmac_mc == VTSS_VCAP_BIT_1 ? "Multicast" :
                key->mac.dmac_mc == VTSS_VCAP_BIT_0 ? "Unicast" : "Any");
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_txt("DMAC", key->mac.dmac.mask[0] ? misc_mac_txt(key->mac.dmac.value, buf) : "Any");
#endif /* VTSS_ARCH_SERVAL */
    evc_ece_txt("Outer Tag Type", evc_cmd_tag_type_txt(tag->tagged, tag->s_tagged));
    evc_ece_txt("Outer VID", evc_cmd_range_txt(&tag->vid, buf));
    evc_ece_txt("Outer PCP", evc_cmd_pcp_txt(&tag->pcp, buf));
    evc_ece_txt("Outer DEI", evc_cmd_vcap_bit_txt(tag->dei));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    tag = &key->inner_tag;
    evc_ece_txt("Inner Tag Type", evc_cmd_tag_type_txt(tag->tagged, tag->s_tagged));
    evc_ece_txt("Inner VID", evc_cmd_range_txt(&tag->vid, buf));
    evc_ece_txt("Inner PCP", evc_cmd_pcp_txt(&tag->pcp, buf));
    evc_ece_txt("Inner DEI", evc_cmd_vcap_bit_txt(tag->dei));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    evc_ece_txt("Frame Type", evc_cmd_ece_type_txt(key->type));
    if (key->type == VTSS_ECE_TYPE_IPV6)
        ipv4 = NULL;
    if (key->type != VTSS_ECE_TYPE_ANY) {
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        vtss_vcap_u8_t *proto = (ipv4 ? &ipv4->proto : &ipv6->proto);
        BOOL           tcp = (proto->value == 6);
        BOOL           udp = (proto->value == 17);
        
        if (proto->mask == 0)
            strcpy(buf, "Any");
        else
            sprintf(buf, "%u %s", proto->value, tcp ? "(TCP)" : udp ? "(UDP)" : "");
        evc_ece_txt("Protocol", buf);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

        evc_ece_txt("DSCP", evc_cmd_range_txt(ipv4 ? &ipv4->dscp : &ipv6->dscp, buf));
        
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        if (ipv4) {
            evc_ece_txt("Fragment", 
                        ipv4->fragment == VTSS_VCAP_BIT_ANY ? "Any" :
                        ipv4->fragment == VTSS_VCAP_BIT_1 ? "Fragment" :
                        ipv4->fragment == VTSS_VCAP_BIT_0 ? "Non-Fragment" : "?");
        }
        evc_ece_txt("Source IP", 
                    ipv4 ? evc_ipv4_txt(&ipv4->sip, buf) : evc_ipv6_txt(&ipv6->sip, buf));
#if defined(VTSS_ARCH_SERVAL)
        evc_ece_txt("Destination IP", 
                    ipv4 ? evc_ipv4_txt(&ipv4->dip, buf) : evc_ipv6_txt(&ipv6->dip, buf));
#endif /* VTSS_ARCH_SERVAL */

        /* UDP/TCP ports */
        if (udp || tcp) {
            evc_ece_txt("Source Port", 
                        evc_cmd_range_txt(ipv4 ? &ipv4->sport : &ipv6->sport, buf));
            evc_ece_txt("Destination Port", 
                        evc_cmd_range_txt(ipv4 ? &ipv4->dport : &ipv6->dport, buf));
        }
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    }

    CPRINTF("\n");
    cli_table_header("Action Parameters");
    evc_ece_txt("Direction", evc_cmd_dir_txt(action->dir));
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_txt("Rule Type", action->rule == VTSS_ECE_RULE_BOTH ? "Both" :
                action->rule == VTSS_ECE_RULE_RX ? "Rx" : "Tx");
    evc_ece_txt("Tx Lookup", action->tx_lookup == VTSS_ECE_TX_LOOKUP_VID ? "VID" :
                action->tx_lookup == VTSS_ECE_TX_LOOKUP_VID_PCP ? "PCP_VID" : "ISDX");
#endif /* VTSS_ARCH_SERVAL */
    evc_ece_txt("EVC ID", evc_cmd_evc_id_txt(action->evc_id, buf));
    evc_ece_u32("Tag Pop Count", action->pop_tag);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    evc_ece_txt("Policer ID", evc_cmd_policer_id_txt(action->policer_id, buf));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    evc_ece_u32("Policy Number", action->policy_no);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    if (action->prio_enable)
        sprintf(buf, "%u", action->prio);
    else
        strcpy(buf, "Disabled");
    evc_ece_txt("Class", buf);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    if (action->dp_enable)
        sprintf(buf, "%u", action->dp);
    else
        strcpy(buf, "Disabled");
    evc_ece_txt("DP", buf);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    evc_ece_txt("Outer Mode", cli_bool_txt(ot->enable));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    evc_ece_u32("Outer VID", ot->vid);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_pcp_mode_txt("Outer PCP Mode", ot->pcp_mode);
#else
    evc_ece_pcp_mode_txt("Outer PCP/DEI", ot->pcp_dei_preserve);
#endif /* VTSS_ARCH_SERVAL */
    evc_ece_u32("Outer PCP", ot->pcp);
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_dei_mode_txt("Outer DEI Mode", ot->dei_mode);
#endif /* VTSS_ARCH_SERVAL */
    evc_ece_u32("Outer DEI", ot->dei);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        vtss_ece_inner_tag_t *it = &action->inner_tag;

        evc_ece_txt("Inner Tag Type",
                    it->type == VTSS_ECE_INNER_TAG_NONE ? "None" :
                    it->type == VTSS_ECE_INNER_TAG_C ? "C-Tag" :
                    it->type == VTSS_ECE_INNER_TAG_S ? "S-Tag" :
                    it->type == VTSS_ECE_INNER_TAG_S_CUSTOM ? "S-Custom-Tag" : "?");
        evc_ece_u32("Inner VID", it->vid);
#if defined(VTSS_ARCH_SERVAL)
    evc_ece_pcp_mode_txt("Inner PCP Mode", it->pcp_mode);
#else
    evc_ece_pcp_mode_txt("Inner PCP/DEI", it->pcp_dei_preserve);
#endif /* VTSS_ARCH_SERVAL */
        evc_ece_u32("Inner PCP", it->pcp);
#if defined(VTSS_ARCH_SERVAL)
        evc_ece_dei_mode_txt("Inner DEI Mode", it->dei_mode);
#endif /* VTSS_ARCH_SERVAL */
        evc_ece_u32("Inner DEI", it->dei);
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    evc_ece_txt("Conflict", evc_cmd_conflict_txt(conf->conflict));
}

static void evc_cmd_evc_show(cli_req_t *req, BOOL status)
{
    vtss_evc_id_t       evc_id;
    evc_mgmt_conf_t     evc_conf;
    vtss_evc_conf_t     *evc = &evc_conf.conf;
    vtss_evc_pb_conf_t  *pb = &evc->network.pb;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_t          *ece = &ece_conf.conf;
    BOOL                iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t         pit;
    vtss_port_no_t      iport;
    BOOL                header = 1;
    char                buf[MGMT_PORT_BUF_SIZE], *p;
    evc_cli_req_t       *evc_req = req->module_req;

    evc_id = (evc_req->evc_id_valid ? evc_req->evc_id : EVC_ID_FIRST);
    while (1) {
        if (evc_req->evc_id_valid) {
            /* Get specific EVC */
            if (evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_OK) {
                CPRINTF("EVC %s does not exist\n", evc_cmd_evc_id_txt(evc_id, buf));
                return;
            }
        } else {
            /* Get first/next EVC */
            if (evc_mgmt_get(&evc_id, &evc_conf, 1) != VTSS_OK) {
                return;
            }
        }

        port_iter_init_local_all(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            iport_list[iport] = pb->nni[iport];
        }
        /* Show EVC */
        if (header) {
            header = 0;
            if (req->all) {
                CPRINTF("\n");
            }
            p = &buf[0];
            p += sprintf(p, "EVC ID  VID   IVID  ");
            if (status) {
                p += sprintf(p, "Conflict");
            } else {
                p += sprintf(p, "Learning  ");
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
                p += sprintf(p, "Policer  ");
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
                p += sprintf(p, "NNI Ports        ");
#if defined(VTSS_ARCH_CARACAL)
                CPRINTF("%-*s%-17s%-11s%s\n", strlen(buf), "", "Inner", "Inner", "Outer");
                p += sprintf(p, "Type/Mode/VID    PCP/DEI    VID  ");
#endif /* VTSS_ARCH_CARACAL */
            }
            cli_table_header(buf);
        }
        CPRINTF("%-8s%-6u%-6u", evc_cmd_evc_id_txt(evc_id, buf), pb->vid, pb->ivid);
        if (status) {
            CPRINTF("%s\n", evc_cmd_conflict_txt(evc_conf.conflict));
        } else {
            CPRINTF("%-10s", cli_bool_txt(evc->learning));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            CPRINTF("%-9s", evc_cmd_policer_id_txt(evc->policer_id, buf));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
            CPRINTF("%-17s", cli_iport_list_txt(iport_list, buf));
#if defined(VTSS_ARCH_CARACAL)
            {
                vtss_evc_inner_tag_t *it = &pb->inner_tag;
                
                sprintf(buf, "%s/%s/%u",
                        it->type == VTSS_EVC_INNER_TAG_NONE ? "None" :
                        it->type == VTSS_EVC_INNER_TAG_C ? "C" :
                        it->type == VTSS_EVC_INNER_TAG_S ? "S" :
                        it->type == VTSS_EVC_INNER_TAG_S_CUSTOM ? "S-C" : "?",
                        it->vid_mode == VTSS_EVC_VID_MODE_NORMAL ? "Normal" : "Tunnel",
                        it->vid);
                CPRINTF("%-17s%s %u/%u  %u",
                        buf,
                        it->pcp_dei_preserve ? "Pres." : "Fixed",
                        it->pcp,
                        it->dei,
                        pb->uvid);
            }
#endif /* VTSS_ARCH_CARACAL */
            CPRINTF("\n");
        }
        if (evc_req->evc_id_valid)
            break;
    }

    if (evc_req->evc_id_valid) {
        /* Show all ECEs mapping to EVC */
        ece->id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(ece->id, &ece_conf, 1) == VTSS_RC_OK) {
            if (ece_conf.conf.action.evc_id == evc_id) {
                CPRINTF("\n");
                evc_cmd_ece_show(&ece_conf);
            }
        }
    }
}

static void evc_cmd_evc_lookup(cli_req_t *req)
{
    evc_cmd_evc_show(req, 0);
}

static void evc_cmd_evc_status(cli_req_t *req)
{
    evc_cmd_evc_show(req, 1);
}

static void evc_cmd_ece_add(cli_req_t *req)
{
    evc_cli_req_t         *evc_req = req->module_req;
    vtss_ece_id_t         id = evc_req->ece_id;
    vtss_ece_id_t         id_next = VTSS_ECE_ID_LAST;
    evc_mgmt_ece_conf_t   conf, next_conf;
    vtss_ece_key_t        *key = &conf.conf.key;
    vtss_ece_tag_t        *tag = &key->tag;
    vtss_ece_frame_ipv4_t *ipv4 = &key->frame.ipv4;
    vtss_ece_frame_ipv6_t *ipv6 = &key->frame.ipv6;
    vtss_ece_action_t     *action = &conf.conf.action;
    vtss_ece_outer_tag_t  *ot = &action->outer_tag;
    port_iter_t           pit;
    vtss_port_no_t        iport;
    vtss_uport_no_t       uport;
    evc_port_info_t       info[VTSS_PORT_ARRAY_SIZE];
    BOOL                  add;

    /* Determine whether to add or modify */
    add = (id == 0 || evc_mgmt_ece_get(id, &conf, 0) != VTSS_RC_OK);
    if (add) {
        /* Add new entry */
        memset(&conf, 0, sizeof(conf));
        conf.conf.id = id;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        ot->vid = VLAN_ID_DEFAULT;
        action->inner_tag.vid = VLAN_ID_DEFAULT;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    }

    if (evc_req->ece_id_next_valid) {
        /* Next ID is specified, check that it is valid */
        id_next = evc_req->ece_id_next;
        if (id_next != VTSS_ECE_ID_LAST &&
            evc_mgmt_ece_get(id_next, &next_conf, 0) != VTSS_RC_OK) {
            CPRINTF("ECE ID %u does not exist\n", id_next);
            return;
        }
    } else if (!add && evc_mgmt_ece_get(id, &next_conf, 1) == VTSS_RC_OK) {
        /* Next ID not specified, preserve the ordering */
        id_next = next_conf.conf.id;
    }

    /* UNI list */
    if (evc_req->uni_list_valid) {
        if (evc_mgmt_port_info_get(info) != VTSS_OK) {
            CPRINTF("Port info failed\n");
            return;
        }
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            uport = iport2uport(iport);
            if (req->uport_list[uport] && info[iport].nni_count) {
                CPRINTF("Port %u is an NNI\n", uport);
                return;
            }
            key->port_list[iport] = (req->uport_list[uport] ? VTSS_ECE_PORT_ROOT :
                                     VTSS_ECE_PORT_NONE);
        }
    }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    /* MAC address matching */
    if (evc_req->smac.valid)
        key->mac.smac = evc_req->smac.value;

    if (evc_req->dmac_type_valid) {
        key->mac.dmac_mc = evc_req->dmac_mc;
        key->mac.dmac_bc = evc_req->dmac_bc;
    }
    
#if defined(VTSS_ARCH_SERVAL)
    if (evc_req->dmac.valid)
        key->mac.dmac = evc_req->dmac.value;
#endif /* VTSS_ARCH_SERVAL */
#endif /* VTSS_ARCH_CARACAL/SERVAL */

    /* Outer tag matching */
    if (evc_req->tag_type.valid) {
        tag->tagged = evc_req->tag_type.tagged;
        tag->s_tagged = evc_req->tag_type.s_tagged;
    }

    if (evc_req->vid.valid)
        tag->vid = evc_req->vid.value;

    if (evc_req->pcp.valid) 
        tag->pcp = evc_req->pcp.value;

    if (evc_req->dei.valid)
        tag->dei = evc_req->dei.value;

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Inner tag matching */
    tag = &key->inner_tag;
    if (evc_req->in_type.valid) {
        tag->tagged = evc_req->in_type.tagged;
        tag->tagged = evc_req->in_type.s_tagged;
    }

    if (evc_req->in_vid.valid)
        tag->vid = evc_req->in_vid.value;

    if (evc_req->in_pcp.valid) 
        tag->pcp = evc_req->in_pcp.value;

    if (evc_req->in_dei.valid)
        tag->dei = evc_req->in_dei.value;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    /* Frame type and specific fields */
    if (evc_req->ece_type_valid && key->type != evc_req->ece_type) {
        key->type = evc_req->ece_type;
        memset(&key->frame, 0, sizeof(key->frame));
    }
    
    if (key->type == VTSS_ECE_TYPE_IPV4) {
        if (evc_req->dscp.valid)
            ipv4->dscp = evc_req->dscp.value;

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        if (evc_req->proto.valid)
            ipv4->proto = evc_req->proto.value;
        
        if (evc_req->sip.valid)
            ipv4->sip = evc_req->sip.ipv4;
        
#if defined(VTSS_ARCH_SERVAL)
        if (evc_req->dip.valid)
            ipv4->dip = evc_req->dip.ipv4;
#endif /* VTSS_ARCH_SERVAL */

        if (evc_req->fragment.valid)
            ipv4->fragment = evc_req->fragment.value;
        
        if (evc_req->sport.valid)
            ipv4->sport = evc_req->sport.value;
        
        if (evc_req->dport.valid)
            ipv4->dport = evc_req->dport.value;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    } else if (key->type == VTSS_ECE_TYPE_IPV6) {
        if (evc_req->dscp.valid)
            ipv6->dscp = evc_req->dscp.value;
        
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        if (evc_req->proto.valid)
            ipv6->proto = evc_req->proto.value;
        
        if (evc_req->sip.valid)
            ipv6->sip = evc_req->sip.ipv6;
        
#if defined(VTSS_ARCH_SERVAL)
        if (evc_req->dip.valid)
            ipv6->dip = evc_req->dip.ipv6;
#endif /* VTSS_ARCH_SERVAL */

        if (evc_req->sport.valid)
            ipv6->sport = evc_req->sport.value;
        
        if (evc_req->dport.valid)
            ipv6->dport = evc_req->dport.value;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    }

    /* Direction */
    if (evc_req->dir_valid)
        action->dir = evc_req->dir;
    
#if defined(VTSS_ARCH_SERVAL)
    /* Rule type */
    if (evc_req->rule_valid)
        action->rule = evc_req->rule;

    /* Tx lookup */
    if (evc_req->tx_lookup_valid)
        action->tx_lookup = evc_req->tx_lookup;
#endif /* VTSS_ARCH_SERVAL */

    /* EVC mapping */
    if (evc_req->evc_id_valid)
        action->evc_id = evc_req->evc_id;

    /* Pop count */
    if (evc_req->pop_valid)
        action->pop_tag = evc_req->pop;

    /* ACL policy number */
    if (evc_req->policy_valid)
        action->policy_no = evc_req->policy_no;

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    /* Class */
    if (evc_req->prio_valid) {
        action->prio_enable = evc_req->prio_enable;
        action->prio = evc_req->prio;
    }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
    /* Drop precedence */
    if (evc_req->dp_valid) {
        action->dp_enable = evc_req->dp_enable;
        action->dp = evc_req->dp;
    }
#endif /* VTSS_ARCH_SERVAL */

    /* Outer tag */
    if (evc_req->ot_mode.valid)
        ot->enable = evc_req->ot_mode.value;
    
    if (evc_req->ot_pcp_mode.valid) {
#if defined(VTSS_ARCH_SERVAL)
        ot->pcp_mode = evc_req->ot_pcp_mode.value;
#else
        ot->pcp_dei_preserve = evc_req->ot_pcp_mode.value;
#endif /* VTSS_ARCH_SERVAL */
    }

    if (evc_req->ot_pcp.valid)
        ot->pcp = evc_req->ot_pcp.value;

#if defined(VTSS_ARCH_SERVAL)
    if (evc_req->ot_dei_mode.valid)
        ot->dei_mode = evc_req->ot_dei_mode.value;
#endif /* VTSS_ARCH_SERVAL */

    if (evc_req->ot_dei.valid)
        ot->dei = evc_req->ot_dei.value;

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    if (evc_req->ot_vid.valid)
        ot->vid = evc_req->ot_vid.value;

    /* Policer mapping */
    if (evc_req->policer_id_valid)
        action->policer_id = evc_req->policer_id;

    {
        vtss_ece_inner_tag_t *it = &action->inner_tag;

        /* Inner tag */
        if (evc_req->it_type_valid)
            it->type = evc_req->it_type;

        if (evc_req->it_vid.valid)
            it->vid = evc_req->it_vid.value;

        if (evc_req->it_pcp_mode.valid) {
#if defined(VTSS_ARCH_SERVAL)
            it->pcp_mode = evc_req->it_pcp_mode.value;
#else
            it->pcp_dei_preserve = evc_req->it_pcp_mode.value;
#endif /* VTSS_ARCH_SERVAL */
        }
        
        if (evc_req->it_pcp.valid)
            it->pcp = evc_req->it_pcp.value;

#if defined(VTSS_ARCH_SERVAL)
        if (evc_req->it_dei_mode.valid)
            it->dei_mode = evc_req->it_dei_mode.value;
#endif /* VTSS_ARCH_SERVAL */

        if (evc_req->it_dei.valid)
            it->dei = evc_req->it_dei.value;
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    /* Add/modify ECE */
    if (evc_mgmt_ece_add(id_next, &conf) != VTSS_RC_OK) {
        CPRINTF("ECE add failed\n");
    }
}

static void evc_cmd_ece_del(cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    if (evc_mgmt_ece_del(evc_req->ece_id) != VTSS_RC_OK) {
        CPRINTF("ECE delete failed\n");
    }
}

static void evc_cmd_ece_list(cli_req_t *req, BOOL status)
{
    evc_mgmt_ece_conf_t conf;
    vtss_ece_t          *ece = &conf.conf;
    vtss_ece_key_t      *key = &ece->key;
    vtss_ece_action_t   *action = &ece->action;
    vtss_ece_tag_t      *tag = &key->tag;
    evc_cli_req_t       *evc_req = req->module_req;
    BOOL                iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t         pit;
    vtss_port_no_t      iport;
    BOOL                header = 1;
    char                buf[MGMT_PORT_BUF_SIZE], *p;

    if (evc_req->ece_id) {
        /* Detailed view of specific ECE */
        if (evc_mgmt_ece_get(evc_req->ece_id, &conf, 0) != VTSS_RC_OK) {
            CPRINTF("ECE ID not found\n");
            return;
        }
        if (req->all) {
            CPRINTF("\n");
        }
        evc_cmd_ece_show(&conf);
        return;
    }

    /* Overview of all ECEs */
    ece->id = EVC_ECE_ID_FIRST;
    while (evc_mgmt_ece_get(ece->id, &conf, 1) == VTSS_RC_OK) {
        if (header) {
            header = 0;
            if (req->all) {
                CPRINTF("\n");
            }
            p = &buf[0];
            p += sprintf(p, "ECE ID  Direction   EVC ID  ");
            if (status) {
                p += sprintf(p, "Conflict");
            } else {
                p += sprintf(p, "Tag Type  VID        PCP  DEI  Frame  UNI Ports  ");
            }
            cli_table_header(buf);
        }
        port_iter_init_local_all(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            iport_list[iport] = (key->port_list[iport] == VTSS_ECE_PORT_NONE ? 0 : 1);
        }
        CPRINTF("%-8u%-12s%-8s", 
                ece->id, evc_cmd_dir_txt(action->dir), evc_cmd_evc_id_txt(action->evc_id, buf));
        if (status) {
            CPRINTF("%s\n", evc_cmd_conflict_txt(conf.conflict));
        } else {
            CPRINTF("%-10s", evc_cmd_tag_type_txt(tag->tagged, tag->s_tagged));
            CPRINTF("%-11s", evc_cmd_range_txt(&tag->vid, buf));
            CPRINTF("%-5s%-5s",
                    evc_cmd_pcp_txt(&tag->pcp, buf),
                    evc_cmd_vcap_bit_txt(tag->dei));
            CPRINTF("%-7s%s\n",
                    evc_cmd_ece_type_txt(key->type),
                    cli_iport_list_txt(iport_list, buf));
        }
    }
}

static void evc_cmd_ece_lookup(cli_req_t *req)
{
    evc_cmd_ece_list(req, 0);
}

static void evc_cmd_ece_status(cli_req_t *req)
{
    evc_cmd_ece_list(req, 1);
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
/* Print counters in two columns with header */
static void evc_cmd_stat_port(vtss_evc_id_t evc_id, vtss_ece_id_t ece_id,
                              vtss_uport_no_t uport, BOOL *header, const char *name,
                              vtss_counter_pair_t *c1, vtss_counter_pair_t *c2, BOOL bytes)
{
    char buf[80], buf1[80], *p;

    if (*header) {
        *header = 0;
        p = &buf[0];
        sprintf(buf1, "%s %s", name, bytes ? "Bytes" : "Frames");
        p += sprintf(p, "%s ID  Port  Rx %-19s",
                     evc_id == VTSS_EVC_ID_NONE ? "ECE" : "EVC", buf1);
        if (c2 != NULL) {
            sprintf(p, "Tx %-19s", buf1);
        }
        cli_table_header(buf);
    }
    if (evc_id == VTSS_EVC_ID_NONE) {
        CPRINTF("%-8u", ece_id);
    } else {
        CPRINTF("%-8s", evc_cmd_evc_id_txt(evc_id, buf));
    }
    CPRINTF("%-6u%-22llu", uport, bytes ? c1->bytes : c1->frames);
    if (c2 != NULL) {
        CPRINTF("%-22llu", bytes ? c2->bytes : c2->frames);
    }
    CPRINTF("\n");
}

/* Print two counters in columns */
static void evc_cmd_stats(const char *name,
                          vtss_counter_pair_t *c1, vtss_counter_pair_t *c2, BOOL bytes)
{
    char buf[80];

    sprintf(buf, "%s %s:", name, bytes ? "Bytes" : "Frames");
    CPRINTF("Rx %-15s%20llu   ", buf, bytes ? c1->bytes : c1->frames);
    if (c2 != NULL) {
        CPRINTF("Tx %-15s%20llu", buf, bytes ? c2->bytes : c2->frames);
    }
    CPRINTF("\n");
}

static void evc_cmd_show_stats(cli_req_t *req, vtss_evc_id_t evc_id, vtss_ece_id_t ece_id,
                               vtss_uport_no_t uport, BOOL *header,
                               vtss_evc_counters_t *counters)
{
    evc_cli_req_t *evc_req = req->module_req;
    BOOL          bytes = evc_req->bytes;
    char          buf[80];

    if (evc_req->green) {
        evc_cmd_stat_port(evc_id, ece_id, uport, header,
                          "Green", &counters->rx_green, &counters->tx_green, bytes);
    } else if (evc_req->yellow) {
        evc_cmd_stat_port(evc_id, ece_id, uport, header,
                          "Yellow", &counters->rx_yellow, &counters->tx_yellow, bytes);
    } else if (evc_req->red) {
        evc_cmd_stat_port(evc_id, ece_id, uport, header,
                          "Red", &counters->rx_red, NULL, bytes);
    } else if (evc_req->discard) {
        evc_cmd_stat_port(evc_id, ece_id, uport, header,
                          "Discard", &counters->rx_discard, &counters->tx_discard, bytes);
    } else {
        /* Detailed statistics */
        if (evc_id == VTSS_EVC_ID_NONE) {
            CPRINTF("ECE ID %u", ece_id);
        } else {
            CPRINTF("EVC ID %s", evc_cmd_evc_id_txt(evc_id, buf));
        }
        CPRINTF(", Port %u Statistics:\n\n", uport);
        for (bytes = 0; bytes < 2; bytes++) {
            if ((bytes == 0 && evc_req->bytes) || (bytes == 1 && evc_req->frames)) {
                continue;
            }
            evc_cmd_stats("Green", &counters->rx_green, &counters->tx_green, bytes);
            evc_cmd_stats("Yellow", &counters->rx_yellow, &counters->tx_yellow, bytes);
            evc_cmd_stats("Red", &counters->rx_red, NULL, bytes);
            evc_cmd_stats("Discard", &counters->rx_discard, &counters->tx_discard, bytes);
        }
        CPRINTF("\n");
    }
}

static void evc_cmd_evc_statistics(cli_req_t *req)
{
    vtss_evc_id_t       evc_id;
    evc_mgmt_conf_t     evc_conf;
    vtss_evc_pb_conf_t  *pb = &evc_conf.conf.network.pb;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_t          *ece = &ece_conf.conf;
    vtss_ece_key_t      *key = &ece->key;
    BOOL                iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t         pit;
    vtss_port_no_t      iport;
    vtss_uport_no_t     uport;
    vtss_evc_counters_t counters;
    BOOL                next, header = 1;
    evc_cli_req_t       *evc_req = req->module_req;
    char                buf[80];

    if (evc_req->evc_id_valid) {
        evc_id = evc_req->evc_id;
        next = 0;
    } else {
        evc_id = EVC_ID_FIRST;
        next = 1;
    }
    while (1) {
        if (evc_mgmt_get(&evc_id, &evc_conf, next) != VTSS_OK) {
            if (!next) {
                CPRINTF("EVC %s does not exist\n", evc_cmd_evc_id_txt(evc_id, buf));
            }
            return;
        }
        /* Find EVC NNI ports */
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            iport_list[iport] = pb->nni[iport];
        }

        /* Find ECE UNI ports */
        ece->id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(ece->id, &ece_conf, 1) == VTSS_RC_OK) {
            if (ece->action.evc_id != evc_id) {
                continue;
            }
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                if (key->port_list[iport] != VTSS_ECE_PORT_NONE) {
                    iport_list[iport] = 1;
                }
            }
        }

        /* Show statistics */
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 || iport_list[iport] == 0) {
                continue;
            }
            if (evc_req->clear) {
                (void)vtss_evc_counters_clear(NULL, evc_id, iport);
            } else if (vtss_evc_counters_get(NULL, evc_id, iport, &counters) != VTSS_RC_OK) {
                CPRINTF("EVC statistics failed for EVC ID %s, port %u",
                        evc_cmd_evc_id_txt(evc_id, buf), uport);
            } else {
                evc_cmd_show_stats(req, evc_id, 0, uport, &header, &counters);
            }
        }
        if (!next) {
            break;
        }
    }
}

static void evc_cmd_ece_statistics(cli_req_t *req)
{
    evc_mgmt_conf_t     evc_conf;
    vtss_evc_pb_conf_t  *pb = &evc_conf.conf.network.pb;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_t          *ece = &ece_conf.conf;
    vtss_ece_key_t      *key = &ece->key;
    evc_cli_req_t       *evc_req = req->module_req;
    vtss_evc_id_t       evc_id;
    port_iter_t         pit;
    vtss_uport_no_t     uport;
    vtss_port_no_t      iport;
    vtss_evc_counters_t counters;
    BOOL                next, first, header = 1;

    if (evc_req->ece_id) {
        ece->id = evc_req->ece_id;
        next = 0;
    } else {
        ece->id = EVC_ECE_ID_FIRST;
        next = 1;
    }

    for (first = 1;
         (next || first) && evc_mgmt_ece_get(ece->id, &ece_conf, next) == VTSS_RC_OK;
         first = 0) {
        evc_id = ece->action.evc_id;
        if (evc_id == VTSS_EVC_ID_NONE || evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_RC_OK) {
            memset(&evc_conf, 0, sizeof(evc_conf));
        }

        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 ||
                (key->port_list[iport] == VTSS_ECE_PORT_NONE && pb->nni[iport] == 0)) {
                continue;
            }
            if (evc_req->clear) {
                (void)vtss_ece_counters_clear(NULL, ece->id, iport);
            } else if (vtss_ece_counters_get(NULL, ece->id, iport, &counters) != VTSS_RC_OK) {
                CPRINTF("ECE statistics failed for ECE ID %u, port %u", ece->id, uport);
            } else {
                evc_cmd_show_stats(req, VTSS_EVC_ID_NONE, ece->id, uport, &header, &counters);
            }
        }
    }
}
#else
/* Print counters in two columns with header */
static void evc_cmd_stat_port(vtss_uport_no_t uport, vtss_prio_t prio,
                              BOOL *header, const char *col1, const char *col2,
                              vtss_port_counter_t c1, vtss_port_counter_t c2)
{
    char buf[80], *p;

    if (*header) {
        *header = 0;
        p = &buf[0];
        p += sprintf(p, "Port  Class  Rx %-19s", col1);
        if (col2 != NULL) {
            if (strlen(col2) == 0)
                sprintf(p, "Tx %-19s", col1);
            else
                sprintf(p, "%-22s", col2);
        }
        cli_table_header(buf);
    }
    CPRINTF("%-6u%-7u%-22llu", uport, prio, c1);
    if (col2 != NULL)
        CPRINTF("%llu", c2);
    CPRINTF("\n");
}

/* Print two counters in columns */
static void evc_cmd_stats(const char *col1, BOOL col2, 
                          vtss_port_counter_t c1, vtss_port_counter_t c2)
{
    char buf[80];
    
    sprintf(buf, "%s:", col1);
    CPRINTF("Rx %-15s%20llu   ", buf, c1);
    if (col2) {
        CPRINTF("Tx %-15s%20llu", buf, c2);
    }
    CPRINTF("\n");
}

static void evc_cmd_evc_statistics(cli_req_t *req)
{
    vtss_port_counters_t     counters;
    vtss_port_evc_counters_t *evc = &counters.evc;
    port_iter_t              pit;
    vtss_uport_no_t          uport;
    vtss_port_no_t           iport;
    vtss_prio_t              prio;
    BOOL                     header = 1;
    evc_cli_req_t            *evc_req = req->module_req;

    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;
        
        if (evc_req->clear) {
            (void)vtss_port_counters_clear(NULL, iport);
            continue;
        } else if (vtss_port_counters_get(NULL, iport, &counters) != VTSS_RC_OK)
            continue;

        for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++) {
            if (evc_req->prio_list[prio] == 0)
                continue;
            if (evc_req->green) {
                evc_cmd_stat_port(uport, prio, &header,
                                  "Green", "", evc->rx_green[prio], evc->tx_green[prio]);
            } else if (evc_req->yellow) {
                evc_cmd_stat_port(uport, prio, &header,
                                  "Yellow", "", evc->rx_yellow[prio], evc->tx_yellow[prio]);
            } else if (evc_req->red) {
                evc_cmd_stat_port(uport, prio, &header, "Red", NULL, evc->rx_red[prio], 0);
            } else if (evc_req->discard) {
                evc_cmd_stat_port(uport, prio, &header, "Green Discard", "Rx Yellow Discard", 
                                  evc->rx_green_discard[prio], evc->rx_yellow_discard[prio]);
            } else {
                /* Detailed statistics */
                CPRINTF("%sPort %u, Priority %u Statistics:\n\n",
                        header ? "" : "\n", uport, prio);
                header = 0;
                evc_cmd_stats("Green", 1,  evc->rx_green[prio], evc->tx_green[prio]);
                evc_cmd_stats("Yellow", 1,  evc->rx_yellow[prio], evc->tx_yellow[prio]);
                evc_cmd_stats("Red", 0,  evc->rx_red[prio], 0);
                evc_cmd_stats("Green Discard", 0, evc->rx_green_discard[prio], 0);
                evc_cmd_stats("Yellow Discard", 0, evc->rx_yellow_discard[prio], 0);
            }
        }
    }
}
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
static void evc_cmd_evc_port_check(cli_req_t *req)
{
    evc_mgmt_global_conf_t conf;

    if (evc_mgmt_conf_get(&conf) != VTSS_OK) {
        CPRINTF("evc_mgmt_conf_get failed\n");
        return;
    }
    if (req->set) {
        conf.port_check = req->enable;
        (void)evc_mgmt_conf_set(&conf);
    } else {
        CPRINTF("Port Check: %s\n", cli_bool_txt(conf.port_check));
    }
}

static void evc_cmd_evc_mep(cli_req_t *req)
{
    evc_cli_req_t            *evc_req = req->module_req;
    vtss_evc_id_t            evc_id = evc_req->evc_id;
    port_iter_t              pit;
    vtss_port_no_t           iport;
    vtss_uport_no_t          uport;
    vtss_evc_oam_port_conf_t conf;
    BOOL                     header = 1;

    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;
        
        if (req->set) {
            conf.voe_idx = evc_req->voe_idx;
            (void)vtss_evc_oam_port_conf_set(NULL, evc_id, iport, &conf);
        } else if (vtss_evc_oam_port_conf_get(NULL, evc_id, iport, &conf) == VTSS_RC_OK) {
            if (header) {
                header = 0;
                cli_table_header("Port  VOE");
            }
            CPRINTF("%-6u", uport);
            if (conf.voe_idx == VTSS_OAM_VOE_IDX_NONE)
                CPRINTF(cli_bool_txt(0));
            else
                CPRINTF("%u", conf.voe_idx);
            CPRINTF("\n");
        }
    }
}

static void evc_cmd_mce_add(cli_req_t *req)
{
    evc_cli_req_t     *evc_req = req->module_req;
    vtss_mce_t        mce;
    vtss_mce_key_t    *key = &mce.key;
    vtss_mce_action_t *action = &mce.action;
    vtss_mce_tag_t    *tag;

    if (vtss_mce_init(NULL, &mce) != VTSS_OK) {
        CPRINTF("MCE init failed\n");
        return;
    }
    
    /* ID and ingress port */
    mce.id = evc_req->mce_id;
    if (evc_req->port_rx == VTSS_PORT_NO_CPU)
        key->port_cpu = 1;
    else if (evc_req->port_rx != VTSS_PORT_NO_NONE)
        key->port_list[evc_req->port_rx] = 1;
    
    /* Outer tag matching */
    tag = &key->tag;
    tag->tagged = evc_req->tag_type.tagged;
    tag->s_tagged = evc_req->tag_type.s_tagged;
    tag->vid.value = evc_req->vid.value.vr.v.value;
    tag->vid.mask = evc_req->vid.value.vr.v.mask;
    tag->pcp = evc_req->pcp.value;
    tag->dei = evc_req->dei.value;

    /* Inner tag matching */
    tag = &key->inner_tag;
    tag->tagged = evc_req->in_type.tagged;
    tag->s_tagged = evc_req->in_type.s_tagged;
    tag->vid.value = evc_req->in_vid.value.vr.v.value;
    tag->vid.mask = evc_req->in_vid.value.vr.v.mask;
    tag->pcp = evc_req->in_pcp.value;
    tag->dei = evc_req->in_dei.value;

    /* MEL and inject flag */
    if (evc_req->mel.valid) {
        key->mel.value = ((1 << evc_req->mel.value) - 1);
        key->mel.mask = 0x7f;
    }
    key->injected = evc_req->inject.value;
    key->lookup = evc_req->lookup;
    
    /* Action */
    if (evc_req->port_tx != VTSS_PORT_NO_NONE)
        action->port_list[evc_req->port_tx] = 1;
    action->voe_idx = evc_req->voe_idx;
    action->pop_cnt = evc_req->pop;
    action->policy_no = (evc_req->policy_valid ? evc_req->policy_no : VTSS_ACL_POLICY_NO_NONE);
    action->prio_enable = evc_req->prio_enable;
    action->prio = evc_req->prio;
    action->vid = evc_req->ivid.value;
    action->isdx = evc_req->isdx;
    action->tx_lookup = evc_req->mce_tx_lookup;
    action->oam_detect = evc_req->oam_detect;
    action->outer_tag.enable = req->enable;
    action->outer_tag.vid = evc_req->ot_vid.value;
    action->outer_tag.pcp_mode = evc_req->pcp_mode;
    action->outer_tag.dei_mode = evc_req->dei_mode;

    if (vtss_mce_add(NULL, evc_req->mce_id_next_valid ? evc_req->mce_id_next : VTSS_MCE_ID_LAST,
                     &mce) != VTSS_OK) {
        CPRINTF("MCE Add failed\n");
    }
}

static void evc_cmd_mce_del(cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    if (vtss_mce_del(NULL, evc_req->mce_id) != VTSS_OK) {
        CPRINTF("MCE Delete failed\n");
    }
}

static void evc_cmd_mce_get(cli_req_t *req)
{
    evc_cli_req_t        *evc_req = req->module_req;
    vtss_mce_port_info_t info;

    if (vtss_mce_port_info_get(NULL, evc_req->mce_id, evc_req->port_rx, &info) == VTSS_OK) {
        CPRINTF("ISDX: %u\n", info.isdx);
    }
}

static void evc_cmd_mce_key(cli_req_t *req)
{
    evc_cli_req_t         *evc_req = req->module_req;
    port_iter_t           pit;
    vtss_uport_no_t       uport;
    vtss_port_no_t        iport;
    vtss_vcap_port_conf_t conf;
    BOOL                  header = 1;

    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        iport = pit.iport;
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0 ||
            vtss_vcap_port_conf_get(NULL, iport, &conf) != VTSS_RC_OK) {
            continue;
        }
        
        if (req->set) {
            conf.key_type_is1_1 = evc_req->key_type;
            conf.dmac_dip_1 = FALSE;
            (void)vtss_vcap_port_conf_set(NULL, iport, &conf);
            continue;
        }
        if (header) {
            header = 0;
            cli_table_header("Port  Key Type");
        }
        CPRINTF("%-6u%s\n", uport, evc_key_type_txt(conf.key_type_is1_1));
    }
}

#include "evc_example.h"

static void evc_cmd_evc_test(cli_req_t *req)
{
    evc_example(req->value);
}
#endif /* VTSS_ARCH_SERVAL */

static void evc_cmd_conf_show(cli_req_t *req)
{
    if (req->all) {
        cli_header("EVC Configuration", 1);
    }
    req->all = 1;
    evc_cmd_port(req, EVC_PORT_FLAGS_ALL);
    evc_cmd_policer(req);
    evc_cmd_evc_lookup(req);
    evc_cmd_ece_lookup(req);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static void evc_cmd_default_set(cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    (void)cli_parm_parse_list(NULL, evc_req->l2cp_list, 0, L2CP_ID_COUNT - 1, 1);
    (void)cli_parm_parse_list(NULL, evc_req->prio_list, 0, VTSS_PRIOS - 1,  1);
}

static int evc_parse_any(char *cmd)
{
    return cli_parse_word(cmd, "any");
}

static int evc_parse_evc_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_ulong(cmd, &value, 1, EVC_ID_COUNT)) == 0) {
        evc_req->evc_id = (value - 1);
        evc_req->evc_id_valid = 1;
    }
    return error;
}

static int evc_parse_evc_id_none(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                 cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_none(cmd)) == 0) {
        evc_req->evc_id = VTSS_EVC_ID_NONE;
        evc_req->evc_id_valid = 1;
    } else {
        error = evc_parse_evc_id(cmd, cmd2, stx, cmd_org, req);
    }
    return error;
}

static int evc_parse_policer_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_ulong(cmd, &value, 1, EVC_POL_COUNT)) == 0) {
        evc_req->policer_id = (value - 1);
        evc_req->policer_id_valid = 1;
    }
    return error;
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static int evc_parse_evc_policer_id(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                    cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_none(cmd)) == 0) {
        evc_req->policer_id = VTSS_EVC_POLICER_ID_NONE;
        evc_req->policer_id_valid = 1;
    } else if ((error = cli_parse_word(cmd, "discard")) == 0) {
        evc_req->policer_id = VTSS_EVC_POLICER_ID_DISCARD;
        evc_req->policer_id_valid = 1;
    } else {
        error = evc_parse_policer_id(cmd, cmd2, stx, cmd_org, req);
    }
    return error;
}

static int evc_parse_ece_policer_id(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                    cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_word(cmd, "evc")) == 0) {
        evc_req->policer_id = VTSS_EVC_POLICER_ID_EVC;
        evc_req->policer_id_valid = 1;
    } else {
        error = evc_parse_evc_policer_id(cmd, cmd2, stx, cmd_org, req);
    }
    return error;
}
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
static int evc_parse_key_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "double_tag|normal|ip_addr|mac_ip_addr");
    evc_cli_req_t *evc_req = req->module_req;
    char          c;
    
    if (found != NULL) {
        c = *found;
        evc_req->key_type = (c == 'n' ? VTSS_VCAP_KEY_TYPE_NORMAL :
                             c == 'd' ? VTSS_VCAP_KEY_TYPE_DOUBLE_TAG :
                             c == 'i' ? VTSS_VCAP_KEY_TYPE_IP_ADDR :
                             VTSS_VCAP_KEY_TYPE_MAC_IP_ADDR);
    }
    return (found == NULL ? 1 : 0);
}
#endif /* VTSS_ARCH_SERVAL */

static int evc_parse_dei_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "coloured|fixed");
    evc_cli_req_t *evc_req = req->module_req;
    
    if (found != NULL && *found == 'c')
        evc_req->coloured = 1;
    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_CARACAL)
static int evc_parse_tag_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "inner|outer");
    evc_cli_req_t *evc_req = req->module_req;
    
    if (found != NULL && *found == 'i')
        evc_req->inner = 1;
    return (found == NULL ? 1 : 0);
}
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static int evc_parse_addr_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "source|destination");
    evc_cli_req_t *evc_req = req->module_req;
    
    if (found != NULL && *found == 'd')
        evc_req->dmac_dip = 1;
    return (found == NULL ? 1 : 0);
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

static int evc_parse_l2cp_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return cli_parm_parse_list(cmd, evc_req->l2cp_list, 0, L2CP_ID_COUNT - 1, 1);
}

static int evc_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, stx);
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "aware", 5)) {
            evc_req->aware = 1;
        } else if (!strncmp(found, "all", 3)) {
            evc_req->ece_type_valid = 1;
            evc_req->ece_type = VTSS_ECE_TYPE_ANY;
        } else if (!strncmp(found, "bytes", 5)) {
            evc_req->bytes = 1;
        } else if (!strncmp(found, "frames", 6)) {
            evc_req->frames = 1;
        } else if (!strncmp(found, "ipv4", 4)) {
            evc_req->ece_type_valid = 1;
            evc_req->ece_type = VTSS_ECE_TYPE_IPV4;
        } else if (!strncmp(found, "ipv6", 4)) {
            evc_req->ece_type_valid = 1;
            evc_req->ece_type = VTSS_ECE_TYPE_IPV6;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_fwd_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    char          *found = cli_parse_find(cmd, "copy|discard|forward|normal|redirect");
#else
    char          *found = cli_parse_find(cmd, "forward|normal|redirect");
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    evc_cli_req_t *evc_req = req->module_req;
    char          c;

    if (found != NULL) {
        c = *found;
        if (c == 'n') {
            evc_req->fwd_type = VTSS_PACKET_REG_NORMAL;
        } else if (c == 'f') {
            evc_req->fwd_type = VTSS_PACKET_REG_FORWARD;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        } else if (c == 'd') {
            evc_req->fwd_type = VTSS_PACKET_REG_DISCARD;
        } else if (c == 'c') {
            evc_req->fwd_type = VTSS_PACKET_REG_CPU_COPY;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
        } else if (c == 'r') {
            evc_req->fwd_type = VTSS_PACKET_REG_CPU_ONLY;
        }
    }
    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_JAGUAR_1)
#define EVC_POL_MODE_TXT "coupled|aware|blind"
#else
#define EVC_POL_MODE_TXT "coupled|aware"
#endif

static int evc_parse_pol_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, EVC_POL_MODE_TXT);
    evc_cli_req_t *evc_req = req->module_req;
    char          c;
    
    if (found != NULL) {
        c = *found;
        if (c == 'c')
            evc_req->coupled = 1;
        else if (c == 'a')
            evc_req->aware = 1;
        else
            evc_req->blind = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_rate(char *cmd, vtss_bitrate_t *rate, BOOL *valid)
{
    int error;

    if ((error = cli_parse_ulong(cmd, rate, 0, EVC_POLICER_RATE_MAX)) == 0) {
        *valid = 1;
    }
    return error;
}

static int evc_parse_level(char *cmd, vtss_burst_level_t *level, BOOL *valid)
{
    int error;

    if ((error = cli_parse_ulong(cmd, level, 0, EVC_POLICER_LEVEL_MAX)) == 0) {
        *valid = 1;
    }
    return error;
}

static int evc_parse_cir(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_rate(cmd, &evc_req->pol_conf.cir, &evc_req->cir_valid);
}

static int evc_parse_cbs(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_level(cmd, &evc_req->pol_conf.cbs, &evc_req->cbs_valid);
}

static int evc_parse_eir(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_rate(cmd, &evc_req->pol_conf.eir, &evc_req->eir_valid);
}

static int evc_parse_ebs(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_level(cmd, &evc_req->pol_conf.ebs, &evc_req->ebs_valid);
}

static int evc_parse_ece_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return cli_parse_ulong(cmd, &evc_req->ece_id, 1, EVC_ECE_COUNT);
}

static int evc_parse_ece_id_next(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                 cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_word(cmd, "last")) == 0) {
        evc_req->ece_id_next = VTSS_ECE_ID_LAST;
        evc_req->ece_id_next_valid = 1;
    } else if ((error = cli_parse_ulong(cmd, &evc_req->ece_id_next, 1, EVC_ECE_COUNT)) == 0) {
        evc_req->ece_id_next_valid = 1;
    }
    return error;
}

static int evc_parse_uni_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parm_parse_list(cmd, req->uport_list, 1, VTSS_PORTS, 1)) == 0) {
        evc_req->uni_list_valid = 1;
    }
    return error;
}

static int evc_parse_nni_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parm_parse_list(cmd, req->uport_list, 1, VTSS_PORTS, 0)) == 0 ||
        (error = cli_parse_none(cmd)) == 0) {
        evc_req->nni_list_valid = 1;
    }

    return error;
}

static int evc_parse_bool(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, "enable|disable");

    if (found != NULL) {
        if (*found == 'e') {
            req->enable = 1;
        } else {
            req->disable = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_vid(char *cmd, evc_cli_vid_t *vid)
{
    int   error;
    ulong value;

    if ((error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX)) == 0) {
        vid->value = value;
        vid->valid = 1;
    }
    return error;
}

static int evc_parse_evc_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid(cmd, &evc_req->evc_vid);
}

static int evc_parse_ivid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid(cmd, &evc_req->ivid);
}

static int evc_parse_it_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid(cmd, &evc_req->it_vid);
}

static int evc_parse_ot_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid(cmd, &evc_req->ot_vid);
}

static int evc_parse_range(char *cmd, evc_cli_range_t *range, u32 min, u32 max, BOOL no_range)
{
    int   error;
    ulong low, high, mask;

    if ((error = evc_parse_any(cmd)) == 0) {
        range->valid = 1;
    } else if ((error = cli_parse_range(cmd, &low, &high, min, max)) == 0) {
        /* Check if range can be done using value/mask (instead of range checker).
           For PCP, inner VID and all Jaguar-1 ranges, it is an error if value/mask 
           can not be used */
#if defined(VTSS_ARCH_JAGUAR_1)
        no_range = 1;
#endif /* VTSS_ARCH_JAGUAR_1 */
        if (no_range)
            error = 1;
        for (mask = 0; mask <= max; mask = (mask * 2 + 1)) {
            if ((low & ~mask) == (high & ~mask) && /* Upper bits match */
                (low & mask) == 0 &&                /* Lower bits of 'low' are zero */
                (high | mask) == high) {            /* Lower bits of 'high are one */
                error = 0;
                break;
            }
        }
        
        if (!error) {
            if (low != min || high != max) {
                range->value.type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
                range->value.vr.r.low = low;
                range->value.vr.r.high = high;
            }
            range->valid = 1;
        }
    }
    return error;
}

static int evc_parse_vid_range(char *cmd, evc_cli_range_t *vid, BOOL no_range)
{
    return evc_parse_range(cmd, vid, 0, VLAN_ID_MAX, no_range);
}

static int evc_parse_outer_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid_range(cmd, &evc_req->vid, 0);
}

static int evc_parse_inner_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_vid_range(cmd, &evc_req->in_vid, 1);
}

static int evc_parse_pcp_range(char *cmd, evc_cli_vcap_u8_t *pcp)
{
    int             error;
    evc_cli_range_t range;
    
    /* Use generic range parser and convert to value/mask */
    memset(&range, 0, sizeof(range));
    if ((error = evc_parse_range(cmd, &range, 0, 7, 1)) == 0) {
        if (range.value.type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE) {
            pcp->value.value = range.value.vr.r.low;
            pcp->value.mask = (7 - range.value.vr.r.high + range.value.vr.r.low);
        }
        pcp->valid = 1;
    }
    return error;
}

static int evc_parse_outer_pcp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_pcp_range(cmd, &evc_req->pcp);
}

static int evc_parse_inner_pcp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_pcp_range(cmd, &evc_req->in_pcp);
}

static int evc_parse_dei(char *cmd, evc_cli_vcap_t *dei)
{
    int   error;
    ulong value;

    if ((error = evc_parse_any(cmd)) == 0) {
        dei->valid = 1;
    } else if ((error = cli_parse_ulong(cmd, &value, 0, 1)) == 0) {
        dei->value = (value ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0);
        dei->valid = 1;
    }
    return error;
}

static int evc_parse_outer_dei(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_dei(cmd, &evc_req->dei);
}

static int evc_parse_inner_dei(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_dei(cmd, &evc_req->in_dei);
}

static int evc_parse_dmac_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "any|unicast|multicast|broadcast");
    evc_cli_req_t *evc_req = req->module_req;
    char          c;
    if (found != NULL) {
        c = *found;
        if (c == 'a') {
            evc_req->dmac_mc = VTSS_VCAP_BIT_ANY;
            evc_req->dmac_bc = VTSS_VCAP_BIT_ANY;
        } else if (c == 'u') {
            evc_req->dmac_mc = VTSS_VCAP_BIT_0;
            evc_req->dmac_bc = VTSS_VCAP_BIT_0;
        } else if (c == 'm') {
            evc_req->dmac_mc = VTSS_VCAP_BIT_1;
            evc_req->dmac_bc = VTSS_VCAP_BIT_0;
        } else {
            evc_req->dmac_mc = VTSS_VCAP_BIT_1;
            evc_req->dmac_bc = VTSS_VCAP_BIT_1;
        }
        evc_req->dmac_type_valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_mac(char *cmd, evc_cli_vcap_u48_t *mac)
{
    int        error;
    cli_spec_t spec;
    int        i;

    if ((error = cli_parse_mac(cmd, mac->value.value, &spec, 1)) == 0) {
        for (i = 0; i < 6; i++)
            mac->value.mask[i] = (spec == CLI_SPEC_ANY ? 0 : 0xff);
        mac->valid = 1;
    }
    return error;
}

static int evc_parse_smac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_mac(cmd, &evc_req->smac);
}

static int evc_parse_dmac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_mac(cmd, &evc_req->dmac);
}

/* Tag type string */
#define EVC_TAG_TYPE_TXT "untagged|tagged|c-tagged|s-tagged|any"

static int evc_parse_tag_type(char *cmd, evc_cli_tag_type_t *type)
{
    char *found = cli_parse_find(cmd, EVC_TAG_TYPE_TXT);

    if (found != NULL) {
        type->tagged = (*found == 'a' ? VTSS_VCAP_BIT_ANY :
                        *found == 'u' ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_1);
        type->s_tagged = (*found == 's' ? VTSS_VCAP_BIT_1 :
                          *found == 'c' ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
        type->valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_outer_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_tag_type(cmd, &evc_req->tag_type);
}

static int evc_parse_inner_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_tag_type(cmd, &evc_req->in_type);
}

static int evc_parse_proto(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = evc_parse_any(cmd)) == 0) {
        evc_req->proto.valid = 1;
    } else if ((error = cli_parse_ulong(cmd, &value, 0, 255)) == 0) {
        evc_req->proto.value.mask = 0xff;
        evc_req->proto.value.value = value;
        evc_req->proto.valid = 1;
    }
    return error;
}

static int evc_parse_ip(char *cmd, evc_cli_vcap_ip_t *ip)
{
    int        error, i, j;
    cli_spec_t spec;
    
    if ((error = cli_parse_ipv4(cmd, &ip->ipv4.value, &ip->ipv4.mask, &spec, 0)) == 0) {
        for (i = 0; i < 4; i++) {
            j = ((3 - i) * 8);
            ip->ipv6.value[i + 12] = ((ip->ipv4.value >> j) & 0xff);
            ip->ipv6.mask[i + 12] = ((ip->ipv4.mask >> j) & 0xff);
        }
        ip->valid = 1;
    }
    return error;
}

static int evc_parse_sip(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_ip(cmd, &evc_req->sip);
}

static int evc_parse_dip(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_ip(cmd, &evc_req->dip);
}

static int evc_parse_dscp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_range(cmd, &evc_req->dscp, 0, 63, 0);
}

static int evc_parse_fragment(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "any|fragment|non-fragment");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->fragment.value = (*found == 'f' ? VTSS_VCAP_BIT_1 :
                                   *found == 'n' ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
        evc_req->fragment.valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_sdport(char *cmd, cli_req_t *req, BOOL sport)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_range(cmd, sport ? &evc_req->sport : &evc_req->dport, 0, 0xffff, 0);
}

static int evc_parse_sport(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    return evc_parse_sdport(cmd, req, 1);
}

static int evc_parse_dport(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    return evc_parse_sdport(cmd, req, 0);
}

static int evc_parse_dir(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "both|uni-to-nni|nni-to-uni");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->dir = (*found == 'b' ? VTSS_ECE_DIR_BOTH :
                        *found == 'u' ? VTSS_ECE_DIR_UNI_TO_NNI : VTSS_ECE_DIR_NNI_TO_UNI);
        evc_req->dir_valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_SERVAL)
static int evc_parse_rule_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "both|rx|tx");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->rule = (*found == 'b' ? VTSS_ECE_RULE_BOTH :
                         *found == 'r' ? VTSS_ECE_RULE_RX : VTSS_ECE_RULE_TX);
        evc_req->rule_valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_tx_lookup(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "vid|pcp_vid|isdx");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->tx_lookup = (*found == 'v' ? VTSS_ECE_TX_LOOKUP_VID :
                              *found == 'p' ? VTSS_ECE_TX_LOOKUP_VID_PCP : 
                              VTSS_ECE_TX_LOOKUP_ISDX);
        evc_req->tx_lookup_valid = 1;
    }
    return (found == NULL ? 1 : 0);
}
#endif /* VTSS_ARCH_SERVAL */

static int evc_parse_policy(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_ulong(cmd, &evc_req->policy_no, 0, VTSS_ACL_POLICIES - 1)) == 0)
        evc_req->policy_valid = 1;
    return error;
}

static int evc_parse_class_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return cli_parm_parse_list(cmd, evc_req->prio_list, 0, VTSS_PRIOS - 1, 1);
}

static int evc_parse_class_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_disable(cmd)) == 0)
        evc_req->prio_valid = 1;
    else if ((error = cli_parse_ulong(cmd, &value, 0, VTSS_PRIOS - 1)) == 0) {
        evc_req->prio_valid = 1;
        evc_req->prio_enable = 1;
        evc_req->prio = value;
    }
    return error;
}

static int evc_parse_dp_dis(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_disable(cmd)) == 0)
        evc_req->dp_valid = 1;
    else if ((error = cli_parse_ulong(cmd, &value, 0, 1)) == 0) {
        evc_req->dp_valid = 1;
        evc_req->dp_enable = 1;
        evc_req->dp = value;
    }
    return error;
}

static int evc_parse_tag_pop(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_ulong(cmd, &value, 0, 2)) == 0) {
        evc_req->pop_valid = 1;
        evc_req->pop = (value == 2 ? VTSS_ECE_POP_TAG_2 :
                        value == 1 ? VTSS_ECE_POP_TAG_1 : VTSS_ECE_POP_TAG_0);
    }
    return error;
}

static int evc_parse_ot_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "enable|disable");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->ot_mode.value = (*found == 'e' ? 1 : 0);
        evc_req->ot_mode.valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_pcp_mode(char *cmd, evc_cli_pcp_mode_t *mode)
{
#if defined(VTSS_ARCH_SERVAL)
    char *found = cli_parse_find(cmd, "classified|fixed|mapped");
#else
    char *found = cli_parse_find(cmd, "preserved|fixed");
#endif /* VTSS_ARCH_SERVAL */

    if (found != NULL) {
#if defined(VTSS_ARCH_SERVAL)
        mode->value = (*found == 'c' ? VTSS_ECE_PCP_MODE_CLASSIFIED :
                       *found == 'f' ? VTSS_ECE_PCP_MODE_FIXED : VTSS_ECE_PCP_MODE_MAPPED);
#else
        mode->value = (*found == 'p' ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */
        mode->valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_ot_pcp_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_pcp_mode(cmd, &evc_req->ot_pcp_mode);
}

static int evc_parse_it_pcp_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_pcp_mode(cmd, &evc_req->it_pcp_mode);
}

static int evc_parse_pcp(char *cmd, evc_cli_pcp_t *pcp)
{
    int   error;
    ulong value;

    if ((error = cli_parse_ulong(cmd, &value, 0, 7)) == 0) {
        pcp->value = value;
        pcp->valid = 1;
    }
    return error;
}

static int evc_parse_ot_pcp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_pcp(cmd, &evc_req->ot_pcp);
}

static int evc_parse_it_pcp(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_pcp(cmd, &evc_req->it_pcp);
}

#if defined(VTSS_ARCH_SERVAL)
static int evc_parse_ece_dei_mode(char *cmd, evc_cli_dei_mode_t *mode)
{
    char *found = cli_parse_find(cmd, "classified|fixed|dp");

    if (found != NULL) {
        mode->value = (*found == 'c' ? VTSS_ECE_DEI_MODE_CLASSIFIED :
                       *found == 'f' ? VTSS_ECE_DEI_MODE_FIXED : VTSS_ECE_DEI_MODE_DP);
        mode->valid = 1;
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_ot_dei_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_ece_dei_mode(cmd, &evc_req->ot_dei_mode);
}

static int evc_parse_it_dei_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_ece_dei_mode(cmd, &evc_req->it_dei_mode);
}
#endif /* VTSS_ARCH_SERVAL */

static int evc_parse_tag_dei(char *cmd, evc_cli_bool_t *dei)
{
    int   error;
    ulong value;

    if ((error = cli_parse_ulong(cmd, &value, 0, 1)) == 0) {
        dei->value = value;
        dei->valid = 1;
    }
    return error;
}

static int evc_parse_ot_dei(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_tag_dei(cmd, &evc_req->ot_dei);
}

static int evc_parse_it_dei(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return evc_parse_tag_dei(cmd, &evc_req->it_dei);
}

static int evc_parse_it_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "none|c-tag|s-tag|s-custom-tag");
    evc_cli_req_t *evc_req = req->module_req;
    char          c;

    if (found != NULL) {
        c = *found;
        evc_req->it_type_valid = 1;
        if (c == 'n') {
#if defined(VTSS_ARCH_CARACAL)
            evc_req->it_type = VTSS_EVC_INNER_TAG_NONE;
#else
            evc_req->it_type = VTSS_ECE_INNER_TAG_NONE;
#endif /* VTSS_ARCH_CARACAL */
        } else if (c == 'c') {
#if defined(VTSS_ARCH_CARACAL)
            evc_req->it_type = VTSS_EVC_INNER_TAG_C;
#else
            evc_req->it_type = VTSS_ECE_INNER_TAG_C;
#endif /* VTSS_ARCH_CARACAL */
        } else if (found[2] == 't') {
#if defined(VTSS_ARCH_CARACAL)
            evc_req->it_type = VTSS_EVC_INNER_TAG_S;
#else
            evc_req->it_type = VTSS_ECE_INNER_TAG_S;
#endif /* VTSS_ARCH_CARACAL */
        } else {
#if defined(VTSS_ARCH_CARACAL)
            evc_req->it_type = VTSS_EVC_INNER_TAG_S_CUSTOM;
#else
            evc_req->it_type = VTSS_ECE_INNER_TAG_S_CUSTOM;
#endif /* VTSS_ARCH_CARACAL */
        }
    }
    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_CARACAL)
static int evc_parse_vid_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "normal|tunnel");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->it_vid_mode_valid = 1;
        evc_req->it_vid_mode = (*found == 'n' ? VTSS_EVC_VID_MODE_NORMAL : 
                                VTSS_EVC_VID_MODE_TUNNEL);
    }
    return (found == NULL ? 1 : 0);
}
#endif /* VTSS_ARCH_CARACAL */

static int evc_parse_command(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "clear|green|yellow|red|discard");
    evc_cli_req_t *evc_req = req->module_req;
    char          c;

    if (found != NULL) {
        c = *found;
        if (c == 'c') {
            evc_req->clear = 1;
        } else if (c == 'g') {
            evc_req->green = 1;
        } else if (c == 'y') {
            evc_req->yellow = 1;
        } else if (c == 'r') {
            evc_req->red = 1;
        } else if (c == 'd') {
            evc_req->discard = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_SERVAL)
static int evc_parse_voe_idx(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;
    
    if ((error = cli_parse_disable(cmd)) == 0)
        evc_req->voe_idx = VTSS_OAM_VOE_IDX_NONE;
    else 
        error = cli_parse_ulong(cmd, &evc_req->voe_idx, 0, VTSS_OAM_PATH_SERVICE_VOE_CNT - 1);
    return error;
}

static int evc_parse_mce_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;

    return cli_parse_ulong(cmd, &evc_req->mce_id, 1, 128);
}

static int evc_parse_mce_id_next(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                 cli_req_t *req)
{
    int           error;
    evc_cli_req_t *evc_req = req->module_req;

    if ((error = cli_parse_word(cmd, "last")) == 0) {
        evc_req->mce_id_next = VTSS_MCE_ID_LAST;
        evc_req->mce_id_next_valid = 1;
    } else if ((error = cli_parse_ulong(cmd, &evc_req->mce_id_next, 1, 128)) == 0) {
        evc_req->mce_id_next_valid = 1;
    }
    return error;
}

static int evc_parse_port(char *cmd, vtss_port_no_t *iport, BOOL cpu)
{
    int             error;
    vtss_uport_no_t uport;

    if ((error = cli_parse_none(cmd)) == 0) {
        *iport = VTSS_PORT_NO_NONE;
    } else if (cpu && (error = cli_parse_cpu(cmd)) == 0) {
        *iport = VTSS_PORT_NO_CPU;
    } else if ((error = cli_parse_ulong(cmd, &uport, 1, port_isid_port_count(VTSS_ISID_LOCAL))) == 0) {
        *iport = uport2iport(uport);
    }
    return error;
}

static int evc_parse_port_rx(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_port(cmd, &evc_req->port_rx, 1);
}

static int evc_parse_port_tx(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_port(cmd, &evc_req->port_tx, 0);
}

static int evc_parse_mce_vid(char *cmd, evc_cli_range_t *vid)
{
    int error;
    vtss_vcap_vr_value_t value, mask;

    if ((error = evc_parse_vid_range(cmd, vid, 1)) == 0) {
        if (vid->value.type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE) {
            value = vid->value.vr.r.low;
            mask = (4095 - vid->value.vr.r.high + vid->value.vr.r.low);
            vid->value.vr.v.value = value;
            vid->value.vr.v.value = mask;
        }
    }
    return error;
}

static int evc_parse_mce_outer_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_mce_vid(cmd, &evc_req->vid);
}

static int evc_parse_mce_inner_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_mce_vid(cmd, &evc_req->in_vid);
}

static int evc_parse_mce_pcp_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    char          *found = cli_parse_find(cmd, "fixed|mapped");

    if (found != NULL) {
        evc_req->pcp_mode = (*found == 'f' ? VTSS_MCE_PCP_MODE_FIXED : VTSS_MCE_PCP_MODE_MAPPED);
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_mce_dei_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    char          *found = cli_parse_find(cmd, "fixed|dp");

    if (found != NULL) {
        evc_req->dei_mode = (*found == 'f' ? VTSS_MCE_DEI_MODE_FIXED : VTSS_MCE_DEI_MODE_DP);
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_mce_tx_lookup(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "vid|isdx|pcp_isdx");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->mce_tx_lookup = (*found == 'v' ? VTSS_MCE_TX_LOOKUP_VID :
                                  *found == 'i' ? VTSS_MCE_TX_LOOKUP_ISDX :
                                  VTSS_MCE_TX_LOOKUP_ISDX_PCP);
    }
    return (found == NULL ? 1 : 0);
}

static int evc_parse_isdx(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    u32           value;
    evc_cli_req_t *evc_req = req->module_req;
    
    if ((error = cli_parse_ulong(cmd, &value, 0, 0xffffffff)) == 0) {
        if (value < 1024 || value == VTSS_MCE_ISDX_NEW || value == VTSS_MCE_ISDX_NONE)
            evc_req->isdx = value;
        else
            error = 1;
    }
    return error;
}

static int evc_parse_mel(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int           error;
    ulong         value;
    evc_cli_req_t *evc_req = req->module_req;
    
    if ((error = cli_parse_ulong(cmd, &value, 0, 7)) == 0) {
        evc_req->mel.value = value;
        evc_req->mel.valid = 1;
    }
    return error;
}

static int evc_parse_inject(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return evc_parse_dei(cmd, &evc_req->inject);
}

static int evc_parse_lookup(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    evc_cli_req_t *evc_req = req->module_req;
    
    return cli_parse_ulong(cmd, &evc_req->lookup, 0, 1);
}

static int evc_parse_oam_detect(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char          *found = cli_parse_find(cmd, "untagged|single|double");
    evc_cli_req_t *evc_req = req->module_req;

    if (found != NULL) {
        evc_req->oam_detect = (*found == 'u' ? VTSS_MCE_OAM_DETECT_UNTAGGED :
                               *found == 's' ?  VTSS_MCE_OAM_DETECT_SINGLE_TAGGED :
                               VTSS_MCE_OAM_DETECT_DOUBLE_TAGGED);
    }
    return (found == NULL ? 1 : 0);
}

int evc_parse_evc_cmd(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
}
#endif /* VTSS_ARCH_SERVAL */

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t evc_parm_table[] = {
#if defined(VTSS_ARCH_SERVAL)
    {
        "<key_type>",
        "The key type takes the following values:\n"
        "double_tag : Quarter key, match inner and outer tag.\n"
        "normal     : Half key, match outer tag, SIP and SMAC.\n"
        "ip_addr    : Half key, match inner and outer tag, SIP and DIP.\n"
        "             For non-IP frames, match outer tag only.\n"
        "mac_ip_addr: Full key, match inner and outer tag, SMAC, DMAC, SIP and DIP",
        CLI_PARM_FLAG_SET,
        evc_parse_key_type,
        NULL
    },
#endif /* VTSS_ARCH_SERVAL */
    {
        "<dei_mode>",
        "DEI mode: coloured|fixed",
        CLI_PARM_FLAG_SET,
        evc_parse_dei_mode,
        NULL
    },
#if defined(VTSS_ARCH_CARACAL)
    {
        "<tag_mode>",
        "Tag mode: inner|outer",
        CLI_PARM_FLAG_SET,
        evc_parse_tag_mode,
        NULL,
    },
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    {
        "<addr_mode>",
        "IP/MAC address mode: source|destination",
        CLI_PARM_FLAG_SET,
        evc_parse_addr_mode,
        NULL,
    },
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    {
        "<l2cp_list>",
        "L2CP ID list (0-31). BPDU range: 0-15, GARP range: 16-31",
        CLI_PARM_FLAG_NONE,
        evc_parse_l2cp_id,
        NULL
    },
    {
        "<mode>",
        "The mode takes the following values:\n"
        "normal     : Default forwarding\n"
        "forward    : Forward\n"
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        "discard    : Discard\n"
        "copy       : Forward and copy to CPU\n"
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
        "redirect   : Redirect to CPU",
        CLI_PARM_FLAG_SET,
        evc_parse_fwd_type,
        NULL
    },
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        "<policer_id>",
        "EVC policer ID (1-"vtss_xstr(EVC_POL_COUNT)") or 'none' or 'discard'",
        CLI_PARM_FLAG_NONE,
        evc_parse_evc_policer_id,
        evc_cmd_evc_add
    },
    {
        "<policer_id>",
        "ECE policer ID (1-"vtss_xstr(EVC_POL_COUNT)") or 'none' or 'discard' or 'evc'",
        CLI_PARM_FLAG_NONE,
        evc_parse_ece_policer_id,
        evc_cmd_ece_add
    },
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    {
        "<policer_id>",
        "Policer ID (1-"vtss_xstr(EVC_POL_COUNT)")",
        CLI_PARM_FLAG_NONE,
        evc_parse_policer_id,
        NULL
    },
    {
        "enable|disable",
        "enable        : Enable policer\n"
        "disable       : Disable policer",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        evc_cmd_policer
    },
    {
        "<policer_mode>",
        "Policer_mode: "EVC_POL_MODE_TXT,
        CLI_PARM_FLAG_SET,
        evc_parse_pol_mode,
        NULL
    },
    {
        "<cir>",
        "Committed Information Rate [kbps]",
        CLI_PARM_FLAG_SET,
        evc_parse_cir,
        NULL
    },
    {
        "<cbs>",
        "Committed Burst Size [bytes]",
        CLI_PARM_FLAG_SET,
        evc_parse_cbs,
        NULL
    },
    {
        "<eir>",
        "Excess Information Rate [kbps]",
        CLI_PARM_FLAG_SET,
        evc_parse_eir,
        NULL
    },
    {
        "<ebs>",
        "Excess Burst Size [bytes]",
        CLI_PARM_FLAG_SET,
        evc_parse_ebs,
        NULL
    },
    {
        "<evc_id>",
        "EVC ID (1-"vtss_xstr(EVC_ID_COUNT)") or 'none'",
        CLI_PARM_FLAG_NONE,
        evc_parse_evc_id_none,
        evc_cmd_ece_add
    },
    {
        "<evc_id>",
        "EVC ID (1-"vtss_xstr(EVC_ID_COUNT)")",
        CLI_PARM_FLAG_NONE,
        evc_parse_evc_id,
        NULL
    },
    {
        "<direction>",
        "ECE direction: both|uni-to-nni|nni-to-uni",
        CLI_PARM_FLAG_NONE,
        evc_parse_dir,
        NULL
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "<rule_type>",
        "ECE rule type: both|rx|tx",
        CLI_PARM_FLAG_NONE,
        evc_parse_rule_type,
        NULL
    },
    {
        "<tx_lookup>",
        "ECE Tx lookup: vid|pcp_vid|isdx",
        CLI_PARM_FLAG_NONE,
        evc_parse_tx_lookup,
        evc_cmd_ece_add
    },
#endif /* VTSS_ARCH_SERVAL */
    {
        "<vid>",
        "EVC VLAN ID",
        CLI_PARM_FLAG_NONE,
        evc_parse_evc_vid,
        evc_cmd_evc_add
    },
    {
        "<ivid>",
        "Internal VLAN ID",
        CLI_PARM_FLAG_NONE,
        evc_parse_ivid,
        NULL
    },
    {
        "<nni_list>",
        "NNI port list (1-"vtss_xstr(VTSS_PORTS)") or 'none'",
        CLI_PARM_FLAG_SET,
        evc_parse_nni_list,
        NULL
    },
    {
        "<learning>",
        "Learning mode: enable|disable",
        CLI_PARM_FLAG_SET,
        evc_parse_bool,
        NULL
    },
    {
        "<ece_id>",
        "ECE ID (1-"vtss_xstr(EVC_ECE_COUNT)")",
        CLI_PARM_FLAG_NONE,
        evc_parse_ece_id,
        NULL
    },
    {
        "<ece_id_next>",
        "Next ECE ID (1-"vtss_xstr(EVC_ECE_COUNT)") or 'last'",
        CLI_PARM_FLAG_NONE,
        evc_parse_ece_id_next,
        NULL
    },
    {
        "uni",
        "UNI keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<uni_list>",
        uni_list_txt,
        CLI_PARM_FLAG_NONE,
        evc_parse_uni_list,
        NULL
    },
    {
        "<dmac_type>",
        "DMAC type: any|unicast|multicast|broadcast",
        CLI_PARM_FLAG_NONE,
        evc_parse_dmac_type,
        NULL
    },
    {
        "<smac>",
        "SMAC or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_smac,
        NULL
    },
    {
        "<dmac>",
        "SMAC or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_dmac,
        NULL
    },
    {
        "tag",
        "Tag matching keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "intag",
        "Inner tag matching keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<tag_type>",
        "Outer tag type: " EVC_TAG_TYPE_TXT,
        CLI_PARM_FLAG_NONE,
        evc_parse_outer_type,
        NULL
    },
    {
        "<vid>",
        "Outer tag VLAN ID value/range (0-4095) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_outer_vid,
        evc_cmd_ece_add
    },
    {
        "<pcp>",
        "Outer tag PCP value/range (0-7) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_outer_pcp,
        NULL
    },
    {
        "<dei>",
        "Outer tag DEI value, 0, 1 or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_outer_dei,
        NULL
    },
    {
        "<in_type>",
        "Inner tag type: " EVC_TAG_TYPE_TXT,
        CLI_PARM_FLAG_NONE,
        evc_parse_inner_type,
        NULL
    },
    {
        "<in_vid>",
        "Inner tag VLAN ID value/range (0-4095) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_inner_vid,
        NULL
    },
    {
        "<in_pcp>",
        "Inner tag PCP value/range (0-7) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_inner_pcp,
        NULL
    },
    {
        "<in_dei>",
        "Inner tag DEI value, 0, 1 or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_inner_dei,
        NULL
    },
    {
        "all",
        "Keyword for matching any frame type",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "ipv4",
        "Keyword for matching IPv4 frames",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "ipv6",
        "Keyword for matching IPv6 frames",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "direction",
        "Direction keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "rule_type",
        "Rule type keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "tx_lookup",
        "Tx lookup",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
#endif /* VTSS_ARCH_SERVAL */
    {
        "evc",
        "EVC keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "policy",
        "Policy keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "class",
        "Class keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "dp",
        "Drop Precedence keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "pop",
        "Pop keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "outer",
        "Outer tag action keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "inner",
        "Inner tag action keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<proto>",
        "IP protocol value (0-255) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_proto,
        NULL,
    },
    {
        "<sip>",
        "IPv4 source address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_sip,
        NULL,
    },
    {
        "<dip>",
        "IPv4 destination address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_dip,
        NULL,
    },
    {
        "<sip_v6>",
        "IPv6 source address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_sip,
        NULL,
    },
    {
        "<dip_v6>",
        "IPv6 destination address (a.b.c.d/n) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_dip,
        NULL,
    },
    {
        "<dscp>",
        "DSCP value/range (0-63) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_dscp,
        NULL,
    },
    {
        "<fragment>",
        "IPv4 fragment: any|fragment|non-fragment",
        CLI_PARM_FLAG_NONE,
        evc_parse_fragment,
        NULL,
    },
    {
        "<sport>",
        "UDP/TCP source port value/range (0-65535) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_sport,
        NULL,
    },
    {
        "<dport>",
        "UDP/TCP destination port value/range (0-65535) or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_dport,
        NULL,
    },
    {
        "<policy>",
        "ACL policy number (0-"vtss_xstr(ACL_POLICIES)")",
        CLI_PARM_FLAG_NONE,
        evc_parse_policy,
        NULL,
    },
    {
        "<class>",
        "QoS class, 'disable' or 0-7",
        CLI_PARM_FLAG_NONE,
        evc_parse_class_dis,
        NULL,
    },
    {
        "<class_list>",
        "QoS class list, 0-7",
        CLI_PARM_FLAG_NONE,
        evc_parse_class_list,
        NULL,
    },
    {
        "<dp>",
        "Drop Precedence, 'disable' or 0-1",
        CLI_PARM_FLAG_NONE,
        evc_parse_dp_dis,
        evc_cmd_ece_add,
    },
    {
        "<pop>",
        "Tag pop count: 0|1|2",
        CLI_PARM_FLAG_NONE,
        evc_parse_tag_pop,
        NULL,
    },
    {
        "<ot_mode>",
        "Outer tag for nni-to-uni direction: enable|disable",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_mode,
        evc_cmd_ece_add,
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "<ot_pcp_mode>",
        "Outer tag PCP mode: classified|fixed|mapped",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_pcp_mode,
        NULL,
    },
    {
        "<ot_dei_mode>",
        "Outer tag DEI mode: classified|fixed|dp",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_dei_mode,
        evc_cmd_ece_add,
    },
#else
    {
        "<ot_preserve>",
        "Outer tag PCP/DEI: preserved|fixed",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_pcp_mode,
        NULL,
    },
#endif /* VTSS_ARCH_SERVAL */
    {
        "<ot_vid>",
        "EVC outer tag VID for UNI ports",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_vid,
#if defined(VTSS_ARCH_CARACAL)
        evc_cmd_evc_add
#else
        evc_cmd_ece_add
#endif /* VTSS_ARCH_CARACAL */
    },
    {
        "<ot_pcp>",
        "Outer tag PCP value (0-7)",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_pcp,
        NULL,
    },
    {
        "<ot_dei>",
        "Outer tag DEI value (0-1)",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_dei,
        NULL,
    },
    {
        "<it_type>",
        "Inner tag type: none|c-tag|s-tag|s-custom-tag",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_type,
        NULL,
    },
#if defined(VTSS_ARCH_CARACAL)
    {
        "<it_vid_mode>",
        "Inner VID mode: normal|tunnel",
        CLI_PARM_FLAG_NONE,
        evc_parse_vid_mode,
        NULL,
    },
#endif /* VTSS_ARCH_CARACAL */
    {
        "<it_vid>",
        "Inner tag VLAN ID (1-4095)",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_vid,
        NULL,
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "<it_pcp_mode>",
        "Inner tag PCP mode: classified|fixed|mapped",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_pcp_mode,
        NULL,
    },
    {
        "<it_dei_mode>",
        "Inner tag DEI mode: classified|fixed|dp",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_dei_mode,
        NULL,
    },
#else
    {
        "<it_preserve>",
        "Inner tag preserved or fixed PCP/DEI: preserved|fixed",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_pcp_mode,
        NULL,
    },
#endif /* VTSS_ARCH_SERVAL */
    {
        "<it_pcp>",
        "Inner tag PCP value (0-7)",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_pcp,
        NULL,
    },
    {
        "<it_dei>",
        "Inner tag DEI value (0-1)",
        CLI_PARM_FLAG_NONE,
        evc_parse_it_dei,
        NULL,
    },
    {
        "<command>",
        "Statistics command: clear|green|yellow|red|discard",
        CLI_PARM_FLAG_NONE,
        evc_parse_command,
        NULL,
    },
    {
        "frames|bytes",
        "Show frame or byte statistics",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
#if defined(VTSS_ARCH_SERVAL)
    {
        "<voe_idx>",
        "MEP VOE index or 'disable'",
        CLI_PARM_FLAG_SET,
        evc_parse_voe_idx,
        NULL
    },
    {
        "<mce_id>",
        "MCE ID (1-128)",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_id,
        NULL
    },
    {
        "<mce_id_next>",
        "Next MCE ID (1-128) or 'last'",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_id_next,
        NULL
    },
    {
        "<port_rx>",
        "Ingress port number or 'none' or 'cpu'",
        CLI_PARM_FLAG_SET,
        evc_parse_port_rx,
        NULL
    },
    {
        "<vid>",
        "Outer tag VLAN ID (0-4095) value/range or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_outer_vid,
        NULL
    },
    {
        "<in_vid>",
        "Inner tag VLAN ID (0-4095) value/range or 'any'",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_inner_vid,
        NULL
    },
    {
        "mel",
        "MEL keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<mel>",
        "MEL, 0-7",
        CLI_PARM_FLAG_NONE,
        evc_parse_mel,
        NULL
    },
    {
        "inject",
        "Inject keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<inject>",
        "Inject, 0-1",
        CLI_PARM_FLAG_NONE,
        evc_parse_inject,
        NULL
    },
    {
        "lookup",
        "Lookup keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<lookup>",
        "Ingress lookup, 0-1",
        CLI_PARM_FLAG_NONE,
        evc_parse_lookup,
        NULL
    },
    {
        "port",
        "Port keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<port_tx>",
        "Egress port number or 'none'",
        CLI_PARM_FLAG_SET,
        evc_parse_port_tx,
        NULL
    },
    {
        "<ot_mode>",
        "Egress outer tag mode: enable|disable",
        CLI_PARM_FLAG_NONE,
        evc_parse_bool,
        NULL,
    },
    {
        "vid",
        "VID keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "isdx",
        "ISDX keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<isdx>",
        "ISDX value, 0-1023 or special values",
        CLI_PARM_FLAG_NONE,
        evc_parse_isdx,
        NULL
    },
    {
        "oam_detect",
        "OAM detect keyword",
        CLI_PARM_FLAG_NONE,
        evc_parse_keyword,
        NULL,
    },
    {
        "<oam_detect>",
        "OAM_detect: untagged|single|double",
        CLI_PARM_FLAG_NONE,
        evc_parse_oam_detect,
        NULL
    },
    {
        "<tx_lookup>",
        "MCE Tx lookup: vid|isdx|pcp_isdx",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_tx_lookup,
        NULL
    },
    {
        "<ot_vid>",
        "Egress outer tag VID",
        CLI_PARM_FLAG_NONE,
        evc_parse_ot_vid,
        NULL
    },
    {
        "<ot_pcp_mode>",
        "Egress outer tag PCP mode: fixed|mapped",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_pcp_mode,
        NULL
    },
    {
        "<ot_dei_mode>",
        "Egress outer tag DEI mode: fixed|dp",
        CLI_PARM_FLAG_NONE,
        evc_parse_mce_dei_mode,
        NULL
    },
    {
        "enable|disable",
        "Set EVC UNI/NNI port check mode",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL,
    },
    {
        "<cmd>",
        "EVC test command",
        CLI_PARM_FLAG_NONE,
        evc_parse_evc_cmd,
        NULL,
    },
#endif /* VTSS_ARCH_SERVAL */
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

cli_cmd_tab_entry (
    "EVC Configuration [<port_list>] [<policer_id>]",
    NULL,
    "Show EVC configuration",
    EVC_CMD_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EVC,
    evc_cmd_conf_show,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

#if defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    "EVC Port Key [<port_list>] [<key_type>]",
    NULL,
    "Set or show port key type",
    EVC_CMD_PORT_KEY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_port_key,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "EVC Port DEI [<port_list>] [<dei_mode>]",
    NULL,
    "Set or show port DEI mode",
    EVC_CMD_PORT_DEI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_port_dei,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL)
cli_cmd_tab_entry (
    "EVC Port Tag [<port_list>] [<tag_mode>]",
    NULL,
    "Set or show port tag match mode",
    EVC_CMD_PORT_TAG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_port_tag,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    "EVC Port Addr [<port_list>] [<addr_mode>]",
    NULL,
    "Set or show port address match mode",
    EVC_CMD_PORT_ADDR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_port_addr,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

cli_cmd_tab_entry (
    "EVC Port L2CP [<port_list>] [<l2cp_list>] [<mode>]",
    NULL,
    "Set or show port L2CP mode",
    EVC_CMD_PORT_L2CP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_port_l2cp,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC Policer [<policer_id>] [enable|disable] [<policer_mode>]\n"
    "            [<cir>] [<cbs>] [<eir>] [<ebs>]",
    NULL,
    "Set or show EVC bandwidth profile",
    EVC_CMD_POLICER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_policer,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
#define EVC_ADD_STX \
    "EVC Add <evc_id> [<vid>] [<ivid>] [<nni_list>] [<learning>] [<policer_id>]"
#else
#define EVC_ADD_STX \
    "EVC Add <evc_id> [<vid>] [<ivid>] [<nni_list>] [<learning>]\n"     \
    "        [inner] [<it_type>] [<it_vid_mode>] [<it_vid>]\n"          \
    "                [<it_preserve>] [<it_pcp>] [<it_dei>]\n"           \
    "        [outer] [<ot_vid>]"
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

cli_cmd_tab_entry (
    EVC_ADD_STX,
    NULL,
    "Add or modify EVC",
    EVC_CMD_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_evc_add,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC Delete <evc_id>",
    NULL,
    "Delete EVC",
    EVC_CMD_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_evc_del,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC Lookup [<evc_id>]",
    NULL,
    "Lookup EVC",
    EVC_CMD_LOOKUP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_evc_lookup,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC Status [<evc_id>]",
    NULL,
    "Show EVC Status",
    EVC_CMD_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EVC,
    evc_cmd_evc_status,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
#define EVC_STAT_STX "EVC Statistics [<evc_id>] [<port_list>] [<command>] [frames|bytes]"
#else
#define EVC_STAT_STX "EVC Statistics [<port_list>] [<class_list>] [<command>]"
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

cli_cmd_tab_entry (
    EVC_STAT_STX,
    NULL,
    "Show or clear EVC statistics",
    EVC_CMD_EVC_STATISTICS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_evc_statistics,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_JAGUAR_1)
#define ECE_ADD_STX                                                     \
    "EVC ECE Add [<ece_id>] [<ece_id_next>] [uni] [<uni_list>]\n"       \
    "            [tag] [<tag_type>] [<vid>] [<pcp>] [<dei>]\n"          \
    "            [intag] [<in_type>] [<in_vid>] [<in_pcp>] [<in_dei>]\n" \
    "            [all | (ipv4 [<dscp>]) | (ipv6 [<dscp>])]\n"           \
    "            [direction] [<direction>]\n"                           \
    "            [evc] [<evc_id>] [<policer_id>]\n"                     \
    "            [pop] [<pop>]\n"                                       \
    "            [policy] [<policy>]\n"                                 \
    "            [outer] [<ot_mode>] [<ot_vid>] [<ot_preserve>] [<ot_pcp>] [<ot_dei>]\n" \
    "            [inner] [<it_type>] [<it_vid>] [<it_preserve>] [<it_pcp>] [<it_dei>]"
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_CARACAL)
#define ECE_ADD_STX                                                     \
    "EVC ECE Add [<ece_id>] [<ece_id_next>] [uni] [<uni_list>]\n"       \
    "            [<smac>] [<dmac_type>]\n"                              \
    "            [tag] [<tag_type>] [<vid>] [<pcp>] [<dei>]\n"          \
    "            [all |\n"                                              \
    "            (ipv4 [<proto>] [<sip>] [<dscp>] [<fragment>] [<sport>] [<dport>]) |\n" \
    "            (ipv6 [<proto>] [<sip_v6>] [<dscp>] [<sport>] [<dport>])]\n" \
    "            [direction] [<direction>]\n"                           \
    "            [evc] [<evc_id>]\n"                                    \
    "            [pop] [<pop>]\n"                                       \
    "            [policy] [<policy>]\n"                                 \
    "            [class] [<class>]\n"                                   \
    "            [outer] [<ot_mode>] [<ot_preserve>] [<ot_pcp>] [<ot_dei>]"
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_SERVAL)
#define ECE_ADD_STX                                                                      \
    "EVC ECE Add [<ece_id>] [<ece_id_next>] [uni] [<uni_list>]\n"                        \
    "            [<smac>] [<dmac_type>] [<dmac>]\n"                                      \
    "            [tag] [<tag_type>] [<vid>] [<pcp>] [<dei>]\n"                           \
    "            [intag] [<in_type>] [<in_vid>] [<in_pcp>] [<in_dei>]\n"                 \
    "            [all |\n"                                                               \
    "            (ipv4 [<proto>] [<sip>] [<dip>] [<dscp>] [<fragment>]\n"                \
    "                  [<sport>] [<dport>]) |\n"                                         \
    "            (ipv6 [<proto>] [<sip_v6>] [<dip_v6>] [<dscp>] [<sport>] [<dport>])]\n" \
    "            [direction] [<direction>]\n"                                            \
    "            [rule_type] [<rule_type>]\n"                                            \
    "            [tx_lookup] [<tx_lookup>]\n"                                            \
    "            [evc] [<evc_id>] [<policer_id>]\n"                                      \
    "            [pop] [<pop>]\n"                                                        \
    "            [policy] [<policy>]\n"                                                  \
    "            [class] [<class>]\n"                                                    \
    "            [dp] [<dp>]\n"                                                          \
    "            [outer] [<ot_mode>] [<ot_vid>] [<ot_pcp_mode>] [<ot_pcp>]\n"            \
    "                    [<ot_dei_mode>] [<ot_dei>]\n"                                   \
    "            [inner] [<it_type>] [<it_vid>] [<it_pcp_mode>] [<it_pcp>]\n"            \
    "                    [<it_dei_mode>] [<it_dei>]"
#endif /* VTSS_ARCH_SERVAL */

cli_cmd_tab_entry (
    ECE_ADD_STX,
    NULL,
    "Add or modify EVC Control Entry (ECE):\n"
    "- If <ece_id> is specified and the ECE exists, the ECE will be modified.\n"
    "- If <ece_id> is omitted or the ECE does not exist, a new ECE will be added.\n"
    "- If <ece_id_next> is specified, the ECE will be placed before this entry.\n"
    "- If <ece_id_next> is 'last', the ECE will be placed at the end of the list.\n"
    "- If <ece_id_next> is omitted and it is a new ECE, the ECE will be placed last.\n"
    "- If <ece_id_next> is omitted and the ECE exists, the ECE will not be moved",
    EVC_CMD_ECE_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_ece_add,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC ECE Delete <ece_id>",
    NULL,
    "Delete ECE",
    EVC_CMD_ECE_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_ece_del,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC ECE Lookup [<ece_id>]",
    NULL,
    "Lookup ECE",
    EVC_CMD_ECE_LOOKUP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_ece_lookup,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "EVC ECE Status [<ece_id>]",
    NULL,
    "Show ECE Status",
    EVC_CMD_ECE_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EVC,
    evc_cmd_ece_status,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    "EVC ECE Statistics [<ece_id>] [<port_list>] [<command>] [frames|bytes]",
    NULL,
    "Show or clear ECE statistics",
    EVC_CMD_ECE_STATISTICS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_EVC,
    evc_cmd_ece_statistics,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    "Debug EVC Check [enable|disable]",
    NULL,
    "Set or show EVC UNI/NNI port check",
    EVC_CMD_DEBUG_EVC_PORT_CHECK,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_evc_port_check,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug EVC MEP <evc_id> [<port_list>] [<voe_idx>]",
    NULL,
    "Set or show EVC MEP",
    EVC_CMD_DEBUG_EVC_MEP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_evc_mep,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug EVC Test [<cmd>]",
    NULL,
    "Setup example EVC configuration",
    EVC_CMD_DEBUG_EVC_TEST,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_evc_test,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MCE Add <mce_id> [<mce_id_next>] [<port_rx>]\n"                  \
    "              [tag] [<tag_type>] [<vid>] [<pcp>] [<dei>]\n"            \
    "              [intag] [<in_type>] [<in_vid>] [<in_pcp>] [<in_dei>]\n"  \
    "              [mel] [<mel>]\n"                                         \
    "              [inject] [<inject>]\n"                                   \
    "              [lookup] [<lookup>]\n"                                   \
    "              [port] [<port_tx>] [<voe_idx>]\n"                        \
    "              [tx_lookup] [<tx_lookup>]\n"                             \
    "              [pop] [<pop>]\n"                                         \
    "              [policy] [<policy>]\n"                                   \
    "              [class] [<class>]\n"                                     \
    "              [isdx] [<isdx>]\n"                                       \
    "              [vid] [<ivid>]\n"                                        \
    "              [oam_detect] [<oam_detect>]\n"                           \
    "              [outer] [<ot_mode>] [<ot_vid>] [<ot_pcp_mode>] [<ot_dei_mode>]",
    NULL,
    "Add MCE",
    EVC_CMD_DEBUG_MCE_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_mce_add,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MCE Del <mce_id>",
    NULL,
    "Delete MCE",
    EVC_CMD_DEBUG_MCE_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_mce_del,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MCE Get <mce_id> <port_rx>",
    NULL,
    "Get MCE Port information",
    EVC_CMD_DEBUG_MCE_GET,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    evc_cmd_mce_get,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MCE Key [<port_list>] [<key_type>]",
    NULL,
    "Set or show MCE port key type",
    EVC_CMD_DEBUG_MCE_KEY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_EVC,
    evc_cmd_mce_key,
    evc_cmd_default_set,
    evc_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_SERVAL */

