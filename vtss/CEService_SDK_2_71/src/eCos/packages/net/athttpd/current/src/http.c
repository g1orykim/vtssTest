/* =================================================================
 *
 *      http.c
 *
 *      Handles the client requests.
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
 *  Contributors: Sergei Gavrikov (w3sg@SoftHome.net)
 *                Lars Povlsen    (lpovlsen@vitesse.com)
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
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#include <time.h>

#include <cyg/hal/hal_tables.h>
#include <cyg/fileio/fileio.h>
#include <stdio.h>                     // sprintf().
#include <stdlib.h>

#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_OBJLOADER
#include <cyg/objloader/elf.h>
#include <cyg/objloader/objelf.h>
#endif

#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/handler.h>
#include <cyg/athttpd/forms.h>

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
#include <openssl/ssl.h>
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

cyg_int32 debug_print = 0;
static char http_readonly[8];

const char *day_of_week[7] = {"Sun", "Mon", "Tue", "Wed", 
                                 "Thu", "Fri", "Sat"};
const char *day_of_week_completed_name[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", 
                                             "Thursday", "Friday", "Saturday"};
const char *month_of_year[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
const char *home_pages[] = {"login.html",   "login.htm",
                            "index.html",   "index.htm",
                            "default.html", "home.html"};
#else
const char *home_pages[] = {"index.html",   "index.htm",
                            "default.html", "home.html"};
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

CYG_HAL_TABLE_BEGIN(cyg_httpd_mime_table, httpd_mime_table);
CYG_HAL_TABLE_END(cyg_httpd_mime_table_end, httpd_mime_table);

__externC cyg_httpd_mime_table_entry cyg_httpd_mime_table[];
__externC cyg_httpd_mime_table_entry cyg_httpd_mime_table_end[];

// Standard handlers added by default. Correspond to the most used extensions.
// The user can add his/her own later.
CYG_HTTPD_MIME_TABLE_ENTRY(hal_htm_entry, "htm",
                                       "text/html; charset=iso-8859-1");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_html_entry, "html", 
                                       "text/html; charset=iso-8859-1");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_gif_entry, "gif", "image/gif");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_jpg_entry, "jpg", "image/jpg");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_png_entry, "png", "image/png");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_css_entry, "css", "text/css; charset=iso-8859-1");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_js_entry, "js", "application/x-javascript; charset=iso-8859-1");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_xml_entry, "xml", "text/xml");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_svg_entry, "svg", "image/svg+xml");
CYG_HTTPD_MIME_TABLE_ENTRY(hal_json_entry, "json", "application/json");

static cyg_halbool
cyg_httpd_has_clock(void)
{
    static cyg_halbool has_clock = 0;
    if(has_clock)
        return true;
    has_clock = time(NULL) > ((time_t) 36 * 365 * 24 * 60 * 60);/* Fri Dec 23 00:00:00 2005 */
    return has_clock;
}

void 
cyg_httpd_send_error(cyg_int32 err_type)
{
    httpstate.status_code = err_type;

    // Errors pages close the socket and are never cached.
    httpstate.hflags |= CYG_HTTPD_HDR_NO_CACHE;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    diag_printf("Sending error: %d\n", err_type);
#endif    

#ifdef CYGOPT_NET_ATHTTPD_USE_FS
    // Check if the user has defines his own error pages.
    struct stat sp;
    char file_name[CYG_HTTPD_MAXPATH];
    strcpy(file_name, CYGDAT_NET_ATHTTPD_SERVEROPT_ROOTDIR);
    if (file_name[strlen(file_name)-1] != '/')
        strcat(file_name, "/");
    strcat(file_name, CYGDAT_NET_ATHTTPD_SERVEROPT_ERRORDIR);
    if (file_name[strlen(file_name)-1] != '/')
        strcat(file_name, "/");
    sprintf(file_name + strlen(file_name), "error_%d.html", err_type);
    cyg_httpd_cleanup_filename(file_name);
    cyg_int32 rc = stat(file_name, &sp);
    if (rc == 0)
    {
        char *extension = rindex(file_name, '.');
        if (extension == NULL)
            // No extension in the file name.
            httpstate.mime_type = 0;
        else
            httpstate.mime_type = cyg_httpd_find_mime_string(++extension);

        httpstate.payload_len  = sp.st_size;
        cyg_int32 header_length = cyg_httpd_format_header();
        cyg_httpd_write(httpstate.outbuffer, header_length);
    
        // File found.
        FILE *fp = fopen(file_name, "r");
        if(fp == NULL)
            return;

        ssize_t payload_size = fread(httpstate.outbuffer, 
                                     1, 
                                     CYG_HTTPD_MAXOUTBUFFER, 
                                     fp);
        while (payload_size > 0)
        {
            ssize_t bytes_written = cyg_httpd_write_chunked(httpstate.outbuffer, 
                                                            payload_size);
            if (bytes_written != payload_size)
                break;

            payload_size = fread(httpstate.outbuffer, 
                                 1, 
                                 CYG_HTTPD_MAXOUTBUFFER, 
                                 fp);
        }

        fclose(fp);
        return;
    }
#endif    
    
    // If no file has been defined, send a simple notification. We use inbuffer
    //  for temporary storage of the error messages, so that we know the length
    //  of the frame prior to building the header, thus avoiding the use of 
    //  chunked frames.
    strcpy(httpstate.inbuffer, 
           "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n");
    switch (err_type)
    {
    case CYG_HTTPD_STATUS_MOVED_PERMANENTLY:
        strcat(httpstate.inbuffer, 
               "<html><head><title>301 Moved Permanently</title></head>\r\n"
               "<body><h1>Moved Permanently</h1>\r\n"
               "<p>The document has moved <a href=\"" );
        strcat(httpstate.inbuffer, httpstate.url);
        strcat(httpstate.inbuffer, "\">here</a>.\r\n");
        break;
    case CYG_HTTPD_STATUS_MOVED_TEMPORARILY:
        strcat(httpstate.inbuffer, 
               "<html><head><title>302 Found</title></head>\r\n"
               "<body><h1>Redirect</h1>\r\n"
               "<p>Please continue <a href=\"" );
        strcat(httpstate.inbuffer, httpstate.url);
        strcat(httpstate.inbuffer, "\">here</a>.\r\n");
        break;
    case CYG_HTTPD_STATUS_NOT_AUTHORIZED:
        strcat(httpstate.inbuffer, 
               "<html><head><title>401 Not Authorized</title></head>\r\n");
        strcat(httpstate.inbuffer, 
               "<p>Authorization required to access this URL.</p>");
        break;    
    case CYG_HTTPD_STATUS_FORBIDDEN:
        strcat(httpstate.inbuffer, 
               "<html><head><title>403 Forbidden</title></head>\r\n");
        strcat(httpstate.inbuffer, 
               "<p>Request does not allow.</p>");
        break;
    case CYG_HTTPD_STATUS_NOT_MODIFIED:
        httpstate.mode |= CYG_HTTPD_MODE_SEND_HEADER_ONLY;
        break;    
    case CYG_HTTPD_STATUS_NOT_FOUND:
        strcat(httpstate.inbuffer, 
               "<html><head><title>404 Not Found</title></head>\r\n");
        sprintf(httpstate.inbuffer + strlen(httpstate.inbuffer),
                "<p>The requested URL: %s was not found on"
                " this server</p>",
                httpstate.url);
        break;
    case CYG_HTTPD_STATUS_SYSTEM_ERROR:
        strcat(httpstate.inbuffer, 
               "<html><head><title>500 Server Error</title></head>\r\n");
        strcat(httpstate.inbuffer, 
               "<p>The server encountered an unexpected condition that "
               "prevented it from fulfilling the request by the client</p>");
        break;
    case CYG_HTTPD_STATUS_NOT_IMPLEMENTED:
        strcat(httpstate.inbuffer, 
               "<html><head><title>501 Not Implemented</title></head>\r\n");
        strcat(httpstate.inbuffer, 
               "<p>The method requested is not implemented</p>");
        break;
    case CYG_HTTPD_STATUS_TOO_LARGE:
        sprintf(httpstate.inbuffer, 
                "<html>"
                "<head><title>413 Request Entity Too Large</title></head>"
                "\r\n"
                "<p>The server cannot accept requests larger than %d bytes.</p>", 
                CYGNUM_ATHTTPD_SERVER_MAX_POST);
        break;
    default:
        strcat(httpstate.inbuffer, 
               "<html><head><title>400 Bad Request</title></head>\r\n");
        strcat(httpstate.inbuffer, 
               "<p>Bad request</p>");
        break;
    }
    
    sprintf(httpstate.inbuffer + strlen(httpstate.inbuffer),
            "</p><hr><address>%s at %d.%d.%d.%d Port %d</address>\r\n"
            "</body></html>\r\n",
            CYGDAT_NET_ATHTTPD_SERVEROPT_SERVERID,
            httpstate.host[0],
            httpstate.host[1],
            httpstate.host[2],
            httpstate.host[3],
            CYGNUM_NET_ATHTTPD_SERVEROPT_PORT);
    
    httpstate.last_modified = -1;
    httpstate.mime_type = cyg_httpd_find_mime_string("html");
    if (err_type != CYG_HTTPD_STATUS_MOVED_TEMPORARILY) {
        // We use HTTP error code 302 to redirect web page.
        // To avoid browser maybe still use the orignal session for communication with server side
        // (IE especially when continuous click save button)
        // For this case, we don't need close the socket here. That the browser can reuse it.
        httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
    }
    httpstate.payload_len  = strlen(httpstate.inbuffer);
    cyg_int32 header_length = cyg_httpd_format_header();
    cyg_httpd_write(httpstate.outbuffer, header_length);
    if (httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY)
        return;
    cyg_httpd_write(httpstate.inbuffer, strlen(httpstate.inbuffer));
}

time_t
cyg_httpd_parse_date(char *time)
{
    int         i, check_week = 1, found = 0;
    char        month[4], week[12], *week_p = NULL;
    struct      tm tm_mod;
    cyg_int32   rc;

    // We are going to get rid of the day of the week. This is always the first
    //  part of the string, separated by a blank.
    time = strchr( time, ' ' );
    if ( time == NULL )
        return 0;
    time++;

    memset(week, 0, sizeof(week));
    memset(month, 0, sizeof(month));
    
    // asctime() in the stdlibc library.
    // The date is in the format: Sun Nov 6 08:49:37 1994.
    rc = sscanf(time,"%3s %3s %2d %2d:%2d:%2d %4d",
                week,
                month,
                &tm_mod.tm_mday,
                &tm_mod.tm_hour,
                &tm_mod.tm_min,
                &tm_mod.tm_sec,
                &tm_mod.tm_year);
    if (rc != 7)
    {
        /// RFC1123. The date is in the format: Sun, 06 Nov 1994 08:49:37 GMT.
        rc = sscanf(time,
                    "%3s, %2d %3s %4d %2d:%2d:%2d GMT",
                    week,
                    &tm_mod.tm_mday,
                    month,
                    &tm_mod.tm_year,
                    &tm_mod.tm_hour,
                    &tm_mod.tm_min,
                    &tm_mod.tm_sec);
        if (rc != 7)
        {
            strncpy(week, time, 10);
            if ((week_p = strtok(week, ",")) == NULL)
                return 0;
            
            for (i = 0; i < 7; i++)
            {
                if (strcmp(week_p, day_of_week_completed_name[i]) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
                return 0;

            check_week = 0;

            // RFC1036. The date is in the format: Sunday, 06-Nov-94 08:49:37 GMT.
            rc = sscanf(time + strlen(week_p) + 2, //skip ", "
                        "%2d-%3s-%2d %2d:%2d:%2d GMT",
                        &tm_mod.tm_mday,
                        month,
                        &tm_mod.tm_year,
                        &tm_mod.tm_hour,
                        &tm_mod.tm_min,
                        &tm_mod.tm_sec);
            if (rc != 6)
                return 0;
        }
    }

    if (check_week)
    {
        found = 0;
        for (i = 0; i < 7; i++)
        {
            if (strcmp(week, day_of_week[i]) == 0)
            {
                found = 1;
                break;
            }
        }
        if (!found)
            return 0;
    }
            
    for (i = 0; i < 12; i++)
        if (strcmp(month, month_of_year[i]) == 0)
        {
            tm_mod.tm_mon = i;
            if (tm_mod.tm_year > 1900)
                tm_mod.tm_year -= 1900;
            return mktime(&tm_mod);
        }

    return 0;
}

// Finds the mime string into the mime_table associated with a specific 
//  extension. Returns the MIME type to send in the header, or NULL if the
//  extension is not in the table.
char*
cyg_httpd_find_mime_string(const char *ext)
{
    cyg_httpd_mime_table_entry *entry = cyg_httpd_mime_table;

    while (entry != cyg_httpd_mime_table_end)
    {
        if (!strcmp((const char*)ext, entry->extension ))
            return entry->mime_string;
        entry++;
    }
            
    return (char*)0;
}

void
cyg_httpd_cleanup_filename(char *filename)
{
    char *src  = filename;

    src = strstr(filename, "//");
    while (src != 0)
    {
        strcpy(src + 1, src + 2);
        src = strstr(filename, "//");
    }

    src = strstr(filename, "/./");
    while (src != 0)
    {
        strcpy(src + 1, src + 3);
        src = strstr(filename, "/./");
    }

    src = strstr(filename, "/../");
    while (src != 0)
    {
        char *comp1 = filename, *comp2 = filename;

        // Search the path component before this redirection.
        while ((comp1 = strchr(comp1, '/')) != src)
            comp2 = ++comp1;

        strcpy(comp2, src + 4);
        src = strstr(filename, "/../");
    }
 }

cyg_int32
cyg_httpd_initialize(void)
{
    httpstate.needs_auth = (cyg_httpd_auth_table_entry *)0;
    return 0;
}

void
cyg_httpd_append_homepage(char *root)
{
#ifdef CYGOPT_NET_ATHTTPD_USE_FS
    struct stat sp;
    cyg_int32 i;

    cyg_int32 root_len = strlen(root);
    for (i = 0; i < sizeof(home_pages)/sizeof(char*); i++)
    {
        root[root_len] = '\0';
        sprintf(root + root_len, "%s", home_pages[i]);
        cyg_int32 rc = stat(root, &sp);
        if (rc == 0)
            return;
    }
    root[root_len] = 0;
#endif    
    
#ifdef CYGDAT_NET_ATHTTPD_ALTERNATE_HOME    
    if (strcmp(root, "/") == 0)
        // The client is trying to open the main index file.
        strcat(root, CYGDAT_NET_ATHTTPD_ALTERNATE_HOME);
#endif    
}

#ifdef CYGOPT_NET_ATHTTPD_USE_FS
void
cyg_httpd_send_file(char *name)
{
    cyg_int32  err;
    FILE      *fp;
    struct stat sp;
    char       file_name[CYG_HTTPD_MAXPATH];

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    diag_printf("Sending file: %s\n", name);
#endif    
    strcpy(file_name, CYGDAT_NET_ATHTTPD_SERVEROPT_ROOTDIR);
    if (file_name[strlen(file_name)-1] != '/')
        strcat(file_name, "/");
    strcat(file_name, name);
    cyg_httpd_cleanup_filename(file_name);
        
    // Check if the file is in the file system. This will also give us the
    //  size of the file, to be used in the HTTP header.
    cyg_int32 rc = stat(file_name, &sp);
    if (rc < 0)
    {
        // Before giving up, we make a last ditch attempt at finding a file
        //  within the internal resources of the server. The user can add
        //  his/her own files to the table.
        cyg_httpd_ires_table_entry *p = cyg_httpd_find_ires(name);
        if (p != NULL)
        {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
            diag_printf("Sending Internal Resource: %s\n", name);
#endif    
            cyg_httpd_send_ires(p);
        }    
        else    
            cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_FOUND);
        return;
    }
    
    if (S_ISDIR(sp.st_mode))
    {
        char tmp_url[CYG_HTTPD_MAXURL];
        strcpy(tmp_url, httpstate.url);
        // Directories need a trialing slash, and if missing, we'll redirect
        //  the client to the right URL. This is called (appropriately
        //  enough) "Trailing-Slash Redirection". 
        if (name[strlen(name)-1] != '/')
        {
            if (CYGNUM_NET_ATHTTPD_SERVEROPT_PORT == 80)
                sprintf(httpstate.url,
                        "http://%d.%d.%d.%d%s/",
                        httpstate.host[0],
                        httpstate.host[1],
                        httpstate.host[2],
                        httpstate.host[3],
                        tmp_url);
            else            
                sprintf(httpstate.url,
                        "http://%d.%d.%d.%d:%d%s/",
                        httpstate.host[0],
                        httpstate.host[1],
                        httpstate.host[2],
                        httpstate.host[3],
                        CYGNUM_NET_ATHTTPD_SERVEROPT_PORT,
                        tmp_url);
            cyg_httpd_send_error(CYG_HTTPD_STATUS_MOVED_PERMANENTLY);
            return;
        }

        // We are going to try to locate an index page in the directory we got
        //  in the URL. 
        cyg_httpd_append_homepage(file_name);
        if (file_name[strlen(file_name)-1] == '/')
        {
#ifdef CYGOPT_NET_ATHTTPD_USE_DIRLIST
            // No home page found, we are sending a directory listing.
            cyg_httpd_send_directory_listing(name);
#else
            cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_FOUND);
#endif
            return;
        }
        stat(file_name, &sp);
    }
    
    httpstate.last_modified = sp.st_mtime;
    
    // Let's see if we luck out and can send a 304.
    if ((httpstate.modified_since != -1) && 
                   (httpstate.modified_since >= httpstate.last_modified))
    {                   
        cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_MODIFIED);
        return;
    }    
    else    
        httpstate.status_code = CYG_HTTPD_STATUS_OK;

    // Here we'll look for extension to the file. Consider the case where
    //  there might be more than one dot in the file name. We'll look for
    //  the last dot, then we'll check the extension.
    char *extension = rindex(file_name, '.');
    if (extension == NULL)
        httpstate.mime_type = 0;
    else    
        httpstate.mime_type = cyg_httpd_find_mime_string(++extension);

    httpstate.payload_len  = sp.st_size;
    cyg_int32 payload_size = cyg_httpd_format_header();
    if ((httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY) != 0)
    {
        cyg_int32 descr = httpstate.sockets[httpstate.client_index].descriptor;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        SSL *ssl = (SSL *)httpstate.sockets[httpstate.client_index].ssl;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
        diag_printf("Sending header only for URL: %s\n", file_name);
#endif 

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        if (ssl)
            SSL_write(ssl, httpstate.outbuffer, payload_size);
        else
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
            send(descr, httpstate.outbuffer, payload_size, 0);
        return;
    }

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    diag_printf("Sending file: %s\n", file_name);
#endif    
    fp = fopen(file_name, "r");
    if(fp == NULL)
    {
        // We should really read errno and send messages accordingly...
        cyg_httpd_send_error(CYG_HTTPD_STATUS_SYSTEM_ERROR);
        return;
    }
    
    // Fill up the rest of the buffer and send it out.
    cyg_int32 bread = fread(httpstate.outbuffer + strlen(httpstate.outbuffer),
                            1, 
                            CYG_HTTPD_MAXOUTBUFFER - payload_size,
                            fp);
    cyg_httpd_write(httpstate.outbuffer, payload_size + bread);

    ssize_t bytes_written = 0;
    sp.st_size -= bread;
    while (bytes_written < sp.st_size)
    {
        bread = fread(httpstate.outbuffer, 1, CYG_HTTPD_MAXOUTBUFFER, fp);
        bytes_written += cyg_httpd_write(httpstate.outbuffer, bread);
    }    
    
    err = fclose(fp);
    if(err < 0)
        cyg_httpd_send_error(CYG_HTTPD_STATUS_SYSTEM_ERROR);
}
#endif

cyg_int32
cyg_httpd_format_header(void)
{
    sprintf(httpstate.outbuffer, "HTTP/1.1 %d", httpstate.status_code);
    time_t time_val = time(NULL);
    
    // Error messages (i.e. with status other than OK, automatically add
    //  the no-cache header.
    switch (httpstate.status_code)
    {
    case CYG_HTTPD_STATUS_MOVED_PERMANENTLY:
        strcat(httpstate.outbuffer, " Moved Permanently\r\n");
        strcat(httpstate.outbuffer, "Location: ");
        strcat(httpstate.outbuffer, httpstate.url);
        strcat(httpstate.outbuffer, "\r\n");
        strcat(httpstate.outbuffer, "Cache-Control: no-cache\r\n");
        sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                "Content-Length: %d\r\n",
                httpstate.payload_len);
        break;
    case CYG_HTTPD_STATUS_MOVED_TEMPORARILY:
        strcat(httpstate.outbuffer, " Found\r\n");
        strcat(httpstate.outbuffer, "Location: ");
        strcat(httpstate.outbuffer, httpstate.url);
        strcat(httpstate.outbuffer, "\r\n");
        sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                "Content-Length: %d\r\n",
                httpstate.payload_len);
        break;
#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
    case CYG_HTTPD_STATUS_NOT_AUTHORIZED:
        // A 401 error closes the connection right away.
        httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
        strcat(httpstate.outbuffer, " Not Authorized\r\n");
        
        // Here we should set the proper header based on the authentication
        //  required (httpstate.needs_authMode) but for now, with only
        //  Basic Authentication supported (default), there is no need to do so.
        if (httpstate.needs_auth)
        {
        	if (httpstate.needs_auth->auth_mode == CYG_HTTPD_AUTH_BASIC)
        	{
        	    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
        	            "WWW-Authenticate: Basic realm=\"%s\"\r\n",
        	            httpstate.needs_auth->auth_domainname);
        	}
        	else             
        	{
        	    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
        	             "WWW-Authenticate: Digest realm=\"%s\" ",
        	             httpstate.needs_auth->auth_domainname);
        	    strftime(cyg_httpd_md5_nonce, 
        	             33,
        	             TIME_FORMAT_RFC1123,
        	             gmtime(&time_val));
        	    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
        	            "nonce=\"%s\" ", cyg_httpd_md5_nonce);
        	    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
        	            "opaque=\"%s\", ", 
        	            CYG_HTTPD_MD5_AUTH_OPAQUE);
        	    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
        	            "stale=false, algorithm=%s, qop=\"%s\"\r\n",
        	            CYG_HTTPD_MD5_AUTH_NAME,
        	            CYG_HTTPD_MD5_AUTH_QOP );
        	}
        }
        else
        {
            cyg_httpd_auth_table_entry *entry = cyg_httpd_auth_entry_from_path(httpstate.url);
            if (entry != NULL)
                sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                        "WWW-Authenticate: Basic realm=\"%s\"\r\n",
                        entry->auth_domainname);
            }
        strcat(httpstate.outbuffer, "Cache-Control: no-cache\r\n");
        break;
#endif
    case CYG_HTTPD_STATUS_NOT_MODIFIED:
        strcat(httpstate.outbuffer, " Not Modified\r\n");
        break;
    case CYG_HTTPD_STATUS_NOT_FOUND:
        strcat(httpstate.outbuffer, " Not Found\r\n");
        strcat(httpstate.outbuffer, "Cache-Control: no-cache\r\n");
        sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                "Content-Length: %d\r\n", 
                httpstate.payload_len);
        break;
    case CYG_HTTPD_STATUS_METHOD_NOT_ALLOWED:
        strcat(httpstate.outbuffer, " Method Not Allowed\r\n");
        strcat(httpstate.outbuffer, "Cache-Control: no-cache\r\n");
        break;
    default:
        strcat(httpstate.outbuffer, " OK\r\n");
        if ((httpstate.hflags & CYG_HTTPD_HDR_TRANSFER_CHUNKED) == 0)
            sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                    "Content-Length: %d\r\n", 
                    httpstate.payload_len);
        if (httpstate.hflags & CYG_HTTPD_HDR_NO_CACHE)
            strcat(httpstate.outbuffer, "Cache-Control: no-cache\r\n");
        if (httpstate.custom_header)
            strcat(httpstate.outbuffer, httpstate.custom_header);
        break;
    }

    /* 
     * RFC2616 says a server MUST NOT do so unless it has a
     * (reasonable) clock source.
     */
    if(cyg_httpd_has_clock()) {
        strcat(httpstate.outbuffer, "Date: ");
        strftime(httpstate.outbuffer + strlen(httpstate.outbuffer), 
                 CYG_HTTPD_MAXOUTBUFFER - strlen(httpstate.outbuffer),
                 TIME_FORMAT_RFC1123,
                 gmtime(&time_val));
        strcat(httpstate.outbuffer, "\r\n");
    }
    sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer), 
            "Server: %s\r\n", 
            CYGDAT_NET_ATHTTPD_SERVEROPT_SERVERID);
    
    if (httpstate.mode & CYG_HTTPD_MODE_CLOSE_CONN)
        strcat(httpstate.outbuffer, "Connection: close\r\n");
    else
        strcat(httpstate.outbuffer, "Connection: keep-alive\r\n");

    if(httpstate.status_code != CYG_HTTPD_STATUS_NOT_MODIFIED) {

        /* Entity headers */
        sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                "Content-Type: %s\r\n",
                httpstate.mime_type ? httpstate.mime_type :
                CYGDAT_NET_ATHTTPD_DEFAULT_MIME_TYPE);

        if (httpstate.hflags & CYG_HTTPD_HDR_ENCODING_GZIP)
            strcat(httpstate.outbuffer, "Content-Encoding: gzip\r\n");

        if (httpstate.hflags & CYG_HTTPD_HDR_TRANSFER_CHUNKED)
            strcat(httpstate.outbuffer, "Transfer-Encoding: chunked\r\n");

        if (httpstate.last_modified != -1)
        {
            time_val = httpstate.last_modified;
            strcat(httpstate.outbuffer, "Last-Modified: ");
            strftime(httpstate.outbuffer + strlen(httpstate.outbuffer), 
                     CYG_HTTPD_MAXOUTBUFFER - strlen(httpstate.outbuffer),
                     TIME_FORMAT_RFC1123,
                     gmtime(&time_val));
            strcat(httpstate.outbuffer, "\r\n");
        }        

#if (CYGOPT_NET_ATHTTPD_DOCUMENT_EXPIRATION_TIME != 0)                 
        time_val += CYGOPT_NET_ATHTTPD_DOCUMENT_EXPIRATION_TIME;
        strcat(httpstate.outbuffer, "Expires: "); 
        strftime(httpstate.outbuffer + strlen(httpstate.outbuffer), 
                 CYG_HTTPD_MAXOUTBUFFER - strlen(httpstate.outbuffer),
                 TIME_FORMAT_RFC1123,
                 gmtime(&time_val));
        strcat(httpstate.outbuffer, "\r\n");
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
        if(httpstate.hflags & CYG_HTTPD_HDR_ETAG) {
            sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                    "ETag: %s\r\n", http_ires_etag);
        }
#endif

        if(httpstate.hflags & CYG_HTTPD_HDR_XREADONLY) {
            sprintf(httpstate.outbuffer + strlen(httpstate.outbuffer),
                    "X-ReadOnly: %s\r\n", http_readonly);
        }
    }
                 
    // There must be 2 carriage returns between the header and the body, 
    //  so if you modify this function make sure that there is another 
    //  CRLF already terminating the buffer thus far.
    strcat(httpstate.outbuffer, "\r\n");
    httpstate.hflags = 0;       /* One-shot flags */
    httpstate.custom_header = NULL; /* One-shot header(s) */
    return strlen(httpstate.outbuffer);
}

void
cyg_httpd_handle_method_GET(void)
{
    // Handlers are executed first.
    handler h = cyg_httpd_find_handler();
    if (h != 0)
    {
        // A handler was found. We'll call the function associated to it. If 
        //  the return value is 0 we'll also try to see if the file by the same
        //  name is available in the file system or internal resources.
#ifdef CYGOPT_NET_ATHTTPD_USE_FS
        cyg_int32 rc = h(&httpstate);
        if (rc != 0)
            return;
#else            
        h(&httpstate);
        return;
#endif        
    }
    
#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_OBJLOADER
    // If the URL is a CGI script, there is a different directory...
    if (httpstate.url[0] == '/' &&
                    !strncmp(httpstate.url + 1, 
                              CYGDAT_NET_ATHTTPD_SERVEROPT_CGIDIR, 
                              strlen(CYGDAT_NET_ATHTTPD_SERVEROPT_CGIDIR)))
    {                              
        cyg_httpd_exec_cgi();
        return;
    }
    // If the OBJLOADER package is not loaded, then the request for a library
    //  will likely generate a 404.
#endif    

#ifdef CYGOPT_NET_ATHTTPD_USE_FS
    // No handler, we'll redirect to the file system.
    cyg_httpd_send_file(httpstate.url);
#else
    // If we do not have a file system, we look for the file within the 
    //  internal resources of the server. The user can add his/her own files
    //  to the table.
    if (strcmp(httpstate.url, "/") == 0)
    {
        int i;
        cyg_httpd_ires_table_entry *p;
        for (i = 0; i < sizeof(home_pages)/sizeof(char*); i++)
        {
            httpstate.url[1] = '\0';
            strcat(httpstate.url, home_pages[i]);
            p = cyg_httpd_find_ires(httpstate.url);
            if (p != NULL)
            {
                cyg_httpd_send_ires(p);
                return;
            }    
        }        
    }
    else
    {
        cyg_httpd_ires_table_entry *p = cyg_httpd_find_ires(httpstate.url);
        if (p != NULL)
        {
            cyg_httpd_send_ires(p);
            return;
        }    
    }        
    cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_FOUND);
#endif    
}

char*
cyg_httpd_get_URL(char* p)
{
    char* dest = httpstate.url;

    // First get rid of multiple leading slashes.
    while ((p[0] == '/') && (p[1] == '/'))
       p++;

    // Store the url, and check if there is a form result in it.
    while ((*p != ' ') && (*p != '?') &&
            ((dest - httpstate.url) < CYG_HTTPD_MAXURL - 1))
    {
        // Look for encoded characters in the URL.
        if (*p == '%') 
        {
            p++;
            if (*p) 
                *dest = cyg_httpd_from_hex(*p++) * 16;
            if (*p) 
                *dest += cyg_httpd_from_hex(*p++);
        }
        else 
            *dest++ = *p++;
    }

    // Terminate the file name...
    *dest = '\0';

    // The URL must start with a leadng slash.
    if (httpstate.url[0] != '/') 
    {
        cyg_httpd_send_error(CYG_HTTPD_STATUS_BAD_REQUEST);
        return (char*)0;
    }

    // Run past white spaces.
    while ((*p == ' ') && (*p != '\0'))
        p++;
    return p;
}

static void
cyg_httpd_copy_args(const char* p)
{
    int rem = sizeof(httpstate.args)-1, i = 0;
    if(p) {
        if(*p == '?')
            p++;                    /* Skip '?' */
        while(rem && *p && *p != '\n' && *p != ' ') {
            httpstate.args[i++] = *p++;
            rem--;
        }
    }
    httpstate.args[i] = '\0';   /* Worst case zero terminate args */
}

char*
cyg_httpd_parse_POST(char* p)
{
    httpstate.method = CYG_HTTPD_METHOD_POST;
    httpstate.mode &= ~CYG_HTTPD_MODE_SEND_HEADER_ONLY;
    p = cyg_httpd_get_URL(p);
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    diag_printf("POST Request URL: %s\n", 
                httpstate.sockets[httpstate.client_index].descriptor, 
                httpstate.url);
#endif    
    cyg_httpd_copy_args(p);
    if (p == 0)
        return 0;

    while (*p++ != '\n');
    return p;
}

char*
cyg_httpd_parse_GET(char* p)
{
    p = cyg_httpd_get_URL(p);
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    diag_printf("%s(%d) Request URL: %s\n", 
                httpstate.method == CYG_HTTPD_METHOD_GET ? "GET" : "HEAD",
                httpstate.sockets[httpstate.client_index].descriptor, 
                httpstate.url);
#endif    
    cyg_httpd_copy_args(p);
    if (p == 0)
        return 0;
    
    if (*(p - 1) != ' ' && *p == '?')
        // If we have a GET header with form variables we'll get the
        //  variables out of it and store them in the variable table.
        p = cyg_httpd_store_form_data(++p);
    else
        // Must reset to avoid stale variables
        (void) cyg_httpd_store_form_data(NULL);

    // Run past white spaces.
    while (p && *p == ' ')
        p++;
    return p;
}

cyg_int32
cyg_httpd_process_header(char *p)
{
    // The default for HTTP 1.1 is keep-alive connections, unless specifically
    //  closed by the far end.
    httpstate.mode &= ~CYG_HTTPD_MODE_CLOSE_CONN;
    httpstate.modified_since = -1;
    httpstate.content_type = CYG_HTTPD_CONTENT_TYPE_UNKNOWN;
    httpstate.content_len = 0;

#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
    // Clear the previous request's response. The client properly authenticated
    //  will always reinitialize this variable during the header parsing
    //  process. This variable is also commandeered to hold the hashed
    //  username:password duo in the basic authentication.
    cyg_httpd_md5_digest[0] = '\0';
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
    httpstate.in_etag = NULL;
#endif // CYGOPT_NET_ATHTTPD_USE_IRES_ETAG

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
    httpstate.cookies = NULL;
#endif // CYGOPT_NET_ATHTTPD_USE_COOKIES

    while (p && p < httpstate.header_end)
    {
        if (strncasecmp("GET ", p, 4) == 0)
        {
            // We need separate flags for HEAD and SEND_HEADERS_ONLY since
            //  we can send a header only even in the case of a GET request
            //  (as a 304 response.)
            httpstate.method = CYG_HTTPD_METHOD_GET;
            httpstate.mode &= ~CYG_HTTPD_MODE_SEND_HEADER_ONLY;
            p = cyg_httpd_parse_GET(p + 4);
            if (p == 0)
                return 0;
        }
        else if (strncasecmp("POST ", p, 5) == 0)
        {
            p = cyg_httpd_parse_POST(p + 5);
            if (p == 0)
                return 0;
        }
        else if (strncasecmp("HEAD ", p, 5) == 0)
        {
            httpstate.method = CYG_HTTPD_METHOD_HEAD;
            httpstate.mode |= CYG_HTTPD_MODE_SEND_HEADER_ONLY;
            p = cyg_httpd_parse_GET(p + 5);
            if (p == 0)
                return 0;
        }
        else if (strncasecmp(p, "Content-Length: ", 16) == 0)
        {
            p = strchr(p, ':') + 2;
            if (p)
                // In the case of a POST request, this is the total length of
                //  the payload, which might be spread across several frames.
                httpstate.content_len = atoi(p);
            while (*p && *p++ != '\n');
        }
        else if (strncasecmp(p, "Content-Type: ", 14) == 0)
        {
            p = strchr(p, ':') + 2;
            if (p) {
                const char *urlencoded = "application/x-www-form-urlencoded";
                const char *formdata = "application/multipart/form-data";
                if (strncasecmp(p, urlencoded, strlen(urlencoded)) == 0)
                    httpstate.content_type = CYG_HTTPD_CONTENT_TYPE_URLENCODED;
                else if (strncasecmp(p, formdata, strlen(formdata)) == 0)
                    httpstate.content_type = CYG_HTTPD_CONTENT_TYPE_FORMDATA;
            }
            while (*p && *p++ != '\n');
        }
        else if (strncasecmp("Host:", p, 5) == 0)
        {
            p += 5;
            if (*p == ' ')
                p++;
            sscanf(p,
                   "%d.%d.%d.%d",
                   &httpstate.host[0],
                   &httpstate.host[1],
                   &httpstate.host[2],
                   &httpstate.host[3]);
            while (*p && *p++ != '\n');
        }
        else if (strncasecmp("If-Modified-Since:", p, 18) == 0)
        {
            p += 18;
            if (*p == ' ')
                p++;
            httpstate.modified_since = cyg_httpd_parse_date(p);
            while (*p && *p++ != '\n');
        }
#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
        else if (strncasecmp("Authorization:", p, 14) == 0)
        {
            p += 14;
            while (*p == ' ')
                p++;
            if (strncasecmp("Basic", p, 5) == 0)
            {
                p += 5;
                while (*p == ' ')
                    p++;
                cyg_int32 auth_data_length = 0;    
                while (*p != '\n') 
                {
                    // We are going to copy only up to 
                    //  AUTH_STORAGE_BUFFER_LENGTH characters to prevent
                    //  overflow of the cyg_httpd_md5_response variable.
                    if (auth_data_length < AUTH_STORAGE_BUFFER_LENGTH)
                        if ((*p != '\r') && (*p != ' '))
                            cyg_httpd_md5_digest[auth_data_length++] = *p;
                    p++;
                }    
                p++;        
                cyg_httpd_md5_digest[auth_data_length] = '\0';
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
                diag_printf("response = %s, auth_data_length: %d\r\n",
                            cyg_httpd_md5_digest,
                            auth_data_length);
#endif
            }
            else if (strncasecmp(p, "Digest", 6) == 0)
            {
                p += 6;
                while (*p == ' ')
                   p++;
                while ((*p != '\r') && (*p != '\n'))
                {
                    if (strncasecmp(p, "realm=", 6) == 0)
                        p = cyg_httpd_digest_skip(p + 6);
                    else if (strncasecmp(p, "username=", 9) == 0)
                        p = cyg_httpd_digest_skip(p + 9);
                    else if (strncasecmp(p, "nonce=", 6) == 0)
                        p = cyg_httpd_digest_skip(p + 6);
                    else if (strncasecmp(p, "response=", 9) == 0)
                        p = cyg_httpd_digest_data(cyg_httpd_md5_response, 
                                                  p + 9);
                    else if (strncasecmp(p, "cnonce=", 7) == 0)
                        p = cyg_httpd_digest_data(cyg_httpd_md5_cnonce, p + 7);
                    else if (strncasecmp(p, "qop=", 4) == 0)
                        p = cyg_httpd_digest_skip(p + 4);
                    else if (strncasecmp(p, "nc=", 3) == 0)
                        p = cyg_httpd_digest_data(cyg_httpd_md5_noncecount, 
                                                  p + 3);
                    else if (strncasecmp(p, "algorithm=", 10) == 0)
                        p = cyg_httpd_digest_skip(p + 10);
                    else if (strncasecmp(p, "opaque=", 7) == 0)
                        p = cyg_httpd_digest_skip(p + 7);
                    else if (strncasecmp(p, "uri=", 4) == 0)
                        p = cyg_httpd_digest_skip(p + 4);
                    else {
                        do {
                            p++;
                        } while (*p == ' ');
                    }
                }
            }    
            else
                while (*p && *p++ != '\n');
        }   
#endif // CYGOPT_NET_ATHTTPD_USE_AUTH
#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
        else if (strncasecmp(p, "If-None-Match:", 14) == 0)
        {
            p += 14;
            while (*p == ' ')
                p++;
            httpstate.in_etag = p;
            while(*p && *p++ != '\n');
        }
#endif // CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
        else if (strncasecmp(p, "Cookie:", 7) == 0)
        {
            p += 7;
            while (*p == ' ')
                p++;
            httpstate.cookies = p;
            while(*p && *p++ != '\n');
        }
#endif // CYGOPT_NET_ATHTTPD_USE_COOKIES
        else if (strncasecmp(p, "Connection:", 11) == 0)
        {
            p += 11;
            while (*p == ' ')
               p++;
            if (strncasecmp(p, "close", 5) == 0)
                httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
            while(*p && *p++ != '\n');
        }
        else if (strncasecmp("User-Agent:", p, 11) == 0)
        {
            p += 11;
            while (*p == ' ')
               p++;
            httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_UNKNOWN;
            while ((*p != '\r') && (*p != '\n')) {
                if (strncasecmp(p, "MSIE", 4) == 0) {
                    httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_MSIE;
                    break;
                } else if (strncasecmp(p, "Firefox", 7) == 0) {
                    httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_FF;
                    break;
                } else if (strncasecmp(p, "Chrome", 6) == 0) {
                    httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_CHROME;
                    break;
                } else if (strncasecmp(p, "Safari", 6) == 0) {
                    httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_SAFARI;
                    break;
                } else if (strncasecmp(p, "Opera", 5) == 0) {
                    httpstate.agent_navigator = CYG_HTTP_AGENT_NAVIGATOR_OPERA;
                    break;
                }
                p++;
            }
            while(*p && *p++ != '\n');
        }
        else
        {
            // We'll just dump the rest of the line and move on to the next.
            while(*p && *p++ != '\n');
        }
    }
    return 1;
}

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
/* When session windows are closed abruptly without
    graceful Logoff, the web sessions at the server side
    will be still keep the session ID until the timeout.
*/
/* Return 0: no timeout, -1: timeout */
static cyg_int32
cyg_httpd_process_session_timeout(cyg_int32 socket_idx, int is_ssl, cyg_int32 sess_descriptor)
{
    char        *ptr, buf[16];
    ssize_t     id_len;
    cyg_int32   is_timeout = 0, sess_id = 0, first_unused_idx = -1, i;
    time_t      active_sess_timeout, absolute_sess_timeout, current_time = cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR;

#ifndef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
    if (!strcmp(httpstate.url, "/") ||
        !strcmp(httpstate.url, "/index.htm")) {
        return 0;
    }
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

    // We use "seid" to save HTTP session ID and "sesslid" to save HTTPS session ID 
    if (is_ssl) {
        active_sess_timeout = httpstate.https_active_sess_timeout;
        absolute_sess_timeout = httpstate.https_absolute_sess_timeout;
        ptr = cyg_httpd_cookie_varable_find(httpstate.cookies, "sesslid", &id_len);
    } else {
        active_sess_timeout = httpstate.active_sess_timeout;
        absolute_sess_timeout = httpstate.absolute_sess_timeout;
        ptr = cyg_httpd_cookie_varable_find(httpstate.cookies, "seid", &id_len);
    }

    /* get sess_id */
    memset(buf, 0, sizeof(buf));
    strncpy(buf, ptr, id_len);
    sess_id = atoi(buf);

    /* There's no real completed solution for clear authen tication cache in Firefox, Chrome or Safari.
       A key technique that we use to effectively log out feature: Change the authentication to a wrong username/password.
       It works on FF, Chrome and Safari but not on Opera.
       we use a specific cookie (sess_id, -1) to trigger a HTTP response with error code 401 (not authorized)
       */
    if (sess_id == -1) {
        return -1;
    }
    
#ifndef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
    if (!active_sess_timeout && !absolute_sess_timeout) {
        return 0;
    }
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

    if (ptr == NULL) {
        return -1;
    }

    /* check if any session timeout */
    for (i = 0; i < CYGNUM_FILEIO_NFILE; i++) {
        if (httpstate.sessions[i].sess_id &&
            (((!httpstate.sessions[i].is_ssl && httpstate.active_sess_timeout && (current_time - httpstate.sessions[i].active_timestamp) >= httpstate.active_sess_timeout) || (!httpstate.sessions[i].is_ssl && httpstate.absolute_sess_timeout && (current_time - httpstate.sessions[i].absolute_timestamp) >= httpstate.absolute_sess_timeout)) ||
             ((httpstate.sessions[i].is_ssl && httpstate.https_active_sess_timeout && (current_time - httpstate.sessions[i].active_timestamp) >= httpstate.https_active_sess_timeout) || (httpstate.sessions[i].is_ssl && httpstate.https_absolute_sess_timeout && (current_time - httpstate.sessions[i].absolute_timestamp) >= httpstate.https_absolute_sess_timeout)))) {
            /* session timeout */
            if (httpstate.sessions[i].is_ssl == is_ssl &&
                httpstate.sessions[i].sess_id == sess_id
               ) {
                is_timeout = 1;
            }
            memset(&httpstate.sessions[i], 0, sizeof(httpstate.sessions[i]));
            if (is_ssl && httpstate.https_session_cnt > 0) {
                httpstate.https_session_cnt--;
            } else if (!is_ssl && httpstate.http_session_cnt > 0) {
                httpstate.http_session_cnt--;
            }
            if (is_timeout) {
                if (socket_idx != -1) {
                    httpstate.sockets[socket_idx].sess_id = 0;
                }
                return -1;
            }
        }

        if (first_unused_idx == -1 && !httpstate.sessions[i].sess_id) {
            first_unused_idx = i;
        } else if (httpstate.sessions[i].is_ssl == is_ssl &&
                   httpstate.sessions[i].sess_id == sess_id) {
            /* no session timeout, update lately session descriptor and timestamp */
            httpstate.sessions[i].sess_descriptor = sess_descriptor;
            httpstate.sessions[i].active_timestamp = current_time;
            httpstate.sockets[socket_idx].sess_id = sess_id;
            return 0;
        }
    }

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
    if (strcmp(httpstate.url, "/config/login")) {
        return -1;
    }
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

    if (first_unused_idx == -1) {
        /* No room for saving the new session information. */
        return -1;
    }

    /* Check if reach max. count */
    if ((!is_ssl && httpstate.http_session_cnt >= CYGOPT_NET_ATHTTPD_HTTP_SESSION_MAX_CNT) ||
        (is_ssl && httpstate.https_session_cnt >= CYGOPT_NET_ATHTTPD_HTTPS_SESSION_MAX_CNT)) {
        return -2;
    }
    if (is_ssl) {
        httpstate.https_session_cnt++;
    } else {
        httpstate.http_session_cnt++;
    }
    
    /* Save the new session information */
    httpstate.sessions[first_unused_idx].is_ssl = is_ssl;
    httpstate.sessions[first_unused_idx].sess_id = sess_id;
    httpstate.sessions[first_unused_idx].sess_descriptor = sess_descriptor;
    httpstate.sessions[first_unused_idx].active_timestamp = httpstate.sessions[first_unused_idx].absolute_timestamp = current_time;
    httpstate.sockets[socket_idx].sess_id = sess_id;
    return 0;
}
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
/* Return 0: no Auth success, -1: Auth failed */
static int cyg_httpd_check_auth(cyg_int32 socket_idx, cyg_int32 login_component, int is_ssl, cyg_int32 sess_descriptor)
{
    cyg_int32 ret = 0;
    cyg_httpd_auth_table_entry *auth_entry_p = NULL;

    if (!login_component &&
    	((auth_entry_p = cyg_httpd_is_authenticated(httpstate.url)) ||
        (!auth_entry_p && (ret = cyg_httpd_process_session_timeout(socket_idx, is_ssl, sess_descriptor))))) {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
        diag_printf("Auth failed: %s\n", httpstate.url);
#endif

        if (ret == -2) { //reach max. count
            cyg_httpd_start_chunked("html");
            sprintf(httpstate.outbuffer, "Reach Maximum Login Count");
            cyg_httpd_write_chunked(httpstate.outbuffer, strlen(httpstate.outbuffer));
            cyg_httpd_end_chunked();
        } else {
            strncpy(httpstate.url, "/login.htm", sizeof(httpstate.url)-1);
            cyg_httpd_send_error(CYG_HTTPD_STATUS_MOVED_TEMPORARILY);
        }
        return -1;
    }

    return 0;
}

#else /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

static int cyg_httpd_check_auth(cyg_int32 socket_idx, cyg_int32 login_component, int is_ssl, cyg_int32 sess_descriptor)
{
#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
    cyg_int32 ret = 0;
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */
    cyg_httpd_auth_table_entry *auth_entry_p = NULL;

    if (!login_component &&
    	((auth_entry_p = cyg_httpd_is_authenticated(httpstate.url))
#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
        || (!auth_entry_p && (ret = cyg_httpd_process_session_timeout(socket_idx, is_ssl, sess_descriptor)))
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */
        )) {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
        diag_printf("Auth failed: %s\n", httpstate.url);
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
        if (ret && !httpstate.cookies) { // authenticated success but no session ID information in cookies
            strncpy(httpstate.url, "/", sizeof(httpstate.url)-1);
            cyg_httpd_send_error(CYG_HTTPD_STATUS_MOVED_TEMPORARILY);
        } else
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */        
        {
            cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_AUTHORIZED);
        }
        return -1;
    }

    return 0;
}
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */

void
cyg_httpd_process_method(cyg_int32 socket_idx, int is_ssl, cyg_int32 sess_descriptor)
{
    char* p = httpstate.inbuffer;

    // Some browsers send an extra '\r\n' after the POST data that is not
    //  accounted in the "Content-Length:" field. We are going to junk all
    //  the leading returns and line carriages we find.
    while ((*p == '\r') || (*p =='\n'))
        p++;
    
    if (cyg_httpd_process_header(p) == 0)
        return;

#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
    /* ignore all login components */
    cyg_int32   login_component = 0;
    char        *extension = rindex(httpstate.url, '.');
    if (extension && (!strcmp(extension, ".ico") ||
                      !strcmp(extension, ".gif") ||
                      !strcmp(extension, ".jpg") ||
                      !strcmp(extension, ".png") ||
                      !strcmp(extension, ".js") ||
                      !strcmp(extension, ".css"))) {
        login_component = 1;
    }

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH
    if (!strcmp(httpstate.url, "/login.htm")) {
        login_component = 1;
    }
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIE_AUTH */
 
    // Let's check that the requested URL is not inside some directory that 
    //  needs authentication. ((function name has polarity error!)
    if (cyg_httpd_check_auth(socket_idx, login_component, is_ssl, sess_descriptor)) {
        return;
    }
#endif /* CYGOPT_NET_ATHTTPD_USE_AUTH */

    switch (httpstate.method)
    {
        case CYG_HTTPD_METHOD_GET:
        case CYG_HTTPD_METHOD_HEAD:
            cyg_httpd_handle_method_GET();
            break;
        case CYG_HTTPD_METHOD_POST:
            cyg_httpd_handle_method_POST();
            break;
        default:
            cyg_httpd_send_error(CYG_HTTPD_STATUS_NOT_IMPLEMENTED);
            break;
    }
}

/* peter, 2009/5,
   Add a tag of "X-ReadOnly" in HTML response header to descript the web page readonly status.
   parameter of status:
   0: Not allow any action
   1: Only allow readonly */
void cyg_httpd_set_xreadonly_tag(int status)
{
    if (status)
        strcpy(http_readonly, "true");
    else
        strcpy(http_readonly, "null");
    cyg_httpd_header_flag(&httpstate, CYG_HTTPD_HDR_XREADONLY);
}

