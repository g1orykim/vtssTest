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
#include "port_api.h"
#include "evc_api.h"
#include "icli_api.h"
#include "icli_porting_util.h"
#include "evc_icli_util.h"
#include "vlan_api.h"

static char *evc_icli_port_txt(evc_icli_req_t *req, char *buf)
{
    return icli_port_info_txt(VTSS_USID_START, req->uport, buf);
}

static vtss_evc_id_t evc_icli_id_get(evc_icli_req_t *req)
{
    vtss_evc_id_t evc_id = req->evc.value;

    return (evc_id == VTSS_EVC_ID_NONE ? evc_id :
            evc_id == 0 ? EVC_ID_FIRST : (evc_id - 1));
}

void evc_icli_port_iter_init(port_iter_t *pit)
{
    (void)icli_port_iter_init(pit, VTSS_ISID_START, PORT_ITER_FLAGS_NORMAL);
}

static void evc_icli_status(evc_icli_req_t *req, BOOL conflict)
{
    u32  session_id = req->session_id;
    char buf[80];

    if (req->header) {
        req->header = 0;
        sprintf(buf, "%s ID  Status", req->evc.valid ? "EVC" : "ECE");
        icli_table_header(session_id, buf);
    }
    ICLI_PRINTF("%-8u%s\n", 
                req->evc.valid ? (req->evc.value + 1) : req->ece.value,
                conflict ? "Conflict" : "Active");
}

void evc_icli_evc_show(evc_icli_req_t *req)
{
    u32                 session_id = req->session_id;
    evc_mgmt_conf_t     evc_conf;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_t          *ece = &ece_conf.conf;
    vtss_evc_id_t       evc_id = evc_icli_id_get(req);
    BOOL                next = (evc_id == EVC_ID_FIRST ? 1 : 0);

    /* EVC status */
    while (req->evc.valid && evc_mgmt_get(&evc_id, &evc_conf, next) == VTSS_OK) {
        req->evc.value = evc_id;
        evc_icli_status(req, evc_conf.conflict);
        if (!next)
            break;
    }
    if (!req->header) {
        req->header = 1;
        ICLI_PRINTF("\n");
    }

    /* ECE status */
    if (req->ece.value) {
        /* Specific ECE */
        ece->id = req->ece.value;
        next = 0;
    } else {
        /* All EVCs */
        ece->id = EVC_ECE_ID_FIRST;
        next = 1;
    }
    req->evc.valid = 0;
    while (req->ece.valid && evc_mgmt_ece_get(ece->id, &ece_conf, next) == VTSS_RC_OK) {
        req->ece.value = ece->id;
        evc_icli_status(req, ece_conf.conflict);
        if (!next)
            break;
    }
}

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static char *evc_icli_evc_id_txt(evc_icli_req_t *req, char *buf)
{
    if (req->evc.value == VTSS_EVC_ID_NONE) {
        strcpy(buf, "None");
    } else {
        sprintf(buf, "%u", req->evc.value + 1);
    }
    return buf;
}

static vtss_evc_policer_id_t evc_icli_policer_id(evc_icli_req_t *req)
{
    return (req->pol.evc ? VTSS_EVC_POLICER_ID_EVC :
            req->pol.none ? VTSS_EVC_POLICER_ID_NONE :
            req->pol.discard ? VTSS_EVC_POLICER_ID_DISCARD : (req->pol.value - 1));
}

/* Print counters in two columns with header */
static void evc_icli_stat_port(evc_icli_req_t *req, const char *name,
                               vtss_counter_pair_t *c1, vtss_counter_pair_t *c2)
{
    u32  session_id = req->session_id;
    char buf[80], buf1[80], *p;

    if (req->header) {
        req->header = 0;
        p = &buf[0];
        sprintf(buf1, "%s %s", name, req->bytes ? "Bytes" : "Frames");
        p += sprintf(p, "%s ID  %-25sRx %-19s", req->evc.valid ? "EVC" : "ECE", "Interface", buf1);
        if (c2 != NULL) {
            sprintf(p, "Tx %-19s", buf1);
        }
        icli_table_header(session_id, buf);
    }
    if (req->evc.valid) {
        ICLI_PRINTF("%-8s", evc_icli_evc_id_txt(req, buf));
    } else {
        ICLI_PRINTF("%-8u", req->ece.value);
    }
    ICLI_PRINTF("%-25s%-22llu", evc_icli_port_txt(req, buf), req->bytes ? c1->bytes : c1->frames);
    if (c2 != NULL) {
        ICLI_PRINTF("%-22llu", req->bytes ? c2->bytes : c2->frames);
    }
    ICLI_PRINTF("\n");
}

/* Print two counters in columns */
static void evc_icli_stats(evc_icli_req_t *req, const char *name,
                           vtss_counter_pair_t *c1, vtss_counter_pair_t *c2)
{
    u32  session_id = req->session_id;
    char buf[80];

    sprintf(buf, "%s %s:", name, req->bytes ? "Bytes" : "Frames");
    ICLI_PRINTF("Rx %-15s%20llu   ", buf, req->bytes ? c1->bytes : c1->frames);
    if (c2 != NULL) {
        ICLI_PRINTF("Tx %-15s%20llu", buf, req->bytes ? c2->bytes : c2->frames);
    }
    ICLI_PRINTF("\n");
}

static void evc_icli_show_stats(evc_icli_req_t *req)
{
    u32                 session_id = req->session_id;
    vtss_evc_counters_t c;
    char                buf[80];
    int                 i;
    BOOL                bytes = req->bytes;
    
    if ((req->evc.valid ? vtss_evc_counters_get(NULL, req->evc.value, req->iport, &c) :
         vtss_ece_counters_get(NULL, req->ece.value, req->iport, &c)) != VTSS_RC_OK) {
        return;
    }
    
    if (req->green) {
        evc_icli_stat_port(req, "Green", &c.rx_green, &c.tx_green);
    } else if (req->yellow) {
        evc_icli_stat_port(req, "Yellow", &c.rx_yellow, &c.tx_yellow);
    } else if (req->red) {
        evc_icli_stat_port(req, "Red", &c.rx_red, NULL);
    } else if (req->discard) {
        evc_icli_stat_port(req, "Discard", &c.rx_discard, &c.tx_discard);
    } else {
        /* Detailed statistics */
        if (req->evc.valid) {
            ICLI_PRINTF("EVC ID %s", evc_icli_evc_id_txt(req, buf));
        } else {
            ICLI_PRINTF("ECE ID %u", req->ece.value);
        }
        ICLI_PRINTF(", Interface %s Statistics:\n\n", evc_icli_port_txt(req, buf));
        for (i = 0; i < 2; i++) {
            if ((i == 0 && bytes) || (i == 1 && req->frames)) {
                continue;
            }
            req->bytes = (i == 0 ? 0 : 1);
            evc_icli_stats(req, "Green", &c.rx_green, &c.tx_green);
            evc_icli_stats(req, "Yellow", &c.rx_yellow, &c.tx_yellow);
            evc_icli_stats(req, "Red", &c.rx_red, NULL);
            evc_icli_stats(req, "Discard", &c.rx_discard, &c.tx_discard);
        }
        ICLI_PRINTF("\n");
        req->bytes = bytes;
    }
}

void evc_icli_evc_stats(evc_icli_req_t *req)
{
    u32                 session_id = req->session_id;
    evc_mgmt_conf_t     evc_conf;
    vtss_evc_pb_conf_t  *pb = &evc_conf.conf.network.pb;
    evc_mgmt_ece_conf_t ece_conf;
    vtss_ece_t          *ece = &ece_conf.conf;
    vtss_ece_key_t      *key = &ece->key;
    port_iter_t         pit;
    BOOL                iport_list[VTSS_PORT_ARRAY_SIZE];
    vtss_evc_id_t       evc_id = evc_icli_id_get(req);
    BOOL                next = (evc_id == EVC_ID_FIRST ? 1 : 0);
        
    /* EVC statistics */
    while (req->evc.valid && evc_mgmt_get(&evc_id, &evc_conf, next) == VTSS_OK) {
        /* Find EVC NNI ports */
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport_list[pit.iport] = pb->nni[pit.iport];
        }

        /* Find ECE UNI ports */
        ece->id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(ece->id, &ece_conf, 1) == VTSS_RC_OK) {
            if (ece->action.evc_id != evc_id) {
                continue;
            }

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                if (key->port_list[pit.iport] != VTSS_ECE_PORT_NONE) {
                    iport_list[pit.iport] = 1;
                }
            }
        }
        
        evc_icli_port_iter_init(&pit);
        while (icli_port_iter_getnext(&pit, req->port_list)) {
            if (iport_list[pit.iport] == 0) {
                continue;
            }

            if (req->clear) {
                (void)vtss_evc_counters_clear(NULL, evc_id, pit.iport);
            } else {
                req->evc.value = evc_id;
                req->iport = pit.iport;
                req->uport = pit.uport;
                evc_icli_show_stats(req);
            }
        }
        if (!next)
            break;
    }
    if (!req->header) {
        req->header = 1;
        ICLI_PRINTF("\n");
    }

    /* ECE statistics */
    if (req->ece.value) {
        /* Specific ECE */
        ece->id = req->ece.value;
        next = 0;
    } else {
        /* All EVCs */
        ece->id = EVC_ECE_ID_FIRST;
        next = 1;
    }
    req->evc.valid = 0;
    while (req->ece.valid && evc_mgmt_ece_get(ece->id, &ece_conf, next) == VTSS_RC_OK) {
        evc_id = ece->action.evc_id;
        if (evc_id == VTSS_EVC_ID_NONE || evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_RC_OK) {
            memset(&evc_conf, 0, sizeof(evc_conf));
        }

        evc_icli_port_iter_init(&pit);
        while (icli_port_iter_getnext(&pit, req->port_list)) {
            if (key->port_list[pit.iport] == VTSS_ECE_PORT_NONE && pb->nni[pit.iport] == 0) {
                continue;
            }
            
            if (req->clear) {
                (void)vtss_ece_counters_clear(NULL, ece->id, pit.iport);
            } else {
                req->ece.value = ece->id;
                req->iport = pit.iport;
                req->uport = pit.uport;
                evc_icli_show_stats(req);
            }
        }
        if (!next)
            break;
    }
}
#else
static void evc_icli_stat_port(evc_icli_req_t *req, const char *col1, const char *col2,
                               vtss_port_counter_t c1, vtss_port_counter_t c2)
{
    u32  session_id = req->session_id;
    char buf[80], *p = buf;

    if (req->header) {
        req->header = 0;
        p += sprintf(p, "%-25s%-7sRx %-19s", "Interface", "Class", col1);
        if (col2 != NULL) {
            if (strlen(col2) == 0)
                sprintf(p, "Tx %-19s", col1);
            else
                sprintf(p, "%-22s", col2);
        }
        icli_table_header(session_id, buf);
    }
    ICLI_PRINTF("%-25s%-7u%-22llu", evc_icli_port_txt(req, buf), req->prio, c1);
    if (col2 != NULL)
        ICLI_PRINTF("%llu", c2);
    ICLI_PRINTF("\n");
}

/* Print two counters in columns */
static void evc_icli_stats(evc_icli_req_t *req, const char *col1, BOOL col2, 
                           vtss_port_counter_t c1, vtss_port_counter_t c2)
{
    u32  session_id = req->session_id;
    char buf[80];
    
    sprintf(buf, "%s:", col1);
    ICLI_PRINTF("Rx %-15s%20llu   ", buf, c1);
    if (col2) {
        ICLI_PRINTF("Tx %-15s%20llu", buf, c2);
    }
    ICLI_PRINTF("\n");
}

void evc_icli_evc_stats(evc_icli_req_t *req)
{
    u32                      session_id = req->session_id;
    vtss_port_counters_t     counters;
    vtss_port_evc_counters_t *evc = &counters.evc;
    port_iter_t              pit;
    icli_unsigned_range_t    *list = req->cos_list;
    BOOL                     prio_list[VTSS_PRIO_ARRAY_SIZE];
    vtss_prio_t              prio;
    uint                     i, j;
    char                     buf[80];

    /* Build list of priorities */
    for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++) {
        prio_list[prio] = (list ? 0 : 1);
    }
    for (i = 0; list != NULL && i < list->cnt; i++) {
        for (j = list->range[i].min; j <= list->range[i].max; j++) {
            if (j < VTSS_PRIO_END)
                prio_list[j] = 1;
        }
    }

    evc_icli_port_iter_init(&pit);
    while (icli_port_iter_getnext(&pit, req->port_list)) {
        if (req->clear) {
            (void)vtss_port_counters_clear(NULL, pit.iport);
            continue;
        } else if (vtss_port_counters_get(NULL, pit.iport, &counters) != VTSS_RC_OK)
            continue;

        for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++) {
            if (prio_list[prio] == 0)
                continue;
            req->prio = prio;
            req->uport = pit.uport;

            if (req->green) {
                evc_icli_stat_port(req, "Green", "", evc->rx_green[prio], evc->tx_green[prio]);
            } else if (req->yellow) {
                evc_icli_stat_port(req, "Yellow", "", evc->rx_yellow[prio], evc->tx_yellow[prio]);
            } else if (req->red) {
                evc_icli_stat_port(req, "Red", NULL, evc->rx_red[prio], 0);
            } else if (req->discard) {
                evc_icli_stat_port(req, "Green Discard", "Rx Yellow Discard", 
                                   evc->rx_green_discard[prio], evc->rx_yellow_discard[prio]);
            } else {
                /* Detailed statistics */
                ICLI_PRINTF("%sInterface %s, Class %u Statistics:\n\n",
                            req->header ? "" : "\n", evc_icli_port_txt(req, buf), prio);
                req->header = 0;
                evc_icli_stats(req, "Green", 1,  evc->rx_green[prio], evc->tx_green[prio]);
                evc_icli_stats(req, "Yellow", 1,  evc->rx_yellow[prio], evc->tx_yellow[prio]);
                evc_icli_stats(req, "Red", 0,  evc->rx_red[prio], 0);
                evc_icli_stats(req, "Green Discard", 0, evc->rx_green_discard[prio], 0);
                evc_icli_stats(req, "Yellow Discard", 0, evc->rx_yellow_discard[prio], 0);
            }
        }
    }
}
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

void evc_icli_evc_policer(evc_icli_req_t *req)
{
    vtss_evc_policer_id_t  policer_id = (req->pol.value - 1);
    evc_mgmt_policer_conf_t pol_conf;
    vtss_evc_policer_conf_t *conf = &pol_conf.conf;
    vtss_evc_policer_conf_t *new_conf = &req->pol_conf;
    
    if (req->update) {
        /* Update existing entry */
        if (evc_mgmt_policer_conf_get(policer_id, &pol_conf) != VTSS_RC_OK)
            return;
    } else {
        /* Get defaults */
        evc_mgmt_policer_conf_get_default(&pol_conf);
    }
    
    if (req->pol.type) {
        conf->type = (req->pol.mef ? VTSS_POLICER_TYPE_MEF : VTSS_POLICER_TYPE_SINGLE);
    }

    if (req->pol.enable) {
        conf->enable = 1;
    } else if (req->pol.disable) {
        conf->enable = 0;
    }
        
    if (req->pol.coupled) {
#if defined(VTSS_ARCH_JAGUAR_1)
        conf->cm = 1;
#endif /* VTSS_ARCH_JAGUAR_1 */
        conf->cf = 1;
    } else if (req->pol.aware) {
#if defined(VTSS_ARCH_JAGUAR_1)
        conf->cm = 1;
#endif /* VTSS_ARCH_JAGUAR_1 */
        conf->cf = 0;
    } else if (req->pol.blind) {
#if defined(VTSS_ARCH_JAGUAR_1)
        conf->cm = 0;
#endif /* VTSS_ARCH_JAGUAR_1 */
        conf->cf = 0;
    }

    if (req->pol.rate_type.valid) {
        conf->line_rate = req->pol.rate_type.value;
    }

    if (req->pol.cir) {
        conf->cir = new_conf->cir;
    }
    if (req->pol.cbs) {
        conf->cbs = new_conf->cbs;
    }
    if (req->pol.eir) {
        conf->eir = new_conf->eir;
    }
    if (req->pol.ebs) {
        conf->ebs = new_conf->ebs;
    }
    (void)evc_mgmt_policer_conf_set(policer_id, &pol_conf);
}

void evc_icli_evc_add(evc_icli_req_t *req)
{
    u32                session_id = req->session_id;
    vtss_evc_id_t      evc_id = evc_icli_id_get(req);
    evc_mgmt_conf_t    conf;
    vtss_evc_conf_t    *evc = &conf.conf;
    vtss_evc_pb_conf_t *pb = &evc->network.pb;
    evc_port_info_t    info[VTSS_PORT_ARRAY_SIZE];
    port_iter_t        pit;
    char               buf[80];
    
    if (req->update) {
        /* Update existing entry */
        if (evc_mgmt_get(&evc_id, &conf, 0) != VTSS_RC_OK) {
            ICLI_PRINTF("EVC %u does not exist\n", req->evc.value);
            return;
        }
    } else {
        /* Get defaults */
        evc_mgmt_get_default(&conf);
    }
    
    /* VLAN ID */
    if (req->vid.valid)
        pb->vid = req->vid.value;
    
    /* Internal VLAN ID */
    if (req->ivid.valid)
        pb->ivid = req->ivid.value;

    /* NNI list */
    if (req->port_list) {
        if (evc_mgmt_port_info_get(info) != VTSS_OK) {
            ICLI_PRINTF("Port info failed\n");
            return;
        }
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            pb->nni[pit.iport] = 0;
        }
        evc_icli_port_iter_init(&pit);
        while (icli_port_iter_getnext(&pit, req->port_list)) {
            if (info[pit.iport].uni_count) {
                req->uport = iport2uport(pit.iport);
                ICLI_PRINTF("%s is a UNI\n", evc_icli_port_txt(req, buf));
                return;
            }
            pb->nni[pit.iport] = 1;
        }
    }

    /* Learning */
    if (req->learning.valid)
        evc->learning = (req->learning.value ? 0 : 1);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Policer ID */
    if (req->pol.valid)
        evc->policer_id = evc_icli_policer_id(req);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_CARACAL)
    /* Inner tag */
    if (req->it.a.type.valid)
        pb->inner_tag.type = (req->it.a.type.untag ? VTSS_EVC_INNER_TAG_NONE :
                              req->it.a.type.c_tag ? VTSS_EVC_INNER_TAG_C :
                              req->it.a.type.s_tag ? VTSS_EVC_INNER_TAG_S : 
                              VTSS_EVC_INNER_TAG_S_CUSTOM);

    if (req->tag_inner.valid)
        pb->inner_tag.vid_mode = (req->tag_inner.value ? VTSS_EVC_VID_MODE_TUNNEL : 
                                  VTSS_EVC_VID_MODE_NORMAL);

    if (req->it.a.vid.valid)
        pb->inner_tag.vid = req->it.a.vid.value;

    if (req->it.a.pcp.mode)
        pb->inner_tag.pcp_dei_preserve = (req->it.a.pcp.fixed ? 0 : 1);

    if (req->it.a.pcp.valid)
        pb->inner_tag.pcp = req->it.a.pcp.value;

    if (req->it.a.dei.valid)
        pb->inner_tag.dei = req->it.a.dei.value;

    /* Outer tag */
    if (req->ot.a.vid.valid)
        pb->uvid = req->ot.a.vid.value;
#endif /* VTSS_ARCH_CARACAL */

    /* Add/modify entry */
    if (evc_mgmt_add(evc_id, &conf) != VTSS_OK) {
        ICLI_PRINTF("EVC configuration failed\n");
    }
}

void evc_icli_evc_del(evc_icli_req_t *req)
{
    u32             session_id = req->session_id;
    vtss_evc_id_t   evc_id = evc_icli_id_get(req);
    evc_mgmt_conf_t evc_conf;

    if (evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_OK) {
        ICLI_PRINTF("EVC %u does not exist\n", req->evc.value);
    } else if (evc_mgmt_del(evc_id) != VTSS_OK) {
        ICLI_PRINTF("EVC delete failed\n");
    }
}

static void evc_icli_range_match(evc_icli_range_t *range, vtss_vcap_vr_t *vr, u16 min, u16 max)
{
    if (!range->valid)
        return;

    if (range->any || (range->value.low == min && range->value.high == max)) {
        /* Match any value */
        vr->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
        vr->vr.v.value = 0;
        vr->vr.v.mask = 0;
    } else {
        /* Match range */
        vr->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
        vr->vr.r.low = range->value.low;
        vr->vr.r.high = range->value.high;
    }
}

static void evc_icli_dscp_match(vtss_vcap_vr_t *vcap_vr, evc_icli_range_t *range)
{
    evc_icli_range_match(range, vcap_vr, 0, 63);
}

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static void evc_icli_port_match(vtss_vcap_vr_t *vcap_vr, evc_icli_range_t *range)
{
    evc_icli_range_match(range, vcap_vr, 0, 0xffff);
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

static void evc_icli_tag_match(vtss_ece_tag_t *key, evc_icli_tag_t *req)
{
    icli_unsigned_range_t *list = req->m.pcp.list;
    uint                  min;

    if (req->m.type.valid) {
        key->tagged = (req->m.type.any ? VTSS_VCAP_BIT_ANY :
                       req->m.type.untag ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_1);
        key->s_tagged = (req->m.type.s_tag ? VTSS_VCAP_BIT_1 :
                         req->m.type.c_tag ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
    }
    

    evc_icli_range_match(&req->m.vid, &key->vid, 0, VLAN_ID_MAX);

    if (list != NULL && list->cnt) {
        min = list->range[0].min;
        key->pcp.value = min;
        key->pcp.mask = (7 - list->range[0].max + min);
    }
    if (req->m.pcp.any) {
        key->pcp.value = 0;
        key->pcp.mask = 0;
    }

    if (req->m.dei.valid)
        key->dei = (req->m.dei.any ? VTSS_VCAP_BIT_ANY :
                    req->m.dei.value ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0);
}

typedef struct {
    vtss_vid_t          *vid;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ece_pcp_mode_t *pcp_mode;
    vtss_ece_dei_mode_t *dei_mode;
#else
    BOOL                *pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
    vtss_tagprio_t      *pcp;
    vtss_dei_t          *dei;
} evc_icli_tag_add_t;

static void evc_icli_tag_add(evc_icli_tag_add_t *tag, evc_icli_tag_t *req)
{
    if (req->a.vid.valid && tag->vid != NULL)
        *tag->vid = req->a.vid.value;

#if defined(VTSS_ARCH_SERVAL)
    if (req->a.pcp.mode)
        *tag->pcp_mode = (req->a.pcp.classified ? VTSS_ECE_PCP_MODE_CLASSIFIED :
                          req->a.pcp.fixed ? VTSS_ECE_PCP_MODE_FIXED : VTSS_ECE_PCP_MODE_MAPPED);
    
    if (req->a.dei.mode)
        *tag->dei_mode = (req->a.dei.classified ? VTSS_ECE_DEI_MODE_CLASSIFIED :
                          req->a.dei.fixed ? VTSS_ECE_DEI_MODE_FIXED : VTSS_ECE_DEI_MODE_DP);
#else
    if (req->a.preserve.valid)
        *tag->pcp_dei_preserve = (req->a.preserve.value ? 0 : 1);
#endif /* VTSS_ARCH_SERVAL */
    
    if (req->a.pcp.valid)
        *tag->pcp = req->a.pcp.value;

    if (req->a.dei.valid)
        *tag->dei = req->a.dei.value;
}

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
static void evc_icli_mac_match(vtss_vcap_u48_t *key, evc_icli_mac_t *req)
{
    int i;
    u8  mask = (req->any || req->unicast || req->multicast || req->broadcast ? 0x00 : 0xff);

    if (req->valid) {
        for (i = 0; i < 6; i++) {
            key->value[i] = req->value.addr[i];
            key->mask[i] = mask;
        }
    }
}

static void evc_icli_proto_match(vtss_vcap_u8_t *key, evc_icli_u32_t *req)
{
    if (req->valid) {
        key->value = (req->tcp ? 6 : req->udp ? 17 : req->value);
        key->mask = (req->any ? 0x00 : 0xff);
    }
}

static void evc_icli_ipv4_match(vtss_vcap_ip_t *key, evc_icli_ipv4_t *req)
{
    if (req->valid) {
        key->value = req->value.ip;
        key->mask = req->value.netmask;
    }
}

static void evc_icli_ipv6_match(vtss_vcap_u128_t *key, evc_icli_ipv4_t *req)
{
    int i, j;

    if (req->valid) {
        for (i = 0; i < 4; i++) {
            j = ((3 - i) * 8);
            key->value[i + 12] = ((req->value.ip >> j) & 0xff);
            key->mask[i + 12] = ((req->value.netmask >> j) & 0xff);
        }
    }
}
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
static void evc_icli_vcap_match(u8 *val, u8 *msk, evc_icli_vcap_u32_t *req, u8 count)
{
    u32 i, j, n;

    if (req->valid) {
        for (i = 0; i < count; i++) {
            j = (count - i - 1);
            n = (i * 8);
            val[j] = ((req->value >> n) & 0xff);
            msk[j] = ((req->mask >> n) & 0xff);
        }
    }
}

static void evc_icli_u8_match(u8 *val, u8 *msk, evc_icli_vcap_u32_t *req)
{
    evc_icli_vcap_match(val, msk, req, 1);
}

static void evc_icli_u16_match(u8 *val, u8 *msk, evc_icli_vcap_u32_t *req)
{
    evc_icli_vcap_match(val, msk, req, 2);
}
#endif /* VTSS_ARCH_SERVAL */

void evc_icli_ece_add(evc_icli_req_t *req)
{
    u32                    session_id = req->session_id;
    vtss_ece_id_t          ece_id = req->ece.value, ece_id_next = VTSS_ECE_ID_LAST;
    evc_mgmt_ece_conf_t    conf;
    vtss_ece_t             *ece = &conf.conf;
    vtss_ece_key_t         *key = &ece->key;
    vtss_ece_frame_ipv4_t  *ipv4 = &key->frame.ipv4;
    vtss_ece_frame_ipv6_t  *ipv6 = &key->frame.ipv6;
#if defined(VTSS_ARCH_SERVAL)
    vtss_ece_frame_etype_t *etype = &key->frame.etype;
    vtss_ece_frame_llc_t   *llc = &key->frame.llc;
    vtss_ece_frame_snap_t  *snap = &key->frame.snap;
    evc_icli_l2cp_t        *l2cp = &req->frame.l2cp;
#endif /* VTSS_ARCH_SERVAL */
    vtss_ece_action_t      *action = &ece->action;
    vtss_ece_outer_tag_t   *ot = &action->outer_tag;
    evc_icli_tag_add_t     tag;
    vtss_ece_type_t        frame_type;
    evc_port_info_t        info[VTSS_PORT_ARRAY_SIZE];
    port_iter_t            pit;
    char                   buf[80];
    BOOL                   changed;
    
    /* Determine next ID */
    if (req->ece_next.valid) {
        if (req->ece_next.value) {
            /* Next ID is specified, check that it is valid */
            ece_id_next = req->ece_next.value;
            if (evc_mgmt_ece_get(ece_id_next, &conf, 0) != VTSS_RC_OK) {
                ICLI_PRINTF("ECE ID %u does not exist\n", ece_id_next);
                return;
            }
        }
    } else if (evc_mgmt_ece_get(ece_id, &conf, 1) == VTSS_RC_OK) {
        /* Next ID not specified, preserve the ordering */
        ece_id_next = ece->id;
    }

    if (req->update) {
        /* Update existing entry */
        if (evc_mgmt_ece_get(ece_id, &conf, 0) != VTSS_RC_OK) {
            ICLI_PRINTF("ECE ID %u does not exist\n", ece_id);
            return;
        }
    } else {
        /* Get defaults */
        evc_mgmt_ece_get_default(&conf);
        ece->id = ece_id;
    }
    
#if defined(VTSS_ARCH_SERVAL)
    /* Lookup */
    if (req->lookup.valid)
        key->lookup = (req->lookup.value ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */

    /* UNI list */
    if (req->port_list) {
        if (evc_mgmt_port_info_get(info) != VTSS_OK) {
            ICLI_PRINTF("Port info failed\n");
            return;
        }
        evc_icli_port_iter_init(&pit);
        while (icli_port_iter_getnext(&pit, req->port_list)) {
            if (info[pit.iport].nni_count) {
                req->uport = iport2uport(pit.iport);
                ICLI_PRINTF("%s is an NNI\n", evc_icli_port_txt(req, buf));
                return;
            }
            key->port_list[pit.iport] = VTSS_ECE_PORT_ROOT;
        }
    }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    /* SMAC */
    evc_icli_mac_match(&key->mac.smac, &req->smac);

#if defined(VTSS_ARCH_SERVAL)
    /* DMAC */
    evc_icli_mac_match(&key->mac.dmac, &req->dmac);
#endif /* VTSS_ARCH_SERVAL */

    /* DMAC type */
    if (req->dmac.valid) {
        key->mac.dmac_mc = ((req->dmac.multicast || req->dmac.broadcast) ? VTSS_VCAP_BIT_1 :
                            req->dmac.unicast ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
        key->mac.dmac_bc = ((req->dmac.unicast || req->dmac.multicast) ? VTSS_VCAP_BIT_0 :
                            req->dmac.broadcast ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_ANY);
    }
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    
    /* Outer tag matching */
    evc_icli_tag_match(&key->tag, &req->ot);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Inner tag matching */
    evc_icli_tag_match(&key->inner_tag, &req->it);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    /* Frame type */
    if (req->frame.valid) {
        frame_type = (req->frame.ipv4 ? VTSS_ECE_TYPE_IPV4 :
                      req->frame.ipv6 ? VTSS_ECE_TYPE_IPV6 :
#if defined(VTSS_ARCH_SERVAL)
                      req->frame.etype.valid ? VTSS_ECE_TYPE_ETYPE :
                      req->frame.llc.valid ? VTSS_ECE_TYPE_LLC :
                      req->frame.snap.valid ? VTSS_ECE_TYPE_SNAP :
#endif /* VTSS_ARCH_SERVAL */
                      VTSS_ECE_TYPE_ANY);
        changed = (frame_type == key->type ? 0 : 1);
#if defined(VTSS_ARCH_SERVAL)
        if (conf.data.l2cp.proto != EVC_L2CP_NONE && !l2cp->valid) {
            /* Change from L2CP to non-L2CP, clear frame data */
            conf.data.l2cp.proto = EVC_L2CP_NONE;
            changed = 1;
        }
#endif /* VTSS_ARCH_SERVAL */
        if (changed)
            memset(&key->frame, 0, sizeof(key->frame));
        key->type = frame_type;
    }

    switch (key->type) {
#if defined(VTSS_ARCH_SERVAL)
    case VTSS_ECE_TYPE_ETYPE:
        evc_icli_u16_match(etype->etype.value, etype->etype.mask, &req->frame.etype.etype);
        evc_icli_u16_match(etype->data.value, etype->data.mask, &req->frame.etype.data);
        break;
    case VTSS_ECE_TYPE_LLC:
        evc_icli_u8_match(&llc->data.value[0], &llc->data.mask[0], &req->frame.llc.dsap);
        evc_icli_u8_match(&llc->data.value[1], &llc->data.mask[1], &req->frame.llc.ssap);
        evc_icli_u8_match(&llc->data.value[2], &llc->data.mask[2], &req->frame.llc.control);
        evc_icli_u16_match(&llc->data.value[3], &llc->data.mask[3], &req->frame.llc.data);
        break;
    case VTSS_ECE_TYPE_SNAP:
        evc_icli_vcap_match(&snap->data.value[0], &snap->data.mask[0], &req->frame.snap.oui, 3);
        evc_icli_u16_match(&snap->data.value[3], &snap->data.mask[3], &req->frame.snap.pid);
        break;
#endif /* VTSS_ARCH_SERVAL */
    case VTSS_ECE_TYPE_IPV4:
        evc_icli_dscp_match(&ipv4->dscp, &req->frame.dscp);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        evc_icli_proto_match(&ipv4->proto, &req->frame.proto);
        evc_icli_ipv4_match(&ipv4->sip, &req->frame.sip);
#if defined(VTSS_ARCH_SERVAL)
        evc_icli_ipv4_match(&ipv4->dip, &req->frame.dip);
#endif /* VTSS_ARCH_SERVAL */
        if (req->frame.frag.valid)
            ipv4->fragment = (req->frame.frag.yes ? VTSS_VCAP_BIT_1 :
                              req->frame.frag.no ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
        evc_icli_port_match(&ipv4->sport, &req->frame.sport);
        evc_icli_port_match(&ipv4->dport, &req->frame.dport);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    case VTSS_ECE_TYPE_IPV6:
        evc_icli_dscp_match(&ipv6->dscp, &req->frame.dscp);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        evc_icli_proto_match(&ipv6->proto, &req->frame.proto);
        evc_icli_ipv6_match(&ipv6->sip, &req->frame.sip);
#if defined(VTSS_ARCH_SERVAL)
        evc_icli_ipv6_match(&ipv6->dip, &req->frame.dip);
#endif /* VTSS_ARCH_SERVAL */
        evc_icli_port_match(&ipv6->sport, &req->frame.sport);
        evc_icli_port_match(&ipv6->dport, &req->frame.dport);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    default:
#if defined(VTSS_ARCH_SERVAL)
        if (l2cp->valid) {
            /* Change from L2CP to non-L2CP, clear frame data */
            conf.data.l2cp.proto = (l2cp->stp ? EVC_L2CP_STP :
                                    l2cp->pause ? EVC_L2CP_PAUSE :
                                    l2cp->lacp ? EVC_L2CP_LACP :
                                    l2cp->lamp ? EVC_L2CP_LAMP :
                                    l2cp->loam ? EVC_L2CP_LOAM :
                                    l2cp->dot1x ? EVC_L2CP_DOT1X :
                                    l2cp->elmi ? EVC_L2CP_ELMI :
                                    l2cp->pb ? EVC_L2CP_PB :
                                    l2cp->pb_gvrp ? EVC_L2CP_PB_GVRP :
                                    l2cp->lldp ? EVC_L2CP_LLDP :
                                    l2cp->gmrp ? EVC_L2CP_GMRP :
                                    l2cp->gvrp ? EVC_L2CP_GVRP :
                                    l2cp->uld ? EVC_L2CP_ULD :
                                    l2cp->pagp ? EVC_L2CP_PAGP :
                                    l2cp->pvst ? EVC_L2CP_PVST :
                                    l2cp->cisco_vlan ? EVC_L2CP_CISCO_VLAN :
                                    l2cp->cdp ? EVC_L2CP_CDP :
                                    l2cp->vtp ? EVC_L2CP_VTP :
                                    l2cp->dtp ? EVC_L2CP_DTP :
                                    l2cp->cisco_stp ? EVC_L2CP_CISCO_STP :
                                    l2cp->cisco_cfm ? EVC_L2CP_CISCO_CFM : EVC_L2CP_NONE);
        }
#endif /* VTSS_ARCH_SERVAL */
        break;
    }

    /* Direction */
    if (req->dir.valid)
        action->dir = (req->dir.uni_to_nni ? VTSS_ECE_DIR_UNI_TO_NNI :
                       req->dir.nni_to_uni ? VTSS_ECE_DIR_NNI_TO_UNI : VTSS_ECE_DIR_BOTH);

#if defined(VTSS_ARCH_SERVAL)
    /* Rule type */
    if (req->rule.valid)
        action->rule = (req->rule.rx ? VTSS_ECE_RULE_RX :
                        req->rule.tx ? VTSS_ECE_RULE_TX : VTSS_ECE_RULE_BOTH);
    
    /* Tx lookup */
    if (req->tx.valid)
        action->tx_lookup = (req->tx.vid ? VTSS_ECE_TX_LOOKUP_VID :
                             req->tx.pcp_vid ? VTSS_ECE_TX_LOOKUP_VID_PCP :
                             VTSS_ECE_TX_LOOKUP_ISDX);
#endif /* VTSS_ARCH_SERVAL */

    /* EVC mapping */
    if (req->evc.valid)
        action->evc_id = evc_icli_id_get(req);

#if defined(VTSS_ARCH_SERVAL)
    /* L2CP mode */
    if (req->l2cp_mode.valid)
        conf.data.l2cp.mode = (req->l2cp_mode.tunnel ? EVC_L2CP_MODE_TUNNEL :
                               req->l2cp_mode.peer ? EVC_L2CP_MODE_PEER :
                               req->l2cp_mode.discard ? EVC_L2CP_MODE_DISCARD : EVC_L2CP_MODE_FORWARD);

    if (req->l2cp_dmac.valid)
        conf.data.l2cp.dmac = (req->l2cp_dmac.cisco ? EVC_L2CP_DMAC_CISCO : EVC_L2CP_DMAC_CUSTOM);
#endif /* VTSS_ARCH_SERVAL */

    /* Pop count */
    if (req->pop.valid)
        action->pop_tag = (req->pop.value == 2 ? VTSS_ECE_POP_TAG_2 :
                           req->pop.value == 1 ? VTSS_ECE_POP_TAG_1 : VTSS_ECE_POP_TAG_0); 

    /* ACL policy number */
    if (req->policy.valid)
        action->policy_no = req->policy.value;

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    /* Class */
    if (req->cos.valid) {
        action->prio_enable = (req->cos.disable ? 0 : 1);
        action->prio = req->cos.value;
    }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
    /* Drop precedence */
    if (req->dpl.valid) {
        action->dp_enable = (req->dpl.disable ? 0 : 1);
        action->dp = req->dpl.value;
    }
#endif /* VTSS_ARCH_SERVAL */

    /* Outer tag adding */
    if (req->ot.a.enable || req->ot.a.disable)
        ot->enable = req->ot.a.enable;
    
#if defined(VTSS_ARCH_CARACAL)
    tag.vid = NULL;
#else
    tag.vid = &ot->vid;
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_SERVAL)
    tag.pcp_mode = &ot->pcp_mode;
    tag.dei_mode = &ot->dei_mode;
#else
    tag.pcp_dei_preserve = &ot->pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
    tag.pcp = &ot->pcp;
    tag.dei = &ot->dei;
    evc_icli_tag_add(&tag, &req->ot);
        
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Policer mapping */
    if (req->pol.valid)
        action->policer_id = evc_icli_policer_id(req);

    /* Inner tag */
    {
        vtss_ece_inner_tag_t *it = &action->inner_tag;

        if (req->it.a.type.valid)
            it->type = (req->it.a.type.untag ? VTSS_ECE_INNER_TAG_NONE :
                        req->it.a.type.c_tag ? VTSS_ECE_INNER_TAG_C :
                        req->it.a.type.s_tag ? VTSS_ECE_INNER_TAG_S : VTSS_ECE_INNER_TAG_S_CUSTOM);
        
        tag.vid = &it->vid;
#if defined(VTSS_ARCH_SERVAL)
        tag.pcp_mode = &it->pcp_mode;
        tag.dei_mode = &it->dei_mode;
#else
        tag.pcp_dei_preserve = &it->pcp_dei_preserve;
#endif /* VTSS_ARCH_SERVAL */
        tag.pcp = &it->pcp;
        tag.dei = &it->dei;
        evc_icli_tag_add(&tag, &req->it);
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    /* Add/modify ECE */
    if (evc_mgmt_ece_add(ece_id_next, &conf) != VTSS_RC_OK) {
        ICLI_PRINTF("ECE add failed\n");
    }
}

void evc_icli_ece_del(evc_icli_req_t *req)
{
    u32                 session_id = req->session_id;
    vtss_ece_id_t       ece_id = req->ece.value;
    evc_mgmt_ece_conf_t conf;

    if (evc_mgmt_ece_get(ece_id, &conf, 0) != VTSS_RC_OK) {
        ICLI_PRINTF("ECE %u does not exist\n", ece_id);
    } else if (evc_mgmt_ece_del(ece_id) != VTSS_RC_OK) {
        ICLI_PRINTF("ECE delete failed\n");
    }
}

void evc_icli_l2cp_list(evc_mgmt_port_conf_t *conf, icli_unsigned_range_t *list,
                        vtss_packet_reg_type_t type)
{
    uint i, j;

    if (list != NULL) {
        for (i = 0; i < list->cnt; i++) {
            for (j = list->range[i].min; j <= list->range[i].max; j++) {
                if (j < 16)
                    conf->reg.bpdu_reg[j] = type;
                else if (j < 32)
                    conf->reg.garp_reg[j - 16] = type;
            }
        }
    }
}

#if defined(VTSS_ARCH_SERVAL)
void evc_icli_key_type(vtss_vcap_key_type_t *key_type, evc_icli_key_t *key)
{
    if (key->valid)
        *key_type = (key->double_tag ? VTSS_VCAP_KEY_TYPE_DOUBLE_TAG :
                     key->normal ? VTSS_VCAP_KEY_TYPE_NORMAL :
                     key->ip_addr ? VTSS_VCAP_KEY_TYPE_IP_ADDR :
                     VTSS_VCAP_KEY_TYPE_MAC_IP_ADDR);
}
#endif /* VTSS_ARCH_SERVAL */

void evc_icli_port(evc_icli_req_t *req)
{
    u32                  session_id = req->session_id;
    port_iter_t          pit;
    evc_mgmt_port_conf_t port_conf;
    vtss_evc_port_conf_t *conf = &port_conf.conf;
    char                 buf[80];

    evc_icli_port_iter_init(&pit);
    while (icli_port_iter_getnext(&pit, req->port_list)) {
        if (req->update) {
            /* Update existing entry */
            if (evc_mgmt_port_conf_get(pit.iport, &port_conf) != VTSS_RC_OK)
                continue;
        } else {
            /* Get defaults */
            evc_mgmt_port_conf_get_default(&port_conf);
        }

#if defined(VTSS_ARCH_SERVAL)
        evc_icli_key_type(&conf->key_type, &req->key);
        evc_icli_key_type(&port_conf.vcap_conf.key_type_is1_1, &req->key_adv);
        if (req->addr_adv.valid)
            port_conf.vcap_conf.dmac_dip_1 = req->addr_adv.value;
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
        if (req->dei_colour.valid)
            conf->dei_colouring = req->dei_colour.value;
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */

#if defined(VTSS_ARCH_CARACAL)
        if (req->tag_inner.valid)
            conf->inner_tag = req->tag_inner.value;
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        if (req->addr.valid)
            conf->dmac_dip = req->addr.value;
#endif /* VTSS_ARCH_CARACAL/SERVAL */

        evc_icli_l2cp_list(&port_conf, req->l2cp.peer_list, VTSS_PACKET_REG_CPU_ONLY);
        evc_icli_l2cp_list(&port_conf, req->l2cp.forward_list, VTSS_PACKET_REG_FORWARD);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        evc_icli_l2cp_list(&port_conf, req->l2cp.discard_list, VTSS_PACKET_REG_DISCARD);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        if (evc_mgmt_port_conf_set(pit.iport, &port_conf) != VTSS_RC_OK) {
            req->uport = iport2uport(pit.iport);
            ICLI_PRINTF("EVC port configuration failed for %s\n", evc_icli_port_txt(req, buf));
        }
    }
}
