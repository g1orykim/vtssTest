/* =================================================================
 *
 *      auth.c
 *
 *      Handles basic authentication.
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
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <cyg/kernel/kapi.h>           // Kernel API.
#include <cyg/hal/hal_tables.h>

#include <stdio.h>
#include <ctype.h>

#include <network.h>
#include <string.h>

#include <cyg/athttpd/http.h>
#include <cyg/athttpd/md5.h>
#include <sys/uio.h>
#include <cyg/athttpd/socket.h>


// This is a string that contains the domain that is currently authorized.
cyg_uint8 *cyg_httpd_current_authName;

CYG_HAL_TABLE_BEGIN(cyg_httpd_auth_table, httpd_auth_table );
CYG_HAL_TABLE_END(cyg_httpd_auth_table_end, httpd_auth_table );

__externC cyg_httpd_auth_table_entry cyg_httpd_auth_table[];
__externC cyg_httpd_auth_table_entry cyg_httpd_auth_table_end[];

// Variables used for authorization.
char cyg_httpd_md5_nonce[AUTH_STORAGE_BUFFER_LENGTH + 1];
char cyg_httpd_md5_digest[AUTH_STORAGE_BUFFER_LENGTH + 1];
char cyg_httpd_md5_response[AUTH_STORAGE_BUFFER_LENGTH + 1];
char cyg_httpd_md5_cnonce[AUTH_STORAGE_BUFFER_LENGTH + 1];
char cyg_httpd_md5_noncecount[9];
char cyg_httpd_md5_ha2[HASHHEXLEN+1] = {'\0'};
char cyg_httpd_md5_ha1[HASHHEXLEN+1];

char b64string[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* peter, 2009/5,
   The web server must be modified to remember the current privilege level.
   The web server (athttpd) is a single threaded server, and the authentication
   is done for each and every request, which means that it will be ok to save
   the current privilege level in a static variable "int current_privilege_level" */
static int current_privilege_level = 15;

/* peter, 2009/5,
   Used internally by the web interface for getting the current privilege level
   return : The current privilege level for the current request. */
int cyg_httpd_current_privilege_level(void)
{
    return current_privilege_level;
}

/* Authentication callback and registration */
static cyg_httpd_auth_callback_t cyg_httpd_auth_callback = NULL;
void cyg_httpd_auth_callback_register(cyg_httpd_auth_callback_t cb)
{
    cyg_httpd_auth_callback = cb;
}

cyg_httpd_auth_table_entry*
cyg_httpd_auth_entry_from_path(char *authPath)
{
    char unprotected_dir[] = "/xml";
    cyg_httpd_auth_table_entry *entry = cyg_httpd_auth_table;
    if (strncmp(unprotected_dir,
                authPath,
                strlen(unprotected_dir)) == 0)
        return (cyg_httpd_auth_table_entry *)0;                        
    
    if (strcmp(authPath, "/") != 0)
        while (entry != cyg_httpd_auth_table_end)
        {
            if (strncmp(entry->auth_dirname,
                        authPath,
                        strlen(entry->auth_dirname)) == 0)
                return entry;
            entry++;
        }
    else
        while (entry != cyg_httpd_auth_table_end)
        {
            if (strcmp(entry->auth_dirname, authPath) == 0)
                return entry;
            entry++;
        }
            
    return (cyg_httpd_auth_table_entry *)0;
}

cyg_httpd_auth_table_entry*
cyg_httpd_auth_entry_from_domain(char *authDomain)
{
    cyg_httpd_auth_table_entry *entry = cyg_httpd_auth_table;
    while (entry != cyg_httpd_auth_table_end)
    {
        if (!strncmp((const char*)authDomain, 
                       entry->auth_domainname,
                       strlen(entry->auth_domainname)))
            return entry;
        entry++;
    }
            
    return (cyg_httpd_auth_table_entry *)0;
}

cyg_int32
cyg_httpd_base64_encode(char* to, char* from, cyg_uint32 len )
{
    char     *fromp = from;
    char     *top = to;
    char      cbyte;
    char      obyte;
    cyg_int8  end[3];

    for (; len >= 3; len -= 3)
    {
        cbyte = *fromp++;
        *top++ = b64string[(int)(cbyte >> 2)];
        obyte = (cbyte << 4) & 0x30;

        cbyte = *fromp++;
        obyte |= (cbyte >> 4);        
        *top++ = b64string[(cyg_int32)obyte];
        obyte = (cbyte << 2) & 0x3C;

        cbyte = *fromp++;
        obyte |= (cbyte >> 6);        
        *top++ = b64string[(cyg_int32)obyte];
        *top++ = b64string[(cyg_int32)(cbyte & 0x3F)];
    }

    if (len)
    {
        end[0] = *fromp++;
        if (--len )
            end[1] = *fromp++; 
        else 
            end[1] = 0;
        end[2] = 0;

        cbyte = end[0];
        *top++ = b64string[(cyg_int32)(cbyte >> 2)];
        obyte = (cbyte << 4) & 0x30;

        cbyte = end[1];
        obyte |= (cbyte >> 4);
        *top++ = b64string[(cyg_int32)obyte];
        obyte = (cbyte << 2) & 0x3C;

        if (len )
            *top++ = b64string[(cyg_int32)obyte];
        else 
            *top++ = '=';
        *top++ = '=';
    }
    *top = 0;
    return top - to;
}

cyg_int32
cyg_httpd_base64_decode(char* to, char* from, cyg_uint32 len )
{
    char     *fromp = from;
    char     *top = to;
    char     *p;
    char      cbyte;
    char      obyte;
    cyg_int32 padding = 0;

    for (; len >= 4; len -= 4)
    {
        if ((cbyte = *fromp++) == '=')
            cbyte = 0;
        else
        {
            if (badchar(cbyte, p ) )
                return -1;
            cbyte = (p - b64string);
        }
        obyte = cbyte << 2;

        if ((cbyte = *fromp++) == '=')
            cbyte = 0;
        else
        {
            if (badchar(cbyte, p))
                return -1;
            cbyte = p - b64string;
        }
        obyte |= cbyte >> 4;
        *top++ = obyte;

        obyte = cbyte << 4;
        if ((cbyte = *fromp++) == '=')
        {
            cbyte = 0; 
            padding++;
        }
        else
        {
            padding = 0;
            if (badchar(cbyte, p))
                return -1;
            cbyte = p - b64string;
        }
        obyte |= cbyte >> 2;
        *top++ = obyte;

        obyte = cbyte << 6;
        if ((cbyte = *fromp++) == '=')
        {
            cbyte = 0;
            padding++;
        }
        else
        {
            padding = 0;
            if (badchar(cbyte, p))
                return -1;
            cbyte = p - b64string;
        }
        obyte |= cbyte;
        *top++ = obyte;
    }

    *top = 0;
    if (len)
        return -1;
    return (top - to) - padding;
}

cyg_httpd_auth_table_entry*
cyg_httpd_verify_auth(char* username, char* password)
{
    if (httpstate.needs_auth &&
    	(strcmp(httpstate.needs_auth->auth_username, username) == 0) &&
        (strcmp(httpstate.needs_auth->auth_password, password) == 0))
        return httpstate.needs_auth;
    else    
        return (cyg_httpd_auth_table_entry*)0;
}

// The following code is a slightly modified version of those available at the
//  end of RFC1270.
void cyg_httpd_cvthex(HASH Bin, HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++)
    {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
         else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
         else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

// Calculate H(A1) as per spec.
void
cyg_httpd_digest_calc_HA1( const char    *pszAlg,
                           const char    *pszUserName,
                           const char    *pszRealm,
                           const char    *pszPassword,
                           const char    *pszNonce,
                           const char    *pszCNonce,
                           HASHHEX  SessionKey )
{
      MD5_CTX Md5Ctx;
      HASH HA1;

      MD5Init(&Md5Ctx);
      MD5Update(&Md5Ctx, (unsigned char*)pszUserName, strlen(pszUserName));
      MD5Update(&Md5Ctx, (unsigned char*)":", 1);
      MD5Update(&Md5Ctx, (unsigned char*)pszRealm, strlen(pszRealm));
      MD5Update(&Md5Ctx, (unsigned char*)":", 1);
      MD5Update(&Md5Ctx, (unsigned char*)pszPassword, strlen(pszPassword));
      MD5Final((unsigned char*)HA1, &Md5Ctx);
      if (strcmp(pszAlg, "md5-sess") == 0) 
      {
          MD5Init(&Md5Ctx);
          MD5Update(&Md5Ctx, (unsigned char*)HA1, HASHLEN);
          MD5Update(&Md5Ctx, (unsigned char*)":", 1);
          MD5Update(&Md5Ctx, (unsigned char*)pszNonce, strlen(pszNonce));
          MD5Update(&Md5Ctx, (unsigned char*)":", 1);
          MD5Update(&Md5Ctx, (unsigned char*)pszCNonce, strlen(pszCNonce));
          MD5Final((unsigned char*)HA1, &Md5Ctx);
      };
      cyg_httpd_cvthex(HA1, SessionKey);
};

// Calculate request-digest/response-digest as per HTTP Digest spec.
void
cyg_httpd_digest_calc_response(HASHHEX  HA1,           
                               char    *pszNonce,       
                               char    *pszNonceCount,  
                               char    *pszCNonce,      
                               char    *pszQop,         
                               char    *pszMethod,      
                               char    *pszDigestUri,   
                               HASHHEX  HEntity,       
                               HASHHEX  Response)
{
    MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    // Calculate H(A2).
    MD5Init(&Md5Ctx);
    MD5Update(&Md5Ctx, (unsigned char*)pszMethod, strlen(pszMethod));
    MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    MD5Update(&Md5Ctx, (unsigned char*)pszDigestUri, strlen(pszDigestUri));
    if (strcmp(pszQop, "auth-int") == 0) {
        MD5Update(&Md5Ctx, (unsigned char*)":", 1);
        MD5Update(&Md5Ctx, (unsigned char*)HEntity, HASHHEXLEN);
    };
    MD5Final((unsigned char*)HA2, &Md5Ctx);
    cyg_httpd_cvthex(HA2, HA2Hex);

    // calculate response
    MD5Init(&Md5Ctx);
    MD5Update(&Md5Ctx, (unsigned char*)HA1, HASHHEXLEN);
    MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    MD5Update(&Md5Ctx, (unsigned char*)pszNonce, strlen(pszNonce));
    MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    if (*pszQop) 
    {
        MD5Update(&Md5Ctx, (unsigned char*)pszNonceCount, strlen(pszNonceCount));
        MD5Update(&Md5Ctx, (unsigned char*)":", 1);
        MD5Update(&Md5Ctx, (unsigned char*)pszCNonce, strlen(pszCNonce));
        MD5Update(&Md5Ctx, (unsigned char*)":", 1);
        MD5Update(&Md5Ctx, (unsigned char*)pszQop, strlen(pszQop));
        MD5Update(&Md5Ctx, (unsigned char*)":", 1);
    };
    MD5Update(&Md5Ctx, (unsigned char*)HA2Hex, HASHHEXLEN);
    MD5Final((unsigned char*)RespHash, &Md5Ctx);
    cyg_httpd_cvthex(RespHash, Response);
};

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
static int
cyg_httpd_a16toi(char ch) {
  if(ch >= '0' && ch <= '9') {
    return ch - '0';
  }

  return toupper(ch) - 'A' + 10;
}

/* Return 0: success, 1: fail */
static cyg_int32
cyg_httpd_unescape(const char *from, char *to, ssize_t from_len, ssize_t to_len)
{
  ssize_t from_i = 0, to_i = 0;
  char ch;

  if(to_len == 0) {
      return 1;
  }

  while(from_i < from_len) {
    if(from[from_i] == '%') {
      // Check if there are chars enough left in @from to complete the conversion
      if(from_i + 2 >= from_len) {
        // There aren't.
        return 1;
      }

      // Check if the two next chars are hexadecimal.
      if(!isxdigit(from[from_i + 1])  || !isxdigit(from[from_i + 2])) {
        return 1;
      }

      ch = 16 * cyg_httpd_a16toi(from[from_i + 1])  + cyg_httpd_a16toi(from[from_i + 2]);

      from_i += 3;
    } else if(from[from_i] == '+') {
      ch = ' '; // Special handling of '+'. The browser converts spaces to '+' and '+' to "%2B" (tested with FF2.0, IE6.0 and IE7.0).
      from_i++;
    } else {
      ch = from[from_i++];
    }

    to[to_i++] = ch;
    if(to_i == to_len) {
      // Not room for trailing '\0'
      return 1;
    }
  }

  to[to_i] = '\0';
  return 0;
}
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

/* Return null: Auth success, none zero: Auth failed */
cyg_httpd_auth_table_entry*
cyg_httpd_is_authenticated(char* fname)
{
    // Let's check if the directory access needs authorization. The 
    //  authentication is done on the directory name.
    cyg_httpd_auth_table_entry* entry =
                                cyg_httpd_auth_entry_from_path(fname);
 
   if (entry != 0)
    {
#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
        if (cyg_httpd_auth_callback) {
            // We use "deviceid" to save username and password. The format is: username:password
            char    *ptr;
            char    username[CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_USERNAME_LEN + 1];
            char    passwd[CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_PASSWORD_LEN + 1];
            int     str_idx = 0;
            ssize_t id_len;
        
            memset(username, 0, sizeof(username));
            memset(passwd, 0, sizeof(passwd));
            cyg_httpd_md5_response[0] = '\0';

            if (httpstate.cookies) {
                /* lookup deviceid */
                ptr = cyg_httpd_cookie_varable_find(httpstate.args, "deviceid", &id_len);
                if (ptr == NULL) {
                    ptr = cyg_httpd_cookie_varable_find(httpstate.cookies, "deviceid", &id_len);
                }

                if (ptr == NULL || id_len > AUTH_STORAGE_BUFFER_LENGTH) {
                    httpstate.needs_auth = entry;
                    return entry;
                }

                /* decode deviceid */
                (void) cyg_httpd_unescape(ptr, cyg_httpd_md5_response, id_len, sizeof(cyg_httpd_md5_response));
                cyg_httpd_base64_decode(cyg_httpd_md5_response, ptr, id_len);

                /* Get username */
                str_idx = 0;
                ptr = cyg_httpd_md5_response;
                while (*ptr != ':' && *ptr != '\0' && str_idx < id_len) {
                    if (str_idx < CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_USERNAME_LEN)
                        username[str_idx++] = *ptr++;
                    else
                        break;
                }
                username[str_idx] = '\0';

                /* Get password */
                str_idx = 0;
                ptr++;
                while (*ptr != '\0' && str_idx < id_len) {
                    if (str_idx < CYGOPT_NET_ATHTTPD_BASIC_AUTH_MAX_PASSWORD_LEN)
                        passwd[str_idx++] = *ptr++;
                    else
                        break;
                }
                passwd[str_idx] = '\0';
            }
            if (cyg_httpd_auth_callback(username, passwd, &current_privilege_level)) {
                httpstate.needs_auth = entry;
                return entry;
            }
        }
#else
        if (entry->auth_mode == CYG_HTTPD_AUTH_BASIC)
        {
            cyg_httpd_base64_decode(cyg_httpd_md5_response,
                                    cyg_httpd_md5_digest,
                                    strlen(cyg_httpd_md5_digest));
            /* Fixed BZ#12390 - Unable to access the switch when password is configured with ":" symbol
               RFC 1945 / 11.1 Basic Authentication Scheme (http://tools.ietf.org/html/rfc1945) says:
               userid-password   = [ token ] ":" *TEXT */
            char *extension = index(cyg_httpd_md5_response, ':');
            if (extension == NULL)
            {
                httpstate.needs_auth = entry;
                return entry;
            }    
            else
            {    
                *extension = '\0'; // Crypto now has the username.
                
                // In the case of a 'Basic" authentication, the HTTP header
                //  did not return to us the domain name that we sent when we
                //  challenged the request: The only things that are returned 
                //  are the username:password duo. In this case I will just 
                //  compare the entry's username/password to those read from 
                //  the header.

                if (cyg_httpd_auth_callback) {
                    if (cyg_httpd_auth_callback(cyg_httpd_md5_response, ++extension, &current_privilege_level)) {
                        httpstate.needs_auth = entry;
                        return entry;
                    }
                }
                else {
                    if ((strcmp(entry->auth_username, 
                                cyg_httpd_md5_response) != 0) ||
                        (strcmp(entry->auth_password, 
                                ++extension) != 0)) {
                        httpstate.needs_auth = entry;
                        return entry;
                    }
                }
            }    
        }
        else
        {
            char *cyg_httpd_md5_method;
            
            switch (httpstate.method)
            {
            case CYG_HTTPD_METHOD_GET:
                cyg_httpd_md5_method = "GET";
                break;
            case CYG_HTTPD_METHOD_POST:
                cyg_httpd_md5_method = "POST";
                break;
            default:
                cyg_httpd_md5_method = "HEAD";
                break;
            }    
            cyg_httpd_digest_calc_HA1(CYG_HTTPD_MD5_AUTH_NAME,
                                      entry->auth_username,
                                      entry->auth_domainname,
                                      entry->auth_password,
                                      cyg_httpd_md5_nonce,
                                      cyg_httpd_md5_cnonce,
                                      cyg_httpd_md5_ha1);
            cyg_httpd_digest_calc_response(cyg_httpd_md5_ha1,
                                           cyg_httpd_md5_nonce,
                                           cyg_httpd_md5_noncecount,
                                           cyg_httpd_md5_cnonce,
                                           CYG_HTTPD_MD5_AUTH_QOP,
                                           cyg_httpd_md5_method,
                                           httpstate.url,
                                           cyg_httpd_md5_ha2,
                                           cyg_httpd_md5_digest);
            if (strcmp(cyg_httpd_md5_response, cyg_httpd_md5_digest) != 0) {
                httpstate.needs_auth = entry;
                return entry;
            }
        }
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */
    }
    // No need for authentication...
    return (cyg_httpd_auth_table_entry*)0;
}

char*
cyg_httpd_digest_data(char *dest, char *src)
{
    int exit = 0;
    while (exit == 0)
    {
        switch (*src )
        {
        case '\r':
        case '\n':
            *dest = '\0';
            exit = 1;
            break;
        case ' ':
            src++;
            *dest = '\0';
            exit = 1;
            break;
        case '"':
        case ',':
            src++;
            break;
        default:
            *dest++ = *src++;
        }    
    }
    return src;
}    

// Skips through fields we do not need.
char*
cyg_httpd_digest_skip(char *p)
{
    if (*p == '"')
    {
        p++;
        while ((*p != '"') && (*p != '\n'))
            p++;
        p++;
        if (*p == ',')
            p++;
        if (*p == ' ')
            p++;
        if (*p == '\n')
            p++;
    }
    else        
    {    
        while ((*p != ' ') && (*p != '\n'))
            p++;
        if (*p == ',')
            p++;
        if (*p == ' ')
            p++;
        if (*p == '\n')
            p++;
    }        
    return p;
}
