#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libtacplus.h"
#include "vtss_md5_api.h"
typedef struct vtss_MD5Context MD5_CTX;

/*
    tac_connect - Connect to TACACS+ server.
        peer    server name (or IP adress)
        timeout waiting for connection to establish
        key a kind of encryption key
        port    TACACS+ server port
    return
        NULL    FAILURE
        session SUCCESS

    ADL 2007-07-27: Don't set session->aborted if tac_close() was already called.

*/

#ifdef VTSS_TACACS
/*lint --e{506} Suppress Lint Warning 506: Constant value Boolean */
/*lint --e{573} Suppress Lint Warning 573: Signed-unsigned mix with divide */

/* tac_connect modified for EStaX to use non-blocking socket instead of signal/alarm */
#define tac_abort(e, str)                                               \
  do {                                                                  \
      if (session) {                                                    \
          session->aborted = 1;                                         \
          tac_close(session);                                           \
      }                                                                 \
      if (res) {                                                        \
          freeaddrinfo(res);                                            \
      }                                                                 \
      (void)tac_error("aborted tac_connect on %s operation: %s\n",      \
                      (str), strerror(e));                              \
      return NULL;                                                      \
  } while (0)

struct session *
tac_connect(const char *peer, int timeout, const char *key, int port)
{
    struct addrinfo  hints;
    struct addrinfo *res = NULL;
    struct session  *session;
    int              error;
    int              flag;
    char             port_string[16];

    if (port == 0) {
        port = 49;
    }

    session = (struct session *)tac_callout_malloc(sizeof(struct session));
    if (session == NULL) {
        tac_abort(ENOMEM, "malloc");
    }
    memset(session, 0, sizeof(struct session));
    session->aborted = 0;
    session->sock = -1;

    session->peer = tac_callout_strdup(peer);
    if (session->peer == NULL) {
        tac_abort(ENOMEM, "tac_callout_strdup(peer)");
    }
    if (key) {
        session->key = tac_callout_strdup(key);
        if (session->key == NULL) {
            tac_abort(ENOMEM, "tac_callout_strdup(key)");
        }
    }

    /* setup hints structure */
    memset(&hints, 0, sizeof(hints));
#ifdef VTSS_SW_OPTION_IPV6
    hints.ai_family = AF_UNSPEC; /* Accept both IPv4 and IPv6 */
#else
    hints.ai_family = AF_INET; /* Accept IPv4 only */
#endif
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    sprintf(port_string, "%d", port);
    if ((error = getaddrinfo(peer, port_string, &hints, &res))) {
        tac_abort(EINVAL, gai_strerror(error));
    }
    // We will only try the first addrinfo in res and NOT traverse the whole list
    if ((session->sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
        tac_abort(errno, "socket");
    }

    /* Set socket to non-blocking mode */
    flag = 1;
    if (ioctl(session->sock, FIONBIO, &flag) < 0) {
        tac_abort(errno, "iocnl(...,FIONBIO) 1");
    }

    error = connect(session->sock, res->ai_addr, res->ai_addrlen);
    if (error < 0) {
        if (errno == EINPROGRESS) {
            fd_set         fdset;
            struct timeval tv;
            do {
                tv.tv_sec = timeout;
                tv.tv_usec = 0;
                FD_ZERO(&fdset);
                FD_SET(session->sock, &fdset);
                error = select(session->sock + 1, NULL, &fdset, NULL, &tv);
                if (error < 0 && errno != EINTR) {
                    tac_abort(errno, "select");
                } else if (error > 0) {
                    // select has now told us that the socket is writable, but we will need to check for errors
                    int       option_value;
                    socklen_t option_length;
                    option_length = sizeof(int);
                    if (getsockopt(session->sock, SOL_SOCKET, SO_ERROR, (void *)(&option_value), &option_length) < 0) {
                        tac_abort(errno, "getsockopt");
                    }
                    if (option_value) {
                        tac_abort(option_value, "delayed connect");
                    }
                    break; /* SUCCESS - we are now connected */
                } else {
                    tac_abort(ETIMEDOUT, "timeout");
                }
            } while (1);
        } else {
            tac_abort(errno, "connect");
        }
    }

    /* Set socket back to blocking mode again */
    flag = 0;
    if (ioctl(session->sock, FIONBIO, &flag) < 0) {
        tac_abort(errno, "iocnl(...,FIONBIO) 2");
    }

    freeaddrinfo(res);
    session->session_id = htonl(getpid());  /* this is for random unique number*/
    session->seq_no = 0;
    session->last_exch = time(NULL);

    return session;
}

#else /* VTSS_TACACS */

#define tac_abort(e, str) { int _e; _e = e;\
       _e = errno; \
       if(session) { \
           session->aborted = 1; \
           tac_close(session); \
       } \
       if((str)) { \
               tac_error("aborted tac_connect on %s operation: %s", \
               (str), strerror(_e)); \
       } \
       errno = _e; \
       return NULL; }

/* if timeout */
static struct session **catch_sess = NULL;  /* ADL 2007-07-27: Use session ** so catchup() can set to NULL. */
static void catchup(int s)
{
    tac_error("*** TACACS+ Server Not Responding!\r\n");
    (*catch_sess)->aborted = 1;             /* ADL 2007-07-27: Dereference new double-pointer. */
    tac_close(*catch_sess);                 /* ADL 2007-07-27: Dereference new double-pointer. */
    *catch_sess = NULL;                     /* ADL 2007-07-27: Prevent tac_close() from freeing session more than once. */
}


struct session *
tac_connect(const char *peer, int timeout, const char *key, int port)
{
    struct sockaddr_in s;
    void (*oldalrm)();
    static struct session *session;

    if (port == 0) {
        port = 49;
    }
    session = (struct session *)tac_callout_malloc(sizeof(struct session));
    if (session == NULL) {
        printf(" tac_connect: Can't allocate memory\n");
        return NULL;
    }
    memset(session, 0, sizeof(struct session));
    /* store */
    session->peer = tac_callout_strdup(peer);
    if (key) {
        session->key = tac_callout_strdup(key);
    }
    session->aborted = 0;

    /* connection */
    if ((session->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        tac_abort(errno, "socket");
    }
    s.sin_addr.s_addr = htonl(INADDR_ANY);
#ifndef __SVR4
#ifndef __linux__
    s.sin_len = sizeof(struct sockaddr_in);
#endif
#endif
    s.sin_family = AF_INET;
    s.sin_port = 0;
    if (bind(session->sock, (struct sockaddr *)&s, sizeof(s)) < 0) {
        tac_abort(errno, "bind");
    }
    /* If the following line does not compile, use the commented one instead,
        and leave me a mail drop with system (OS) name/info so I can take this
        into account. mailto:lorin@adesium-services.fr  */
    /*  if (!inet_aton(session->peer, &s.sin_addr.s_addr)) tac_abort;   */
    /*  if (!inet_aton(session->peer, &s.sin_addr)) tac_abort; */
    if ((s.sin_addr.s_addr = inet_addr(session->peer)) == 0xffffff) {
        tac_abort(errno, NULL);
    }
#ifndef __SVR4
#ifndef __linux__
    s.sin_len = sizeof(struct sockaddr_in);
#endif
#endif
    s.sin_family = AF_INET;
    s.sin_port = htons(port);
    oldalrm = signal(SIGALRM, catchup);
    catch_sess = &session;            /* ADL 2007-07-27: Use &session so catchup() can set session to NULL. */
    alarm(timeout);
    if (connect(session->sock, (struct sockaddr *)&s, sizeof(s)) < 0) {
        int e;
        e = errno;
        alarm(0);
        signal(SIGALRM, oldalrm);
        if (e == EINTR) {
            e = ETIMEDOUT;
        }   /* this is kinda awful, but an easy way
    to get a good message */
        tac_abort(e, "connect");
    }
    alarm(0);
    /* for session_id set process pid */
    session->session_id = htonl(getpid());    /* this is for random unique number*/
    /* sequence to zero */
    session->seq_no = 0;
    /* and dont see using this */
    session->last_exch = time(NULL);
    signal(SIGALRM, oldalrm);

    return session;
}
#endif /* VTSS_TACACS */


/*
    tac_close - Close connection with TACACS+ server
*/
void tac_close(struct session *session)
{
    if (session) {
        if (session->sock >= 0 ) {
            (void)shutdown(session->sock, SHUT_RDWR);
            close(session->sock);
        }
        if (session->peer) {
            tac_callout_free(session->peer);
        }
        if (session->key) {
            tac_callout_free(session->key);
        }
        tac_callout_free(session);
    }
}


/*
 * create_md5_hash(): create an md5 hash of the "session_id", "the user's
 * key", "the version number", the "sequence number", and an optional
 * 16 bytes of data (a previously calculated hash). If not present, this
 * should be NULL pointer.
 *
 * Write resulting hash into the array pointed to by "hash".
 *
 * The caller must allocate sufficient space for the resulting hash
 * (which is 16 bytes long). The resulting hash can safely be used as
 * input to another call to create_md5_hash, as its contents are copied
 * before the new hash is generated.
 */
static void create_md5_hash(int session_id, char *key, u_char version, u_char seq_no, u_char *prev_hash, u_char *hash)
{
    u_char *md_stream, *mdp;
    int md_len;
    MD5_CTX mdcontext;

    md_len = sizeof(session_id) + strlen(key) + sizeof(version) + sizeof(seq_no);

    if (prev_hash) {
        md_len += MD5_LEN;
    }
    mdp = md_stream = (u_char *)tac_callout_malloc(md_len);
    if (md_stream == NULL) {
        return;
    }
    bcopy(&session_id, mdp, sizeof(session_id));
    mdp += sizeof(session_id);

    bcopy(key, mdp, strlen(key));
    mdp += strlen(key);

    bcopy(&version, mdp, sizeof(version));
    mdp += sizeof(version);

    bcopy(&seq_no, mdp, sizeof(seq_no));
    mdp += sizeof(seq_no);

    if (prev_hash) {
        bcopy(prev_hash, mdp, MD5_LEN);
    }
    vtss_MD5Init(&mdcontext);
    vtss_MD5Update(&mdcontext, md_stream, md_len);
    vtss_MD5Final(hash, &mdcontext);
    tac_callout_free(md_stream);
    return;
}


/*
 * Overwrite input data with en/decrypted version by generating an MD5 hash and
 * xor'ing data with it.
 *
 * When more than 16 bytes of hash is needed, the MD5 hash is performed
 * again with the same values as before, but with the previous hash value
 * appended to the MD5 input stream.
 *
 * Return 0 on success, -1 on failure.
 */
static int md5_xor(HDR *hdr, u_char *data, char *key)
{
    int i, j;
    u_char hash[MD5_LEN];       /* the md5 hash */
    u_char last_hash[MD5_LEN];  /* the last hash we generated */
    u_char *prev_hashp = (u_char *) NULL;       /* pointer to last created
                         * hash */
    int data_len;
    int session_id;
    u_char version;
    u_char seq_no;

    data_len = ntohl(hdr->datalength);
    session_id = hdr->session_id; /* always in network order for hashing */
    version = hdr->version;
    seq_no = hdr->seq_no;

    if (!key) {
        return 0;
    }

    for (i = 0; i < data_len; i += 16) {

        create_md5_hash(session_id, key, version, seq_no, prev_hashp, hash);

#ifdef DEBUG_MD5
        tac_error(
            "hash: session_id=%u, key=%s, version=%d, seq_no=%d\n",
            session_id, key, version, seq_no);
        /* debug */
#endif
        bcopy(hash, last_hash, MD5_LEN);
        prev_hashp = last_hash;

        for (j = 0; j < 16; j++) {
            if ((i + j) >= data_len) {
                hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR) ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
                return 0;
            }
            data[i + j] ^= hash[j];
        }
    }
    hdr->encryption = (hdr->encryption == TAC_PLUS_CLEAR) ? TAC_PLUS_ENCRYPTED : TAC_PLUS_CLEAR;
    return 0;
}


/* Reading n bytes from descriptor fd to array ptr with timeout t sec
 * Timeout set for each read
 *
 * Return -1 if error, eof or timeout. Else returns
 * number reads bytes. */
static int sockread(struct session *session, int fd, u_char *ptr, int nbytes, int timeout)
{
    int nleft, nread;
    fd_set readfds, exceptfds;
    struct timeval tout;

    if (fd == -1) {
        return -1;
    }

    tout.tv_sec = timeout;
    tout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    FD_ZERO(&exceptfds);
    FD_SET(fd, &exceptfds);

    nleft = nbytes;

    while (nleft > 0) {
        int status = select(fd + 1, &readfds, (fd_set *) NULL,
                            &exceptfds, &tout);

        if (status == 0) {
            (void)tac_error("%s: timeout reading fd %d", session->peer, fd);
            return (-1);
        }
        if (status < 0) {
            if (errno == EINTR) {
                continue;
            }
            (void)tac_error("%s: error in select %s fd %d", session->peer, strerror(errno), fd);
            return (-1);
        }
        if (FD_ISSET(fd, &exceptfds)) {
            (void)tac_error("%s: exception on fd %d", session->peer, fd);
            return (-1);
        }
        if (!FD_ISSET(fd, &readfds)) {
            (void)tac_error("%s: spurious return from select", session->peer);
            continue;
        }
again:
        nread = read(fd, ptr, nleft);

        if (nread < 0) {
            if (errno == EINTR) {
                goto again;
            }
            (void)tac_error("%s %s: error reading fd %d nread=%d %s", session->peer, session->port, fd, nread, strerror(errno));
            return (-1);        /* error */

        } else if (nread == 0) {
            (void)tac_error("%s %s: fd %d eof (connection closed)", session->peer, session->port, fd);
            return (-1);        /* eof */
        }
        nleft -= nread;
        if (nleft) {
            ptr += nread;
        }
    }
    return (nbytes - nleft);
}


/* Write n bytes to descriptor fd from array ptr with timeout t
 * seconds. Note the timeout is applied to each write, not for the
 * overall operation.
 *
 * Return -1 on error, eof or timeout. Otherwise return number of
 * bytes written. */
static int sockwrite(struct session *session, int fd, const u_char *ptr, int bytes, int timeout)
{
    int remaining, sent;
    fd_set writefds, exceptfds;
    struct timeval tout;

    if (fd == -1) {
        return -1;
    }

    sent = 0;

    tout.tv_sec = timeout;
    tout.tv_usec = 0;


    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    FD_ZERO(&exceptfds);
    FD_SET(fd, &exceptfds);

    remaining = bytes;

    while (remaining > 0) {
        int status = select(fd + 1, (fd_set *) NULL,
                            &writefds, &exceptfds, &tout);

        if (status == 0) {
            (void)tac_error("%s: timeout writing to fd %d", session->peer, fd);
            return (-1);
        }
        if (status < 0) {
            (void)tac_error("%s: error in select fd %d", session->peer, fd);
            return (-1);
        }
        if (FD_ISSET(fd, &exceptfds)) {
            (void)tac_error("%s: exception on fd %d", session->peer, fd);
            return (sent);      /* error */
        }

        if (!FD_ISSET(fd, &writefds)) {
            (void)tac_error("%s: spurious return from select", session->peer);
            continue;
        }
        sent = write(fd, ptr, remaining);

        if (sent <= 0) {
            (void)tac_error("%s: error writing fd %d sent=%d", session->peer, fd, sent);
            return (sent);      /* error */
        }
        remaining -= sent;
        ptr += sent;
    }
    return (bytes - remaining);
}


/*
    read_packet - Read a packet and decrypt it from TACACS+ server
    return
        pointer to a newly allocated memory buffer containing packet data
        NULL    FAILURE
*/
u_char *tac_read_packet(struct session *session)
{
    HDR hdr;
    u_char *pkt, *data;
    int len;

    if (session == NULL) {
        return NULL;
    }

    /* read a packet header */
    len = sockread(session, session->sock, (u_char *) & hdr, TAC_PLUS_HDR_SIZE, TAC_PLUS_READ_TIMEOUT);
    if (len != TAC_PLUS_HDR_SIZE) {
        (void)tac_error("Read %d bytes from %s %s, expecting %d", len, session->peer, session->port, TAC_PLUS_HDR_SIZE);
        return (NULL);
    }

    if ((hdr.version & TAC_PLUS_MAJOR_VER_MASK) != TAC_PLUS_MAJOR_VER) {
        (void)tac_error("%s: Illegal major version specified: found %d wanted %d\n", session->peer, hdr.version, TAC_PLUS_MAJOR_VER);
        return (NULL);
    }

    /* get memory for the packet */
    len = TAC_PLUS_HDR_SIZE + ntohl(hdr.datalength);
    pkt = (u_char *)tac_callout_malloc(len);
    if (pkt == NULL) {
        return (NULL);
    }
    /* initialise the packet */
    bcopy(&hdr, pkt, TAC_PLUS_HDR_SIZE);

    /* the data start here */
    data = pkt + TAC_PLUS_HDR_SIZE;

    /* read the rest of the packet data */
    if (sockread(session, session->sock, data, ntohl(hdr.datalength),
                 TAC_PLUS_READ_TIMEOUT) !=
        ntohl(hdr.datalength)) {
        (void)tac_error("%s: start_session: bad socket read", session->peer);
        tac_callout_free(pkt);
        return (NULL);
    }
    session->seq_no++;           /* should now equal that of incoming packet */
    session->last_exch = time(NULL);

    if (session->seq_no != hdr.seq_no) {
        (void)tac_error("%s: Illegal session seq # %d != packet seq # %d", session->peer, session->seq_no, hdr.seq_no);
        tac_callout_free(pkt);
        return (NULL);
    }

    /* decrypt the data portion */
    if (session->key && md5_xor((HDR *)pkt, data, session->key)) {
        (void)tac_error("%s: start_session error decrypting data", session->peer);
        tac_callout_free(pkt);
        return (NULL);
    }

    session->version = hdr.version;

    return (pkt);
}


/*
    write_packet - Send a data packet to TACACS+ server
        pak pointer to packet data to send
    return
        1       SUCCESS
        0       FAILURE
*/
int tac_write_packet(struct session *session, u_char *pak)
{
    HDR *hdr = (HDR *) pak;
    u_char *data;
    int len;

    if (session == NULL) {
        printf("session = NULL\n");
        return 0;
    }

    len = TAC_PLUS_HDR_SIZE + ntohl(hdr->datalength);


    /* the data start here */
    data = pak + TAC_PLUS_HDR_SIZE;


    /* encrypt the data portion */
    if (session->key && md5_xor((HDR *)pak, data, session->key)) {
        printf("%s: write_packet: error encrypting data", session->peer);
        (void)tac_error("%s: write_packet: error encrypting data", session->peer);
        return (0);
    }

    if (sockwrite(session, session->sock, pak, len, TAC_PLUS_WRITE_TIMEOUT) != len) {
        return (0);
    }
    session->last_exch = time(NULL);
    return (1);
}

