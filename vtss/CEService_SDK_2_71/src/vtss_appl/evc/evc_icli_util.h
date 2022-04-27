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

*/

#ifndef _EVC_ICLI_UTIL_H_
#define _EVC_ICLI_UTIL_H_

typedef struct {
    BOOL valid;
    BOOL value;
} evc_icli_bool_t;

typedef struct {
    BOOL           valid;
    BOOL           any;
    icli_vcap_vr_t value;
} evc_icli_range_t;

typedef struct {
    BOOL                  any;
    icli_unsigned_range_t *list;
} evc_icli_urange_t;

typedef struct {
    BOOL valid;
    BOOL any;
    BOOL udp;
    BOOL tcp;
    BOOL disable;
    u32  value;
} evc_icli_u32_t;

typedef struct {
    BOOL valid;
    u32  value;
    u32  mask;
} evc_icli_vcap_u32_t;

typedef struct {
    BOOL mode;
    BOOL classified;
    BOOL fixed;
    BOOL valid;
    u32  value;          
} evc_icli_pcp_dei_t;

typedef struct {
    BOOL valid;
    BOOL untag;
    BOOL tag;
    BOOL c_tag;
    BOOL s_tag;
    BOOL s_custom_tag;
    BOOL any;
} evc_icli_tag_type_t;

typedef struct {
    /* Tag matching */
    struct {
        evc_icli_tag_type_t type;
        evc_icli_range_t    vid;
        evc_icli_urange_t   pcp;
        evc_icli_u32_t      dei;
    } m;

    /* Tag adding */
    struct {
        BOOL                enable;
        BOOL                disable;
        evc_icli_u32_t      vid;
        evc_icli_bool_t     preserve;
        evc_icli_pcp_dei_t  pcp;
        evc_icli_pcp_dei_t  dei;
        evc_icli_tag_type_t type;
    } a;
} evc_icli_tag_t;

typedef struct {
    icli_ipv4_subnet_t value;
    BOOL               valid;
} evc_icli_ipv4_t;

typedef struct {
    BOOL valid;
    BOOL any;
    BOOL yes;
    BOOL no;
} evc_icli_fragment_t;

typedef struct {
    BOOL                valid;
    evc_icli_vcap_u32_t etype;
    evc_icli_vcap_u32_t data;
} evc_icli_etype_t;

typedef struct {
    BOOL                valid;
    evc_icli_vcap_u32_t dsap;
    evc_icli_vcap_u32_t ssap;
    evc_icli_vcap_u32_t control;
    evc_icli_vcap_u32_t data;
} evc_icli_llc_t;

typedef struct {
    BOOL                valid;
    evc_icli_vcap_u32_t oui;
    evc_icli_vcap_u32_t pid;
} evc_icli_snap_t;

typedef struct {
    BOOL valid;
    BOOL stp;
    BOOL pause;
    BOOL lacp;
    BOOL lamp;
    BOOL loam;
    BOOL dot1x;
    BOOL elmi;
    BOOL pb;
    BOOL pb_gvrp;
    BOOL lldp;
    BOOL gmrp;
    BOOL gvrp;
    BOOL uld;
    BOOL pagp;
    BOOL pvst;
    BOOL cisco_vlan;
    BOOL cdp;
    BOOL vtp;
    BOOL dtp;
    BOOL cisco_stp;
    BOOL cisco_cfm;
} evc_icli_l2cp_t;

typedef struct {
    BOOL                valid;
    BOOL                any;
    BOOL                ipv4;
    BOOL                ipv6;
    evc_icli_etype_t    etype;
    evc_icli_llc_t      llc;
    evc_icli_snap_t     snap;
    evc_icli_l2cp_t     l2cp;
    evc_icli_u32_t      proto;
    evc_icli_ipv4_t     sip;
    evc_icli_ipv4_t     dip;
    evc_icli_range_t    dscp;
    evc_icli_fragment_t frag;
    evc_icli_range_t    sport;
    evc_icli_range_t    dport;
} evc_icli_frame_t;

typedef struct {
    BOOL       valid;
    BOOL       any;
    BOOL       unicast;
    BOOL       multicast;
    BOOL       broadcast;
    vtss_mac_t value;
} evc_icli_mac_t;

typedef struct {
    BOOL valid;
    BOOL both;
    BOOL uni_to_nni;
    BOOL nni_to_uni;
} evc_icli_dir_t;

typedef struct {
    BOOL valid;
    BOOL both;
    BOOL rx;
    BOOL tx;
} evc_icli_rule_t;

typedef struct {
    BOOL valid;
    BOOL vid;
    BOOL pcp_vid;
    BOOL isdx;
} evc_icli_tx_lookup_t;

typedef struct {
    BOOL valid;
    BOOL tunnel;
    BOOL peer;
    BOOL discard;
} evc_icli_l2cp_mode_t;

typedef struct {
    BOOL valid;
    BOOL cisco;
} evc_icli_l2cp_dmac_t;

typedef struct {
    BOOL            valid;
    u32             value;
    BOOL            type;
    BOOL            mef;
    BOOL            single;
    BOOL            enable;
    BOOL            disable;
    BOOL            none;
    BOOL            evc;
    BOOL            discard;
    BOOL            mode;
    BOOL            coupled;
    BOOL            aware;
    BOOL            blind;
    evc_icli_bool_t rate_type;
    BOOL            cir;
    BOOL            cbs;
    BOOL            eir;
    BOOL            ebs;
} evc_icli_policer_t;

typedef struct {
    BOOL valid;
    BOOL double_tag;
    BOOL normal;
    BOOL ip_addr;
} evc_icli_key_t;

typedef struct {
    icli_unsigned_range_t *peer_list;
    icli_unsigned_range_t *forward_list;
    icli_unsigned_range_t *discard_list;
} evc_icli_l2cp_list_t;

/* ICLI request structure */
typedef struct {
    u32                     session_id;
    evc_icli_u32_t          evc;
    evc_icli_u32_t          ece;
    evc_icli_u32_t          ece_next;
    icli_stack_port_range_t *port_list;
    vtss_port_no_t          iport;
    vtss_port_no_t          uport;
    vtss_prio_t             prio;
    evc_icli_policer_t      pol;
    vtss_evc_policer_conf_t pol_conf;
    evc_icli_u32_t          vid;
    evc_icli_u32_t          ivid;
    evc_icli_mac_t          smac;
    evc_icli_mac_t          dmac;
    evc_icli_tag_t          it;
    evc_icli_tag_t          ot;
    evc_icli_frame_t        frame;
    evc_icli_u32_t          pop;
    evc_icli_u32_t          policy;
    evc_icli_u32_t          cos;
    evc_icli_u32_t          dpl;
    evc_icli_dir_t          dir;
    evc_icli_rule_t         rule;
    evc_icli_tx_lookup_t    tx;
    evc_icli_bool_t         lookup;
    evc_icli_l2cp_mode_t    l2cp_mode;
    evc_icli_l2cp_dmac_t    l2cp_dmac;
    evc_icli_bool_t         learning;
    evc_icli_key_t          key;
    evc_icli_key_t          key_adv;
    evc_icli_bool_t         addr;
    evc_icli_bool_t         addr_adv;
    evc_icli_l2cp_list_t    l2cp;
    evc_icli_bool_t         dei_colour;
    evc_icli_bool_t         tag_inner;
    icli_unsigned_range_t   *cos_list;
    BOOL                    header;
    BOOL                    green;
    BOOL                    yellow;
    BOOL                    red;
    BOOL                    discard;
    BOOL                    clear;
    BOOL                    bytes;
    BOOL                    frames;
    BOOL                    update;
    BOOL                    dummy;  /* Unused, for Lint */
} evc_icli_req_t;

void evc_icli_evc_show(evc_icli_req_t *req);
void evc_icli_evc_stats(evc_icli_req_t *req);
void evc_icli_evc_policer(evc_icli_req_t *req);
void evc_icli_evc_add(evc_icli_req_t *req);
void evc_icli_evc_del(evc_icli_req_t *req);
void evc_icli_ece_add(evc_icli_req_t *req);
void evc_icli_ece_del(evc_icli_req_t *req);
void evc_icli_port(evc_icli_req_t *req);

#endif /* _EVC_ICLI_UTIL_H_ */
