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
#include "dhcp_helper_api.h"
#include "topo_api.h"
#include "cli_trace_def.h"
#include <network.h>

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void DHCP_HELPER_cli_cmd_debug_frame_info(cli_req_t *req)
{
    u8                          mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    vtss_vid_t                  vid = 0;
    uint                        transaction_id = 0;
    dhcp_helper_frame_info_t    info;
    BOOL                        rc;
    char                        buf[32];
    u32                         cnt = 0;

    CPRINTF("DHCP Helper Frame Information :\n");
    CPRINTF("-------------------------------\n");
    do {
        rc = dhcp_helper_frame_info_getnext(mac, vid, transaction_id, &info);
        if (rc) {
            CPRINTF("\nEntry %d :\n", ++cnt);
            CPRINTF("---------------\n");
            CPRINTF("MAC Address    : %s\n", misc_mac_txt(info.mac, buf));
            CPRINTF("VLAN ID        : %d\n", info.vid);
            CPRINTF("USID           : %d\n", topo_isid2usid(info.isid));
            CPRINTF("Port NO        : %d\n", iport2uport(info.port_no));
            CPRINTF("OP Code        : %d\n", info.op_code);
            CPRINTF("Transaction ID : 0x%x\n", ntohl(info.transaction_id));
            CPRINTF("IP Address     : %s\n", misc_ipv4_txt(info.assigned_ip, buf));
            CPRINTF("Lease Time     : %d\n", info.lease_time);
            CPRINTF("Time Stamp     : %d\n", info.timestamp);
            memcpy(mac, info.mac, 6);
            vid = info.vid;
            transaction_id = info.transaction_id;
        }
    } while (rc);
    CPRINTF("\nTotal Entries Number : %d\n", cnt);
}

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    DHCP_HELPER_PRIO_DEBUG_FRAME_INFO = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
    "Debug DHCP Helper Frame Information",
    NULL,
    "Show DHCP helper frame information",
    DHCP_HELPER_PRIO_DEBUG_FRAME_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    DHCP_HELPER_cli_cmd_debug_frame_info,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
