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

// Avoid "cli_grp_help.h not used in module system_cli.c"
/*lint --e{766} */

#include "main.h"
#include "conf_api.h"
#include "icli_api.h"
#include "cli_api.h"
#include "msg_api.h"
#include "misc_api.h"
#include "vtss_api_if_api.h" /* For vtss_api_chipid() */
#include "control_api.h"
#include "sysutil_api.h"
#include "topo_api.h"
#include "firmware_icli_functions.h"

#include <cyg/io/flash.h>

static char *_board_type_name(
    int     board_type
)
{
    switch ( board_type ) {
    case VTSS_BOARD_UNKNOWN:
    default:
        return "Unknown";

    case VTSS_BOARD_ESTAX_34_REF:
        return "E-StaX-34 Reference";

    case VTSS_BOARD_ESTAX_34_ENZO:
        return "E-StaX-34 Enzo";

    case VTSS_BOARD_ESTAX_34_ENZO_SFP:
        return "E-StaX-34 Enzo SFP";

    case VTSS_BOARD_LUTON10_REF:
        return "Luton10";

    case VTSS_BOARD_LUTON26_REF:
        return "Luton26";

    case VTSS_BOARD_JAG_CU24_REF:
        return "Jaguar-1 CU24 Reference";

    case VTSS_BOARD_JAG_SFP24_REF:
        return "Jaguar-1 SFP24 Reference";

    case VTSS_BOARD_JAG_PCB107_REF:
        return "Jaguar-1 Tesla CU24 Reference";

    case VTSS_BOARD_UNUSED:
        return "Unused";

    case VTSS_BOARD_JAG_CU48_REF:
        return "Jaguar-1 CU48 Reference";

    case VTSS_BOARD_SERVAL_REF:
        return "Serval";

    case VTSS_BOARD_SERVAL_PCB106_REF:
        return "Serval PCB106";
    }
}

/* System configuration */
void sysutil_icli_func_conf(
    u32     session_id
)
{
    uchar                   mac[6];
    msg_switch_info_t       info;
    vtss_isid_t             isid;
    system_conf_t           conf;
    vtss_restart_status_t   status;
    char                    buf[32];
    cyg_flash_info_t        flash_info;
    int                     ret;
    u32                     i;
    u32                     j;
    const char              *code_rev;

    ICLI_PRINTF("\n");

#if CYGINT_ISO_MALLINFO
{
    struct mallinfo mem_info;

    mem_info = mallinfo();

    ICLI_PRINTF("MEMORY           : Total=%d KBytes, Free=%d KBytes, Max=%d KBytes\n",
        mem_info.arena / 1024, mem_info.fordblks / 1024, mem_info.maxfree / 1024);
}
#endif /* CYGINT_ISO_MALLINFO */

    i = 0;
    do {
        ret = cyg_flash_get_info(i, &flash_info);
        if (ret == CYG_FLASH_ERR_OK) {
            ICLI_PRINTF("FLASH            : %p-%p", (void*)flash_info.start, (void*)flash_info.end);
            for ( j = 0; j < flash_info.num_block_infos; j++ ) {
                ICLI_PRINTF(", %d x 0x%x blocks",
                    flash_info.block_info[j].blocks,
                    (unsigned int)flash_info.block_info[j].block_size);
            }
            ICLI_PRINTF("\n");
        }
        i++;
    } while (ret != CYG_FLASH_ERR_INVALID);

    if ( conf_mgmt_mac_addr_get(mac, 0) >= 0 ) {
        ICLI_PRINTF("MAC Address      : %s\n", misc_mac_txt(mac, buf));
    }

    if ( vtss_restart_status_get(NULL, &status) == VTSS_RC_OK ) {
        strcpy(buf, control_system_restart_to_str(status.restart));
        buf[0] = toupper(buf[0]);
        ICLI_PRINTF("Previous Restart : %s\n", buf);
    }

    ICLI_PRINTF("\n");

    if ( system_get_config(&conf) == VTSS_OK ) {
        ICLI_PRINTF("System Contact   : %s\n", conf.sys_contact);
        ICLI_PRINTF("System Name      : %s\n", conf.sys_name);
        ICLI_PRINTF("System Location  : %s\n", conf.sys_location);
    }

    ICLI_PRINTF("System Time      : %s\n", misc_time2str(time(NULL)));
    ICLI_PRINTF("System Uptime    : %s\n", cli_time_txt(cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR));

    ICLI_PRINTF("\n");
    firmware_icli_show_version( session_id );

    for ( isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++ ) {
        if ( msg_switch_info_get(isid, &info) == VTSS_RC_OK ) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("------------------\n");
            ICLI_PRINTF("SID : %-2d\n",    topo_isid2usid(isid));
            ICLI_PRINTF("------------------\n");
            ICLI_PRINTF("Chipset ID       : VSC%-5x\n", info.info.api_inst_id);
            ICLI_PRINTF("Board Type       : %s\n",      _board_type_name(info.info.board_type));
            ICLI_PRINTF("Port Count       : %u\n",      info.info.port_cnt);
            ICLI_PRINTF("Product          : %s\n",      info.product_name);
            ICLI_PRINTF("Software Version : %s\n",      misc_software_version_txt());
            ICLI_PRINTF("Build Date       : %s\n",      misc_software_date_txt());

            code_rev = misc_software_code_revision_txt();
            if ( strlen(code_rev) ) {
                // version.c is always compiled, this file is not, so we must
                // check for whether there's something in the code revision
                // string or not. Only version.c knows about the CODE_REVISION
                // environment variable.
                ICLI_PRINTF("Code Revision    : %s\n", code_rev);
            }
        }
    }

    ICLI_PRINTF("\n");
}

