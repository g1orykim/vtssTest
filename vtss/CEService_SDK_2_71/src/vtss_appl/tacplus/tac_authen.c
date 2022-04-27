/*
 *   AUTHENTICATION
 *
 */

#include "libtacplus.h"
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*
          types of authentication
TACACS_ENABLE_REQUEST  1    Enable Requests
TACACS_ASCII_LOGIN     2    Inbound ASCII Login
TACACS_PAP_LOGIN       3    Inbound PAP Login
TACACS_CHAP_LOGIN      4    Inbound CHAP login
TACACS_ARAP_LOGIN      5    Inbound ARAP login
TACACS_PAP_OUT         6    Outbound PAP request
TACACS_CHAP_OUT        7    Outbound CHAP request
TACACS_ASCII_ARAP_OUT  8    Outbound ASCII and ARAP request
TACACS_ASCII_CHPASS    9    ASCII change password request
TACACS_PPP_CHPASS      10   PPP change password request
TACACS_ARAP_CHPASS     11   ARAP change password request
TACACS_MSCHAP_LOGIN    12   MS-CHAP inbound login
TACACS_MSCHAP_OUT      13   MS-CHAP outbound login

    tac_authen_send_start - ending start authentication packet
        (we are as client initiate connection)
        port        tty10 or Async10
        username
        type
        data        external data to tacacs+ server
    return
        1       SUCCESS
        0       FAILURE
*/
#define TAC_AUTHEN_START_FIXED_FIELDS_SIZE 8
struct authen_start {
    u_char action;
    u_char priv_lvl;
    /*
    #define TAC_PLUS_PRIV_LVL_MIN 0x0
    #define TAC_PLUS_PRIV_LVL_MAX 0xf
    */
    u_char authen_type;

#define TAC_PLUS_AUTHEN_TYPE_ASCII  1
#define TAC_PLUS_AUTHEN_TYPE_PAP    2
#define TAC_PLUS_AUTHEN_TYPE_CHAP   3
#define TAC_PLUS_AUTHEN_TYPE_ARAP   4
#define TAC_PLUS_AUTHEN_TYPE_MSCHAP 5

    u_char service;

#define TAC_PLUS_AUTHEN_SVC_LOGIN  1
#define TAC_PLUS_AUTHEN_SVC_ENABLE 2
#define TAC_PLUS_AUTHEN_SVC_PPP    3
#define TAC_PLUS_AUTHEN_SVC_ARAP   4
#define TAC_PLUS_AUTHEN_SVC_PT     5
#define TAC_PLUS_AUTHEN_SVC_RCMD   6
#define TAC_PLUS_AUTHEN_SVC_X25    7
#define TAC_PLUS_AUTHEN_SVC_NASI   8

    u_char user_len;
    u_char port_len;
    u_char rem_addr_len;
    u_char data_len;
    /* <user_len bytes of char data> */
    /* <port_len bytes of char data> */
    /* <rem_addr_len bytes of u_char data> */
    /* <data_len bytes of u_char data> */
};
/***************************************************/
int tac_authen_send_start(struct session *session,
                          const char *port,
                          const char *username,
                          int type,
                          const char *data)
{
    char buf[512] = {0};
    char addr[1] = {0};
    HDR *hdr = (HDR *)buf;
    struct authen_start *ask = (struct authen_start *)(buf + TAC_PLUS_HDR_SIZE);
    /* username */
    char *u = (char *)(buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
    size_t ulen = strlen(username);
    /* port */
    char *p = (char *)(u + ulen);
    size_t plen = strlen(port);
    /* addr */
    char *a = (char *)(p + plen);
    size_t alen = strlen(addr);
    /* data */
    char *d = (char *)(a + alen);
    size_t dlen = strlen(data);

    if (session == NULL) {
        return 0;
    }

    if (ulen > 128) {
        (void)tac_error("Invalid username length.\n");
        return 0;
    }

    /*** header ***/
    /* version (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_0) */
    if (type == TACACS_ENABLE_REQUEST || type == TACACS_ASCII_LOGIN) {
        hdr->version = TAC_PLUS_VER_0;
    } else {
        hdr->version = TAC_PLUS_VER_1;
    }

    /* type of packet - TAC_PLUS_AUTHEN */
    hdr->type = TAC_PLUS_AUTHEN;
    /* set sequence, for first request it will be 1 */
    hdr->seq_no = ++(session->seq_no);
    /* encryption TAC_PLUS_ENCRYPTED || TAC_PLUS_CLEAR */
    hdr->encryption = TAC_PLUS_CLEAR;  /*TAC_PLUS_ENCRYPTED;*/
    /* session id */
    hdr->session_id = htonl(session->session_id);
    /* data length */
    if (type == TACACS_CHAP_LOGIN || type == TACACS_MSCHAP_LOGIN) {
        hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE + ulen + plen + alen + 1 + dlen);
    } else if (type == TACACS_PAP_LOGIN || type == TACACS_ARAP_LOGIN) {
        hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE + ulen + plen + alen + dlen);
    } else {
        hdr->datalength = htonl(TAC_AUTHEN_START_FIXED_FIELDS_SIZE + ulen + plen + alen);
    }

    /* privilege level */
    ask->priv_lvl = TAC_PLUS_PRIV_LVL_MIN;
    switch (type) {
    case TACACS_ENABLE_REQUEST:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->service = TAC_PLUS_AUTHEN_SVC_ENABLE;
        break;
    case TACACS_ASCII_LOGIN:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
        ask->service = TAC_PLUS_AUTHEN_SVC_LOGIN;
        break;
    case TACACS_PAP_LOGIN:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
        break;
    case TACACS_PAP_OUT:
        ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
        break;
    case TACACS_CHAP_LOGIN:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
        break;
    case TACACS_CHAP_OUT:
        ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
        break;
    case TACACS_MSCHAP_LOGIN:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
        break;
    case TACACS_MSCHAP_OUT:
        ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
        break;
    case TACACS_ARAP_LOGIN:
        ask->action = TAC_PLUS_AUTHEN_LOGIN;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ARAP;
        break;
    case TACACS_ASCII_CHPASS:
        ask->action = TAC_PLUS_AUTHEN_CHPASS;
        ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
        break;
    }
    /*
     * The length fields in start packet are single byte.
     * Network/host conversion not needed.
     */
    /* username */
    ask->user_len = ulen;
    if (ulen > 0) {
        strcpy(u, username);
    }

    /* port */
    ask->port_len = plen;
    if (plen > 0) {
        strcpy(p, port);
    }

    /* addr */
    ask->rem_addr_len = alen;
    if (alen > 0) {
        strcpy(a, addr);
    }

    /* data */
    ask->data_len = dlen;
    if (type == TACACS_CHAP_LOGIN) {
        *d++ = 1;
        strcpy(d, data);
    }
    if (type == TACACS_ARAP_LOGIN || type == TACACS_PAP_LOGIN) {
        strcpy(d, data);
    }

    /* write_packet encripting datas */
    if (tac_write_packet(session, (u_char *)buf)) {
        return 1;
    }
    return 0;
}

/**********************************************
 *  tac_authen_get_start  (server function)
 *  review start packet
 *  return 0 - error, type - success
 */
#ifdef TAC_SERVER_INCLUDE
int tac_authen_get_start_s(const char *pak,
                           struct session *session,
                           char *username,
                           char *port,
                           char *rem_addr,
                           char *data)
{
    int type = 0;
    HDR *hdr = (HDR *) pak;
    struct authen_start *start = (struct authen_start *)(pak + TAC_PLUS_HDR_SIZE);
    char *u = (char *)
              (pak + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
    char *p = (char *)(u + start->user_len);
    char *r = (char *)(p + start->port_len);
    char *d = (char *)(r + start->rem_addr_len);

    session->session_id = ntohl(hdr->session_id);

    bzero(username, sizeof(username));
    bzero(port, sizeof(port));
    bzero(rem_addr, sizeof(rem_addr));
    bzero(data, sizeof(data));

    if (hdr->seq_no != 1) {
        (void)tac_error("Invalid sequence");
        return 0;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->service == TAC_PLUS_AUTHEN_SVC_ENABLE) {
        type = TACACS_ENABLE_REQUEST;
    }
    if (start->action == TAC_PLUS_AUTHEN_CHPASS &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_ASCII) {
        type = TACACS_ASCII_CHPASS;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->service == TAC_PLUS_AUTHEN_SVC_LOGIN &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_ASCII) {
        type = TACACS_ASCII_LOGIN;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_PAP) {
        type = TACACS_PAP_LOGIN;
    }
    if (start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_PAP) {
        type = TACACS_PAP_OUT;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_CHAP) {
        type = TACACS_CHAP_LOGIN;
    }
    if (start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_CHAP) {
        type = TACACS_CHAP_OUT;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_MSCHAP) {
        type = TACACS_MSCHAP_LOGIN;
    }
    if (start->action == TAC_PLUS_AUTHEN_SENDAUTH &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_MSCHAP) {
        type = TACACS_MSCHAP_OUT;
    }
    if (start->action == TAC_PLUS_AUTHEN_LOGIN &&
        start->authen_type == TAC_PLUS_AUTHEN_TYPE_ARAP) {
        type = TACACS_ARAP_LOGIN;
    }

    /* user */
    strncpy(username, u, start->user_len);
    /* port */
    strncpy(port, p, start->port_len);
    /* addr */
    strncpy(rem_addr, r, start->rem_addr_len);
    /* data */
    strncpy(data, d, start->data_len);

    return (type);
}

int tac_authen_get_start(struct session *session,
                         char *username,
                         char *port,
                         char *rem_addr,
                         char *data)
{
    char *pak = (char *)tac_read_packet(session);
    if (pak == NULL) {
        return 0;
    }
    return (tac_authen_get_start_s(pak, session, username,
                                   port, rem_addr, data));
}
#endif /* TAC_SERVER_INCLUDE */


/*********************************************
   send REPLY packet (server function)
   return status packet
   and set variables
    return
        0   SUCCESS
        -1  FAILURE
Status:
TAC_PLUS_AUTHEN_STATUS_PASS     1
TAC_PLUS_AUTHEN_STATUS_FAIL     2
TAC_PLUS_AUTHEN_STATUS_GETDATA  3
TAC_PLUS_AUTHEN_STATUS_GETUSER  4
TAC_PLUS_AUTHEN_STATUS_GETPASS  5
TAC_PLUS_AUTHEN_STATUS_RESTART  6
TAC_PLUS_AUTHEN_STATUS_ERROR    7
TAC_PLUS_AUTHEN_STATUS_FOLLOW   0x21
***********************************************/
#define TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE 6

struct authen_reply {
    u_char status;
    u_char flags;
#define TAC_PLUS_AUTHEN_FLAG_NOECHO     0x1
    u_short msg_len;
    u_short data_len;
    /* <msg_len bytes of char data> */
    /* <data_len bytes of u_char data> */
};
/*************************************/
#ifdef TAC_SERVER_INCLUDE
int tac_authen_send_reply(struct session *session,
                          const int status,
                          const char *server_msg,
                          const char *data)
{
    char buf[256];
    /* header */
    HDR *hdr = (HDR *)buf;
    /* data */
    struct authen_reply *dat = (struct authen_reply *)(buf + TAC_PLUS_HDR_SIZE);
    char *s = (char *)(buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE);
    char *d = (char *)(s + strlen(server_msg));

    if (session == NULL) {
        return -1;
    }

    /* clean */
    bzero(buf, sizeof(buf));
    /* version */
    hdr->version = TAC_PLUS_VER_0;
    /* packet type */
    hdr->type = TAC_PLUS_AUTHEN;
    /* sequence number */
    hdr->seq_no = ++session->seq_no;
    /* set encryption */
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
    /* session id */
    hdr->session_id = htonl(session->session_id);
    /* packet length */
    hdr->datalength = htonl(TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE
                            + strlen(server_msg) + strlen(data));

    /* compose packet */
    dat->status = status;
    dat->flags = 1;
    dat->msg_len = htons(strlen(server_msg));
    dat->data_len = htons(strlen(data));

    if (strlen(server_msg) > 0) {
        strcpy(s, server_msg);
    }
    if (strlen(data) > 0) {
        strcpy(d, data);
    }

    if (tac_write_packet(session, (u_char *)buf)) {
        return 1;
    }
    return 0;
}
#endif /* TAC_SERVER_INCLUDE */

/* get REPLY reply (client function) */
/* return status packet and set variables
    return
        -1  FAILURE
Status:

   TAC_PLUS_AUTHEN_STATUS_PASS     := 0x01
   TAC_PLUS_AUTHEN_STATUS_FAIL     := 0x02
   TAC_PLUS_AUTHEN_STATUS_GETDATA  := 0x03
   TAC_PLUS_AUTHEN_STATUS_GETUSER  := 0x04
   TAC_PLUS_AUTHEN_STATUS_GETPASS  := 0x05
   TAC_PLUS_AUTHEN_STATUS_RESTART  := 0x06
   TAC_PLUS_AUTHEN_STATUS_ERROR    := 0x07
   TAC_PLUS_AUTHEN_STATUS_FOLLOW   := 0x21

*/
int tac_authen_get_reply(struct session *session,
                         char *server_msg,
                         size_t server_msg_len,
                         char *data,
                         size_t data_len)
{
    char *buf;
    /* header */
    HDR *hdr;
    /* static datas */
    struct authen_reply *rep;
    /* server message */
    char *serv_msg;
    /* server datas */
    char *dat_pak;
    size_t mlen;
    size_t dlen;

    int rc;

    if (session == NULL) {
        return -1;
    }

    buf = (char *)tac_read_packet(session);
    if (buf == NULL) {
        return -1;
    }

    hdr = (HDR *)buf;
    rep = (struct authen_reply *)(buf + TAC_PLUS_HDR_SIZE);
    serv_msg = (char *)(buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE);
    dat_pak = (char *)(serv_msg + ntohs(rep->msg_len));

    /* fields length */
    mlen = ntohs(rep->msg_len);
    dlen = ntohs(rep->data_len);

    if (hdr->datalength != htonl(TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE + mlen + dlen)) {
        (void)tac_error("Invalid AUTHEN/REPLY packet, check keys.\n");
        tac_callout_free(buf);
        return -1;
    }
    session->session_id = ntohl(hdr->session_id);

    bzero(server_msg, server_msg_len);
    if (mlen > 0) {
        strncpy(server_msg, serv_msg, MIN(mlen, server_msg_len)); /* Don't copy outside server_msg */
        server_msg[server_msg_len - 1] = '\0';
    }

    bzero(data, data_len);
    if (dlen > 0) {
        strncpy(data, dat_pak, MIN(dlen, data_len)); /* Don't copy outside data */
        data[data_len - 1] = '\0';
    }

    rc = rep->status;
    tac_callout_free(buf);
    return (rc);
}

/************************************
   Send CONTINUE packet
      (client function)

   tac_authen_send_cont

    return
        1       SUCCESS
        0       FAILURE
*************************************/
#define TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE 5
struct authen_cont {
    u_short user_msg_len;
    u_short user_data_len;
    u_char flags;

#define TAC_PLUS_CONTINUE_FLAG_ABORT 0x1

    /* <user_msg_len bytes of u_char data> */
    /* <user_data_len bytes of u_char data> */
};
/* --------------------------------------------------- */
int tac_authen_send_cont(struct session *session,
                         const char *user_msg,
                         const char *data)
{
    char buf[512] = {0};
    /* header */
    HDR *hdr = (HDR *)buf;
    /* datas */
    struct authen_cont *ask = (struct authen_cont *)(buf + TAC_PLUS_HDR_SIZE);
    /* user_msg */
    char *u = (char *)(buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);
    size_t ulen = strlen(user_msg);
    /* data */
    char *d = (char *)(u + ulen);
    size_t dlen = strlen(data);

    /* version */
    hdr->version = TAC_PLUS_VER_0;
    /* packet type */
    hdr->type = TAC_PLUS_AUTHEN;
    /* sequence number */
    hdr->seq_no = ++session->seq_no;
    /* set encryption */
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
    /* session id */
    hdr->session_id = htonl(session->session_id);
    /* data length */
    hdr->datalength = htonl(TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE + ulen + dlen);

    /* user_msg */
    ask->user_msg_len = htons(ulen);
    if (ulen > 0) {
        strcpy(u, user_msg);
    }

    /* data  */
    ask->user_data_len = htons(dlen);
    if (dlen > 0) {
        strcpy(d, data);
    }

    /* send packet */
    if (tac_write_packet(session, (u_char *)buf)) {
        return 1;
    }
    return 0;
}

/**************************************************
  tac_authen_get_cont (server function)
  get CONTINUE packet

    return
        1       SUCCESS
        0       FAILURE
*/
#ifdef TAC_SERVER_INCLUDE
int tac_authen_get_cont(struct session *session,
                        char *user_msg,
                        char *data)
{
    int len;
    char *buf = (char *)tac_read_packet(session);
    HDR *hdr = (HDR *)buf;
    /* data */
    struct authen_cont *ask = (struct authen_cont *)(buf + TAC_PLUS_HDR_SIZE);
    /* packet */
    char *p = (char *)
              (buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);
    char *d = (char *)(p + ntohs(ask->user_msg_len));

    if (buf == NULL) {
        return 0;
    }
    if (hdr->datalength != htonl(TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE +
                                 ntohs(ask->user_msg_len) + ntohs(ask->user_data_len))) {
        (void)tac_error("Invalid AUTHEN/CONT packet, check keys.\n");
        return 0;
    }
    session->session_id = ntohl(hdr->session_id);

    bzero(user_msg, sizeof(user_msg));
    bzero(data, sizeof(data));

    len = ntohs(ask->user_msg_len);
    strncpy(user_msg, p, len);
    len = ntohs(ask->user_data_len);
    strncpy(data, d, len);

    return 1;
}
#endif /* TAC_SERVER_INCLUDE */
