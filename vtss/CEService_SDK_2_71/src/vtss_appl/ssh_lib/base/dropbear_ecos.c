/*

 Vitesse Switch API software.

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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
#include <network.h>
#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <time.h>

#include "dropbear_config_ecos.h"
#include "options_ecos.h"
#include "runopts.h"
#include "random.h"
#include "dbutil.h"
#include "session.h"
#include "ssh_lib_callout.h"

/* Global variables */
unsigned long dropbear_ssh_conn_num = 0;

/* Local variables */
static fd_set dropbear_rfds;
static cyg_int32 dropbear_sockets[DROPBEAR_MAX_SOCK_NUM + 1];
static struct sockaddr_in dropbear_server_conn;
#ifdef CYGPKG_NET_FREEBSD_INET6
static struct sockaddr_in6 dropbear_server_conn_v6;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
static unsigned long dropbear_ssh_mode = 0;

/* SSH socket FD */
static cyg_int32 fdmax = 0;
static cyg_int32 listener = -1;
#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 listener_v6 = -1;
#endif /* CYGPKG_NET_FREEBSD_INET6 */

/* Callback function ptr */
static dropbear_exist_log_callback_t dropbear_exist_log_cb = NULL;

/* Thread variables */
#define DROPBEAR_ECOS_THREAD_NAME_MAX  16       /* Maximum thread name */

#define CYG_DROPBEAR_MSEC2TICK(msec) (msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000))

static char ssh_child_name[DROPBEAR_MAX_SOCK_NUM + 1][DROPBEAR_ECOS_THREAD_NAME_MAX];
static char ssh_child_stack[DROPBEAR_MAX_SOCK_NUM + 1][16384];
static cyg_thread ssh_child_block[DROPBEAR_MAX_SOCK_NUM + 1];
static cyg_handle_t ssh_child_handle[DROPBEAR_MAX_SOCK_NUM + 1];

void dropbear_ecos_child_thread(cyg_addrword_t data)
{
    cyg_int32 descr = dropbear_sockets[data];
    char *addrstring;
    struct sockaddr remoteaddr;
    int remoteaddrlen = sizeof(remoteaddr);
    struct sockaddr_in *sockaddr_in;
#ifdef CYGPKG_NET_FREEBSD_INET6
    struct sockaddr_in6 *sockaddr_in6;
    char straddr[INET6_ADDRSTRLEN];
#endif /* CYGPKG_NET_FREEBSD_INET6 */

    dropbear_ssh_conn_num++;
    addrstring = ssh_lib_callout_malloc(64);
    memset(addrstring, 0x0, sizeof(addrstring));
    getpeername(descr, &remoteaddr, (int *)&remoteaddrlen);
    if (remoteaddr.sa_family == AF_INET) {
        sockaddr_in = (struct sockaddr_in *)&remoteaddr;
        snprintf(addrstring, 64 - 1, "%s:%d", inet_ntoa(sockaddr_in->sin_addr), ntohs(sockaddr_in->sin_port));
        /* start the session, dropbear excute session_loop() until end the session */
        svr_session(descr, -1, inet_ntoa(sockaddr_in->sin_addr), addrstring);
    }
#ifdef CYGPKG_NET_FREEBSD_INET6
    else if (remoteaddr.sa_family == AF_INET6) {
        sockaddr_in6 = (struct sockaddr_in6 *)&remoteaddr;
        inet_ntop(AF_INET6, (char *)&(sockaddr_in6->sin6_addr), straddr, sizeof(straddr));
        snprintf(addrstring, 64 - 1, "%s:%d", straddr, ntohs(sockaddr_in6->sin6_port));
        /* start the session, dropbear excute session_loop() until end the session */
        svr_session(descr, -1, straddr, addrstring);
    }
#endif /* CYGPKG_NET_FREEBSD_INET6 */

    ssh_lib_callout_free(addrstring);

    /* end the session */
    /* the session maybe close by cli_io_ssh_close(), we get socket_fd again here */
    descr = dropbear_sockets[data];
    if (descr != -1) {
        close(descr);
        FD_CLR(descr, &dropbear_rfds);
    }
    dropbear_sockets[data] = -1;
    dropbear_ssh_conn_num--;

    /* termination client thread */
    cyg_thread_exit();
}

void dropbear_ecos_create_child_thread(int index)
{
    sprintf(ssh_child_name[index], "SSH Child %01d", index + 1);

    cyg_thread_create(7 /*THREAD_DEFAULT_PRIO*/,            // Priority
                      dropbear_ecos_child_thread,           // entry
                      (cyg_addrword_t)index,                // entry parameter
                      ssh_child_name[index],                // Name
                      &ssh_child_stack[index][0],           // Stack
                      sizeof(ssh_child_stack[index]),       // Size
                      &ssh_child_handle[index],             // Handle
                      &ssh_child_block[index]               // Thread data structure
                     );
    cyg_thread_resume(ssh_child_handle[index]);             // Start it
}

void dropbear_ecos_clean_child_thread(int child_index)
{
    cyg_int32 descr = dropbear_sockets[child_index];
    if (descr != -1)
        close(descr);
    dropbear_sockets[child_index] = -1;
}

static cyg_int32 dropbear_ecos_socket_init(unsigned long port)
{
    int rc;

    cyg_int32 listener = socket(AF_INET, SOCK_STREAM, 0);
    //CYG_ASSERT(listener != -1, "SSH IPv4 Socket create failed");
    if (listener < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&dropbear_server_conn, 0, sizeof(struct sockaddr_in));
    dropbear_server_conn.sin_family = AF_INET;
    dropbear_server_conn.sin_addr.s_addr = INADDR_ANY;
    dropbear_server_conn.sin_port = htons(port);
    while (bind(listener,
                (struct sockaddr *)&dropbear_server_conn,
                sizeof(struct sockaddr)) < 0)
        cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(1000));

    rc = listen(listener, SOMAXCONN);
    CYG_ASSERT(rc == 0, "SSH IPv4 listen() returned error");
    if (rc != 0)
        return -1;

    cyg_int32 i;
    for (i = 0; i < DROPBEAR_MAX_SOCK_NUM + 1; i++)
        dropbear_sockets[i] = -1;

    FD_ZERO(&dropbear_rfds);

    return listener;
}

#ifdef CYGPKG_NET_FREEBSD_INET6
static cyg_int32 dropbear_ecos_socket_init_v6(unsigned long port)
{
    int rc;

    cyg_int32 listener = socket(AF_INET6, SOCK_STREAM, 0);
    //CYG_ASSERT(listener != -1, "SSH IPv6 Socket create failed");
    if (listener < 0)
        return -1;

    cyg_int32 yes = 1;
    rc = setsockopt(listener, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
    if (rc == -1)
        return -1;

    memset(&dropbear_server_conn_v6, 0, sizeof(struct sockaddr_in6));
    dropbear_server_conn_v6.sin6_family = AF_INET6;
    dropbear_server_conn_v6.sin6_len = sizeof(struct sockaddr_in6);
    dropbear_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[0] = INADDR_ANY;
    dropbear_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[1] = INADDR_ANY;
    dropbear_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[2] = INADDR_ANY;
    dropbear_server_conn_v6.sin6_addr.__u6_addr.__u6_addr32[3] = INADDR_ANY;
    dropbear_server_conn_v6.sin6_port = htons(port);
    while (bind(listener,
                (struct sockaddr *)&dropbear_server_conn_v6,
                sizeof(struct sockaddr_in6)) < 0)
        cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(1000));

    rc = listen(listener, SOMAXCONN);
    CYG_ASSERT(rc == 0, "SSH IPv6 listen() returned error");
    if (rc != 0)
        return -1;

    cyg_int32 i;
    for (i = 0; i < DROPBEAR_MAX_SOCK_NUM + 1; i++)
        dropbear_sockets[i] = -1;

    FD_ZERO(&dropbear_rfds);

    return listener;
}
#endif /* CYGPKG_NET_FREEBSD_INET6 */

void dropbear_ecos_handle_new_connection(cyg_int32 listener)
{
    cyg_int32               i;
    int                     fd_client;
    socklen_t               client_len;
    struct sockaddr_storage client_addr;

    client_len = sizeof(client_addr);
    fd_client = accept(listener, (struct sockaddr *)&client_addr, &client_len);
    if (fd_client < 0) {
        return;
    }

    if (dropbear_ssh_mode) {
        for (i = 0; i < DROPBEAR_MAX_SOCK_NUM + 1; i++) {
            if (dropbear_sockets[i] == -1) {
                dropbear_sockets[i] = fd_client;
                dropbear_ecos_create_child_thread(i);
                dropbear_client_ip_port_set(i, (struct sockaddr *)&client_addr, client_len);
                return;
            }
        }
    } else {
        char *buf = "\r\nThis device's SSH mode is disabled, please use the Telnet instead of SSH or enable SSH mode on the device first.\r\n";
        write(fd_client, buf, strlen(buf));
        cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(500));  /* Allow to drain */
    }
    close(fd_client);
}

/* Before dropbear deamon initial, we should initial hostkeky first */
void dropbear_ecos_daemon(void)
{
    cyg_int32 rc;
    struct timeval tv = {DROPBEAR_SOCKET_IDLE_TIMEOUT, 0};

    cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(3000)); /* always wait 3 seconds for CLI thread starting */

    dropbear_local_socket_init();

    _dropbear_exit = svr_dropbear_exit;
    if (dropbear_exist_log_cb) {
        _dropbear_log = dropbear_exist_log_cb;
    } else {
        _dropbear_log = svr_dropbear_log;
    }

    svr_getopts(0, NULL);

    /* Now we can setup the hostkeys - needs to be after logging is on,
     * otherwise we might end up blatting error messages to the socket */
    rc = loadhostkeys();
    CYG_ASSERT(rc != DROPBEAR_FAILURE, "SSH load hostkeys failed");

    seedrandom();

    while (1) {
        cyg_thread_delay(CYG_DROPBEAR_MSEC2TICK(100));
        if (!dropbear_ssh_mode || listener < 0
#ifdef CYGPKG_NET_FREEBSD_INET6
        || listener_v6 < 0
#endif /* CYGPKG_NET_FREEBSD_INET6 */
        ) {
             continue;       
        }

        fdmax = listener;
        FD_SET(listener, &dropbear_rfds);
#ifdef CYGPKG_NET_FREEBSD_INET6
        FD_SET(listener_v6, &dropbear_rfds);
        fdmax = (listener > listener_v6) ? listener : listener_v6;
#endif /* CYGPKG_NET_FREEBSD_INET6 */

        rc = select(fdmax + 1, &dropbear_rfds, NULL, NULL, &tv);
        if (rc > 0) {
            if (FD_ISSET(listener, &dropbear_rfds))
                    dropbear_ecos_handle_new_connection(listener);
#ifdef CYGPKG_NET_FREEBSD_INET6
            else if (FD_ISSET(listener_v6, &dropbear_rfds))
                    dropbear_ecos_handle_new_connection(listener_v6);
#endif /* CYGPKG_NET_FREEBSD_INET6 */
        }
    }
}

unsigned char dropbear_rsa_hostkey[DROPBEAR_MAX_HOSTKEY_LEN];
unsigned long dropbear_rsa_hostkey_len = 0;
unsigned char dropbear_dss_hostkey[DROPBEAR_MAX_HOSTKEY_LEN];
unsigned long dropbear_dss_hostkey_len = 0;

/* Configure hostkey */
void dropbear_hostkey_init(unsigned char *rsa_hostkey, unsigned long rsa_hostkey_len,
                           unsigned char *dss_hostkey, unsigned long dss_hostkey_len)
{
    if (rsa_hostkey_len > DROPBEAR_MAX_HOSTKEY_LEN ||
        dss_hostkey_len > DROPBEAR_MAX_HOSTKEY_LEN)
        return;

    memcpy(dropbear_rsa_hostkey, rsa_hostkey, rsa_hostkey_len);
    dropbear_rsa_hostkey_len = rsa_hostkey_len;
    memcpy(dropbear_dss_hostkey, dss_hostkey, dss_hostkey_len);
    dropbear_dss_hostkey_len = dss_hostkey_len;
}

static char default_passwd [] = "password"; //"hL8nrFDt0aJ3E";
static char default_dir [] = "/";
static char default_shell [] = "sh";

struct passwd *getpwnam(const char *name)
{
    static struct passwd rv;
    rv.pw_name = (char*)name;
    rv.pw_uid = getuid();
    rv.pw_gid = getgid();

    rv.pw_dir = default_dir;
    rv.pw_shell = default_shell;
    rv.pw_passwd = default_passwd;

    return &rv;
}

/* Set SSH mode */
void dropbear_set_ssh_mode(unsigned long mode)
{
    cyg_int32 i;

    if (dropbear_ssh_mode == mode) {
        return;
    }
    dropbear_ssh_mode = mode;
    if (!mode) { // change SSH mode from enable to disable
        for (i = 0; i < DROPBEAR_MAX_SOCK_NUM + 1; i++) {
            if (dropbear_sockets[i] != -1)
                dropbear_ecos_clean_child_thread(i);
        }
        dropbear_cli_child_socket_close();
        if (listener >= 0) {
            close(listener);
            listener = -1;
        }
#ifdef CYGPKG_NET_FREEBSD_INET6
        if (listener_v6 >= 0) {
            close(listener_v6);
            listener_v6 = -1;
        }
#endif /* CYGPKG_NET_FREEBSD_INET6 */
        fdmax = 0;
    } else {
        listener = dropbear_ecos_socket_init(DROPBEAR_SSH_PORT);
        if (listener < 0)
            return;
#ifdef CYGPKG_NET_FREEBSD_INET6
        listener_v6 = dropbear_ecos_socket_init_v6(DROPBEAR_SSH_PORT);
        if (listener_v6 < 0)
            return;
#endif /* CYGPKG_NET_FREEBSD_INET6 */
    }
}

/* Get SSH mode */
void dropbear_get_ssh_mode(unsigned long *mode)
{
    *mode = dropbear_ssh_mode;
}

int get_child_index(void)
{
    int chile_index;
    cyg_handle_t handle;

    handle = cyg_thread_self();
    for (chile_index = 0; chile_index < DROPBEAR_MAX_SOCK_NUM + 1; chile_index++) {
        if (handle == ssh_child_handle[chile_index])
            return chile_index;
    }
    return 0;
}

/* Register dropbear exist log callback function */
void dropbear_exist_log_register(dropbear_exist_log_callback_t cb)
{
    dropbear_exist_log_cb = cb;
}
