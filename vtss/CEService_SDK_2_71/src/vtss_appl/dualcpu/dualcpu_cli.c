/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "cli.h"
#include "rpc_api.h"

#ifdef DUALCPU_MASTER

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void cli_dualcpu_port_status(cli_req_t *req)
{
    vtss_port_status_t status;
    vtss_rc rc;
    if((rc = rpc_vtss_port_status_get(NULL, req->uport-1, &status)) == VTSS_RC_OK) {
        CPRINTF("port(%ld): Link %d, speed %d, fdx %d\n", req->uport, 
                status.link, status.speed, status.fdx);
    } else {
        CPRINTF("vtss_port_status_get(%ld): Failure - rc = 0x%0x\n", req->uport, rc);
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t dualcpu_cli_parm_table[] = {
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* DUALCPU CLI Command Sorting Order */
enum {
    CLI_CMD_DUALCPU_PORT_STATUS = 0,
};

/* Command table entries */
cli_cmd_tab_entry(
        "dualcpu port <port>",
        NULL,
        "Show Remote port state",
        CLI_CMD_DUALCPU_PORT_STATUS,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_DUALCPU,
        cli_dualcpu_port_status,
        NULL,
        dualcpu_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
