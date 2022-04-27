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
#include "port_api.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "voice_vlan_api.h"
#include "voice_vlan.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "voice_vlan_cli.h"
#endif
#include "vlan_api.h"
#include "mac_api.h"
#if defined(VOICE_VLAN_CLASS_SUPPORTED)
#include "qos_api.h"
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
#include "psec_api.h"
#if defined(VTSS_SW_OPTION_LLDP)
#include "lldp_api.h"
#include "lldp_remote.h"
#endif /* VTSS_SW_OPTION_LLDP */
#if defined(VTSS_SW_OPTION_MVR)
#include "mvr_api.h"
#endif /* VTSS_SW_OPTION_MVR */

#ifdef VTSS_SW_OPTION_ICFG
#include "voice_vlan_icfg.h"
#endif

#define VOICE_VLAN_CONF_CHANGE_MODE             0x1
#define VOICE_VLAN_CONF_CHANGE_VID              0x2
#define VOICE_VLAN_CONF_CHANGE_AGE_TIME         0x4
#define VOICE_VLAN_CONF_CHANGE_TRAFFIC_CLASS    0x8

#define VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE       0x1
#define VOICE_VLAN_PORT_CONF_CHANGE_SECURITY        0x2
#define VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL        0x4

/* Set VOICE_VLAN reserved QCE */
#if defined(VTSS_FEATURE_QCL_V1)
#define VOICE_VLAN_RESERVED_QCL_ID  (QCL_MAX + 1)
#endif /* VTSS_FEATURE_QCL_V1 */
#define VOICE_VLAN_RESERVED_QCE_ID  1

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_VOICE_VLAN

/****************************************************************************/
/*  Private global variables                                                */
/****************************************************************************/

/* Private global structure */
static voice_vlan_global_t VOICE_VLAN_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t VOICE_VLAN_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "voice_vlan",
    .descr     = "VOICE_VLAN"
};

static vtss_trace_grp_t VOICE_VLAN_trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define VOICE_VLAN_CRIT_ENTER() critd_enter(&VOICE_VLAN_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define VOICE_VLAN_CRIT_EXIT()  critd_exit( &VOICE_VLAN_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define VOICE_VLAN_CRIT_ENTER() critd_enter(&VOICE_VLAN_global.crit)
#define VOICE_VLAN_CRIT_EXIT()  critd_exit( &VOICE_VLAN_global.crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
static vtss_rc VOICE_VLAN_mgmt_oui_conf_set(voice_vlan_oui_conf_t *conf);


/****************************************************************************/
// Voice VLAN LLDP telephony MAC entry functions
/****************************************************************************/

/* Get VOICE_VLAN LLDP telephony MAC entry
 * The entry key is MAC address.
 * Use null MAC address to get first entry. */
vtss_rc voice_vlan_lldp_telephony_mac_entry_get(voice_vlan_lldp_telephony_mac_entry_t *entry, BOOL next)
{
    u32 i, num, found = 0;
    u8  null_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    T_D("enter");

    if (entry == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        /* Get first entry */
        if (memcmp(null_mac, entry->mac, 6) == 0 && next) {
            *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
            found = 1;
            break;
        }

        /* Lookup this entry */
        if (!memcmp(VOICE_VLAN_global.lldp_telephony_mac.entry[i].mac, entry->mac, 6)) {
            if (next) { /* Get next entry */
                if (num == VOICE_VLAN_global.lldp_telephony_mac.entry_num) {
                    break;
                }
                i++;
                while (i < VOICE_VLAN_OUI_ENTRIES_CNT) {
                    if (VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
                        *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else { /* Get this entry */
                *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
                found = 1;
            }
            break;
        }
    }

    VOICE_VLAN_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

/* Get VOICE_VLAN LLDP telephony MAC entry
 * fill null oui_address will get first entry */
static vtss_rc VOICE_VLAN_lldp_telephony_mac_entry_get_by_port(vtss_isid_t isid, vtss_port_no_t iport, voice_vlan_lldp_telephony_mac_entry_t *entry, BOOL next)
{
    u32 i, num, found = 0;
    u8  null_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    T_D("enter");

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        if (VOICE_VLAN_global.lldp_telephony_mac.entry[i].isid != isid ||
            VOICE_VLAN_global.lldp_telephony_mac.entry[i].port_no != iport) {
            continue;
        }

        /* Get first entry */
        if (memcmp(null_mac, entry->mac, 6) == 0 && next) {
            *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
            found = 1;
            break;
        }

        /* Lookup this entry */
        if (!memcmp(VOICE_VLAN_global.lldp_telephony_mac.entry[i].mac, entry->mac, 6)) {
            if (next) { /* Get next entry */
                if (num == VOICE_VLAN_global.lldp_telephony_mac.entry_num) {
                    break;
                }
                i++;
                while (i < VOICE_VLAN_OUI_ENTRIES_CNT) {
                    if (VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid &&
                        VOICE_VLAN_global.lldp_telephony_mac.entry[i].isid == isid &&
                        VOICE_VLAN_global.lldp_telephony_mac.entry[i].port_no == iport) {
                        *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else { /* Get this entry */
                *entry = VOICE_VLAN_global.lldp_telephony_mac.entry[i];
                found = 1;
            }
            break;
        }
    }

    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

#if defined(VTSS_SW_OPTION_LLDP)
/* Add/Set VOICE_VLAN LLDP telephony MAC entry */
static vtss_rc VOICE_VLAN_lldp_telephony_mac_entry_add(voice_vlan_lldp_telephony_mac_entry_t *entry)
{
    vtss_rc rc = VTSS_RC_ERROR;
    u32     i, num, found = 0;
    u8      null_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    T_D("enter");
    if (memcmp(null_mac, entry->mac, 6) == 0) {
        return VTSS_RC_ERROR;
    }

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        if (!memcmp(VOICE_VLAN_global.lldp_telephony_mac.entry[i].mac, entry->mac, 6)) {
            found = 1;
            break;
        }
    }

    if (found) {
        VOICE_VLAN_global.lldp_telephony_mac.entry[i] = *entry;
        rc = VTSS_OK;
    } else {
        /* Lookup a empty entry for using */
        for (i = 0; i < VOICE_VLAN_OUI_ENTRIES_CNT; i++) {
            if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
                found = 1;
                break;
            }
        }

        if (found) { /* Fill this entry */
            VOICE_VLAN_global.lldp_telephony_mac.entry[i] = *entry;
            VOICE_VLAN_global.lldp_telephony_mac.entry_num++;
            rc = VTSS_OK;
        }
    }

    T_D("exit");

    return rc;
}

/* Delete VOICE_VLAN LLDP telephony MAC entry */
static vtss_rc VOICE_VLAN_lldp_telephony_mac_entry_del(voice_vlan_lldp_telephony_mac_entry_t *entry)
{
    u32     i, num;
    u8      null_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    T_D("enter");
    if (memcmp(null_mac, entry->mac, 6) == 0) {
        T_D("exit");
        return VTSS_RC_ERROR;
    }

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        if (!memcmp(VOICE_VLAN_global.lldp_telephony_mac.entry[i].mac, entry->mac, 6)) {
            if (VOICE_VLAN_global.lldp_telephony_mac.entry_num > 0) {
                VOICE_VLAN_global.lldp_telephony_mac.entry_num--;
            }
            memset(&VOICE_VLAN_global.lldp_telephony_mac.entry[i], 0x0, sizeof(VOICE_VLAN_global.lldp_telephony_mac.entry[i]));
            break;
        }
    }

    T_D("exit");
    return VTSS_OK;
}
#endif /* VTSS_SW_OPTION_LLDP */

/* Clear VOICE_VLAN LLDP telephony MAC entry by switch */
static void VOICE_VLAN_lldp_telephony_mac_entry_clear_by_switch(vtss_isid_t isid)
{
    u32 i, num;

    T_D("enter");

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        if (VOICE_VLAN_global.lldp_telephony_mac.entry[i].isid != isid) {
            continue;
        }
        if (VOICE_VLAN_global.lldp_telephony_mac.entry_num > 0) {
            VOICE_VLAN_global.lldp_telephony_mac.entry_num--;
        }
        memset(&VOICE_VLAN_global.lldp_telephony_mac.entry[i], 0x0, sizeof(VOICE_VLAN_global.lldp_telephony_mac.entry[i]));
    }

    T_D("exit");
}

/* Clear VOICE_VLAN LLDP telephony MAC entry by port */
static void VOICE_VLAN_lldp_telephony_mac_entry_clear_by_port(vtss_isid_t isid, vtss_port_no_t iport)
{
    u32 i, num;

    T_D("enter");

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.lldp_telephony_mac.entry_num;
         i++) {
        if (!VOICE_VLAN_global.lldp_telephony_mac.entry[i].valid) {
            continue;
        }
        num++;

        if (VOICE_VLAN_global.lldp_telephony_mac.entry[i].isid != isid ||
            VOICE_VLAN_global.lldp_telephony_mac.entry[i].port_no != iport) {
            continue;
        }
        if (VOICE_VLAN_global.lldp_telephony_mac.entry_num > 0) {
            VOICE_VLAN_global.lldp_telephony_mac.entry_num--;
        }
        memset(&VOICE_VLAN_global.lldp_telephony_mac.entry[i], 0x0, sizeof(VOICE_VLAN_global.lldp_telephony_mac.entry[i]));
    }

    T_D("exit");
}

/* Clear VOICE_VLAN LLDP telephony MAC entry */
static void VOICE_VLAN_lldp_telephony_mac_entry_clear(void)
{
    memset(&VOICE_VLAN_global.lldp_telephony_mac, 0x0, sizeof(VOICE_VLAN_global.lldp_telephony_mac));
}

static BOOL VOICE_VLAN_is_lldp_telephony_addr(vtss_isid_t isid, vtss_port_no_t iport, u8 addr[6])
{
    voice_vlan_lldp_telephony_mac_entry_t entry;

    memset(&entry, 0x0, sizeof(entry));
    while (VOICE_VLAN_lldp_telephony_mac_entry_get_by_port(isid, iport, &entry, TRUE) == VTSS_OK) {
        if (!memcmp(entry.mac, addr, 6)) {
            return TRUE;
        }
    };

    return FALSE;
}

static BOOL VOICE_VLAN_exist_telephony_lldp_device(vtss_isid_t isid, vtss_port_no_t iport)
{
    voice_vlan_lldp_telephony_mac_entry_t entry;

    memset(&entry, 0x0, sizeof(entry));
    while (VOICE_VLAN_lldp_telephony_mac_entry_get_by_port(isid, iport, &entry, TRUE) == VTSS_OK) {
        return TRUE;
    };
    return FALSE;
}


/****************************************************************************/
// VOICE_VLAN LLDP OUI entry functions
/****************************************************************************/

/* Get VOICE_VLAN OUI entry
 * The entry key is OUI address.
 * Use null OUI address to get first entry. */
vtss_rc voice_vlan_oui_entry_get(voice_vlan_oui_entry_t *entry, BOOL next)
{
    u32 i, num, found = 0;
    u8  null_oui_addr[3] = {0x0, 0x0, 0x0};

    T_D("enter");

    if (entry == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.oui_conf.entry_num;
         i++) {
        if (!VOICE_VLAN_global.oui_conf.entry[i].valid) {
            continue;
        }
        num++;

        /* Get first entry */
        if (memcmp(null_oui_addr, entry->oui_addr, 3) == 0 && next) {
            *entry = VOICE_VLAN_global.oui_conf.entry[i];
            found = 1;
            break;
        }

        /* Lookup this entry */
        if (!memcmp(VOICE_VLAN_global.oui_conf.entry[i].oui_addr, entry->oui_addr, 3)) {
            if (next) { /* Get next entry */
                if (num == VOICE_VLAN_global.oui_conf.entry_num) {
                    break;
                }
                i++;
                while (i < VOICE_VLAN_OUI_ENTRIES_CNT) {
                    if (VOICE_VLAN_global.oui_conf.entry[i].valid) {
                        *entry = VOICE_VLAN_global.oui_conf.entry[i];
                        found = 1;
                        break;
                    }
                    i++;
                }
                break;
            } else { /* Get this entry */
                *entry = VOICE_VLAN_global.oui_conf.entry[i];
                found = 1;
            }
            break;
        }
    }

    VOICE_VLAN_CRIT_EXIT();
    T_D("exit");

    if (found) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

/* Add/Set VOICE_VLAN OUI entry */
vtss_rc voice_vlan_oui_entry_add(voice_vlan_oui_entry_t *entry)
{
    vtss_rc                 rc = VTSS_RC_ERROR;
    u32                     i, num, found = 0;
    voice_vlan_oui_conf_t   conf;
    u8                      null_oui_addr[3] = {0x0, 0x0, 0x0};

    T_D("enter");
    if (entry == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    if (memcmp(null_oui_addr, entry->oui_addr, 3) == 0) {
        T_D("exit");
        return VOICE_VLAN_ERROR_PARM_NULL_OUI_ADDR;
    }

    VOICE_VLAN_CRIT_ENTER();

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.oui_conf.entry_num;
         i++) {
        if (!VOICE_VLAN_global.oui_conf.entry[i].valid) {
            continue;
        }
        num++;

        if (!memcmp(VOICE_VLAN_global.oui_conf.entry[i].oui_addr, entry->oui_addr, 3)) {
            found = 1;
            break;
        }
    }

    if (found && i < VOICE_VLAN_OUI_ENTRIES_CNT) {
        conf = VOICE_VLAN_global.oui_conf;
        conf.entry[i] = *entry;
    } else {
        /* Lookup a empty entry for using */
        for (i = 0; i < VOICE_VLAN_OUI_ENTRIES_CNT; i++) {
            if (!VOICE_VLAN_global.oui_conf.entry[i].valid) {
                found = 1;
                break;
            }
        }

        if (found && i < VOICE_VLAN_OUI_ENTRIES_CNT) { /* Fill this entry */
            conf = VOICE_VLAN_global.oui_conf;
            conf.entry_num++;
            conf.entry[i] = *entry;
        } else {
            T_D("exit");
            VOICE_VLAN_CRIT_EXIT();
            return VOICE_VLAN_ERROR_REACH_MAX_OUI_ENTRY;
        }
    }

    VOICE_VLAN_CRIT_EXIT();

    rc = VOICE_VLAN_mgmt_oui_conf_set(&conf);
    T_D("exit");
    return rc;
}

/* Delete VOICE_VLAN OUI entry */
vtss_rc voice_vlan_oui_entry_del(voice_vlan_oui_entry_t *entry)
{
    vtss_rc                 rc = VOICE_VLAN_ERROR_ENTRY_NOT_EXIST;
    u32                     i, num, found = 0;
    voice_vlan_oui_conf_t   conf;

    T_D("enter");

    if (entry == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();

    /* Lookup this entry */
    for (i = 0, num = 0;
         i < VOICE_VLAN_OUI_ENTRIES_CNT && num < VOICE_VLAN_global.oui_conf.entry_num;
         i++) {
        if (!VOICE_VLAN_global.oui_conf.entry[i].valid) {
            continue;
        }
        num++;

        if (!memcmp(VOICE_VLAN_global.oui_conf.entry[i].oui_addr, entry->oui_addr, 3)) {
            found = 1;
            break;
        }
    }

    VOICE_VLAN_CRIT_EXIT();

    if (found && i < VOICE_VLAN_OUI_ENTRIES_CNT) { /* Delete this entry */
        VOICE_VLAN_CRIT_ENTER();
        conf = VOICE_VLAN_global.oui_conf;
        VOICE_VLAN_CRIT_EXIT();

        if (conf.entry_num > 0) {
            conf.entry_num--;
        }
        memset(&conf.entry[i], 0x0, sizeof(conf.entry[i]));
        rc = VOICE_VLAN_mgmt_oui_conf_set(&conf);
    }

    T_D("exit");
    return rc;
}

/* Clear VOICE_VLAN OUI entry */
vtss_rc voice_vlan_oui_entry_clear(void)
{
    voice_vlan_oui_conf_t   conf;
    vtss_rc                 rc;

    T_D("enter");
    memset(&conf, 0x0, sizeof(conf));
    rc = VOICE_VLAN_mgmt_oui_conf_set(&conf);
    T_D("exit");
    return rc;
}

static BOOL VOICE_VLAN_is_oui_addr(u8 addr[6])
{
    voice_vlan_oui_entry_t  entry;

    memset(&entry, 0x0, sizeof(entry));
    while (voice_vlan_oui_entry_get(&entry, TRUE) == VTSS_OK) {
        if (entry.oui_addr[0] == addr[0] && entry.oui_addr[1] == addr[1] && entry.oui_addr[2] == addr[2]) {
            return TRUE;
        }
    };

    return FALSE;
}


/****************************************************************************/
// Voice VLAN QCE functions
/****************************************************************************/
#if defined(VTSS_FEATURE_QCL_V2)
// Avoid "Custodual pointer 'msg' has not been freed or returned, since
// the msg is freed by the message module.
/*lint -e{429} */
static void VOICE_VLAN_local_qce_set(vtss_isid_t isid, voice_vlan_msg_req_t *msg_reg)
{
    size_t                  msg_len = sizeof(voice_vlan_msg_req_t);
    voice_vlan_msg_req_t    *msg;

    msg = VTSS_MALLOC(msg_len);
    if (msg) {
        *msg = *msg_reg;
        msg->msg_id = VOICE_VLAN_MSG_ID_QCE_SET_REQ;
        if (!msg_reg->req.qce_conf_set.change_traffic_class) {
            msg->req.qce_conf_set.traffic_class = VOICE_VLAN_global.conf.traffic_class;
        }
        if (!msg_reg->req.qce_conf_set.change_vid) {
            msg->req.qce_conf_set.vid = VOICE_VLAN_global.conf.vid;
        }
        msg_tx(VTSS_MODULE_ID_VOICE_VLAN, isid, msg, msg_len);
    }
}
#endif /* VTSS_FEATURE_QCL_V2 */

#if defined(VTSS_FEATURE_QCL_V1)
/* Clear none reserved QCL */
static void VOICE_VLAN_reserved_qcl_clear(void)
{
    qos_qce_entry_conf_t qce_conf;

    while (qos_mgmt_qce_entry_get(VOICE_VLAN_RESERVED_QCL_ID, QCE_ID_NONE, &qce_conf, TRUE) == VTSS_OK) {
        (void) qos_mgmt_qce_entry_del(VOICE_VLAN_RESERVED_QCL_ID, qce_conf.id);
    }
}

/* Create a reserved QCE for Voice VLAN */
static vtss_rc VOICE_VLAN_reserved_qce_add(void)
{
    vtss_rc                 rc = VTSS_OK;
    vtss_qcl_id_t           qcl_id = VOICE_VLAN_RESERVED_QCL_ID;
    vtss_qce_id_t           qce_id = QCE_ID_NONE;
    qos_qce_entry_conf_t    new_qce_conf, old_qce_conf;

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    memset(&new_qce_conf, 0x0, sizeof(new_qce_conf));
    new_qce_conf.id = VOICE_VLAN_RESERVED_QCE_ID;
    new_qce_conf.type = VTSS_QCE_TYPE_VLAN;
    new_qce_conf.frame.vlan.prio = VOICE_VLAN_global.conf.traffic_class;
    new_qce_conf.frame.vlan.vid = VOICE_VLAN_global.conf.vid;

    /* Check this QCE is existing */
    if (qos_mgmt_qce_entry_get(qcl_id, VOICE_VLAN_RESERVED_QCE_ID, &old_qce_conf, FALSE) == VTSS_OK &&
        !memcmp(&new_qce_conf, &old_qce_conf, sizeof(new_qce_conf))) {
        return VTSS_OK;
    }

    rc = qos_mgmt_qce_entry_add(qcl_id, qce_id, &new_qce_conf);

    return rc;
}
#endif /* VTSS_FEATURE_QCL_V1 */

/* Create a reserved QCE for Voice VLAN */
static void VOICE_VLAN_reserved_qce_del(void)
{
#if defined(VTSS_FEATURE_QCL_V1)
    /* Ignore return value here, this entry maybe not exist */
    (void) qos_mgmt_qce_entry_del(VOICE_VLAN_RESERVED_QCL_ID, VOICE_VLAN_RESERVED_QCE_ID);
#else
    voice_vlan_msg_req_t    msg_reg;
    vtss_isid_t             isid;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists(isid)) {
            continue;
        }
        memset(&msg_reg, 0x0, sizeof(msg_reg));
        msg_reg.req.qce_conf_set.del_qce = TRUE;
        VOICE_VLAN_local_qce_set(isid, &msg_reg);
    }
#endif /* VTSS_FEATURE_QCL_V1 */
}


/****************************************************************************/
// Voice VLAN functions
/****************************************************************************/

static vtss_rc VOICE_VLAN_join(vtss_isid_t isid, vtss_port_no_t iport, vtss_vid_t vid)
{
    vtss_rc                 rc = VTSS_OK;
    vlan_mgmt_entry_t       vlan_member;
#if defined(VTSS_FEATURE_QCL_V1)
    qos_port_conf_t         qos_port_conf;
#else
    voice_vlan_msg_req_t    msg_reg;
#endif /* VTSS_FEATURE_QCL_V1 */

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    /* Add to VOICE_VLAN */
    if (vlan_mgmt_vlan_get(isid, vid, &vlan_member, FALSE, VLAN_USER_VOICE_VLAN) != VTSS_OK) {
        memset(&vlan_member, 0x0, sizeof(vlan_member));
        vlan_member.vid = vid;
    }
    if (vlan_member.ports[iport] == 0) {
        vlan_member.ports[iport] = 1;
        rc = vlan_mgmt_vlan_add(isid, &vlan_member, VLAN_USER_VOICE_VLAN);
    }

    /* Set port default QCL to VOICE_VLAN reserved QCL */
#if defined(VTSS_FEATURE_QCL_V1)
    if ((rc = qos_port_conf_get(isid, iport, &qos_port_conf)) == VTSS_OK) {
        if (qos_port_conf.qcl_no != VOICE_VLAN_RESERVED_QCL_ID) {
            VOICE_VLAN_global.qcl_no[isid][iport] = qos_port_conf.qcl_no;
        } else if (VOICE_VLAN_global.qcl_no[isid][iport] == 0) {
            VOICE_VLAN_global.qcl_no[isid][iport] = QOS_PORT_DEFAULT_QCL;
        }
        qos_port_conf.qcl_no = VOICE_VLAN_RESERVED_QCL_ID;
        rc = qos_port_conf_set(isid, iport, &qos_port_conf);
    }
#else
    memset(&msg_reg, 0x0, sizeof(msg_reg));
    msg_reg.req.qce_conf_set.is_port_add = TRUE;
    msg_reg.req.qce_conf_set.iport = iport;
    VOICE_VLAN_local_qce_set(isid, &msg_reg);
#endif /* VTSS_FEATURE_QCL_V1 */

    return rc;
}

static vtss_rc VOICE_VLAN_disjoin(vtss_isid_t isid, vtss_port_no_t iport, vtss_vid_t vid)
{
    vtss_rc                 rc = VTSS_OK;
    vlan_mgmt_entry_t       vlan_member;
#if defined(VTSS_FEATURE_QCL_V1)
    qos_port_conf_t         qos_port_conf;
#else
    voice_vlan_msg_req_t    msg_reg;
#endif /* VTSS_FEATURE_QCL_V1 */

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    /* Remove from VOICE_VLAN */
    if (vlan_mgmt_vlan_get(isid, vid, &vlan_member, FALSE, VLAN_USER_VOICE_VLAN) != VTSS_OK) {
        memset(&vlan_member, 0x0, sizeof(vlan_member));
        vlan_member.vid = vid;
    }
    if (vlan_member.ports[iport] == 1) {
        vlan_member.ports[iport] = 0;
        rc = vlan_mgmt_vlan_add(isid, &vlan_member, VLAN_USER_VOICE_VLAN);
    }

    /* Restore port default QCL */
#if defined(VTSS_FEATURE_QCL_V1)
    if ((rc = qos_port_conf_get(isid, iport, &qos_port_conf)) == VTSS_OK) {
        if (qos_port_conf.qcl_no == VOICE_VLAN_RESERVED_QCL_ID) {
            qos_port_conf.qcl_no = VOICE_VLAN_global.qcl_no[isid][iport];
            rc = qos_port_conf_set(isid, iport, &qos_port_conf);
        }
    }
#else
    memset(&msg_reg, 0x0, sizeof(msg_reg));
    msg_reg.req.qce_conf_set.is_port_del = TRUE;
    msg_reg.req.qce_conf_set.iport = iport;
    VOICE_VLAN_local_qce_set(isid, &msg_reg);
#endif /* VTSS_FEATURE_QCL_V1 */

    return rc;
}


/****************************************************************************/
/* Callback functions                                                       */
/****************************************************************************/

/* Process port change status */
static void VOICE_VLAN_porcess_port_change_cb(vtss_isid_t isid, vtss_port_no_t iport, BOOL link)
{
    vtss_rc rc;

    /* do nothing when port link-on, since everything should be no problem at this moment */
    if (!link) {
        VOICE_VLAN_CRIT_ENTER();
        /* Disjoin from VOice VLAN when port link-down */
        if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED &&
            VOICE_VLAN_global.port_conf[isid].port_mode[iport] == VOICE_VLAN_PORT_MODE_AUTO) {
            if ((rc = VOICE_VLAN_disjoin(isid, iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
                T_W("VOICE_VLAN_disjoin(%u, %d, %d): failed rc = %d", isid, iport, VOICE_VLAN_global.conf.vid, rc);
            }
        }

        /* Clear LLDP telephony MAC entries on this port */
        VOICE_VLAN_lldp_telephony_mac_entry_clear_by_port(isid, iport);

        /* Clear OUI counter on this port */
        memset(&VOICE_VLAN_global.oui_cnt[isid][iport], 0x0, sizeof(VOICE_VLAN_global.oui_cnt[isid][iport]));
        VOICE_VLAN_CRIT_EXIT();
    }
}

/* Port chage link status callback function */
static void VOICE_VLAN_port_change_cb(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    if (msg_switch_is_master() && !info->stack) {
        T_D("port_no: [%d,%u] link %s", isid, port_no, info->link ? "up" : "down");
        if (!msg_switch_exists(isid)) {
            return;
        }
        VOICE_VLAN_porcess_port_change_cb(isid, port_no, info->link);
    }
}

#if defined(VTSS_SW_OPTION_LLDP)
/* Process LLDP entry change */
static void VOICE_VLAN_porcess_lldp_cb(vtss_isid_t isid, vtss_port_no_t iport, lldp_remote_entry_t *lldp_entry)
{
    vtss_rc                                 rc;
    int                                     del_flag = 0, port_entry_cnt = 0;
    voice_vlan_lldp_telephony_mac_entry_t   lldp_telephony_mac_entry;

    T_D("enter, isid: %d, iport: %d, LLDP entry: %s, telphone capability: %s, capability:%s",
        isid,
        iport,
        lldp_entry->in_use ? "Del" : "Add/Change",
        (lldp_entry->system_capabilities[1] & 0x20) ? "Yes" : "No",
        (lldp_entry->system_capabilities[3] & 0x20) ? "Enabled" : "Disabled");

    if (!msg_switch_is_master()) {
        T_W("not master");
        return;
    }

    VOICE_VLAN_CRIT_ENTER();

    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED &&
        VOICE_VLAN_global.port_conf[isid].port_mode[iport] == VOICE_VLAN_PORT_MODE_AUTO &&
        VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI) {
        lldp_telephony_mac_entry.isid = isid;
        lldp_telephony_mac_entry.port_no = iport;
        memcpy(lldp_telephony_mac_entry.mac, lldp_entry->smac, 6);
        if (lldp_entry->in_use &&
            (lldp_entry->system_capabilities[1] & 0x20) &&
            (lldp_entry->system_capabilities[3] & 0x20)) { //bit5 is telephone capability (has telephone capability and is enabled)

            /* Add LLDP telephony MAC entry */
            T_D("Add LLDP telephony MAC entry");
            lldp_telephony_mac_entry.valid = 1;
            if ((rc = VOICE_VLAN_lldp_telephony_mac_entry_add(&lldp_telephony_mac_entry)) != VTSS_OK) {
                T_W("VOICE_VLAN_lldp_telephony_mac_entry_add(): failed rc = %d", rc);
            }

            /* Join to VOICE_VLAN when LLDP device provide telephone capability */
            if ((rc = VOICE_VLAN_join(isid, iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
                T_W("VOICE_VLAN_join(%u, %d, %d): failed rc = %d", isid, iport, VOICE_VLAN_global.conf.vid, rc);
            }
        } else {
            /* Delete LLDP telephony MAC entry */
            T_D("Delete LLDP telephony MAC entry");
            if (VOICE_VLAN_lldp_telephony_mac_entry_del(&lldp_telephony_mac_entry) != VTSS_OK) {
                T_D("Calling VOICE_VLAN_lldp_telephony_mac_entry_del() failed\n");
            }
            del_flag = 1;
        }
    }

    if (del_flag) {
        memset(&lldp_telephony_mac_entry, 0x0, sizeof(lldp_telephony_mac_entry));
        while (VOICE_VLAN_lldp_telephony_mac_entry_get_by_port(isid, iport, &lldp_telephony_mac_entry, TRUE) == VTSS_OK) {
            port_entry_cnt++;
        }

        if (port_entry_cnt == 0) {
            /* Remove from VOICE_VLAN when
               1. In LLDP mode, last telephony LLDP device leaving
               2. In both mode, disjoin from Voice VALN if there's no any mac entry exist in Voice VALN */
            if (VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP ||
                (VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH && VOICE_VLAN_global.oui_cnt[isid][iport] == 0)) {
                if ((rc = VOICE_VLAN_disjoin(isid, iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
                    T_W("VOICE_VLAN_disjoin(%u, %d, %d): failed rc = %d", isid, iport, VOICE_VLAN_global.conf.vid, rc);
                }
            }
        }
    }

    VOICE_VLAN_CRIT_EXIT();
    T_D("exit");
}
#endif /* VTSS_SW_OPTION_LLDP */

/* LLDP entry change callback function */
// Avoid "Custodual pointer 'msg' has not been freed or returned, since
// the msg is freed by the message module.
/*lint -e{429} */
#if defined(VTSS_SW_OPTION_LLDP)
static void VOICE_VLAN_lldp_entry_cb(vtss_port_no_t iport, lldp_remote_entry_t *entry)
{
    size_t msg_len = sizeof(voice_vlan_msg_req_t);
    voice_vlan_msg_req_t *msg = VTSS_MALLOC(msg_len);
    if (msg) {
        msg->msg_id = VOICE_VLAN_MSG_ID_LLDP_CB_IND;
        msg->req.lldp_cb_ind.port_no = iport;
        msg->req.lldp_cb_ind.entry = *entry;
        msg_tx(VTSS_MODULE_ID_VOICE_VLAN, 0, msg, msg_len);
    }
}
#endif /* VTSS_SW_OPTION_LLDP */

/* Port security module loop through callback function
 * In all cases where this function is called back, we delete all existing. */
static psec_add_method_t VOICE_VLAN_psec_on_mac_loop_through_cb(void                       *user_ctx,
                                                                vtss_isid_t                isid,
                                                                vtss_port_no_t             port,
                                                                vtss_vid_mac_t             *vid_mac,
                                                                u32                        mac_cnt_before_callback,
                                                                BOOL                       *keep,
                                                                psec_loop_through_action_t *action)
{
    // Remove this entry.
    *keep = FALSE;
    // Return value doesn't matter when we remove it.
    return PSEC_ADD_METHOD_FORWARD;
}

/* Port security module MAC add callback function */
static psec_add_method_t VOICE_VLAN_psec_mac_add_cb(vtss_isid_t isid, vtss_port_no_t iport, vtss_vid_mac_t *vid_mac, u32 mac_cnt_before_callback, psec_add_action_t *action)
{
    vtss_rc             rc;
    BOOL                is_oui_addr = 0;
    psec_add_method_t   psec_rc = PSEC_ADD_METHOD_FORWARD;

    is_oui_addr = VOICE_VLAN_is_oui_addr(vid_mac->mac.addr);

    VOICE_VLAN_CRIT_ENTER();

    if (is_oui_addr) {
        if (VOICE_VLAN_global.port_conf[isid].port_mode[iport] != VOICE_VLAN_PORT_MODE_AUTO ||
            VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP) {
            VOICE_VLAN_CRIT_EXIT();
            return PSEC_ADD_METHOD_FORWARD;
        }
        /* Join to VOICE_VLAN when incoming packet's source MAC address is OUI address */
        if ((rc = VOICE_VLAN_join(isid, iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
            T_W("VOICE_VLAN_join(%u, %d, %d): failed rc = %d", isid, iport, VOICE_VLAN_global.conf.vid, rc);
        }

        if (vid_mac->vid == VOICE_VLAN_global.conf.vid) {
            VOICE_VLAN_global.oui_cnt[isid][iport]++;
        }
    } else if (VOICE_VLAN_global.port_conf[isid].security[iport] == VOICE_VLAN_MGMT_ENABLED && vid_mac->vid == VOICE_VLAN_global.conf.vid) {
        if (!VOICE_VLAN_is_lldp_telephony_addr(isid, iport, vid_mac->mac.addr)) {
            /* Block non-OUI address in VOICE_VLAN */
            psec_rc = PSEC_ADD_METHOD_BLOCK;
        }
    }

    VOICE_VLAN_CRIT_EXIT();

    return psec_rc;
}

/* Port security module MAC delete callback function */
static void VOICE_VLAN_psec_mac_del_cb(vtss_isid_t isid, vtss_port_no_t iport, vtss_vid_mac_t *vid_mac, psec_del_reason_t reason, psec_add_method_t add_method)
{
    vtss_rc rc;
    BOOL    is_oui_addr = 0;

    switch (reason) {
    case PSEC_DEL_REASON_SWITCH_DOWN:
        /* Do nothing, case INIT_CMD_SWITCH_DEL will handle it */
        break;
    case PSEC_DEL_REASON_PORT_LINK_DOWN:
        /* Do nothing, VOICE_VLAN_porcess_port_change_cb() will handle it */
        break;
    case PSEC_DEL_REASON_STATION_MOVED:
    case PSEC_DEL_REASON_AGED_OUT:
    case PSEC_DEL_REASON_USER_DELETED:
        is_oui_addr = VOICE_VLAN_is_oui_addr(vid_mac->mac.addr);
        VOICE_VLAN_CRIT_ENTER();
        if (vid_mac->vid == VOICE_VLAN_global.conf.vid) {
            if (is_oui_addr) {
                if (VOICE_VLAN_global.oui_cnt[isid][iport] > 0) {
                    VOICE_VLAN_global.oui_cnt[isid][iport]--;
                }

                /* Disjoin from VOICE_VLAN when last OUI address aged out */
                if (VOICE_VLAN_global.oui_cnt[isid][iport] == 0 &&
                    (VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_OUI ||
                     (VOICE_VLAN_global.port_conf[isid].discovery_protocol[iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH && !VOICE_VLAN_exist_telephony_lldp_device(isid, iport)))) {
                    if ((rc = VOICE_VLAN_disjoin(isid, iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
                        T_W("VOICE_VLAN_disjoin(%u, %d, %d): failed rc = %d", isid, iport, VOICE_VLAN_global.conf.vid, rc);
                    }
                }
            }
        }
        VOICE_VLAN_CRIT_EXIT();
        break;
    case PSEC_DEL_REASON_PORT_SHUT_DOWN:
        /* Process the condition the same as link-down */
        VOICE_VLAN_porcess_port_change_cb(isid, iport, 0);
        break;
    default:
        return;
    }
}


/****************************************************************************/
// Voice VLAN configuration changed apply functions
/****************************************************************************/

#if defined(VTSS_SW_OPTION_LLDP)
static void VOICE_VLAN_update_lldp_info(vtss_isid_t isid, vtss_port_no_t port_no, voice_vlan_port_conf_t *new_conf)
{
    port_iter_t         pit;
    lldp_remote_entry_t *lldp_table = NULL, *lldp_entry = NULL;
    int                 i, count = 0;
    BOOL                need_process = FALSE;

    /* Only process it when port mode is enabled and discovery protocol is LLDP/Both */
    if (port_no == VTSS_PORT_NO_NONE) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO &&
                new_conf->discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI) {
                need_process = TRUE;
                break;
            }
        }
        if (!need_process) {
            return;
        }
    }

    lldp_mgmt_get_lock();

    count = lldp_remote_get_max_entries();
    lldp_table = lldp_mgmt_get_entries(isid); // Get the LLDP entries for the switch in question.
    for (i = 0, lldp_entry = lldp_table; (lldp_entry && i < count); i++) {
        if (lldp_entry->in_use &&
            (lldp_entry->system_capabilities[1] & 0x20) &&
            (lldp_entry->system_capabilities[3] & 0x20) &&
            new_conf->port_mode[lldp_entry->receive_port] == VOICE_VLAN_PORT_MODE_AUTO &&
            new_conf->discovery_protocol[lldp_entry->receive_port] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI) {
            if (port_no == VTSS_PORT_NO_NONE) {
                VOICE_VLAN_porcess_lldp_cb(isid, lldp_entry->receive_port, lldp_entry);
            } else if (port_no == lldp_entry->receive_port) {
                VOICE_VLAN_porcess_lldp_cb(isid, lldp_entry->receive_port, lldp_entry);
                break;
            }
        }
        lldp_entry++;
    }

    lldp_mgmt_get_unlock();
}
#endif /* VTSS_SW_OPTION_LLDP */

/* Apply VOICE_VLAN configuration */
static vtss_rc VOICE_VLAN_conf_apply(BOOL init, vtss_isid_t isid)
{
    vtss_rc                 rc = VTSS_OK;
    port_iter_t             pit;
    vlan_mgmt_entry_t       vlan_member;
    vlan_port_conf_t        vlan_port_conf;
    int                     vlan_member_cnt = 0;
#if defined(VTSS_FEATURE_QCL_V1)
    qos_port_conf_t         qos_port_conf;
#else
    voice_vlan_msg_req_t    msg_reg;
#endif /* VTSS_FEATURE_QCL_V1 */

    T_D("enter, isid: %d", isid);

    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit, isid: %d", isid);
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }
    if (!msg_switch_exists(isid)) {
        T_D("isid: %d not exist", isid);
        T_D("exit, isid: %d", isid);
        return VOICE_VLAN_ERROR_ISID;
    }

    if (init) {
        /* Clear none reserved QCEs */
#if defined(VTSS_FEATURE_QCL_V1)
        VOICE_VLAN_reserved_qcl_clear();
#endif /* VTSS_FEATURE_QCL_V1 */

        /* Set age time */
        VOICE_VLAN_CRIT_ENTER();
        rc = psec_mgmt_time_cfg_set(PSEC_USER_VOICE_VLAN, VOICE_VLAN_global.conf.age_time, VOICE_VLAN_MGMT_DEFAULT_HOLD_TIME);
        VOICE_VLAN_CRIT_EXIT();
    }

    VOICE_VLAN_CRIT_ENTER();
    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED) {
#if defined(VTSS_FEATURE_QCL_V1)
        /* Set VOICE_VLAN reserved QCE */
        if ((rc = VOICE_VLAN_reserved_qce_add()) != VTSS_OK) {
            T_W("VOICE_VLAN_reserved_qce_add(): failed rc = %d", rc);
        }
#endif /* VTSS_FEATURE_QCL_V1 */

        if (vlan_mgmt_vlan_get(isid, VOICE_VLAN_global.conf.vid, &vlan_member, FALSE, VLAN_USER_VOICE_VLAN) != VTSS_OK) {
            /* Create Voice VLAN */
            memset(&vlan_member, 0x0, sizeof(vlan_member));
            vlan_member.vid = VOICE_VLAN_global.conf.vid;
            if ((rc = vlan_mgmt_vlan_add(isid, &vlan_member, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                T_W("vlan_mgmt_vlan_add(%u): failed rc = %d", isid, rc);
            }
        }
    }
    VOICE_VLAN_CRIT_EXIT();

    if (rc != VTSS_OK || VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_DISABLED) {
        T_D("exit, isid: %d", isid);
        return rc;
    }

    VOICE_VLAN_CRIT_ENTER();
    memset(&vlan_member, 0x0, sizeof(vlan_member));
    vlan_member.vid = VOICE_VLAN_global.conf.vid;

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        if (port_isid_port_no_is_stack(isid, pit.iport)) {
            VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] = VOICE_VLAN_MGMT_DISABLED;
            continue;
        }
        if (VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_DISABLED) {
            /* Un-override any parameter */
            memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf));
            if ((rc = vlan_mgmt_port_conf_set(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                T_E("vlan_mgmt_port_conf_set(%u, %d): failed rc = %s", isid, pit.iport, error_txt(rc));
            }
        } else {
            /* Disable ingress filter when port mode isn't disabled */
            if (vlan_mgmt_port_conf_get(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN) == VTSS_OK) {
                vlan_port_conf.ingress_filter = FALSE;
                vlan_port_conf.port_type = VLAN_PORT_TYPE_C;
                vlan_port_conf.flags = VLAN_PORT_FLAGS_INGR_FILT | VLAN_PORT_FLAGS_AWARE;
                if ((rc = vlan_mgmt_port_conf_set(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                    T_E("vlan_mgmt_port_conf_set(%u, %d): failed rc = %s", isid, pit.iport, error_txt(rc));
                }
            }
        }

        /* Set port default QCL */
#if defined(VTSS_FEATURE_QCL_V1)
        if ((rc = qos_port_conf_get(isid, pit.iport, &qos_port_conf)) == VTSS_OK) {
            if (VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_FORCED) {
                if (qos_port_conf.qcl_no != VOICE_VLAN_RESERVED_QCL_ID) {
                    VOICE_VLAN_global.qcl_no[isid][pit.iport] = qos_port_conf.qcl_no;
                } else if (VOICE_VLAN_global.qcl_no[isid][pit.iport] == 0) {
                    VOICE_VLAN_global.qcl_no[isid][pit.iport] = QOS_PORT_DEFAULT_QCL;
                }
                qos_port_conf.qcl_no = VOICE_VLAN_RESERVED_QCL_ID;
                rc = qos_port_conf_set(isid, pit.iport, &qos_port_conf);
            }
        }
#endif /* VTSS_FEATURE_QCL_V1 */

        /* Set VOICE_VLAN member */
        if (VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_FORCED) {
            vlan_member.ports[pit.iport] = 1;
            vlan_member_cnt++;
        }

        /* Enable software-based learning when:
         * 1. Voice VLAN and port security mode are enabled
         * 2. Support auto OUI discovery protocol */
        if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED) {
            if ((VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] != VOICE_VLAN_PORT_MODE_DISABLED && VOICE_VLAN_global.port_conf[isid].security[pit.iport] == VOICE_VLAN_MGMT_ENABLED) ||
                (VOICE_VLAN_global.port_conf[isid].port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && VOICE_VLAN_global.port_conf[isid].discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP)) {
                if (VOICE_VLAN_global.sw_learn[isid][pit.iport] == 0) {
                    if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, isid, pit.iport, TRUE, FALSE, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                        VOICE_VLAN_global.sw_learn[isid][pit.iport] = 1;
                    }
                }
            }
        } else if (VOICE_VLAN_global.sw_learn[isid][pit.iport]) {
            /* Disable software-based learning */
            if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, isid, pit.iport, FALSE, FALSE, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                VOICE_VLAN_global.sw_learn[isid][pit.iport] = 0;
            }
        }
    }

    if (vlan_member_cnt) {
        rc = vlan_mgmt_vlan_add(isid, &vlan_member, VLAN_USER_VOICE_VLAN);
    }

#if defined(VTSS_FEATURE_QCL_V2)
    memset(&msg_reg, 0x0, sizeof(msg_reg));
    msg_reg.req.qce_conf_set.is_port_list = TRUE;
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        msg_reg.req.qce_conf_set.port_list[pit.iport] = vlan_member.ports[pit.iport];
    }
    VOICE_VLAN_local_qce_set(isid, &msg_reg);
#endif /* VTSS_FEATURE_QCL_V2 */

    VOICE_VLAN_CRIT_EXIT();
    T_D("exit, isid: %d", isid);
    return rc;
}

/* Apply VOICE_VLAN configuration pre-changed */
static vtss_rc VOICE_VLAN_conf_pre_changed_apply(int change_field, voice_vlan_conf_t *old_conf)
{
    vtss_rc             rc = VTSS_OK;
    switch_iter_t       sit;
    port_iter_t         pit;
    vlan_port_conf_t    vlan_port_conf;
#if defined(VTSS_FEATURE_QCL_V1)
    qos_port_conf_t     qos_port_conf;
#endif /* VTSS_FEATURE_QCL_V1 */
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if defined(VTSS_SW_OPTION_LLDP)
    lldp_struc_0_t      lldp_conf;
#endif /* VTSS_SW_OPTION_LLDP */
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */

    /* 1. Change mode from enabled to disabled
     * 2. Change VOICE_VLAN ID when mode is enabled */
    if (old_conf->mode == VOICE_VLAN_MGMT_ENABLED &&
        ((change_field & VOICE_VLAN_CONF_CHANGE_MODE) || (change_field & VOICE_VLAN_CONF_CHANGE_VID))) {

        if (old_conf->mode == VOICE_VLAN_MGMT_ENABLED &&
            (change_field & VOICE_VLAN_CONF_CHANGE_MODE)) {
            /* Delete reserved QCE */
            VOICE_VLAN_reserved_qce_del();
        }

        /* Delete original VOICE_VLAN */
        if ((rc = vlan_mgmt_vlan_del(VTSS_ISID_GLOBAL, old_conf->vid, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
            return rc;
        }

        /* Clear LLDP telephony MAC entries */
        if (change_field & VOICE_VLAN_CONF_CHANGE_MODE) {
            VOICE_VLAN_lldp_telephony_mac_entry_clear();
        }

        /* Clear "oui_cnt" */
        memset(VOICE_VLAN_global.oui_cnt, 0x0, sizeof(VOICE_VLAN_global.oui_cnt));

        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (change_field & VOICE_VLAN_CONF_CHANGE_MODE) {
                    /* Un-override any parameter when change mode from enabled to disabled */
                    memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf));
                    if ((rc = vlan_mgmt_port_conf_set(sit.isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                        T_E("vlan_mgmt_port_conf_set(%u, %d): failed rc = %s", sit.isid, pit.iport, error_txt(rc));
                    }

                    /* Restore port default QCL */
#if defined(VTSS_FEATURE_QCL_V1)
                    if ((rc = qos_port_conf_get(sit.isid, pit.iport, &qos_port_conf)) == VTSS_OK) {
                        if (qos_port_conf.qcl_no == VOICE_VLAN_RESERVED_QCL_ID) {
                            qos_port_conf.qcl_no = VOICE_VLAN_global.qcl_no[sit.isid][pit.iport];
                            rc = qos_port_conf_set(sit.isid, pit.iport, &qos_port_conf);
                        }
                    }
#endif /* VTSS_FEATURE_QCL_V1 */

                    /* Disable software-based learning */
                    if (VOICE_VLAN_global.sw_learn[sit.isid][pit.iport]) {
                        if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, sit.isid, pit.iport, FALSE, FALSE, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                            VOICE_VLAN_global.sw_learn[sit.isid][pit.iport] = 0;
                        }
                    }
                }
            }
        }
    }

#if VOICE_VLAN_CHECK_CONFLICT_CONF
    /* Check LLDP port mode when change mode from disabled to enabled */
    if (old_conf->mode == VOICE_VLAN_MGMT_DISABLED &&
        (change_field & VOICE_VLAN_CONF_CHANGE_MODE)) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
#if defined(VTSS_SW_OPTION_LLDP)
            lldp_mgmt_get_config(&lldp_conf, sit.isid);
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (VOICE_VLAN_global.port_conf[sit.isid].port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO &&
                    VOICE_VLAN_global.port_conf[sit.isid].discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI &&
                    (lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_DISABLED || lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_ENABLED_TX_ONLY)) {
                    return VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP;
                }
            }
#endif /* VTSS_SW_OPTION_LLDP */
        }
    }
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */

    return rc;
}

/* Apply VOICE_VLAN configuration post-changed */
static vtss_rc VOICE_VLAN_conf_post_changed_apply(int change_field, voice_vlan_conf_t *new_conf)
{
    vtss_rc     rc = VTSS_OK;
    vtss_isid_t isid;

    /* Change mode from disabled to enabled */
    if (new_conf->mode == VOICE_VLAN_MGMT_ENABLED &&
        ((change_field & VOICE_VLAN_CONF_CHANGE_MODE) || (change_field & VOICE_VLAN_CONF_CHANGE_VID))) {
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (!msg_switch_exists(isid)) {
                continue;
            }
            rc = VOICE_VLAN_conf_apply(0, isid);
        }
    }

    if (change_field & VOICE_VLAN_CONF_CHANGE_AGE_TIME) {
        rc = psec_mgmt_time_cfg_set(PSEC_USER_VOICE_VLAN, new_conf->age_time, PSEC_ZOMBIE_HOLD_TIME_SECS);
    }

    if (new_conf->mode == VOICE_VLAN_MGMT_ENABLED &&
#if defined(VTSS_FEATURE_QCL_V1)
        (change_field & VOICE_VLAN_CONF_CHANGE_TRAFFIC_CLASS)) {
#else
        ((change_field & VOICE_VLAN_CONF_CHANGE_TRAFFIC_CLASS) || (change_field & VOICE_VLAN_CONF_CHANGE_VID))) {
#endif /* VTSS_FEATURE_QCL_V1 */
#if defined(VTSS_FEATURE_QCL_V1)
        rc = VOICE_VLAN_reserved_qce_add();
#else
        voice_vlan_msg_req_t msg_reg;

        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (!msg_switch_exists(isid)) {
                continue;
            }
            memset(&msg_reg, 0x0, sizeof(msg_reg));
            if (change_field & VOICE_VLAN_CONF_CHANGE_TRAFFIC_CLASS) {
                msg_reg.req.qce_conf_set.change_traffic_class = TRUE;
                msg_reg.req.qce_conf_set.traffic_class = new_conf->traffic_class;
            }
            if (change_field & VOICE_VLAN_CONF_CHANGE_VID) {
                msg_reg.req.qce_conf_set.change_vid = TRUE;
                msg_reg.req.qce_conf_set.vid = new_conf->vid;
            }
            VOICE_VLAN_CRIT_ENTER();
            VOICE_VLAN_local_qce_set(isid, &msg_reg);
            VOICE_VLAN_CRIT_EXIT();
        }
#endif /* VTSS_FEATURE_QCL_V1 */
    }

#if defined(VTSS_SW_OPTION_LLDP)
    /* Only process it when global mode changed from disabled to enabled */
    if ((change_field & VOICE_VLAN_CONF_CHANGE_MODE) && new_conf->mode == VOICE_VLAN_MGMT_ENABLED) {
        voice_vlan_port_conf_t port_conf;

        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (!msg_switch_exists(isid)) {
                continue;
            }
            VOICE_VLAN_CRIT_ENTER();
            port_conf = VOICE_VLAN_global.port_conf[isid];
            VOICE_VLAN_CRIT_EXIT();
            VOICE_VLAN_update_lldp_info(isid, VTSS_PORT_NO_NONE, &port_conf);
        }
    }
#endif /* VTSS_SW_OPTION_LLDP */

    return rc;
}

/* Apply VOICE_VLAN port configuration post-changed */
static vtss_rc VOICE_VLAN_port_conf_post_changed_apply(u32 *change_field, vtss_isid_t isid, voice_vlan_port_conf_t *new_conf)
{
    vtss_rc                 rc = VTSS_OK;
    port_iter_t             pit;
    vlan_mgmt_entry_t       vlan_member;
    vlan_port_conf_t        vlan_port_conf;
#if defined(VTSS_FEATURE_QCL_V1)
    qos_port_conf_t         qos_port_conf;
#else
    voice_vlan_msg_req_t    msg_reg;
#endif /* VTSS_FEATURE_QCL_V1 */
    BOOL                    reopen1, reopen2, reopen3;
#if defined(VTSS_SW_OPTION_LLDP)
    BOOL                    global_mode;
#endif /* VTSS_SW_OPTION_LLDP */

    T_D("enter");

    if (!msg_switch_exists(isid)) {
        T_D("exit");
        return rc;
    }

    VOICE_VLAN_CRIT_ENTER();

    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED) {
        if (vlan_mgmt_vlan_get(isid, VOICE_VLAN_global.conf.vid, &vlan_member, FALSE, VLAN_USER_VOICE_VLAN) != VTSS_OK) {
            memset(&vlan_member, 0x0, sizeof(vlan_member));
            vlan_member.vid = VOICE_VLAN_global.conf.vid;
        }
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (change_field[pit.iport] == 0) {
                continue;
            }
            reopen1 = reopen2 = reopen3 = FALSE;

            /* Change VOICE_VLAN members */
            if ((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) ||
                (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_SECURITY) ||
                (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL))) {
                reopen1 = (new_conf->security[pit.iport] == VOICE_VLAN_MGMT_ENABLED &&
                           (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) && new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO);

                if ((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) ||
                    (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL))) {
                    if (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_FORCED) {
                        /* Join to VOICE_VLAN when port mode is forced */
                        vlan_member.ports[pit.iport] = 1;
                    } else {
                        /* Disjoin to VOICE_VLAN when port mode is disabled or auto */
                        vlan_member.ports[pit.iport] = 0;
                    }
                }

#if defined(VTSS_FEATURE_QCL_V1)
                /* Change port default QCL */
                if ((rc = qos_port_conf_get(isid, pit.iport, &qos_port_conf)) == VTSS_OK) {
                    if (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_FORCED) {
                        /* Set port default QCL to VOICE_VLAN when mode is forced */
                        if (qos_port_conf.qcl_no != VOICE_VLAN_RESERVED_QCL_ID) {
                            VOICE_VLAN_global.qcl_no[isid][pit.iport] = qos_port_conf.qcl_no;
                        } else if (VOICE_VLAN_global.qcl_no[isid][pit.iport] == 0) {
                            VOICE_VLAN_global.qcl_no[isid][pit.iport] = QOS_PORT_DEFAULT_QCL;
                        }
                        qos_port_conf.qcl_no = VOICE_VLAN_RESERVED_QCL_ID;
                    } else {
                        /* Restore port default QCL when port mode is disabled or auto */
                        if (qos_port_conf.qcl_no == VOICE_VLAN_RESERVED_QCL_ID) {
                            qos_port_conf.qcl_no = VOICE_VLAN_global.qcl_no[isid][pit.iport];
                        }
                    }
                    rc = qos_port_conf_set(isid, pit.iport, &qos_port_conf);
                }
#endif /* VTSS_FEATURE_QCL_V1 */

                /* Clear LLDP telephony MAC entries on this port when:
                 * 1. Change port mode
                 * 2. Change port discovery protocol to OUI */
                reopen2 = ((!(change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE)) && new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL) && (new_conf->discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_OUI));
                if (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE || reopen2) {
                    VOICE_VLAN_lldp_telephony_mac_entry_clear_by_port(isid, pit.iport);
                }

                /* Clear "oui_-cnt" on this port when:
                 * 1. Change port mode
                 * 2. Change port discovery protocol to LLDP
                 * 3. Change port security mode */
                reopen3 = ((!(change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE)) && new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL) && (new_conf->discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP)) ||
                          ((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_SECURITY) && new_conf->security[pit.iport] == VOICE_VLAN_MGMT_ENABLED);
                if (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE || reopen3) {
                    VOICE_VLAN_global.oui_cnt[isid][pit.iport] = 0;
                }
            }

            /* Change MAC learning mode */
            /* Enable software-based learning when:
             * 1. Voice VLAN and port security mode are enabled
             * 2. Support auto OUI discovery protocol */
            if ((new_conf->port_mode[pit.iport] != VOICE_VLAN_PORT_MODE_DISABLED && new_conf->security[pit.iport] == VOICE_VLAN_MGMT_ENABLED) ||
                (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO && new_conf->discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP)) {
                if (VOICE_VLAN_global.sw_learn[isid][pit.iport] == 0 || reopen1 || reopen2 || reopen3) {
                    if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, isid, pit.iport, TRUE, reopen1 || reopen2 || reopen3, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                        VOICE_VLAN_global.sw_learn[isid][pit.iport] = 1;
                    }
                }
            } else if (VOICE_VLAN_global.sw_learn[isid][pit.iport]) {
                /* Disable software-based learning */
                if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, isid, pit.iport, FALSE, FALSE, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                    VOICE_VLAN_global.sw_learn[isid][pit.iport] = 0;
                }
            }

            /* Change ingress filter */
            if (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) {
                if (new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_DISABLED) {
                    /* Un-override any parameter */
                    memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf));
                    if ((rc = vlan_mgmt_port_conf_set(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                        T_E("vlan_mgmt_port_conf_set(%u, %d): failed rc = %s", isid, pit.iport, error_txt(rc));
                    }
                } else {
                    /* Disable ingress filter when port mode isn't disabled */
                    if (vlan_mgmt_port_conf_get(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN) == VTSS_OK) {
                        vlan_port_conf.ingress_filter = FALSE;
                        vlan_port_conf.port_type = VLAN_PORT_TYPE_C;
                        vlan_port_conf.flags = VLAN_PORT_FLAGS_INGR_FILT | VLAN_PORT_FLAGS_AWARE;
                        if ((rc = vlan_mgmt_port_conf_set(isid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN)) != VTSS_OK) {
                            T_E("vlan_mgmt_port_conf_set(%u, %d): failed rc = %s", isid, pit.iport, error_txt(rc));
                        }
                    }
                }
            }
        }

        rc = vlan_mgmt_vlan_add(isid, &vlan_member, VLAN_USER_VOICE_VLAN);

#if defined(VTSS_FEATURE_QCL_V2)
        memset(&msg_reg, 0x0, sizeof(msg_reg));
        msg_reg.req.qce_conf_set.is_port_list = TRUE;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            msg_reg.req.qce_conf_set.port_list[pit.iport] = vlan_member.ports[pit.iport];
        }
        VOICE_VLAN_local_qce_set(isid, &msg_reg);
#endif /* VTSS_FEATURE_QCL_V2 */
    }

    VOICE_VLAN_CRIT_EXIT();

#if defined(VTSS_SW_OPTION_LLDP)
    VOICE_VLAN_CRIT_ENTER();
    global_mode = VOICE_VLAN_global.conf.mode;
    VOICE_VLAN_CRIT_EXIT();

    if (global_mode == VOICE_VLAN_MGMT_ENABLED) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) && new_conf->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO) ||
                ((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL) && new_conf->port_mode[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI)) {
                VOICE_VLAN_update_lldp_info(isid, pit.iport, new_conf);
            }
        }
    }
#endif /* VTSS_SW_OPTION_LLDP */

    T_D("exit");
    return rc;
}


/****************************************************************************/
/*  Default and configuration chagned functions                             */
/****************************************************************************/

/* Determine if VOICE_VLAN configuration has changed */
static int VOICE_VLAN_conf_changed(voice_vlan_conf_t *old, voice_vlan_conf_t *new)
{
    int change_field = 0;

    if (old->mode != new->mode) {
        change_field |= VOICE_VLAN_CONF_CHANGE_MODE;
    }
    if (old->vid != new->vid) {
        change_field |= VOICE_VLAN_CONF_CHANGE_VID;
    }
    if (old->age_time != new->age_time) {
        change_field |= VOICE_VLAN_CONF_CHANGE_AGE_TIME;
    }
    if (old->traffic_class != new->traffic_class) {
        change_field |= VOICE_VLAN_CONF_CHANGE_TRAFFIC_CLASS;
    }

    return change_field;
}

/* Set VOICE_VLAN defaults */
static void VOICE_VLAN_default_set(voice_vlan_conf_t *conf)
{
    conf->mode = VOICE_VLAN_MGMT_DEFAULT_MODE;
    conf->vid = VOICE_VLAN_MGMT_DEFAULT_VID;
    conf->age_time = VOICE_VLAN_MGMT_DEFAULT_AGE_TIME;
    conf->traffic_class = uprio2iprio(VOICE_VLAN_MGMT_DEFAULT_TRAFFIC_CLASS);
}

/* Determine if VOICE_VLAN port configuration has changed */
static int VOICE_VLAN_port_conf_changed(vtss_isid_t isid, voice_vlan_port_conf_t *old, voice_vlan_port_conf_t *new, u32 *change_field)
{
    port_iter_t pit;

    if (change_field != NULL) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (old->port_mode[pit.iport] != new->port_mode[pit.iport]) {
                change_field[pit.iport] |= VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE;
            }
            if (old->security[pit.iport] != new->security[pit.iport]) {
                change_field[pit.iport] |= VOICE_VLAN_PORT_CONF_CHANGE_SECURITY;
            }
            if (old->discovery_protocol[pit.iport] != new->discovery_protocol[pit.iport]) {
                change_field[pit.iport] |= VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL;
            }
        }

    }

    return (memcmp(new, old, sizeof(*new)));
}

/* Set VOICE_VLAN port defaults */
static void VOICE_VLAN_port_default_set(vtss_isid_t isid, voice_vlan_port_conf_t *conf)
{
    vtss_port_no_t  port_no;

    if (isid == VTSS_ISID_GLOBAL) {
        return;
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        conf->port_mode[port_no] = VOICE_VLAN_MGMT_DEFAULT_PORT_MODE;
        conf->security[port_no] = VOICE_VLAN_MGMT_DEFAULT_SECURITY;
        conf->discovery_protocol[port_no] = VOICE_VLAN_MGMT_DEFAULT_DISCOVERY_PROTOCOL;
    }
}

/* Determine if VOICE_VLAN OUI configuration has changed */
static int VOICE_VLAN_oui_conf_changed(voice_vlan_oui_conf_t *old, voice_vlan_oui_conf_t *new)
{
    return (memcmp(new, old, sizeof(*new)));
}

/* Set VOICE_VLAN OUI defaults */
static void VOICE_VLAN_oui_default_set(voice_vlan_oui_conf_t *conf)
{
    memset(conf, 0x0, sizeof(*conf));

    // Default set of phone OUIs is no longer set here; it has been moved to
    // icfg/icfg-default-config.txt. Otherwise it is not possible to delete
    // OUIs across a reboot: startup-config won't list the deleted OUIs, but
    // they would be added here.
}


/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

static vtss_rc VOICE_VLAN_switch_del(vtss_isid_t isid)
{
    if (!msg_switch_is_master()) {
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();
    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED) {
        /* Clear LLDP telephony MAC entries on this switch */
        VOICE_VLAN_lldp_telephony_mac_entry_clear_by_switch(isid);

        /* Clear "oui_cnt" */
        memset(VOICE_VLAN_global.oui_cnt[isid], 0x0, sizeof(VOICE_VLAN_global.oui_cnt[isid]));

        /* Clear "sw_learn" flag */
        memset(VOICE_VLAN_global.sw_learn[isid], 0x0, sizeof(VOICE_VLAN_global.sw_learn[isid]));
    }
    VOICE_VLAN_CRIT_EXIT();

    return VTSS_OK;
}

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *VOICE_VLAN_msg_id_txt(voice_vlan_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case VOICE_VLAN_MSG_ID_LLDP_CB_IND:
        txt = "VOICE_VLAN_MSG_ID_LLDP_CB_IND";
        break;
    case VOICE_VLAN_MSG_ID_QCE_SET_REQ:
        txt = "VOICE_VLAN_MSG_ID_QCE_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

static BOOL VOICE_VLAN_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const u32 isid)
{
    voice_vlan_msg_id_t msg_id = *(voice_vlan_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, VOICE_VLAN_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case VOICE_VLAN_MSG_ID_LLDP_CB_IND: {
#if defined(VTSS_SW_OPTION_LLDP)
        voice_vlan_msg_req_t *msg = (voice_vlan_msg_req_t *)rx_msg;
        VOICE_VLAN_porcess_lldp_cb(isid, msg->req.lldp_cb_ind.port_no, &msg->req.lldp_cb_ind.entry);
#endif /* VTSS_SW_OPTION_LLDP */
        break;
    }
    case VOICE_VLAN_MSG_ID_QCE_SET_REQ: {
#if defined(VTSS_FEATURE_QCL_V2)
        voice_vlan_msg_req_t *msg = (voice_vlan_msg_req_t *)rx_msg;
        vtss_port_no_t       iport;

        if (msg->req.qce_conf_set.del_qce) {
            (void) qos_mgmt_qce_entry_del(VTSS_ISID_LOCAL, QCL_USER_VOICE_VLAN, QCL_ID_END, VOICE_VLAN_RESERVED_QCE_ID);
        } else {
            qos_qce_entry_conf_t    qce_conf;

            if (qos_mgmt_qce_entry_get(VTSS_ISID_LOCAL, QCL_USER_VOICE_VLAN, QCL_ID_END, VOICE_VLAN_RESERVED_QCE_ID, &qce_conf, FALSE) != VTSS_OK) {
                memset(&qce_conf, 0x0, sizeof(qce_conf));
                qce_conf.id = VOICE_VLAN_RESERVED_QCE_ID;
                qce_conf.type = VTSS_QCE_TYPE_ANY;
                qce_conf.isid = VTSS_ISID_LOCAL;
            }

            QCE_ENTRY_CONF_ACTION_SET(qce_conf.action.action_bits,
                                      QOS_QCE_ACTION_PRIO, 1);
            qce_conf.action.prio = msg->req.qce_conf_set.traffic_class;
            qce_conf.key.vid.in_range = FALSE;
            qce_conf.key.vid.vr.v.value = msg->req.qce_conf_set.vid;
            qce_conf.key.vid.vr.v.mask = 0xFFF;

            if (msg->req.qce_conf_set.is_port_list) {
                for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                    VTSS_PORT_BF_SET(qce_conf.port_list, iport,
                                     msg->req.qce_conf_set.port_list[iport]);
                }
            } else if (msg->req.qce_conf_set.is_port_add) {
                VTSS_PORT_BF_SET(qce_conf.port_list, msg->req.qce_conf_set.iport, 1);
            } else if (msg->req.qce_conf_set.is_port_del) {
                VTSS_PORT_BF_SET(qce_conf.port_list, msg->req.qce_conf_set.iport, 0);
            }
            (void) qos_mgmt_qce_entry_add(QCL_USER_VOICE_VLAN, QCL_ID_END, QCE_ID_NONE, &qce_conf);
        }
#endif /* VTSS_FEATURE_QCL_V2 */
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc VOICE_VLAN_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = VOICE_VLAN_msg_rx;
    filter.modid = VTSS_MODULE_ID_VOICE_VLAN;
    return msg_rx_filter_register(&filter);
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* VOICE_VLAN error text */
char *voice_vlan_error_txt(vtss_rc rc)
{
    switch (rc) {
    case VOICE_VLAN_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case VOICE_VLAN_ERROR_ISID:
        return "Invalid Switch ID";

    case VOICE_VLAN_ERROR_ISID_NON_EXISTING:
        return "Switch ID is non-existing";

    case VOICE_VLAN_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    case VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MGMT_VID:
        return "Voice VID is conflict with managed VID";

    case VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID:
        return "Voice VID is conflict with MVR VID";

    case VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_STATIC_VID:
        return "Voice VID is conflict with static VID";

    case VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_PVID:
        return "Voice VID is conflict with PVID";

    case VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP:
        return "Voice VLAN port configuration is conflict with LLDP";

    case VOICE_VLAN_ERROR_PARM_NULL_OUI_ADDR:
        return "Voice VLAN module parameter error of null OUI address";

    case VOICE_VLAN_ERROR_ENTRY_NOT_EXIST:
        return "Voice VLAN table entry not exist";

    case VOICE_VLAN_ERROR_REACH_MAX_OUI_ENTRY:
        return "Voice VLAN OUI table reach max entries";

    default:
        return "Voice VLAN: Unknown error code";
    }
}

/* Get VOICE_VLAN configuration */
vtss_rc voice_vlan_mgmt_conf_get(voice_vlan_conf_t *glbl_cfg)
{
    T_D("enter");

    if (glbl_cfg == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();
    *glbl_cfg = VOICE_VLAN_global.conf;
    VOICE_VLAN_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set VOICE_VLAN configuration */
vtss_rc voice_vlan_mgmt_conf_set(voice_vlan_conf_t *glbl_cfg)
{
    vtss_rc                 rc      = VTSS_OK;
    int                     changed = 0;

    T_D("enter, mode: %d, vid: %d age_time: %d traffic_class: %d",
        glbl_cfg->mode, glbl_cfg->vid, glbl_cfg->age_time, glbl_cfg->traffic_class);

    if (glbl_cfg == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    /* Check illegal parameter */
    if ((glbl_cfg->mode != VOICE_VLAN_MGMT_ENABLED && glbl_cfg->mode != VOICE_VLAN_MGMT_DISABLED) ||
        (glbl_cfg->vid < VLAN_ID_MIN || glbl_cfg->vid > VLAN_ID_MAX) ||
        (glbl_cfg->age_time < VOICE_VLAN_MIN_AGE_TIME || glbl_cfg->age_time > VOICE_VLAN_MAX_AGE_TIME) ||
        (glbl_cfg->traffic_class > uprio2iprio(VOICE_VLAN_MAX_TRAFFIC_CLASS)) ||
        VOICE_VLAN_is_valid_voice_vid(glbl_cfg->vid)) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }

    VOICE_VLAN_CRIT_ENTER();
    changed = VOICE_VLAN_conf_changed(&VOICE_VLAN_global.conf, glbl_cfg);
    if (changed) {
        if ((rc = VOICE_VLAN_conf_pre_changed_apply(changed, &VOICE_VLAN_global.conf)) != VTSS_OK) {
            VOICE_VLAN_CRIT_EXIT();
            T_D("exit");
            return rc;
        }
    }
    VOICE_VLAN_global.conf = *glbl_cfg;
    VOICE_VLAN_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t           blk_id  = CONF_BLK_VOICE_VLAN_CONF;
        voice_vlan_conf_blk_t  *voice_vlan_conf_blk_p;
        if ((voice_vlan_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open VOICE_VLAN table");
        } else {
            voice_vlan_conf_blk_p->conf = *glbl_cfg;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        /* Apply configuration post-changed */
        rc = VOICE_VLAN_conf_post_changed_apply(changed, glbl_cfg);
    }

    T_D("exit");
    return rc;
}

/* Get VOICE_VLAN port configuration */
vtss_rc voice_vlan_mgmt_port_conf_get(vtss_isid_t isid, voice_vlan_port_conf_t *switch_cfg)
{
    T_D("enter");

    if (switch_cfg == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return VOICE_VLAN_ERROR_ISID_NON_EXISTING;
    }

    VOICE_VLAN_CRIT_ENTER();
    *switch_cfg = VOICE_VLAN_global.port_conf[isid];
    VOICE_VLAN_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set VOICE_VLAN port configuration */
vtss_rc voice_vlan_mgmt_port_conf_set(vtss_isid_t isid, voice_vlan_port_conf_t *switch_cfg)
{
    vtss_rc                     rc      = VTSS_OK;
    int                         changed = 0;
    port_iter_t                 pit;
    u32                         change_field[VTSS_PORTS];
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if defined(VTSS_SW_OPTION_LLDP)
    lldp_struc_0_t              lldp_conf;
#endif /* VTSS_SW_OPTION_LLDP */
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */

    T_D("enter");

    if (switch_cfg == NULL) {
        T_D("exit");
        return VOICE_VLAN_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return VOICE_VLAN_ERROR_ISID_NON_EXISTING;
    }

    /* Check illegal parameter */
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if ((switch_cfg->port_mode[pit.iport] > VOICE_VLAN_PORT_MODE_FORCED) ||
            (switch_cfg->security[pit.iport] != VOICE_VLAN_MGMT_ENABLED && switch_cfg->security[pit.iport] != VOICE_VLAN_MGMT_DISABLED) ||
            (switch_cfg->discovery_protocol[pit.iport] > VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH)) {
            T_D("exit");
            return VOICE_VLAN_ERROR_INV_PARAM;
        }
#ifndef VTSS_SW_OPTION_LLDP
        switch_cfg->discovery_protocol[pit.iport] = VOICE_VLAN_DISCOVERY_PROTOCOL_OUI;
#endif /* VTSS_SW_OPTION_LLDP */
    }

    VOICE_VLAN_CRIT_ENTER();
    memset(change_field, 0x0, sizeof(change_field));
    changed = VOICE_VLAN_port_conf_changed(isid, &VOICE_VLAN_global.port_conf[isid], switch_cfg, change_field);

#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if defined(VTSS_SW_OPTION_LLDP)
    lldp_mgmt_get_config(&lldp_conf, isid);
    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (change_field[pit.iport] == 0) {
                continue;
            }
            if (changed) {
                /* Check LLDP port mode */
                if ((change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PORT_MODE) || (change_field[pit.iport] & VOICE_VLAN_PORT_CONF_CHANGE_PROTOCOL)) {
                    if (switch_cfg->port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO &&
                        switch_cfg->discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI &&
                        (lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_DISABLED || lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_ENABLED_TX_ONLY)) {
                        VOICE_VLAN_CRIT_EXIT();
                        T_D("exit");
                        return VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP;
                    }
                }
            }
        }
    }
#endif /* VTSS_SW_OPTION_LLDP */
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */

    VOICE_VLAN_global.port_conf[isid] = *switch_cfg;
    VOICE_VLAN_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t               blk_id  = CONF_BLK_VOICE_VLAN_PORT_CONF;
        voice_vlan_port_conf_blk_t  *voice_vlan_port_conf_blk_p;
        if ((voice_vlan_port_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open VOICE_VLAN table");
        } else {
            voice_vlan_port_conf_blk_p->port_conf[isid - VTSS_ISID_START] = *switch_cfg;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        /* Apply configuration post-changed */
        rc = VOICE_VLAN_port_conf_post_changed_apply(change_field, isid, switch_cfg);
    }

    T_D("exit");
    return rc;
}

/* Apply VOICE_VLAN OUI configuration pre-changed */
static vtss_rc VOICE_VLAN_oui_conf_post_changed_apply(void)
{
    vtss_rc         rc = VTSS_OK;
    switch_iter_t   sit;
    port_iter_t     pit;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_D("not master");
        T_D("exit");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

    VOICE_VLAN_CRIT_ENTER();
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (VOICE_VLAN_global.sw_learn[sit.isid][pit.iport]) {
                if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_VOICE_VLAN, NULL, sit.isid, pit.iport, TRUE, TRUE, VOICE_VLAN_psec_on_mac_loop_through_cb, PSEC_PORT_MODE_NORMAL)) == VTSS_OK) {
                    VOICE_VLAN_global.sw_learn[sit.isid][pit.iport] = 1;
                    VOICE_VLAN_global.oui_cnt[sit.isid][pit.iport] = 0;

                    /* Change Voice VLAN member set */
                    if ((VOICE_VLAN_global.port_conf[sit.isid].discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_OUI ||
                         (VOICE_VLAN_global.port_conf[sit.isid].discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH && !VOICE_VLAN_exist_telephony_lldp_device(sit.isid, pit.iport)))) {
                        if ((rc = VOICE_VLAN_disjoin(sit.isid, pit.iport, VOICE_VLAN_global.conf.vid)) != VTSS_OK) {
                            T_W("VOICE_VLAN_disjoin(%u, %d, %d): failed rc = %d", sit.isid, pit.iport, VOICE_VLAN_global.conf.vid, rc);
                        }
                    }
                }
            }
        }
    }
    VOICE_VLAN_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Set VOICE_VLAN OUI configuration */
static vtss_rc VOICE_VLAN_mgmt_oui_conf_set(voice_vlan_oui_conf_t *conf)
{
    vtss_rc                     rc      = VTSS_OK;
    int                         changed = 0;

    T_D("enter");

    VOICE_VLAN_CRIT_ENTER();
    if (msg_switch_is_master()) {
        changed = VOICE_VLAN_oui_conf_changed(&VOICE_VLAN_global.oui_conf, conf);
        VOICE_VLAN_global.oui_conf = *conf;
    } else {
        T_W("not master");
        rc = VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }
    VOICE_VLAN_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t               blk_id  = CONF_BLK_VOICE_VLAN_OUI_TABLE;
        voice_vlan_oui_conf_blk_t   *voice_vlan_oui_conf_blk_p;
        if ((voice_vlan_oui_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open VOICE_VLAN OUI table");
        } else {
            voice_vlan_oui_conf_blk_p->oui_conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        /* Apply configuration post-changed */
        rc = VOICE_VLAN_oui_conf_post_changed_apply();
    }

    T_D("exit");
    return rc;
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Restore VOICE_VLAN configuration */
static vtss_rc VOICE_VLAN_conf_reset(void)
{
    vtss_rc                 rc = VTSS_OK;
    vtss_isid_t             isid;
    voice_vlan_conf_t       conf;
    voice_vlan_port_conf_t  port_conf;
    voice_vlan_oui_conf_t   oui_conf;

    T_D("enter");

    /* Initialize VOICE_VLAN configuration */
    VOICE_VLAN_default_set(&conf);
    rc = voice_vlan_mgmt_conf_set(&conf);

    /* Initialize VOICE_VLAN port configuration */
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (!msg_switch_exists(isid)) {
            continue;
        }
        VOICE_VLAN_port_default_set(isid, &port_conf);
        rc = voice_vlan_mgmt_port_conf_set(isid, &port_conf);
    }

    /* Initialize VOICE_VLAN OUI configuration */
    VOICE_VLAN_oui_default_set(&oui_conf);
    rc = VOICE_VLAN_mgmt_oui_conf_set(&oui_conf);

    T_D("exit");
    return rc;
}

/* Read/create VOICE_VLAN switch configuration */
static void VOICE_VLAN_conf_read_switch(vtss_isid_t isid_add)
{
    conf_blk_id_t               blk_id;
    voice_vlan_port_conf_t      new_port_conf;
    voice_vlan_port_conf_blk_t  *port_blk;
    BOOL                        do_create;
    u32                         size;
    vtss_isid_t                 isid;

    T_D("enter, isid_add: %d", isid_add);

    /* Read VOICE_VLAN port configuration */
    blk_id = CONF_BLK_VOICE_VLAN_PORT_CONF;
    if (misc_conf_read_use()) {
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*port_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            port_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_blk));
            do_create = 1;
        } else if (port_blk->version != VOICE_VLAN_PORT_CONF_BLK_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        port_blk  = NULL;
        do_create = 1;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        VOICE_VLAN_CRIT_ENTER();
        if (do_create) {
            /* Use default values */
            VOICE_VLAN_port_default_set(isid, &new_port_conf);
            if (port_blk != NULL) {
                port_blk->port_conf[isid - VTSS_ISID_START] = new_port_conf;
            }
            VOICE_VLAN_global.port_conf[isid] = new_port_conf;
        } else {
            /* Use new configuration */
            if (port_blk) {  // Quiet lint
                new_port_conf = port_blk->port_conf[isid - VTSS_ISID_START];
                VOICE_VLAN_global.port_conf[isid] = new_port_conf;
            }
        }
        VOICE_VLAN_CRIT_EXIT();
    }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (port_blk == NULL) {
        T_W("failed to open VOICE_VLAN port table");
    } else {
        port_blk->version = VOICE_VLAN_PORT_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif
    T_D("exit");
}

/* Read/create VOICE_VLAN stack configuration */
static vtss_rc VOICE_VLAN_conf_read_stack(BOOL create)
{
    BOOL                        do_create;
    u32                         size;
    voice_vlan_conf_t           new_voice_vlan_conf;
    voice_vlan_conf_blk_t       *conf_blk_p;
    conf_blk_id_t               blk_id;
    u32                         blk_version;
    voice_vlan_oui_conf_t       new_voice_vlan_oui_conf;
    voice_vlan_oui_conf_blk_t   *oui_conf_blk_p;
    vtss_rc                     rc = VTSS_OK;

    T_D("enter, create: %d", create);

    /* Read/create VOICE_VLAN configuration */
    blk_id = CONF_BLK_VOICE_VLAN_CONF;
    blk_version = VOICE_VLAN_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = 1;
    }

    VOICE_VLAN_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        VOICE_VLAN_default_set(&new_voice_vlan_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->conf = new_voice_vlan_conf;
        }
        VOICE_VLAN_lldp_telephony_mac_entry_clear();
        memset(VOICE_VLAN_global.oui_cnt, 0x0, sizeof(VOICE_VLAN_global.oui_cnt));
        memset(VOICE_VLAN_global.sw_learn, 0x0, sizeof(VOICE_VLAN_global.sw_learn));
        VOICE_VLAN_global.conf = new_voice_vlan_conf;
    } else {
        /* Use new configuration */
        if (conf_blk_p) {  // Quiet lint
            VOICE_VLAN_global.conf = conf_blk_p->conf;
        }
    }
    VOICE_VLAN_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open VOICE_VLAN table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    /* Read/create VOICE_VLAN OUI configuration */
    blk_id = CONF_BLK_VOICE_VLAN_OUI_TABLE;
    blk_version = VOICE_VLAN_OUI_CONF_BLK_VERSION;
    if (misc_conf_read_use()) {
        if ((oui_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*oui_conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            oui_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*oui_conf_blk_p));
            do_create = 1;
        } else if (oui_conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        oui_conf_blk_p = NULL;
        do_create      = 1;
    }

    VOICE_VLAN_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        VOICE_VLAN_oui_default_set(&new_voice_vlan_oui_conf);
        if (oui_conf_blk_p != NULL) {
            oui_conf_blk_p->oui_conf = new_voice_vlan_oui_conf;
        }
        VOICE_VLAN_global.oui_conf = new_voice_vlan_oui_conf;
    } else {
        /* Use new configuration */
        if (oui_conf_blk_p) {  // Quiet lint
            VOICE_VLAN_global.oui_conf = oui_conf_blk_p->oui_conf;
        }
    }
    VOICE_VLAN_CRIT_EXIT();
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (oui_conf_blk_p == NULL) {
        T_W("failed to open VOICE_VLAN OUI table");
    } else {
        oui_conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    T_D("exit");
    return rc;
}

/* Module start */
static void VOICE_VLAN_start(BOOL init)
{
    voice_vlan_conf_t       *conf_p;
    voice_vlan_port_conf_t  *port_conf_p;
    voice_vlan_oui_conf_t   *oui_conf_p;
    vtss_isid_t             isid;
    vtss_rc                 rc;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize VOICE_VLAN configuration */
        conf_p = &VOICE_VLAN_global.conf;
        VOICE_VLAN_default_set(conf_p);

        /* Initialize VOICE_VLAN port configuration */
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            port_conf_p = &VOICE_VLAN_global.port_conf[isid];
            VOICE_VLAN_port_default_set(isid, port_conf_p);
        }

        /* Initialize VOICE_VLAN OUI configuration */
        oui_conf_p = &VOICE_VLAN_global.oui_conf;
        VOICE_VLAN_oui_default_set(oui_conf_p);

        /* Initialize LLDP telephony MAC entry */
        VOICE_VLAN_lldp_telephony_mac_entry_clear();

        /* Initialize counters */
        memset(VOICE_VLAN_global.oui_cnt, 0x0, sizeof(VOICE_VLAN_global.oui_cnt));
        memset(VOICE_VLAN_global.sw_learn, 0x0, sizeof(VOICE_VLAN_global.sw_learn));

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&VOICE_VLAN_global.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&VOICE_VLAN_global.crit, "VOICE_VLAN_global.crit", VTSS_MODULE_ID_VOICE_VLAN, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        VOICE_VLAN_CRIT_EXIT();
    } else {
        /* Register for stack messages */
        if ((rc = VOICE_VLAN_stack_register()) != VTSS_OK) {
            T_W("VOICE_VLAN_stack_register(): failed rc = %d", rc);
        }

        /* Register for port linkup/link-down status changed */
        if ((rc = port_global_change_register(VTSS_MODULE_ID_VOICE_VLAN, VOICE_VLAN_port_change_cb)) != VTSS_OK) {
            T_W("port_global_change_register(Voice VLAN): failed rc = %d", rc);
        }

        /* Register to port security module */
        if ((rc = psec_mgmt_register_callbacks(PSEC_USER_VOICE_VLAN, VOICE_VLAN_psec_mac_add_cb, VOICE_VLAN_psec_mac_del_cb)) != VTSS_OK) {
            T_W("psec_mgmt_register_callbacks(Voice VLAN): failed rc = %d", rc);
        }

#if defined(VTSS_SW_OPTION_LLDP)
        /* Register to LLDP module */
        lldp_mgmt_entry_updated_callback_register(VOICE_VLAN_lldp_entry_cb);
#endif /* VTSS_SW_OPTION_LLDP */
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc voice_vlan_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&VOICE_VLAN_trace_reg, VOICE_VLAN_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&VOICE_VLAN_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        VOICE_VLAN_start(1);
#ifdef VTSS_SW_OPTION_VCLI
        voice_vlan_cli_init();
#endif /* VTSS_SW_OPTION_VCLI */

#ifdef VTSS_SW_OPTION_ICFG
        rc = voice_vlan_icfg_init();
        if (rc != VTSS_OK) {
            T_D("fail to init icfg registration, rc = %s", error_txt(rc));
        }
#endif
        break;

    case INIT_CMD_START:
        T_D("START");
        VOICE_VLAN_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            if ((rc = VOICE_VLAN_conf_reset()) != VTSS_OK) {
                T_D("Calling VOICE_VLAN_conf_reset(): failed rc = %s", voice_vlan_error_txt(rc));
            }
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");
        /* Read stack and switch configuration */
        if ((rc = VOICE_VLAN_conf_read_stack(0)) != VTSS_OK) {
            T_W("Calling VOICE_VLAN_conf_read_stack(0): failed rc = %s", voice_vlan_error_txt(rc));
        }
        VOICE_VLAN_conf_read_switch(VTSS_ISID_GLOBAL);
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);

        /* Apply VOICE_VLAN initial configuration */
        if ((rc = VOICE_VLAN_conf_apply(1, isid)) != VTSS_OK) {
            T_D("Calling VOICE_VLAN_conf_apply(isid = %u): failed rc = %s", isid, voice_vlan_error_txt(rc));
        }
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        if ((rc = VOICE_VLAN_switch_del(isid)) != VTSS_OK) {
            T_D("Calling VOICE_VLAN_switch_del(isid = %u): failed rc = %s", isid, voice_vlan_error_txt(rc));
        }
        break;
    default:
        break;
    }

    T_D("exit");
    return rc;
}


/****************************************************************************/
/*  Other management functions                                              */
/****************************************************************************/

/* The API uses for checking conflicted configuration with LLDP module.
 * User cannot set LLDP port mode to ��disabled or TX only�� when Voice-VLAN
 * support LLDP discovery protocol. */
BOOL voice_vlan_is_supported_LLDP_discovery(vtss_isid_t isid, vtss_port_no_t port_no)
{
    T_D("enter");

    VOICE_VLAN_CRIT_ENTER();
    if (VOICE_VLAN_global.conf.mode == VOICE_VLAN_MGMT_ENABLED &&
        VOICE_VLAN_global.port_conf[isid].port_mode[port_no] == VOICE_VLAN_PORT_MODE_AUTO &&
        VOICE_VLAN_global.port_conf[isid].discovery_protocol[port_no] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI) {
        VOICE_VLAN_CRIT_EXIT();
        T_D("exit");
        return TRUE;
    }
    VOICE_VLAN_CRIT_EXIT();

    T_D("exit");
    return FALSE;
}

/* Check Voice VLAN ID is conflict with other configurations */
vtss_rc VOICE_VLAN_is_valid_voice_vid(vtss_vid_t voice_vid)
{
#if VOICE_VLAN_CHECK_CONFLICT_CONF
    vtss_rc             rc = VTSS_OK;
#if defined(VTSS_SW_OPTION_MVR)
    mvr_mgmt_interface_t    mvr_entry;
#endif /* VTSS_SW_OPTION_MVR */

    if (!msg_switch_is_master()) {
        T_W("not master");
        return VOICE_VLAN_ERROR_MUST_BE_MASTER;
    }

#if defined(VTSS_SW_OPTION_MVR)
    /* Check conflict with MVR VID */
    mvr_entry.vid = voice_vid;
    if (mvr_mgmt_get_intf_entry(VTSS_ISID_GLOBAL, &mvr_entry) == VTSS_OK) {
        return VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID;
    }
#endif /* VTSS_SW_OPTION_MVR */

    return rc;
#else
    return VTSS_OK;
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
