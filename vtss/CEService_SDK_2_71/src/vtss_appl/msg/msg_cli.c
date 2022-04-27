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
#include "cli.h"
#include "cli_api.h"
#include "vtss_module_id.h"
#include "msg_api.h"
#include "cli_trace_def.h"

/*lint -esym(459, cli_cmd_debug_msg) */

static void cli_cmd_debug_msg(cli_req_t *req)
{
    u32 parms[CLI_INT_VALUES_MAX];
    int i;

    for (i = 0; i < CLI_INT_VALUES_MAX; i++) {
        parms[i] = req->int_values[i];
    }

    msg_dbg(cli_printf, req->int_value_cnt, parms);
} /* cli_cmd_debug_msg */


int32_t cli_msg_integer_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    return error;
}
enum {
    PRIO_DEBUG_MSG = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    NULL,
    "Debug Msg [<integer>] [<integer>] [<integer>]\n"
    "          [<integer>] [<integer>] [<integer>]",
    "Debug Message Module",
    PRIO_DEBUG_MSG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_msg,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);
