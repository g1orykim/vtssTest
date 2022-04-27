/* =================================================================
 *
 *      auth.h
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
#ifndef __AUTH_H__
#define __AUTH_H__

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <cyg/hal/hal_tables.h>

#define badchar(c,p) (!(p = memchr(b64string, c, 64)))

extern cyg_uint8 *cyg_httpd_current_authName;

typedef enum cyg_httpd_auth_type
{
    CYG_HTTPD_AUTH_BASIC = 0, 
    CYG_HTTPD_AUTH_DIGEST = 1
} cyg_httpd_auth_type;

// Maximum user and password lengths excluding terminating NULL.
#define CYG_HTTPD_AUTH_MAX_USR_LEN 32
#define CYG_HTTPD_AUTH_MAX_PW_LEN  32

// It must be stressed that the auth_dirname field is the directory name
//  that will be requested by the web server, and _not_ the absolute name
//  in the eCos file system. 
// Lets' make an example. Let's say that the files of your web site reside in 
//  the directory '/fs/jffs2/html'. Your CYG_HTTPD_DEFAULT_PATH will likely be
//  set to '/fs/jffs2/' and the CYG_HTTPD_DEFAULT_HTML_DIR will likely be
//  set to 'html/'. The web server will then request a file 
//  (i.e. 'images/foo.jpg') whose absolute location in the eCos file system is
//  '/fs/jffs2/html/images/foo.jpg', and if we want to protect the 'images/'
//  directory with some kind of authentication, the auth_dirname must be 
//  pointing to a string such as "images/".
struct cyg_httpd_auth_table_entry
{
    const char           *auth_dirname;
    const char           *auth_domainname;
    const char           *auth_username;
    const char           *auth_password;
    cyg_httpd_auth_type  auth_mode;
} CYG_HAL_TABLE_TYPE;

typedef struct cyg_httpd_auth_table_entry cyg_httpd_auth_table_entry;
#define CYG_HTTPD_AUTH_TABLE_ENTRY( __name, __path, __domain, __un, __pw, __mode )  \
 cyg_httpd_auth_table_entry __name CYG_HAL_TABLE_ENTRY( httpd_auth_table ) =  \
                             { __path, __domain, __un, __pw, __mode } 

cyg_int32 cyg_httpd_base64_encode(char*, char*, cyg_uint32 );
cyg_int32 cyg_httpd_base64_decode(char*, char*, cyg_uint32 );
cyg_httpd_auth_table_entry* cyg_httpd_auth_entry_from_path(char *);
cyg_httpd_auth_table_entry* cyg_httpd_auth_entry_from_domain(char *);
cyg_httpd_auth_table_entry* cyg_httpd_verify_auth(char*, char*);
cyg_httpd_auth_table_entry* cyg_httpd_is_authenticated(char*);
char* cyg_httpd_digest_data(char *, char *);
char* cyg_httpd_digest_skip(char *);

/**
 * \brief Authentication callback (cyg_httpd_auth_callback_t)
 * This function (if installed) will be called on every request from a web client (browser).
 * \param username [IN] The username.
 * \param password [IN] The password.
 * \param userlevel [OUT] The userlevel for this username/password combination.
 *
 * \return : 0 on success, 1 on error
 */
typedef int (*cyg_httpd_auth_callback_t)(char *username, char *password, int *userlevel);
void cyg_httpd_auth_callback_register(cyg_httpd_auth_callback_t cb);

// The following code is a slightly modified version of those available at the
//  end of RFC1270.
#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

//extern char cyg_httpd_md5_username[];
//extern char cyg_httpd_md5_opaque[];
//extern char cyg_httpd_md5_alg[];
//extern char cyg_httpd_md5_realm[];
extern char cyg_httpd_md5_nonce[];
extern char cyg_httpd_md5_cnonce[];
extern char cyg_httpd_md5_digest[];
extern char cyg_httpd_md5_qop[];
extern char cyg_httpd_md5_response[];
extern char cyg_httpd_md5_noncecount[];
extern char cyg_httpd_md5_ha2[];
extern char cyg_httpd_md5_ha1[];


// Calculate H(A1) as per HTTP Digest spec.
void cyg_httpd_digest_calc_HA1(const char *,
                               const char *,
                               const char *,
                               const char *,
                               const char *,
                               const char *,
                               HASHHEX);

// Calculate request-digest/response-digest as per HTTP Digest spec.
void cyg_httpd_digest_calc_response(HASHHEX,           
                                    char *,
                                    char *,
                                    char *,
                                    char *,
                                    char *,
                                    char *,
                                    HASHHEX,
                                    HASHHEX);
#endif // __AUTH_H__
