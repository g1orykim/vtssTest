/* =================================================================
 *
 *      http.h
 *
 *      Improved HTTPD server.
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2005 eCosCentric Ltd.
 * 
 * eCos is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or (at your option)
 * any later version.
 * 
 * eCos is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with eCos; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile this
 * file and link it with other works to produce a work based on this
 * file, this file does not by itself cause the resulting work to be
 * covered by the GNU General Public License. However the source code
 * for this file must still be made available in accordance with
 * section (3) of the GNU General Public License.
 * 
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 *
 * -------------------------------------------
 * ####ECOSGPLCOPYRIGHTEND####
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    Anthony Tonizzo (atonizzo@gmail.com)
 *  Contributors: 
 *  Date:         2006-06-12
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */
#ifndef __HTTP_H__
#define __HTTP_H__

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <cyg/hal/hal_tables.h>

#include <pkgconf/athttpd.h>
#include <cyg/athttpd/auth.h>

#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_TCL
#include <cyg/athttpd/jim.h>
#endif

#ifdef CYGPKG_NET_FREEBSD_INET6
#include <netinet/ip6.h>
#endif /* CYGPKG_NET_FREEBSD_INET6 */

typedef enum cyg_httpd_req_type
{
    CYG_HTTPD_METHOD_GET = 1,
    CYG_HTTPD_METHOD_HEAD = 2,
    CYG_HTTPD_METHOD_POST = 3
} cyg_httpd_req_type;

#define CYG_HTTPD_MAXURL                    128
#define CYG_HTTPD_MAXPATH                   128

#define CYG_HTTPD_MAXINBUFFER               CYGNUM_ATHTTPD_SERVER_BUFFER_SIZE
#define CYG_HTTPD_MAXOUTBUFFER              CYGNUM_ATHTTPD_SERVER_BUFFER_SIZE

#define CYG_HTTPD_DEFAULT_CGIBIN_OBJLOADER_EXTENSION ".o"
#define CYG_HTTPD_DEFAULT_CGIBIN_TCL_EXTENSION       ".tcl"

#define CYG_HTTPD_TIME_STRING_LEN                 32

#define CYG_HTTPD_STATUS_OK                      200
#define CYG_HTTPD_STATUS_MOVED_PERMANENTLY       301
#define CYG_HTTPD_STATUS_MOVED_TEMPORARILY       302
#define CYG_HTTPD_STATUS_NOT_MODIFIED            304
#define CYG_HTTPD_STATUS_BAD_REQUEST             400
#define CYG_HTTPD_STATUS_NOT_AUTHORIZED          401
#define CYG_HTTPD_STATUS_FORBIDDEN               403
#define CYG_HTTPD_STATUS_NOT_FOUND               404
#define CYG_HTTPD_STATUS_TOO_LARGE               413
#define CYG_HTTPD_STATUS_METHOD_NOT_ALLOWED      405
#define CYG_HTTPD_STATUS_SYSTEM_ERROR            500
#define CYG_HTTPD_STATUS_NOT_IMPLEMENTED         501

/* Mode flags (connection) */
#define CYG_HTTPD_MODE_CLOSE_CONN            (1 << 0)
#define CYG_HTTPD_MODE_SEND_HEADER_ONLY      (1 << 1)

/* Header flags (one-shot) */
#define CYG_HTTPD_HDR_TRANSFER_CHUNKED       (1 << 0)
#define CYG_HTTPD_HDR_ENCODING_GZIP          (1 << 1)
#define CYG_HTTPD_HDR_NO_CACHE               (1 << 2)
#define CYG_HTTPD_HDR_ETAG                   (1 << 3)
#define CYG_HTTPD_HDR_XREADONLY              (1 << 4)

#define CYG_HTTPD_HEADER_END_MARKER          "\r\n\r\n"

// This must be generated at random...
#define CYG_HTTPD_MD5_AUTH_NAME                "MD5"
#define CYG_HTTPD_MD5_AUTH_QOP                 "auth"
#define CYG_HTTPD_MD5_AUTH_OPAQUE              "0000000000000000"

#define TIME_FORMAT_RFC1123                    "%a, %d %b %Y %H:%H:%S GMT"

// Content-Type equivalents
#define CYG_HTTPD_CONTENT_TYPE_UNKNOWN          0
#define CYG_HTTPD_CONTENT_TYPE_URLENCODED       1
#define CYG_HTTPD_CONTENT_TYPE_FORMDATA         2

// When using Basic Authorization, this defines the length of the buffer for holding
// the Base64 encoded Username:Password string, excluding the terminating NULL.
#define AUTH_STORAGE_BUFFER_LENGTH (4 * (((CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_USERNAME_LEN + CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_PASSWORD_LEN + 1) + 2) / 3))

typedef struct __socket_entry
{
    cyg_int32 descriptor;
    time_t    timestamp;
    cyg_int32 sess_id;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    void      *ssl;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
} socket_entry; 

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
typedef struct __sess_entry
{
    int         is_ssl;
    cyg_int32   sess_id;
    cyg_int32   sess_descriptor;
    time_t      active_timestamp;
    time_t      absolute_timestamp;
} sess_entry; 
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

// =============================================================================
// Main HTTP structure.
// =============================================================================
#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
#define CYGOPT_NET_ATHTTPD_HTTP_SESSION_MAX_CNT     CYGPKG_NET_MAXSOCKETS
#define CYGOPT_NET_ATHTTPD_HTTPS_SESSION_MAX_CNT    CYGPKG_NET_MAXSOCKETS
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

enum cyg_http_agent_navigator {
    CYG_HTTP_AGENT_NAVIGATOR_MSIE,
    CYG_HTTP_AGENT_NAVIGATOR_FF,
    CYG_HTTP_AGENT_NAVIGATOR_CHROME,
    CYG_HTTP_AGENT_NAVIGATOR_SAFARI,
    CYG_HTTP_AGENT_NAVIGATOR_OPERA,
    CYG_HTTP_AGENT_NAVIGATOR_UNKNOWN
};

typedef struct
{
    cyg_httpd_req_type method;    
    fd_set       rfds;

    cyg_int32    host[4];

    char         url[CYG_HTTPD_MAXURL+1];
    char         args[CYGNUM_ATHTTPD_SERVER_BUFFER_SIZE+1];
    char         inbuffer[CYG_HTTPD_MAXINBUFFER+1], *header_end, *post_data;
    cyg_int32    inbuffer_len, content_len;
    char         content_type;
    cyg_int32    agent_navigator;

    // Connection status.
    cyg_uint8   mode;

    // (Output) HTTP Header flags.
    cyg_uint8   hflags;

    // Custom (output) HTTP header
    char        *custom_header;
    
    // Ouptut data.
    cyg_uint16   status_code;
    char        *mime_type;
    cyg_int32    payload_len;
    char         outbuffer[CYG_HTTPD_MAXOUTBUFFER+1];

    socket_entry sockets[CYGNUM_FILEIO_NFILE];
    cyg_int32    fdmax;
    
    // Socket handle.
    cyg_int32    client_index;

    time_t       modified_since;
    time_t       last_modified;
    
#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
    const char  *in_etag;
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
    const char  *cookies;
    sess_entry   sessions[CYGNUM_FILEIO_NFILE];
    time_t       active_sess_timeout;
    time_t       absolute_sess_timeout;
    time_t       https_active_sess_timeout;
    time_t       https_absolute_sess_timeout;
    cyg_int32    http_session_cnt;
    cyg_int32    https_session_cnt;
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_TCL
    Jim_Interp *jim_interp;
#endif    

    // This pointer points to the information about the domain that needs
    //  to be authenticated. It is only used by the function that builds the
    //  header.
    cyg_httpd_auth_table_entry *needs_auth;

    struct sockaddr_in server_conn;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    struct sockaddr_in https_server_conn;
#ifdef CYGPKG_NET_FREEBSD_INET6
    struct sockaddr_in6 https_server_conn_v6;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
    struct sockaddr_in client_conn;
#ifdef CYGPKG_NET_FREEBSD_INET6
    struct sockaddr_in6 server_conn_v6;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
} CYG_HTTPD_STATE;

extern CYG_HTTPD_STATE httpstate;

extern const char *day_of_week[];
extern const char *month_of_year[]; 

struct cyg_httpd_mime_table_entry
{
    char *extension;
    char *mime_string;
} CYG_HAL_TABLE_TYPE;

typedef struct cyg_httpd_mime_table_entry cyg_httpd_mime_table_entry;

#define CYG_HTTPD_MIME_TABLE_ENTRY(__name, __pattern, __arg) \
cyg_httpd_mime_table_entry __name CYG_HAL_TABLE_ENTRY(httpd_mime_table) = \
                                                          { __pattern, __arg } 

extern cyg_int32 debug_print;

#define cyg_httpd_header_flag(p, f)   ((p)->hflags |= (f))
#define cyg_httpd_header_custom(p, h) ((p)->custom_header = (h))

void cyg_httpd_set_home_page(cyg_int8*);
char* cyg_httpd_find_mime_string(const char*);
cyg_int32 cyg_httpd_initialize(void);
void cyg_httpd_cleanup_filename(char*);
char* cyg_httpd_parse_GET(char*);
char* cyg_httpd_parse_POST(char*);
char* cyg_httpd_parse_HEAD(char*);
void cyg_httpd_handle_method_GET(void);
void cyg_httpd_handle_method_HEAD(void);
void cyg_httpd_process_method(cyg_int32, int, cyg_int32);
void cyg_httpd_send_file(char*);
void cyg_httpd_send_error(cyg_int32);
cyg_int32 cyg_httpd_format_header(void);

#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
extern char http_ires_etag[];
#endif

/* peter, 2009/5, added in http.c
   Add a tag of "X-ReadOnly" in HTML response header to descript the web page readonly status.
   parameter of status:
   0: Not allow any action
   1: Only allow readonly */
void cyg_httpd_set_xreadonly_tag(int status);

/* peter, 2009/5, added in auth.c
   Used internally by the web interface for getting the current privilege level
   return : The current privilege level for the current request. */
int cyg_httpd_current_privilege_level(void);

#endif
