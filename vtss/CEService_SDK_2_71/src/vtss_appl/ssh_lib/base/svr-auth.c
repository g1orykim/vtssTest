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

/* This file (auth.c) handles authentication requests, passing it to the
 * particular type (auth-passwd, auth-pubkey). */

#include "includes.h"
#include "dbutil.h"
#include "session.h"
#include "buffer.h"
#include "ssh.h"
#include "packet.h"
#include "auth.h"
#include "runopts.h"
#ifdef DROPBEAR_ECOS
#include <cyg/hal/hal_diag.h> //HAL_DELAY_US()
#endif /* DROPBEAR_ECOS */

static void authclear(void);
static int checkusername(unsigned char *username, unsigned int userlen);
static void send_msg_userauth_banner(void);

/* initialise the first time for a session, resetting all parameters */
void svr_authinitialise(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	ses[child_index].authstate.failcount = 0;
	ses[child_index].authstate.pw_name = NULL;
	ses[child_index].authstate.pw_dir = NULL;
	ses[child_index].authstate.pw_shell = NULL;
	ses[child_index].authstate.pw_passwd = NULL;
	authclear();
	
}

/* Reset the auth state, but don't reset the failcount. This is for if the
 * user decides to try with a different username etc, and is also invoked
 * on initialisation */
static void authclear(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}
	
	memset(&ses[child_index].authstate, 0, sizeof(ses[child_index].authstate));
#ifdef ENABLE_SVR_PUBKEY_AUTH
	ses[child_index].authstate.authtypes |= AUTH_TYPE_PUBKEY;
#endif
#if defined(ENABLE_SVR_PASSWORD_AUTH) || defined(ENABLE_SVR_PAM_AUTH)
	if (!svr_opts.noauthpass) {
		ses[child_index].authstate.authtypes |= AUTH_TYPE_PASSWORD;
	}
#endif
	if (ses[child_index].authstate.pw_name) {
		m_free(ses[child_index].authstate.pw_name);
	}
	if (ses[child_index].authstate.pw_shell) {
		m_free(ses[child_index].authstate.pw_shell);
	}
	if (ses[child_index].authstate.pw_dir) {
		m_free(ses[child_index].authstate.pw_dir);
	}
	if (ses[child_index].authstate.pw_passwd) {
		m_free(ses[child_index].authstate.pw_passwd);
	}
	
}

/* Send a banner message if specified to the client. The client might
 * ignore this, but possibly serves as a legal "no trespassing" sign */
static void send_msg_userauth_banner(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_userauth_banner"))
	if (svr_opts.banner == NULL) {
		TRACE(("leave send_msg_userauth_banner: banner is NULL"))
		return;
	}

	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_USERAUTH_BANNER);
	buf_putstring(ses[child_index].writepayload, buf_getptr(svr_opts.banner,
				svr_opts.banner->len), svr_opts.banner->len);
	buf_putstring(ses[child_index].writepayload, "en", 2);

	encrypt_packet();
	buf_free(svr_opts.banner);
	svr_opts.banner = NULL;

	TRACE(("leave send_msg_userauth_banner"))
}

/* handle a userauth request, check validity, pass to password or pubkey
 * checking, and handle success or failure */
void recv_msg_userauth_request(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned char *username = NULL, *servicename = NULL, *methodname = NULL;
	unsigned int userlen, servicelen, methodlen;

	TRACE(("enter recv_msg_userauth_request"))

	/* ignore packets if auth is already done */
	if (ses[child_index].authstate.authdone == 1) {
		TRACE(("leave recv_msg_userauth_request: authdone already"))
		return;
	}

	/* send the banner if it exists, it will only exist once */
	if (svr_opts.banner) {
		send_msg_userauth_banner();
	}

	
	username = buf_getstring(ses[child_index].payload, &userlen);
	servicename = buf_getstring(ses[child_index].payload, &servicelen);
	methodname = buf_getstring(ses[child_index].payload, &methodlen);

	/* only handle 'ssh-connection' currently */
	if (servicelen != SSH_SERVICE_CONNECTION_LEN
			&& (strncmp(servicename, SSH_SERVICE_CONNECTION,
					SSH_SERVICE_CONNECTION_LEN) != 0)) {
		
		/* TODO - disconnect here */
		m_free(username);
		m_free(servicename);
		m_free(methodname);
		dropbear_exit("unknown service in auth");
	}

	/* user wants to know what methods are supported */
	if (methodlen == AUTH_METHOD_NONE_LEN &&
			strncmp(methodname, AUTH_METHOD_NONE,
				AUTH_METHOD_NONE_LEN) == 0) {
		TRACE(("recv_msg_userauth_request: 'none' request"))
		send_msg_userauth_failure(0, 0);
		goto out;
	}
	
	/* check username is good before continuing */
	if (checkusername(username, userlen) == DROPBEAR_FAILURE) {
		/* username is invalid/no shell/etc - send failure */
		TRACE(("sending checkusername failure"))
		send_msg_userauth_failure(0, 1);
		goto out;
	}

#ifdef ENABLE_SVR_PASSWORD_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses[child_index].authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_password();
			goto out;
		}
	}
#endif

#ifdef ENABLE_SVR_PAM_AUTH
	if (!svr_opts.noauthpass &&
			!(svr_opts.norootpass && ses[child_index].authstate.pw_uid == 0) ) {
		/* user wants to try password auth */
		if (methodlen == AUTH_METHOD_PASSWORD_LEN &&
				strncmp(methodname, AUTH_METHOD_PASSWORD,
					AUTH_METHOD_PASSWORD_LEN) == 0) {
			svr_auth_pam();
			goto out;
		}
	}
#endif

#ifdef ENABLE_SVR_PUBKEY_AUTH
	/* user wants to try pubkey auth */
	if (methodlen == AUTH_METHOD_PUBKEY_LEN &&
			strncmp(methodname, AUTH_METHOD_PUBKEY,
				AUTH_METHOD_PUBKEY_LEN) == 0) {
		svr_auth_pubkey();
		goto out;
	}
#endif

	/* nothing matched, we just fail */
	send_msg_userauth_failure(0, 1);

out:

	m_free(username);
	m_free(servicename);
	m_free(methodname);
}

static void fill_passwd(const char* username) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	struct passwd *pw = NULL;
	if (ses[child_index].authstate.pw_name)
		m_free(ses[child_index].authstate.pw_name);
	if (ses[child_index].authstate.pw_dir)
		m_free(ses[child_index].authstate.pw_dir);
	if (ses[child_index].authstate.pw_shell)
		m_free(ses[child_index].authstate.pw_shell);
	if (ses[child_index].authstate.pw_passwd)
		m_free(ses[child_index].authstate.pw_passwd);

	pw = getpwnam(username);
	if (!pw) {
		return;
	}
	ses[child_index].authstate.pw_uid = pw->pw_uid;
	ses[child_index].authstate.pw_gid = pw->pw_gid;
	ses[child_index].authstate.pw_name = m_strdup(pw->pw_name);
	ses[child_index].authstate.pw_dir = m_strdup(pw->pw_dir);
	ses[child_index].authstate.pw_shell = m_strdup(pw->pw_shell);
	ses[child_index].authstate.pw_passwd = m_strdup(pw->pw_passwd);
}


/* Check that the username exists, has a non-empty password, and has a valid
 * shell.
 * returns DROPBEAR_SUCCESS on valid username, DROPBEAR_FAILURE on failure */
static int checkusername(unsigned char *username, unsigned int userlen) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return DROPBEAR_FAILURE;
	}

#ifndef DROPBEAR_NO_SHELL
	char* listshell = NULL;
#endif /* DROPBEAR_NO_SHELL */
	char* usershell = NULL;
	TRACE(("enter checkusername"))
	if (userlen > MAX_USERNAME_LEN) {
		return DROPBEAR_FAILURE;
	}

	/* new user or username has changed */
	if (ses[child_index].authstate.username == NULL ||
		strcmp(username, ses[child_index].authstate.username) != 0) {
			/* the username needs resetting */
			if (ses[child_index].authstate.username != NULL) {
				dropbear_log(LOG_WARNING, "client trying multiple usernames from %s",
							svr_ses[child_index].addrstring);
				m_free(ses[child_index].authstate.username);
			}
			authclear();
			fill_passwd(username);
			ses[child_index].authstate.username = m_strdup(username);
	}

	/* check that user exists */
	if (!ses[child_index].authstate.pw_name) {
		TRACE(("leave checkusername: user '%s' doesn't exist", username))
		dropbear_log(LOG_WARNING,
				"login attempt for nonexistent user from %s",
				svr_ses[child_index].addrstring);
		send_msg_userauth_failure(0, 1);
		return DROPBEAR_FAILURE;
	}

	/* check for non-root if desired */
	if (svr_opts.norootlogin && ses[child_index].authstate.pw_uid == 0) {
		TRACE(("leave checkusername: root login disabled"))
		dropbear_log(LOG_WARNING, "root login rejected");
		send_msg_userauth_failure(0, 1);
		return DROPBEAR_FAILURE;
	}

	/* check for an empty password */
#ifndef DROPBEAR_ALLOW_EMPTY_PASSWORD
	if (ses[child_index].authstate.pw_passwd[0] == '\0') {
		TRACE(("leave checkusername: empty pword"))
		dropbear_log(LOG_WARNING, "user '%s' has blank password, rejected",
				ses[child_index].authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return DROPBEAR_FAILURE;
	}
#endif /* DROPBEAR_ALLOW_EMPTY_PASSWORD */

	TRACE(("shell is %s", ses[child_index].authstate.pw_shell))

	/* check that the shell is set */
	usershell = ses[child_index].authstate.pw_shell;
	if (usershell[0] == '\0') {
		/* empty shell in /etc/passwd means /bin/sh according to passwd(5) */
		usershell = "/bin/sh";
	}

#ifndef DROPBEAR_NO_SHELL
	/* check the shell is valid. If /etc/shells doesn't exist, getusershell()
	 * should return some standard shells like "/bin/sh" and "/bin/csh" (this
	 * is platform-specific) */
	setusershell();
	while ((listshell = getusershell()) != NULL) {
		TRACE(("test shell is '%s'", listshell))
		if (strcmp(listshell, usershell) == 0) {
			/* have a match */
			goto goodshell;
		}
	}
	/* no matching shell */
	endusershell();
	TRACE(("no matching shell"))
	dropbear_log(LOG_WARNING, "user '%s' has invalid shell, rejected",
				ses[child_index].authstate.pw_name);
	send_msg_userauth_failure(0, 1);
	return DROPBEAR_FAILURE;
	
goodshell:
#endif /* DROPBEAR_NO_SHELL */
	endusershell();
	TRACE(("matching shell"))

	TRACE(("uid = %d", ses[child_index].authstate.pw_uid))
	TRACE(("leave checkusername"))
	return DROPBEAR_SUCCESS;

}

/* Send a failure message to the client, in responds to a userauth_request.
 * Partial indicates whether to set the "partial success" flag,
 * incrfail is whether to count this failure in the failure count (which
 * is limited. This function also handles disconnection after too many
 * failures */
void send_msg_userauth_failure(int partial, int incrfail) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	buffer *typebuf = NULL;

	TRACE(("enter send_msg_userauth_failure"))

	CHECKCLEARTOWRITE();
	
	buf_putbyte(ses[child_index].writepayload, SSH_MSG_USERAUTH_FAILURE);

	/* put a list of allowed types */
	typebuf = buf_new(30); /* long enough for PUBKEY and PASSWORD */

	if (ses[child_index].authstate.authtypes & AUTH_TYPE_PUBKEY) {
		buf_putbytes(typebuf, AUTH_METHOD_PUBKEY, AUTH_METHOD_PUBKEY_LEN);
		if (ses[child_index].authstate.authtypes & AUTH_TYPE_PASSWORD) {
			buf_putbyte(typebuf, ',');
		}
	}
	
	if (ses[child_index].authstate.authtypes & AUTH_TYPE_PASSWORD) {
		buf_putbytes(typebuf, AUTH_METHOD_PASSWORD, AUTH_METHOD_PASSWORD_LEN);
	}

	buf_setpos(typebuf, 0);
	buf_putstring(ses[child_index].writepayload, buf_getptr(typebuf, typebuf->len),
			typebuf->len);

	TRACE(("auth fail: methods %d, '%s'", ses[child_index].authstate.authtypes,
				buf_getptr(typebuf, typebuf->len)));

	buf_free(typebuf);

	buf_putbyte(ses[child_index].writepayload, partial ? 1 : 0);
	encrypt_packet();

	if (incrfail) {
#ifndef DROPBEAR_ECOS
		usleep(300000); /* XXX improve this */
#else
		HAL_DELAY_US(3000);
#endif /* DROPBEAR_ECOS */
		ses[child_index].authstate.failcount++;
	}

	if (ses[child_index].authstate.failcount >= MAX_AUTH_TRIES) {
		char * userstr;
		/* XXX - send disconnect ? */
		TRACE(("Max auth tries reached, exiting"))

		if (ses[child_index].authstate.pw_name == NULL) {
			userstr = "is invalid";
		} else {
			userstr = ses[child_index].authstate.pw_name;
		}
		dropbear_exit("Max auth tries reached - user '%s' from %s",
				userstr, svr_ses[child_index].addrstring);
	}
	
	TRACE(("leave send_msg_userauth_failure"))
}

/* Send a success message to the user, and set the "authdone" flag */
void send_msg_userauth_success(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_userauth_success"))

	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_USERAUTH_SUCCESS);
	encrypt_packet();

	ses[child_index].authstate.authdone = 1;
	ses[child_index].connect_time = 0;


	if (ses[child_index].authstate.pw_uid == 0) {
		ses[child_index].allowprivport = 1;
	}

	/* Remove from the list of pre-auth sockets. Should be m_close(), since if
	 * we fail, we might end up leaking connection slots, and disallow new
	 * logins - a nasty situation. */							
#ifndef DROPBEAR_NO_PIPE
	m_close(svr_ses[child_index].childpipe);
#endif /* DROPBEAR_NO_PIPE */

	TRACE(("leave send_msg_userauth_success"))

}
