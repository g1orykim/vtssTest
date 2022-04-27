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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_WEB_API_H_
#define _VTSS_WEB_API_H_

#if VTSS_SWITCH_MANAGED

#include "main.h" /* For vtss_usid_t */
#include <network.h>

#include <cyg/io/file.h>        /* iovec */
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>
#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
#include <cyg/crc/crc.h>
#endif
#ifdef VTSS_SW_OPTION_CLI
#include "cli_api.h"
#endif /* VTSS_SW_OPTION_CLI */
#include "misc_api.h"

#define STACK_ERR_URL "/stack_error.htm"

#define MAX_FORM_NAME_LEN       32

typedef struct {
    char        name[MAX_FORM_NAME_LEN];
    const char  *value;
    size_t      value_len;
} form_data_t;

#define HTML_VAR_LEN   16

/* Web dummy CLI IO types */
enum {
    WEB_CLI_IO_TYPE_BOOT_LOADER,
    WEB_CLI_IO_TYPE_FIRMWARE,
    WEB_CLI_IO_TYPE_PING,
    WEB_CLI_IO_MAX = 8
};

extern char var_SelectQceId[];
extern char var_SelectQclId[];
extern char var_bool[];
extern char var_clear[];
extern char var_clock_inst[];
extern char var_Flush[];
extern char var_dyn_get_next_entry[];
extern char var_dyn_num_of_ent[];
extern char var_dyn_start_mac_addr[];
extern char var_dyn_start_vid[];
extern char var_eps[];
extern char var_evc[];
extern char var_mep[];
extern char var_qceConfigFlag[];
extern char var_server[];
extern char var_syslogEntryNum[];
extern char var_syslogFlag[];
extern char var_syslogLevel[];
extern char var_syslogStartId[];
extern char var_uni[];
extern char var_uport[];
extern char var_vid[];
extern char var_erps[];

/*START: Table definition for Config.js */
struct lib_config_js_t {
    size_t (*module_fun)(char **, char **, size_t *);
} CYG_HAL_TABLE_TYPE;


/* Entry definition rule for dynamically created JS lib table */
#define WEB_JS_LIB_TAB_ENTRY_DECL(_mf_) \
    struct lib_config_js_t _web_js__lib_tab_##_mf_ CYG_HAL_TABLE_QUALIFIED_ENTRY(web_lib_config_js_table, _mf_)

#define web_lib_config_js_tab_entry(_mf_) \
    WEB_JS_LIB_TAB_ENTRY_DECL(_mf_) = {_mf_}
/*END : Table definition for Config.js */

/*START:Table definition for fileter.css */
struct lib_filter_css_t {
        size_t (*module_fun)(char **, char **, size_t *);
} CYG_HAL_TABLE_TYPE;

/* Entry definition rule for dynamically created Filter CSS lib table */
#define WEB_CSS_LIB_TAB_ENTRY_DECL(_mf_) \
        struct lib_filter_css_t _web_css__lib_tab_##_mf_ CYG_HAL_TABLE_QUALIFIED_ENTRY(web_lib_filter_css_table, _mf_)

#define web_lib_filter_css_tab_entry(_mf_) \
        WEB_CSS_LIB_TAB_ENTRY_DECL(_mf_) = {_mf_}
/*END : Table definition for filter.css */

vtss_usid_t web_retreive_request_sid(CYG_HTTPD_STATE* p);

/* Initialize module */
vtss_rc web_init(vtss_init_data_t *data);

#ifdef VTSS_SW_OPTION_CLI
/* Pseudo HTTP CLI iolayer */
cli_iolayer_t *web_get_iolayer(int web_io_type);

/* When web_io_type is equal to WEB_CLI_IO_TYPE_BOOT_LOADER or WEB_CLI_IO_TYPE_FIRMWARE,
   the parameter of "io" should be NULL.
   When web_io_type is equal to WEB_CLI_IO_TYPE_PING,
   the parameter of "io" should be specific memory address.
  */
void web_send_iolayer_output(int web_io_type, cli_iolayer_t *io, const char *mimetype);
#endif /* VTSS_SW_OPTION_CLI */

/* Overrideable */
void stat_portstate_switch(CYG_HTTPD_STATE* p, vtss_usid_t usid, vtss_isid_t isid);

BOOL
redirectUnmanagedOrInvalid(CYG_HTTPD_STATE* p, vtss_isid_t isid);
void
redirect(CYG_HTTPD_STATE* p, const char *to);
void
send_custom_error(CYG_HTTPD_STATE *p,
                  const char      *title,
                  const char      *errtxt,
                  size_t           errtxt_len);

#define CHECK_FORMAT(strix, checkix) CYGBLD_ATTRIB_PRINTF_FORMAT(strix, checkix)

#if defined(VTSS_FEATURE_ACL_V2)
/* The parameter of 'idx' start from 1, it means the first matched.
   When the value is 2, it means the seconds matched and so on. */
cyg_bool
cyg_httpd_form_multi_varable_int(CYG_HTTPD_STATE* p, const char *name, int *pVal, int idx);
#endif /* VTSS_FEATURE_ACL_V2 */
const char *
cyg_httpd_form_varable_string(CYG_HTTPD_STATE* p, const char *name, size_t *pLen);
vtss_isid_t web_retrieve_request_sid(CYG_HTTPD_STATE* p);
cyg_bool
cyg_httpd_form_varable_int(CYG_HTTPD_STATE* p, const char *name, int *pVal);
cyg_bool
cyg_httpd_form_varable_uint64(CYG_HTTPD_STATE* p, const char *name, u64 *pVal);
cyg_bool
cyg_httpd_form_varable_ipv4(CYG_HTTPD_STATE* p, const char *name, vtss_ipv4_t *pVal);
cyg_bool
cyg_httpd_form_varable_ipv6(CYG_HTTPD_STATE* p, const char *name, vtss_ipv6_t *pVal);

int cgi_escape(const char *from, char *to);
int cgi_escape_n(const char *from, char *to, size_t maxlen);
BOOL cgi_unescape(const char *from, char *to, uint from_len, uint to_len);
int cgi_text_str_to_ascii_str(const char *from, char *to, uint from_len, uint to_len);
BOOL cgi_ascii_str_to_text_str(const char *from, char *to, uint from_len, uint to_len);

/* HACK! (type converting macro - pval pointer) */
#define cyg_httpd_form_varable_long_int(p, name, pval) ({               \
            long _arg;                                                  \
            cyg_bool ret = _cyg_httpd_form_varable_long_int(p, name, &_arg); \
            if(ret) { *(pval) = (u32) _arg; }                           \
            ret;                                                        \
        })

/* Inner function */
cyg_bool
_cyg_httpd_form_varable_long_int(CYG_HTTPD_STATE* p, const char *name, long *pVal);

const char *
cyg_httpd_form_varable_find(CYG_HTTPD_STATE* p, const char *name);
char *
cyg_httpd_form_varable_strdup(CYG_HTTPD_STATE* p, const char *fmt, ...) CHECK_FORMAT(2, 3);
cyg_bool
cyg_httpd_form_variable_int_fmt(CYG_HTTPD_STATE* p, int *pVal, const char *fmt, ...) CHECK_FORMAT(3, 4);
cyg_bool
cyg_httpd_form_variable_check_fmt(CYG_HTTPD_STATE* p, const char *fmt, ...) CHECK_FORMAT(2, 3);
const char *
cyg_httpd_form_variable_str_fmt(CYG_HTTPD_STATE* p, size_t *pLen, const char *fmt, ...) CHECK_FORMAT(3, 4);
cyg_bool
cyg_httpd_form_varable_hex(CYG_HTTPD_STATE* p, const char *name, ulong *pVal);
int httpd_form_get_value_int(CYG_HTTPD_STATE* p, const char form_name[255],int min_value,int max_value);
cyg_bool
cyg_httpd_form_varable_mac(CYG_HTTPD_STATE* p, const char *name, uchar pVal[6]);
int
cyg_httpd_form_parse_formdata(CYG_HTTPD_STATE* p, form_data_t *formdata, int maxdata);
cyg_bool
cyg_httpd_form_variable_long_int_fmt(CYG_HTTPD_STATE* p, ulong *pVal, const char *fmt, ...) CHECK_FORMAT(3, 4);
cyg_bool
cyg_httpd_form_variable_mac(CYG_HTTPD_STATE* p, const char *name, vtss_mac_t *mac);
cyg_bool
cyg_httpd_str_to_hex(const char *str_p, ulong *hex_value_p);
cyg_bool
cyg_httpd_form_varable_oui(CYG_HTTPD_STATE* p, const char *name, uchar pVal[3]);
#endif
size_t webCommonBufferHandler(char **base, char **offset, size_t *length, const char *buff);
int web_get_method(CYG_HTTPD_STATE *p, int module_id);

#endif /* _VTSS_WEB_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
