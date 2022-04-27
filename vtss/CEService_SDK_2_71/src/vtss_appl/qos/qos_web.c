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

#include "web_api.h"
#include "qos_api.h"
#include "msg_api.h"
#include "port_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifdef VTSS_FEATURE_QCL_V2
#include "topo_api.h"
#include "mgmt_api.h"
#endif

#if defined(VTSS_SW_OPTION_JR_STORM_POLICERS)
/* Make it look exactly like a Luton26 regarding port policers */
#undef VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL
#undef VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM
#endif /* defined(VTSS_SW_OPTION_JR_STORM_POLICERS) */

#define QOS_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_stat_qos_counter(CYG_HTTPD_STATE *p)
{
    vtss_isid_t          sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t          pit;
    vtss_port_counters_t counters;
    int                  ct;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PORT)) {
        return -1;
    }
#endif

    /*
    Format: [uport]/[low_queue_receive]/[normal_queue_transmit]/[normal_queue_receive]/[low_queue_transmit]/[medium_queue_receive]/[medium_queue_transmit]/[high_queue_receive]/[high_queue_transmit]|[uport]/[low_queue_receive]/[normal_queue_transmit]/[normal_queue_receive]/[low_queue_transmit]/[medium_queue_receive]/[medium_queue_transmit]/[high_queue_receive]/[high_queue_transmit]|...
    */
    cyg_httpd_start_chunked("html");

    if (VTSS_ISID_LEGAL(sid) && msg_switch_exists(sid)) {
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (cyg_httpd_form_varable_find(p, "clear")) { /* Clear? */
                if (port_mgmt_counters_clear(sid, pit.iport) != VTSS_OK) {
                    T_W("Unable to clear counters for sid %u, port %u", sid, pit.uport);
                    break;
                }
                memset(&counters, 0, sizeof(counters)); /* Cheating a little... */
            } else {
                /* Normal read */
                if (port_mgmt_counters_get(sid, pit.iport, &counters) != VTSS_OK) {
                    T_W("Unable to get counters for sid %u, port %u", sid, pit.uport);
                    break;              /* Most likely stack error - bail out */
                }
            }
            /* Output the counters */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu%s",
                          pit.uport,
                          counters.prop.rx_prio[0],
                          counters.prop.tx_prio[0],
                          counters.prop.rx_prio[1],
                          counters.prop.tx_prio[1],
                          counters.prop.rx_prio[2],
                          counters.prop.tx_prio[2],
                          counters.prop.rx_prio[3],
                          counters.prop.tx_prio[3],
                          counters.prop.rx_prio[4],
                          counters.prop.tx_prio[4],
                          counters.prop.rx_prio[5],
                          counters.prop.tx_prio[5],
                          counters.prop.rx_prio[6],
                          counters.prop.tx_prio[6],
                          counters.prop.rx_prio[7],
                          counters.prop.tx_prio[7],
                          pit.last ? "" : "|");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
static cyg_int32 handler_config_stormctrl(CYG_HTTPD_STATE *p)
{
    int          ct, var_rate;
    qos_conf_t   conf, newconf;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (qos_conf_get(&conf) < 0) {
            errors++;   /* Probably stack error */
        } else {
            newconf = conf;

            //unicast
            if (cyg_httpd_form_varable_find(p, "status_0")) { /* "on" if checked */
                newconf.policer_uc_status = RATE_STATUS_ENABLE;
            } else {
                newconf.policer_uc_status = RATE_STATUS_DISABLE;
            }
            if (cyg_httpd_form_varable_int(p, "rate_0", &var_rate)) {
                newconf.policer_uc = var_rate;
            }

            //multicast
            if (cyg_httpd_form_varable_find(p, "status_1")) { /* "on" if checked */
                newconf.policer_mc_status = RATE_STATUS_ENABLE;
            } else {
                newconf.policer_mc_status = RATE_STATUS_DISABLE;
            }
            if (cyg_httpd_form_varable_int(p, "rate_1", &var_rate)) {
                newconf.policer_mc = var_rate;
            }

            //broadcast
            if (cyg_httpd_form_varable_find(p, "status_2")) { /* "on" if checked */
                newconf.policer_bc_status = RATE_STATUS_ENABLE;
            } else {
                newconf.policer_bc_status = RATE_STATUS_DISABLE;
            }
            if (cyg_httpd_form_varable_int(p, "rate_2", &var_rate)) {
                newconf.policer_bc = var_rate;
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                vtss_rc rc;
                if ((rc = qos_conf_set(&newconf)) < 0) {
                    T_D("qos_conf_set: failed rc = %d", rc);
                    errors++; /* Probably stack error */
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/stormctrl.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        /* should get these values from management APIs */
        if (qos_conf_get(&conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u|%d/%u|%d/%u",
                          conf.policer_uc_status,
                          conf.policer_uc,
                          conf.policer_mc_status,
                          conf.policer_mc,
                          conf.policer_bc_status,
                          conf.policer_bc);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */

#ifdef VTSS_FEATURE_QCL_V2
//***qce handler
static char *qcl_qce_port_list_txt(qos_qce_entry_conf_t *conf, char *buf)
{
    BOOL          port_list[VTSS_PORTS];
    int           i;
    char          *ret_buf;

    for (i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
        if (VTSS_PORT_BF_GET(conf->port_list, i)) {
            port_list[i] = 1;
        } else {
            port_list[i] = 0;
        }
    }

    ret_buf = mgmt_iport_list2txt(port_list, buf);
    if (strlen(ret_buf) == 0) {
        strcpy(buf, "None");
    }
    return buf;
}

static char *qcl_qce_ipv6_txt(vtss_qce_u32_t *ip, char *buf)
{
    if (!ip->mask[0] && !ip->mask[1] && !ip->mask[2] && !ip->mask[3]) {
        sprintf(buf, "Any");
    } else {
        sprintf(buf, "%u.%u.%u.%u", ip->value[0], ip->value[1],
                ip->value[2], ip->value[3]);
    }
    return (buf);
}

static char *qcl_qce_range_txt_u16(qos_qce_vr_u16_t *range, char *buf)
{
    if (range->in_range) {
        sprintf(buf, "%u-%u", range->vr.r.low, range->vr.r.high);
    } else if (range->vr.v.mask) {
        sprintf(buf, "%u", range->vr.v.value);
    } else {
        sprintf(buf, "Any");
    }
    return (buf);
}

static const char *qcl_qce_tag_type_text(vtss_vcap_bit_t tagged, vtss_vcap_bit_t s_tag)
{
    switch (tagged) {
    case VTSS_VCAP_BIT_ANY:
        switch (s_tag) {
        case VTSS_VCAP_BIT_ANY:
            return "Any";
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
            return "Untagged";
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
            return "Tagged";
        case VTSS_VCAP_BIT_0:
            return "C-Tagged";
        case VTSS_VCAP_BIT_1:
            return "S-Tagged";
        default:
            return "invalid 's-tag'";
        }
    default:
        return "invalid 'tagged'";
    }
}

static u8 qcl_qce_tag_type(vtss_vcap_bit_t tagged, vtss_vcap_bit_t s_tag)
{
    switch (tagged) {
    case VTSS_VCAP_BIT_0:
        return 1; // Untagged
    case VTSS_VCAP_BIT_1:
        switch (s_tag) {
        case VTSS_VCAP_BIT_0:
            return 3; // C_Tagged
        case VTSS_VCAP_BIT_1:
            return 4; // S_Tagged
        default:
            return 2; // Tagged
        }
    default:
        return 0; // Any
    }
}

static cyg_int32 handler_config_qcl_v2(CYG_HTTPD_STATE *p)
{
    vtss_qce_id_t        qce_id = QCE_ID_NONE;
    vtss_qce_id_t        next_qce_id = QCE_ID_NONE;
    int                  qce_flag = 0;
    int                  var_value1 = 0, var_value2 = 0, var_value3 = 0, var_value4 = 0;
    qos_qce_entry_conf_t qce_conf, newconf, qce_next;
    const char           *var_string;
    BOOL                 first;
    int                  i;
    BOOL                 add;
    char                 str_buff1[24], str_buff2[24];
#if defined(VTSS_ARCH_SERVAL)
    char                 str_buff3[24], str_buff4[24];
#endif
    vtss_port_no_t       iport;
    const i8             *str;
    size_t               len;
    int                  ct;
    switch_iter_t        sit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;
        char search_str[32];
        memset(&qce_conf, 0, sizeof(qce_conf));
        memset(&newconf, 0, sizeof(newconf));
        memset(&qce_next, 0, sizeof(qce_next));
        newconf.isid = VTSS_ISID_GLOBAL;

        //switch_id
#if VTSS_SWITCH_STACKABLE
        if (cyg_httpd_form_varable_int(p, "portMemSIDSelect", &var_value1)) {//Specific sid
            if (var_value1 != -1) {
                newconf.isid = topo_usid2isid(var_value1);
                if (redirectUnmanagedOrInvalid(p, newconf.isid)) {/* Redirect unmanaged/invalid access to handler */
                    return -1;
                }
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */

        //qce_id, next_qce_id
        qce_id = QCE_ID_NONE;
        if (cyg_httpd_form_varable_int(p, "qce_id", &var_value1)) {
            qce_id = var_value1;
        }
        add = (qce_id == QCE_ID_NONE ||
               qos_mgmt_qce_entry_get(VTSS_ISID_GLOBAL, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_conf, 0) != VTSS_OK);
        if (cyg_httpd_form_varable_int(p, "next_qce_id", &var_value1)) {
            next_qce_id = var_value1;
        }
        if (add == FALSE && next_qce_id == QCE_ID_NONE) {//get next entry when qce alreay exists & next_qce_id is none
            if (qos_mgmt_qce_entry_get(VTSS_ISID_GLOBAL, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_next, 1) == VTSS_OK) {
                next_qce_id = qce_next.id;
            }
        }
        if (next_qce_id == QCE_ID_END) {//if entry being edited is at last then set qce_next_id to none i.e to 0
            next_qce_id = QCE_ID_NONE;
        }
        newconf.id = qce_id;
        //Key Key -> Frame Type
        if (cyg_httpd_form_varable_int(p, "KeyFrameTypeSelect", &var_value1)) {
            switch (var_value1) {
            case 0:
                newconf.type = VTSS_QCE_TYPE_ANY;
                break;
            case 1:
                newconf.type = VTSS_QCE_TYPE_ETYPE;
                if (cyg_httpd_form_varable_int(p, "ether_type_filter", &var_value2)) {
                    newconf.key.frame.etype.etype.mask[0] = newconf.key.frame.etype.etype.mask[1] = 0;
                    if (var_value2 == 1) {//i.e etype is Specific value
                        if (cyg_httpd_form_varable_hex(p, "ether_type", (ulong *)&var_value3)) {
                            newconf.key.frame.etype.etype.value[0] = (var_value3 >> 8) & 0xFF;
                            newconf.key.frame.etype.etype.value[1] = var_value3 & 0xFF;
                            newconf.key.frame.etype.etype.mask[0] = newconf.key.frame.etype.etype.mask[1] = 0xFF;
                        }
                    }
                }
                break;
            case 2:
                newconf.type = VTSS_QCE_TYPE_LLC;
                break;
            case 3:
                newconf.type = VTSS_QCE_TYPE_SNAP;
                break;
            case 4:
                newconf.type = VTSS_QCE_TYPE_IPV4;
                break;
            case 5:
                newconf.type = VTSS_QCE_TYPE_IPV6;
                break;
            default:
                T_E("Invalid frame type value");
                break;
            }
        }

        //Look for por list in post query
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            sprintf(search_str, "new_ckBox_%d", iport2uport(iport));
            VTSS_PORT_BF_SET(newconf.port_list, iport, 0);
            if (cyg_httpd_form_varable_find(p, search_str)) { // "on" if checked
                VTSS_PORT_BF_SET(newconf.port_list, iport, 1);
            }
        }

        //qce key tag type
        QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_ANY);
        if (cyg_httpd_form_varable_int(p, "KeyTagSelect", &var_value1)) {
#if defined(VTSS_ARCH_SERVAL)
            switch (var_value1) {
            case 1: // Untagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_0);
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_S_TAG, VTSS_VCAP_BIT_ANY);
                break;
            case 2: // Tagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_1);
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_S_TAG, VTSS_VCAP_BIT_ANY);
                break;
            case 3: // C-Tagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_1);
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_S_TAG, VTSS_VCAP_BIT_0);
                break;
            case 4: // S-Tagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_1);
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_S_TAG, VTSS_VCAP_BIT_1);
                break;
            default: // Any
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_ANY);
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_S_TAG, VTSS_VCAP_BIT_ANY);
                break;
            }
#else
            switch (var_value1) {
            case 1: // Untagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_0);
                break;
            case 2: // Tagged
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_1);
                break;
            default: // Any
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_TAG, VTSS_VCAP_BIT_ANY);
                break;
            }
#endif /* defined(VTSS_ARCH_SERVAL) */
        }

        //vid specific|Range fields
        if (cyg_httpd_form_varable_int(p, "KeyVIDSelect", &var_value1)) {
            switch (var_value1) {
            case 1: // Specific
                if (cyg_httpd_form_varable_int(p, "KeyVIDSpecific", &var_value1)) { // Specific
                    newconf.key.vid.in_range = FALSE;
                    newconf.key.vid.vr.v.value = var_value1;
                    newconf.key.vid.vr.v.mask = 0xFFFF;
                }
                break;
            case 2: // Range
                if (cyg_httpd_form_varable_int(p, "KeyVidStart", &var_value1)) { // Range
                    newconf.key.vid.in_range = TRUE;
                    newconf.key.vid.vr.r.low = var_value1;
                    if (cyg_httpd_form_varable_int(p, "KeyVidLast", &var_value2)) {
                        newconf.key.vid.vr.r.high = var_value2;
                    }
                }
                break;
            default: // Any
                newconf.key.vid.in_range = FALSE;
                newconf.key.vid.vr.v.value = 0;
                newconf.key.vid.vr.v.mask = 0;
                break;
            }
        }

        //Key -> pcp field
        if (cyg_httpd_form_varable_int(p, "KeyPCPSelect", &var_value1)) {
            if (var_value1 == 0) {
                newconf.key.pcp.mask = 0;
                newconf.key.pcp.value = var_value1;
            } else if (var_value1 > 0 && var_value1 <= 8) { // pcp values other than 'any'
                newconf.key.pcp.value = (var_value1 - 1);
                newconf.key.pcp.mask = 0xFF;
            } else if (var_value1 > 8 && var_value1 <= 12) {
                newconf.key.pcp.value = (var_value1 - 9) * 2;
                newconf.key.pcp.mask = 0xFE;
            } else {
                newconf.key.pcp.value = (var_value1 == 13) ? 0 : 4;
                newconf.key.pcp.mask = 0xFC;
            }
        }

        // Key -> dei
        if (cyg_httpd_form_varable_int(p, "KeyDEISelect", &var_value1)) {
            QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_VLAN_DEI, var_value1);
        }

#if defined(VTSS_ARCH_SERVAL)
        // Key -> inner tag

        // inner tag type
        if (cyg_httpd_form_varable_int(p, "KeyITagSelect", &var_value1)) {
            switch (var_value1) {
            case 1: // Untagged
                newconf.key.inner_tag.tagged = VTSS_VCAP_BIT_0;
                newconf.key.inner_tag.s_tag = VTSS_VCAP_BIT_ANY;
                break;
            case 2: // Tagged
                newconf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.key.inner_tag.s_tag = VTSS_VCAP_BIT_ANY;
                break;
            case 3: // C-Tagged
                newconf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.key.inner_tag.s_tag = VTSS_VCAP_BIT_0;
                break;
            case 4: // S-Tagged
                newconf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.key.inner_tag.s_tag = VTSS_VCAP_BIT_1;
                break;
            default: // Any
                newconf.key.inner_tag.tagged = VTSS_VCAP_BIT_ANY;
                newconf.key.inner_tag.s_tag = VTSS_VCAP_BIT_ANY;
                break;
            }
        }

        //vid specific|Range fields
        if (cyg_httpd_form_varable_int(p, "KeyIVIDSelect", &var_value1)) {
            switch (var_value1) {
            case 1: // Specific
                if (cyg_httpd_form_varable_int(p, "KeyIVIDSpecific", &var_value1)) { // Specific
                    newconf.key.inner_tag.vid.type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
                    newconf.key.inner_tag.vid.vr.v.value = var_value1;
                    newconf.key.inner_tag.vid.vr.v.mask = 0xFFFF;
                }
                break;
            case 2: // Range
                if (cyg_httpd_form_varable_int(p, "KeyIVidStart", &var_value1)) { // Range
                    newconf.key.inner_tag.vid.type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
                    newconf.key.inner_tag.vid.vr.r.low = var_value1;
                    if (cyg_httpd_form_varable_int(p, "KeyIVidLast", &var_value2)) {
                        newconf.key.inner_tag.vid.vr.r.high = var_value2;
                    }
                }
                break;
            default: // Any
                newconf.key.inner_tag.vid.type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
                newconf.key.inner_tag.vid.vr.v.value = 0;
                newconf.key.inner_tag.vid.vr.v.mask = 0;
                break;
            }
        }

        //Key -> pcp field
        if (cyg_httpd_form_varable_int(p, "KeyIPCPSelect", &var_value1)) {
            if (var_value1 == 0) {
                newconf.key.inner_tag.pcp.mask = 0;
                newconf.key.inner_tag.pcp.value = var_value1;
            } else if (var_value1 > 0 && var_value1 <= 8) { // pcp values other than 'any'
                newconf.key.inner_tag.pcp.value = (var_value1 - 1);
                newconf.key.inner_tag.pcp.mask = 0xFF;
            } else if (var_value1 > 8 && var_value1 <= 12) {
                newconf.key.inner_tag.pcp.value = (var_value1 - 9) * 2;
                newconf.key.inner_tag.pcp.mask = 0xFE;
            } else {
                newconf.key.inner_tag.pcp.value = (var_value1 == 13) ? 0 : 4;
                newconf.key.inner_tag.pcp.mask = 0xFC;
            }
        }

        // Key -> dei
        if (cyg_httpd_form_varable_int(p, "KeyIDEISelect", &var_value1)) {
            newconf.key.inner_tag.dei = var_value1;
        }
#endif /* defined(VTSS_ARCH_SERVAL) */

        // Key -> smac address
        if (cyg_httpd_form_varable_int(p, "KeySMACSelect", &var_value1)) {
            memset(&newconf.key.smac, 0, sizeof(newconf.key.smac));
            uint mac[6];
            if (var_value1 == 1) {//smac value will be present when var_value1 is == 1(Specific)
                if ((str = cyg_httpd_form_varable_string(p, "qceKeySMACValue", &len)) != NULL) {
#if defined(VTSS_ARCH_JAGUAR_1)
                    if (sscanf(str, "%2x-%2x-%2x", &mac[0], &mac[1], &mac[2]) == 3) {
                        for (i = 0; i < 3; i++) {
                            newconf.key.smac.value[i] = mac[i];
                            newconf.key.smac.mask[i] = 0xFF;
                        }
                    }
#else
                    if (sscanf(str, "%2x-%2x-%2x-%2x-%2x-%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
                        for (i = 0; i < 6; i++) {
                            newconf.key.smac.value[i] = mac[i];
                            newconf.key.smac.mask[i] = 0xFF;
                        }
                    }
#endif /* defined(VTSS_ARCH_JAGUAR_1) */
                }
            }
        }

        // Key -> DMAC type
        if (cyg_httpd_form_varable_int(p, "KeyDMACSelect", &var_value1)) {
#if defined(VTSS_ARCH_SERVAL)
            memset(&newconf.key.dmac, 0, sizeof(newconf.key.dmac));
            vtss_mac_t mac;
            if (var_value1 == 4) {// dmac value will be present when var_value1 is == 4 (Specific)
                if (cyg_httpd_form_variable_mac(p, "qceKeyDMACValue", &mac)) {
                    for (i = 0; i < 6; i++) {
                        newconf.key.dmac.value[i] = mac.addr[i];
                        newconf.key.dmac.mask[i] = 0xFF;
                    }
                }
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_DMAC_TYPE, 0); // Set to Any
            } else
#endif /* defined(VTSS_ARCH_SERVAL) */
            {
                QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_DMAC_TYPE, var_value1);
            }
        }

        //Key Key -> Frame Type
        if (cyg_httpd_form_varable_int(p, "KeyFrameTypeSelect", &var_value1)) {
            switch (var_value1) {
            case VTSS_QCE_TYPE_ANY://frame_type == Any
                break;
            case VTSS_QCE_TYPE_ETYPE://frame_type == Ethernet
                //etype.value already assinged earlier
                break;
            case VTSS_QCE_TYPE_LLC://frame_type == LLC
                // SSAP Address
                if (cyg_httpd_form_varable_int(p, "SSAPAddrSel", &var_value1)) {
                    newconf.key.frame.llc.ssap.value = 0;
                    newconf.key.frame.llc.ssap.mask = 0;
                    if (var_value1 == 1) {
                        if (cyg_httpd_form_varable_hex(p, "LLCSSAPVal", (ulong *)&var_value1)) {
                            newconf.key.frame.llc.ssap.value = var_value1;
                            newconf.key.frame.llc.ssap.mask = 0xFF;
                        }
                    }
                }
                // DSAP Address
                if (cyg_httpd_form_varable_int(p, "DSAPAddrSel", &var_value1)) {
                    newconf.key.frame.llc.dsap.value = 0;
                    newconf.key.frame.llc.dsap.mask = 0;
                    if (var_value1 == 1) {
                        if (cyg_httpd_form_varable_hex(p, "LLCDSAPVal", (ulong *)&var_value1)) {
                            newconf.key.frame.llc.dsap.value = var_value1;
                            newconf.key.frame.llc.dsap.mask = 0xFF;
                        }
                    }
                }
                // LLC Control
                if (cyg_httpd_form_varable_int(p, "LLCCntrlSel", &var_value1)) {
                    newconf.key.frame.llc.control.value = 0;
                    newconf.key.frame.llc.control.mask = 0;
                    if (var_value1 == 1) {
                        if (cyg_httpd_form_varable_hex(p, "LLCControlVal", (ulong *)&var_value1)) {
                            newconf.key.frame.llc.control.value = var_value1;
                            newconf.key.frame.llc.control.mask = 0xFF;
                        }
                    }
                }
                break;
            case VTSS_QCE_TYPE_SNAP://frame_type == SNAP
                if (cyg_httpd_form_varable_int(p, "SNAP_Ether_type_filter", &var_value2)) {
                    if (var_value2 == 1) {// if ether_type is specific
                        if (cyg_httpd_form_varable_hex(p, "ether_type", (ulong *)&var_value3)) {
                            newconf.key.frame.snap.pid.mask[0] = newconf.key.frame.snap.pid.mask[1] = 0xff;
                            newconf.key.frame.snap.pid.value[0] = var_value3 >> 8;
                            newconf.key.frame.snap.pid.value[1] = var_value3 & 0xff;
                        } else {// if ether_type is Any
                            newconf.key.frame.snap.pid.mask[0] = newconf.key.frame.snap.pid.mask[1] = 0;
                        }
                    }
                }
                break;
            case VTSS_QCE_TYPE_IPV4://frame_type == IPv4
                if (cyg_httpd_form_varable_int(p, "protocol_filterIPv4", &var_value1)) {
                    switch (var_value1) {
                    case 0://Protocol Type is Any
                        newconf.key.frame.ipv4.proto.mask = 0;
                        break;
                    case 1://Protocol Type is UDP
                        newconf.key.frame.ipv4.sport.in_range = newconf.key.frame.ipv4.dport.in_range = FALSE;
                        newconf.key.frame.ipv4.sport.vr.v.mask = newconf.key.frame.ipv4.dport.vr.v.mask = 0;
                        newconf.key.frame.ipv4.sport.vr.v.value = newconf.key.frame.ipv4.dport.vr.v.value = 0;
                        newconf.key.frame.ipv4.proto.value = 17;//UDP protocol number
                        newconf.key.frame.ipv4.proto.mask = 0xFF;// for specific value
                        if (cyg_httpd_form_varable_int(p, "KeySportSelect", &var_value2)) {
                            if (var_value2 != 0) {//if Specific
                                if (cyg_httpd_form_varable_int(p, "keySportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv4.sport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv4.sport.vr.v.value = var_value3;
                                }
                                //if in range
                                if (cyg_httpd_form_varable_int(p, "keySportStart", &var_value3)) {
                                    newconf.key.frame.ipv4.sport.in_range = TRUE;
                                    newconf.key.frame.ipv4.sport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keySportLast", &var_value4)) {
                                        newconf.key.frame.ipv4.sport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        if (cyg_httpd_form_varable_int(p, "KeyDportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keyDportSpecValue", &var_value3)) {//if Specific
                                    newconf.key.frame.ipv4.dport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv4.dport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keyDportStart", &var_value3)) {
                                    newconf.key.frame.ipv4.dport.in_range = TRUE;
                                    newconf.key.frame.ipv4.dport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keyDportLast", &var_value4)) {
                                        newconf.key.frame.ipv4.dport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        break;
                    case 2://Protocol Type is TCP
                        newconf.key.frame.ipv4.sport.in_range = newconf.key.frame.ipv4.dport.in_range = FALSE;
                        newconf.key.frame.ipv4.sport.vr.v.mask = newconf.key.frame.ipv4.dport.vr.v.mask = 0;
                        newconf.key.frame.ipv4.sport.vr.v.value = newconf.key.frame.ipv4.dport.vr.v.value = 0;

                        newconf.key.frame.ipv4.proto.value = 6;//TCP protocol number
                        newconf.key.frame.ipv4.proto.mask = 0xFF;// for specific value
                        if (cyg_httpd_form_varable_int(p, "KeySportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keySportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv4.sport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv4.sport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keySportStart", &var_value3)) {
                                    newconf.key.frame.ipv4.sport.in_range = TRUE;
                                    newconf.key.frame.ipv4.sport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keySportLast", &var_value4)) {
                                        newconf.key.frame.ipv4.sport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        if (cyg_httpd_form_varable_int(p, "KeyDportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keyDportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv4.dport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv4.dport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keyDportStart", &var_value3)) {
                                    newconf.key.frame.ipv4.dport.in_range = TRUE;
                                    newconf.key.frame.ipv4.dport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keyDportLast", &var_value4)) {
                                        newconf.key.frame.ipv4.dport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        break;
                    case 3://Protocol Type is Other
                        if (cyg_httpd_form_varable_int(p, "KeyProtoNbr", &var_value1)) {
                            newconf.key.frame.ipv4.proto.value = var_value1;//Other protocol number
                            newconf.key.frame.ipv4.proto.mask = 0xFF;// for specific value
                        }
                        break;
                    default:
                        // through an error saying invalid input
                        break;
                    }

                }//IPv4 protocol filter block

                // ipv4 set sip
                if (cyg_httpd_form_varable_int(p, "IPv4IPAddrSelect", &var_value1) == 1) {
                    if (var_value1 == 1) { // specific
                        if (cyg_httpd_form_varable_ipv4(p, "IPv4Addr", &newconf.key.frame.ipv4.sip.value)) {
                            (void)cyg_httpd_form_varable_ipv4(p, "IPMaskValue", &newconf.key.frame.ipv4.sip.mask);
                        }
                    }
                }

#if defined(VTSS_ARCH_SERVAL)
                // ipv4 set dip
                if (cyg_httpd_form_varable_int(p, "IPv4DIPAddrSelect", &var_value1) == 1) {
                    if (var_value1 == 1) { // specific
                        if (cyg_httpd_form_varable_ipv4(p, "IPv4DAddr", &newconf.key.frame.ipv4.dip.value)) {
                            (void)cyg_httpd_form_varable_ipv4(p, "DIPMaskValue", &newconf.key.frame.ipv4.dip.mask);
                        }
                    }
                }
#endif /* defined(VTSS_ARCH_SERVAL) */

                //set ipv4 fragment (Any:0, Yes:1, No:2) - name:value
                if (cyg_httpd_form_varable_int(p, "IPv4IPfragMenu", &var_value1)) {
                    if (var_value1 == 0) {//fragment == "Any"
                        QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_IPV4_FRAGMENT, VTSS_VCAP_BIT_ANY);
                    }
                    if (var_value1 == 1) {// fragment == "Yes"
                        QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_IPV4_FRAGMENT, VTSS_VCAP_BIT_1);
                    }
                    if (var_value1 == 2) {// fragment == "No"
                        QCE_ENTRY_CONF_KEY_SET(newconf.key.key_bits, QOS_QCE_IPV4_FRAGMENT, VTSS_VCAP_BIT_0);
                    }
                }

                //dscp choice field
                newconf.key.frame.ipv4.dscp.in_range = FALSE;
                newconf.key.frame.ipv4.dscp.vr.v.value = 0;
                newconf.key.frame.ipv4.dscp.vr.v.mask = 0;
                if (cyg_httpd_form_varable_int(p, "IPv4DSCPChoice", &var_value1)) {
                    if (var_value1 == 1) {//Specific value
                        if (cyg_httpd_form_varable_int(p, "keyDSCPSpcValue", &var_value2)) {
                            newconf.key.frame.ipv4.dscp.vr.v.value = var_value2;
                            newconf.key.frame.ipv4.dscp.vr.v.mask = 0x3F;
                        }
                    }
                    if (var_value1 == 2) {//Range of values
                        if (cyg_httpd_form_varable_int(p, "keyDSCPRngStart", &var_value2)) {
                            newconf.key.frame.ipv4.dscp.in_range = TRUE;
                            newconf.key.frame.ipv4.dscp.vr.r.low = var_value2;
                            if (cyg_httpd_form_varable_int(p, "keyDSCPRngLast", &var_value3)) {
                                newconf.key.frame.ipv4.dscp.vr.r.high = var_value3;
                            }
                        }
                    }
                }
                break;
            case VTSS_QCE_TYPE_IPV6://frame_type == IPv6
                if (cyg_httpd_form_varable_int(p, "protocol_filterIPv6", &var_value1)) {
                    switch (var_value1) {
                    case 0://Protocol Type is Any
                        newconf.key.frame.ipv6.proto.mask = 0;
                        newconf.key.frame.ipv6.proto.value = 0;
                        break;
                    case 1://Protocol Type is UDP
                        newconf.key.frame.ipv6.sport.in_range =  newconf.key.frame.ipv6.dport.in_range = FALSE;
                        newconf.key.frame.ipv6.sport.vr.v.mask = newconf.key.frame.ipv6.dport.vr.v.mask = 0;
                        newconf.key.frame.ipv6.sport.vr.v.value = newconf.key.frame.ipv6.dport.vr.v.value = 0;
                        newconf.key.frame.ipv6.proto.value = 17;//UDP protocol number
                        newconf.key.frame.ipv6.proto.mask = 0xFF;
                        if (cyg_httpd_form_varable_int(p, "KeySportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keySportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv6.sport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv6.sport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keySportStart", &var_value3)) {
                                    newconf.key.frame.ipv6.sport.in_range = TRUE;
                                    newconf.key.frame.ipv6.sport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keySportLast", &var_value4)) {
                                        newconf.key.frame.ipv6.sport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        if (cyg_httpd_form_varable_int(p, "KeyDportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keyDportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv6.dport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv6.dport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keyDportStart", &var_value3)) {
                                    newconf.key.frame.ipv6.dport.in_range = TRUE;
                                    newconf.key.frame.ipv6.dport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keyDportLast", &var_value4)) {
                                        newconf.key.frame.ipv6.dport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        break;
                    case 2://Protocol Type is TCP
                        newconf.key.frame.ipv6.sport.in_range =  newconf.key.frame.ipv6.dport.in_range = FALSE;
                        newconf.key.frame.ipv6.sport.vr.v.mask = newconf.key.frame.ipv6.dport.vr.v.mask = 0;
                        newconf.key.frame.ipv6.sport.vr.v.value = newconf.key.frame.ipv6.dport.vr.v.value = 0;
                        newconf.key.frame.ipv6.proto.value = 6;//UDP protocol number
                        newconf.key.frame.ipv6.proto.mask = 0xFF;
                        if (cyg_httpd_form_varable_int(p, "KeySportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keySportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv6.sport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv6.sport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keySportStart", &var_value3)) {
                                    newconf.key.frame.ipv6.sport.in_range = TRUE;
                                    newconf.key.frame.ipv6.sport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keySportLast", &var_value4)) {
                                        newconf.key.frame.ipv6.sport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        if (cyg_httpd_form_varable_int(p, "KeyDportSelect", &var_value2)) {
                            if (var_value2 != 0) {
                                if (cyg_httpd_form_varable_int(p, "keyDportSpecValue", &var_value3)) {
                                    newconf.key.frame.ipv6.dport.vr.v.mask = 0xFFFF;
                                    newconf.key.frame.ipv6.dport.vr.v.value = var_value3;
                                }
                                if (cyg_httpd_form_varable_int(p, "keyDportStart", &var_value3)) {
                                    newconf.key.frame.ipv6.dport.in_range = TRUE;
                                    newconf.key.frame.ipv6.dport.vr.r.low = var_value3;
                                    if (cyg_httpd_form_varable_int(p, "keyDportLast", &var_value4)) {
                                        newconf.key.frame.ipv6.dport.vr.r.high = var_value4;
                                    }
                                }
                            }
                        }
                        break;
                    case 3://Protocol Type is Other
                        if (cyg_httpd_form_varable_int(p, "KeyProtoNbr", &var_value1)) {
                            newconf.key.frame.ipv6.proto.value = var_value1;
                            newconf.key.frame.ipv6.proto.mask = 0xFF;
                        }
                        break;
                    default:
                        // through an error saying invalid input
                        break;
                    }
                }//IPv6 Protocol filter block

                // ipv6 set sip
                uint ipAddr[4], ipMask[4];
                if (cyg_httpd_form_varable_int(p, "IPv6IPAddrSelect", &var_value1)) {
                    if (var_value1 == 1) { // Specific ipaddress value
                        if ((str = cyg_httpd_form_varable_string(p, "IPv6Addr", &len)) != NULL) {
                            if (sscanf(str, "%3u.%3u.%3u.%3u", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]) == 4) {
                                if ((str = cyg_httpd_form_varable_string(p, "IPMaskValue", &len)) != NULL) {
                                    if (sscanf(str, "%3u.%3u.%3u.%3u", &ipMask[0], &ipMask[1], &ipMask[2], &ipMask[3]) == 4) {
                                        for (i = 0; i < 4; i++) {
                                            newconf.key.frame.ipv6.sip.value[i] = ipAddr[i] & 0xFF;
                                            newconf.key.frame.ipv6.sip.mask[i] = ipMask[i];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

#if defined(VTSS_ARCH_SERVAL)
                // ipv6 set dip
                if (cyg_httpd_form_varable_int(p, "IPv6DIPAddrSelect", &var_value1)) {
                    if (var_value1 == 1) { // Specific ipaddress value
                        if ((str = cyg_httpd_form_varable_string(p, "IPv6DAddr", &len)) != NULL) {
                            if (sscanf(str, "%3u.%3u.%3u.%3u", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]) == 4) {
                                if ((str = cyg_httpd_form_varable_string(p, "DIPMaskValue", &len)) != NULL) {
                                    if (sscanf(str, "%3u.%3u.%3u.%3u", &ipMask[0], &ipMask[1], &ipMask[2], &ipMask[3]) == 4) {
                                        for (i = 0; i < 4; i++) {
                                            newconf.key.frame.ipv6.dip.value[i] = ipAddr[i] & 0xFF;
                                            newconf.key.frame.ipv6.dip.mask[i] = ipMask[i];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
#endif /* defined(VTSS_ARCH_SERVAL) */

                //dscp choice field
                newconf.key.frame.ipv6.dscp.in_range = FALSE;
                newconf.key.frame.ipv6.dscp.vr.v.mask = 0;
                newconf.key.frame.ipv6.dscp.vr.v.value = 0;
                if (cyg_httpd_form_varable_int(p, "IPv6DSCPChoice", &var_value1)) {
                    if (var_value1 == 1) {
                        if (cyg_httpd_form_varable_int(p, "keyDSCPSpcValue", &var_value2)) {
                            newconf.key.frame.ipv6.dscp.vr.v.mask = 0x3F;
                            newconf.key.frame.ipv6.dscp.vr.v.value = var_value2;
                        }
                    }
                    if (var_value1 == 2) {
                        if (cyg_httpd_form_varable_int(p, "keyDSCPRngStart", &var_value2)) {
                            newconf.key.frame.ipv6.dscp.in_range = TRUE;
                            newconf.key.frame.ipv6.dscp.vr.r.low = var_value2;
                            if (cyg_httpd_form_varable_int(p, "keyDSCPRngLast", &var_value3)) {
                                newconf.key.frame.ipv6.dscp.vr.r.high = var_value3;
                            }
                        }
                    }
                }
                break;
            default:
                //through an error unexpected value in var_value1
                break;
            }// frame type switch case block
        }// frame type

        //qce action configuration
        //action configuration Format-(default:0, class 0:1 .. class 7:8) - Class:var_value1
        if (cyg_httpd_form_varable_int(p, "actionCoSSel",  &var_value1)) {
            if (var_value1 != 0) {
                QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_PRIO, 1);
                newconf.action.prio = (var_value1 - 1);
            }
        }
        // DP
        if (cyg_httpd_form_varable_int(p, "actionDPSel",  &var_value1)) {
            if (var_value1 != 0) {
                QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_DP, 1);
                newconf.action.dp = (var_value1 - 1);
            }
        }
        // DSCP
        if (cyg_httpd_form_varable_int(p, "actionDSCPSel", &var_value1)) {
            QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_DSCP, 0);
            if (var_value1 != 0) {
                QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_DSCP, 1);
                newconf.action.dscp = var_value1 - 1;
            }
        }

#if defined(VTSS_ARCH_SERVAL)
        // PCP
        if (cyg_httpd_form_varable_int(p, "actionPCPSel",  &var_value1)) {
            if (var_value1 != 0) {
                QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_PCP_DEI, 1);
                newconf.action.pcp = (var_value1 - 1);
            }
        }
        // DEI
        if (cyg_httpd_form_varable_int(p, "actionDEISel",  &var_value1)) {
            if (var_value1 != 0) {
                QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_PCP_DEI, 1);
                newconf.action.dei = (var_value1 - 1);
            }
        }
        // Policy
        if (cyg_httpd_form_varable_int(p, "actionPolicy",  &var_value1)) {
            QCE_ENTRY_CONF_ACTION_SET(newconf.action.action_bits, QOS_QCE_ACTION_POLICY, 1);
            newconf.action.policy_no = var_value1;
        }
#endif /* defined(VTSS_ARCH_SERVAL) */

        // add the qce entry finally
        if (qos_mgmt_qce_entry_add(QCL_USER_STATIC, QCL_ID_END, next_qce_id, &newconf) == VTSS_OK) {
            T_D("QCE ID %d %s ", newconf.id, add ? "added" : "modified");
            if (next_qce_id) {
                T_D("before QCE ID %d\n", next_qce_id);
            } else {
                T_D("last\n");
            }
            redirect(p, errors ? STACK_ERR_URL : "/qcl_v2.htm");
        } else {
            T_E("QCL Add failed\n");
            redirect(p, errors ? STACK_ERR_URL : "/qcl_v2.htm?error=1");
        }
    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        /*
           Format:
           <Del>         : qceConfigFlag=1 and qce_id=<qce_id to delete>
           <Move>        : qceConfigFlag=2 and qce_id=<qce_id to move>
           <Edit>        : qceConfigFlag=3 and qce_id=<qce_id to edit>
           <Insert>      : qceConfigFlag=4 and qce_id=<next_qce_id to insert before>
           <Add to Last> : qceConfigFlag=4 and qce_id=0 (inserts last if next_qce_id=0)
        */
        u8   bit_val;
        char buf[MGMT_PORT_BUF_SIZE];
        cyg_httpd_start_chunked("html");
        vtss_isid_t isid = VTSS_ISID_GLOBAL;
        if ((var_string = cyg_httpd_form_varable_find(p, "qceConfigFlag")) != NULL) {
            qce_flag = atoi(var_string);
            if ((var_string = cyg_httpd_form_varable_find(p, "qce_id")) != NULL) {
                qce_id = atoi(var_string);
            }
            switch (qce_flag) {
            case 1://<Del> qce entry
                (void) qos_mgmt_qce_entry_del(isid, QCL_USER_STATIC, QCL_ID_END, qce_id);
                break;
            case 2://<Move> qce entry
                if (cyg_httpd_form_varable_int(p, "qce_id",  &var_value1)) {
                    qce_id = var_value1;
                    if (qos_mgmt_qce_entry_get(isid, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_conf, 0) == VTSS_OK) {
                        if (qos_mgmt_qce_entry_get(isid, QCL_USER_STATIC, QCL_ID_END, qce_id, &newconf, 1) == VTSS_OK) {
                            if (qos_mgmt_qce_entry_add(QCL_USER_STATIC, QCL_ID_END, qce_id, &newconf) != VTSS_OK) {
                                T_W("qos_mgmt_qce_entry_add() failed");
                            }
                        }
                    } else {
                        T_W ("qos_mgmt_qce_entry_get() failed");
                    }
                }
                break;
            case 3://<Edit> This format is for 'qos/html/qcl_v2_edit.htm' webpage
            case 4://<Insert> or <Add to Last> This format is for 'qos/html/qcl_v2_edit.htm' webpage
                // Create a list of present switches in usid order separated by "#"
                (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
                while (switch_iter_getnext(&sit)) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d", sit.first ? "" : "#", sit.usid);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                if (qce_flag == 4) {
                    cyg_httpd_write_chunked(",-1", 3);
                    cyg_httpd_end_chunked();
                    return -1; // return from <Insert> or <Add to Last> case
                }
                /* Format:
                 * <qce_info>;<frame_info>
                 *
                 * qce_info        :== <sid_next>/<port_list>/<tag>/<vid>/<pcp>/<dei>/<itag>/<ivid>/<ipcp>/<idei>/<smac>/<dmac_type>/<dmac>/<act_cos>/<act_dpl>/<act_dscp>/<act_pcp>/<act_dei>/<act_policy>
                 *   sid_next      :== <usid_list>,<usid>,<next_qce_id>
                 *     usid_list   :== <usid l>#<usid m>#<usid n> // List of present (active) switches in usid order.
                 *     usid        :== 1..16 or -1 for all switches
                 *     next_qce_id :== 0..256
                 *   port_list     :== list of disabled uports separated by ','. Empty list ~ All ports enabled
                 *   tag           :== 0: Any, 1: Untagged, 2: Tagged, 3: C-Tagged, 4: S-Tagged
                 *   vid           :== <vid_low>,<vid_high>
                 *     vid_low     :== -1 or 1..4095  // -1: Any
                 *     vid_high    :== -1 or 1..4095  // -1: Use vid_low as specific, else high range.
                 *   pcp           :== <pcp_low>,<pcp_high>
                 *     pcp_low     :== -1..7   // -1 is Any
                 *     pcp_high    :== -1..7    // Not used if pcp_low == -1. If pcp_low != pcp_high then it is a range, else use pcp_low.
                 *   dei           :== -1: Any , else dei value
                 *   itag          :== 0: Any, 1: Untagged, 2: Tagged, 3: C-Tagged, 4: S-Tagged
                 *   ivid          :== <vid_low>,<vid_high>
                 *     vid_low     :== -1 or 1..4095  // -1: Any
                 *     vid_high    :== -1 or 1..4095  // -1: Use vid_low as specific, else high range.
                 *   ipcp          :== <pcp_low>,<pcp_high>
                 *     pcp_low     :== -1..7   // -1 is Any
                 *     pcp_high    :== -1..7    // Not used if pcp_low == -1. If pcp_low != pcp_high then it is a range, else use pcp_low.
                 *   idei          :== -1: Any , else dei value
                 *   smac          :== String  // "Any" or "xx-xx-xx-xx-xx-xx"
                 *   dmac_type     :== 0..3    // One of qos_qce_dmac_type_t
                 *   dmac          :== String  // "" (use dmac_type) or "xx-xx-xx-xx-xx-xx"
                 *   act_cos       :== -1..7   // -1 is no action, else classify to selected value
                 *   act_dpl       :== -1..3   // -1 is no action, else classify to selected value
                 *   act_dscp      :== -1..63  // -1 is no action, else classify to selected value
                 *   act_pcp       :== -1..7   // -1 is no action, else classify to selected value
                 *   act_dei       :== -1..3   // -1 is no action, else classify to selected value
                 *   act_policy    :== -1..63  // -1 is no action, else classify to selected value
                 *
                 * frame_info      :== <frame_type>/<type_any> or <type_eth> or <type_llc> or <type_snap> or <type_ipv4> or <type_ipv6>
                 *   frame_type    :== 0..5    // One of vtss_qce_type_t
                 *   type_any      :== String "Any"
                 *   type_eth      :== <eth_spec>/<eth_val>
                 *     eth_spec    :== 0 or 1 where 0 is Any and 1 is use eth_val
                 *     eth_val     :== 0 or 0600..ffff - value in hex (without 0x prepended)
                 *   type_llc      :== <ssap_spec>/<dsap_spec>/<ctrl_spec>/<ssap_val>/<dsap_val>/<ctrl_val>
                 *     ssap_spec   :== -1 for Any, else value in decimal
                 *     dsap_spec   :== -1 for Any, else value in decimal
                 *     ctrl_spec   :== -1 for Any, else value in decimal
                 *     ssap_val    :== 0 or value in hex (without 0x prepended)
                 *     dsap_val    :== 0 or value in hex (without 0x prepended)
                 *     ctrl_val    :== 0 or value in hex (without 0x prepended)
                 *   type_snap     :== <snap_spec>/<snap_hbyte>/<snap_lbyte>
                 *     snap_spec   :== 0 or 1 where 0 is Any and 1 is use snap_hbyte and snap_lbyte
                 *     snap_hbyte  :== 00..ff - value in hex (without 0x prepended)
                 *     snap_lbyte  :== 00..ff - value in hex (without 0x prepended)
                 *   type_ipv4     :== <proto>/<sip>/<ip_frag>/<dscp_low>/<dscp_high>[/<sport_low>/<sport_high>/<dport_low>/<dport_high>]
                 *     proto       :== -1 for Any, else value in decimal
                 *     sip         :== <ip>,<mask>
                 *       ip        :== String // "Any" or "x.y.z.w"
                 *       mask      :== String // "x.y.z.w"
                 *     dip         :== <ip>,<mask>
                 *       ip        :== String // "Any" or "x.y.z.w"
                 *       mask      :== String // "x.y.z.w"
                 *     ip_frag     :== 0: Any, 1: No, 2: Yes
                 *     dscp_low    :== -1..63 // -1 if Any
                 *     dscp_high   :== -1..63 // -1: Use dscp_low as specific, else high range.
                 *     sport_low   :== -1..65535 // -1 if Any                                         Only present if proto is TCP or UDP!
                 *     sport_high  :== -1..65535 // -1: Use sport_low as specific, else high range.   Only present if proto is TCP or UDP!
                 *     dport_low   :== -1..65535 // -1 if Any                                         Only present if proto is TCP or UDP!
                 *     dport_high  :== -1..65535 // -1: Use dport_low as specific, else high range.   Only present if proto is TCP or UDP!
                 *   type_ipv6     :== <proto>/<sip>/<dscp_low>/<dscp_high>[/<sport_low>/<sport_high>/<dport_low>/<dport_high>]
                 *     proto       :== -1 for Any, else value in decimal
                 *     sip_ip      :== String // "Any" or "x.y.z.w"
                 *     sip_mask    :== String // "x.y.z.w"
                 *     dip_ip      :== String // "Any" or "x.y.z.w"
                 *     dip_mask    :== String // "x.y.z.w"
                 *     dscp_low    :== -1..63 // -1 if Any
                 *     dscp_high   :== -1..63 // -1: Use dscp_low as specific, else high range.
                 *     sport_low   :== -1..65535 // -1 if Any                                         Only present if proto is TCP or UDP!
                 *     sport_high  :== -1..65535 // -1: Use sport_low as specific, else high range.   Only present if proto is TCP or UDP!
                 *     dport_low   :== -1..65535 // -1 if Any                                         Only present if proto is TCP or UDP!
                 *     dport_high  :== -1..65535 // -1: Use dport_low as specific, else high range.   Only present if proto is TCP or UDP!
                 */
                cyg_httpd_write_chunked(",", 1);

                if (qos_mgmt_qce_entry_get(isid, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_conf, 0) == VTSS_OK) {
                    u8 mask;
                    int value;
#if defined(VTSS_ARCH_SERVAL)
                    int v1, v2;
#endif
                    if (qos_mgmt_qce_entry_get(isid, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_next, 1) == VTSS_OK) {
                        next_qce_id = qce_next.id;
                    } else {
                        next_qce_id = 0;//if it is last entry
                    }
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d/",
                                  (int)((qce_conf.isid == VTSS_ISID_GLOBAL) ? -1 : topo_isid2usid(qce_conf.isid)),
                                  (int)next_qce_id);
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    first = TRUE;
                    for (iport = VTSS_PORT_NO_START; iport < VTSS_FRONT_PORT_COUNT; iport++) {
                        if (!VTSS_PORT_BF_GET(qce_conf.port_list, iport)) { // if not checked
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), (first) ? "%u" : ",%u", iport + 1);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                            first = FALSE;
                        }
                    }
#if defined(VTSS_ARCH_SERVAL)
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                                  qcl_qce_tag_type(QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_TAG),
                                                   QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_S_TAG)));
#else
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                                  qcl_qce_tag_type(QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_TAG),
                                                   VTSS_VCAP_BIT_ANY));
#endif
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    bit_val = QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_DEI);
                    mask = qce_conf.key.pcp.mask;
                    value = (mask == 0) ? -1 : qce_conf.key.pcp.value;

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d,%d/%d,%d/%d",
                                  (qce_conf.key.vid.in_range == TRUE) ?
                                  (qce_conf.key.vid.vr.r.low) :
                                  (qce_conf.key.vid.vr.v.mask == 0) ? -1 :
                                  (qce_conf.key.vid.vr.v.value),
                                  (qce_conf.key.vid.in_range == TRUE) ?
                                  (qce_conf.key.vid.vr.r.high) : -1,
                                  value,//pcp low value
                                  (mask == 0) ? -1 :
                                  (int )(value + ((mask == 0xFF) ? 0 : (mask == 0xFE) ? 1 : 3)),//pcp high
                                  (bit_val != 0) ? (bit_val - 1) : (-1));
                    cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                                  qcl_qce_tag_type(qce_conf.key.inner_tag.tagged, qce_conf.key.inner_tag.s_tag));
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    if (qce_conf.key.inner_tag.vid.type  != VTSS_VCAP_VR_TYPE_VALUE_MASK) {
                        v1 = qce_conf.key.inner_tag.vid.vr.r.low;
                        v2 = qce_conf.key.inner_tag.vid.vr.r.high;
                    } else if (qce_conf.key.inner_tag.vid.vr.v.mask) {
                        v1 = qce_conf.key.inner_tag.vid.vr.v.value;
                        v2 = -1;
                    } else {
                        v1 = v2 = -1;
                    }
                    mask = qce_conf.key.inner_tag.pcp.mask;
                    value = (mask == 0) ? -1 : qce_conf.key.inner_tag.pcp.value;
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d,%d/%d,%d/%d",
                                  v1, v2,
                                  value, // pcp low value
                                  (mask == 0) ? -1 :  (int )(value + ((mask == 0xFF) ? 0 : (mask == 0xFE) ? 1 : 3)),//pcp high
                                  qce_conf.key.inner_tag.dei - 1);
#else
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/-1,-1/-1,-1/-1"); // Dummy inner tag
#endif /* defined(VTSS_ARCH_SERVAL) */
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s/%d",
                                  (qce_conf.key.smac.mask[0] == 0 &&
                                   qce_conf.key.smac.mask[1] == 0 &&
                                   qce_conf.key.smac.mask[2] == 0 &&
                                   qce_conf.key.smac.mask[3] == 0 &&
                                   qce_conf.key.smac.mask[4] == 0 &&
                                   qce_conf.key.smac.mask[5] == 0) ? "Any" : (misc_mac_txt(qce_conf.key.smac.value, str_buff2)),
                                  (QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_DMAC_TYPE)));
                    cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                                  (qce_conf.key.dmac.mask[0] == 0 &&
                                   qce_conf.key.dmac.mask[1] == 0 &&
                                   qce_conf.key.dmac.mask[2] == 0 &&
                                   qce_conf.key.dmac.mask[3] == 0 &&
                                   qce_conf.key.dmac.mask[4] == 0 &&
                                   qce_conf.key.dmac.mask[5] == 0) ? "" : (misc_mac_txt(qce_conf.key.dmac.value, str_buff2)));
#else
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/"); // Empty dmac
#endif /* defined(VTSS_ARCH_SERVAL) */
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d",
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PRIO))    ? qce_conf.action.prio      : -1,
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_DP))      ? qce_conf.action.dp        : -1,
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_DSCP))    ? qce_conf.action.dscp      : -1);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_SERVAL)
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d",
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf.action.pcp       : -1,
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf.action.dei       : -1,
                                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_POLICY))  ? qce_conf.action.policy_no : -1);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* defined(VTSS_ARCH_SERVAL) */

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ";%u", qce_conf.type);
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    switch (qce_conf.type) {
                    case VTSS_QCE_TYPE_ANY:
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", "Any");
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        break;
                    case VTSS_QCE_TYPE_ETYPE://Format: /[Eternet Type]/[Ethernet value]
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%04x",
                                      (qce_conf.key.frame.etype.etype.mask[0] != 0 ||
                                       qce_conf.key.frame.etype.etype.mask[1] != 0) ? 1 : 0,
                                      qce_conf.key.frame.etype.etype.value[0] << 8 |
                                      qce_conf.key.frame.etype.etype.value[1]);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        break;
                    case VTSS_QCE_TYPE_LLC://Format: /[SSAP Value]/[DSAP Value]/[Control Value]
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d/%02x/%02x/%02x",
                                      (qce_conf.key.frame.llc.ssap.mask == 0) ? -1 : qce_conf.key.frame.llc.ssap.value,
                                      (qce_conf.key.frame.llc.dsap.mask == 0) ? -1 : qce_conf.key.frame.llc.dsap.value,
                                      (qce_conf.key.frame.llc.control.mask == 0) ? -1 : qce_conf.key.frame.llc.control.value,
                                      qce_conf.key.frame.llc.ssap.value,
                                      qce_conf.key.frame.llc.dsap.value,
                                      qce_conf.key.frame.llc.control.value);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        break;
                    case VTSS_QCE_TYPE_SNAP://Format: [Eternet Type]/[Ethernet value 0]/[Ethernet value 1]
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%02x/%02x",
                                      (qce_conf.key.frame.snap.pid.mask[0] == 0 &&
                                       qce_conf.key.frame.snap.pid.mask[1] == 0) ? 0 : 1,
                                      qce_conf.key.frame.snap.pid.value[0],
                                      qce_conf.key.frame.snap.pid.value[1]);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        break;
                    case VTSS_QCE_TYPE_IPV4://Format: - [proto no]/[sip],[sip mask]/[ip fragment]/[dscp low]/[dscp high] -
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%s,%s/%s,%s/%d/%d/%d",
                                      (qce_conf.key.frame.ipv4.proto.mask == 0) ? -1 :
                                      qce_conf.key.frame.ipv4.proto.value,
                                      (qce_conf.key.frame.ipv4.sip.mask == 0) ? "Any" :
                                      misc_ipv4_txt(qce_conf.key.frame.ipv4.sip.value, str_buff1),
                                      misc_ipv4_txt(qce_conf.key.frame.ipv4.sip.mask, str_buff2),
#if defined(VTSS_ARCH_SERVAL)
                                      (qce_conf.key.frame.ipv4.dip.mask == 0) ? "Any" :
                                      misc_ipv4_txt(qce_conf.key.frame.ipv4.dip.value, str_buff3),
                                      misc_ipv4_txt(qce_conf.key.frame.ipv4.dip.mask, str_buff4),
#else
                                      "Any", "0.0.0.0",
#endif /* defined(VTSS_ARCH_SERVAL) */
                                      (QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits,
                                                              QOS_QCE_IPV4_FRAGMENT)),
                                      (qce_conf.key.frame.ipv4.dscp.in_range == TRUE) ?
                                      qce_conf.key.frame.ipv4.dscp.vr.r.low :
                                      (qce_conf.key.frame.ipv4.dscp.vr.v.mask == 0) ? -1 :
                                      qce_conf.key.frame.ipv4.dscp.vr.v.value,
                                      (qce_conf.key.frame.ipv4.dscp.in_range == TRUE) ?
                                      qce_conf.key.frame.ipv4.dscp.vr.r.high : -1);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        // format if TCP || UDP: /[sport low]/[sport high]/[dport low]/[dport high]
                        if (qce_conf.key.frame.ipv4.proto.value == 17 || qce_conf.key.frame.ipv4.proto.value == 6) { //UDP or TCP
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d/%d",
                                          (qce_conf.key.frame.ipv4.sport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv4.sport.vr.r.low :
                                          (qce_conf.key.frame.ipv4.sport.vr.v.mask == 0) ? -1 :
                                          qce_conf.key.frame.ipv4.sport.vr.v.value,
                                          (qce_conf.key.frame.ipv4.sport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv4.sport.vr.r.high : -1,
                                          (qce_conf.key.frame.ipv4.dport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv4.dport.vr.r.low :
                                          (qce_conf.key.frame.ipv4.dport.vr.v.mask == 0) ? -1 :
                                          qce_conf.key.frame.ipv4.dport.vr.v.value,
                                          (qce_conf.key.frame.ipv4.dport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv4.dport.vr.r.high : -1);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                        }
                        break;
                    case VTSS_QCE_TYPE_IPV6://Format: - /[proto no]/[sip - ?]/[dscp low]/[dscp high]
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%s/%u.%u.%u.%u/%s/%u.%u.%u.%u/%d/%d",
                                      (qce_conf.key.frame.ipv6.proto.mask == 0) ? -1 :
                                      (qce_conf.key.frame.ipv6.proto.value),
                                      (qce_conf.key.frame.ipv6.sip.mask[0] == 0 &&
                                       qce_conf.key.frame.ipv6.sip.mask[1] == 0 &&
                                       qce_conf.key.frame.ipv6.sip.mask[2] == 0 &&
                                       qce_conf.key.frame.ipv6.sip.mask[3] == 0) ? "Any" :
                                      qcl_qce_ipv6_txt(&qce_conf.key.frame.ipv6.sip, str_buff1),
                                      qce_conf.key.frame.ipv6.sip.mask[0],
                                      qce_conf.key.frame.ipv6.sip.mask[1],
                                      qce_conf.key.frame.ipv6.sip.mask[2],
                                      qce_conf.key.frame.ipv6.sip.mask[3],
#if defined(VTSS_ARCH_SERVAL)
                                      qcl_qce_ipv6_txt(&qce_conf.key.frame.ipv6.dip, str_buff2),
                                      qce_conf.key.frame.ipv6.dip.mask[0],
                                      qce_conf.key.frame.ipv6.dip.mask[1],
                                      qce_conf.key.frame.ipv6.dip.mask[2],
                                      qce_conf.key.frame.ipv6.dip.mask[3],
#else
                                      "Any", 0, 0, 0, 0,
#endif /* defined(VTSS_ARCH_SERVAL) */
                                      (qce_conf.key.frame.ipv6.dscp.in_range == TRUE) ?
                                      qce_conf.key.frame.ipv6.dscp.vr.r.low :
                                      (qce_conf.key.frame.ipv6.dscp.vr.v.mask == 0) ? -1 :
                                      qce_conf.key.frame.ipv6.dscp.vr.v.value,
                                      (qce_conf.key.frame.ipv6.dscp.in_range == TRUE) ?
                                      qce_conf.key.frame.ipv6.dscp.vr.r.high : -1);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        // format if TCP || UDP: /[sport low]/[sport high]/[dport low]/[dport high]
                        if (((qce_conf.key.frame.ipv6.proto.value) == 17) || ((qce_conf.key.frame.ipv6.proto.value) == 6)) { //UDP or TCP
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d/%d",
                                          (qce_conf.key.frame.ipv6.sport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv6.sport.vr.r.low :
                                          (qce_conf.key.frame.ipv6.sport.vr.v.mask == 0) ? -1 :
                                          qce_conf.key.frame.ipv6.sport.vr.v.value,
                                          (qce_conf.key.frame.ipv6.sport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv6.sport.vr.r.high : -1,
                                          (qce_conf.key.frame.ipv6.dport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv6.dport.vr.r.low :
                                          (qce_conf.key.frame.ipv6.dport.vr.v.mask == 0) ? -1 :
                                          qce_conf.key.frame.ipv6.dport.vr.v.value,
                                          (qce_conf.key.frame.ipv6.dport.in_range == TRUE) ?
                                          qce_conf.key.frame.ipv6.dport.vr.r.high : -1);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                        }
                        break;
                    default:
                        T_W("Invalid qce_conf.type field value");
                        break;
                    }
                }
                cyg_httpd_end_chunked();
                return -1; // return from edit case
            default:
                break;
            }
        }

        /* Format (for 'qcl_v2.htm' webpage only):
         * <usid_list>#<qce_list>
         *
         * usid_list :== <usid l>;<usid m>;<usid n> // List of present (active) switches in usid order.
         *
         * qce_list  :== <qce 1>;<qce 2>;<qce 3>;...<qce n> // List of currently defined QCEs (might be empty).
         *   qce  x  :== <usid>/<qce_id>/<ports>/<key_dmac>/<key_smac>/<key_tag>/<key_vid>/<key_pcp>/<key_dei>/<frame_type>/<act_cos>/<act_dpl>/<act_dscp>/<act_pcp>/<act_dei>/<act_policy>
         *     usid       :== 1..16 or -1 for all switches
         *     qce_id     :== 1..256
         *     ports      :== String  // List of ports e.g. "1,3,5,7,9-53"
         *     key_dmac   :== String  // "Any", "Unicast", "Multicast", "Broadcast" or "xx-xx-xx-xx-xx-xx" (Serval1 only)
         *     key_smac   :== String  // "Any", "xx-xx-xx" (Jag1 only) or "xx-xx-xx-xx-xx-xx" (all other)
         *     key_tag    :== String  // "Any", "Untagged", "Tagged", "C-Tagged" (Serval1 only) or "S-Tagged" (Serval1 only)
         *     key_vid    :== String  // "Any" or vid or vid-vid
         *     key_pcp    :== String  // "Any" or pcp or pcp-pcp
         *     key_dei    :== 0..2    // 0: Any, 1: 0, 2: 1
         *     frame_type :== 0..5    // One of vtss_qce_type_t
         *     act_cos    :== -1..7   // -1 is no action, else classify to selected value
         *     act_dpl    :== -1..3   // -1 is no action, else classify to selected value
         *     act_dscp   :== -1..63  // -1 is no action, else classify to selected value
         *     act_pcp    :== -1..7   // -1 is no action, else classify to selected value
         *     act_dei    :== -1..1   // -1 is no action, else classify to selected value
         *     act_policy :== -1..63  // -1 is no action, else classify to selected value
         *
         */

        // Create a list of present switches in usid order separated by ";"
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d", sit.first ? "" : ";", sit.usid);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        // Create a list of currently defined QCEs
        first = TRUE;
        qce_id = QCE_ID_NONE;
        while (qos_mgmt_qce_entry_get(VTSS_ISID_GLOBAL, QCL_USER_STATIC, QCL_ID_END, qce_id, &qce_conf, 1) == VTSS_OK) {
            char vid_txt[24];
            char pcp_txt[24];

            qce_id = qce_conf.id;
            (void) qcl_qce_port_list_txt(&qce_conf, buf); // Create port list

#if defined(VTSS_ARCH_SERVAL)
            if (qce_conf.key.dmac.mask[0] || qce_conf.key.dmac.mask[1] || qce_conf.key.dmac.mask[2] ||
                qce_conf.key.dmac.mask[3] || qce_conf.key.dmac.mask[4] || qce_conf.key.dmac.mask[5]) {
                (void)misc_mac_txt(qce_conf.key.dmac.value, str_buff1);
            } else
#endif /* defined(VTSS_ARCH_SERVAL) */
            {
                switch (QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_DMAC_TYPE)) {
                case QOS_QCE_DMAC_TYPE_ANY:
                    sprintf(str_buff1, "Any");
                    break;
                case QOS_QCE_DMAC_TYPE_UC:
                    sprintf(str_buff1, "Unicast");
                    break;
                case QOS_QCE_DMAC_TYPE_MC:
                    sprintf(str_buff1, "Multicast");
                    break;
                case QOS_QCE_DMAC_TYPE_BC:
                    sprintf(str_buff1, "Broadcast");
                    break;
                default:
                    sprintf(str_buff1, "?");
                    break;
                }
            }

            if (!qce_conf.key.pcp.mask) {
                sprintf(pcp_txt, "Any");
            } else if (qce_conf.key.pcp.mask == 0xFF) {
                sprintf(pcp_txt, "%u", qce_conf.key.pcp.value);
            } else {
                sprintf(pcp_txt, "%u-%u", qce_conf.key.pcp.value, (uchar)(qce_conf.key.pcp.value + ((~qce_conf.key.pcp.mask) & 0xFF)));
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d/%u/%s/%s/%s/%s/%s/%s/%u/%u/%d/%d/%d",
                          (first) ? "#" : ";",
                          (qce_conf.isid == VTSS_ISID_GLOBAL) ? -1 : topo_isid2usid(qce_conf.isid),
                          qce_conf.id,
                          buf,
                          str_buff1,
#if defined(VTSS_ARCH_JAGUAR_1)
                          (qce_conf.key.smac.mask[0] == 0 &&
                           qce_conf.key.smac.mask[1] == 0 &&
                           qce_conf.key.smac.mask[2] == 0) ? "Any" : (misc_oui_addr_txt(qce_conf.key.smac.value, str_buff2)),
#else
                          (qce_conf.key.smac.mask[0] == 0 &&
                           qce_conf.key.smac.mask[1] == 0 &&
                           qce_conf.key.smac.mask[2] == 0 &&
                           qce_conf.key.smac.mask[3] == 0 &&
                           qce_conf.key.smac.mask[4] == 0 &&
                           qce_conf.key.smac.mask[5] == 0) ? "Any" : (misc_mac_txt(qce_conf.key.smac.value, str_buff2)),
#endif /* defined(VTSS_ARCH_JAGUAR_1) */
#if defined(VTSS_ARCH_SERVAL)
                          qcl_qce_tag_type_text(QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_TAG),
                                                QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_S_TAG)),
#else
                          qcl_qce_tag_type_text(QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_TAG),
                                                VTSS_VCAP_BIT_ANY),
#endif /* defined(VTSS_ARCH_SERVAL) */
                          qcl_qce_range_txt_u16(&qce_conf.key.vid, vid_txt),
                          pcp_txt,
                          QCE_ENTRY_CONF_KEY_GET(qce_conf.key.key_bits, QOS_QCE_VLAN_DEI),
                          qce_conf.type,
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PRIO))    ? qce_conf.action.prio      : -1,
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_DP))      ? qce_conf.action.dp        : -1,
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_DSCP))    ? qce_conf.action.dscp      : -1);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d",
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf.action.pcp       : -1,
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf.action.dei       : -1,
                          (QCE_ENTRY_CONF_ACTION_GET(qce_conf.action.action_bits, QOS_QCE_ACTION_POLICY))  ? qce_conf.action.policy_no : -1);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* defined(VTSS_ARCH_SERVAL) */
            first = FALSE;
        }// end of while loop
        cyg_httpd_end_chunked();
    }// end of Get method response section
    return -1; // Do not further search the file system.
}// end of qce_handler function

/* support for qce status*/
static void qce_overview(CYG_HTTPD_STATE *p, int qcl_user, qos_qce_entry_conf_t *qce_conf)
{
    int ct;
    char buf[MGMT_PORT_BUF_SIZE];
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%u/%s/%d/%d/%d/%d",
                  qce_conf->conflict ? "Yes" : "No",
                  qcl_user,
                  qce_conf->id,
                  qcl_qce_port_list_txt(qce_conf, buf),
                  qce_conf->type,
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_PRIO))  ? qce_conf->action.prio        : -1,
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_DP))    ? qce_conf->action.dp          : -1,
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_DSCP))  ? qce_conf->action.dscp        : -1);
    cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_SERVAL)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d/%d/%d",
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf->action.pcp       : -1,
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_PCP_DEI)) ? qce_conf->action.dei       : -1,
                  (QCE_ENTRY_CONF_ACTION_GET(qce_conf->action.action_bits, QOS_QCE_ACTION_POLICY))  ? qce_conf->action.policy_no : -1);
    cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* defined(VTSS_ARCH_SERVAL) */
    cyg_httpd_write_chunked(";", 1);
}

static cyg_int32 handler_stat_qcl_v2(CYG_HTTPD_STATE *p)
{
    vtss_isid_t          isid;
    vtss_qce_id_t        qce_id;
    qos_qce_entry_conf_t qce_conf;
    int                  ct, qcl_user = -1, conflict, user_cnt;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PORT)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        isid = web_retrieve_request_sid(p); /* Includes USID = ISID */

        if ((cyg_httpd_form_varable_int(p, "ConflictResolve", &conflict))) {
            if (conflict == 1) {
                (void)qos_mgmt_qce_conflict_resolve(isid, QCL_USER_STATIC, QCL_ID_END);
            }
        }
        if ((cyg_httpd_form_varable_int(p, "qclUser", &qcl_user))) {
        }

        /* Format:
         * <qcl_info>|<qce_list>
         *
         * qcl_info       :== <sel_user>/<user 1>/<user 2>/...<user n>
         *   sel_user     :== -2: Show Conflict, -1: Show Combined, 0: Show Static, 1: Show Voice VLAN
         *   user x       :== 0..n    // List of defined users to show in user selector between "Combined" and "Conflict".
         *
         * qce_list       :== <qce 1>;<qce 2>;<qce 3>;...<qce n> // List of currently defined QCEs (might be empty).
         *   qce  x       :== <conflict>/<user>/<qce_id>/<frame_type>/<ports>/<act_class>/<act_dpl>/<act_dscp>
         *     conflict   :== String  // "Yes" or "No"
         *     user       :== 0..n    // One of the defined users
         *     qce_id     :== 1..256
         *     ports      :== String  // List of ports e.g. "1,3,5,7,9-53"
         *     frame_type :== 0..5    // One of vtss_qce_type_t
         *     act_cos    :== -1..7   // -1 is no action, else classify to selected value
         *     act_dpl    :== -1..3   // -1 is no action, else classify to selected value
         *     act_dscp   :== -1..63  // -1 is no action, else classify to selected value
         *     act_pcp    :== -1..7   // -1 is no action, else classify to selected value
         *     act_dei    :== -1..1   // -1 is no action, else classify to selected value
         *     act_policy :== -1..63  // -1 is no action, else classify to selected value
         */

        cyg_httpd_start_chunked("html");
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d", qcl_user);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        for (user_cnt = QCL_USER_STATIC; user_cnt < QCL_USER_CNT; user_cnt++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d", user_cnt);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_write_chunked("|", 1);
        if (qcl_user < 0) { // Show Combined (-1) or Conflict (-2)
            for (user_cnt = QCL_USER_CNT - 1; user_cnt >= QCL_USER_STATIC; user_cnt--) {
                qce_id = QCE_ID_NONE;
                while (qos_mgmt_qce_entry_get(isid, user_cnt, QCL_ID_END, qce_id, &qce_conf, 1) == VTSS_OK) {
                    qce_id = qce_conf.id;
                    if (qcl_user == -1 || (qcl_user == -2 && qce_conf.conflict)) {
                        qce_overview(p, user_cnt, &qce_conf);
                    }
                }
            }
        } else { // Show selected user only (0..n)
            qce_id = QCE_ID_NONE;
            while (qos_mgmt_qce_entry_get(isid, qcl_user, QCL_ID_END, qce_id, &qce_conf, 1) == VTSS_OK) {
                qce_id = qce_conf.id;
                qce_overview(p, qcl_user, &qce_conf);
            }
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}
#endif /* VTSS_FEATURE_QCL_V2 */
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
static cyg_int32 handler_config_dscp_port_config (CYG_HTTPD_STATE *p)
{
    vtss_isid_t     isid;
    port_iter_t     pit;
    qos_port_conf_t conf;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif
    isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
            return -1;
        }
        int val, errors = 0;
        vtss_rc rc;
        char search_str[32];
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if ((rc = qos_port_conf_get(isid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_get(%u, %d): failed rc = %d", isid, pit.uport, rc);
                continue;
            }
            sprintf(search_str, "enable_%d", pit.uport);
            /* Translate */
            conf.dscp_translate = FALSE;
            if (cyg_httpd_form_varable_find(p, search_str)) {/* "on" if checked */
                conf.dscp_translate = TRUE;
            }
            /* Classify */
            if (cyg_httpd_form_variable_int_fmt(p, &val, "classify_%u", pit.uport)) {
                conf.dscp_imode = val;
            }
            /* Rewrite */
            if (cyg_httpd_form_variable_int_fmt(p, &val, "rewrite_%u", pit.uport)) {
                switch (val) {
                case 0:/* Disable */
                case 1:/* Ensable */
                    conf.dscp_emode = val;
                    break;
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
                case 2:/* Remap DP Unaware */
                    conf.dscp_emode = VTSS_DSCP_EMODE_REMAP;
                    break;
                case 3:/* Remap DP aware */
                    conf.dscp_emode = VTSS_DSCP_EMODE_REMAP_DPA;
                    break;
#else
                case 2:/* Remap */
                    conf.dscp_emode = VTSS_DSCP_EMODE_REMAP;
                    break;
#endif
                default:
                    /* printf Error msg here */
                    break;
                }
            }
            (void) qos_port_conf_set(isid, pit.iport, &conf);
        }
        redirect(p, errors ? STACK_ERR_URL : "/qos_port_dscp_config.htm");
    } else {//GET Method
        /*Format:
         <port number>/<ingr. translate>/<ingr. classify>/<egr. rewrite>|
        */
        int ct;
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(isid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u#%u#%u#%u",
                          pit.first ? "" : "|",
                          pit.uport,
                          conf.dscp_translate,
                          conf.dscp_imode,
                          conf.dscp_emode);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}
static cyg_int32 handler_config_dscp_based_qos_ingr_classi (CYG_HTTPD_STATE *p)
{
    qos_conf_t conf;
    //vtss_isid_t     isid;
    int             cnt;
    int val;
    int errors = 0;
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif
    if (qos_conf_get(&conf) != VTSS_OK) {
        errors++;
        T_D("qos_conf_get(): failed no of errors = %d", errors);
        return -1;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        char search_str[32];
        for (cnt = 0; cnt < 64; cnt++) {
            /* Trust trust_chk_ */
            sprintf(search_str, "trust_chk_%d", cnt);
            conf.dscp.dscp_trust[cnt] = FALSE;
            if (cyg_httpd_form_varable_find(p, search_str)) {/* "on" if checked */
                conf.dscp.dscp_trust[cnt] = TRUE;
            }
            /* Class classify_sel_ */
            if (cyg_httpd_form_variable_int_fmt(p, &val, "classify_sel_%d", cnt)) {
                conf.dscp.dscp_qos_class_map[cnt] = val;
            }
            /* DPL dpl_sel_ */
            if (cyg_httpd_form_variable_int_fmt(p, &val, "dpl_sel_%d", cnt)) {
                conf.dscp.dscp_dp_level_map[cnt] = val;
            }
        }
        (void) qos_conf_set(&conf);
        redirect(p, errors ? STACK_ERR_URL : "/dscp_based_qos_ingr_classifi.htm");
    } else {/* GET Method */
        /*Format:
         <dscp number>#<trust>#<QoS class>#<dpl>|
        */
        int ct;
        cyg_httpd_start_chunked("html");
        for (cnt = 0; cnt < 64; cnt++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d#%d#%u#%d",
                          (cnt != 0) ? "|" : "",
                          cnt,
                          conf.dscp.dscp_trust[cnt],
                          conf.dscp.dscp_qos_class_map[cnt],
                          conf.dscp.dscp_dp_level_map[cnt]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}
static cyg_int32 handler_config_dscp_translation (CYG_HTTPD_STATE *p)
{
    qos_conf_t      conf;
    int             cnt;
    int             temp;
    int errors = 0;
    char search_str[32];
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif
    if (qos_conf_get(&conf) != VTSS_OK) {
        errors++;
        T_D("qos_conf_get(): failed no of errors = %d", errors);
        return -1;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        for (cnt = 0; cnt < 64; cnt++) {
            /*DSCP translate */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "trans_sel_%d", cnt)) {
                conf.dscp.translate_map[cnt] = temp;
            }
            /* classify */
            conf.dscp.ingress_remark[cnt] = FALSE;
            sprintf(search_str, "classi_chk_%d", cnt);
            if (cyg_httpd_form_varable_find(p, search_str)) {
                conf.dscp.ingress_remark[cnt] = TRUE;
            }
            /* Remap DP0 */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "rmp_dp0_%d", cnt)) {
                conf.dscp.egress_remap[cnt] = temp;
            }
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            /* Remap DP1 */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "rmp_dp1_%d", cnt)) {
                conf.dscp.egress_remap_dp1[cnt] = temp;
            }
#endif
        }
        (void) qos_conf_set(&conf);
        redirect(p, errors ? STACK_ERR_URL : "/dscp_translation.htm");
    } else {/* GET method */
        /*Format
         <dscp number>/<DSCP translate>/<classify>/<remap DP0>/<remap DP1>|...
        */
        int ct;
        cyg_httpd_start_chunked("html");
        for (cnt = 0; cnt < 64; cnt++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d/%d/%d/%d",
                          (cnt != 0) ? "|" : "",
                          cnt,
                          conf.dscp.translate_map[cnt],
                          conf.dscp.ingress_remark[cnt],
                          conf.dscp.egress_remap[cnt]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d",
                          conf.dscp.egress_remap_dp1[cnt]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#endif
        }
        cyg_httpd_write_chunked("|", 1);
        cyg_httpd_end_chunked();
    }
    return -1;
}
static cyg_int32 handler_config_qos_dscp_classification_map (CYG_HTTPD_STATE *p)
{
    qos_conf_t      conf;
    int             cls_cnt, temp;
    int errors = 0;
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif
    if (qos_conf_get(&conf) != VTSS_OK) {
        errors++;
        T_D("qos_conf_get(): failed no of errors = %d", errors);
        return -1;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        for (cls_cnt = 0; cls_cnt < QOS_PORT_PRIO_CNT; cls_cnt++) {
            /* DSCP map to dp 0 */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "dscp_0_%d", cls_cnt)) {
                conf.dscp.qos_class_dscp_map[cls_cnt] = temp;
            }
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            /* DSCP map to dp 1 */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "dscp_1_%d", cls_cnt)) {
                conf.dscp.qos_class_dscp_map_dp1[cls_cnt] = temp;
            }
#endif
        }
        (void) qos_conf_set(&conf);
        redirect(p, errors ? STACK_ERR_URL : "/dscp_classification.htm");
    } else {
        /*Format:
         <dscp class>/<dscp 0>/<dscp 1>|
        */
        int ct;
        cyg_httpd_start_chunked("html");
        for (cls_cnt = 0; cls_cnt < QOS_PORT_PRIO_CNT; cls_cnt++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d/%d",
                          (cls_cnt != 0) ? "|" : "",
                          cls_cnt,
                          conf.dscp.qos_class_dscp_map[cls_cnt]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%d",
                          conf.dscp.qos_class_dscp_map_dp1[cls_cnt]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
#else
            cyg_httpd_write_chunked("/", 1);
#endif
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /*DSCP*/

#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
static cyg_int32 handler_config_qos_port_classification(CYG_HTTPD_STATE *p)
{
    vtss_isid_t     isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int val, errors = 0;
        vtss_rc rc;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if ((rc = qos_port_conf_get(isid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_get(%u, %d): failed rc = %d", isid, pit.uport, rc);
                continue;
            }
            if (cyg_httpd_form_variable_int_fmt(p, &val, "class_%u", pit.uport)) {
                conf.default_prio = val;
            }
            if (cyg_httpd_form_variable_int_fmt(p, &val, "dpl_%u", pit.uport)) {
                conf.default_dpl = val;
            }
            if (cyg_httpd_form_variable_int_fmt(p, &val, "pcp_%u", pit.uport)) {
                conf.usr_prio = val;
            }
            if (cyg_httpd_form_variable_int_fmt(p, &val, "dei_%u", pit.uport)) {
                conf.default_dei = val;
            }
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
            conf.dscp_class_enable = cyg_httpd_form_variable_check_fmt(p, "dscp_enable_%d", pit.uport);
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
            if (cyg_httpd_form_variable_int_fmt(p, &val, "dmac_dip_%u", pit.uport)) {
                conf.dmac_dip = val;
            }
#endif /* defined(VTSS_FEATURE_QCL_DMAC_DIP) */
#if defined(VTSS_ARCH_SERVAL)
            if (cyg_httpd_form_variable_int_fmt(p, &val, "key_type_%u", pit.uport)) {
                conf.key_type = val;
            }
#endif /* defined(VTSS_ARCH_SERVAL) */

            if ((rc = qos_port_conf_set(isid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", isid, pit.uport, rc);
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/qos_port_classification.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        int ct;
        cyg_httpd_start_chunked("html");

        /*
         * Format:
         * <options>|<ports>
         *
         * options :== <show_pcp_dei>,<show_tag_classification>,<show_dscp_classification>,<show_qcl_addr_mode>,<show_qcl_key_type>
         *   show_pcp_dei             :== 0..1 // 0: hide - , 1: show pcp and dei select cells
         *   show_tag_classification  :== 0..1 // 0: hide - , 1: show tag classification
         *   show_dscp_classification :== 0..1 // 0: hide - , 1: show dscp classification
         *   show_qcl_addr_mode       :== 0..1 // 0: hide - , 1: show qcl address mode
         *   show_qcl_key_type        :== 0..1 // 0: hide - , 1: show qcl key type
         *
         * ports :== <port 1>,<port 2>,<port 3>,...<port n>
         *   port x :== <port_no>#<default_pcp>#<default_dei>#<default_class>#<volatile_class>#<default_dpl>#<tag_class>#<dscp_class>#<dmac_dip>#<key_type>
         *     port_no        :== 1..max
         *     default_pcp    :== 0..7
         *     default_dei    :== 0..1
         *     default_class  :== 0..7
         *     volatile_class :== 0..7 or -1 if volatile is not set
         *     default_dpl    :== 0..3 on jaguar, 0..1 on luton26/Serval
         *     tag_class      :== 0..1 // 0: Disabled, 1: Enabled
         *     dscp_class     :== 0..1 // 0: Disabled, 1: Enabled
         *     dmac_dip       :== 0..1 // 0: Disabled, 1: Enabled
         *     key_type       :== 0..3 // One of vtss_vcap_key_type_t
         */

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%u,%u,%u|",
#if defined(QOS_USE_FIXED_PCP_QOS_MAP)
                      0, 0,
#else
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
                      1, 1,
#else
                      1, 0,
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* defined(QOS_USE_FIXED_PCP_QOS_MAP) */

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
                      1,
#else
                      0,
#endif

#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
                      1,
#else
                      0,
#endif
#if defined(VTSS_ARCH_SERVAL)
                      1
#else
                      0
#endif
                     );

        cyg_httpd_write_chunked(p->outbuffer, ct);
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            vtss_prio_t vol_prio = QOS_PORT_PRIO_UNDEF;

            if (qos_port_conf_get(isid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            (void) qos_port_volatile_get_default_prio(isid, pit.iport, &vol_prio);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u#%u#%u#%u#%d#%u#%u#%u#%u#%u",
                          pit.first ? "" : ",",
                          pit.uport,
                          conf.usr_prio,
                          conf.default_dei,
                          conf.default_prio,
                          vol_prio == QOS_PORT_PRIO_UNDEF ? -1 : vol_prio,
                          conf.default_dpl,
                          conf.tag_class_enable,
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
                          conf.dscp_class_enable,
#else
                          0,
#endif
#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
                          conf.dmac_dip,
#else
                          0,
#endif
#if defined(VTSS_ARCH_SERVAL)
                          conf.key_type);
#else
                          0);
#endif
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
static cyg_int32 handler_config_qos_port_classification_map(CYG_HTTPD_STATE *p)
{
    int             ct, i, uport;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t  iport = VTSS_PORT_NO_START;
    qos_port_conf_t conf;
    char            buf[64];

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
            if (iport >= VTSS_PORT_NO_END) {
                errors++;
            }
        } else {
            errors++;
        }

        if (errors || qos_port_conf_get(sid, iport, &conf) != VTSS_OK) {
            errors++;
        } else {
            int            val;
            vtss_rc        rc;

            if (cyg_httpd_form_varable_int(p, "tag_class", &val)) {
                conf.tag_class_enable = val;
            }
            for (i = 0; i < (VTSS_PCPS * 2); i++) {
                if (cyg_httpd_form_variable_int_fmt(p, &val, "default_class_%d", i)) {
                    conf.qos_class_map[i / 2][i % 2] = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "default_dpl_%d", i)) {
                    conf.dp_level_map[i / 2][i % 2] = val;
                }
            }

            if ((rc = qos_port_conf_set(sid, iport, &conf)) < 0) {
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, iport, rc);
                errors++; /* Probably stack error */
            }
        }

        (void)snprintf(buf, sizeof(buf), "/qos_port_classification_map.htm?port=%u", iport2uport(iport));
        redirect(p, errors ? STACK_ERR_URL : buf);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
        }
        if (iport >= VTSS_PORT_NO_END) {
            iport = VTSS_PORT_NO_START;
        }

        if (qos_port_conf_get(sid, iport, &conf) == VTSS_OK) {

            /* Format:
             * <port_no>#<tag_class>#<map>
             *
             * port_no       :== 1..max
             * tag_class     :== 0..1 // 0: Disabled, 1: Enabled
             * map           :== <entry_0>/<entry_1>/...<entry_n> // n is 15.
             *   entry_x     :== <class|dpl>
             *     class     :== 0..7
             *     dpl       :== 0..3
             *
             * The map is organized as follows:
             * Entry corresponds to PCP,       DEI
             *  0                   0          0
             *  1                   0          1
             *  2                   1          0
             *  3                   1          1
             *  4                   2          0
             *  5                   2          1
             *  6                   3          0
             *  7                   3          1
             *  8                   4          0
             *  9                   4          1
             * 10                   5          0
             * 11                   5          1
             * 12                   6          0
             * 13                   6          1
             * 14                   7          0
             * 15                   7          1
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%u",
                          iport2uport(iport),
                          conf.tag_class_enable);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (i = 0; i < (VTSS_PCPS * 2); i++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u",
                              (i != 0) ? "/" : "#",
                              conf.qos_class_map[i / 2][i % 2],
                              conf.dp_level_map[i / 2][i % 2]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

/* Support for a given feature is encoded like this:
   0: No supported
   1: Supported on other ports
   2: Supported on this port */
static u8 port_feature_support(port_cap_t cap, port_cap_t port_cap, port_cap_t mask)
{
    return ((cap & mask) ? ((port_cap & mask) ? 2 : 1) : 0);
}

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL) && defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM)
static cyg_int32 handler_config_qos_port_policers_multi(CYG_HTTPD_STATE *p)
{
    int             ct;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/qos_port_policers_multi.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>#<policers>
             *   port_no  :== 1..max
             *   policers :== <policer 1>/<policer 2>/<policer 3>/...<policer n>
             *     policer x :== <enabled>|<fps>|<rate>
             *       enabled :== 0..1           // 0: no, 1: yes
             *       fps     :== 0..1           // 0: unit for rate is kilobits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *       rate    :== 0..0xffffffff  // actual bit or frame rate
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%lu#%u|%u|%lu/%u|%u|%lu/%u|%u|%lu/%u|%u|%lu,",
                          pit.uport,
                          conf.port_policer[0].enabled,
                          conf.port_policer_ext[0].frame_rate,
                          conf.port_policer[0].policer.rate,
                          conf.port_policer[1].enabled,
                          conf.port_policer_ext[1].frame_rate,
                          conf.port_policer[1].policer.rate,
                          conf.port_policer[2].enabled,
                          conf.port_policer_ext[2].frame_rate,
                          conf.port_policer[2].policer.rate,
                          conf.port_policer[3].enabled,
                          conf.port_policer_ext[3].frame_rate,
                          conf.port_policer[3].policer.rate);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_qos_port_policer_edit_multi(CYG_HTTPD_STATE *p)
{
    int             ct, policer, uport;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t  iport = VTSS_PORT_NO_START;
    qos_port_conf_t conf;
    char            buf[64];

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
            if (iport >= VTSS_PORT_NO_END) {
                errors++;
            }
        } else {
            errors++;
        }

        if (errors || qos_port_conf_get(sid, iport, &conf) != VTSS_OK) {
            errors++;
        } else {
            vtss_bitrate_t rate;
            int            val;
            vtss_rc        rc;

            for (policer = 0; policer < QOS_PORT_POLICER_CNT; policer++) {
                conf.port_policer[policer].enabled = cyg_httpd_form_variable_check_fmt(p, "enabled_%d", policer);

                if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "rate_%u", policer)) {
                    conf.port_policer[policer].policer.rate = rate;
                }

                if (cyg_httpd_form_variable_int_fmt(p, &val, "fps_%d", policer)) {
                    conf.port_policer_ext[policer].frame_rate = (val != 0);
                }

                if (cyg_httpd_form_variable_int_fmt(p, &val, "dp_bypass_level_%d", policer)) {
                    conf.port_policer_ext[policer].dp_bypass_level = val;
                }

                conf.port_policer_ext[policer].unicast      = cyg_httpd_form_variable_check_fmt(p, "unicast_%d",      policer);
                conf.port_policer_ext[policer].multicast    = cyg_httpd_form_variable_check_fmt(p, "multicast_%d",    policer);
                conf.port_policer_ext[policer].broadcast    = cyg_httpd_form_variable_check_fmt(p, "broadcast_%d",    policer);
                conf.port_policer_ext[policer].flooded      = cyg_httpd_form_variable_check_fmt(p, "flooding_%d",     policer);
                conf.port_policer_ext[policer].learning     = cyg_httpd_form_variable_check_fmt(p, "learning_%d",     policer);
                conf.port_policer_ext[policer].flow_control = cyg_httpd_form_variable_check_fmt(p, "flow_control_%d", policer);
            }

            if ((rc = qos_port_conf_set(sid, iport, &conf)) < 0) {
                T_D("qos_port_conf_set(%u, %ld): failed rc = %d", sid, iport, rc);
                errors++; /* Probably stack error */
            }
        }


        (void)snprintf(buf, sizeof(buf), "/qos_port_policer_edit_multi.htm?port=%lu", iport2uport(iport));
        redirect(p, errors ? STACK_ERR_URL : buf);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        port_status_t    port_status;
        port_isid_info_t info;
        port_cap_t       cap = (port_isid_info_get(sid, &info) == VTSS_RC_OK ? info.cap : 0);
        cyg_httpd_start_chunked("html");

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
        }
        if (iport >= VTSS_PORT_NO_END) {
            iport = VTSS_PORT_NO_START;
        }

        if ((qos_port_conf_get(sid, iport, &conf) == VTSS_OK) &&
            (port_mgmt_status_get_all(sid, iport, &port_status) == VTSS_OK)) {

            /* Format:
             * <port_no>#<fc_mode>#<policers>
             *
             * port_no  :== 1..max
             * fc_mode  :== 0..2                      // 0: No ports has fc (don't show fc column), 1: This port has no fc, 2: This port has fc
             * policers :== <policer 1>/<policer 2>/<policer 3>/...<policer n>
             *   policer x         :== <enabled>|<fps>|<rate>|<dp_bypass_level>|<unicast>|<multicast>|<broadcast>|<flooded>|<learning>|<flow_control>
             *     enabled         :== 0..1           // 0: no, 1: yes
             *     fps             :== 0..1           // 0: unit for rate is kbits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *     rate            :== 0..0xffffffff  // actual bit or frame rate
             *     dp_bypass_level :== 0..3           // drop precedence bypass level
             *     unicast         :== 0..1           // unicast frames are policed
             *     multicast,      :== 0..1           // multicast frames are policed
             *     brooadcast      :== 0..1           // broadcast frames are policed
             *     flooded         :== 0..1           // flooded frames are policed
             *     learning        :== 0..1           // learning frames are policed
             *     flow_control    :== 0..1           // flow control is enabled
             */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%lu#%u#", iport2uport(iport), port_feature_support(cap, port_status.cap, PORT_CAP_FLOW_CTRL));
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (policer = 0; policer < QOS_PORT_POLICER_CNT; policer++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u|%lu|%u|%u|%u|%u|%u|%u|%u",
                              (policer != 0) ? "/" : "",
                              conf.port_policer[policer].enabled,
                              conf.port_policer_ext[policer].frame_rate,
                              conf.port_policer[policer].policer.rate,
                              conf.port_policer_ext[policer].dp_bypass_level,
                              conf.port_policer_ext[policer].unicast,
                              conf.port_policer_ext[policer].multicast,
                              conf.port_policer_ext[policer].broadcast,
                              conf.port_policer_ext[policer].flooded,
                              conf.port_policer_ext[policer].learning,
                              conf.port_policer_ext[policer].flow_control);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#else
static cyg_int32 handler_config_qos_port_policers(CYG_HTTPD_STATE *p)
{
    int             ct;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            vtss_rc        rc;
            vtss_bitrate_t rate;
            int            val;
            if ((rc = qos_port_conf_get(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_get(%u, %d): failed rc = %d", sid, pit.uport, rc);
                continue;
            }
            conf.port_policer[0].enabled = cyg_httpd_form_variable_check_fmt(p, "enabled_%u", pit.iport);

            if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "rate_%u", pit.iport)) {
                conf.port_policer[0].policer.rate = rate;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "fps_%u", pit.iport)) {
                conf.port_policer_ext[0].frame_rate = (val != 0);
            }

            conf.port_policer_ext[0].flow_control = cyg_httpd_form_variable_check_fmt(p, "flow_control_%u", pit.iport);

            if ((rc = qos_port_conf_set(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, pit.uport, rc);
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/qos_port_policers.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        port_status_t    port_status;
        port_isid_info_t info;
        port_cap_t       cap = (port_isid_info_get(sid, &info) == VTSS_RC_OK ? info.cap : 0);
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if ((qos_port_conf_get(sid, pit.iport, &conf) < 0) ||
                (port_mgmt_status_get_all(sid, pit.iport, &port_status) < 0)) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>/<enabled>/<fps>/<rate>/<flow_control>
             *   port_no      :== 1..max
             *   enabled      :== 0..1           // 0: no, 1: yes
             *   fps          :== 0..1           // 0: unit for rate is kilobits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *   rate         :== 0..0xffffffff  // actual bit or frame rate
             *   fc_mode      :== 0..2           // 0: No ports has fc (don't show fc column), 1: This port has no fc, 2: This port has fc
             *   flow_control :== 0..1           // flow control is enabled
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%u/%u/%u/%u/%u",
                          pit.first ? "" : ",",
                          pit.uport,
                          conf.port_policer[0].enabled,
                          conf.port_policer_ext[0].frame_rate,
                          conf.port_policer[0].policer.rate,
                          port_feature_support(cap, port_status.cap, PORT_CAP_FLOW_CTRL),
                          conf.port_policer_ext[0].flow_control);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL) && defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM) */
#endif /* defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS) */

#if defined(VTSS_SW_OPTION_BUILD_CE)
static cyg_int32 handler_config_qos_queue_policers(CYG_HTTPD_STATE *p)
{
    int             ct, queue;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            vtss_rc        rc;
            vtss_bitrate_t rate;
            if ((rc = qos_port_conf_get(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_get(%u, %d): failed rc = %d", sid, pit.uport, rc);
                continue;
            }
            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE); queue++) {
                conf.queue_policer[queue].enabled = cyg_httpd_form_variable_check_fmt(p, "enabled_%d_%u", queue, pit.iport);

                if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "rate_%d_%u", queue, pit.iport)) {
                    conf.queue_policer[queue].policer.rate = rate;
                }
            }

            if ((rc = qos_port_conf_set(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, pit.uport, rc);
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/qos_queue_policers.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>#<queues>
             *   port_no :== 1..max
             *   queues  :== <queue 0>/<queue 1>/<queue 2>/...<queue n>
             *     queue x :== <enabled>|<rate>
             *       enabled :== 0..1           // 0: no, 1: yes
             *       rate    :== 0..0xffffffff  // bit rate
             */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u",
                          pit.first ? "" : ",",
                          pit.uport);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE); queue++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u",
                              (queue != 0) ? "/" : "#",
                              conf.queue_policer[queue].enabled,
                              conf.queue_policer[queue].policer.rate);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */

#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
static cyg_int32 handler_config_qos_port_schedulers(CYG_HTTPD_STATE *p)
{
    int             ct, queue;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/qos_port_schedulers.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>#<scheduler_mode>#<queue_weights>
             *   port_no          :== 1..max
             *   scheduler_mode   :== 0..1           // 0: Strict Priority, 1: Weighted
             *   queue_weights    :== <queue_1_weight>/<queue_2_weight>/...<queue_n_weight>  // n is 6.
             *     queue_x_weight :== 1..100         // Just a number. If you set all 6 weights to 100, each queue will have a weigth of 100/6 = 16.7 ~ 17%
             */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u#%u",
                          pit.first ? "" : ",",
                          pit.uport,
                          conf.dwrr_enable);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE - 2); queue++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u",
                              (queue != 0) ? "/" : "#",
                              conf.queue_pct[queue]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_qos_port_shapers(CYG_HTTPD_STATE *p)
{
    int             ct, queue;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/qos_port_shapers.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x          :== <port_no>#<queue_shapers>#<port_shaper>
             *   port_no       :== 1..max
             *   queue_shapers :== <shaper_1>/<shaper_2>/...<shaper_n>                 // n is 8.
             *     shaper_x    :== <enable|rate>
             *       enable    :== 0..1           // 0: Shaper is disabled, 1: Shaper is disabled
             *       rate      :== 0..0xffffffff  // Actual bit rate in kbps
             *   port_shaper   :== <enable|rate>
             *     enable      :== 0..1           // 0: Shaper is disabled, 1: Shaper is disabled
             *     rate        :== 0..0xffffffff  // Actual bit rate in kbps
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u",
                          pit.first ? "" : ",",
                          pit.uport);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE); queue++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u",
                              (queue != 0) ? "/" : "#",
                              conf.queue_shaper[queue].enable,
                              conf.queue_shaper[queue].rate);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%u|%u",
                          conf.shaper_status,
                          conf.shaper_rate);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_qos_port_scheduler_edit(CYG_HTTPD_STATE *p)
{
    int             ct, queue, uport;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t  iport = VTSS_PORT_NO_START;
    qos_port_conf_t conf;
    char            buf[64];

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
            if (iport >= VTSS_PORT_NO_END) {
                errors++;
            }
        } else {
            errors++;
        }

        if (errors || qos_port_conf_get(sid, iport, &conf) != VTSS_OK) {
            errors++;
        } else {
            vtss_rc        rc;
            int            val;
            vtss_bitrate_t rate = 0;

            if (cyg_httpd_form_varable_int(p, "dwrr_enable", &val)) {
                conf.dwrr_enable = val;
            }

            // Get Port Scheduler configuration if dwrr is enabled
            if (conf.dwrr_enable) {
                for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE - 2); queue++) {
                    if (cyg_httpd_form_variable_int_fmt(p, &val, "weight_%d", queue)) {
                        conf.queue_pct[queue] = val;
                    }
                }
            }

            // Get Queue Shaper configuration
            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE); queue++) {
                conf.queue_shaper[queue].enable = cyg_httpd_form_variable_check_fmt(p, "q_shaper_enable_%d", queue);

                // Only save rate and excess if shaper is enabled
                if (conf.queue_shaper[queue].enable) {
                    if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "q_shaper_rate_%d", queue)) {
                        conf.queue_shaper[queue].rate = rate;
                    }
                    conf.excess_enable[queue] = cyg_httpd_form_variable_check_fmt(p, "q_shaper_excess_%d", queue);
                }
            }

            // Get Port Shaper configuration
            conf.shaper_status = cyg_httpd_form_varable_find(p, "p_shaper_enable") ? TRUE : FALSE;

            // Only save rate if shaper is enabled
            if (conf.shaper_status && cyg_httpd_form_varable_long_int(p, "p_shaper_rate", &rate)) {
                conf.shaper_rate = rate;
            }

            if ((rc = qos_port_conf_set(sid, iport, &conf)) < 0) {
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, iport, rc);
                errors++; /* Probably stack error */
            }
        }

        (void)snprintf(buf, sizeof(buf), "/qos_port_scheduler_edit.htm?port=%u", iport2uport(iport));
        redirect(p, errors ? STACK_ERR_URL : buf);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
        }
        if (iport >= VTSS_PORT_NO_END) {
            iport = VTSS_PORT_NO_START;
        }

        if (qos_port_conf_get(sid, iport, &conf) == VTSS_OK) {

            /* Format:
             * <port_no>#<scheduler_mode>#<queue_weights>#<queue_shapers>#<port_shaper>
             *
             * port_no            :== 1..max
             * scheduler_mode     :== 0..1           // 0: Strict Priority, 1: Weighted
             * queue_weights      :== <queue_1_weight>/<queue_2_weight>/...<queue_n_weight>  // n is 6.
             *   queue_x_weight   :== 1..100         // Just a number. If you set all 6 weights to 100, each queue will have a weigth of 100/6 = 16.7 ~ 17%
             * queue_shapers      :== <queue_shaper_1>/<queue_shaper_2>/...<queue_shaper_n>  // n is 8.
             *   queue_shaper_x   :== <enable|rate|excess>
             *     enable         :== 0..1           // 0: Shaper is disabled, 1: Shaper is disabled
             *     rate           :== 0..0xffffffff  // Actual bit rate in kbps
             *     excess         :== 0..1           // 0: Excess bandwidth disabled, 1: Excess bandwidth enabled
             * port_shaper        :== <enable|rate>
             *   enable           :== 0..1           // 0: Shaper is disabled, 1: Shaper is disabled
             *   rate             :== 0..0xffffffff  // Actual bit rate in kbps
             */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%u",
                          iport2uport(iport),
                          conf.dwrr_enable);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE - 2); queue++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u",
                              (queue != 0) ? "/" : "#",
                              conf.queue_pct[queue]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            for (queue = 0; queue < (VTSS_QUEUE_ARRAY_SIZE); queue++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u|%u",
                              (queue != 0) ? "/" : "#",
                              conf.queue_shaper[queue].enable,
                              conf.queue_shaper[queue].rate,
                              conf.excess_enable[queue]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%u|%u",
                          conf.shaper_status,
                          conf.shaper_rate);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
static cyg_int32 handler_config_qos_port_tag_remarking(CYG_HTTPD_STATE *p)
{
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/qos_port_tag_remarking.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        port_iter_t     pit;
        qos_port_conf_t conf;
        int             ct, mode;
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>#<mode>
             *   port_no       :== 1..max
             *   mode          :== 0..2              // 0: Classified, 1: Default, 2: Mapped
             */

            switch (conf.tag_remark_mode) {
            case VTSS_TAG_REMARK_MODE_DEFAULT:
                mode = 1;
                break;
            case VTSS_TAG_REMARK_MODE_MAPPED:
                mode = 2;
                break;
            default:
                mode = 0;
                break;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u#%u",
                          pit.first ? "" : ",",
                          pit.uport,
                          mode);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_qos_port_tag_remarking_edit(CYG_HTTPD_STATE *p)
{
    int             ct, i, uport;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t  iport = VTSS_PORT_NO_START;
    qos_port_conf_t conf;
    char            buf[64];

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
            if (iport >= VTSS_PORT_NO_END) {
                errors++;
            }
        } else {
            errors++;
        }

        if (errors || qos_port_conf_get(sid, iport, &conf) != VTSS_OK) {
            errors++;
        } else {
            int            val;
            vtss_rc        rc;

            if (cyg_httpd_form_varable_int(p, "tr_mode", &val)) {
                switch (val) {
                case 1:
                    conf.tag_remark_mode = VTSS_TAG_REMARK_MODE_DEFAULT;
                    break;
                case 2:
                    conf.tag_remark_mode = VTSS_TAG_REMARK_MODE_MAPPED;
                    break;
                default:
                    conf.tag_remark_mode = VTSS_TAG_REMARK_MODE_CLASSIFIED;
                    break;
                }
            }

            // Get default PCP and DEI if mode is set to "Default"
            if (conf.tag_remark_mode == VTSS_TAG_REMARK_MODE_DEFAULT) {
                if (cyg_httpd_form_varable_int(p, "default_pcp", &val)) {
                    conf.tag_default_pcp = val;
                }
                if (cyg_httpd_form_varable_int(p, "default_dei", &val)) {
                    conf.tag_default_dei = val;
                }
            }

            // Get DP level and Map if mode is set to "Mapped"
            if (conf.tag_remark_mode == VTSS_TAG_REMARK_MODE_MAPPED) {
                for (i = 0; i < (QOS_PORT_PRIO_CNT * 2); i++) {
                    if (cyg_httpd_form_variable_int_fmt(p, &val, "pcp_%d", i)) {
                        conf.tag_pcp_map[i / 2][i % 2] = val;
                    }
                    if (cyg_httpd_form_variable_int_fmt(p, &val, "dei_%d", i)) {
                        conf.tag_dei_map[i / 2][i % 2] = val;
                    }
                }
            }

            if ((rc = qos_port_conf_set(sid, iport, &conf)) < 0) {
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, iport, rc);
                errors++; /* Probably stack error */
            }
        }

        (void)snprintf(buf, sizeof(buf), "/qos_port_tag_remarking_edit.htm?port=%u", iport2uport(iport));
        redirect(p, errors ? STACK_ERR_URL : buf);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        if (cyg_httpd_form_varable_int(p, "port", &uport)) {
            iport = uport2iport(uport);
        }
        if (iport >= VTSS_PORT_NO_END) {
            iport = VTSS_PORT_NO_START;
        }

        if (qos_port_conf_get(sid, iport, &conf) == VTSS_OK) {

            /* Format:
             * <port_no>#<mode>#<default_params>#<map>
             *
             * port_no          :== 1..max
             * mode             :== 0..2       // 0: Classified, 1: Default, 2: Mapped
             * default_params   :== <pcp|dei>
             *   pcp            :== 0..7
             *   dei            :== 0..1
             * map              :== <entry_0>/<entry_1>/...<entry_n> // n is 15.
             *   entry_x        :== <pcp|dei>
             *     pcp          :== 0..7
             *     dei          :== 0..1
             *
             * The map is organized as follows:
             * Entry corresponds to QoS class, DP level
             *  0                   0          0
             *  1                   0          1
             *  2                   1          0
             *  3                   1          1
             *  4                   2          0
             *  5                   2          1
             *  6                   3          0
             *  7                   3          1
             *  8                   4          0
             *  9                   4          1
             * 10                   5          0
             * 11                   5          1
             * 12                   6          0
             * 13                   6          1
             * 14                   7          0
             * 15                   7          1
             */
            int mode;
            switch (conf.tag_remark_mode) {
            case VTSS_TAG_REMARK_MODE_DEFAULT:
                mode = 1;
                break;
            case VTSS_TAG_REMARK_MODE_MAPPED:
                mode = 2;
                break;
            default:
                mode = 0;
                break;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#%u#%u|%u",
                          iport2uport(iport),
                          mode,
                          conf.tag_default_pcp,
                          conf.tag_default_dei);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            for (i = 0; i < (QOS_PORT_PRIO_CNT * 2); i++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u|%u",
                              (i != 0) ? "/" : "#",
                              conf.tag_pcp_map[i / 2][i % 2],
                              conf.tag_dei_map[i / 2][i % 2]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#if defined(VTSS_SW_OPTION_JR_STORM_POLICERS)
static cyg_int32 handler_config_stormctrl_jr(CYG_HTTPD_STATE *p)
{
    int             ct;
    vtss_isid_t     sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t     pit;
    qos_port_conf_t conf;

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            vtss_rc        rc;
            vtss_bitrate_t rate;
            int            val;
            if ((rc = qos_port_conf_get(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_get(%u, %d): failed rc = %d", sid, pit.uport, rc);
                continue;
            }
            conf.port_policer[QOS_STORM_POLICER_UNICAST].enabled = cyg_httpd_form_variable_check_fmt(p, "uc_enabled_%u", pit.iport);

            if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "uc_rate_%u", pit.iport)) {
                conf.port_policer[QOS_STORM_POLICER_UNICAST].policer.rate = rate;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "uc_fps_%u", pit.iport)) {
                conf.port_policer_ext[QOS_STORM_POLICER_UNICAST].frame_rate = (val != 0);
            }

            conf.port_policer[QOS_STORM_POLICER_BROADCAST].enabled = cyg_httpd_form_variable_check_fmt(p, "bc_enabled_%u", pit.iport);

            if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "bc_rate_%u", pit.iport)) {
                conf.port_policer[QOS_STORM_POLICER_BROADCAST].policer.rate = rate;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "bc_fps_%u", pit.iport)) {
                conf.port_policer_ext[QOS_STORM_POLICER_BROADCAST].frame_rate = (val != 0);
            }

            conf.port_policer[QOS_STORM_POLICER_UNKNOWN].enabled = cyg_httpd_form_variable_check_fmt(p, "un_enabled_%u", pit.iport);

            if (cyg_httpd_form_variable_long_int_fmt(p, &rate, "un_rate_%u", pit.iport)) {
                conf.port_policer[QOS_STORM_POLICER_UNKNOWN].policer.rate = rate;
            }

            if (cyg_httpd_form_variable_int_fmt(p, &val, "un_fps_%u", pit.iport)) {
                conf.port_policer_ext[QOS_STORM_POLICER_UNKNOWN].frame_rate = (val != 0);
            }

            if ((rc = qos_port_conf_set(sid, pit.iport, &conf)) < 0) {
                errors++; /* Probably stack error */
                T_D("qos_port_conf_set(%u, %d): failed rc = %d", sid, pit.uport, rc);
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/stormctrl_jr.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_get(sid, pit.iport, &conf) < 0) {
                break;          /* Probably stack error - bail out */
            }

            /* Format:
             * <port 1>,<port 2>,<port 3>,...<port n>
             *
             * port x :== <port_no>/<unicast>/<broadcast>/<unknown>
             *   port_no   :== 1..max
             *   unicast   :== <enabled>|<fps>|<rate>
             *     enabled :== 0..1           // 0: no, 1: yes
             *     fps     :== 0..1           // 0: unit for rate is kilobits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *     rate    :== 0..0xffffffff  // actual bit or frame rate
             *   broadcast :== <enabled>|<fps>|<rate>
             *     enabled :== 0..1           // 0: no, 1: yes
             *     fps     :== 0..1           // 0: unit for rate is kilobits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *     rate    :== 0..0xffffffff  // actual bit or frame rate
             *   unknown   :== <enabled>|<fps>|<rate>
             *     enabled :== 0..1           // 0: no, 1: yes
             *     fps     :== 0..1           // 0: unit for rate is kilobits pr seconds (kbps), 1: unit for rate is frames pr second (fps)
             *     rate    :== 0..0xffffffff  // actual bit or frame rate
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%u|%u|%u/%u|%u|%u/%u|%u|%u",
                          pit.first ? "" : ",",
                          pit.uport,
                          conf.port_policer[QOS_STORM_POLICER_UNICAST].enabled,
                          conf.port_policer_ext[QOS_STORM_POLICER_UNICAST].frame_rate,
                          conf.port_policer[QOS_STORM_POLICER_UNICAST].policer.rate,
                          conf.port_policer[QOS_STORM_POLICER_BROADCAST].enabled,
                          conf.port_policer_ext[QOS_STORM_POLICER_BROADCAST].frame_rate,
                          conf.port_policer[QOS_STORM_POLICER_BROADCAST].policer.rate,
                          conf.port_policer[QOS_STORM_POLICER_UNKNOWN].enabled,
                          conf.port_policer_ext[QOS_STORM_POLICER_UNKNOWN].frame_rate,
                          conf.port_policer[QOS_STORM_POLICER_UNKNOWN].policer.rate);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_SW_OPTION_JR_STORM_POLICERS) */

#if defined(VTSS_FEATURE_QOS_WRED) || defined(VTSS_FEATURE_QOS_WRED_V2)
static cyg_int32 handler_config_qos_wred(CYG_HTTPD_STATE *p)
{
    int          ct, queue;
    qos_conf_t   conf, newconf;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (qos_conf_get(&conf) != VTSS_OK) {
            errors++;   /* Probably stack error */
        } else {
            newconf = conf;

            for (queue = 0; queue < QOS_PORT_WEIGHTED_QUEUE_CNT; queue++) {
                int  val;
#if defined(VTSS_FEATURE_QOS_WRED)
#define QOS_WRED_URL "/qos_wred.htm"
                newconf.wred[queue].enable = cyg_httpd_form_variable_check_fmt(p, "enable_%d", queue);
                if (cyg_httpd_form_variable_int_fmt(p, &val, "min_th_%d", queue)) {
                    newconf.wred[queue].min_th = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "mdp_1_%d", queue)) {
                    newconf.wred[queue].max_prob_1 = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "mdp_2_%d", queue)) {
                    newconf.wred[queue].max_prob_2 = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "mdp_3_%d", queue)) {
                    newconf.wred[queue].max_prob_3 = val;
                }
#else
#define QOS_WRED_URL "/qos_wred_v2.htm"
                newconf.wred[queue][1].enable = cyg_httpd_form_variable_check_fmt(p, "enable_%d", queue);
                if (cyg_httpd_form_variable_int_fmt(p, &val, "min_fl_%d", queue)) {
                    newconf.wred[queue][1].min_fl = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "max_%d", queue)) {
                    newconf.wred[queue][1].max = val;
                }
                if (cyg_httpd_form_variable_int_fmt(p, &val, "max_unit_%d", queue)) {
                    newconf.wred[queue][1].max_unit = val;
                }
#endif /* VTSS_FEATURE_QOS_WRED */
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                vtss_rc rc;
                if ((rc = qos_conf_set(&newconf)) != VTSS_OK) {
                    T_D("qos_conf_set: failed rc = %d", rc);
                    errors++; /* Probably stack error */
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : QOS_WRED_URL);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        /* should get these values from management APIs */
        if (qos_conf_get(&conf) == VTSS_OK) {
            for (queue = 0; queue < QOS_PORT_WEIGHTED_QUEUE_CNT; queue++) {
#if defined(VTSS_FEATURE_QOS_WRED)
                /*
                 * Format:
                 * <queue 0>,<queue 1>,<queue 2>,...<queue n> // n is 5.
                 *
                 * queue x :== <enable>#<min_th>#<mdp_1>#<mdp_2>#<mdp_3>
                 *   enable :== 0..1
                 *   min_th :== 0..100
                 *   mdp_1  :== 0..100
                 *   mdp_2  :== 0..100
                 *   mdp_3  :== 0..100
                 */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d#%u#%u#%u#%u",
                              (queue != 0) ? "," : "",
                              conf.wred[queue].enable,
                              conf.wred[queue].min_th,
                              conf.wred[queue].max_prob_1,
                              conf.wred[queue].max_prob_2,
                              conf.wred[queue].max_prob_3);
                cyg_httpd_write_chunked(p->outbuffer, ct);
#else
                /*
                 * Format:
                 * <queue 0>,<queue 1>,<queue 2>,...<queue n> // n is 5.
                 *
                 * queue x :== <enable>#<min_fl>#<max>#<max_unit
                 *   enable   :== 0..1
                 *   min_fl   :== 0..100
                 *   max      :== 1..100
                 *   max_unit :== 0..1   // 0: unit for max is 'drop probability', 1: unit for max is 'fill level'
                 */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d#%u#%u#%u",
                              (queue != 0) ? "," : "",
                              conf.wred[queue][1].enable,
                              conf.wred[queue][1].min_fl,
                              conf.wred[queue][1].max,
                              conf.wred[queue][1].max_unit);
                cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_FEATURE_QOS_WRED */
            }
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* defined(VTSS_FEATURE_QOS_WRED) || defined(VTSS_FEATURE_QOS_WRED_V2) */

#ifdef VTSS_FEATURE_VSTAX_V2
static cyg_int32 handler_config_qos_cmef(CYG_HTTPD_STATE *p)
{
    int          ct;
    qos_conf_t   conf;
    BOOL         enabled;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_QOS)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;

        if (qos_conf_get(&conf) != VTSS_OK) {
            errors++;   /* Probably stack error */
        } else {
            enabled = (cyg_httpd_form_varable_find(p, "cmef_mode") != NULL);
            if (enabled == conf.cmef_disable) { /* NOTE: Different polarity!!! */
                vtss_rc rc;
                conf.cmef_disable = !enabled;
                if ((rc = qos_conf_set(&conf)) != VTSS_OK) {
                    T_D("qos_conf_set: failed rc = %d", rc);
                    errors++; /* Probably stack error */
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/qos_cmef.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");

        if (qos_conf_get(&conf) == VTSS_OK) {
            /*
             * Format:
             * cmef_mode :== 0..1 // 0: Disabled, 1: Enabled
             */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d", !conf.cmef_disable);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t qos_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    int i;
    unsigned int cnt = 0;
    char dscp_names[64 * 10];
    char buff[QOS_WEB_BUF_LEN + sizeof(dscp_names)];

    for (i = 0; i < 64; i++) {
        cnt += snprintf(dscp_names + cnt, sizeof(dscp_names) - cnt, "%s'%s'", i == 0 ? "" : ",", qos_dscp2str(i));
    }

    (void) snprintf(buff, sizeof(buff),
                    "var configQosClassMax = %d;\n"
                    "var configQosBitRateMin = %d;\n"
                    "var configQosBitRateMax = %d;\n"
                    "var configQosBitRateDef = %d;\n"
                    "var configQosDplMax = %d;\n"
                    "var configQCLMax = %d;\n"
                    "var configQCEMax = %d;\n"
                    "var configQosDscpNames = [%s];\n",
                    QOS_CLASS_CNT,
                    QOS_BITRATE_MIN,
                    QOS_BITRATE_MAX,
                    QOS_BITRATE_DEF,
                    QOS_DPL_MAX,
                    QCL_MAX,
                    QCE_MAX,
                    dscp_names);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib_config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(qos_lib_config_js);

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t qos_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[QOS_WEB_BUF_LEN];
    (void) snprintf(buff, sizeof(buff), "%s" // <---- Add at least one string if the defines below ends up in nothing

#if defined(QOS_USE_FIXED_PCP_QOS_MAP)
                    ".has_qos_pcp_dei { display: none; }\r\n"
                    ".has_qos_tag_class { display: none; }\r\n"
#else
                    ".has_qos_fixed_map { display: none; }\r\n"
#endif

#if !defined(VTSS_SW_OPTION_BUILD_SMB) && !defined(VTSS_SW_OPTION_BUILD_CE)
                    ".has_qos_tag_class { display: none; }\r\n"
                    ".has_qos_dscp_class { display: none; }\r\n"
#endif

#if !defined(VTSS_FEATURE_QCL_DMAC_DIP)
                    ".has_qcl_smac_sip_and_dmac_dip { display: none; }\r\n"
#endif

#if defined(VTSS_ARCH_JAGUAR_1)
                    ".has_qcl_smac_48_bit { display: none; }\r\n"
#else
                    ".has_qcl_smac_24_bit { display: none; }\r\n"
#endif

                    , ""); // <---- This is the (empty) string that is always added
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(qos_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_qos_counter, "/stat/qos_counter",   handler_stat_qos_counter);

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_stormctrl, "/config/stormconfig", handler_config_stormctrl);
#endif

#ifdef VTSS_FEATURE_QCL_V2
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qcl_v2, "/config/qcl_v2", handler_config_qcl_v2);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_qcl_v2,   "/stat/qcl_v2",   handler_stat_qcl_v2);
#endif
#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_classification,      "/config/qos_port_classification",      handler_config_qos_port_classification);
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_classification_map,  "/config/qos_port_classification_map",  handler_config_qos_port_classification_map);
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_dscp_classification_map,  "/config/qos_dscp_classification_map",  handler_config_qos_dscp_classification_map);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_dscp_translation, "/config/qos_dscp_translation", handler_config_dscp_translation);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dscp_port_config, "/config/dscp_port_config", handler_config_dscp_port_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dscp_bsd_qos_ingr_cls, "/config/dscp_based_qos_ingr_cls", handler_config_dscp_based_qos_ingr_classi);
#endif /* (VTSS_FEATURE_QOS_DSCP_REMARK_V2) */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL) && defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_policers_multi,      "/config/qos_port_policers_multi",      handler_config_qos_port_policers_multi);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_policer_edit_multi,  "/config/qos_port_policer_edit_multi",  handler_config_qos_port_policer_edit_multi);
#else
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_policers,            "/config/qos_port_policers",            handler_config_qos_port_policers);
#endif /* defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL) && defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM) */
#endif /* defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS) */
#if defined(VTSS_SW_OPTION_BUILD_CE)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_queue_policers,           "/config/qos_queue_policers",           handler_config_qos_queue_policers);
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */
#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_schedulers,          "/config/qos_port_schedulers",          handler_config_qos_port_schedulers);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_shapers,             "/config/qos_port_shapers",             handler_config_qos_port_shapers);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_scheduler_edit,      "/config/qos_port_scheduler_edit",      handler_config_qos_port_scheduler_edit);
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */
#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_tag_remarking,       "/config/qos_port_tag_remarking",       handler_config_qos_port_tag_remarking);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_port_tag_remarking_edit,  "/config/qos_port_tag_remarking_edit",  handler_config_qos_port_tag_remarking_edit);
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */
#if defined(VTSS_SW_OPTION_JR_STORM_POLICERS)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_stormctrl_jr,                 "/config/stormctrl_jr",                 handler_config_stormctrl_jr);
#endif /* defined(VTSS_SW_OPTION_JR_STORM_POLICERS) */

#if defined(VTSS_FEATURE_QOS_WRED)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_wred, "/config/qos_wred", handler_config_qos_wred);
#elif defined(VTSS_FEATURE_QOS_WRED_V2)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_wred, "/config/qos_wred_v2", handler_config_qos_wred);
#endif /* defined(VTSS_FEATURE_QOS_WRED) */

#if defined(VTSS_FEATURE_VSTAX_V2)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_qos_cmef, "/config/qos_cmef", handler_config_qos_cmef);
#endif /* VTSS_FEATURE_VSTAX_V2 */
