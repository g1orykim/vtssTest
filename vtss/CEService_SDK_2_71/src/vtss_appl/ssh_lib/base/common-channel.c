/*
 * Dropbear SSH
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

/* Handle the multiplexed channels, such as sessions, x11, agent connections */

#include "includes.h"
#include "session.h"
#include "packet.h"
#include "ssh.h"
#include "buffer.h"
#include "circbuffer.h"
#include "dbutil.h"
#include "channel.h"
#include "ssh.h"
#include "listener.h"
#include "runopts.h"

static void send_msg_channel_open_failure(unsigned int remotechan, int reason,
		const unsigned char *text, const unsigned char *lang);
static void send_msg_channel_open_confirmation(struct Channel* channel,
		unsigned int recvwindow, 
		unsigned int recvmaxpacket);
static void writechannel(struct Channel* channel, int fd, circbuffer *cbuf);
static void send_msg_channel_window_adjust(struct Channel *channel, 
		unsigned int incr);
static void send_msg_channel_data(struct Channel *channel, int isextended);
static void send_msg_channel_eof(struct Channel *channel);
static void send_msg_channel_close(struct Channel *channel);
static void remove_channel(struct Channel *channel);
static void delete_channel(struct Channel *channel);
static void check_in_progress(struct Channel *channel);
static unsigned int write_pending(struct Channel * channel);
static void check_close(struct Channel *channel);
static void close_chan_fd(struct Channel *channel, int fd, int how);

#define FD_UNINIT (-2)
#define FD_CLOSED (-1)

#define ERRFD_IS_READ(channel) ((channel)->extrabuf == NULL)
#define ERRFD_IS_WRITE(channel) (!ERRFD_IS_READ(channel))

/* Initialise all the channels */
void chaninitialise(const struct ChanType *chantypes[]) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	/* may as well create space for a single channel */
	ses[child_index].channels = (struct Channel**)m_malloc(sizeof(struct Channel*));
	ses[child_index].chansize = 1;
	ses[child_index].channels[0] = NULL;
	ses[child_index].chancount = 0;

	ses[child_index].chantypes = chantypes;

#ifdef USING_LISTENERS
	listeners_initialise();
#endif

}

/* Clean up channels, freeing allocated memory */
void chancleanup(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned int i;

	TRACE(("enter chancleanup"))
	for (i = 0; i < ses[child_index].chansize; i++) {
		if (ses[child_index].channels) {
			if (ses[child_index].channels[i] != NULL) {
				TRACE(("channel %d closing", i))
				remove_channel(ses[child_index].channels[i]);
			}
		}
	}
	m_free(ses[child_index].channels);
	TRACE(("leave chancleanup"))
}

/* Create a new channel entry, send a reply confirm or failure */
/* If remotechan, transwindow and transmaxpacket are not know (for a new
 * outgoing connection, with them to be filled on confirmation), they should
 * all be set to 0 */
struct Channel* newchannel(unsigned int remotechan, 
		const struct ChanType *type, 
		unsigned int transwindow, unsigned int transmaxpacket) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return NULL;
	}

	struct Channel * newchan;
	unsigned int i, j;

	TRACE(("enter newchannel"))
	
	/* first see if we can use existing channels */
	for (i = 0; i < ses[child_index].chansize; i++) {
		if (ses[child_index].channels[i] == NULL) {
			break;
		}
	}

	/* otherwise extend the list */
	if (i == ses[child_index].chansize) {
		if (ses[child_index].chansize >= MAX_CHANNELS) {
			TRACE(("leave newchannel: max chans reached"))
			return NULL;
		}

		/* extend the channels */
		ses[child_index].channels = (struct Channel**)m_realloc(ses[child_index].channels,
				(ses[child_index].chansize+CHAN_EXTEND_SIZE)*sizeof(struct Channel*));

		ses[child_index].chansize += CHAN_EXTEND_SIZE;

		/* set the new channels to null */
		for (j = i; j < ses[child_index].chansize; j++) {
			ses[child_index].channels[j] = NULL;
		}

	}
	
	newchan = (struct Channel*)m_malloc(sizeof(struct Channel));
	newchan->type = type;
	newchan->index = i;
	newchan->sent_close = newchan->recv_close = 0;
	newchan->sent_eof = newchan->recv_eof = 0;

	newchan->remotechan = remotechan;
	newchan->transwindow = transwindow;
	newchan->transmaxpacket = transmaxpacket;
	
	newchan->typedata = NULL;
	newchan->writefd = FD_UNINIT;
	newchan->readfd = FD_UNINIT;
	newchan->errfd = FD_CLOSED; /* this isn't always set to start with */
	newchan->initconn = 0;
	newchan->await_open = 0;
	newchan->flushing = 0;

	newchan->writebuf = cbuf_new(opts.recv_window);
	newchan->extrabuf = NULL; /* The user code can set it up */
	newchan->recvwindow = opts.recv_window;
	newchan->recvdonelen = 0;
	newchan->recvmaxpacket = RECV_MAX_PAYLOAD_LEN;

	ses[child_index].channels[i] = newchan;
	ses[child_index].chancount++;

	TRACE(("leave newchannel"))

	return newchan;
}

/* Returns the channel structure corresponding to the channel in the current
 * data packet (ses[child_index].payload must be positioned appropriately).
 * A valid channel is always returns, it will fail fatally with an unknown
 * channel */
static struct Channel* getchannel_msg(const char* kind) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return NULL;
	}

	unsigned int chan;

	chan = buf_getint(ses[child_index].payload);
	if (chan >= ses[child_index].chansize || ses[child_index].channels[chan] == NULL) {
		if (kind) {
			dropbear_exit("%s for unknown channel %d", kind, chan);
		} else {
			dropbear_exit("Unknown channel %d", chan);
		}
	}
	return ses[child_index].channels[chan];
}

struct Channel* getchannel(void) {
	return getchannel_msg(NULL);
}

/* Iterate through the channels, performing IO if available */
void channelio(fd_set *readfds, fd_set *writefds) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	struct Channel *channel;
	unsigned int i;

	/* foreach channel */
	for (i = 0; i < ses[child_index].chansize; i++) {

		channel = ses[child_index].channels[i];
		if (channel == NULL) {
			/* only process in-use channels */
			continue;
		}

		/* read data and send it over the wire */
		if (channel->readfd >= 0 && FD_ISSET(channel->readfd, readfds)) {
			TRACE(("send normal readfd"))
			send_msg_channel_data(channel, 0);
		}

		/* read stderr data and send it over the wire */
		if (ERRFD_IS_READ(channel) && channel->errfd >= 0 
			&& FD_ISSET(channel->errfd, readfds)) {
				TRACE(("send normal errfd"))
				send_msg_channel_data(channel, 1);
		}

		/* write to program/pipe stdin */
		if (channel->writefd >= 0 && FD_ISSET(channel->writefd, writefds)) {
			if (channel->initconn) {
				/* XXX should this go somewhere cleaner? */
				check_in_progress(channel);
				continue; /* Important not to use the channel after
							 check_in_progress(), as it may be NULL */
			}
			writechannel(channel, channel->writefd, channel->writebuf);
		}
		
		/* stderr for client mode */
		if (ERRFD_IS_WRITE(channel)
				&& channel->errfd >= 0 && FD_ISSET(channel->errfd, writefds)) {
			writechannel(channel, channel->errfd, channel->extrabuf);
		}
	
		/* handle any channel closing etc */
		check_close(channel);

	}

	/* Listeners such as TCP, X11, agent-auth */
#ifdef USING_LISTENERS
	handle_listeners(readfds);
#endif
}


/* Returns true if there is data remaining to be written to stdin or
 * stderr of a channel's endpoint. */
static unsigned int write_pending(struct Channel * channel) {

	if (channel->writefd >= 0 && cbuf_getused(channel->writebuf) > 0) {
		return 1;
	} else if (channel->errfd >= 0 && channel->extrabuf && 
			cbuf_getused(channel->extrabuf) > 0) {
		return 1;
	}
	return 0;
}


/* EOF/close handling */
static void check_close(struct Channel *channel) {
	int close_allowed = 0;

	TRACE(("check_close: writefd %d, readfd %d, errfd %d, sent_close %d, recv_close %d",
				channel->writefd, channel->readfd,
				channel->errfd, channel->sent_close, channel->recv_close))
	TRACE(("writebuf size %d extrabuf size %d",
				cbuf_getused(channel->writebuf),
				channel->extrabuf ? cbuf_getused(channel->extrabuf) : 0))

	if (!channel->flushing && channel->type->check_close
		&& channel->type->check_close(channel))
	{
		channel->flushing = 1;
	}
	
	// if a type-specific check_close is defined we will only exit
	// once that has been triggered. this is only used for a server "session"
	// channel, to ensure that the shell has exited (and the exit status 
	// retrieved) before we close things up.	
	if (!channel->type->check_close	
			|| channel->type->check_close(channel)) {
		close_allowed = 1;
	}

	if (channel->recv_close && !write_pending(channel) && close_allowed) {
		if (!channel->sent_close) {
			TRACE(("Sending MSG_CHANNEL_CLOSE in response to same."))
			send_msg_channel_close(channel);
		}
		remove_channel(channel);
		return;
	}

	if (channel->recv_eof && !write_pending(channel) && (channel->writefd >= 0)) {
		close_chan_fd(channel, channel->writefd, SHUT_WR);
	}

	/* Special handling for flushing read data after an exit. We
	   read regardless of whether the select FD was set,
	   and if there isn't data available, the channel will get closed. */
	if (channel->flushing) {
		TRACE(("might send data, flushing"))
		if (channel->readfd >= 0 && channel->transwindow > 0) {
			TRACE(("send data readfd"))
			send_msg_channel_data(channel, 0);
		}
		if (ERRFD_IS_READ(channel) && channel->errfd >= 0 
			&& channel->transwindow > 0) {
			TRACE(("send data errfd"))
			send_msg_channel_data(channel, 1);
		}
	}

	/* If we're not going to send any more data, send EOF */
	if (!channel->sent_eof
			&& channel->readfd == FD_CLOSED 
			&& (ERRFD_IS_WRITE(channel) || channel->errfd == FD_CLOSED)) {
		send_msg_channel_eof(channel);
	}

	/* And if we can't receive any more data from them either, close up */
	if (channel->readfd == FD_CLOSED
			&& (ERRFD_IS_WRITE(channel) || channel->errfd == FD_CLOSED)
			&& !channel->sent_close
			&& close_allowed
			&& !write_pending(channel)) {
		TRACE(("sending close, readfd is closed"))
		send_msg_channel_close(channel);
	}
}

/* Check whether a deferred (EINPROGRESS) connect() was successful, and
 * if so, set up the channel properly. Otherwise, the channel is cleaned up, so
 * it is important that the channel reference isn't used after a call to this
 * function */
static void check_in_progress(struct Channel *channel) {

	int val;
	socklen_t vallen = sizeof(val);

	TRACE(("enter check_in_progress"))

	if (getsockopt(channel->writefd, SOL_SOCKET, SO_ERROR, &val, &vallen)
			|| val != 0) {
		send_msg_channel_open_failure(channel->remotechan,
				SSH_OPEN_CONNECT_FAILED, "", "");
		close(channel->writefd);
		delete_channel(channel);
		TRACE(("leave check_in_progress: fail"))
	} else {
		send_msg_channel_open_confirmation(channel, channel->recvwindow,
				channel->recvmaxpacket);
		channel->readfd = channel->writefd;
		channel->initconn = 0;
		TRACE(("leave check_in_progress: success"))
	}
}


/* Send the close message and set the channel as closed */
static void send_msg_channel_close(struct Channel *channel) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_close"))
	if (channel->type->closehandler) {
		channel->type->closehandler(channel);
	}
	
	CHECKCLEARTOWRITE();

    if (ses[child_index].writepayload) {
	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_CLOSE);
	buf_putint(ses[child_index].writepayload, channel->remotechan);

	encrypt_packet();

	channel->sent_eof = 1;
	channel->sent_close = 1;
    }

    if (channel->readfd >= 0)
	    close_chan_fd(channel, channel->readfd, SHUT_RD);
    if (channel->errfd >= 0)
	    close_chan_fd(channel, channel->errfd, SHUT_RDWR);
    if (channel->writefd >= 0)
	    close_chan_fd(channel, channel->writefd, SHUT_WR);
	TRACE(("leave send_msg_channel_close"))
}

/* call this when trans/eof channels are closed */
static void send_msg_channel_eof(struct Channel *channel) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_eof"))
	CHECKCLEARTOWRITE();

    if (ses[child_index].writepayload) {
	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_EOF);
	buf_putint(ses[child_index].writepayload, channel->remotechan);

	encrypt_packet();

	channel->sent_eof = 1;
    }

	TRACE(("leave send_msg_channel_eof"))
}

/* Called to write data out to the local side of the channel. 
 * Only called when we know we can write to a channel, writes as much as
 * possible */
static void writechannel(struct Channel* channel, int fd, circbuffer *cbuf) {

	int len, maxlen;

	TRACE(("enter writechannel fd %d", fd))

	maxlen = cbuf_readlen(cbuf);

	/* Write the data out */
	len = write(fd, cbuf_readptr(cbuf, maxlen), maxlen);
	if (len <= 0) {
		TRACE(("errno %d len %d", errno, len))
		if (len < 0 && errno != EINTR) {
			close_chan_fd(channel, fd, SHUT_WR);
		}
		TRACE(("leave writechannel: len <= 0"))
		return;
	}
	TRACE(("writechannel wrote %d", len))

	cbuf_incrread(cbuf, len);
	channel->recvdonelen += len;

	/* Window adjust handling */
	if (channel->recvdonelen >= RECV_WINDOWEXTEND) {
		/* Set it back to max window */
		send_msg_channel_window_adjust(channel, channel->recvdonelen);
		channel->recvwindow += channel->recvdonelen;
		channel->recvdonelen = 0;
	}

	dropbear_assert(channel->recvwindow <= opts.recv_window);
	dropbear_assert(channel->recvwindow <= cbuf_getavail(channel->writebuf));
	dropbear_assert(channel->extrabuf == NULL ||
			channel->recvwindow <= cbuf_getavail(channel->extrabuf));
	
	TRACE(("leave writechannel"))
}

/* Set the file descriptors for the main select in session.c
 * This avoid channels which don't have any window available, are closed, etc*/
void setchannelfds(fd_set *readfds, fd_set *writefds) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}
	
	unsigned int i;
	struct Channel * channel;
	
	for (i = 0; i < ses[child_index].chansize; i++) {

		channel = ses[child_index].channels[i];
		if (channel == NULL) {
			continue;
		}

		/* Stuff to put over the wire */
		if (channel->transwindow > 0) {

			if (channel->readfd >= 0) {
				FD_SET(channel->readfd, readfds);
			}
			
			if (ERRFD_IS_READ(channel) && channel->errfd >= 0) {
					FD_SET(channel->errfd, readfds);
			}
		}

		/* Stuff from the wire */
		if ((channel->writefd >= 0 && cbuf_getused(channel->writebuf) > 0 )
				|| channel->initconn) {
				FD_SET(channel->writefd, writefds);
		}

		if (ERRFD_IS_WRITE(channel) && channel->errfd >= 0 
				&& cbuf_getused(channel->extrabuf) > 0 ) {
				FD_SET(channel->errfd, writefds);
		}

	} /* foreach channel */

#ifdef USING_LISTENERS
	set_listener_fds(readfds);
#endif

}

/* handle the channel EOF event, by closing the channel filedescriptor. The
 * channel isn't closed yet, it is left until the incoming (from the program
 * etc) FD is also EOF */
void recv_msg_channel_eof(void) {

	struct Channel * channel;

	TRACE(("enter recv_msg_channel_eof"))

	channel = getchannel_msg("EOF");

	channel->recv_eof = 1;

	check_close(channel);
	TRACE(("leave recv_msg_channel_eof"))
}


/* Handle channel closure(), respond in kind and close the channels */
void recv_msg_channel_close(void) {

	struct Channel * channel;

	TRACE(("enter recv_msg_channel_close"))

	channel = getchannel_msg("Close");

	channel->recv_eof = 1;
	channel->recv_close = 1;

	check_close(channel);
	TRACE(("leave recv_msg_channel_close"))
}

/* Remove a channel entry, this is only executed after both sides have sent
 * channel close */
static void remove_channel(struct Channel * channel) {

	TRACE(("enter remove_channel"))
	TRACE(("channel index is %d", channel->index))

	cbuf_free(channel->writebuf);
	channel->writebuf = NULL;

	if (channel->extrabuf) {
		cbuf_free(channel->extrabuf);
		channel->extrabuf = NULL;
	}


	/* close the FDs in case they haven't been done
	 * yet (they might have been shutdown etc) */
	TRACE(("CLOSE writefd %d", channel->writefd))
	if (channel->writefd >= 0)
	    close(channel->writefd);
	TRACE(("CLOSE readfd %d", channel->readfd))
	if ((channel->readfd >= 0) && (channel->readfd != channel->writefd))
	    close(channel->readfd);
	TRACE(("CLOSE errfd %d", channel->errfd))
	if (channel->errfd >= 0)
		close(channel->errfd);

	if (channel->typedata) {
		struct ChanSess *chansess = (struct ChanSess*)channel->typedata;
		m_free(chansess->term);
		m_free(channel->typedata);
		channel->typedata = NULL;
	}

	delete_channel(channel);

	TRACE(("leave remove_channel"))
}

/* Remove a channel entry */
static void delete_channel(struct Channel *channel) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	ses[child_index].channels[channel->index] = NULL;
	m_free(channel);
	ses[child_index].chancount--;
	
}


/* Handle channel specific requests, passing off to corresponding handlers
 * such as chansession or x11fwd */
void recv_msg_channel_request(void) {

	struct Channel *channel;

	TRACE(("enter recv_msg_channel_request"))
	
	channel = getchannel();

	if (channel->type->reqhandler) {
		channel->type->reqhandler(channel);
	} else {
		send_msg_channel_failure(channel);
	}

	TRACE(("leave recv_msg_channel_request"))

}

/* Reads data from the server's program/shell/etc, and puts it in a
 * channel_data packet to send.
 * chan is the remote channel, isextended is 0 if it is normal data, 1
 * if it is extended data. if it is extended, then the type is in
 * exttype */
static void send_msg_channel_data(struct Channel *channel, int isextended) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	int len;
	size_t maxlen, size_pos;
	int fd;

	CHECKCLEARTOWRITE();

	TRACE(("enter send_msg_channel_data"))
	dropbear_assert(!channel->sent_close);
	if (channel->sent_close) {
		TRACE(("leave send_msg_channel_data: channel is closed"))
		return;
	}

	if (isextended) {
		fd = channel->errfd;
	} else {
		fd = channel->readfd;
	}
	TRACE(("enter send_msg_channel_data isextended %d fd %d", isextended, fd))
	dropbear_assert(fd >= 0);
	if (fd < 0) {
		TRACE(("leave send_msg_channel_data: fd < 0"))
		return;
	}

	if (ses[child_index].writepayload == NULL) {
		TRACE(("leave send_msg_channel_data: NULL point of writepayload"))
		return;
	}

	maxlen = MIN(channel->transwindow, channel->transmaxpacket);
	/* -(1+4+4) is SSH_MSG_CHANNEL_DATA, channel number, string length, and 
	 * exttype if is extended */
	maxlen = MIN(maxlen, 
			ses[child_index].writepayload->size - 1 - 4 - 4 - (isextended ? 4 : 0));
	TRACE(("maxlen %d", maxlen))
	if (maxlen == 0) {
		TRACE(("leave send_msg_channel_data: no window"))
		return;
	}

	buf_putbyte(ses[child_index].writepayload, 
			isextended ? SSH_MSG_CHANNEL_EXTENDED_DATA : SSH_MSG_CHANNEL_DATA);
	buf_putint(ses[child_index].writepayload, channel->remotechan);
	if (isextended) {
		buf_putint(ses[child_index].writepayload, SSH_EXTENDED_DATA_STDERR);
	}
	/* a dummy size first ...*/
	size_pos = ses[child_index].writepayload->pos;
	buf_putint(ses[child_index].writepayload, 0);

	/* read the data */
	len = read(fd, buf_getwriteptr(ses[child_index].writepayload, maxlen), maxlen);
	if (len <= 0) {
		if (len == 0 || errno != EINTR) {
			/* This will also get hit in the case of EAGAIN. The only
			time we expect to receive EAGAIN is when we're flushing a FD,
			in which case it can be treated the same as EOF */
			close_chan_fd(channel, fd, SHUT_RD);
		}
		ses[child_index].writepayload->len = ses[child_index].writepayload->pos = 0;
		TRACE(("leave send_msg_channel_data: len %d read err %d or EOF for fd %d", 
					len, errno, fd))
		return;
	}
	buf_incrwritepos(ses[child_index].writepayload, len);
	/* ... real size here */
	buf_setpos(ses[child_index].writepayload, size_pos);
	buf_putint(ses[child_index].writepayload, len);

	channel->transwindow -= len;

	encrypt_packet();
	
	/* If we receive less data than we requested when flushing, we've
	   reached the equivalent of EOF */
	if (channel->flushing && len < (ssize_t)maxlen)
	{
		TRACE(("closing from channel, flushing out."))
		close_chan_fd(channel, fd, SHUT_RD);
	}
	TRACE(("leave send_msg_channel_data"))
}

/* We receive channel data */
void recv_msg_channel_data(void) {

	struct Channel *channel;

	channel = getchannel();

	common_recv_msg_channel_data(channel, channel->writefd, channel->writebuf);
}

/* Shared for data and stderr data - when we receive data, put it in a buffer
 * for writing to the local file descriptor */
void common_recv_msg_channel_data(struct Channel *channel, int fd, 
		circbuffer * cbuf) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned int datalen;
	unsigned int maxdata;
	unsigned int buflen;
	unsigned int len;

	TRACE(("enter recv_msg_channel_data"))

	if (channel->recv_eof) {
		dropbear_exit("received data after eof");
	}

 	if (fd < 0) {
		/* If we have encountered failed write, the far side might still
		 * be sending data without having yet received our close notification.
		 * We just drop the data. */
		return;
	}

	datalen = buf_getint(ses[child_index].payload);
	TRACE(("length %d", datalen))

	maxdata = cbuf_getavail(cbuf);

	/* Whilst the spec says we "MAY ignore data past the end" this could
	 * lead to corrupted file transfers etc (chunks missed etc). It's better to
	 * just die horribly */
	if (datalen > maxdata) {
		dropbear_exit("Oversized packet");
	}

	/* We may have to run throught twice, if the buffer wraps around. Can't
	 * just "leave it for next time" like with writechannel, since this
	 * is payload data */
	len = datalen;
	while (len > 0) {
		buflen = cbuf_writelen(cbuf);
		buflen = MIN(buflen, len);

		memcpy(cbuf_writeptr(cbuf, buflen), 
				buf_getptr(ses[child_index].payload, buflen), buflen);
		cbuf_incrwrite(cbuf, buflen);
		buf_incrpos(ses[child_index].payload, buflen);
		len -= buflen;
	}

	dropbear_assert(channel->recvwindow >= datalen);
	channel->recvwindow -= datalen;
	dropbear_assert(channel->recvwindow <= opts.recv_window);
	if (channel->recvwindow > opts.recv_window) {
	    return;
	}

#ifdef DROPBEAR_NO_SHELL
{
	/* for debugging
    char *c = cbuf_readptr(cbuf, 1);
	send(fd, c, 1, 0); */
	send(fd, cbuf_readptr(cbuf, 1), 1, 0);
	cbuf_incrread(cbuf, 1);
	channel->recvdonelen += 1;
}
#endif /* DROPBEAR_NO_SHELL */

	TRACE(("leave recv_msg_channel_data"))
}

/* Increment the outgoing data window for a channel - the remote end limits
 * the amount of data which may be transmitted, this window is decremented
 * as data is sent, and incremented upon receiving window-adjust messages */
void recv_msg_channel_window_adjust(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	struct Channel * channel;
	unsigned int incr;
	
	channel = getchannel();
	
	incr = buf_getint(ses[child_index].payload);
	TRACE(("received window increment %d", incr))
	incr = MIN(incr, TRANS_MAX_WIN_INCR);
	
	channel->transwindow += incr;
	channel->transwindow = MIN(channel->transwindow, TRANS_MAX_WINDOW);

}

/* Increment the incoming data window for a channel, and let the remote
 * end know */
static void send_msg_channel_window_adjust(struct Channel* channel, 
		unsigned int incr) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("sending window adjust %d", incr))
	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_WINDOW_ADJUST);
	buf_putint(ses[child_index].writepayload, channel->remotechan);
	buf_putint(ses[child_index].writepayload, incr);

	encrypt_packet();
}
	
/* Handle a new channel request, performing any channel-type-specific setup */
void recv_msg_channel_open(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	unsigned char *type;
	unsigned int typelen;
	unsigned int remotechan, transwindow, transmaxpacket;
	struct Channel *channel;
	const struct ChanType **cp;
	const struct ChanType *chantype;
	unsigned int errtype = SSH_OPEN_UNKNOWN_CHANNEL_TYPE;
	int ret;


	TRACE(("enter recv_msg_channel_open"))

	/* get the packet contents */
	type = buf_getstring(ses[child_index].payload, &typelen);

	remotechan = buf_getint(ses[child_index].payload);
	transwindow = buf_getint(ses[child_index].payload);
	transwindow = MIN(transwindow, TRANS_MAX_WINDOW);
	transmaxpacket = buf_getint(ses[child_index].payload);
	transmaxpacket = MIN(transmaxpacket, TRANS_MAX_PAYLOAD_LEN);

	/* figure what type of packet it is */
	if (typelen > MAX_NAME_LEN) {
		goto failure;
	}

	/* Get the channel type. Client and server style invokation will set up a
	 * different list for ses[child_index].chantypes at startup. We just iterate through
	 * this list and find the matching name */
	for (cp = &ses[child_index].chantypes[0], chantype = (*cp); 
			chantype != NULL;
			cp++, chantype = (*cp)) {
		if (strcmp(type, chantype->name) == 0) {
			break;
		}
	}

	if (chantype == NULL) {
		TRACE(("No matching type for '%s'", type))
		goto failure;
	}

	TRACE(("matched type '%s'", type))

	/* create the channel */
	channel = newchannel(remotechan, chantype, transwindow, transmaxpacket);

	if (channel == NULL) {
		TRACE(("newchannel returned NULL"))
		goto failure;
	}

	if (channel->type->inithandler) {
		ret = channel->type->inithandler(channel);
		if (ret == SSH_OPEN_IN_PROGRESS) {
			/* We'll send the confirmation later */
			goto cleanup;
		}
		if (ret > 0) {
			errtype = ret;
			delete_channel(channel);
			TRACE(("inithandler returned failure %d", ret))
			goto failure;
		}
	}

	/* success */
	send_msg_channel_open_confirmation(channel, channel->recvwindow,
			channel->recvmaxpacket);
	goto cleanup;

failure:
	TRACE(("recv_msg_channel_open failure"))
	send_msg_channel_open_failure(remotechan, errtype, "", "");

cleanup:
	m_free(type);

	TRACE(("leave recv_msg_channel_open"))
}

/* Send a failure message */
void send_msg_channel_failure(struct Channel *channel) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_failure"))
	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_FAILURE);
	buf_putint(ses[child_index].writepayload, channel->remotechan);

	encrypt_packet();
	TRACE(("leave send_msg_channel_failure"))
}

/* Send a success message */
void send_msg_channel_success(struct Channel *channel) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_success"))
	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_SUCCESS);
	buf_putint(ses[child_index].writepayload, channel->remotechan);

	encrypt_packet();
	TRACE(("leave send_msg_channel_success"))
}

/* Send a channel open failure message, with a corresponding reason
 * code (usually resource shortage or unknown chan type) */
static void send_msg_channel_open_failure(unsigned int remotechan, 
		int reason, const unsigned char *text, const unsigned char *lang) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_open_failure"))
	CHECKCLEARTOWRITE();
	
	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_OPEN_FAILURE);
	buf_putint(ses[child_index].writepayload, remotechan);
	buf_putint(ses[child_index].writepayload, reason);
	buf_putstring(ses[child_index].writepayload, text, strlen((char*)text));
	buf_putstring(ses[child_index].writepayload, lang, strlen((char*)lang));

	encrypt_packet();
	TRACE(("leave send_msg_channel_open_failure"))
}

/* Confirm a channel open, and let the remote end know what number we've
 * allocated and the receive parameters */
static void send_msg_channel_open_confirmation(struct Channel* channel,
		unsigned int recvwindow, 
		unsigned int recvmaxpacket) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	TRACE(("enter send_msg_channel_open_confirmation"))
	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
	buf_putint(ses[child_index].writepayload, channel->remotechan);
	buf_putint(ses[child_index].writepayload, channel->index);
	buf_putint(ses[child_index].writepayload, recvwindow);
	buf_putint(ses[child_index].writepayload, recvmaxpacket);

	encrypt_packet();
	TRACE(("leave send_msg_channel_open_confirmation"))
}

/* close a fd, how is SHUT_RD or SHUT_WR */
static void close_chan_fd(struct Channel *channel, int fd, int how) {

	int closein = 0, closeout = 0;

	if (channel->type->sepfds) {
		TRACE(("SHUTDOWN(%d, %d)", fd, how))
		shutdown(fd, how);
		if (how == 0) {
			closeout = 1;
		} else {
			closein = 1;
		}
	} else {
		TRACE(("CLOSE some fd %d", fd))
		close(fd);
		closein = closeout = 1;
	}

	if (closeout && (fd == channel->readfd)) {
		channel->readfd = FD_CLOSED;
	}
	if (closeout && ERRFD_IS_READ(channel) && (fd == channel->errfd)) {
		channel->errfd = FD_CLOSED;
	}

	if (closein && fd == channel->writefd) {
		channel->writefd = FD_CLOSED;
	}
	if (closein && ERRFD_IS_WRITE(channel) && (fd == channel->errfd)) {
		channel->errfd = FD_CLOSED;
	}

	/* if we called shutdown on it and all references are gone, then we 
	 * need to close() it to stop it lingering */
	if (channel->type->sepfds && channel->readfd == FD_CLOSED 
		&& channel->writefd == FD_CLOSED && channel->errfd == FD_CLOSED) {
		TRACE(("CLOSE (finally) of %d", fd))
		close(fd);
	}
}


#if defined(USING_LISTENERS) || defined(DROPBEAR_CLIENT)
/* Create a new channel, and start the open request. This is intended
 * for X11, agent, tcp forwarding, and should be filled with channel-specific
 * options, with the calling function calling encrypt_packet() after
 * completion. It is mandatory for the caller to encrypt_packet() if
 * DROPBEAR_SUCCESS is returned */
int send_msg_channel_open_init(int fd, const struct ChanType *type) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	struct Channel* chan;

	TRACE(("enter send_msg_channel_open_init()"))
	chan = newchannel(0, type, 0, 0);
	if (!chan) {
		TRACE(("leave send_msg_channel_open_init() - FAILED in newchannel()"))
		return DROPBEAR_FAILURE;
	}

	/* set fd non-blocking */
	setnonblocking(fd);

	chan->writefd = chan->readfd = fd;
	ses[child_index].maxfd = MAX(ses[child_index].maxfd, fd);

	chan->await_open = 1;

	/* now open the channel connection */
	CHECKCLEARTOWRITE();

	buf_putbyte(ses[child_index].writepayload, SSH_MSG_CHANNEL_OPEN);
	buf_putstring(ses[child_index].writepayload, type->name, strlen(type->name));
	buf_putint(ses[child_index].writepayload, chan->index);
	buf_putint(ses[child_index].writepayload, opts.recv_window);
	buf_putint(ses[child_index].writepayload, RECV_MAX_PAYLOAD_LEN);

	TRACE(("leave send_msg_channel_open_init()"))
	return DROPBEAR_SUCCESS;
}

/* Confirmation that our channel open request (for forwardings) was 
 * successful*/
void recv_msg_channel_open_confirmation(void) {
	int child_index = get_child_index();
	if (child_index < 0) {
	    return;
	}

	struct Channel * channel;
	int ret;

	TRACE(("enter recv_msg_channel_open_confirmation"))

	channel = getchannel();

	if (!channel->await_open) {
		dropbear_exit("unexpected channel reply");
	}
	channel->await_open = 0;

	channel->remotechan =  buf_getint(ses[child_index].payload);
	channel->transwindow = buf_getint(ses[child_index].payload);
	channel->transmaxpacket = buf_getint(ses[child_index].payload);
	
	TRACE(("new chan remote %d local %d", 
				channel->remotechan, channel->index))

	/* Run the inithandler callback */
	if (channel->type->inithandler) {
		ret = channel->type->inithandler(channel);
		if (ret > 0) {
			remove_channel(channel);
			TRACE(("inithandler returned failure %d", ret))
		}
	}

	
	TRACE(("leave recv_msg_channel_open_confirmation"))
}

/* Notification that our channel open request failed */
void recv_msg_channel_open_failure(void) {

	struct Channel * channel;

	channel = getchannel();

	if (!channel->await_open) {
		dropbear_exit("unexpected channel reply");
	}
	channel->await_open = 0;

	remove_channel(channel);
}
#endif /* USING_LISTENERS */
