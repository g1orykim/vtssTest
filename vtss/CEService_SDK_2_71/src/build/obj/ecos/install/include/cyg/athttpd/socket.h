/* =================================================================
 *
 *      socket.h
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
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <pkgconf/system.h>

#define CYG_HTTPD_SOCKET_IDLE_TIMEOUT          300 // In seconds

extern cyg_thread   cyg_httpd_thread_object;
extern cyg_handle_t cyg_httpd_thread_handle;
extern cyg_uint8    cyg_httpd_thread_stack[];


void cyg_httpd_daemon(cyg_addrword_t);
void cyg_httpd_start(void);
void cyg_httpd_create_std_header(char *extension, int);
ssize_t cyg_httpd_write(char*, int);
ssize_t cyg_httpd_start_chunked(const char*);
ssize_t cyg_httpd_write_chunked(const char*, int);
void cyg_httpd_end_chunked(void);
cyg_int32 cyg_httpd_socket_setup(void);
cyg_int32 cyg_httpd_socket_handle_request(void);
ssize_t cyg_httpd_writev(cyg_iovec*, int );

/* peter, 2008/8, add for https */
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
#define CYG_HTTPS_NEW_CERT_MAX_LEN    16

typedef struct {
    char country[CYG_HTTPS_NEW_CERT_MAX_LEN];
    char state[CYG_HTTPS_NEW_CERT_MAX_LEN];
    char locality[CYG_HTTPS_NEW_CERT_MAX_LEN];
    char organization[CYG_HTTPS_NEW_CERT_MAX_LEN];
    char mail[CYG_HTTPS_NEW_CERT_MAX_LEN];
    char common[CYG_HTTPS_NEW_CERT_MAX_LEN];
} cyg_https_new_cert_info_t;

typedef struct {
    int sess_accept;	            /* SSL new accept - started             */
    int sess_accept_renegotiate;    /* SSL reneg - requested                */
    int sess_accept_good;	        /* SSL accept/reneg - finished          */
    int sess_miss;		            /* session lookup misses                */
    int sess_timeout;	            /* reuse attempt on timeouted session   */
    int sess_cache_full;	        /* session removed due to full cache    */
    int sess_hit;		            /* session reuse actually done          */
    int sess_cb_hit;	            /* session-id that was not
					                 * in the cache was
					                 * passed back via the callback.  This
					                 * indicates that the application is
					                 * supplying session-id's from other
					                 * processes - spooky :-)               */
} cyg_https_stats_t;

void cyg_https_set_new_cert_info(cyg_https_new_cert_info_t *new_cert_info);
int cyg_https_generate_cert(char *server_cert, char *server_private_key, char *server_pass_phrase, cyg_int32 is_rsa);
void cyg_https_set_mode(int mode);
void cyg_https_shutdown(void);

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_start(char *server_cert, char *server_private_key, char *server_pass_phrase, char *dh_parameters);

void cyg_https_set_redirect(int redirect_flag);
int cyg_https_get_cert_info(char *server_cert, char *cert_info);
void cyg_https_get_sess_info(char *sess_info, ssize_t sess_info_len);
void cyg_https_get_stats(cyg_https_stats_t *counter);
void cyg_https_set_session_timeout(long active_sess_timeout, long absolute_sess_timeout);

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_check_cert(char *cert, char *private_key, char *pass_phrase);

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_check_dh_parameters(char *dh_parameters);
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
char *cyg_httpd_cookie_varable_find(const char *ref_addr, char *name, ssize_t *len);
void cyg_http_release_session(int is_ssl, cyg_int32 sess_id);
void cyg_http_set_session_timeout(long active_sess_timeout, long absolute_sess_timeout);
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

#endif
