/* =================================================================
 *
 *      socket.c
 *
 *      Opens socket and starts the daemon.
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
 *  Contributors: Sergei Gavrikov (w3sg@SoftHome.net),
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
#include <cyg/kernel/ktypes.h>         // base kernel types.
#include <cyg/infra/diag.h>            // For diagnostic printing.
#include <network.h>
#ifdef CYGPKG_NET_FREEBSD_INET6
#include <netinet/ip6.h>
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>                     // sprintf().
#include <time.h>                      // sprintf().

#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>
#include <cyg/athttpd/cgi.h>

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

/* HTTPS socket FD */
#define HTTPD_MAX_SOCKETS   CYGPKG_NET_MAXSOCKETS

static cyg_int32 https_listener = -1;
#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 https_listener_v6 = -1;
#endif /* CYGPKG_NET_FREEBSD_INET6 */

static cyg_int32 cyg_https_init_socket(unsigned long port);
static void cyg_httpd_shutdown(int index);
#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 cyg_https_init_socket_v6(unsigned long port);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
static ssize_t cyg_https_writev(SSL *ssl, const struct iovec *vector, int count);
static void cyg_httpd_redirect_to_https(int fd);
static void cyg_https_redirect_to_httpd(SSL *ssl);

static SSL_CTX *server_ctx = NULL;
static int https_running_flag = 0;
static cyg_https_new_cert_info_t https_new_cert_info;

extern cyg_int32 cyg_httpd_process_header(char *p); /* declared in http.c */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

static int redirect_to_https_flag = 0;

#ifndef MAX
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

#define CYG_HTTPD_MSEC2TICK(msec) (msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000))

#define CYG_HTTPD_DAEMON_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_MINIMUM + \
                                          CYGNUM_NET_ATHTTPD_THREADOPT_STACKSIZE)
static cyg_int32    cyg_httpd_initialized = 0;
cyg_thread   cyg_httpd_thread_object;
cyg_handle_t cyg_httpd_thread_handle;
cyg_uint8    cyg_httpd_thread_stack[CYG_HTTPD_DAEMON_STACK_SIZE]
                                       __attribute__((__aligned__ (16)));
CYG_HTTPD_STATE httpstate;

__inline__ ssize_t
cyg_httpd_write(char* buf, int buf_len)
{
    cyg_int32 descr = httpstate.sockets[httpstate.client_index].descriptor;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    SSL *ssl = (SSL *)httpstate.sockets[httpstate.client_index].ssl;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
    ssize_t sent;

    // We are not going to write anything in case
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    if (ssl)
        sent = SSL_write(ssl, buf, buf_len);
    else
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
        sent = send(descr, buf, buf_len, 0);
    return sent;
}

__inline__ ssize_t
cyg_httpd_writev(cyg_iovec *iovec_bufs, int count)
{
    int i;
    ssize_t sent;
    cyg_int32 descr = httpstate.sockets[httpstate.client_index].descriptor;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    SSL *ssl = (SSL *)httpstate.sockets[httpstate.client_index].ssl;

    if (ssl)
        sent = cyg_https_writev(ssl, iovec_bufs, count);
    else
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
        sent = writev(descr, iovec_bufs, count);

    ssize_t buf_len = 0;
    for (i = 0; i < count; i++)
        buf_len += iovec_bufs[i].iov_len;
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 1
    if (sent != buf_len)
        diag_printf("writev() did not send out all bytes (%ld of %ld)\n",
                    sent,
                    buf_len);
#endif
    return sent;
}

// The need for chunked transfers arises from the fact that with dinamic
//  pages it is not always possible to know the packet size upfront, and thus
//  it is not possible to fill the 'Content-Length:' field in the header.
// Today's web browser use 'Content-Length:' when present in the header and
//  when not present they read everything that comes in up to the last 2 \r\n
//  and then figure it out. The HTTP standard _mandates_ 'Content-Length:' to
//  be present in the header with a correct value, and whenever that is not
//  possible, chunked transfers must be used.
//
// A chunked transer takes the form of:
// -----------------------------------------------------------------------------
//    cyg_httpd_start_chunked("html");
//    sprintf(phttpstate->payload, ...);
//    cyg_httpd_write_chunked(phttpstate->payload,
//                             strlen(phttpstate->payload));
//    ...
//    cyg_httpd_end_chunked();
// -----------------------------------------------------------------------------
ssize_t
cyg_httpd_start_chunked(const char *extension)
{
    httpstate.status_code = CYG_HTTPD_STATUS_OK;

#if defined(CYGOPT_NET_ATHTTPD_CLOSE_CHUNKED_CONNECTIONS)
     // I am not really sure that this is necessary, but even if it isn't, the
    //  added overhead is not such a big deal. In simple terms, I am not sure
    //  how much I can rely on the client to understand that the frame has ended
    //  with the last 5 bytes sent out. In an ideal world, the data '0\r\n\r\n'
    //  should be enough, but several posting on the subject I read seem to
    //  imply otherwise, at least with early generation browsers that supported
    //  the "Transfer-Encoding: chunked" mechanism. Things might be getting
    //  better now but I snooped some sites that use the chunked stuff (Yahoo!
    //  for one) and all of them with no exception issue a "Connection: close"
    //  on chunked frames even if there is nothing in the HTTP 1.1 spec that
    //  requires it.
    httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
#endif

    // We do not cache chunked frames. In case they are used to display dynamic
    //  data we want them to be executed any every time they are requested.
    httpstate.hflags |=
              (CYG_HTTPD_HDR_TRANSFER_CHUNKED | CYG_HTTPD_HDR_NO_CACHE);

    httpstate.last_modified = -1;
    httpstate.mime_type = cyg_httpd_find_mime_string(extension);
    cyg_int32 header_length = cyg_httpd_format_header();
    return cyg_httpd_write(httpstate.outbuffer, header_length);
}

ssize_t
cyg_httpd_write_chunked(const char* buf, int len)
{
    char leader[16], trailer[] = {'\r', '\n'};

    cyg_iovec iovec_bufs[] = { {leader, 0}, {(char*) buf, 0}, {trailer, 2} };
    sprintf(leader, "%x\r\n", len);
    iovec_bufs[0].iov_len = strlen(leader);
    iovec_bufs[1].iov_len = len;
    iovec_bufs[2].iov_len = 2;
    if (httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY)
        return (iovec_bufs[0].iov_len + iovec_bufs[1].iov_len +
                                                  iovec_bufs[2].iov_len);
    return cyg_httpd_writev(iovec_bufs, 3);
}

void
cyg_httpd_end_chunked(void)
{
    if (httpstate.mode & CYG_HTTPD_MODE_SEND_HEADER_ONLY)
        return;
    strcpy(httpstate.outbuffer, "0\r\n\r\n");
    cyg_httpd_write(httpstate.outbuffer, 5);
}

// This function builds and send out a standard header. It is likely going to
//  be used by a c language callback function, and thus followed by one or
//  more calls to cyg_httpd_write(). Unlike cyg_httpd_start_chunked(), this
//  call requires prior knowledge of the final size of the frame (browsers
//  _will_trust_ the "Content-Length:" field when present!), and the user
//  is expected to make sure that the total number of bytes (octets) sent out
//  via 'cyg_httpd_write()' matches the number passed in the len parameter.
// Its use is thus more limited, and the more flexible chunked frames should
//  be used whenever possible.
void
cyg_httpd_create_std_header(char *extension, int len)
{
    httpstate.status_code = CYG_HTTPD_STATUS_OK;
    httpstate.mode        = 0;

    // We do not want to send out a "Last-Modified:" field for c language
    //  callbacks.
    httpstate.last_modified = -1;
    httpstate.mime_type = cyg_httpd_find_mime_string(extension);
    httpstate.payload_len = len;
    cyg_int32 header_length = cyg_httpd_format_header();
    cyg_httpd_write(httpstate.outbuffer, header_length);
}

void
cyg_httpd_process_request(cyg_int32 index)
{
    httpstate.client_index = index;
    cyg_int32 descr = httpstate.sockets[index].descriptor;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    SSL *ssl = (SSL *)httpstate.sockets[httpstate.client_index].ssl;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
    int len = 0;

    httpstate.inbuffer_len = 0;
    time_t maxtime = time(NULL) + 10; /* Per-request *header* timeout */
    do
    {
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        if (ssl)
            len = SSL_read(ssl,
                           httpstate.inbuffer + httpstate.inbuffer_len,
                           CYG_HTTPD_MAXINBUFFER - httpstate.inbuffer_len);
        else
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
            len = recv(descr,
                       httpstate.inbuffer + httpstate.inbuffer_len,
                       CYG_HTTPD_MAXINBUFFER - httpstate.inbuffer_len,
                       0);

        if (len == 0)
        {
            // This is the client that has closed its TX socket, possibly as
            //  a response from a shutdown() initiated by the server. Another
            //  possibility is that the client was closed altogether, in
            //  which case the client sent EOFs on each open sockets before
            //  dying.
            cyg_httpd_shutdown(index);
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            diag_printf("EOF received on descriptor: %d. Closing it.\n", descr);
#endif
            return;
        }

        if (len < 0 || time(NULL) >= maxtime)
        {
            // There was an error or timeout reading from this socket. Play it
            //  safe and close it. This will force the client to generate a
            //  shutdown and we will read a len = 0 the next time around.
            /* peter, 2008/11, shutdown() will take a long time then free resouce
            shutdown(descr, SHUT_WR); */
            cyg_httpd_shutdown(index);
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            diag_printf("ERROR reading from socket. recv() returned: %d\n", len);
#endif
            return;
        }

        httpstate.inbuffer_len += len;
        httpstate.inbuffer[httpstate.inbuffer_len] = '\0'; // Terminate before calling strstr()
        httpstate.header_end = strstr(httpstate.inbuffer, CYG_HTTPD_HEADER_END_MARKER);
    } while(httpstate.header_end == NULL);

    // Advance header_end past the marker.
    httpstate.header_end += strlen(CYG_HTTPD_HEADER_END_MARKER);

    // Timestamp the socket.
    httpstate.sockets[index].timestamp = time(NULL);

    if (!ssl && redirect_to_https_flag && https_running_flag)
    {
        // Send out redirect to HTTPS htm
        cyg_httpd_redirect_to_https(descr);
    }
    else if (ssl && !https_running_flag)
    {
        // Send out redirect to HTTP htm
        cyg_https_redirect_to_httpd(ssl);
    }
    else
    {
        // This is where it all happens.
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        cyg_httpd_process_method(index, ssl ? 1 : 0, descr);
#else
        cyg_httpd_process_method(index, 0, descr);
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
    }

    if (httpstate.mode & CYG_HTTPD_MODE_CLOSE_CONN)
        // There are 2 cases we can be here:
        // 1) chunked frames close their connection by default
        // 2) The client requested the connection be terminated with a
        //     "Connection: close" in the header
        // In any case, we close the TX pipe and wait for the client to
        //  send us an EOF on the receive pipe. This is a more graceful way
        //  to handle the closing of the socket, compared to just calling
        //  close() without first asking the opinion of the client, and
        //  running the risk of stray data lingering around.
        /* peter, 2008/11, shutdown() will take a long time then free resouce
        shutdown(descr, SHUT_WR); */
        cyg_httpd_shutdown(index);
}

void
cyg_httpd_handle_new_connection(cyg_int32 fd, int is_security_port)
{
    cyg_int32 i;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    int rc;
    int fd_client;
    SSL *ssl = NULL;

#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

    fd_client = accept(fd, NULL, NULL);
    //CYG_ASSERT(fd_client != -1, "accept() failed");
    if (fd_client < 0)
        return;

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    if (is_security_port)
    {
        if (!https_running_flag || !server_ctx)
        {
            close(fd_client);
            return;
        }

        /* Create a new SSL */
        if ((ssl = SSL_new(server_ctx)) == NULL)
        {
            close(fd_client);
            return;
        }

        /* Assign socket into SSL */
        if (!SSL_set_fd(ssl, fd_client))
        {
            SSL_free(ssl);
            close(fd_client);
            return;
        }

        /* SSL handshake */
        if ((rc = SSL_accept(ssl)) <= 0)
        {
		    // for debug,
            //rc = ERR_GET_REASON(ERR_peek_error());
            SSL_free(ssl);
            close(fd_client);
            return;
        }
    }
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

    /* Establish per-read timeout to deal with hesitant clients */
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    getpeername(fd_client, (struct sockaddr *)&client_addr, &client_len);
    diag_printf("Opening descriptor: %d from %s:%d\n",
                fd_client,
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
#endif
    // Timestamp the socket and process the frame immediately, since the accept
    //  guarantees the presence of valid data on the newly opened socket.
    for (i = 0; i < HTTPD_MAX_SOCKETS; i++)
        if (httpstate.sockets[i].descriptor == -1)
        {
            httpstate.sockets[i].descriptor = fd_client;
            httpstate.sockets[i].timestamp  = time(NULL);
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
            httpstate.sockets[i].ssl = (void *)ssl;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
            cyg_httpd_process_request(i);
            return;
        }

        /* Colse the socket when internal HTTP sockets exhausted */
        close(fd_client);
        return;
}

// This is the "garbage collector" (or better, the "garbage disposer") of
//  the server. It closes any socket that has been idle for a time period
//  of CYG_HTTPD_SELECT_TIMEOUT seconds.
void
cyg_httpd_close_unused_sockets(void)
{
    cyg_int32 i;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Garbage collector called\r\n");
#endif
    for (i = 0; i < HTTPD_MAX_SOCKETS; i++)
    {
        if (httpstate.sockets[i].descriptor != -1)
        {
            if (time(NULL) - httpstate.sockets[i].timestamp >
                                          CYG_HTTPD_SOCKET_IDLE_TIMEOUT)
            {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
                diag_printf("Closing descriptor: %d\n",
                            httpstate.sockets[i].descriptor);
#endif
                /* peter, 2008/11, shutdown() will take a long time then free resouce
                shutdown(httpstate.sockets[i].descriptor, SHUT_WR); */
                cyg_httpd_shutdown(i);
            }
            else
                httpstate.fdmax = MAX(httpstate.fdmax,
                                      httpstate.sockets[i].descriptor);
        }
    }
}

static cyg_int32 cyg_http_init_socket(unsigned long port)
{
    cyg_int32 rc;

    cyg_int32 fd = socket(AF_INET, SOCK_STREAM, 0);
    //CYG_ASSERT(fd != -1, "Socket create failed");
    if (fd < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&(httpstate.server_conn), 0, sizeof(struct sockaddr_in));
    httpstate.server_conn.sin_family = AF_INET;
    httpstate.server_conn.sin_addr.s_addr = INADDR_ANY;
    httpstate.server_conn.sin_port = htons(port);
    while (bind(fd,
                (struct sockaddr *)&httpstate.server_conn,
                sizeof(struct sockaddr)) < 0)
        cyg_thread_delay(CYG_HTTPD_MSEC2TICK(1000));

    rc = listen(fd, SOMAXCONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0)
        return -1;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Web server HTTP IPv4 Started and listening...\n");
#endif

    return fd;
}

#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 cyg_http_init_socket_v6(unsigned long port)
{
    cyg_int32 rc;

    cyg_int32 fd = socket(AF_INET6, SOCK_STREAM, 0);
    //CYG_ASSERT(fd != -1, "Socket create failed");
    if (fd < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&(httpstate.server_conn_v6), 0, sizeof(struct sockaddr_in6));
    httpstate.server_conn_v6.sin6_family = AF_INET6;
    httpstate.server_conn_v6.sin6_len = sizeof(struct sockaddr_in6);
    httpstate.server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[0] = INADDR_ANY;
    httpstate.server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[1] = INADDR_ANY;
    httpstate.server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[2] = INADDR_ANY;
    httpstate.server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[3] = INADDR_ANY;
    httpstate.server_conn_v6.sin6_port = htons(port);
    while (bind(fd,
                (struct sockaddr *)&httpstate.server_conn_v6,
                sizeof(struct sockaddr_in6)) < 0)
        cyg_thread_delay(CYG_HTTPD_MSEC2TICK(1000));

    rc = listen(fd, SOMAXCONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0)
        return -1;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Web server HTTP IPv6 Started and listening...\n");
#endif

    return fd;
}
#endif /* CYGPKG_NET_FREEBSD_INET6 */

void
cyg_httpd_daemon(cyg_addrword_t data)
{
    cyg_int32 rc;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    struct timeval tv = {1, 0}; // Wake up from select() per-second for checking HTTPS service routing is needed or not
    cyg_int32 time_cnt = 0;
#else
    struct timeval tv = {CYG_HTTPD_SOCKET_IDLE_TIMEOUT, 0};
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */


#ifdef CYGOPT_NET_ATHTTPD_INIT_ALL_NETWORK_INTERFACES
    init_all_network_interfaces();
#endif

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up)
    {
        struct bootp* bps = &eth0_bootp_data;
        diag_printf("ETH0 is up. IP address: %s\n", inet_ntoa(bps->bp_yiaddr));
    }
#endif
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_CGIBIN_TCL
    cyg_httpd_init_tcl_interpreter();
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Tcl interpreter has been initialized...\n");
#endif
#endif

    cyg_httpd_initialize();

    // Get the network going. This is benign if the application has
    //  already done this.
    /* IPv4 */
    cyg_int32 listener = cyg_http_init_socket(CYGNUM_NET_ATHTTPD_SERVEROPT_PORT);
    if (listener < 0)
        return;

#ifdef CYGPKG_NET_FREEBSD_INET6
    /* IPv6 */
    cyg_int32 listener_v6 = cyg_http_init_socket_v6(CYGNUM_NET_ATHTTPD_SERVEROPT_PORT);
    if (listener_v6 < 0)
        return;
#endif /* CYGPKG_NET_FREEBSD_INET6 */

    cyg_int32 i;
    for (i = 0; i < CYGNUM_FILEIO_NFILE; i++)
    {
        httpstate.sockets[i].descriptor = -1;
        httpstate.sockets[i].timestamp = (time_t)0;
        httpstate.sockets[i].sess_id = 0;
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        httpstate.sockets[i].ssl = NULL;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
    }

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
    memset(httpstate.sessions, 0, sizeof(httpstate.sessions));
    httpstate.http_session_cnt = httpstate.https_session_cnt = 0;
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

    FD_ZERO(&httpstate.rfds);
    httpstate.fdmax = listener;
#ifdef CYGPKG_NET_FREEBSD_INET6
    httpstate.fdmax = MAX(httpstate.fdmax, listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
    if (https_running_flag && https_listener >= 0)
        httpstate.fdmax = MAX(httpstate.fdmax, https_listener);
#ifdef CYGPKG_NET_FREEBSD_INET6
    if (https_running_flag && https_listener_v6 >= 0)
        httpstate.fdmax = MAX(httpstate.fdmax, https_listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

    while (1)
    {
        // The listener is always added to the select() sensitivity list.
        FD_SET(listener, &httpstate.rfds);
#ifdef CYGPKG_NET_FREEBSD_INET6
        FD_SET(listener_v6, &httpstate.rfds);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
        if (https_running_flag && https_listener > 0)
            FD_SET(https_listener, &httpstate.rfds);
#ifdef CYGPKG_NET_FREEBSD_INET6
        if (https_running_flag && https_listener_v6 > 0)
            FD_SET(https_listener_v6, &httpstate.rfds);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

        rc = select(httpstate.fdmax + 1, &httpstate.rfds, NULL, NULL, &tv);
        if (rc > 0)
        {
            if (FD_ISSET(listener, &httpstate.rfds))
            {
                // If the request is from the listener socket, then
                //  this must be a new connection.
                cyg_httpd_handle_new_connection(listener, 0);
            }
#ifdef CYGPKG_NET_FREEBSD_INET6
            else if (FD_ISSET(listener_v6, &httpstate.rfds))
            {
                // If the request is from the listener socket, then
                //  this must be a new connection.
                cyg_httpd_handle_new_connection(listener_v6, 0);
            }
#endif /* CYGPKG_NET_FREEBSD_INET6 */
            else if (https_running_flag && https_listener >= 0 && FD_ISSET(https_listener, &httpstate.rfds))
            {
                // If the request is from the listener socket, then
                //  this must be a new connection.
                cyg_httpd_handle_new_connection(https_listener, 1);
            }
#ifdef CYGPKG_NET_FREEBSD_INET6
            else if (https_running_flag && https_listener_v6 >= 0 && FD_ISSET(https_listener_v6, &httpstate.rfds))
            {
                // If the request is from the listener socket, then
                //  this must be a new connection.
                cyg_httpd_handle_new_connection(https_listener_v6, 1);
            }
#endif /* CYGPKG_NET_FREEBSD_INET6 */

            httpstate.fdmax = listener;
#ifdef CYGPKG_NET_FREEBSD_INET6
            httpstate.fdmax = MAX(httpstate.fdmax, listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
            if (https_running_flag && https_listener >= 0)
                httpstate.fdmax = MAX(httpstate.fdmax, https_listener);
#ifdef CYGPKG_NET_FREEBSD_INET6
            if (https_running_flag && https_listener_v6 >= 0)
                httpstate.fdmax = MAX(httpstate.fdmax, https_listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

            // The sensitivity list returned by select() can have multiple
            //  socket descriptors that need service. Loop through the whole
            //  descriptor list to see if one or more need to be served.
            for (i = 0; i < HTTPD_MAX_SOCKETS; i ++)
            {
                cyg_int32 descr = httpstate.sockets[i].descriptor;
                if (descr != -1)
                {
                    // If the descriptor is set in the descriptor list, we
                    //  service it. Otherwise, we add it to the descriptor list
                    //  to listen for. The rfds list gets rewritten each time
                    //  select() is called and after the call it contains only
                    //  the descriptors that need be serviced. Before calling
                    //  select() again we must repopulate the list with all the
                    //  descriptors that must be listened for.
                    if (FD_ISSET(descr, &httpstate.rfds))
                        cyg_httpd_process_request(i);
                    else
                        FD_SET(descr, &httpstate.rfds);

                    if (httpstate.sockets[i].descriptor != -1)
                        httpstate.fdmax = MAX(httpstate.fdmax, descr);
                }
            }
        }
        else if (rc == 0)
        {
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
            if (++time_cnt < CYG_HTTPD_SOCKET_IDLE_TIMEOUT)
                continue; // don't need call close_unused_sockets()
            time_cnt = 0;
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
            httpstate.fdmax = listener;
#ifdef CYGPKG_NET_FREEBSD_INET6
            httpstate.fdmax = MAX(httpstate.fdmax, listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#ifdef CYGOPT_NET_ATHTTPD_HTTPS
            if (https_running_flag && https_listener >= 0)
                httpstate.fdmax = MAX(httpstate.fdmax, https_listener);
#ifdef CYGPKG_NET_FREEBSD_INET6
            if (https_running_flag && https_listener_v6 >= 0)
                httpstate.fdmax = MAX(httpstate.fdmax, https_listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

            cyg_httpd_close_unused_sockets();
        }
        else
        {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
            cyg_int8 *ptr = (cyg_int8*)&httpstate.rfds;
            diag_printf("rfds: %x %x %x %x\n", ptr[0], ptr[1], ptr[2], ptr[3] );
#endif

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
            // Shutdown all HTTPS descriptors when HTTPS mode is disabled
            if (https_running_flag)
                continue;

            FD_ZERO(&httpstate.rfds);
            httpstate.fdmax = listener;
#ifdef CYGPKG_NET_FREEBSD_INET6
            httpstate.fdmax = MAX(httpstate.fdmax, listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */

            for (i = 0; i < HTTPD_MAX_SOCKETS; i++)
            {
                if (httpstate.sockets[i].descriptor != -1)
                {
#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
                    diag_printf("Socket in list: %d\n",
                                 httpstate.sockets[i].descriptor);
#endif
                    if (httpstate.sockets[i].ssl != NULL)
                    {
                        cyg_httpd_shutdown(i);
                    }
                    else
                    {
                        FD_SET(httpstate.sockets[i].descriptor, &httpstate.rfds);
                        httpstate.fdmax = MAX(httpstate.fdmax, httpstate.sockets[i].descriptor);
                    }
                }
            }
            /* CYG_ASSERT(rc != -1, "Error during select()"); */
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */
        }
    }
}

void
cyg_httpd_start(void)
{
    if (cyg_httpd_initialized)
        return;
    cyg_httpd_initialized = 1;

    cyg_thread_create(CYGNUM_NET_ATHTTPD_THREADOPT_PRIORITY,
                      cyg_httpd_daemon,
                      (cyg_addrword_t)0,
                      "HTTPD Thread",
                      (void *)cyg_httpd_thread_stack,
                      CYG_HTTPD_DAEMON_STACK_SIZE,
                      &cyg_httpd_thread_handle,
                      &cyg_httpd_thread_object);
    cyg_thread_resume(cyg_httpd_thread_handle);
}

static void cyg_httpd_shutdown(int index)
{
#if 0 /* The method isn't suit to all browsers, the Safari will close all sockets due to socket resouce limitation or other unknown case. */
    cyg_int32 i, same_counter = 0;

    if ((!httpstate.sockets[index].ssl && (httpstate.active_sess_timeout || httpstate.absolute_sess_timeout)) ||
        (httpstate.sockets[index].ssl && (httpstate.https_active_sess_timeout || httpstate.https_absolute_sess_timeout))) {
        /* Clear login entry when last socket shutdown */
        for (i = 0; i < CYGNUM_FILEIO_NFILE; i++) {
            if (httpstate.sockets[i].sess_id && httpstate.sockets[i].sess_id == httpstate.sockets[index].sess_id) {
                same_counter++;
                if (same_counter > 1) {
                    break;
                }
            }
        }
        if (same_counter == 1) {
            cyg_http_release_session(httpstate.sockets[index].ssl ? 1: 0, httpstate.sockets[index].sess_id);
        }
    }
#endif

    if (httpstate.sockets[index].descriptor != -1) {
        if (httpstate.sockets[index].ssl) {
            SSL_free(httpstate.sockets[index].ssl);
        }
        httpstate.sockets[index].ssl = NULL;
        close(httpstate.sockets[index].descriptor);
        FD_CLR(httpstate.sockets[index].descriptor, &httpstate.rfds);
        httpstate.sockets[index].descriptor = -1;
        httpstate.sockets[index].timestamp = (time_t)0;
        httpstate.sockets[index].sess_id = 0;
    }
}

#ifdef CYGOPT_NET_ATHTTPD_HTTPS
int cyg_https_save_pkey(char *out, EVP_PKEY *pkey, const EVP_CIPHER *cipher, char *pass_phrase)
{
	BIO *in = NULL;

	if ((in = BIO_new(BIO_s_file())) == NULL)
	    return -1;

	if (BIO_write_filename(in, out) <= 0) {
        BIO_free(in);
	    return -1;
    }

    if (!PEM_write_bio_PrivateKey(in, pkey, cipher, (unsigned char *) pass_phrase, strlen(pass_phrase), 0, NULL)) {
        BIO_free(in);
	    return -1;
    }

    BIO_free(in);
    return 0;
}

int cyg_https_save_cert(char *out, X509 *x509_cert)
{
	BIO *in = NULL;

	if ((in = BIO_new(BIO_s_file())) == NULL)
	    return -1;

	if (BIO_write_filename(in, out) <= 0) {
        BIO_free(in);
	    return -1;
    }

    if (!PEM_write_bio_X509(in, x509_cert)) {
        BIO_free(in);
        return -1;
    }

    BIO_free(in);
    return 0;
}

static unsigned char dsa_seed[20] = {
	0xd5,0x01,0x4e,0x4b,0x60,0xef,0x2b,0xa8,0xb6,0x21,0x1b,0x40,
	0x62,0xba,0x32,0x24,0xe0,0x42,0x7d,0xd3,
};
static const unsigned char dsa_str[] = "12345678901234567890";

static int dsa_cb(int p, int n, BN_GENCB *arg)
{
	char c = '*';
	static int ok = 0, num = 0;

	if (p == 0) { c = '.'; num++;};
	if (p == 1) c = '+';
	if (p == 2) { c = '*'; ok++;}
	if (p == 3) c = '\n';
	BIO_write(arg->arg, &c, 1);
	(void)BIO_flush(arg->arg);

	if (!ok && (p == 0) && (num > 1))
    {
		BIO_printf((BIO *)arg,"error in dsatest\n");
		return 0;
    }
	return 1;
}

EVP_PKEY *cyg_https_generate_key(int key_bits, cyg_int32 is_rsa)
{
    int ret, ok = 0;
    EVP_PKEY *pkey = NULL;
    BN_GENCB cb;
	DSA *dsa = NULL;
	int counter;
	unsigned long h;
    BIO *bio_err = NULL;

    if ((pkey = EVP_PKEY_new()) == NULL)
		return (EVP_PKEY *)NULL;

    if (is_rsa) {
        /* PRNG, MUST to feed RAND_seed() with lots of interesting
           and varied data before using RSA_generate_key(). */
        while (RAND_status() == 0) {
            //unsigned long rand_ret = time(NULL);
            unsigned long rand_ret = rand() % 65535;
            RAND_seed(&rand_ret, sizeof(rand_ret));
        }

        ret = EVP_PKEY_assign_RSA(pkey, RSA_generate_key(key_bits, RSA_F4, NULL, NULL));
        if (!ret)
            goto err;
        ok = 1;
    } else {  // dsa
        bio_err = BIO_new(BIO_s_file_internal());
        if (bio_err == NULL)
            goto err;
            
	    BN_GENCB_set(&cb, dsa_cb, bio_err);
	    if ((dsa = DSA_new()) == NULL) {
            goto err;
        }

        ret = DSA_generate_parameters_ex(dsa, key_bits,
	    			dsa_seed, 20, &counter, &h, &cb);
	    if (!ret)
            goto err;

        DSA_generate_key(dsa);
        ret = EVP_PKEY_assign_DSA(pkey, dsa);
        if (!ret)
            goto err;
        ok = 1;
    }

err:
    if (bio_err)
        BIO_free(bio_err);
    if (ok)
        return pkey;

    if (pkey)
        EVP_PKEY_free(pkey);
    return (EVP_PKEY *) NULL;
}

int cyg_https_add_subject_entry(X509_NAME *subject_name, char *key, char *value)
{
    int nid;
    X509_NAME_ENTRY *ent;

    if ((nid = OBJ_txt2nid(key)) == NID_undef)
        return -1;

    ent = X509_NAME_ENTRY_create_by_NID(NULL, nid, MBSTRING_ASC /*MBSTRING_UTF8*/, (unsigned char*) value, -1);
    if (ent == NULL)
        return -1;

    if (X509_NAME_add_entry(subject_name, ent, -1, 0) != 1)
        return -1;

    return 0;
}

X509_NAME *cyg_https_generate_req(X509_REQ *req)
{
    X509_NAME *subject_name = NULL;

    /* Create a new subject */
    if ((subject_name = X509_NAME_new()) == NULL)
        return (X509_NAME *)NULL;

    /* Add subject entries,
       Refer to ..\eCos\packages\net\openssl\current\include\obj_mac.h */
    if (strlen(https_new_cert_info.country))
        cyg_https_add_subject_entry(subject_name, "countryName", https_new_cert_info.country);
    if (strlen(https_new_cert_info.state))
        cyg_https_add_subject_entry(subject_name, "stateOrProvinceName", https_new_cert_info.state);
    if (strlen(https_new_cert_info.locality))
        cyg_https_add_subject_entry(subject_name, "localityName", https_new_cert_info.locality);
    if (strlen(https_new_cert_info.organization))
        cyg_https_add_subject_entry(subject_name, "organizationName", https_new_cert_info.organization);
    if (strlen(https_new_cert_info.mail))
        cyg_https_add_subject_entry(subject_name, "Mail", https_new_cert_info.mail);
    if (strlen(https_new_cert_info.common))
        cyg_https_add_subject_entry(subject_name, "commonName", https_new_cert_info.common);
    else
        cyg_https_add_subject_entry(subject_name, "commonName", "E-Stax-34");
    if (!X509_REQ_set_subject_name(req, subject_name))
        return (X509_NAME *)NULL;

    return subject_name;
}

int cyg_https_generate_cert(char *server_cert, char *server_private_key, char *server_pass_phrase, cyg_int32 is_rsa)
{
    int ok = 0;
    X509_REQ *req = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *tmp_cert = NULL;
    X509_NAME *subject_name = NULL;
    long time;
    time_t base_time = 0;

    req = X509_REQ_new();

    /* Nessus 60108 - SSL Certificate Chain Contains Weak RSA Keys.
       1. Generate private key, at least 1024 bits */
    if ((pkey = cyg_https_generate_key(1024, is_rsa)) == NULL)
        goto err;
    X509_REQ_set_pubkey(req, pkey);

    /* 2. Generate require */
    if ((subject_name = cyg_https_generate_req(req)) == NULL)
        goto err;

    /* 3. Generate certificate */
    if ((tmp_cert = X509_new()) == NULL)
        goto err;

    /* 3.1 Set certificate version
       0: X509_V1    1: X509_V2    2: X509_V3 */
    if (!X509_set_version(tmp_cert, 2))
        goto err;

    /* 3.2 Set certificate serial number (need > 1, browser don't accept serial number 1) */
    if (!ASN1_INTEGER_set(X509_get_serialNumber(tmp_cert), 2))
        goto err;

    /*3.3 Set certificate valid start time
          The time base is 00:00 Jan 1 1970;
          We set valid start time from 2010/01/01
          and the 10 of these years were leap years.
          60(s) * 60(m) * 24(h) * (365(d) * 40 + 10) */
    time = 60 * 60 * 24 * (365 * 40 + 10);
    if (!X509_time_adj(X509_get_notBefore(tmp_cert), time, &base_time))
        goto err;

    /* 3.4 Set certificate valid end time (20 years) */
    time += 60 * 60 * 24 * (365 * 20 + 4);
    if (!X509_time_adj(X509_get_notAfter(tmp_cert), time, &base_time))
        goto err;

    /* 3.5 Set certificate subject name */
    if (!X509_set_subject_name(tmp_cert, X509_REQ_get_subject_name(req)))
        goto err;

    /* 3.6 Set certificate pulic key */
    if (!X509_set_pubkey(tmp_cert, pkey))
        goto err;

    /* 3.7 Set certificate issuer name */
    if (!X509_set_issuer_name(tmp_cert, subject_name))
        goto err;

    /* 3.8 Set certificate sign */
    if (!X509_sign(tmp_cert, pkey, is_rsa ? EVP_sha1() : EVP_dss1()))
        goto err;

    /* Until now, a new certificate should be generated success! */

    /* Save private key */
    if (cyg_https_save_pkey(server_private_key, pkey, EVP_des_ede3_cbc(), server_pass_phrase))
        goto err;

    /* Save certificate */
    if (cyg_https_save_cert(server_cert, tmp_cert))
        goto err;

    ok = 1;

err:
    /* Free allocate resources */
    if (subject_name)
        X509_NAME_free(subject_name);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (req)
        X509_REQ_free(req);
    if (tmp_cert)
        X509_free(tmp_cert);

    if (ok)
        return 0;
    else
        return -1;
}

void cyg_https_set_mode(int mode)
{
    if (https_running_flag == mode)
        return;

    https_running_flag = mode;
    if (!redirect_to_https_flag)
        redirect_to_https_flag = 0;

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
    cyg_http_release_session(1, 0);
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

    if (mode) { // change HTTPS mode from disable to enable
    https_listener = cyg_https_init_socket(CYGNUM_NET_ATHTTPD_HTTPS_SERVEROPT_PORT);
    if (https_listener < 0)
        return;
#ifdef CYGPKG_NET_FREEBSD_INET6
    https_listener_v6 = cyg_https_init_socket_v6(CYGNUM_NET_ATHTTPD_HTTPS_SERVEROPT_PORT);
    if (https_listener_v6 < 0)
        return;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
    } else { // change HTTPS mode from enable to disable
        if (https_listener >= 0) {
            close(https_listener);
            https_listener = -1;
        }
#ifdef CYGPKG_NET_FREEBSD_INET6
        if (https_listener_v6 >= 0) {
            close(https_listener_v6);
            https_listener_v6 = -1;
        }
#endif /* CYGPKG_NET_FREEBSD_INET6 */
    }
}

void cyg_https_shutdown(void)
{
    cyg_https_set_mode(0);
    if (server_ctx)
        SSL_CTX_free(server_ctx);
    server_ctx = NULL;
}

#if 0 // Used for verification mode
static int session_id_context = 0;

static int cyg_https_verify_callback(int ok, X509_STORE_CTX *ctx)
{
	char *s, buf[256], err[296];

	s = X509_NAME_oneline(X509_get_subject_name(ctx->current_cert), buf, 256);
	if (s != NULL) {
		if (ok) {
			sprintf(err, "depth=%d %s\n", ctx->error_depth, buf);
		} else {
			sprintf(err, "depth=%d error=%d %s\n", ctx->error_depth, ctx->error, buf);
        }
    }

	if (ok == 0) {
		switch (ctx->error) {
		    case X509_V_ERR_CERT_NOT_YET_VALID:
		    case X509_V_ERR_CERT_HAS_EXPIRED:
		    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			ok = 1;
		}
    }

	return(ok);
}
#endif

#define CYG_HTTPS_DEFAULT_DH_PARAMETERS_512 "-----BEGIN DH PARAMETERS-----\n\
MEYCQQDaWDwW2YUiidDkr3VvTMqS3UvlM7gE+w/tlO+cikQD7VdGUNNpmdsp13Yn\n\
a6LT1BLiGPTdHghM9tgAPnxHdOgzAgEC\n\
-----END DH PARAMETERS-----\n\
"
static const char rnd_seed[] = "string to make the random number generator think it has entropy";

#ifndef NO_RSA
static RSA *temp_rsa = NULL;
#endif

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_start(char *server_cert, char *server_private_key, char *server_pass_phrase, char *dh_parameters)
{
    int rc;
    BIO *in = NULL;
    DH *dh;
    long ssl_option;

	RAND_seed(rnd_seed, sizeof(rnd_seed));
    /* Inital SSL library */
	SSL_library_init();
    SSLeay_add_all_algorithms();
	SSL_load_error_strings();

    /* 1. Create a new SSL CTX */
	/* Nessus 20007 - SSL Version 2 (v2) Protocol Detection.
    server_ctx = SSL_CTX_new(TLSv1_server_method()); */
	server_ctx = SSL_CTX_new(SSLv23_server_method());
    CYG_ASSERT(server_ctx != NULL, "SSL_CTX_new() failed");
	if (server_ctx == NULL)
		return -1;
    ssl_option = SSL_CTX_get_options(server_ctx);
    ssl_option |= SSL_OP_NO_SSLv2;
    SSL_CTX_set_options(server_ctx, ssl_option);

    /* 2. Use encryption parameter that provide algorithms
          for encrypting the key exchanges.
          The key is inherited by all ssl objects created from ctx */
    in = BIO_new(BIO_s_file_internal());
    if (in != NULL) {
        if (BIO_read_filename(in, dh_parameters ? dh_parameters : CYG_HTTPS_DEFAULT_DH_PARAMETERS_512) <= 0) {
            BIO_free(in);
        } else if ((dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL)) == NULL) {
            BIO_free(in);
        } else {
            SSL_CTX_set_tmp_dh(server_ctx, dh);
            BIO_free(in);
            DH_free(dh);
        }
    }

#ifndef NO_RSA
    if (!temp_rsa) {
        temp_rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
    }
    rc = SSL_CTX_set_tmp_rsa(server_ctx, temp_rsa);
    if (!rc)
		return -1;
#endif

    rc = SSL_CTX_load_verify_locations(server_ctx, server_cert, NULL);
	if (!rc)
		return -1;
    rc = SSL_CTX_set_default_verify_paths(server_ctx);
	if (!rc)
		return -1;

    /* 3. Set server certificate */
    rc = SSL_CTX_use_certificate_file(server_ctx, server_cert, SSL_FILETYPE_PEM);
	if (!rc)
		return -1;

    /* 4. Set the private key password,
       MUST to set SSL_CTX_set_default_passwd_cb_userdata()
       before using SSL_CTX_use_PrivateKey_file(). */
    SSL_CTX_set_default_passwd_cb_userdata(server_ctx, server_pass_phrase);

    /* 5. Set the private key,
       5.1 NEED corresponding to the server certificate
       5.2 MUST set the private key password first
       5.3 Certificate file may include private keky or not */
    rc = SSL_CTX_use_PrivateKey_file(server_ctx, (server_private_key ? server_private_key : server_cert), SSL_FILETYPE_PEM);
	if (!rc)
		return -1;

    /* 6. Check if the server certificate and private key matches */
    rc = SSL_CTX_check_private_key(server_ctx);
	if (!rc)
		return -1;

    /* 7. Set verification mode.
          If the verification falg is "SSL_VERIFY_PEER", then we must set session ID context and callback function */
	SSL_CTX_set_verify(server_ctx, SSL_VERIFY_NONE, NULL); /* Don't verify */
    //rc = SSL_CTX_set_session_id_context(server_ctx, (void *)&session_id_context, sizeof(session_id_context));
    //if (!rc)
	//	return -1;
    //SSL_CTX_set_verify(server_ctx, SSL_VERIFY_PEER, cyg_https_verify_callback);

    return 0;
}

static cyg_int32 cyg_https_init_socket(unsigned long port)
{
    cyg_int32 rc;

    cyg_int32 fd = socket(AF_INET, SOCK_STREAM, 0);
    //CYG_ASSERT(fd != -1, "Socket create failed");
    if (fd < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&(httpstate.https_server_conn), 0, sizeof(struct sockaddr_in));
    httpstate.https_server_conn.sin_family = AF_INET;
    httpstate.https_server_conn.sin_addr.s_addr = INADDR_ANY;
    httpstate.https_server_conn.sin_port = htons(port);
    while (bind(fd,
                (struct sockaddr *)&httpstate.https_server_conn,
                sizeof(struct sockaddr)) < 0)
        cyg_thread_delay(CYG_HTTPD_MSEC2TICK(1000));

    rc = listen(fd, SOMAXCONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0)
        return -1;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Web server HTTPS IPv4 Started and listening...\n");
#endif

    return fd;
}

#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 cyg_https_init_socket_v6(unsigned long port)
{
    cyg_int32 rc;

    cyg_int32 fd = socket(AF_INET6, SOCK_STREAM, 0);
    //CYG_ASSERT(fd != -1, "Socket create failed");
    if (fd < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&(httpstate.https_server_conn_v6), 0, sizeof(struct sockaddr_in6));
    httpstate.https_server_conn_v6.sin6_family = AF_INET6;
    httpstate.https_server_conn_v6.sin6_len = sizeof(struct sockaddr_in6);
    httpstate.https_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[0] = INADDR_ANY;
    httpstate.https_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[1] = INADDR_ANY;
    httpstate.https_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[2] = INADDR_ANY;
    httpstate.https_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[3] = INADDR_ANY;
    httpstate.https_server_conn_v6.sin6_port = htons(port);
    while (bind(fd,
                (struct sockaddr *)&httpstate.https_server_conn_v6,
                sizeof(struct sockaddr_in6)) < 0)
        cyg_thread_delay(CYG_HTTPD_MSEC2TICK(1000));

    rc = listen(fd, SOMAXCONN);
    CYG_ASSERT(rc == 0, "listen() returned error");
    if (rc != 0)
        return -1;

#if CYGOPT_NET_ATHTTPD_DEBUG_LEVEL > 0
    diag_printf("Web server HTTPS IPv6 Started and listening...\n");
#endif

    return fd;
}
#endif /* CYGPKG_NET_FREEBSD_INET6 */

static ssize_t cyg_https_writev(SSL *ssl, const struct iovec *vector, int count)
{
    int i, len = 0;
    for (i = 0; i < count; i++)
        len += SSL_write(ssl, vector[i].iov_base, vector[i].iov_len);
    return len;
}

static void cyg_httpd_redirect_to_https(int fd)
{
    char *buf_header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char *buf_start = "<html><head><meta http-equiv=\"refresh\" content=\"3; url=https://";
	char *buf_end = "/\"></head><body><p>The HTTPS mode is enabled on this device. Redirect for using an HTTPS connection ...</p></body></html>";

    cyg_httpd_process_header(httpstate.inbuffer);
    sprintf(httpstate.outbuffer, "%s%s%d.%d.%d.%d:%d%s",
            buf_header,
            buf_start,
            httpstate.host[0],
            httpstate.host[1],
            httpstate.host[2],
            httpstate.host[3],
            CYGNUM_NET_ATHTTPD_HTTPS_SERVEROPT_PORT,
            buf_end);
    send(fd, httpstate.outbuffer, strlen(httpstate.outbuffer), 0);
    httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
}

static void cyg_https_redirect_to_httpd(SSL *ssl)
{
    char *buf_header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char *buf_start = "<html><head><meta http-equiv=\"refresh\" content=\"3; url=http://";
	char *buf_end = "/\"></head><body><p>The HTTPS mode is disabled on this device. Redirect for using an HTTP connection ...</p></body></html>";

    cyg_httpd_process_header(httpstate.inbuffer);
    sprintf(httpstate.outbuffer, "%s%s%d.%d.%d.%d:%d%s",
            buf_header,
            buf_start,
            httpstate.host[0],
            httpstate.host[1],
            httpstate.host[2],
            httpstate.host[3],
            CYGNUM_NET_ATHTTPD_SERVEROPT_PORT,
            buf_end);
    SSL_write(ssl, httpstate.outbuffer, strlen(httpstate.outbuffer));
    httpstate.mode |= CYG_HTTPD_MODE_CLOSE_CONN;
}

void cyg_https_set_redirect(int redirect_flag)
{
    if (https_running_flag)
    {
        if (redirect_to_https_flag != redirect_flag)
            redirect_to_https_flag = redirect_flag;
    }
    else
    {
        redirect_to_https_flag = 0;
    }
}

int cyg_https_get_cert_info(char *server_cert, char *cert_info)
{
    int ok = 0;
    BIO *in = NULL, *out = NULL;
    X509 *x = NULL;

    in = BIO_new(BIO_s_file_internal());
    if (in == NULL)
        goto err;
    if (BIO_read_filename(in, server_cert) <= 0)
        goto err;
    x = PEM_read_bio_X509(in, NULL, NULL, NULL);
    if (x == NULL)
        goto err;

    out = BIO_new(BIO_s_file_internal());
    if (out == NULL)
        goto err;
    if (BIO_read_filename(out, cert_info) <= 0)
        goto err;
    X509_print(out, x);

    ok = 1;

err:
    if (in)
        BIO_free(in);
    if (out)
        BIO_free(out);
    if (x)
        X509_free(x);
    if (ok)
        return 0;
    else
        return -1;
}

void cyg_https_set_new_cert_info(cyg_https_new_cert_info_t *new_cert_info)
{
    https_new_cert_info = *new_cert_info;
}

void cyg_https_get_sess_info(char *sess_info, ssize_t sess_info_len)
{
    cyg_int32 i, j, first_unused_ptr, found;
    BIO *in = NULL;
    SSL_SESSION *x;
    ssize_t total_info_len = 0;
    char buffer[512], *ptr[CYGPKG_NET_MAXSOCKETS], *sess_info_ptr = sess_info;

    for (j = 0; j < CYGPKG_NET_MAXSOCKETS; j++) {
        ptr[j] = NULL;
    }
    for (i = 0; i < CYGPKG_NET_MAXSOCKETS; i++) {
        // Get valid session
        if (httpstate.sockets[i].descriptor != -1 &&
            httpstate.sockets[i].ssl) {
            x = SSL_get_session(httpstate.sockets[i].ssl);
            if (x == NULL) {
                continue;
            }

            // Get session information
            in = BIO_new(BIO_s_file_internal());
            if (in == NULL) {
                continue;
            }
            memset(buffer, 0, sizeof(buffer));
            if (BIO_read_filename(in, buffer) <= 0) {
                BIO_free(in);
                continue;
            }
            SSL_SESSION_print(in, x);
            BIO_free(in);

            // Check if this information already existing
            found = 0;
            for (j = 0, first_unused_ptr = -1; j < CYGPKG_NET_MAXSOCKETS; j++) {
                if (ptr[j] != NULL && !strncmp(buffer, ptr[j], strlen(buffer))) {
                    found = 1;
                    break;
                } else if (ptr[j] == NULL && first_unused_ptr == -1) {
                    first_unused_ptr = j;
                }
            }

            // Save new session information
            if (!found && first_unused_ptr != -1) {
                // Check total length
                if ((total_info_len + strlen(buffer) + strlen("----\n")) > sess_info_len) {
                    break;
                }
                ptr[first_unused_ptr] = sess_info_ptr + strlen("----");
                sprintf(sess_info_ptr, "----%s\n", buffer);
                sess_info_ptr = sess_info_ptr + strlen(buffer) + strlen("----\n");
                total_info_len += strlen(buffer) + strlen("----\n");
            }
        }
    }
}

void cyg_https_get_stats(cyg_https_stats_t *stats)
{
    stats->sess_accept              = SSL_CTX_sess_accept(server_ctx);
    stats->sess_accept_renegotiate  = SSL_CTX_sess_accept_renegotiate(server_ctx);
    stats->sess_accept_good         = SSL_CTX_sess_accept_good(server_ctx);
    stats->sess_miss                = SSL_CTX_sess_misses(server_ctx);
    stats->sess_timeout             = SSL_CTX_sess_timeouts(server_ctx);
    stats->sess_cache_full          = SSL_CTX_sess_cache_full(server_ctx);
    stats->sess_hit                 = SSL_CTX_sess_hits(server_ctx);
    stats->sess_cb_hit              = SSL_CTX_sess_cb_hits(server_ctx);
}

void cyg_https_set_session_timeout(long active_sess_timeout, long absolute_sess_timeout)
{
    httpstate.https_active_sess_timeout  = active_sess_timeout;
    httpstate.https_absolute_sess_timeout = absolute_sess_timeout;
    if (active_sess_timeout == 0 && absolute_sess_timeout == 0) {
        cyg_http_release_session(1, 0);
    }
}

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_check_cert(char *cert, char *private_key, char *pass_phrase)
{
    BIO *cert_in = NULL, *key_in = NULL;
    X509 *x509 = NULL;
    EVP_PKEY *pkey = NULL;   
    int ok = 0;

    cert_in = BIO_new(BIO_s_file_internal());
    if (cert_in == NULL)
        goto err;
    if (BIO_read_filename(cert_in, cert) <= 0)
        goto err;
    x509 = PEM_read_bio_X509(cert_in, NULL, NULL, NULL);
    if (x509 == NULL)
        goto err;

    /* Try to read private form this certificate */
    pkey = PEM_read_bio_PrivateKey(cert_in, NULL, NULL, pass_phrase);
    if (pkey == NULL && private_key == NULL) {
        /* Only check public key */
        ok = 1;
        return 0;
    }

    if (pkey == NULL) {
        /* Check private key if we have */
        key_in = BIO_new(BIO_s_file_internal());
        if (key_in == NULL)
            goto err;
        if (BIO_read_filename(key_in, private_key) <= 0)
            goto err;
        
        pkey = PEM_read_bio_PrivateKey(key_in, NULL, NULL, pass_phrase);   
        if (pkey == NULL)   
            goto err;
    }

    // Check private is matched
    if (X509_check_private_key(x509, pkey))
        ok = 1;

err:
    if (cert_in)
        BIO_free(cert_in);
    if (key_in)
        BIO_free(key_in);
    if (x509)
        X509_free(x509);
    if (pkey)
        EVP_PKEY_free(pkey);

    if (ok)
        return 0;
    else
        return -1;
}

/* Return 0: success, others value means failed */
cyg_int32 cyg_https_check_dh_parameters(char *dh_parameters)
{
	BIO *in = NULL;
    DH *dh = NULL;
    int ret, i;

    in = BIO_new(BIO_s_file_internal());
    if (in == NULL)
        return -1;
    if (BIO_read_filename(in, dh_parameters) <= 0) {
        BIO_free(in);
        return -1;
    }

	if ((dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL)) == NULL) {
        BIO_free(in);
        return -1;
    }

	/* Check DH paramters */
	ret = DH_check(dh, &i);

    BIO_free(in);
        
    return (ret == 1 ? 0 : 1);
}
#endif /* CYGOPT_NET_ATHTTPD_HTTPS */

#ifdef CYGOPT_NET_ATHTTPD_USE_COOKIES
char *
cyg_httpd_cookie_varable_find(const char *ref_addr, char *name, ssize_t *len)
{
    char    *ptr, *name_ptr;
    ssize_t name_len = 0;

    *len = name_len;
    if (!ref_addr) {
        return NULL;
    }

    ptr = (char *) ref_addr;
    while (*ptr != 0xD /*CR*/ && *ptr != '\0') {
        if (strncasecmp(ptr, name, strlen(name)) == 0) {
            name_ptr = ptr + strlen(name) + 1; // ignore following character "="
            ptr = name_ptr;
            while (*ptr != ';' && *ptr != 0xD /*CR*/ && *ptr != '\0') {
                name_len++;
                ptr++;
            }
            *len = name_len;
            return (name_ptr); // ignore following character "="
        }
        ptr++;
    }

    return NULL;
}

#if 0
static void cyg_http_release_session_descriptor(int is_ssl, cyg_int32 sess_descriptor)
{
    cyg_int32 i;

    /* lookup sess_id */
    for (i = 0; i < CYGNUM_FILEIO_NFILE; i++) {
        if (httpstate.sessions[i].is_ssl == is_ssl &&
            (httpstate.sessions[i].sess_descriptor == sess_descriptor)) {
                memset(&httpstate.sessions[i], 0, sizeof(httpstate.sessions[i]));
                if (is_ssl && httpstate.https_session_cnt > 0) {
                    httpstate.https_session_cnt--;
                } else if (!is_ssl && httpstate.http_session_cnt > 0) {
                    httpstate.http_session_cnt--;
                }
                return;
            }
    }
}
#endif

/* sess_id = 0 means release all releated sessions */
void cyg_http_release_session(int is_ssl, cyg_int32 sess_id)
{
    cyg_int32 i;

    /* lookup sess_id */
    for (i = 0; i < CYGNUM_FILEIO_NFILE; i++) {
        if (httpstate.sessions[i].is_ssl == is_ssl &&
            (sess_id == 0 || httpstate.sessions[i].sess_id == sess_id)) {
                memset(&httpstate.sessions[i], 0, sizeof(httpstate.sessions[i]));
                if (is_ssl) {
                    httpstate.https_session_cnt--;
                } else {
                    httpstate.http_session_cnt--;
                }
                if (sess_id) {
                    return;
                }
            }
    }
}

void cyg_http_set_session_timeout(long active_sess_timeout, long absolute_sess_timeout)
{
    httpstate.active_sess_timeout  = active_sess_timeout;
    httpstate.absolute_sess_timeout = absolute_sess_timeout;
    if (active_sess_timeout == 0 && absolute_sess_timeout == 0) {
        cyg_http_release_session(0, 0);
    }
}
#endif /* CYGOPT_NET_ATHTTPD_USE_COOKIES */

