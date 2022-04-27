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

/* Validates a user password */

#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"

/* Callback function ptr */
static dropbear_auth_callback_t auth_cb = NULL;

#ifdef ENABLE_SVR_PASSWORD_AUTH

/* Process a password auth request, sending success or failure messages as
 * appropriate */
void svr_auth_password(void) {
	int rc, child_index = get_child_index();
	
#ifdef HAVE_SHADOW_H
	struct spwd *spasswd = NULL;
#endif
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
	unsigned char * password;
	unsigned int passwordlen;

	unsigned int changepw;

        int userlevel;

	passwdcrypt = ses[child_index].authstate.pw_passwd;
#ifdef HAVE_SHADOW_H
	/* get the shadow password if possible */
	spasswd = getspnam(ses[child_index].authstate.pw_name);
	if (spasswd != NULL && spasswd->sp_pwdp != NULL) {
		passwdcrypt = spasswd->sp_pwdp;
	}
#endif

#ifdef DEBUG_HACKCRYPT
	/* debugging crypt for non-root testing with shadows */
	passwdcrypt = DEBUG_HACKCRYPT;
#endif

	/* check for empty password - need to do this again here
	 * since the shadow password may differ to that tested
	 * in auth.c */
#ifndef DROPBEAR_ALLOW_EMPTY_PASSWORD
	if (passwdcrypt[0] == '\0') {
		dropbear_log(LOG_WARNING, "user '%s' has blank password, rejected",
				ses[child_index].authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
	}
#endif /* DROPBEAR_ALLOW_EMPTY_PASSWORD */

	/* check if client wants to change password */
	changepw = buf_getbool(ses[child_index].payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses[child_index].payload, &passwordlen);

	/* the first bytes of passwdcrypt are the salt */
#ifdef DROPBEAR_NO_CRYPT_PASSWORD
    testcrypt = password;
#else
	testcrypt = crypt((char*)password, passwdcrypt);
#endif /* DROPBEAR_NO_CRYPT_PASSWORD */
    /* peter, 2008/10, free 'password' later
	m_burn(password, passwordlen);
	m_free(password); */

    /* peter, 2009/4, add authentication callback function */
    if (auth_cb) {
        rc = auth_cb(1 /* VTSS_AUTH_AGENT_SSH refer to: vtss_auth_api.h */, ses[child_index].authstate.pw_name, password, &userlevel);
    } else {
    	rc = strcmp(testcrypt, passwdcrypt);
    }

    if (!rc) {
		/* successful authentication */
#ifdef DROPBEAR_ECOS
        dropbear_current_priv_lvl_set(child_index, ses[child_index].authstate.pw_name, userlevel);
#endif
		dropbear_log(LOG_NOTICE, 
				"password auth succeeded for '%s' from %s",
				ses[child_index].authstate.pw_name,
				svr_ses[child_index].addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"bad password attempt for '%s' from %s",
				ses[child_index].authstate.pw_name,
				svr_ses[child_index].addrstring);
		send_msg_userauth_failure(0, 1);
	}

	m_burn(password, passwordlen);
	m_free(password);
}

/* Register dropbear authentication callback function */
void dropbear_auth_register(dropbear_auth_callback_t cb)
{
    auth_cb = cb;
}

#endif
