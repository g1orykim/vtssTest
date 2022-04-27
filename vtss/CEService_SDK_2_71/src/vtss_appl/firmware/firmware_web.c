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
#include "web_api.h"
#include "firmware_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#include "flash_mgmt_api.h"

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_FIRMWARE
#include <vtss_trace_api.h>
/* ============== */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_FIRMWARE

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_firmware_status(CYG_HTTPD_STATE* p)
{
    const char *firmware_status = firmware_status_get();
    (void)cyg_httpd_start_chunked("html");
    (void)cyg_httpd_write_chunked(firmware_status, strlen(firmware_status));
    (void)cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_firmware(CYG_HTTPD_STATE* p)
{
    form_data_t formdata[2];
    int cnt;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC))
        return -1;
#endif


    if(p->method == CYG_HTTPD_METHOD_POST) {
        if((cnt = cyg_httpd_form_parse_formdata(p, formdata, ARRSZ(formdata))) > 0) {
            int i;
            form_data_t *firmware = NULL;
            vtss_restart_t restart_type = VTSS_RESTART_WARM; // Default
            // Figure out which of the entries found in the POST request contains the firmware
            // and whether the coolstart entry exists (meaning that the checkbox is checked on the page).
            for (i = 0; i < cnt; i++) {
                if (!strcmp(formdata[i].name, "firmware")) {
                    firmware = &formdata[i];
                } else if (!strcmp(formdata[i].name, "coolstart")) {
                    restart_type = VTSS_RESTART_COOL;
                }
            }

            if (firmware) {
                unsigned long image_version;
                unsigned char *buffer;
                /* NB: We malloc and copy to ensure proper aligment of image! */
                if((buffer = VTSS_MALLOC(firmware->value_len))) {
                    memcpy(buffer, firmware->value, firmware->value_len);
                    vtss_rc result = firmware_check(buffer, firmware->value_len, FIRMWARE_TRAILER_V1_TYPE_MANAGED, 0, &image_version);
                    if(result != VTSS_OK) {
                        VTSS_FREE(buffer);
                        if(result == FIRMWARE_ERROR_INVALID)
                            redirect(p, "/upload_invalid.htm");
                        else {
                            const char *err_buf_ptr = error_txt(result);
                            send_custom_error(p, "Firmware Upload Error", err_buf_ptr, strlen(err_buf_ptr));
                        }
                    } else {
                        vtss_rc result = firmware_update_async(web_get_iolayer(WEB_CLI_IO_TYPE_FIRMWARE), buffer, firmware->value_len, restart_type);
                        if(result == VTSS_OK) {
                            firmware_status_set("Flashing, please wait...");
                            redirect(p, "/upload_flashing.htm");
                        } else {
                            /* NB: Buffer *is* freed */
                            /* Unfortunately, we cannot use cli_io_mem but must rely on the return code, because
                             * when CLI is not compiled into the build, the cli_io_mem will be empty.
                             */
                            const char *err_buf_ptr = error_txt(result);
                            send_custom_error(p, "Firmware Upload Error", err_buf_ptr, strlen(err_buf_ptr));
                        }
                    }
                } else {
                    char * err = "Allocation of firmware buffer failed";
                    T_E("%s (len %zu)", err, firmware->value_len);
                    send_custom_error(p, err, err, strlen(err));
                }
            } else {
                char *err = "Firmware not found in data";
                T_E(err);
                send_custom_error(p, err, err, strlen(err));
            }
        } else {
            cyg_httpd_send_error(CYG_HTTPD_STATUS_BAD_REQUEST);
        }
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        web_send_iolayer_output(WEB_CLI_IO_TYPE_FIRMWARE, NULL, "html");
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_sw_select(CYG_HTTPD_STATE* p)
{
    int ct = 0;
    struct fis_table_entry fis_act, fis_bak;
    const char *pri_name = "managed", *alt_name = "managed.bk";
    BOOL have_alt;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MISC))
        return -1;
#endif
    (void)cyg_httpd_start_chunked("html");

    fis_act.name[0] = '\0';
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis_act);
    if(strcmp(fis_act.name, pri_name) == 0) {
        have_alt = (flash_mgmt_fis_lookup(alt_name, &fis_bak) >= 0);
    } else    
        have_alt = FALSE;

    /* Current image info */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), 
                  "%s/%s/%s",
                  fis_act.name,
                  misc_software_version_txt(),
                  misc_software_date_txt());
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);

    if(have_alt) {
        uchar vers[256], date[256];
        firmware_scan_props(&fis_bak, vers, sizeof(vers), date, sizeof(date));
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), 
                      "|%s/%s/%s",
                      fis_bak.name,
                      vers,
                      date);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    (void)cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_firmware_status, "/config/firmware_status", handler_firmware_status);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_firmware, "/config/firmware", handler_firmware);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_sw_select, "/config/sw_select", handler_sw_select);
