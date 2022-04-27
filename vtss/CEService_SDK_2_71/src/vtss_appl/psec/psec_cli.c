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

/*lint -esym(459, PSEC_cli_cmd_port)       */
/*lint -esym(459, PSEC_cli_cmd_dbg_port)   */
/*lint -esym(459, PSEC_cli_cmd_dbg_shaper) */

#include "cli.h"           /* Should've been called cli_api.h!                        */
#include "cli_grp_help.h" /* For Security group defs access */
#include "psec_api.h"      /* Interface to the module that this file supports         */
#include "psec_cli.h"      /* Check that public function decls. and defs. are in sync */
#include "cli_trace_def.h" /* Import the CLI trace definitions                        */
#include "psec.h"          /* For semi-public PSEC functions                          */
#include "msg_api.h"       /* For msg_abstime_get()                                   */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PSEC

#define PSEC_CLI_PATH     "Psec "
#define PSEC_DBG_CLI_PATH "Debug Psec "

/******************************************************************************/
// The order that the commands shall appear.
/******************************************************************************/
typedef enum {
    PSEC_CLI_PRIO_SWITCH,
    PSEC_CLI_PRIO_PORT,
    PSEC_CLI_PRIO_DBG_SWITCH      = CLI_CMD_SORT_KEY_DEFAULT,
    PSEC_CLI_PRIO_DBG_PORT        = CLI_CMD_SORT_KEY_DEFAULT,
    PSEC_CLI_PRIO_DBG_SHAPER      = CLI_CMD_SORT_KEY_DEFAULT,
    PSEC_CLI_PRIO_DBG_SHAPER_STAT = CLI_CMD_SORT_KEY_DEFAULT,
} psec_cli_prio_t;

/******************************************************************************/
// This defines the things that this module can parse.
// The fields are filled in by the dedicated parsers, and used by the
// PSEC_cli_cmd() function.
/******************************************************************************/
typedef struct {
    psec_rate_limit_cfg_t rate_limit_cfg;
    BOOL                  fill_level_min_set;
    BOOL                  fill_level_max_set;
    BOOL                  rate_set;
    BOOL                  drop_age_set;
} psec_cli_req_t;

/******************************************************************************/
//
// Static variables
//
/******************************************************************************/

// Forward declaration of static functions
static void    PSEC_cli_cmd_switch         (cli_req_t *req);
static void    PSEC_cli_cmd_port           (cli_req_t *req);
static void    PSEC_cli_cmd_dbg_switch     (cli_req_t *req);
static void    PSEC_cli_cmd_dbg_port       (cli_req_t *req);
static void    PSEC_cli_cmd_dbg_shaper     (cli_req_t *req);
static void    PSEC_cli_cmd_dbg_shaper_stat(cli_req_t *req);
static int32_t PSEC_cli_parse_fill_level_min(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t PSEC_cli_parse_fill_level_max(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t PSEC_cli_parse_rate          (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);
static int32_t PSEC_cli_parse_drop_age      (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req);

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
static cli_parm_t PSEC_cli_parm_table[] = {
    {
        "<fill_level_min>",
        "Hysteresis: After the burst capacity has been reached, do not allow new frames until it reaches this amount\n"
        "(default: Show current minimum fill level - measured in frames",
        CLI_PARM_FLAG_SET,
        PSEC_cli_parse_fill_level_min,
        PSEC_cli_cmd_dbg_shaper
    },
    {
        "<fill_level_max>",
        "Burst capacity. At most this amount of frames in a burst\n"
        "(default: Show current burst capacity)",
        CLI_PARM_FLAG_SET,
        PSEC_cli_parse_fill_level_max,
        PSEC_cli_cmd_dbg_shaper
    },
    {
        "<shaper_rate>",
        "At most this amount of frames per second over time (better to pick a power-of-two for this number).\n"
        "(default: Show current rate)",
        CLI_PARM_FLAG_SET,
        PSEC_cli_parse_rate,
        PSEC_cli_cmd_dbg_shaper
    },
    {
        "<drop_age>",
        "Drop the frame if it is less than this amount of milliseconds since last frame with this MAC address was seen.\n"
        "(default: Show current drop age)",
        CLI_PARM_FLAG_SET,
        PSEC_cli_parse_drop_age,
        PSEC_cli_cmd_dbg_shaper
    },
    {
        "clear",
        "Clear the current statistics.",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        PSEC_cli_cmd_dbg_shaper_stat
    },
    {NULL, NULL, 0, 0, NULL}
};

/******************************************************************************/
// Commands defined in this module
/******************************************************************************/
cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_CLI_PATH "Switch [<port_list>]",
    NULL,
    "Show Port Security status",
    PSEC_CLI_PRIO_SWITCH,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    PSEC_cli_cmd_switch,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    VTSS_CLI_GRP_SEC_NETWORK_PATH PSEC_CLI_PATH "Port [<port_list>]",
    NULL,
    "Show MAC Addresses learned by Port Security",
    PSEC_CLI_PRIO_PORT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    PSEC_cli_cmd_port,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    PSEC_DBG_CLI_PATH "Switch [<port_list>]",
    NULL,
    "Show Port Security status",
    PSEC_CLI_PRIO_DBG_SWITCH,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    PSEC_cli_cmd_dbg_switch,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    PSEC_DBG_CLI_PATH "Port [<port_list>]",
    NULL,
    "Show MAC Addresses learned by Port Security",
    PSEC_CLI_PRIO_DBG_PORT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    PSEC_cli_cmd_dbg_port,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    PSEC_DBG_CLI_PATH "Shaper",
    PSEC_DBG_CLI_PATH "Shaper [<fill_level_min>] [<fill_level_max>] [<shaper_rate>] [<drop_age>]",
    "Show Port Security shaper configuration",
    PSEC_CLI_PRIO_DBG_SHAPER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PSEC_cli_cmd_dbg_shaper,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    PSEC_DBG_CLI_PATH "Statistics",
    PSEC_DBG_CLI_PATH "Statistics [clear]",
    "Get or clear shaper statistics",
    PSEC_CLI_PRIO_DBG_SHAPER_STAT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    PSEC_cli_cmd_dbg_shaper_stat,
    NULL,
    PSEC_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

/******************************************************************************/
//
// Module Private Functions
//
/******************************************************************************/

/******************************************************************************/
// PSEC_cli_print_users()
/******************************************************************************/
static void PSEC_cli_print_users(void)
{
    psec_users_t user;
    cli_printf("Users:\n");

    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        cli_printf("%c = %s\n", psec_user_abbr(user), psec_user_name(user));
    }
}

/******************************************************************************/
// PSEC_cli_populate_ena_str()
/******************************************************************************/
static void PSEC_cli_populate_ena_str(u8 *buf, psec_port_state_t *port_state)
{
    psec_users_t user;
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        *(buf++) = PSEC_USER_ENA_GET(port_state, user) ? psec_user_abbr(user) : '-';
    }
    *buf = '\0';
}

/******************************************************************************/
// PSEC_cli_populate_add_method_str()
/******************************************************************************/
static void PSEC_cli_populate_add_method_str(i8 *buf, psec_port_state_t *port_state, psec_mac_state_t *mac_state, psec_add_method_t method)
{
    psec_users_t user;
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        *(buf++) = (PSEC_USER_ENA_GET(port_state, user) && PSEC_FORWARD_DECISION_GET(mac_state, user) == method) ? psec_user_abbr(user) : '-';
    }
    *buf = '\0';
}

/******************************************************************************/
// PSEC_cli_cmd_switch_status()
/******************************************************************************/
static void PSEC_cli_cmd_switch_status(cli_req_t *req, BOOL debug)
{
    vtss_rc           rc;
    u8                ena_str[PSEC_USER_CNT + 1]; // +1 for terminating NULL
    psec_port_state_t port_state;
    switch_iter_t     sit;

    PSEC_cli_print_users();

    if (debug) {
        cli_printf(
            "Flags:\n"
            "  D = Port shut down\n"
            "  L = Limit is reached\n"
            "  E = Secure learning enabled\n"
            "  C = CPU copying enabled\n"
            "  U = Link is up\n"
            "  H = H/W add failed\n"
            "  S = S/W add failed\n"
        );
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            if (pit.first) {
                cli_cmd_usid_print(sit.usid, req, TRUE);
                if (debug) {
                    cli_table_header("Port  Users  Flags    MAC Cnt");
                } else {
                    cli_table_header("Port  Users  State          MAC Cnt");
                }
            }

            // The FALSE argument tells the psec_mgmt_port_state_get() function
            // that we haven't obtained the critical section, so that the function
            // should do it itself.
            if ((rc = psec_mgmt_port_state_get(sit.isid, pit.iport, &port_state, FALSE)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                return;
            }

            PSEC_cli_populate_ena_str(ena_str, &port_state);

            cli_printf("%-4d  %-5s  ", pit.uport, ena_str);

            if (debug) {
                cli_printf(
                    "%c%c%c%c%c%c%c",
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN)     ? 'D' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) ? 'L' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_SEC_LEARNING)  ? 'E' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_CPU_COPYING)   ? 'C' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_LINK_UP)       ? 'U' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_HW_ADD_FAILED) ? 'H' : '-',
                    (port_state.flags & PSEC_PORT_STATE_FLAGS_SW_ADD_FAILED) ? 'S' : '-'
                );
            } else {
                cli_printf("%-13s", port_state.ena_mask == 0 ? "No users" : (port_state.flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN) ? "Shutdown" : (port_state.flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) ? "Limit Reached" : "Ready");
            }

            cli_printf("  %7u\n", port_state.mac_cnt);
        } // while (cli_port_iter_getnext())
        cli_printf("\n");
    } // while (cli_switch_iter_getnext())
}

/******************************************************************************/
// PSEC_cli_cmd_switch()
/******************************************************************************/
static void PSEC_cli_cmd_switch(cli_req_t *req)
{
    PSEC_cli_cmd_switch_status(req, FALSE);
}

/******************************************************************************/
// PSEC_cli_cmd_dbg_switch()
/******************************************************************************/
static void PSEC_cli_cmd_dbg_switch(cli_req_t *req)
{
    PSEC_cli_cmd_switch_status(req, TRUE);
}

/******************************************************************************/
// PSEC_cli_cmd_port()
/******************************************************************************/
static void PSEC_cli_cmd_port(cli_req_t *req)
{
    vtss_rc           rc;
    i8                buf[30];
    psec_port_state_t port_state;
    psec_mac_state_t  *mac_state;
    switch_iter_t     sit;

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            if (req->stack.count > 1) {
                sprintf(buf, "Switch %d, Port %d", sit.usid, pit.uport);
                cli_header_nl(buf, 1, 1);
            } else {
                sprintf(buf, "Port %d", pit.uport);
                cli_header_nl(buf, 1, 1);
            }

            cli_table_header("MAC Address        VID   State       Added                      Age/Hold Time");

            // The TRUE in the call to psec_mgmt_port_state_get() means that we also need
            // the MAC addresses. The function VTSS_MALLOC()s an array of such addresses, and
            // we must free it.
            if ((rc = psec_mgmt_port_state_get(sit.isid, pit.iport, &port_state, TRUE)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                return;
            }

            // Loop through all MAC addresses attached to this port
            mac_state = port_state.macs;
            if (mac_state) {
                while (mac_state) {
                    if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
                        if (mac_state->age_or_hold_time_secs == 0) {
                            sprintf(buf, "N/A");
                        } else {
                            sprintf(buf, "%13u", mac_state->age_or_hold_time_secs);
                        }

                        // Don't bother the user with entries not added to the MAC table for one or the other reason.
                        cli_printf(
                            "%17s  %4u  %-10s  %20s  %13s\n",
                            misc_mac2str(mac_state->vid_mac.mac.addr),
                            mac_state->vid_mac.vid,
                            (mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED) ? "Blocked" : "Forwarding",
                            misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, mac_state->creation_time_secs)), /* 25 chars */
                            buf
                        );
                    }

                    mac_state = mac_state->next;
                } /* while (mac_state) */
                cli_printf("\n");
            } else {
                cli_printf("<none>\n");
            } /* if (mac_state) */

            // Must free the MAC address "array".
            if (port_state.macs) {
                VTSS_FREE(port_state.macs);
            }

        } // while (cli_port_iter_getnext())
    } // while (cli_switch_iter_getnext())
}

/******************************************************************************/
// PSEC_cli_cmd_dbg_port()
/******************************************************************************/
static void PSEC_cli_cmd_dbg_port(cli_req_t *req)
{
    vtss_rc           rc;
    i8                buf1[30];
    i8                buf2[30];
    i8                buf3[30];
    psec_port_state_t port_state;
    psec_mac_state_t  *mac_state;
    switch_iter_t     sit;

    PSEC_cli_print_users();
    cli_printf(
        "Flags:\n"
        "  B = Entry is blocked but subject to 'aging'\n"
        "  K = Entry is kept blocked (not subject to 'aging')\n"
        "  F = Entry is forwarding\n"
        "  M = Entry is indeed in MAC table\n"
        "  C = Entry is being aged (copying to CPU)\n"
        "  A = An Age frame was seen\n"
        "  H = H/W add of entry failed\n"
        "  S = S/W add of entry failed\n"
        "Per-User Forward Decisions:\n"
        "  F = Forward By\n"
        "  B = Block By\n"
        "  K = Keep Blocked By\n"
    );

    // Handle the per-port configuration
    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {
            if (req->stack.count > 1) {
                sprintf(buf1, "Switch %d, Port %d", sit.usid, pit.uport);
                cli_header_nl(buf1, 1, 1);
            } else {
                sprintf(buf1, "Port %d", pit.uport);
                cli_header_nl(buf1, 1, 1);
            }

            cli_table_header("MAC Address        VID   Flags   First Added  Last Changed  Age/Hold Time  F     B     K     ");

            if ((rc = psec_mgmt_port_state_get(sit.isid, pit.iport, &port_state, TRUE)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                return;
            }

            // Loop through all MAC addresses attached to this port
            mac_state = port_state.macs;
            if (mac_state) {
                while (mac_state) {
                    PSEC_cli_populate_add_method_str(buf1, &port_state, mac_state, PSEC_ADD_METHOD_FORWARD);
                    PSEC_cli_populate_add_method_str(buf2, &port_state, mac_state, PSEC_ADD_METHOD_BLOCK);
                    PSEC_cli_populate_add_method_str(buf3, &port_state, mac_state, PSEC_ADD_METHOD_KEEP_BLOCKED);
                    cli_printf(
                        "%17s  %4u  %c%c%c%c%c%c  %11u  %12u  %13u  %-3s  %-3s  %-3s\n",
                        misc_mac2str(mac_state->vid_mac.mac.addr),
                        mac_state->vid_mac.vid,
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED)        ? ((mac_state->flags & PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED) ? 'K' : 'B') : 'F',
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE)  ? 'M' : '-',
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_CPU_COPYING)    ? 'C' : '-',
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN) ? 'A' : '-',
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED)  ? 'H' : '-',
                        (mac_state->flags & PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED)  ? 'S' : '-',
                        mac_state->creation_time_secs,
                        mac_state->changed_time_secs,
                        mac_state->age_or_hold_time_secs,
                        buf1,
                        buf2,
                        buf3
                    );

                    mac_state = mac_state->next;
                } /* while (mac_state) */
                cli_printf("\n");
            } else {
                cli_printf("<none>\n");
            } /* if (mac_state) */

            // Must free the MAC address "array".
            if (port_state.macs) {
                VTSS_FREE(port_state.macs);
            }

        } // while (cli_port_iter_getnext())
    } // while (cli_switch_iter_getnext())
}

/******************************************************************************/
// PSEC_cli_cmd_dbg_shaper()
/******************************************************************************/
static void PSEC_cli_cmd_dbg_shaper(cli_req_t *req)
{
    vtss_rc               rc;
    psec_rate_limit_cfg_t rate_limit_cfg;
    psec_cli_req_t        *psec_cli_req = (psec_cli_req_t *)req->module_req;

    if ((rc = psec_dbg_shaper_cfg_get(&rate_limit_cfg)) == VTSS_OK) {
        if (req->set) {
            if (psec_cli_req->fill_level_min_set) {
                rate_limit_cfg.fill_level_min = psec_cli_req->rate_limit_cfg.fill_level_min;
            }
            if (psec_cli_req->fill_level_max_set) {
                rate_limit_cfg.fill_level_max = psec_cli_req->rate_limit_cfg.fill_level_max;
            }
            if (psec_cli_req->rate_set) {
                rate_limit_cfg.rate = psec_cli_req->rate_limit_cfg.rate;
            }
            if (psec_cli_req->drop_age_set) {
                rate_limit_cfg.drop_age_ms = psec_cli_req->rate_limit_cfg.drop_age_ms;
            }

            rc = psec_dbg_shaper_cfg_set(&rate_limit_cfg);
        } else {
            // Print current settings
            cli_printf(
                "Min Fill Level: %10u frames\n"
                "Max Fill Level: %10u frames\n"
                "Emptying Rate : %10u frames/second\n"
                "Drop age      : %10llu msecs\n",
                rate_limit_cfg.fill_level_min,
                rate_limit_cfg.fill_level_max,
                rate_limit_cfg.rate,
                rate_limit_cfg.drop_age_ms
            );
        }
    }

    if (rc != VTSS_OK) {
        cli_printf("%s\n", error_txt(rc));
    }
}

/****************************************************************************/
// PSEC_cli_cmd_dbg_shaper_stat()
/****************************************************************************/
static void PSEC_cli_cmd_dbg_shaper_stat(cli_req_t *req)
{
    vtss_rc                rc;
    psec_rate_limit_stat_t stat;
    u64                    cur_fill_level;

    if (req->clear) {
        rc = psec_dbg_shaper_stat_clr();
    } else {
        if ((rc = psec_dbg_shaper_stat_get(&stat, &cur_fill_level)) == VTSS_OK) {
            cli_printf(
                "Frame Counters:\n"
                "Forwarded       : %12llu\n"
                "Dropped (shaper): %12llu\n"
                "Dropped (filter): %12llu\n"
                "Fill-level      : %12llu\n",
                stat.forward_cnt,
                stat.drop_cnt,
                stat.filter_drop_cnt,
                cur_fill_level
            );
        }
    }

    if (rc != VTSS_OK) {
        cli_printf("%s\n", error_txt(rc));
    }
}

/******************************************************************************/
// PSEC_cli_parse_fill_level_min()
/******************************************************************************/
static int32_t PSEC_cli_parse_fill_level_min(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    psec_cli_req_t *psec_cli_req = (psec_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    psec_cli_req->fill_level_min_set = TRUE;
    return cli_parse_ulong(cmd, &psec_cli_req->rate_limit_cfg.fill_level_min, 0, UINT_MAX);
}

/******************************************************************************/
// PSEC_cli_parse_fill_level_max()
/******************************************************************************/
static int32_t PSEC_cli_parse_fill_level_max(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    psec_cli_req_t *psec_cli_req = (psec_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    psec_cli_req->fill_level_max_set = TRUE;
    return cli_parse_ulong(cmd, &psec_cli_req->rate_limit_cfg.fill_level_max, 1, UINT_MAX);
}

/******************************************************************************/
// PSEC_cli_parse_rate()
/******************************************************************************/
static int32_t PSEC_cli_parse_rate(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    psec_cli_req_t *psec_cli_req = (psec_cli_req_t *)req->module_req;

    req->parm_parsed = 1;
    psec_cli_req->rate_set = TRUE;
    return cli_parse_ulong(cmd, &psec_cli_req->rate_limit_cfg.rate, 1, UINT_MAX);
}

/******************************************************************************/
// PSEC_cli_parse_rate()
/******************************************************************************/
static int32_t PSEC_cli_parse_drop_age(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    psec_cli_req_t *psec_cli_req = (psec_cli_req_t *)req->module_req;
    u32            temp;
    int32_t        result;

    req->parm_parsed = 1;
    psec_cli_req->drop_age_set = TRUE;
    result = cli_parse_ulong(cmd, &temp, 0, UINT_MAX);
    psec_cli_req->rate_limit_cfg.drop_age_ms = (u64)temp;
    return result;
}

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// psec_cli_init()
/******************************************************************************/
void psec_cli_init(void)
{
    // Register the size required for this module's structure
    cli_req_size_register(sizeof(psec_cli_req_t));
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
