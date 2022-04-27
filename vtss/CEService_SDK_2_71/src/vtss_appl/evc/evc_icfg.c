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

#include "main.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "icfg_api.h"
#include "icli_porting_util.h"
#include "evc_api.h"

/* Number of optional prepended headers */
#define EVC_ICLI_HDR_COUNT 3

typedef struct {
    const vtss_icfg_query_request_t *req;
    vtss_icfg_query_result_t        *result;
    BOOL                            all_defaults;
    char                            *hdr[EVC_ICLI_HDR_COUNT];
    char                            buf[128];
} evc_icfg_info_t;

static void evc_icfg_info_init(evc_icfg_info_t *info, const vtss_icfg_query_request_t *req,
                               vtss_icfg_query_result_t *result)
{
    memset(info, 0, sizeof(*info));
    info->req = req;
    info->result = result;
    info->all_defaults = req->all_defaults;
}

static void evc_icfg_hdr_set(evc_icfg_info_t *info, int i, char *hdr)
{
    if (i < EVC_ICLI_HDR_COUNT) {
        info->hdr[i] = hdr;
    }
}

static vtss_rc evc_icfg_printf(evc_icfg_info_t *info)
{
    int  i;
    char *txt;

    /* Print headers */
    for (i = 0; i < EVC_ICLI_HDR_COUNT; i++) {
        if (info->hdr[i] == NULL)
            continue;
        txt = (i == 0 && info->req->cmd_mode == ICLI_CMD_MODE_GLOBAL_CONFIG ? "" : " ");
        VTSS_RC(vtss_icfg_printf(info->result, "%s%s", txt, info->hdr[i]));
        info->hdr[i] = NULL;
    }

    /* Print buffer */
    return vtss_icfg_printf(info->result, " %s", info->buf);
}

static vtss_rc evc_icfg_end(evc_icfg_info_t *info)
{
    return (info->hdr[0] == NULL ? vtss_icfg_printf(info->result, "\n") : VTSS_RC_OK);
}

static vtss_rc evc_icfg_txt(evc_icfg_info_t *info, char *name, char *txt)
{
    char *p = info->buf;

    if (name != NULL)
        p += sprintf(p, "%s", name);
    if (txt != NULL)
        sprintf(p, "%s%s", name == NULL ? "" : " ", txt);
    return evc_icfg_printf(info);
}

static vtss_rc evc_icfg_bool(evc_icfg_info_t *info, BOOL val, BOOL def,
                             char *name, char *txt_ena, char *txt_dis)
{
    if (val == def && info->all_defaults == 0)
        return VTSS_RC_OK;

    return evc_icfg_txt(info, name, val ? txt_ena : txt_dis);
}

static vtss_rc evc_icfg_u32_dis(evc_icfg_info_t *info, u32 val, BOOL val_ena,
                                u32 def, BOOL def_ena, char *name)
{
    char buf[16];

    if (val == def && val_ena == def_ena && info->all_defaults == 0)
        return VTSS_RC_OK;

    if (val_ena)
        sprintf(buf, "%u", val);
    else
        strcpy(buf, "disable");
    return evc_icfg_txt(info, name, buf);
}

static vtss_rc evc_icfg_u32(evc_icfg_info_t *info, u32 val, u32 def, char *name)
{
    return evc_icfg_u32_dis(info, val, 1, def, 1, name);
}

static vtss_rc evc_icfg_enum(evc_icfg_info_t *info, int val, int def, char *name, char *txt)
{
    if (val == def && info->all_defaults == 0)
        return VTSS_RC_OK;

    return evc_icfg_txt(info, name, txt);
}

static vtss_rc evc_icfg_any(evc_icfg_info_t *info, char *name, char *txt)
{
    if (strcmp(txt, "any") == 0 && info->all_defaults == 0) /* Default 'any' */
        return VTSS_RC_OK;

    return evc_icfg_txt(info, name, txt);
}

static vtss_rc evc_icfg_range(evc_icfg_info_t *info, vtss_vcap_vr_t *range, char *name)
{
    char                 buf[32];
    vtss_vcap_vr_value_t low = range->vr.r.low;
    vtss_vcap_vr_value_t high = range->vr.r.high;

    if (range->type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        if (info->all_defaults == 0)
            return VTSS_RC_OK;
        strcpy(buf, "any");
    } else if (low == high)
        sprintf(buf, "%u", low);
    else
        sprintf(buf, "%u-%u", low, high);
    return evc_icfg_txt(info, name, buf);
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static char *evc_icfg_policer_txt(vtss_evc_policer_id_t policer_id, char *buf)
{
    if (policer_id == VTSS_EVC_POLICER_ID_DISCARD)
        strcpy(buf, "discard");
    else if (policer_id == VTSS_EVC_POLICER_ID_NONE)
        strcpy(buf, "none");
    else if (policer_id == VTSS_EVC_POLICER_ID_EVC)
        strcpy(buf, "evc");
    else
        sprintf(buf, "%u", policer_id + 1);
    return buf;
}
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

typedef enum {
    EVC_POL_MODE_COUPLED,
    EVC_POL_MODE_AWARE,
    EVC_POL_MODE_BLIND
} evc_pol_mode_t;

static evc_pol_mode_t evc_pol_mode_get(vtss_evc_policer_conf_t *conf)
{
    return (conf->cf ? EVC_POL_MODE_COUPLED :
#if defined(VTSS_ARCH_JAGUAR_1)
            conf->cm == 0 ? EVC_POL_MODE_BLIND :
#endif /* VTSS_ARCH_JAGUAR_1 */
            EVC_POL_MODE_AWARE);
}

static vtss_rc evc_icfg_policer_conf(evc_icfg_info_t *info)
{
    vtss_evc_policer_id_t   policer_id;
    evc_mgmt_policer_conf_t pol_conf, def_conf;
    vtss_evc_policer_conf_t *c = &pol_conf.conf, *d = &def_conf.conf;
    u32                     count = 0;
    evc_pol_mode_t          pol_mode, def_mode;
    char                    hdr[32];

    evc_mgmt_policer_conf_get_default(&def_conf);
    for (policer_id = 0; policer_id < EVC_POL_COUNT; policer_id++) {
        if (evc_mgmt_policer_conf_get(policer_id, &pol_conf) != VTSS_RC_OK) {
            continue;
        }

        pol_mode = evc_pol_mode_get(c);
        def_mode = evc_pol_mode_get(d);
        if (c->type == d->type && c->enable == d->enable && pol_mode == def_mode && c->line_rate == d->line_rate &&
            c->cir == d->cir && c->cbs == d->cbs && c->eir == d->eir && c->ebs == d->ebs) {
            /* Avoid showing policers with default configuration */
            count++;
            continue;
        }

        /* Prepare header */
        sprintf(hdr, "evc policer %u", policer_id + 1);
        evc_icfg_hdr_set(info, 0, hdr);

        /* Print fields */
        VTSS_RC(evc_icfg_bool(info, c->enable, d->enable, NULL, "enable", "disable"));
        VTSS_RC(evc_icfg_enum(info, c->type, d->type, "type",
                              c->type == VTSS_POLICER_TYPE_MEF ? "mef" : "single"));
        VTSS_RC(evc_icfg_enum(info, pol_mode, def_mode, "mode",
                              pol_mode == EVC_POL_MODE_COUPLED ? "coupled" :
                              pol_mode == EVC_POL_MODE_AWARE ? "aware" : "blind"));
        VTSS_RC(evc_icfg_bool(info, c->line_rate, d->line_rate, "rate-type", "line", "data"));
        VTSS_RC(evc_icfg_u32(info, c->cir, d->cir, "cir"));
        VTSS_RC(evc_icfg_u32(info, c->cbs, d->cbs, "cbs"));
        VTSS_RC(evc_icfg_u32(info, c->eir, d->eir, "eir"));
        VTSS_RC(evc_icfg_u32(info, c->ebs, d->ebs, "ebs"));
        VTSS_RC(evc_icfg_end(info));
    }
    if (count && info->all_defaults) {
        VTSS_RC(vtss_icfg_printf(info->result, "! evc: %u disabled policers not shown\n", count));
    }
    
    return VTSS_RC_OK;
}

static vtss_rc evc_icfg_evc_conf(evc_icfg_info_t *info)
{
    vtss_evc_id_t        evc_id = EVC_ID_FIRST;
    evc_mgmt_conf_t      evc_conf, def_conf;
    vtss_evc_conf_t      *evc = &evc_conf.conf, *def_evc = &def_conf.conf;
    vtss_evc_pb_conf_t   *c = &evc->network.pb, *d = &def_evc->network.pb;
#if defined(VTSS_ARCH_CARACAL)
    vtss_evc_inner_tag_t *pt = &c->inner_tag, *dt = &d->inner_tag;
#endif /* VTSS_ARCH_CARACAL */
    port_iter_t          pit;
    BOOL                 found;
    char                 buf[ICLI_STR_MAX_LEN]; /* Large buffer for interface list */
    char                 hdr[32];

    evc_mgmt_get_default(&def_conf);
    while (evc_mgmt_get(&evc_id, &evc_conf, 1) == VTSS_RC_OK) {
        /* Prepare header */
        sprintf(hdr, "evc %u", evc_id + 1);
        evc_icfg_hdr_set(info, 0, hdr);

        /* VID and IVID */
        VTSS_RC(evc_icfg_u32(info, c->vid, c->vid + 1, "vid")); /* Always show EVC VID */
        VTSS_RC(evc_icfg_u32(info, c->ivid, d->ivid, "ivid"));

        /* Interfaces */
        found = 0;
        port_iter_init_local_all(&pit);
        while (port_iter_getnext(&pit)) {
            if (c->nni[pit.iport]) {
                found = 1;
                break;
            }
        }
        if (found) {
            VTSS_RC(vtss_icfg_printf(info->result, " interface %s", icli_port_list_info_txt(VTSS_ISID_START, c->nni, buf, FALSE)));
        }

        VTSS_RC(evc_icfg_bool(info, evc->learning, def_evc->learning, "learning", NULL, "disable"));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_enum(info, evc->policer_id, def_evc->policer_id,
                              "policer", evc_icfg_policer_txt(evc->policer_id, buf)));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_CARACAL)
        /* Inner tag */
        evc_icfg_hdr_set(info, 1, "inner-tag add");
        VTSS_RC(evc_icfg_enum(info, pt->type, dt->type, "type",
                              pt->type == VTSS_EVC_INNER_TAG_NONE ? "none" :
                              pt->type == VTSS_EVC_INNER_TAG_C ? "c-tag" :
                              pt->type == VTSS_EVC_INNER_TAG_S ? "s-tag" : "s-custom-tag"));
        VTSS_RC(evc_icfg_enum(info, pt->vid_mode, dt->vid_mode, "vid-mode",
                              pt->vid_mode == VTSS_EVC_VID_MODE_TUNNEL ? "tunnel" : "normal"));
        VTSS_RC(evc_icfg_u32(info, pt->vid, dt->vid, "vid"));
        VTSS_RC(evc_icfg_bool(info, pt->pcp_dei_preserve, dt->pcp_dei_preserve,
                              "preserve", NULL, "disable"));
        VTSS_RC(evc_icfg_u32(info, pt->pcp, dt->pcp, "pcp"));
        VTSS_RC(evc_icfg_u32(info, pt->dei, dt->dei, "dei"));

        /* Outer tag */
        evc_icfg_hdr_set(info, 1, "outer-tag add");
        VTSS_RC(evc_icfg_u32(info, c->uvid, d->uvid, "vid"));
        evc_icfg_hdr_set(info, 1, NULL);
#endif /* VTSS_ARCH_CARACAL */
        VTSS_RC(evc_icfg_end(info));
    }

    return VTSS_RC_OK;
}

/* Super set of outer and inner add tag fields */
typedef struct {
    BOOL                      enable;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_ece_inner_tag_type_t type;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    vtss_vid_t                vid;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ece_pcp_mode_t       pcp_mode;
    vtss_ece_dei_mode_t       dei_mode;
#else
    BOOL                      pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
    vtss_tagprio_t            pcp;
    vtss_dei_t                dei;
} evc_icfg_tag_t;

static void evc_icfg_outer2tag(vtss_ece_outer_tag_t *ot, evc_icfg_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));
    tag->enable = ot->enable;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    tag->vid = ot->vid;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    tag->pcp_mode = ot->pcp_mode;
    tag->dei_mode = ot->dei_mode;
#else
    tag->pcp_dei_preserve = ot->pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
    tag->pcp = ot->pcp;
    tag->dei = ot->dei;
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static void evc_icfg_inner2tag(vtss_ece_inner_tag_t *it, evc_icfg_tag_t *tag)
{
    memset(tag, 0, sizeof(*tag));
    tag->type = it->type;
    tag->vid = it->vid;
#if defined(VTSS_ARCH_SERVAL)
    tag->pcp_mode = it->pcp_mode;
    tag->dei_mode = it->dei_mode;
#else
    tag->pcp_dei_preserve = it->pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
    tag->pcp = it->pcp;
    tag->dei = it->dei;
}
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

static vtss_rc evc_icfg_tag(evc_icfg_info_t *info, vtss_ece_t *c, vtss_ece_t *d, BOOL inner)
{
    vtss_ece_tag_t *tag;
    evc_icfg_tag_t ct, dt;
    vtss_vcap_vr_t pcp;

    if (inner) {
        /* Inner tag options */
        evc_icfg_hdr_set(info, 1, "inner-tag");
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        tag = &c->key.inner_tag;
        evc_icfg_inner2tag(&c->action.inner_tag, &ct);
        evc_icfg_inner2tag(&d->action.inner_tag, &dt);
#else
        return VTSS_RC_OK;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    } else {
        /* Outer tag options */
        evc_icfg_hdr_set(info, 1, "outer-tag");
        tag = &c->key.tag;
        evc_icfg_outer2tag(&c->action.outer_tag, &ct);
        evc_icfg_outer2tag(&d->action.outer_tag, &dt);
    }

    /* Tag matching */
    evc_icfg_hdr_set(info, 2, "match");
    VTSS_RC(evc_icfg_any(info, "type",
                         tag->tagged == VTSS_VCAP_BIT_ANY ? "any" :
                         tag->tagged == VTSS_VCAP_BIT_0 ? "untagged" :
                         tag->s_tagged == VTSS_VCAP_BIT_0 ? "c-tagged" :
                         tag->s_tagged == VTSS_VCAP_BIT_1 ? "s-tagged" : "tagged"));
    VTSS_RC(evc_icfg_range(info, &tag->vid, "vid"));
    if (tag->pcp.mask == 0)
        pcp.type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
    else {
        pcp.type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
        pcp.vr.r.low = tag->pcp.value;
        pcp.vr.r.high = (tag->pcp.mask == 7 ? tag->pcp.value :
                         tag->pcp.value + (tag->pcp.mask == 6 ? 1 : 3));
    }
    VTSS_RC(evc_icfg_range(info, &pcp, "pcp"));
    VTSS_RC(evc_icfg_any(info, "dei",
                         tag->dei == VTSS_VCAP_BIT_ANY ? "any" :
                         tag->dei == VTSS_VCAP_BIT_1 ? "1" : "0"));

    /* Tag adding */
    evc_icfg_hdr_set(info, 2, "add");
    if (inner) {
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_enum(info, ct.type, dt.type, "type",
                              ct.type == VTSS_ECE_INNER_TAG_NONE ? "none" :
                              ct.type == VTSS_ECE_INNER_TAG_C ? "c-tag" :
                              ct.type == VTSS_ECE_INNER_TAG_S ? "s-tag" : "s-custom-tag"));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    } else {
        VTSS_RC(evc_icfg_bool(info, ct.enable, dt.enable, "mode", "enable", "disable"));
    }
    VTSS_RC(evc_icfg_u32(info, ct.vid, dt.vid, "vid"));
#if defined(VTSS_ARCH_SERVAL)
    VTSS_RC(evc_icfg_enum(info, ct.pcp_mode, dt.pcp_mode, "pcp-mode",
                          ct.pcp_mode == VTSS_ECE_PCP_MODE_CLASSIFIED ? "classified" :
                          ct.pcp_mode == VTSS_ECE_PCP_MODE_FIXED ? "fixed" : "mapped"));
    VTSS_RC(evc_icfg_enum(info, ct.dei_mode, dt.dei_mode, "dei-mode",
                          ct.dei_mode == VTSS_ECE_DEI_MODE_CLASSIFIED ? "classified" :
                          ct.dei_mode == VTSS_ECE_DEI_MODE_FIXED ? "fixed" : "dp"));
#else
    VTSS_RC(evc_icfg_bool(info, ct.pcp_dei_preserve, dt.pcp_dei_preserve,
                          "preserve", NULL, "disable"));
#endif /* VTSS_ARCH_SERVAL */
    VTSS_RC(evc_icfg_u32(info, ct.pcp, dt.pcp, "pcp"));
    VTSS_RC(evc_icfg_u32(info, ct.dei, dt.dei, "dei"));

    evc_icfg_hdr_set(info, 1, NULL);
    evc_icfg_hdr_set(info, 2, NULL);

    return VTSS_RC_OK;
}

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static char *evc_icfg_ipv4_txt(vtss_vcap_ip_t *ip, char *buf)
{
    u32 i, n;

    for (n = 0, i = 0; i < 32; i++)
        if (ip->mask & (1<<i))
            n++;
    if (n == 0)
        strcpy(buf, "any");
    else {
        i = strlen(misc_ipv4_txt(ip->value, buf));
        sprintf(&buf[i], "/%u", n);
    }
    return buf;
}

static char *evc_icfg_ipv6_txt(vtss_vcap_u128_t *ipv6, char *buf)
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
    return evc_icfg_ipv4_txt(&ipv4, buf);
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
static vtss_rc evc_icfg_vcap_any(evc_icfg_info_t *info, char *name, u8 *val, u8 *msk, u8 count)
{
    u32  i, value = 0, mask = 0, max = 0;
    char buf[32], *p = buf;

    for (i = 0; i < count; i++) {
        value = ((value << 8) + val[i]);
        mask = ((mask << 8) + msk[i]);
        max = ((max << 8) + 0xff);
    }

    if (mask == 0) {
        if (info->all_defaults == 0) /* Default mask zero */
            return VTSS_RC_OK;
        strcpy(buf, "any");
    } else {
        p += sprintf(p, "0x%x", value);
        if (mask != max)
            p += sprintf(p, " 0x%x", mask);
    }

    return evc_icfg_txt(info, name, buf);
}

static vtss_rc evc_icfg_vcap_u8(evc_icfg_info_t *info, char *name, u8 val, u8 msk)
{
    return evc_icfg_vcap_any(info, name, &val, &msk, 1);
}

static vtss_rc evc_icfg_vcap_u16(evc_icfg_info_t *info, char *name, u8 *val, u8 *msk)
{
    return evc_icfg_vcap_any(info, name, val, msk, 2);
}
#endif /* VTSS_ARCH_SERVAL */

static vtss_rc evc_icfg_ece_conf(evc_icfg_info_t *info)
{
    evc_mgmt_ece_conf_t    conf, def_conf;
    vtss_ece_t             *c = &conf.conf, *d = &def_conf.conf;
    vtss_ece_key_t         *key = &c->key;
    vtss_ece_frame_ipv4_t  *ipv4;
    vtss_ece_frame_ipv6_t  *ipv6 = &key->frame.ipv6;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ece_frame_etype_t *etype = &key->frame.etype;
    vtss_ece_frame_llc_t   *llc = &key->frame.llc;
    vtss_ece_frame_snap_t  *snap = &key->frame.snap;
    evc_l2cp_data_t        *l2cp = &conf.data.l2cp;
#endif /* VTSS_ARCH_SERVAL */
    vtss_ece_action_t     *ca = &c->action, *da = &d->action;
    BOOL                  iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t           pit;
    BOOL                  found;
    char                  *txt, buf[ICLI_STR_MAX_LEN]; /* Large buffer for interface list */

    c->id = EVC_ECE_ID_FIRST;
    while (evc_mgmt_ece_get(c->id, &conf, 1) == VTSS_RC_OK) {
        /* Print header */
        VTSS_RC(vtss_icfg_printf(info->result, "evc ece %u", c->id));

        /* Get defaults */
        evc_mgmt_ece_get_default(&def_conf);

#if defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_bool(info, key->lookup, d->key.lookup, "lookup", "advanced", "basic"));
#endif /* VTSS_ARCH_SERVAL */

        /* Interfaces */
        found = 0;
        port_iter_init_local_all(&pit);
        while (port_iter_getnext(&pit)) {
            iport_list[pit.iport] = 0;
            if (key->port_list[pit.iport] != VTSS_ECE_PORT_NONE) {
                iport_list[pit.iport] = 1;
                found = 1;
            }
        }
        if (found) {
            txt = icli_port_list_info_txt(VTSS_ISID_START, iport_list, buf, FALSE);
            VTSS_RC(vtss_icfg_printf(info->result, " interface %s", txt));
        }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        {
            vtss_ece_mac_t *mac = &key->mac;

            /* MAC addresses */
            VTSS_RC(evc_icfg_any(info, "smac",
                                 mac->smac.mask[0] ? misc_mac_txt(mac->smac.value, buf) :
                                 "any"));
            txt = (mac->dmac_bc == VTSS_VCAP_BIT_1 ? "broadcast" :
                   mac->dmac_mc == VTSS_VCAP_BIT_1 ? "multicast" :
                   mac->dmac_mc == VTSS_VCAP_BIT_0 ? "unicast" :
#if defined(VTSS_ARCH_SERVAL)
                   mac->dmac.mask[0] ? misc_mac_txt(key->mac.dmac.value, buf) :
#endif /* VTSS_ARCH_SERVAL */
                   "any");
            VTSS_RC(evc_icfg_any(info, "dmac", txt));
        }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

        /* Outer tag */
        VTSS_RC(evc_icfg_tag(info, c, d, 0));

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        /* Inner tag */
        VTSS_RC(evc_icfg_tag(info, c, d, 1));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        /* Frame type */
        txt = (key->type == VTSS_ECE_TYPE_IPV4 ? "ipv4" :
               key->type == VTSS_ECE_TYPE_IPV6 ? "ipv6" :
#if defined(VTSS_ARCH_SERVAL)
               key->type == VTSS_ECE_TYPE_ETYPE ? "etype" :
               key->type == VTSS_ECE_TYPE_LLC ? "llc" :
               key->type == VTSS_ECE_TYPE_SNAP ? "snap" :
               l2cp->proto != EVC_L2CP_NONE ? "l2cp" :
#endif /* VTSS_ARCH_SERVAL */
               "any");
        VTSS_RC(evc_icfg_any(info, "frame-type", txt));

        ipv4 = NULL;
        switch (key->type) {
#if defined(VTSS_ARCH_SERVAL)
        case VTSS_ECE_TYPE_ETYPE:
            VTSS_RC(evc_icfg_vcap_u16(info, "etype-value", &etype->etype.value[0], &etype->etype.mask[0]));
            VTSS_RC(evc_icfg_vcap_u16(info, "etype-data", &etype->data.value[0], &etype->data.mask[0]));
            break;
        case VTSS_ECE_TYPE_LLC:
            VTSS_RC(evc_icfg_vcap_u8(info, "dsap", llc->data.value[0], llc->data.mask[0]));
            VTSS_RC(evc_icfg_vcap_u8(info, "ssap", llc->data.value[1], llc->data.mask[1]));
            VTSS_RC(evc_icfg_vcap_u8(info, "control", llc->data.value[2], llc->data.mask[2]));
            VTSS_RC(evc_icfg_vcap_u16(info, "llc-data", &llc->data.value[3], &llc->data.mask[3]));
            break;
        case VTSS_ECE_TYPE_SNAP:
            VTSS_RC(evc_icfg_vcap_any(info, "oui", &snap->data.value[0], &snap->data.mask[0], 3));
            VTSS_RC(evc_icfg_vcap_u16(info, "pid", &snap->data.value[3], &snap->data.mask[3]));
            break;
#endif /* VTSS_ARCH_SERVAL */
        case VTSS_ECE_TYPE_IPV4:
            ipv4 = &key->frame.ipv4;
            /* Fall through */
        case VTSS_ECE_TYPE_IPV6:
            /* IPv4/IPv6 */
        {
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            vtss_vcap_u8_t *proto = (ipv4 ? &ipv4->proto : &ipv6->proto);
            BOOL           tcp = (proto->value == 6);
            BOOL           udp = (proto->value == 17);

            sprintf(buf, "%u", proto->value);
            VTSS_RC(evc_icfg_any(info, "proto",
                                 proto->mask == 0 ? "any" : udp ? "udp" : tcp ? "tcp" : buf));
#endif /* VTSS_ARCH_CARACAL/SERVAL */

            VTSS_RC(evc_icfg_range(info, ipv4 ? &ipv4->dscp : &ipv6->dscp, "dscp"));

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            if (ipv4) {
                VTSS_RC(evc_icfg_any(info, "fragment",
                                     ipv4->fragment == VTSS_VCAP_BIT_ANY ? "any" :
                                     ipv4->fragment == VTSS_VCAP_BIT_1 ? "yes" : "no"));
            }

            /* IP addresses */
            txt = (ipv4 ? evc_icfg_ipv4_txt(&ipv4->sip, buf) : evc_icfg_ipv6_txt(&ipv6->sip, buf));
            VTSS_RC(evc_icfg_any(info, "sip", txt));
#if defined(VTSS_ARCH_SERVAL)
            txt = (ipv4 ? evc_icfg_ipv4_txt(&ipv4->dip, buf) : evc_icfg_ipv6_txt(&ipv6->dip, buf));
            VTSS_RC(evc_icfg_any(info, "dip", txt));
#endif /* VTSS_ARCH_SERVAL */

            /* UDP/TCP ports */
            if (udp || tcp) {
                VTSS_RC(evc_icfg_range(info, ipv4 ? &ipv4->sport : &ipv6->sport, "sport"));
                VTSS_RC(evc_icfg_range(info, ipv4 ? &ipv4->dport : &ipv6->dport, "dport"));
            }
#endif /* VTSS_ARCH_CARACAL/SERVAL */
            break;
        }
        default:
#if defined(VTSS_ARCH_SERVAL)
            if (l2cp->proto != EVC_L2CP_NONE) {
                txt = (l2cp->proto == EVC_L2CP_STP ? "stp" :
                       l2cp->proto == EVC_L2CP_PAUSE ? "pause" :
                       l2cp->proto == EVC_L2CP_LACP ? "lacp" :
                       l2cp->proto == EVC_L2CP_LAMP ? "lamp" :
                       l2cp->proto == EVC_L2CP_LOAM ? "loam" :
                       l2cp->proto == EVC_L2CP_DOT1X ? "dot1x" :
                       l2cp->proto == EVC_L2CP_ELMI ? "elmi" :
                       l2cp->proto == EVC_L2CP_PB ? "pb" :
                       l2cp->proto == EVC_L2CP_PB_GVRP ? "pb-gvrp" :
                       l2cp->proto == EVC_L2CP_LLDP ? "lldp" :
                       l2cp->proto == EVC_L2CP_GMRP ? "gmrp" :
                       l2cp->proto == EVC_L2CP_GVRP ? "gvrp" :
                       l2cp->proto == EVC_L2CP_ULD ? "uld" :
                       l2cp->proto == EVC_L2CP_PAGP ? "pagp" :
                       l2cp->proto == EVC_L2CP_PVST ? "pvst" :
                       l2cp->proto == EVC_L2CP_CISCO_VLAN ? "cisco-vlan" :
                       l2cp->proto == EVC_L2CP_CDP ? "cdp" :
                       l2cp->proto == EVC_L2CP_VTP ? "vtp" :
                       l2cp->proto == EVC_L2CP_DTP ? "dtp" :
                       l2cp->proto == EVC_L2CP_CISCO_STP ? "cisco-stp" :
                       l2cp->proto == EVC_L2CP_CISCO_CFM ? "cisco-cfm" : "?");
                VTSS_RC(evc_icfg_txt(info, NULL, txt));
            }
#endif /* VTSS_ARCH_SERVAL */
            break;
        }

        /* Action fields */
        VTSS_RC(evc_icfg_enum(info, ca->dir, da->dir, "direction",
                              ca->dir == VTSS_ECE_DIR_BOTH ? "both" :
                              ca->dir == VTSS_ECE_DIR_UNI_TO_NNI ? "uni-to-nni" : "nni-to-uni"));
#if defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_enum(info, ca->rule, da->rule, "rule-type",
                              ca->rule == VTSS_ECE_RULE_BOTH ? "both" :
                              ca->rule == VTSS_ECE_RULE_RX ? "rx" : "rx"));
        VTSS_RC(evc_icfg_enum(info, ca->tx_lookup, da->tx_lookup, "tx-lookup",
                              ca->tx_lookup == VTSS_ECE_TX_LOOKUP_VID ? "vid" :
                              ca->tx_lookup == VTSS_ECE_TX_LOOKUP_VID_PCP ? "pcp-vid" : "isdx"));
#endif /* VTSS_ARCH_SERVAL */
        if (ca->evc_id == VTSS_EVC_ID_NONE) {
            VTSS_RC(evc_icfg_txt(info, "evc", "none"));
        } else {
            VTSS_RC(evc_icfg_u32(info, ca->evc_id + 1, da->evc_id + 1, "evc"));
        }
#if defined(VTSS_ARCH_SERVAL)
        evc_icfg_hdr_set(info, 1, "l2cp");
        VTSS_RC(evc_icfg_enum(info, l2cp->mode, def_conf.data.l2cp.mode, "mode",
                              l2cp->mode == EVC_L2CP_MODE_FORWARD ? "forward" :
                              l2cp->mode == EVC_L2CP_MODE_TUNNEL ? "tunnel" :
                              l2cp->mode == EVC_L2CP_MODE_DISCARD ? "discard" :
                              l2cp->mode == EVC_L2CP_MODE_PEER ? "peer" : "?"));
        VTSS_RC(evc_icfg_enum(info, l2cp->dmac, def_conf.data.l2cp.dmac, "tmac",
                              l2cp->dmac == EVC_L2CP_DMAC_CUSTOM ? "custom" :
                              l2cp->dmac == EVC_L2CP_DMAC_CISCO ? "cisco" : "?"));
        evc_icfg_hdr_set(info, 1, NULL);
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_enum(info, ca->policer_id, da->policer_id, "policer",
                              evc_icfg_policer_txt(ca->policer_id, buf)));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
        VTSS_RC(evc_icfg_u32(info, ca->pop_tag, da->pop_tag, "pop"));
        VTSS_RC(evc_icfg_u32(info, ca->policy_no, da->policy_no, "policy"));
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_u32_dis(info, ca->prio, ca->prio_enable,
                                 da->prio, da->prio_enable, "cos"));
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
        VTSS_RC(evc_icfg_u32_dis(info, ca->dp, ca->dp_enable, da->dp, da->dp_enable, "dpl"));
#endif /* VTSS_ARCH_CARACAL/SERVAL */

        VTSS_RC(evc_icfg_end(info));
    }

    return VTSS_RC_OK;
}

static vtss_rc evc_icfg_global_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    evc_icfg_info_t info;

    /* Policer configuration */
    evc_icfg_info_init(&info, req, result);
    VTSS_RC(evc_icfg_policer_conf(&info));

    /* EVC configuration */
    evc_icfg_info_init(&info, req, result);
    VTSS_RC(evc_icfg_evc_conf(&info));

    /* ECE configuration */
    evc_icfg_info_init(&info, req, result);
    VTSS_RC(evc_icfg_ece_conf(&info));

    return VTSS_RC_OK;
}

static vtss_rc evc_icfg_l2cp(evc_icfg_info_t *info,
                             evc_mgmt_port_conf_t *pc, evc_mgmt_port_conf_t *dc,
                             vtss_packet_reg_type_t type, char *txt)
{
    uint                   i;
    BOOL                   list[32], found = 0;
    vtss_packet_reg_type_t port_type, def_type;
    char                   buf[80];

    /* Build list of L2CP differing from default */
    for (i = 0; i < 32; i++) {
        list[i] = 0;
        if (i < 16) {
            port_type = pc->reg.bpdu_reg[i];
            def_type = dc->reg.bpdu_reg[i];
        } else {
            port_type = pc->reg.garp_reg[i - 16];
            def_type = dc->reg.garp_reg[i - 16];
        }
        if ((port_type != def_type || info->all_defaults) && port_type == type) {
            list[i] = 1;
            found = 1;
        }
    }

    if (found) {
        sprintf(info->buf, "%s %s", txt, mgmt_list2txt(&list[1], -1, 30, buf));
        return evc_icfg_printf(info);
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_ARCH_SERVAL)
static vtss_rc evc_icfg_key_type(evc_icfg_info_t *info,
                                 vtss_vcap_key_type_t c_key_type, vtss_vcap_key_type_t d_key_type,
                                 char *name)
{
    return (c_key_type == d_key_type && info->all_defaults == 0 ? VTSS_RC_OK :
            evc_icfg_txt(info, name,
                         c_key_type == VTSS_VCAP_KEY_TYPE_DOUBLE_TAG ? "double-tag" :
                         c_key_type == VTSS_VCAP_KEY_TYPE_NORMAL ? "normal" :
                         c_key_type == VTSS_VCAP_KEY_TYPE_IP_ADDR ? "ip-addr" : "mac-ip-addr"));
}
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static vtss_rc evc_icfg_addr(evc_icfg_info_t *info, BOOL c_dmac_dip, BOOL d_dmac_dip, char *name)
{
    return evc_icfg_bool(info, c_dmac_dip, d_dmac_dip, name, "destination", "source");
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

static vtss_rc evc_icfg_port_conf(const vtss_icfg_query_request_t *req,
                                  vtss_icfg_query_result_t *result)
{
    evc_icfg_info_t      info;
    vtss_port_no_t       iport = uport2iport(req->instance_id.port.begin_uport);
    evc_mgmt_port_conf_t port_conf, def_conf;
    vtss_evc_port_conf_t *c = &port_conf.conf, *d = &def_conf.conf;

    evc_icfg_info_init(&info, req, result);
    evc_icfg_hdr_set(&info, 0, "evc");

    evc_mgmt_port_conf_get_default(&def_conf);
    if (evc_mgmt_port_conf_get(iport, &port_conf) != VTSS_RC_OK)
        return VTSS_RC_OK;

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
    VTSS_RC(evc_icfg_bool(&info, c->dei_colouring, d->dei_colouring, "dei", "colored", "fixed"));
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */

#if defined(VTSS_ARCH_CARACAL)
    VTSS_RC(evc_icfg_bool(&info, c->inner_tag, d->inner_tag, "tag", "inner", "outer"));
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_SERVAL)
    VTSS_RC(evc_icfg_key_type(&info, c->key_type, d->key_type, "key"));
    VTSS_RC(evc_icfg_key_type(&info, port_conf.vcap_conf.key_type_is1_1, def_conf.vcap_conf.key_type_is1_1, "key-advanced"));
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    VTSS_RC(evc_icfg_addr(&info, c->dmac_dip, d->dmac_dip, "addr"));
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    VTSS_RC(evc_icfg_addr(&info, port_conf.vcap_conf.dmac_dip_1, def_conf.vcap_conf.dmac_dip_1, "addr-advanced"));
#endif /* VTSS_ARCH_SERVAL */

    evc_icfg_hdr_set(&info, 1, "l2cp");
    VTSS_RC(evc_icfg_l2cp(&info, &port_conf, &def_conf, VTSS_PACKET_REG_CPU_ONLY, "peer"));
    VTSS_RC(evc_icfg_l2cp(&info, &port_conf, &def_conf, VTSS_PACKET_REG_FORWARD, "forward"));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    VTSS_RC(evc_icfg_l2cp(&info, &port_conf, &def_conf, VTSS_PACKET_REG_DISCARD, "discard"));
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    return evc_icfg_end(&info);
}

void evc_icfg_init(void)
{
    (void)vtss_icfg_query_register(VTSS_ICFG_EVC_GLOBAL_CONF, "evc", evc_icfg_global_conf);
    (void)vtss_icfg_query_register(VTSS_ICFG_EVC_PORT_CONF, "evc", evc_icfg_port_conf);
}
