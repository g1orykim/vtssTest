/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002-2004 Matt Johnston
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
#include "packet.h"
#include "session.h"
#include "dbutil.h"
#include "ssh.h"
#include "algo.h"
#include "buffer.h"
#include "kex.h"
#include "random.h"
#include "service.h"
#include "auth.h"
#include "channel.h"

#define MAX_UNAUTH_PACKET_TYPE SSH_MSG_USERAUTH_PK_OK

static void recv_unimplemented(void);

/* process a decrypted packet, call the appropriate handler */
void process_packet(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned char type;
	unsigned int i;

	TRACE(("enter process_packet"))

	type = buf_getbyte(ses[child_index].payload);
	TRACE(("process_packet: packet type = %d", type))

	ses[child_index].lastpacket = type;

	/* These packets we can receive at any time */
	switch(type) {

		case SSH_MSG_IGNORE:
			goto out;
		case SSH_MSG_DEBUG:
			goto out;

		case SSH_MSG_UNIMPLEMENTED:
			/* debugging XXX */
			TRACE(("SSH_MSG_UNIMPLEMENTED"))
			dropbear_exit("received SSH_MSG_UNIMPLEMENTED");
			
		case SSH_MSG_DISCONNECT:
			/* TODO cleanup? */
			dropbear_close("Disconnect received");
	}


	/* This applies for KEX, where the spec says the next packet MUST be
	 * NEWKEYS */
	if (ses[child_index].requirenext != 0) {
		if (ses[child_index].requirenext != type) {
			/* TODO send disconnect? */
			dropbear_exit("unexpected packet type %d, expected %d", type,
					ses[child_index].requirenext);
		} else {
			/* Got what we expected */
			ses[child_index].requirenext = 0;
		}
	}

	/* Check if we should ignore this packet. Used currently only for
	 * KEX code, with first_kex_packet_follows */
	if (ses[child_index].ignorenext) {
		TRACE(("Ignoring packet, type = %d", type))
		ses[child_index].ignorenext = 0;
		goto out;
	}


	/* Kindly the protocol authors gave all the preauth packets type values
	 * less-than-or-equal-to 60 ( == MAX_UNAUTH_PACKET_TYPE ).
	 * NOTE: if the protocol changes and new types are added, revisit this 
	 * assumption */
	if ( !ses[child_index].authstate.authdone && type > MAX_UNAUTH_PACKET_TYPE ) {
		dropbear_exit("received message %d before userauth", type);
		goto out;
	}

	for (i = 0; ; i++) {
		if (ses[child_index].packettypes[i].type == 0) {
			/* end of list */
			break;
		}

		if (ses[child_index].packettypes[i].type == type) {
			ses[child_index].packettypes[i].handler();
			goto out;
		}
	}

	
	/* TODO do something more here? */
	TRACE(("preauth unknown packet"))
	recv_unimplemented();

out:
	buf_free(ses[child_index].payload);
	ses[child_index].payload = NULL;

	TRACE(("leave process_packet"))
}



/* This must be called directly after receiving the unimplemented packet.
 * Isn't the most clean implementation, it relies on packet processing
 * occurring directly after decryption (direct use of ses[child_index].recvseq).
 * This is reasonably valid, since there is only a single decryption buffer */
static void recv_unimplemented(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

    if (!ses[child_index].writepayload)
        return;

	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_UNIMPLEMENTED);
	/* the decryption routine increments the sequence number, we must
	 * decrement */
	buf_putint(ses[child_index].writepayload, ses[child_index].recvseq - 1);

	/* Peter, 2009/7, don't try encrypting packet
	encrypt_packet(); */
}
