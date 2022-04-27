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

#include "includes.h"
#include "session.h"
#include "dbutil.h"
#include "packet.h"
#include "algo.h"
#include "buffer.h"
#include "dss.h"
#include "ssh.h"
#include "random.h"
#include "kex.h"
#include "channel.h"
#include "atomicio.h"
#include "runopts.h"
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

static void checktimeouts(void);
static long select_timeout(void);
static int ident_readln(int fd, char* buf, int count);

/* Global structs storing the state */
#ifdef DROPBEAR_ECOS
struct sshsession ses[DROPBEAR_MAX_SOCK_NUM + 1]; /* GLOBAL */
#else
struct sshsession ses[0]; /* GLOBAL */
#endif /* DROPBEAR_ECOS */

/* need to know if the session struct has been initialised, this way isn't the
 * cleanest, but works OK */
int sessinitdone = 0; /* GLOBAL */

/* this is set when we get SIGINT or SIGTERM, the handler is in main.c */
int exitflag = 0; /* GLOBAL */



/* called only at the start of a session, set up initial state */
void common_session_init(int sock, char* remotehost) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter session_init"))

	ses[child_index].remotehost = remotehost;

	ses[child_index].sock = sock;
	ses[child_index].maxfd = sock;

	ses[child_index].connect_time = 0;
	ses[child_index].last_packet_time = 0;
	
#ifndef DROPBEAR_NO_PIPE
	if (pipe(ses[child_index].signal_pipe) < 0) {
		dropbear_exit("signal pipe failed");
	}
	setnonblocking(ses[child_index].signal_pipe[0]);
	setnonblocking(ses[child_index].signal_pipe[1]);
#endif /* DROPBEAR_NO_PIPE */
	
	kexfirstinitialise(); /* initialise the kex state */

	ses[child_index].writepayload = buf_new(TRANS_MAX_PAYLOAD_LEN);
	ses[child_index].transseq = 0;

	ses[child_index].readbuf = NULL;
	ses[child_index].decryptreadbuf = NULL;
	ses[child_index].payload = NULL;
	ses[child_index].recvseq = 0;

	initqueue(&ses[child_index].writequeue);

	ses[child_index].requirenext = SSH_MSG_KEXINIT;
	ses[child_index].dataallowed = 1; /* we can send data until we actually 
							send the SSH_MSG_KEXINIT */
	ses[child_index].ignorenext = 0;
	ses[child_index].lastpacket = 0;
	ses[child_index].reply_queue_head = NULL;
	ses[child_index].reply_queue_tail = NULL;

	/* set all the algos to none */
	ses[child_index].keys = (struct key_context*)m_malloc(sizeof(struct key_context));
	ses[child_index].newkeys = NULL;
	ses[child_index].keys->recv_algo_crypt = &dropbear_nocipher;
	ses[child_index].keys->trans_algo_crypt = &dropbear_nocipher;
	
	ses[child_index].keys->recv_algo_mac = &dropbear_nohash;
	ses[child_index].keys->trans_algo_mac = &dropbear_nohash;

	ses[child_index].keys->algo_kex = -1;
	ses[child_index].keys->algo_hostkey = -1;
	ses[child_index].keys->recv_algo_comp = DROPBEAR_COMP_NONE;
	ses[child_index].keys->trans_algo_comp = DROPBEAR_COMP_NONE;

#ifndef DISABLE_ZLIB
	ses[child_index].keys->recv_zstream = NULL;
	ses[child_index].keys->trans_zstream = NULL;
#endif

	/* key exchange buffers */
	ses[child_index].session_id = NULL;
	ses[child_index].kexhashbuf = NULL;
	ses[child_index].transkexinit = NULL;
	ses[child_index].dh_K = NULL;
	ses[child_index].remoteident = NULL;

	ses[child_index].chantypes = NULL;

	ses[child_index].allowprivport = 0;

#ifdef DROPBEAR_NO_FORK
	ses[child_index].exit = 0;
#endif /* DROPBEAR_NO_FORK */

	TRACE(("leave session_init"))
}

void session_loop(void(*loophandler)(void)) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	fd_set readfd, writefd;
	struct timeval timeout;
	int val;

	/* main loop, select()s for all sockets in use */
	for(;;) {

		timeout.tv_sec = select_timeout();
		timeout.tv_usec = 0;
		FD_ZERO(&writefd);
		FD_ZERO(&readfd);
		dropbear_assert(ses[child_index].payload == NULL);
		if (ses[child_index].sock != -1) {
			FD_SET(ses[child_index].sock, &readfd);
			if (!isempty(&ses[child_index].writequeue)) {
				FD_SET(ses[child_index].sock, &writefd);
			}
		}
		
#ifndef DROPBEAR_NO_PIPE
		/* We get woken up when signal handlers write to this pipe.
		   SIGCHLD in svr-chansession is the only one currently. */
		FD_SET(ses[child_index].signal_pipe[0], &readfd);
#endif /* DROPBEAR_NO_PIPE */

		/* set up for channels which require reading/writing */
		if (ses[child_index].dataallowed) {
			setchannelfds(&readfd, &writefd);
		}
		val = select(ses[child_index].maxfd+1, &readfd, &writefd, NULL, &timeout);

		if (exitflag) {
			dropbear_exit("Terminated by signal");
		}
		
		if (val < 0 && errno != EINTR) {
			dropbear_exit("Error in select");
		}

		if (val <= 0) {
			/* If we were interrupted or the select timed out, we still
			 * want to iterate over channels etc for reading, to handle
			 * server processes exiting etc. 
			 * We don't want to read/write FDs. */
			FD_ZERO(&writefd);
			FD_ZERO(&readfd);
		}
		
#ifndef DROPBEAR_NO_PIPE
		/* We'll just empty out the pipe if required. We don't do
		any thing with the data, since the pipe's purpose is purely to
		wake up the select() above. */
		if (FD_ISSET(ses[child_index].signal_pipe[0], &readfd)) {
			char x;
			while (read(ses[child_index].signal_pipe[0], &x, 1) > 0) {}
		}
#endif /* DROPBEAR_NO_PIPE */

		/* check for auth timeout, rekeying required etc */
		checktimeouts();

		/* process session socket's incoming/outgoing data */
		if (ses[child_index].sock != -1 && FD_ISSET(ses[child_index].sock, &writefd) && !isempty(&ses[child_index].writequeue)) {
			write_packet();
		}

		if (ses[child_index].sock != -1 && FD_ISSET(ses[child_index].sock, &readfd)) {
			read_packet();
		}
		
		/* Process the decrypted packet. After this, the read buffer
		 * will be ready for a new packet */
		if (ses[child_index].sock != -1 && ses[child_index].payload != NULL) {
			process_packet();
		}
		
		/* if required, flush out any queued reply packets that
		were being held up during a KEX */
		maybe_flush_reply_queue();

#ifdef DROPBEAR_NO_FORK
		if (ses[child_index].exit) {
	        if (ses[child_index].readbuf) {
	        	buf_burn(ses[child_index].readbuf);
	        	buf_free(ses[child_index].readbuf);
	        	ses[child_index].readbuf = NULL;
	        }
	        if (ses[child_index].decryptreadbuf) {
	        	buf_burn(ses[child_index].decryptreadbuf);
	        	buf_free(ses[child_index].decryptreadbuf);
	        	ses[child_index].decryptreadbuf = NULL;
	        }
			break;
        }
#endif /* DROPBEAR_NO_FORK */

		/* process pipes etc for the channels, ses[child_index].dataallowed == 0
		 * during rekeying ) */
		if (ses[child_index].dataallowed) {
#ifdef DROPBEAR_NO_FORK
			if (!ses[child_index].session_id)
				break;
#endif /* DROPBEAR_NO_FORK */
			channelio(&readfd, &writefd);
		}

		if (loophandler) {
			loophandler();
		}
	} /* for(;;) */
	
	/* Not reached */
}

/* clean up a session on exit */
void common_session_cleanup(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}
	
	TRACE(("enter session_cleanup"))
	
	/* we can't cleanup if we don't know the session state */
	if (!sessinitdone) {
		TRACE(("leave session_cleanup: !sessinitdone"))
		return;
	}

    if (ses[child_index].session_id)
	    m_free(ses[child_index].session_id);
    if (ses[child_index].keys) {
	    m_burn(ses[child_index].keys, sizeof(struct key_context));
	    m_free(ses[child_index].keys);
    }

	if (ses[child_index].writepayload) {
		buf_free(ses[child_index].writepayload);
		ses[child_index].writepayload = NULL;
	}
	if (ses[child_index].newkeys) {
		m_free(ses[child_index].newkeys);
		ses[child_index].newkeys = NULL;
	}

	/* key exchange buffers */
	if (ses[child_index].kexhashbuf) {
		buf_free(ses[child_index].kexhashbuf);
		ses[child_index].kexhashbuf = NULL;
	}
	if (ses[child_index].transkexinit) {
		buf_free(ses[child_index].transkexinit);
		ses[child_index].transkexinit = NULL;
	}
	if (ses[child_index].dh_K) {
		mp_clear(ses[child_index].dh_K);
		m_free(ses[child_index].dh_K);
		ses[child_index].dh_K = NULL;
	}
	if (ses[child_index].remoteident) {
		m_free(ses[child_index].remoteident);
		ses[child_index].remoteident = NULL;
	}
	while (ses[child_index].writequeue.count) {
		buffer *writebuf = dequeue(&ses[child_index].writequeue);
		buf_free(writebuf);
	}

	chancleanup();

	TRACE(("leave session_cleanup"))
}


int session_identification(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
            dropbear_exit("Cannot get SSH version from remote identification");
	    return -1;
	}

	/* max length of 255 chars */
	char linebuf[256];
	int len = 0;
	char done = 0;
	int i;

	/* write our version string, this blocks */
	if (atomicio(write, ses[child_index].sock, LOCAL_IDENT "\r\n",
				strlen(LOCAL_IDENT "\r\n")) == DROPBEAR_FAILURE) {
		ses[child_index].remoteclosed();
	}

    /* If they send more than 50 lines, something is wrong */
	for (i = 0; i < 50; i++) {
		len = ident_readln(ses[child_index].sock, linebuf, sizeof(linebuf));

		if (len < 0 && errno != EINTR) {
			/* It failed */
			break;
		}

		if (len >= 4 && memcmp(linebuf, "SSH-", 4) == 0) {
			/* start of line matches */
			done = 1;
			break;
		}
	}

	if (!done) {
		TRACE(("err: %s for '%s'\n", strerror(errno), linebuf))
		ses[child_index].remoteclosed();
	} else {
		/* linebuf is already null terminated */
		ses[child_index].remoteident = m_malloc(len);
		memcpy(ses[child_index].remoteident, linebuf, len);
	}

    /* Shall assume that 2.x will be backwards compatible. */
    if (!ses[child_index].remoteident ||
        (strncmp(ses[child_index].remoteident, "SSH-2.", 6) != 0
            && strncmp(ses[child_index].remoteident, "SSH-1.99-", 9) != 0)) {
        if (ses[child_index].remoteident)
            dropbear_exit("Incompatible remote version '%s'", ses[child_index].remoteident);
        else
            dropbear_exit("Cannot get SSH version from remote identification");
        return -1;
    }

	TRACE(("remoteident: %s", ses[child_index].remoteident))

    return 0;
}

/* returns the length including null-terminating zero on success,
 * or -1 on failure */
static int ident_readln(int fd, char* buf, int count) {
	
	char in;
	int pos = 0;
	int num = 0;
	fd_set fds;
	struct timeval timeout;

	TRACE(("enter ident_readln"))

	if (count < 1) {
		return -1;
	}

	FD_ZERO(&fds);

	/* select since it's a non-blocking fd */
	
	/* leave space to null-terminate */
	while (pos < count-1) {

		FD_SET(fd, &fds);

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		if (select(fd+1, &fds, NULL, NULL, &timeout) < 0) {
			if (errno == EINTR) {
				continue;
			}
			TRACE(("leave ident_readln: select error"))
			return -1;
		}

		checktimeouts();
		
		/* Have to go one byte at a time, since we don't want to read past
		 * the end, and have to somehow shove bytes back into the normal
		 * packet reader */
		if (FD_ISSET(fd, &fds)) {
			num = read(fd, &in, 1);
			/* a "\n" is a newline, "\r" we want to read in and keep going
			 * so that it won't be read as part of the next line */
			if (num < 0) {
				/* error */
				if (errno == EINTR) {
					continue; /* not a real error */
				}
				TRACE(("leave ident_readln: read error"))
				return -1;
			}
			if (num == 0) {
				/* EOF */
				TRACE(("leave ident_readln: EOF"))
				return -1;
			}
			if (in == '\n') {
				/* end of ident string */
				break;
			}
			/* we don't want to include '\r's */
			if (in != '\r') {
				buf[pos] = in;
				pos++;
			}
		}
	}

	buf[pos] = '\0';
	TRACE(("leave ident_readln: return %d", pos+1))
	return pos+1;
}

void send_msg_ignore(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	CHECKCLEARTOWRITE();
	buf_putbyte(ses[child_index].writepayload, SSH_MSG_IGNORE);
	buf_putstring(ses[child_index].writepayload, "", 0);
	encrypt_packet();
}

/* Check all timeouts which are required. Currently these are the time for
 * user authentication, and the automatic rekeying. */
static void checktimeouts(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	time_t now;

	now = time(NULL);
	
	if (ses[child_index].connect_time != 0 && now - ses[child_index].connect_time >= AUTH_TIMEOUT) {
			dropbear_close("Timeout before auth");
	}

	/* we can't rekey if we haven't done remote ident exchange yet */
	if (ses[child_index].remoteident == NULL) {
		return;
	}

	if (!ses[child_index].kexstate.sentkexinit
			&& (now - ses[child_index].kexstate.lastkextime >= KEX_REKEY_TIMEOUT
			|| ses[child_index].kexstate.datarecv+ses[child_index].kexstate.datatrans >= KEX_REKEY_DATA)) {
		TRACE(("rekeying after timeout or max data reached"))
		send_msg_kexinit();
	}
	
	if (opts.keepalive_secs > 0 
		&& now - ses[child_index].last_packet_time >= opts.keepalive_secs) {
		send_msg_ignore();
	}
}

static long select_timeout(void) {
	/* determine the minimum timeout that might be required, so
	as to avoid waking when unneccessary */
	long ret = LONG_MAX;
	if (KEX_REKEY_TIMEOUT > 0)
		ret = MIN(KEX_REKEY_TIMEOUT, ret);
	if (AUTH_TIMEOUT > 0)
		ret = MIN(AUTH_TIMEOUT, ret);
	if (opts.keepalive_secs > 0)
		ret = MIN(opts.keepalive_secs, ret);
	return ret;
}
