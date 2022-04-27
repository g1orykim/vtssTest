/*
 * Dropbear SSH
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
#include "listener.h"
#include "session.h"
#include "dbutil.h"

void listeners_initialise(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	/* just one slot to start with */
	ses[child_index].listeners = (struct Listener**)m_malloc(sizeof(struct Listener*));
	ses[child_index].listensize = 1;
	ses[child_index].listeners[0] = NULL;

}

void set_listener_fds(fd_set * readfds) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned int i, j;
	struct Listener *listener;

	/* check each in turn */
	for (i = 0; i < ses[child_index].listensize; i++) {
		listener = ses[child_index].listeners[i];
		if (listener != NULL) {
			for (j = 0; j < listener->nsocks; j++) {
				FD_SET(listener->socks[j], readfds);
			}
		}
	}
}


void handle_listeners(fd_set * readfds) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned int i, j;
	struct Listener *listener;
	int sock;

	/* check each in turn */
	for (i = 0; i < ses[child_index].listensize; i++) {
		listener = ses[child_index].listeners[i];
		if (listener != NULL) {
			for (j = 0; j < listener->nsocks; j++) {
				sock = listener->socks[j];
				if (FD_ISSET(sock, readfds)) {
					listener->acceptor(listener, sock);
				}
			}
		}
	}
} /* Woo brace matching */


/* acceptor(int fd, void* typedata) is a function to accept connections, 
 * cleanup(void* typedata) happens when cleaning up */
struct Listener* new_listener(int socks[], unsigned int nsocks,
		int type, void* typedata, 
		void (*acceptor)(struct Listener* listener, int sock), 
		void (*cleanup)(struct Listener*)) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return NULL;
	}

	unsigned int i, j;
	struct Listener *newlisten = NULL;
	/* try get a new structure to hold it */
	for (i = 0; i < ses[child_index].listensize; i++) {
		if (ses[child_index].listeners[i] == NULL) {
			break;
		}
	}

	/* or create a new one */
	if (i == ses[child_index].listensize) {
		if (ses[child_index].listensize > MAX_LISTENERS) {
			TRACE(("leave newlistener: too many already"))
			for (j = 0; j < nsocks; j++) {
				close(socks[i]);
			}
			return NULL;
		}
		
		ses[child_index].listeners = (struct Listener**)m_realloc(ses[child_index].listeners,
				(ses[child_index].listensize+LISTENER_EXTEND_SIZE)
				*sizeof(struct Listener*));

		ses[child_index].listensize += LISTENER_EXTEND_SIZE;

		for (j = i; j < ses[child_index].listensize; j++) {
			ses[child_index].listeners[j] = NULL;
		}
	}

	for (j = 0; j < nsocks; j++) {
		ses[child_index].maxfd = MAX(ses[child_index].maxfd, socks[j]);
	}

	TRACE(("new listener num %d ", i))

	newlisten = (struct Listener*)m_malloc(sizeof(struct Listener));
	newlisten->index = i;
	newlisten->type = type;
	newlisten->typedata = typedata;
	newlisten->nsocks = nsocks;
	memcpy(newlisten->socks, socks, nsocks * sizeof(int));
	newlisten->acceptor = acceptor;
	newlisten->cleanup = cleanup;

	ses[child_index].listeners[i] = newlisten;
	return newlisten;
}

/* Return the first listener which matches the type-specific comparison
 * function. Particularly needed for global requests, like tcp */
struct Listener * get_listener(int type, void* typedata,
		int (*match)(void*, void*)) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return NULL;
	}

	unsigned int i;
	struct Listener* listener;

	for (i = 0, listener = ses[child_index].listeners[i]; i < ses[child_index].listensize; i++) {
		if (listener->type == type
				&& match(typedata, listener->typedata)) {
			return listener;
		}
	}

	return NULL;
}

void remove_listener(struct Listener* listener) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned int j;

	if (listener->cleanup) {
		listener->cleanup(listener);
	}

	for (j = 0; j < listener->nsocks; j++) {
		close(listener->socks[j]);
	}
	ses[child_index].listeners[listener->index] = NULL;
	m_free(listener);

}
