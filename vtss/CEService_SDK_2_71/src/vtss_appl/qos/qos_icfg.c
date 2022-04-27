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
#include "qos_api.h"
#include "qos_icfg.h"
#include "topo_api.h"
#include "misc_api.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/

#undef IC_RC
#define IC_RC ICLI_VTSS_RC

// Helper macros:
#define SHOW_(p)  ((req->all_defaults) || (c.p != dc.p))
#define PRT_(...) do { IC_RC(vtss_icfg_printf(result, __VA_ARGS__)); } while (0)

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

static vtss_rc QOS_ICFG_global_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    qos_conf_t dc;
    qos_conf_t c;

    IC_RC(qos_conf_get_default(&dc));
    IC_RC(qos_conf_get(&c));

#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
    {
        char buf[32];
        (void)qos_icfg_storm_rate_txt(c.policer_uc, buf);
        if (SHOW_(policer_uc_status)) {
            if (c.policer_uc_status) {
                PRT_("qos storm unicast %s\n", buf);
            } else {
                PRT_("no qos storm unicast\n");
            }
        }
    }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
    {
        char buf[32];
        (void)qos_icfg_storm_rate_txt(c.policer_mc, buf);
        if (SHOW_(policer_mc_status)) {
            if (c.policer_mc_status) {
                PRT_("qos storm multicast %s\n", buf);
            } else {
                PRT_("no qos storm multicast\n");
            }
        }
    }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
    {
        char buf[32];
        (void)qos_icfg_storm_rate_txt(c.policer_bc, buf);
        if (SHOW_(policer_bc_status)) {
            if (c.policer_bc_status) {
                PRT_("qos storm broadcast %s\n", buf);
            } else {
                PRT_("no qos storm broadcast\n");
            }
        }
    }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_WRED) || defined(VTSS_FEATURE_QOS_WRED_V2)
    {
        int i;
        for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
#if defined(VTSS_FEATURE_QOS_WRED)
            if (SHOW_(wred[i].enable)) {
                if (c.wred[i].enable) {
                    PRT_("qos wred queue %d min-th %u mdp-1 %u mdp-2 %u mdp-3 %u\n", i, c.wred[i].min_th, c.wred[i].max_prob_1, c.wred[i].max_prob_2, c.wred[i].max_prob_3);
                } else {
                    PRT_("no qos wred queue %d\n", i);
                }
            }
#elif defined(VTSS_FEATURE_QOS_WRED_V2)
            if (SHOW_(wred[i][1].enable)) {
                if (c.wred[i][1].enable) {
                    PRT_("qos wred queue %d min-fl %u max %u %s\n", i, c.wred[i][1].min_fl, c.wred[i][1].max, (c.wred[i][1].max_unit == VTSS_WRED_V2_MAX_FL) ? "fill-level" : "");
                } else {
                    PRT_("no qos wred queue %d\n", i);
                }
            }
#endif
        }
    }
#endif /* defined(VTSS_FEATURE_QOS_WRED) || defined(VTSS_FEATURE_QOS_WRED_V2) */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    {
        int i;
        for (i = 0; i < 64; i++) {
            if (SHOW_(dscp.dscp_trust[i])) {
                if (c.dscp.dscp_trust[i]) {
                    PRT_("qos map dscp-cos %d cos %u dpl %u\n", i, c.dscp.dscp_qos_class_map[i], c.dscp.dscp_dp_level_map[i]);
                } else {
                    PRT_("no qos map dscp-cos %d\n", i);
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    {
        int i;
        for (i = 0; i < 64; i++) {
            if (SHOW_(dscp.translate_map[i])) {
                PRT_("qos map dscp-ingress-translation %d to %u\n", i, c.dscp.translate_map[i]);
            }
        }

        for (i = 0; i < 64; i++) {
            if (SHOW_(dscp.ingress_remark[i])) {
                if (c.dscp.ingress_remark[i]) {
                    PRT_("qos map dscp-classify %d\n", i);
                } else {
                    PRT_("no qos map dscp-classify %d\n", i);
                }
            }
        }

        for (i = 0; i < VTSS_PRIO_ARRAY_SIZE; i++) {
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            if (SHOW_(dscp.qos_class_dscp_map[i])) {
                if (c.dscp.qos_class_dscp_map[i]) {
                    PRT_("qos map cos-dscp %d dpl 0 dscp %u\n", i, c.dscp.qos_class_dscp_map[i]);
                } else {
                    PRT_("no qos map cos-dscp %d dpl 0\n", i);
                }
            }
            if (SHOW_(dscp.qos_class_dscp_map_dp1[i])) {
                if (c.dscp.qos_class_dscp_map_dp1[i]) {
                    PRT_("qos map cos-dscp %d dpl 1 dscp %u\n", i, c.dscp.qos_class_dscp_map_dp1[i]);
                } else {
                    PRT_("no qos map cos-dscp %d dpl 1\n", i);
                }
            }
#else
            if (SHOW_(dscp.qos_class_dscp_map[i])) {
                if (c.dscp.qos_class_dscp_map[i]) {
                    PRT_("qos map cos-dscp %d dscp %u\n", i, c.dscp.qos_class_dscp_map[i]);
                } else {
                    PRT_("no qos map cos-dscp %d\n", i);
                }
            }
#endif
        }

        for (i = 0; i < 64; i++) {
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            if (SHOW_(dscp.egress_remap[i])) {
                PRT_("qos map dscp-egress-translation %d 0 to %u\n", i, c.dscp.egress_remap[i]);
            }
            if (SHOW_(dscp.egress_remap_dp1[i])) {
                PRT_("qos map dscp-egress-translation %d 1 to %u\n", i, c.dscp.egress_remap_dp1[i]);
            }
#else
            if (SHOW_(dscp.egress_remap[i])) {
                PRT_("qos map dscp-egress-translation %d to %u\n",   i, c.dscp.egress_remap[i]);
            }
#endif
        }
    }
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2) */
    return VTSS_OK;
}

static vtss_rc QOS_ICFG_port_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
    qos_port_conf_t dc;
    qos_port_conf_t c;
    vtss_isid_t     isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t  iport = uport2iport(req->instance_id.port.begin_uport);

    IC_RC(qos_port_conf_get_default(&dc));
    IC_RC(qos_port_conf_get(isid, iport, &c));

    if (SHOW_(default_prio)) {
        PRT_(" qos cos %u\n", c.default_prio);
    }
#if !defined(QOS_USE_FIXED_PCP_QOS_MAP)
    if (SHOW_(usr_prio)) {
        PRT_(" qos pcp %u\n", c.usr_prio);
    }
#endif /* !defined(QOS_USE_FIXED_PCP_QOS_MAP) */

#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
    if (SHOW_(default_dpl)) {
        PRT_(" qos dpl %u\n", c.default_dpl);
    }
#if !defined(QOS_USE_FIXED_PCP_QOS_MAP)
    if (SHOW_(default_dei)) {
        PRT_(" qos dei %u\n", c.default_dei);
    }
#endif /* !defined(QOS_USE_FIXED_PCP_QOS_MAP) */

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if !defined(QOS_USE_FIXED_PCP_QOS_MAP)
    if (SHOW_(tag_class_enable)) {
        if (c.tag_class_enable) {
            PRT_(" qos trust tag\n");
        } else {
            PRT_(" no qos trust tag\n");
        }
    }

    {
        int pcp, dei;
        for (pcp = 0; pcp < VTSS_PCP_ARRAY_SIZE; pcp++) {
            for (dei = 0; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
                if (SHOW_(qos_class_map[pcp][dei]) || SHOW_(dp_level_map[pcp][dei])) {
                    PRT_(" qos map tag-cos pcp %d dei %d cos %u dpl %u\n", pcp, dei, c.qos_class_map[pcp][dei], c.dp_level_map[pcp][dei]);
                }
            }
        }
    }
#endif /* !defined(QOS_USE_FIXED_PCP_QOS_MAP) */
    if (SHOW_(dscp_class_enable)) {
        if (c.dscp_class_enable) {
            PRT_(" qos trust dscp\n");
        } else {
            PRT_(" no qos trust dscp\n");
        }
    }
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

    {
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
        BOOL fr = c.port_policer_ext[0].frame_rate;
#else
        BOOL fr = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
        BOOL fc = c.port_policer_ext[0].flow_control;
#else
        BOOL fc = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC */

        if (SHOW_(port_policer[0].enabled)) {
            if (c.port_policer[0].enabled) {
                PRT_(" qos policer %u %s %s\n", c.port_policer[0].policer.rate, fr ? "fps" : "", fc ? "flowcontrol" : "");
            } else {
                PRT_(" no qos policer\n");
            }
        }
    }

#if defined(VTSS_SW_OPTION_BUILD_CE)
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
    {
        int i;
        for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
            if (SHOW_(queue_policer[i].enabled)) {
                if (c.queue_policer[i].enabled) {
                    PRT_(" qos queue-policer queue %d %u\n", i, c.queue_policer[i].policer.rate);
                } else {
                    PRT_(" no qos queue-policer queue %d\n", i);
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */

    if (SHOW_(shaper_status)) {
        if (c.shaper_status) {
            PRT_(" qos shaper %u\n", c.shaper_rate);
        } else {
            PRT_(" no qos shaper\n");
        }
    }

#ifdef VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS
    {
        int i;
        for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
            if (SHOW_(queue_shaper[i].enable)) {
                if (c.queue_shaper[i].enable) {
                    PRT_(" qos queue-shaper queue %d %u %s\n", i, c.queue_shaper[i].rate, c.excess_enable[i] ? "excess" : "");
                } else {
                    PRT_(" no qos queue-shaper queue %d\n", i);
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
    if (SHOW_(dwrr_enable)) {
        if (c.dwrr_enable) {
            PRT_(" qos wrr %u %u %u %u %u %u\n", c.queue_pct[0], c.queue_pct[1], c.queue_pct[2], c.queue_pct[3], c.queue_pct[4], c.queue_pct[5]);
        } else {
            PRT_(" no qos wrr\n");
        }
    }
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */


#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
    if (SHOW_(tag_remark_mode)) {
        switch (c.tag_remark_mode) {
        case VTSS_TAG_REMARK_MODE_CLASSIFIED:
            PRT_(" no qos tag-remark\n");
            break;
        case VTSS_TAG_REMARK_MODE_DEFAULT:
            PRT_(" qos tag-remark pcp %u dei %u\n", c.tag_default_pcp, c.tag_default_dei);
            break;
        case VTSS_TAG_REMARK_MODE_MAPPED: {
            PRT_(" qos tag-remark mapped\n");
            break;
        }
        default:
            break;
        }
    }

    {
        int cos, dpl;
        for (cos = 0; cos < QOS_PORT_PRIO_CNT; cos++) {
            for (dpl = 0; dpl < 2; dpl++) {
                if (SHOW_(tag_pcp_map[cos][dpl]) || SHOW_(tag_dei_map[cos][dpl])) {
                    PRT_(" qos map cos-tag cos %d dpl %d pcp %u dei %u\n", cos, dpl, c.tag_pcp_map[cos][dpl], c.tag_dei_map[cos][dpl]);
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    if (SHOW_(dscp_translate)) {
        if (c.dscp_translate) {
            PRT_(" qos dscp-translate\n");
        } else {
            PRT_(" no qos dscp-translate\n");
        }
    }

    if (SHOW_(dscp_imode)) {
        switch (c.dscp_imode) {
        case VTSS_DSCP_MODE_NONE:
            PRT_(" no qos dscp-classify\n");
            break;
        case VTSS_DSCP_MODE_ZERO:
            PRT_(" qos dscp-classify zero\n");
            break;
        case VTSS_DSCP_MODE_SEL:
            PRT_(" qos dscp-classify selected\n");
            break;
        case VTSS_DSCP_MODE_ALL:
            PRT_(" qos dscp-classify any\n");
            break;
        default:
            break;
        }
    }

    if (SHOW_(dscp_emode)) {
        switch (c.dscp_emode) {
        case VTSS_DSCP_EMODE_DISABLE:
            PRT_(" no qos dscp-remark\n");
            break;
        case VTSS_DSCP_EMODE_REMARK:
            PRT_(" qos dscp-remark rewrite\n");
            break;
        case VTSS_DSCP_EMODE_REMAP:
            PRT_(" qos dscp-remark remap\n");
            break;
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
        case VTSS_DSCP_EMODE_REMAP_DPA:
            PRT_(" qos dscp-remark remap-dp\n");
            break;
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
        default:
            break;
        }
    }
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_SW_OPTION_JR_STORM_POLICERS)
    {
        BOOL fr;

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
        fr = c.port_policer_ext[QOS_STORM_POLICER_UNICAST].frame_rate;
#else
        fr = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */

        if (SHOW_(port_policer[QOS_STORM_POLICER_UNICAST].enabled)) {
            if (c.port_policer[QOS_STORM_POLICER_UNICAST].enabled) {
                PRT_(" qos storm unicast %u %s\n", c.port_policer[QOS_STORM_POLICER_UNICAST].policer.rate, fr ? "fps" : "");
            } else {
                PRT_(" no qos storm unicast\n");
            }
        }

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
        fr = c.port_policer_ext[QOS_STORM_POLICER_BROADCAST].frame_rate;
#else
        fr = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */

        if (SHOW_(port_policer[QOS_STORM_POLICER_BROADCAST].enabled)) {
            if (c.port_policer[QOS_STORM_POLICER_BROADCAST].enabled) {
                PRT_(" qos storm broadcast %u %s\n", c.port_policer[QOS_STORM_POLICER_BROADCAST].policer.rate, fr ? "fps" : "");
            } else {
                PRT_(" no qos storm broadcast\n");
            }
        }

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
        fr = c.port_policer_ext[QOS_STORM_POLICER_UNKNOWN].frame_rate;
#else
        fr = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */

        if (SHOW_(port_policer[QOS_STORM_POLICER_UNKNOWN].enabled)) {
            if (c.port_policer[QOS_STORM_POLICER_UNKNOWN].enabled) {
                PRT_(" qos storm unknown %u %s\n", c.port_policer[QOS_STORM_POLICER_UNKNOWN].policer.rate, fr ? "fps" : "");
            } else {
                PRT_(" no qos storm unknown\n");
            }
        }
    }

#endif /* defined(VTSS_SW_OPTION_JR_STORM_POLICERS) */

#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
    if (SHOW_(dmac_dip)) {
        if (c.dmac_dip) {
            PRT_(" qos qce addr destination\n");
        } else {
            PRT_(" no qos qce addr\n");
        }
    }
#endif /* defined(VTSS_FEATURE_QCL_DMAC_DIP) */

#if defined(VTSS_ARCH_SERVAL)
    if (SHOW_(key_type)) {
        if (c.key_type != VTSS_VCAP_KEY_TYPE_NORMAL) {
            PRT_(" qos qce key %s\n", qos_icfg_key_type_txt(c.key_type));
        } else {
            PRT_(" no qos qce key\n");
        }
    }
#endif /* defined(VTSS_ARCH_SERVAL) */

    return VTSS_OK;
}

#if defined(VTSS_FEATURE_QCL_V2)

#if defined(VTSS_ARCH_SERVAL)
static vtss_rc QOS_ICFG_qce_range(vtss_icfg_query_result_t *result, char *name, vtss_vcap_vr_t *range)
{
    if (range->type != VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        IC_RC(vtss_icfg_printf(result, "%s %u-%u", name, range->vr.r.low, range->vr.r.high));
    } else if (range->vr.v.mask) {
        IC_RC(vtss_icfg_printf(result, "%s %u", name, range->vr.v.value));
    }
    return VTSS_OK;
}
#endif /* defined(VTSS_ARCH_SERVAL) */

static vtss_rc QOS_ICFG_qce_range_u16(vtss_icfg_query_result_t *result, char *name, qos_qce_vr_u16_t *range)
{
    if (range->in_range) {
        IC_RC(vtss_icfg_printf(result, "%s %u-%u", name, range->vr.r.low, range->vr.r.high));
    } else if (range->vr.v.mask) {
        IC_RC(vtss_icfg_printf(result, "%s %u", name, range->vr.v.value));
    }
    return VTSS_OK;
}

static vtss_rc QOS_ICFG_qce_range_u8(vtss_icfg_query_result_t *result, char *name, qos_qce_vr_u8_t *range)
{
    if (range->in_range) {
        IC_RC(vtss_icfg_printf(result, "%s %u-%u", name, range->vr.r.low, range->vr.r.high));
    } else if (range->vr.v.mask) {
        IC_RC(vtss_icfg_printf(result, "%s %u", name, range->vr.v.value));
    }
    return VTSS_OK;
}

static vtss_rc QOS_ICFG_qce_proto(vtss_icfg_query_result_t *result, char *name, vtss_qce_u8_t *proto)
{
    if (proto->mask) {
        if (proto->value == 6) {
            IC_RC(vtss_icfg_printf(result, "%s tcp", name));
        } else if (proto->value == 17) {
            IC_RC(vtss_icfg_printf(result, "%s udp", name));
        } else {
            IC_RC(vtss_icfg_printf(result, "%s %u", name, proto->value));
        }
    }
    return VTSS_OK;
}

static vtss_rc QOS_ICFG_qce_ipv4(vtss_icfg_query_result_t *result, char *name, vtss_qce_ip_t *ip)
{
    ulong i, n = 0;
    char  buf[64];

    if (ip->mask) {
        for (i = 0; i < 32; i++) {
            if (ip->mask & (1 << i)) {
                n++;
            }
        }
        (void)misc_ipv4_txt(ip->value, buf);
        sprintf(&buf[strlen(buf)], "/%d", n);
        IC_RC(vtss_icfg_printf(result, "%s %s", name, buf));
    }
    return VTSS_OK;
}

static vtss_rc QOS_ICFG_qce_ipv6(vtss_icfg_query_result_t *result, char *name, vtss_qce_u32_t *ip)
{
    ulong i, n = 0;

    if (ip->mask[0] || ip->mask[1] || ip->mask[2] || ip->mask[3]) {
        for (i = 0; i < 32; i++) {
            if (ip->mask[i / 8] & (1 << (i % 8))) {
                n++;
            }
        }
        IC_RC(vtss_icfg_printf(result, "%s %u.%u.%u.%u/%d", name, ip->value[0], ip->value[1], ip->value[2], ip->value[3], n));
    }
    return VTSS_OK;
}
#endif /* defined(VTSS_FEATURE_QCL_V2) */

static vtss_rc QOS_ICFG_qce_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
#if defined(VTSS_FEATURE_QCL_V2)
    vtss_qce_id_t        id = QCE_ID_NONE;
    qos_qce_entry_conf_t conf;
    char                 buf[ICLI_STR_MAX_LEN];
    BOOL                 port_list[VTSS_PORT_ARRAY_SIZE];

    while (qos_mgmt_qce_entry_get(VTSS_ISID_GLOBAL, QCL_USER_STATIC, QCL_ID_END, id, &conf, TRUE) == VTSS_OK) {
        vtss_port_no_t iport;
        u32            port_cnt;
        u8             bit_val;

        id = conf.id;
        // qce id:
        IC_RC(vtss_icfg_printf(result, "qos qce %u", id));

        // ingress:
        port_cnt = 0;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            if ((port_list[iport] = VTSS_PORT_BF_GET(conf.port_list, iport))) {
                port_cnt++;
            }
        }
        if (vtss_stacking_enabled()) {
            // currently no support for QCEs on stackable platforms
        } else {
            if (port_cnt != VTSS_PORT_NO_END) {
                IC_RC(vtss_icfg_printf(result, " interface %s", icli_port_list_info_txt(VTSS_ISID_START, port_list, buf, FALSE)));
            }
        }

        // tag:
        if ((QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_VLAN_TAG) != VTSS_VCAP_BIT_ANY) ||
            (conf.key.vid.in_range || conf.key.vid.vr.v.mask)                                  ||
            conf.key.pcp.mask                                                                  ||
            (QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_VLAN_DEI) != VTSS_VCAP_BIT_ANY)) {

            IC_RC(vtss_icfg_printf(result, " tag"));
            bit_val = QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_VLAN_TAG);
            if (bit_val != VTSS_VCAP_BIT_ANY) {
#if defined(VTSS_ARCH_SERVAL)
                IC_RC(vtss_icfg_printf(result, " type %s", qos_icfg_tag_type_text(bit_val, QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_VLAN_S_TAG))));
#else
                IC_RC(vtss_icfg_printf(result, " type %s", qos_icfg_tag_type_text(bit_val, VTSS_VCAP_BIT_ANY)));
#endif
            }

            // vid:
            IC_RC(QOS_ICFG_qce_range_u16(result, " vid", &conf.key.vid));

            // pcp:
            if (conf.key.pcp.mask) {
                if (conf.key.pcp.mask == 0xFF) {
                    IC_RC(vtss_icfg_printf(result, " pcp %u", conf.key.pcp.value));
                } else {
                    IC_RC(vtss_icfg_printf(result, " pcp %u-%u", conf.key.pcp.value, (uchar)(conf.key.pcp.value + ((~conf.key.pcp.mask) & 0xFF))));
                }
            }

            // dei:
            bit_val = QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_VLAN_DEI);
            if (bit_val != VTSS_VCAP_BIT_ANY) {
                IC_RC(vtss_icfg_printf(result, " dei %s", (bit_val == VTSS_VCAP_BIT_0) ? "0" : "1"));
            }
        }

#if defined(VTSS_ARCH_SERVAL)
        // inner tag:
        if ((conf.key.inner_tag.tagged != VTSS_VCAP_BIT_ANY)                                                    ||
            ((conf.key.inner_tag.vid.type != VTSS_VCAP_VR_TYPE_VALUE_MASK) || conf.key.inner_tag.vid.vr.v.mask) ||
            conf.key.inner_tag.pcp.mask                                                                         ||
            (conf.key.inner_tag.dei != VTSS_VCAP_BIT_ANY)) {

            IC_RC(vtss_icfg_printf(result, " inner-tag"));
            if (conf.key.inner_tag.tagged != VTSS_VCAP_BIT_ANY) {
                IC_RC(vtss_icfg_printf(result, " type %s", qos_icfg_tag_type_text(conf.key.inner_tag.tagged, conf.key.inner_tag.s_tag)));
            }

            // vid:
            IC_RC(QOS_ICFG_qce_range(result, " vid", &conf.key.inner_tag.vid));

            // pcp:
            if (conf.key.inner_tag.pcp.mask) {
                if (conf.key.inner_tag.pcp.mask == 0xFF) {
                    IC_RC(vtss_icfg_printf(result, " pcp %u", conf.key.inner_tag.pcp.value));
                } else {
                    IC_RC(vtss_icfg_printf(result, " pcp %u-%u", conf.key.inner_tag.pcp.value, (uchar)(conf.key.inner_tag.pcp.value + ((~conf.key.inner_tag.pcp.mask) & 0xFF))));
                }
            }

            // dei:
            if (conf.key.inner_tag.dei != VTSS_VCAP_BIT_ANY) {
                IC_RC(vtss_icfg_printf(result, " dei %u", (conf.key.inner_tag.dei == VTSS_VCAP_BIT_0) ? 0 : 1));
            }
        }
#endif /* defined(VTSS_ARCH_SERVAL) */

        // smac:
#if defined(VTSS_ARCH_JAGUAR_1)
        if (conf.key.smac.mask[0] || conf.key.smac.mask[1] || conf.key.smac.mask[2]) {
            IC_RC(vtss_icfg_printf(result, " smac %02x-%02x-%02x",
                                   conf.key.smac.value[0],
                                   conf.key.smac.value[1],
                                   conf.key.smac.value[2]));
        }
#else
        if (conf.key.smac.mask[0] || conf.key.smac.mask[1] || conf.key.smac.mask[2] ||
            conf.key.smac.mask[3] || conf.key.smac.mask[4] || conf.key.smac.mask[5]) {
            IC_RC(vtss_icfg_printf(result, " smac %02x-%02x-%02x-%02x-%02x-%02x",
                                   conf.key.smac.value[0],
                                   conf.key.smac.value[1],
                                   conf.key.smac.value[2],
                                   conf.key.smac.value[3],
                                   conf.key.smac.value[4],
                                   conf.key.smac.value[5]));
        }
#endif /* defined(VTSS_ARCH_JAGUAR_1) */

        // dmac
        bit_val = QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_DMAC_TYPE);
        if (bit_val != QOS_QCE_DMAC_TYPE_ANY) {
            IC_RC(vtss_icfg_printf(result, " dmac %s", (bit_val == QOS_QCE_DMAC_TYPE_UC) ? "unicast" : (bit_val == QOS_QCE_DMAC_TYPE_MC) ? "multicast" : "broadcast"));
        }
#if defined(VTSS_ARCH_SERVAL)
        if (conf.key.dmac.mask[0] || conf.key.dmac.mask[1] || conf.key.dmac.mask[2] ||
            conf.key.dmac.mask[3] || conf.key.dmac.mask[4] || conf.key.dmac.mask[5]) {
            IC_RC(vtss_icfg_printf(result, " dmac %02x-%02x-%02x-%02x-%02x-%02x",
                                   conf.key.dmac.value[0],
                                   conf.key.dmac.value[1],
                                   conf.key.dmac.value[2],
                                   conf.key.dmac.value[3],
                                   conf.key.dmac.value[4],
                                   conf.key.dmac.value[5]));
        }
#endif /* defined(VTSS_ARCH_SERVAL) */

        // frametype:
        if (conf.type != VTSS_QCE_TYPE_ANY) {
            IC_RC(vtss_icfg_printf(result, " frame-type"));
            switch (conf.type) {
            case VTSS_QCE_TYPE_ETYPE:
                IC_RC(vtss_icfg_printf(result, " etype"));
                if (conf.key.frame.etype.etype.mask[0] || conf.key.frame.etype.etype.mask[1]) {
                    IC_RC(vtss_icfg_printf(result, " 0x%x", (conf.key.frame.etype.etype.value[0] << 8) | conf.key.frame.etype.etype.value[1]));
                }
                break;
            case VTSS_QCE_TYPE_LLC:
                IC_RC(vtss_icfg_printf(result, " llc"));
                if (conf.key.frame.llc.dsap.mask) {
                    IC_RC(vtss_icfg_printf(result, " dsap 0x%x", conf.key.frame.llc.dsap.value));
                }
                if (conf.key.frame.llc.ssap.mask) {
                    IC_RC(vtss_icfg_printf(result, " ssap 0x%x", conf.key.frame.llc.ssap.value));
                }
                if (conf.key.frame.llc.control.mask) {
                    IC_RC(vtss_icfg_printf(result, " control 0x%x", conf.key.frame.llc.control.value));
                }
                break;
            case VTSS_QCE_TYPE_SNAP:
                IC_RC(vtss_icfg_printf(result, " snap"));
                if (conf.key.frame.snap.pid.mask[0] || conf.key.frame.snap.pid.mask[1]) {
                    IC_RC(vtss_icfg_printf(result, " 0x%x", (conf.key.frame.snap.pid.value[0] << 8) | conf.key.frame.snap.pid.value[1]));
                }
                break;
            case VTSS_QCE_TYPE_IPV4:
                IC_RC(vtss_icfg_printf(result, " ipv4"));
                IC_RC(QOS_ICFG_qce_proto(result, " proto", &conf.key.frame.ipv4.proto));
                IC_RC(QOS_ICFG_qce_ipv4(result, " sip", &conf.key.frame.ipv4.sip));
#if defined(VTSS_ARCH_SERVAL)
                IC_RC(QOS_ICFG_qce_ipv4(result, " dip", &conf.key.frame.ipv4.dip));
#endif /* defined(VTSS_ARCH_SERVAL) */
                IC_RC(QOS_ICFG_qce_range_u8(result, " dscp", &conf.key.frame.ipv4.dscp));
                bit_val = QCE_ENTRY_CONF_KEY_GET(conf.key.key_bits, QOS_QCE_IPV4_FRAGMENT);
                if (bit_val != VTSS_VCAP_BIT_ANY) {
                    IC_RC(vtss_icfg_printf(result, " frag %s", (bit_val == VTSS_VCAP_BIT_0) ? "no" : "yes"));
                }
                if (conf.key.frame.ipv4.proto.mask && (conf.key.frame.ipv4.proto.value == 6 || conf.key.frame.ipv4.proto.value == 17)) {
                    IC_RC(QOS_ICFG_qce_range_u16(result, " sport", &conf.key.frame.ipv4.sport));
                    IC_RC(QOS_ICFG_qce_range_u16(result, " dport", &conf.key.frame.ipv4.dport));
                }
                break;
            case VTSS_QCE_TYPE_IPV6:
                IC_RC(vtss_icfg_printf(result, " ipv6"));
                IC_RC(QOS_ICFG_qce_proto(result, " proto", &conf.key.frame.ipv6.proto));
                IC_RC(QOS_ICFG_qce_ipv6(result, " sip", &conf.key.frame.ipv6.sip));
#if defined(VTSS_ARCH_SERVAL)
                IC_RC(QOS_ICFG_qce_ipv6(result, " dip", &conf.key.frame.ipv6.dip));
#endif /* defined(VTSS_ARCH_SERVAL) */
                IC_RC(QOS_ICFG_qce_range_u8(result, " dscp", &conf.key.frame.ipv6.dscp));
                if (conf.key.frame.ipv6.proto.mask && (conf.key.frame.ipv6.proto.value == 6 || conf.key.frame.ipv6.proto.value == 17)) {
                    IC_RC(QOS_ICFG_qce_range_u16(result, " sport", &conf.key.frame.ipv6.sport));
                    IC_RC(QOS_ICFG_qce_range_u16(result, " dport", &conf.key.frame.ipv6.dport));
                }
                break;
            default:
                break;
            }
        }

        if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_PRIO) ||
            QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_DP)   ||
#if defined(VTSS_ARCH_SERVAL)
            QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)   ||
            QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_POLICY)   ||
#endif /* defined(VTSS_ARCH_SERVAL) */
            QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_DSCP)) {

            IC_RC(vtss_icfg_printf(result, " action"));
            if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_PRIO)) {
                IC_RC(vtss_icfg_printf(result, " cos %u", conf.action.prio));
            }
            if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_DP)) {
                IC_RC(vtss_icfg_printf(result, " dpl %u", conf.action.dp));
            }
            if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_DSCP)) {
                IC_RC(vtss_icfg_printf(result, " dscp %u", conf.action.dscp));
            }
#if defined(VTSS_ARCH_SERVAL)
            if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)) {
                IC_RC(vtss_icfg_printf(result, " pcp-dei %u %u", conf.action.pcp, conf.action.dei));
            }
            if (QCE_ENTRY_CONF_ACTION_GET(conf.action.action_bits, QOS_QCE_ACTION_POLICY)) {
                IC_RC(vtss_icfg_printf(result, " policy %u", conf.action.policy_no));
            }
#endif /* defined(VTSS_ARCH_SERVAL) */
        }

        IC_RC(vtss_icfg_printf(result, "\n"));
    }
#endif /* VTSS_FEATURE_QCL_V2 */

    return VTSS_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
char *qos_icfg_storm_rate_txt(u32 rate, char *buf)
{
    if (rate >= 1000 && (rate % 1000) == 0) {
        rate /= 1000;
        sprintf(buf, "%u kfps", rate);
    } else {
        sprintf(buf, "%u", rate);
    }
    return buf;
}
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */

#if defined(VTSS_ARCH_SERVAL)
const char *qos_icfg_key_type_txt(vtss_vcap_key_type_t key_type)
{
    return (key_type == VTSS_VCAP_KEY_TYPE_NORMAL ? "normal" :
            key_type == VTSS_VCAP_KEY_TYPE_DOUBLE_TAG ? "double-tag" :
            key_type == VTSS_VCAP_KEY_TYPE_IP_ADDR ? "ip-addr" :
            key_type == VTSS_VCAP_KEY_TYPE_MAC_IP_ADDR ? "mac-ip-addr" : "invalid");
}
#endif /* defined(VTSS_ARCH_SERVAL) */

const char *qos_icfg_tag_type_text(vtss_vcap_bit_t tagged, vtss_vcap_bit_t s_tag)
{
    switch (tagged) {
    case VTSS_VCAP_BIT_ANY:
        switch (s_tag) {
        case VTSS_VCAP_BIT_ANY:
            return "any";
        case VTSS_VCAP_BIT_0:
            return "N/A";
        case VTSS_VCAP_BIT_1:
            return "N/A";
        default:
            return "invalid 's-tag'";
        }
    case VTSS_VCAP_BIT_0:
        switch (s_tag) {
        case VTSS_VCAP_BIT_ANY:
            return "untagged";
        case VTSS_VCAP_BIT_0:
            return "N/A";
        case VTSS_VCAP_BIT_1:
            return "N/A";
        default:
            return "invalid 's-tag'";
        }
    case VTSS_VCAP_BIT_1:
        switch (s_tag) {
        case VTSS_VCAP_BIT_ANY:
            return "tagged";
        case VTSS_VCAP_BIT_0:
            return "c-tagged";
        case VTSS_VCAP_BIT_1:
            return "s-tagged";
        default:
            return "invalid 's-tag'";
        }
    default:
        return "invalid 'tagged'";
    }
}

/* Initialization function */
vtss_rc qos_icfg_init(void)
{
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_QOS_GLOBAL_CONF, "qos", QOS_ICFG_global_conf));
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_QOS_PORT_CONF, "qos", QOS_ICFG_port_conf));
    IC_RC(vtss_icfg_query_register(VTSS_ICFG_QOS_QCE_CONF, "qos", QOS_ICFG_qce_conf));
    return VTSS_OK;
}
