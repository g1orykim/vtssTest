/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "cli.h"                /* Should've been called cli_api.h!                */
#include "voice_vlan_api.h"     /* Interface to the module that this file supports */
#include "cli_trace_def.h"      /* Import the CLI trace definitions                */
#include "msg_api.h"            /* For msg_abstime_get()                           */
#include "psec_api.h"
#include "misc_api.h"
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if 0
#include "vlan_api.h"
#endif
#endif
#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif

#include "mgmt_api.h"
#include "topo_api.h"

#define VOICE_VLAN_CLI_PATH     "Voice VLAN "
#define VOICE_VLAN_DBG_CLI_PATH "Debug Voice VLAN "

/******************************************************************************/
// The order that the commands shall appear.
/******************************************************************************/
enum {
    VOICE_VLAN_PRIO_CONF,
    VOICE_VLAN_PRIO_MODE,
    VOICE_VLAN_PRIO_VID,
    VOICE_VLAN_PRIO_AGE_TIME,
    VOICE_VLAN_PRIO_TRAFFIC_CLASS,
    VOICE_VLAN_PRIO_OUI_ADD,
    VOICE_VLAN_PRIO_OUI_DEL,
    VOICE_VLAN_PRIO_OUI_CLEAR,
    VOICE_VLAN_PRIO_OUI_LOOKUP,
    VOICE_VLAN_PRIO_PORT_MODE,
    VOICE_VLAN_PRIO_PORT_SECURITY,
    VOICE_VLAN_PRIO_PORT_DISCOVERY_PROTOCOL,
    VOICE_VLAN_PRIO_LLDP_TELPHONY_MAC_ENTRY = CLI_CMD_SORT_KEY_DEFAULT,
};

/******************************************************************************/
// This defines the things that this module can parse.
// The fields are filled in by the dedicated parsers, and used by the
// PSEC_cli_cmd() function.
/******************************************************************************/
typedef struct {
    u32     relay_info_policy;
    u32     age_time;
    u8      oui_addr[3];
    u32     port_mode;
    u32     discovery_protocol;
} voice_vlan_cli_req_t;


/******************************************************************************/
//
// Module Private Functions
//
/******************************************************************************/

/******************************************************************************/
// VOICE_VLAN_cli_cmd_oui_add()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_oui_add(cli_req_t *req)
{
    vtss_rc                 rc;
    voice_vlan_cli_req_t    *voice_vlan_req = req->module_req;
    voice_vlan_oui_entry_t  entry;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    entry.valid = 1;
    memcpy(entry.oui_addr, voice_vlan_req->oui_addr, 3);
    strcpy(entry.description, req->parm);
    if ((rc = voice_vlan_oui_entry_add(&entry)) != VTSS_OK) {
        if (rc == VOICE_VLAN_ERROR_PARM_NULL_OUI_ADDR) {
            CPRINTF("The null OUI address isn't allowed\n");
        } else if (rc == VOICE_VLAN_ERROR_REACH_MAX_OUI_ENTRY) {
            CPRINTF("The maximum OUI entry number is %d\n", VOICE_VLAN_OUI_ENTRIES_CNT);
        }
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_oui_del()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_oui_del(cli_req_t *req)
{
    vtss_rc                 rc;
    voice_vlan_cli_req_t    *voice_vlan_req = req->module_req;
    voice_vlan_oui_entry_t  entry;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    memcpy(entry.oui_addr, voice_vlan_req->oui_addr, 3);
    if (voice_vlan_oui_entry_get(&entry, FALSE) != VTSS_OK) {
        CPRINTF("Non-existing entry\n");
        return;
    }

    memcpy(entry.oui_addr, voice_vlan_req->oui_addr, 3);
    if ((rc = voice_vlan_oui_entry_del(&entry)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_oui_clear()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_oui_clear(cli_req_t *req)
{
    vtss_rc rc;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if ((rc = voice_vlan_oui_entry_clear()) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_oui_lookup()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_oui_lookup(cli_req_t *req)
{
    voice_vlan_cli_req_t    *voice_vlan_req = req->module_req;
    voice_vlan_oui_entry_t  entry;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    memset(&entry, 0x0, sizeof(entry));

    if (req->set) {
        memcpy(entry.oui_addr, voice_vlan_req->oui_addr, 3);
        if (voice_vlan_oui_entry_get(&entry, FALSE) == VTSS_OK) {
            cli_header("Voice VLAN OUI Table", 1);
            CPRINTF("Telephony OUI Description\n");
            CPRINTF("------------- -----------\n");
            CPRINTF("%02X-%02X-%02X      %s\n", entry.oui_addr[0], entry.oui_addr[1], entry.oui_addr[2], entry.description);
        } else {
            CPRINTF("Non-existing entry\n");
            return;
        }
    } else {
        cli_header("Voice VLAN OUI Table", 1);
        CPRINTF("Telephony OUI Description\n");
        CPRINTF("------------- -----------\n");
        while (voice_vlan_oui_entry_get(&entry, TRUE) == VTSS_OK) {
            CPRINTF("%02X-%02X-%02X      %s\n", entry.oui_addr[0], entry.oui_addr[1], entry.oui_addr[2], entry.description);
        };
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf(cli_req_t *req, BOOL mode, BOOL vid, BOOL age_time, BOOL traffic_class)
{
    voice_vlan_cli_req_t    *voice_vlan_req = req->module_req;
    voice_vlan_conf_t       conf;
    vtss_rc                 rc;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) ||
        voice_vlan_mgmt_conf_get(&conf) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (mode) {
            conf.mode = req->enable;
        }
        if (vid) {
            if ((rc = VOICE_VLAN_is_valid_voice_vid(req->vid)) != VTSS_OK) {
                if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MGMT_VID) {
                    CPRINTF("The Voice VLAN ID should not equal switch management VLAN ID\n");
                    return;
                } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID) {
                    CPRINTF("The Voice VLAN ID should not equal MVR VLAN ID\n");
                    return;
                } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_STATIC_VID) {
                    CPRINTF("The Voice VLAN ID should not equal existing VLAN ID\n");
                    return;
                } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_PVID) {
                    CPRINTF("The Voice VLAN ID should not equal Port PVID\n");
                    return;
                }
            }
            conf.vid = req->vid;
        }
        if (age_time) {
            conf.age_time = voice_vlan_req->age_time;
        }
        if (traffic_class) {
            conf.traffic_class = req->class_;
        }
        if ((rc = voice_vlan_mgmt_conf_set(&conf)) != VTSS_OK) {
            if (rc == VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP) {
                CPRINTF("The LLDP feature should be enabled first.\n");
            } else {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    } else {
        if (mode) {
            CPRINTF("Voice VLAN Mode               : %s\n", cli_bool_txt(conf.mode));
        }
        if (vid) {
            CPRINTF("Voice VLAN VLAN ID            : %d\n", conf.vid);
        }
        if (age_time) {
            CPRINTF("Voice VLAN Age Time(seconds)  : %d\n", conf.age_time);
        }
#if defined(VOICE_VLAN_CLASS_SUPPORTED)
        if (traffic_class) {
            CPRINTF("Voice VLAN Traffic Class      : %s\n", mgmt_prio2txt(conf.traffic_class, 0));
        }
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf_mode()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf_mode(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_conf(req, 1, 0, 0, 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf_vid()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf_vid(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_conf(req, 0, 1, 0, 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf_age_time()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf_age_time(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_conf(req, 0, 0, 1, 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf_traffic_class()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf_traffic_class(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_conf(req, 0, 0, 0, 1);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_port_conf_port_mode()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_port_conf(cli_req_t *req, BOOL mode, bool security, BOOL discovery_protocol)
{
    vtss_rc                 rc;
    voice_vlan_cli_req_t    *voice_vlan_req = req->module_req;
    switch_iter_t           sit;
    port_iter_t             pit;
    BOOL                    first;
    voice_vlan_conf_t       voice_conf;
    voice_vlan_port_conf_t  voice_vlan_port_conf;
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if 0
    vlan_port_conf_t        vlan_port_conf;
#endif
#ifdef VTSS_SW_OPTION_LLDP
    lldp_struc_0_t          lldp_conf;
#endif
#endif
    i8                      buf[80], *p;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    if ((rc = voice_vlan_mgmt_conf_get(&voice_conf)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        if (req->stack.isid[sit.usid] == VTSS_ISID_END || voice_vlan_mgmt_port_conf_get(sit.isid, &voice_vlan_port_conf) != VTSS_OK) {
            continue;
        }

#if VOICE_VLAN_CHECK_CONFLICT_CONF
#ifdef VTSS_SW_OPTION_LLDP
        lldp_mgmt_get_config(&lldp_conf, sit.isid);
#endif
#endif

        first = 1;
        (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            if (req->set) {
                if (mode) {
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if 0
                    if ((rc = vlan_mgmt_port_conf_get(sit.isid, pit.iport, &vlan_port_conf, VLAN_USER_ALL)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }

                    // Check port VLAN awareness mode
                    if (voice_conf.mode == VOICE_VLAN_MGMT_ENABLED &&
                        voice_vlan_req->port_mode != VOICE_VLAN_PORT_MODE_DISABLED &&
                        vlan_port_conf.aware == 0) {
                        CPRINTF("The VLAN awareness is disabled on Port %lu\n", uport);
                        return;
                    }
#endif

                    // Check LLDP port mode
                    if (voice_conf.mode == VOICE_VLAN_MGMT_ENABLED &&
                        voice_vlan_req->port_mode == VOICE_VLAN_PORT_MODE_AUTO &&
                        voice_vlan_port_conf.discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI
#ifdef VTSS_SW_OPTION_LLDP
                        && (lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_DISABLED || lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_ENABLED_TX_ONLY)
#endif
                       ) {

                        CPRINTF("The LLDP mode is disabled on Port %u\n", pit.uport);
                        return;
                    }
#endif /* VOICE_VLAN_CHECK_CONFLICT_CONF */

                    voice_vlan_port_conf.port_mode[pit.iport] = voice_vlan_req->port_mode;
                }
                if (security) {
                    voice_vlan_port_conf.security[pit.iport] = req->enable;
                }
                if (discovery_protocol) {
#if VOICE_VLAN_CHECK_CONFLICT_CONF
#if 0
                    // Check port VLAN awareness mode
                    if (voice_conf.mode == VOICE_VLAN_MGMT_ENABLED &&
                        voice_vlan_port_conf.port_mode[pit.iport] != VOICE_VLAN_PORT_MODE_DISABLED &&
                        vlan_port_conf.aware == 0) {
                        CPRINTF("The VLAN awareness is disabled on Port %lu\n", uport);
                        return;
                    }
#endif

                    // Check LLDP port mode
                    if (voice_conf.mode == VOICE_VLAN_MGMT_ENABLED &&
                        voice_vlan_port_conf.port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO &&
                        voice_vlan_req->discovery_protocol != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI
#ifdef VTSS_SW_OPTION_LLDP
                        && (lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_DISABLED || lldp_conf.admin_state[pit.iport] == (lldp_admin_state_t)LLDP_ENABLED_TX_ONLY)
#endif
                       ) {
                        CPRINTF("The LLDP mode is disabled on Port %u\n", pit.uport);
                        return;
                    }
#endif

                    voice_vlan_port_conf.discovery_protocol[pit.iport] = voice_vlan_req->discovery_protocol;
                }
            } else {
                if (first) {
                    cli_header("Voice VLAN Port Configuration", 1);
                    first = 0;
                    cli_cmd_usid_print(sit.usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Port  ");
                    if (mode) {
                        p += sprintf(p, "Mode      ");
                    }
                    if (security) {
                        p += sprintf(p, "Security  ");
                    }
#ifdef VTSS_SW_OPTION_LLDP
                    if (discovery_protocol) {
                        p += sprintf(p, "Discovery Protocol");
                    }
#endif
                    cli_table_header(buf);
                }
                CPRINTF("%-2u    ", pit.uport);
                if (mode) {
                    CPRINTF("%-10s", voice_vlan_port_conf.port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_DISABLED ? "Disabled" : voice_vlan_port_conf.port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO ? "Auto" : "Forced");
                }
                if (security) {
                    CPRINTF("%-10s  ", cli_bool_txt(voice_vlan_port_conf.security[pit.iport]));
                }
#ifdef VTSS_SW_OPTION_LLDP
                if (discovery_protocol) {
                    CPRINTF("%-5s", voice_vlan_port_conf.discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_OUI ? "OUI" : voice_vlan_port_conf.discovery_protocol[pit.iport] == VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP ? "LLDP" : "Both");
                }
#endif
                CPRINTF("\n");
            }
        }

        if (req->set) {
            if ((rc = voice_vlan_mgmt_port_conf_set(sit.isid, &voice_vlan_port_conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_port_conf_mode()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_port_conf_mode(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_port_conf(req, 1, 0, 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_port_conf_secruity()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_port_conf_secruity(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_port_conf(req, 0, 1, 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_port_conf_discovery_protocol()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_port_conf_discovery_protocol(cli_req_t *req)
{
    VOICE_VLAN_cli_cmd_port_conf(req, 0, 0, 1);
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_show_lldp_telephony_mac_entry()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_show_lldp_telephony_mac_entry(cli_req_t *req)
{
    voice_vlan_lldp_telephony_mac_entry_t   entry;
    i8                                      buf[32];
    u32                                     cnt = 0;

    cli_header("Voice VLAN LLDP telephony MAC entries", 1);
    memset(&entry, 0x0, sizeof(entry));
    while (voice_vlan_lldp_telephony_mac_entry_get(&entry, TRUE) == VTSS_OK) {
        CPRINTF("Entry %d :\n", ++cnt);
        CPRINTF("---------------\n");
        CPRINTF("MAC Address    : %s\n", misc_mac_txt(entry.mac, buf));
        CPRINTF("USID           : %d\n", topo_isid2usid(entry.isid));
        CPRINTF("Port NO        : %d\n\n", iport2uport(entry.port_no));
    }
}

/******************************************************************************/
// VOICE_VLAN_cli_cmd_conf_show()
/******************************************************************************/
static void VOICE_VLAN_cli_cmd_conf_show(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Voice VLAN Configuration", 1);
    }
    VOICE_VLAN_cli_cmd_conf(req, 1, 1, 1, 1);
    VOICE_VLAN_cli_cmd_oui_lookup(req);
    VOICE_VLAN_cli_cmd_port_conf(req, 1, 1, 1);
}

/******************************************************************************/
// VOICE_VLAN_cli_parse_age_time()
/******************************************************************************/
static int32_t VOICE_VLAN_cli_parse_age_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    voice_vlan_cli_req_t *voice_vlan_req = req->module_req;

    req->parm_parsed = 1;

    return cli_parse_ulong(cmd, &voice_vlan_req->age_time, PSEC_AGE_TIME_MIN, PSEC_AGE_TIME_MAX);
}

/******************************************************************************/
// cli_parse_oui_addr()
/******************************************************************************/
static int cli_parse_oui_addr(i8 *cmd, u8 *oui_addr)
{
    uint    i, oui[3];
    int     error = 1;

    if (sscanf(cmd, "%2x-%2x-%2x", &oui[0], &oui[1], &oui[2]) == 3) {
        for (i = 0; i < 3; i++) {
            oui_addr[i] = (oui[i] & 0xff);
        }
        error = 0;
    }

    return error;
}

/******************************************************************************/
// VOICE_VLAN_cli_parse_description()
/******************************************************************************/
static int32_t VOICE_VLAN_cli_parse_description(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0, max = VOICE_VLAN_MAX_DESCRIPTION_LEN;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, max);

    return error;
}

/******************************************************************************/
// VOICE_VLAN_cli_parse_oui_addr()
/******************************************************************************/
/*
 * OUI address:
 * The organizationally unique identifier. An OUI address is a globally unique
 * identifier assigned to a vendor by IEEE. You can determine which vendor a
 * device belongs to according to the OUI address which forms the first 24 bits
 * of a MAC address.
 */
static int32_t VOICE_VLAN_cli_parse_oui_addr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    voice_vlan_cli_req_t *voice_vlan_req = req->module_req;

    req->parm_parsed = 1;

    return (cli_parse_oui_addr(cmd, voice_vlan_req->oui_addr));
}

/******************************************************************************/
// VOICE_VLAN_cli_parse_keyword_port_mode()
/******************************************************************************/
static int32_t VOICE_VLAN_cli_parse_keyword_port_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    i8 *found = cli_parse_find(cmd, stx);
    voice_vlan_cli_req_t *voice_vlan_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "disable", 7)) {
            voice_vlan_req->port_mode = VOICE_VLAN_PORT_MODE_DISABLED;
        } else if (!strncmp(found, "auto", 4)) {
            voice_vlan_req->port_mode = VOICE_VLAN_PORT_MODE_AUTO;
        } else if (!strncmp(found, "force", 5)) {
            voice_vlan_req->port_mode = VOICE_VLAN_PORT_MODE_FORCED;
        }
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// VOICE_VLAN_cli_parse_keyword_discovery_protocol()
/******************************************************************************/
static int32_t VOICE_VLAN_cli_parse_keyword_discovery_protocol(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    i8 *found = cli_parse_find(cmd, stx);
    voice_vlan_cli_req_t *voice_vlan_req = req->module_req;

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "oui", 3)) {
            voice_vlan_req->discovery_protocol = VOICE_VLAN_DISCOVERY_PROTOCOL_OUI;
        } else if (!strncmp(found, "lldp", 4)) {
            voice_vlan_req->discovery_protocol = VOICE_VLAN_DISCOVERY_PROTOCOL_LLDP;
        } else if (!strncmp(found, "both", 4)) {
            voice_vlan_req->discovery_protocol = VOICE_VLAN_DISCOVERY_PROTOCOL_BOTH;
        }
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// voice_vlan_cli_init()
/******************************************************************************/
void voice_vlan_cli_init(void)
{
    // Register the size required for this module's structure */
    cli_req_size_register(sizeof(voice_vlan_cli_req_t));
}


/******************************************************************************/
// Parameters used in this module
// Note to myself, because I forget it all the time:
// CLI_PARM_FLAG_NO_TXT:
//   If included in flags, then help text will look like:
//      enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   If excluded in flags, then help text will look like:
//     enable|disable: enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   I.e., the parameter name is printed first if excluded. And it looks silly
//   in some cases, but is OK in other.
//   If it's a pipe-separated list of keywords (e.g. enable|disable), then
//   the flag should generally be included.
//   If it's a triangle-parenthesis-enclosed keyword (e.g. <age_time>), then
//   the flag should generally be excluded.
/******************************************************************************/
static cli_parm_t voice_vlan_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable Voice VLAN mode.\n"
        "disable: Disable Voice VLAN mode\n"
        "(default: Show flow Voice VLAN mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        VOICE_VLAN_cli_cmd_conf_mode
    },
    {
        "enable|disable",
        "enable : Enable Voice VLAN security mode.\n"
        "disable: Disable Voice VLAN security mode\n"
        "(default: Show flow Voice VLAN security mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        VOICE_VLAN_cli_cmd_port_conf_secruity
    },
    {
        "<age_time>",
        "MAC address age time ("vtss_xstr(PSEC_AGE_TIME_MIN)"-" vtss_xstr(PSEC_AGE_TIME_MAX) ") default: Show age time",
        CLI_PARM_FLAG_SET,
        VOICE_VLAN_cli_parse_age_time,
        VOICE_VLAN_cli_cmd_conf_age_time
    },
    {
        "<class>",
#ifdef VTSS_ARCH_LUTON28
        "Traffic class low/normal/medium/high or 1/2/3/4",
#else
        "Traffic class (0-7)",
#endif
        CLI_PARM_FLAG_SET,
        cli_parm_parse_class,
        VOICE_VLAN_cli_cmd_conf_traffic_class
    },
    {
        "<oui_addr>",
        "OUI address (xx-xx-xx), default: Show OUI address",
        CLI_PARM_FLAG_SET,
        VOICE_VLAN_cli_parse_oui_addr,
        VOICE_VLAN_cli_cmd_oui_lookup
    },
    {
        "<oui_addr>",
        "OUI address (xx-xx-xx). The null OUI address isn't allowed",
        CLI_PARM_FLAG_NONE,
        VOICE_VLAN_cli_parse_oui_addr,
        NULL
    },
    {
        "<description>",
        "Entry description. Use 'clear' or \"\" to clear the string\n"
        "               No blank or space characters are permitted as part of a contact.\n"
        "               (only in CLI)",
        CLI_PARM_FLAG_SET,
        VOICE_VLAN_cli_parse_description,
        VOICE_VLAN_cli_cmd_oui_add
    },
    {
        "disable|auto|force",
        "disable : Disjoin from Voice VLAN.\n"
        "auto    : Enable auto detect mode. It detects whether there is VoIP\n"
        "          phone attached on the specific port and configure the Voice\n"
        "          VLAN members automatically.\n"
        "force   : Forced join to Voice VLAN.\n"
        "(default: Show Voice VLAN port mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        VOICE_VLAN_cli_parse_keyword_port_mode,
        VOICE_VLAN_cli_cmd_port_conf_mode
    },
    {
        "oui|lldp|both",
        "OUI  : Detect telephony device by OUI address.\n"
        "LLDP : Detect telephony device by LLDP.\n"
        "Both : Both OUI and LLDP.\n"
        "(default: Show Voice VLAN discovery protocol)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        VOICE_VLAN_cli_parse_keyword_discovery_protocol,
        VOICE_VLAN_cli_cmd_port_conf_discovery_protocol
    },
    {NULL, NULL, 0, 0, NULL}
};


/******************************************************************************/
// Commands defined in this module
/******************************************************************************/
cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "Configuration",
    NULL,
    "Show Voice VLAN configuration",
    VOICE_VLAN_PRIO_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_conf_show,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "Mode",
    VOICE_VLAN_CLI_PATH "Mode [enable|disable]",
    "Set or show the Voice VLAN mode.\n"
    "We must disable MSTP feature before we enable Voice VLAN.\n"
    "It can avoid the conflict of ingress filter",
    VOICE_VLAN_PRIO_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_conf_mode,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "ID",
    VOICE_VLAN_CLI_PATH "ID [<vid>]",
    "Set or show Voice VLAN ID",
    VOICE_VLAN_PRIO_VID,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_conf_vid,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "Agetime",
    VOICE_VLAN_CLI_PATH "Agetime [<age_time>]",
    "Set or show Voice VLAN age time",
    VOICE_VLAN_PRIO_AGE_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_conf_age_time,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VOICE_VLAN_CLASS_SUPPORTED)
cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "Traffic Class",
    VOICE_VLAN_CLI_PATH "Traffic Class [<class>]",
    "Set or show Voice VLAN ID",
    VOICE_VLAN_PRIO_TRAFFIC_CLASS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_conf_traffic_class,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VOICE_VLAN_CLASS_SUPPORTED */

cli_cmd_tab_entry(
    NULL,
    VOICE_VLAN_CLI_PATH "OUI Add <oui_addr> [<description>]",
    "Add Voice VLAN OUI entry.\n"
    "Modify OUI table will restart auto detect OUI process.\n"
    "The maximum entry number is " vtss_xstr(VOICE_VLAN_OUI_ENTRIES_CNT) "",
    VOICE_VLAN_PRIO_OUI_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_oui_add,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VOICE_VLAN_CLI_PATH "OUI Delete <oui_addr>",
    "Delete Voice VLAN OUI entry.\n"
    "Modify OUI table will restart auto detect OUI process",
    VOICE_VLAN_PRIO_OUI_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_oui_del,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    NULL,
    VOICE_VLAN_CLI_PATH "OUI Clear",
    "Clear Voice VLAN OUI entry.\n"
    "Modify OUI table will restart auto detect OUI process",
    VOICE_VLAN_PRIO_OUI_CLEAR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_oui_clear,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VOICE_VLAN_CLI_PATH "OUI Lookup [<oui_addr>]",
    NULL,
    "Lookup Voice VLAN OUI entry",
    VOICE_VLAN_PRIO_OUI_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_oui_lookup,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VOICE_VLAN_CLI_PATH "Port Mode [<port_list>]",
    VOICE_VLAN_CLI_PATH "Port Mode [<port_list>] [disable|auto|force]",
    "Set or show the Voice VLAN port mode.\n"
    "When the port mode isn't disabled, we must disable MSTP feature\n"
    "before we enable Voice VLAN. It can avoid the conflict of ingress filter",
    VOICE_VLAN_PRIO_PORT_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_port_conf_mode,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VOICE_VLAN_CLI_PATH "Security [<port_list>]",
    VOICE_VLAN_CLI_PATH "Security [<port_list>] [enable|disable]",
    "Set or show the Voice VLAN port security mode. When the function is enabled,\n"
    "all non-telephone MAC address in Voice VLAN will be blocked 10 seconds",
    VOICE_VLAN_PRIO_PORT_SECURITY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_port_conf_secruity,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_LLDP
cli_cmd_tab_entry (
    VOICE_VLAN_CLI_PATH "Discovery Protocol [<port_list>]",
    VOICE_VLAN_CLI_PATH "Discovery Protocol [<port_list>] [oui|lldp|both]",
    "Set or show the Voice VLAN port discovery protocol mode.\n"
    "It only work under auto detect mode is enabled.\n"
    "We should enable LLDP feature before configure discovery protocol\n"
    "to 'LLDP' or 'Both'.\n"
    "Change discovery protocol to 'OUI' or 'LLDP' will restart auto\n"
    "detect process",
    VOICE_VLAN_PRIO_PORT_DISCOVERY_PROTOCOL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VOICE_VLAN,
    VOICE_VLAN_cli_cmd_port_conf_discovery_protocol,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry (
    VOICE_VLAN_DBG_CLI_PATH "LLDP Telephony MAC Entry [<port_list>]",
    NULL,
    "Show the Voice VLAN LLDP telephony MAC entry",
    VOICE_VLAN_PRIO_LLDP_TELPHONY_MAC_ENTRY,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    VOICE_VLAN_cli_cmd_show_lldp_telephony_mac_entry,
    NULL,
    voice_vlan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
