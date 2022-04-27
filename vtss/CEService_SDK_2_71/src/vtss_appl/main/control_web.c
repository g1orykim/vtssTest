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

#include "web_api.h"
#include "control_api.h"
#include "firmware_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_web_api.h"
#endif
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
#include "vtss_eth_link_oam_api.h"
#endif

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MAIN
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static const char *wreset_status = "idle";
cyg_int32 handler_wreset_status(CYG_HTTPD_STATE* p)
{
    cyg_httpd_start_chunked("html");
    cyg_httpd_write_chunked(wreset_status, strlen(wreset_status));
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}


cyg_int32 handler_misc(CYG_HTTPD_STATE* p)
{
    int rc = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC))
        return -1;
#endif

    if(p->method == CYG_HTTPD_METHOD_POST) {
        if(cyg_httpd_form_varable_find(p, "warm")) {
            // "Yes" button on restart page clicked
            cyg_httpd_ires_table_entry *reboot = cyg_httpd_find_ires("/wreset_booting.htm");
            vtss_restart_t restart_type = cyg_httpd_form_varable_find(p, "coolstart") ? VTSS_RESTART_COOL : VTSS_RESTART_WARM;
            VTSS_ASSERT(reboot != NULL);
            cyg_httpd_send_ires(reboot);
#ifdef VTSS_SW_OPTION_ETH_LINK_OAM
            /* This notification allows Link OAM to send out the dyning gasp events */
            /* This action handler makes sure that after PDU exit only the sys reboot happens */
            vtss_eth_link_oam_mgmt_sys_reboot_action_handler();
#endif
            if(vtss_switch_standalone()) {
                wreset_status = "Restarting...";
                rc = control_system_reset(TRUE, VTSS_USID_ALL, restart_type);
            } else {
                wreset_status = "Restarting stack...";
                rc = control_system_reset(FALSE, VTSS_USID_ALL, restart_type);
            }
            if (rc) {
                wreset_status = "Restart failed! System is updating by another process.";
            }
        } else if(cyg_httpd_form_varable_find(p, "altimage")) {
            cyg_httpd_ires_table_entry *reboot = cyg_httpd_find_ires("/wreset_booting.htm");
            VTSS_ASSERT(reboot != NULL);
            cyg_httpd_send_ires(reboot);
            wreset_status = "Swapping images ... ";
            if(firmware_swap_entries() == 0)
                wreset_status = "Swapping images done, rebooting!";
            else
                wreset_status = "Swapping images failed!";
        } else if(cyg_httpd_form_varable_find(p, "factory")) {
            redirect(p, "/factory_done.htm");
            control_config_reset(VTSS_USID_ALL, INIT_CMD_PARM2_FLAGS_IP);
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        cyg_httpd_end_chunked();
    }
    return -1;
}

static cyg_int32 handler_wreset(CYG_HTTPD_STATE* p)
{
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC))
        return -1;
#endif
    if(p->method == CYG_HTTPD_METHOD_GET) {
        cyg_httpd_start_chunked("html");
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(post_cb_misc, "/config/misc", handler_misc);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_wreset_status, "/config/wreset_status", handler_wreset_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_wreset, "/config/wreset", handler_wreset);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
