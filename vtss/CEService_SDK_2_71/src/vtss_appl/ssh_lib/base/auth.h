/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef _AUTH_H_
#define _AUTH_H_

#include "includes.h"

void svr_authinitialise(void);
void cli_authinitialise(void);

/* Server functions */
void recv_msg_userauth_request(void);
void send_msg_userauth_failure(int partial, int incrfail);
void send_msg_userauth_success(void);
void svr_auth_password(void);
void svr_auth_pubkey(void);
void svr_auth_pam(void);

/* Client functions */
void recv_msg_userauth_failure(void);
void recv_msg_userauth_success(void);
void recv_msg_userauth_specific_60(void);
void recv_msg_userauth_pk_ok(void);
void recv_msg_userauth_info_request(void);
void cli_get_user(void);
void cli_auth_getmethods(void);
void cli_auth_try(void);
void recv_msg_userauth_banner(void);
void cli_pubkeyfail(void);
void cli_auth_password(void);
int cli_auth_pubkey(void);
void cli_auth_interactive(void);
char* getpass_or_cancel(char* prompt);


#define MAX_USERNAME_LEN 25 /* arbitrary for the moment */

#define AUTH_TYPE_NONE      1
#define AUTH_TYPE_PUBKEY    1 << 1
#define AUTH_TYPE_PASSWORD  1 << 2
#define AUTH_TYPE_INTERACT  1 << 3

#define AUTH_METHOD_NONE "none"
#define AUTH_METHOD_NONE_LEN 4
#define AUTH_METHOD_PUBKEY "publickey"
#define AUTH_METHOD_PUBKEY_LEN 9
#define AUTH_METHOD_PASSWORD "password"
#define AUTH_METHOD_PASSWORD_LEN 8
#define AUTH_METHOD_INTERACT "keyboard-interactive"
#define AUTH_METHOD_INTERACT_LEN 20



/* This structure is shared between server and client - it contains
 * relatively little extraneous bits when used for the client rather than the
 * server */
struct AuthState {

	char *username; /* This is the username the client presents to check. It
					   is updated each run through, used for auth checking */
	unsigned char authtypes; /* Flags indicating which auth types are still 
								valid */
	unsigned int failcount; /* Number of (failed) authentication attempts.*/
	unsigned authdone : 1; /* 0 if we haven't authed, 1 if we have. Applies for
							  client and server (though has differing [obvious]
							  meanings). */
	unsigned perm_warn : 1; /* Server only, set if bad permissions on 
							   ~/.ssh/authorized_keys have already been
							   logged. */

	/* These are only used for the server */
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_dir;
	char *pw_shell;
	char *pw_name;
	char *pw_passwd;
};

struct SignKeyList;
/* A singly linked list of signing keys */
struct SignKeyList {

	sign_key *key;
	int type; /* The type of key */
	struct SignKeyList *next;
	/* filename? or the buffer? for encrypted keys, so we can later get
	 * the private key portion */

};

#endif /* _AUTH_H_ */
