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

/* eCos Dropbear SSH Features (2008/11/04)
   ---------------------------------------
    * SSH-2 protocol compatible (SSH-1 not supported)
    * Both public-key and password authentication are supported
    * DSS and RSA key authentication algorithms
    * Only server side has implemented 
    * Agent, X11 forwarding, compression, scp, or sftp does not support
*/

#ifndef _DROPBEAR_ECOS_CONFIG_H_
#define _DROPBEAR_ECOS_CONFIG_H_

#include <stdarg.h>
#include <sys/socket.h> /* for struct sockaddr_storage */

//#define DROPBEAR_ECOS

/* We only implemented server side and doesn't support
   agent or X11 forwarding, compression, scp, or sftp */
#define DROPBEAR_SERVER
#define DROPBEAR_MULTI

/* Our eCos doesn't support file systm, haven't the
`fork' and 'pipe' function; and use embedded CLI instead of shell */
#define DROPBEAR_NO_FP
#define DROPBEAR_NO_FORK
#define DROPBEAR_NO_PIPE
#define DROPBEAR_NO_SHELL
#define DROPBEAR_NO_CRYPT_PASSWORD
//#define DROPBEAR_ALLOW_EMPTY_PASSWORD

/* I/O macros
   Refer to <TOP_DIR>\managed\eCos\packages\hal\synth\arch\current\include\hal_io.h */
#define SIGCHLD         17  //CYG_HAL_SYS_SIGCHLD
#define SIGTTOU         22  //CYG_HAL_SYS_SIGTTOU

#define LOG_EMERG       0
#define LOG_ALERT       1
#define LOG_CRIT        2
#define LOG_ERR         3
#define LOG_WARNING     4
#define LOG_NOTICE      5
#define LOG_INFO        6
#define LOG_DEBUG       7


#define DROPBEAR_MAX_HOSTKEY_LEN        512     //Refer to <TOP_DIR>\managed\sw_mgd_switch\proto\vtss_ssh_api.h
#define DROPBEAR_SSH_PORT               22      //Refer to <TOP_DIR>\managed\sw_ssh\options.h
#define DROPBEAR_SOCKET_IDLE_TIMEOUT    300
#define DROPBEAR_MAX_SOCK_NUM           4
#define DROPBEAR_LOCAL_PORT             1234


/* Configure hostkey */
void dropbear_hostkey_init(unsigned char *rsa_hostkey, unsigned long rsa_hostkey_len,
                           unsigned char *dss_hostkey, unsigned long dss_hostkey_len);

/* Before dropbear deamon initial, we should initial hostkeky first */
void dropbear_ecos_daemon(void);

void dropbear_ecos_clean_child_thread(int child_index);

typedef const char* (*dropbear_get_passwd_callback_t)(void);

/* get_passwd() registration */
void dropbear_get_passwd_register(dropbear_get_passwd_callback_t callback);

struct passwd *getpwnam(const char *name);

/* We use ourselves CLI and don't use fork() in dropbear package.
   That will establish a local socket connection to process receive data.
   There's a sample in dropbear_ecos_localsocket.c.
   And it only work on Estax project currently */
#ifdef DROPBEAR_NO_SHELL
void dropbear_local_socket_init(void);
void dropbear_cli_child_socket_close(void);
#endif /* DROPBEAR_NO_SHELL */

/* Generate hostkey */
void dropbear_generate_hostkey(int keytype, unsigned char *hostkey, unsigned long *hostkey_len);

/* Set SSH mode */
void dropbear_set_ssh_mode(unsigned long mode);

/* Get SSH mode */
void dropbear_get_ssh_mode(unsigned long *mode);

int get_child_index(void);

/* Get public key fingerprint string */
void dropbear_printpubkey(const unsigned char *hostkey, unsigned long hostkey_len, unsigned char *str_buff);

/* Callback function for authentication */
typedef int (*dropbear_auth_callback_t)(int agent, char *username, char *password, int *userlevel);

/* Register dropbear authentication callback function */
void dropbear_auth_register(dropbear_auth_callback_t cb);

/* Set user name and his privilege level */
void dropbear_current_priv_lvl_set(int child_index, char *user_name, int privilege_level);

/* Set client IP and port */
void dropbear_client_ip_port_set(int child_index, struct sockaddr *client_addr, socklen_t client_len);

/* Callback function for dropbear exist log */
typedef void (*dropbear_exist_log_callback_t)(int priority, const char* format, va_list param);

/* Register dropbear exist log callback function */
void dropbear_exist_log_register(dropbear_exist_log_callback_t cb);

#endif /* _DROPBEAR_ECOS_CONFIG_H_ */
