/*
 * authorization
 */

#include "libtacplus.h"
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/*************************************************
       The AV-pairs list depends from Cisco IOS version
char *avpair[]=
{
   "service=(*)slip|ppp|arap|shell|tty-daemon|
          connection|system|firewall|multilink|...",
     this attribute MUST always be included !

   "protocol=(*)lcp|ip|ipx|atalk|vines|lat|xremote|
           tn3270|telnet|rlogin|pad|vpdn|ftp|
           http|deccp|osicp|unknown|multilink",
   "cmd=(*)command, if service=shell",
This attribute MUST be specified if service equals "shell".
A NULL value (cmd=NULL) indicates that the shell itself is being referred to.

   "cmd-arg=(*)argument to command",
Multiple cmd-arg attributes may be specified

   "acl=(*)access list, if service=shell É cmd=NULL",
Used only when service=shell and cmd=NULL

   "inacl=(*)input access list",
   "outacl=(*)output access list",
   "zonelist=(*)numeric zonelist value to AppleTalk only",
   "addr=(*)network address",
   "addr-pool=(*)address pool",
   "routing=(*)true|false, routing propagated",
   "route=(*)<dst_address> <mask> [<routing_addr>]",
MUST be of the form "<dst_address> <mask> [<routing_addr>]"

   "timeout=(*)timer for the connection (in minutes)",
zero - no timeout

   "idletime=(*)idle-timeout (in minutes)",
   "autocmd=(*)auto-command, service=shell and cmd=NULL",
   "noescape=(*)true|false, deny using symbol escape",
service=shell and cmd=NULL

   "nohangup=(*)true|false, Do no disconnect after autocmd",
service=shell and cmd=NULL

   "priv_lvl=(*)privilege level",
   "remote_user=(*)remote userid, for AUTHEN_METH_RCMD",
   "remote_host=(*)remote host, for AUTHEN_METH_RCMD",
   "callback-dialstring=(*)NULL, or a dialstring",
   "callback-line=(*)line number to use for a callback",
   "callback-rotary=(*)rotary number to use for a callback",
   "nocallback-verify=(*)not require authen after callback"

     ...

   This list can increase for new versions of Cisco IOS

   NULL - end of array

   = - mandatory argument
   * - optional argument

   maximum length of 1 AV-pair is 255 chars
};
*/

/*
 methods:
TAC_PLUS_AUTHEN_METH_NOT_SET    := 0x00
TAC_PLUS_AUTHEN_METH_NONE       := 0x01
TAC_PLUS_AUTHEN_METH_KRB5       := 0x02
TAC_PLUS_AUTHEN_METH_LINE       := 0x03
TAC_PLUS_AUTHEN_METH_ENABLE     := 0x04
TAC_PLUS_AUTHEN_METH_LOCAL      := 0x05
TAC_PLUS_AUTHEN_METH_TACACSPLUS := 0x06     * use this *
TAC_PLUS_AUTHEN_METH_GUEST      := 0x08
TAC_PLUS_AUTHEN_METH_RADIUS     := 0x10
TAC_PLUS_AUTHEN_METH_KRB4       := 0x11
TAC_PLUS_AUTHEN_METH_RCMD       := 0x20

priv_lvl:
TAC_PLUS_PRIV_LVL_MAX   := 0x0f               ?
TAC_PLUS_PRIV_LVL_ROOT  := 0x0f               ?
TAC_PLUS_PRIV_LVL_USER  := 0x01               ?
TAC_PLUS_PRIV_LVL_MIN   := 0x00               ?

authen_type:
TAC_PLUS_AUTHEN_TYPE_ASCII      := 0x01       ascii
TAC_PLUS_AUTHEN_TYPE_PAP        := 0x02       pap
TAC_PLUS_AUTHEN_TYPE_CHAP       := 0x03       chap
TAC_PLUS_AUTHEN_TYPE_ARAP       := 0x04       arap
TAC_PLUS_AUTHEN_TYPE_MSCHAP     := 0x05       mschap

authen_service:
TAC_PLUS_AUTHEN_SVC_NONE        := 0x00
TAC_PLUS_AUTHEN_SVC_LOGIN       := 0x01
TAC_PLUS_AUTHEN_SVC_ENABLE      := 0x02
TAC_PLUS_AUTHEN_SVC_PPP         := 0x03
TAC_PLUS_AUTHEN_SVC_ARAP        := 0x04
TAC_PLUS_AUTHEN_SVC_PT          := 0x05
TAC_PLUS_AUTHEN_SVC_RCMD        := 0x06
TAC_PLUS_AUTHEN_SVC_X25         := 0x07
TAC_PLUS_AUTHEN_SVC_NASI        := 0x08
TAC_PLUS_AUTHEN_SVC_FWPROXY     := 0x09
*/
#define TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE 8
/* An authorization request packet */
struct author {
    u_char authen_method;
    u_char priv_lvl;
    u_char authen_type;
    u_char service;

    u_char user_len;
    u_char port_len;
    u_char rem_addr_len;
    u_char arg_cnt;             /* the number of args */
    /* <arg_cnt u_chars containing the lengths of args 1 to arg n> */
    /* <user_len bytes of char data> */
    /* <port_len bytes of char data> */
    /* <rem_addr_len bytes of u_char data> */
    /* <char data for each arg> */
};

/****************************************
    send request (client function)
****************************************/
int tac_author_send_request(struct session *session,
                            const int method,
                            const int priv_lvl,
                            const int authen_type,
                            const int authen_service,
                            const char *user,
                            const char *port,
                            char **avpair)
{
    char buf[512] = {0};
    char rem_addr[1] = {0};
    HDR *hdr = (HDR *)buf;
    struct author *auth = (struct author *)(buf + TAC_PLUS_HDR_SIZE);
    char *lens = (char *)(buf + TAC_PLUS_HDR_SIZE + TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE);
    int arglens = 0;
    int i;
    size_t ulen = strlen(user);
    size_t plen = strlen(port);
    size_t alen = strlen(rem_addr);

    hdr->version = TAC_PLUS_VER_0;
    hdr->type = TAC_PLUS_AUTHOR;      /* set packet type to authorization */
    hdr->seq_no = ++session->seq_no;
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
    hdr->session_id = htonl(session->session_id);

    /* count length */
    for (i = 0; avpair[i] != NULL ; i++) {
        if (strlen(avpair[i]) > 255) {  /* if lenght of AVP>255 set it to 255 */
            avpair[i][255] = 0;
        }
        arglens += strlen(avpair[i]);
    }

    hdr->datalength = htonl(TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE + i + ulen + plen + alen + arglens);

    auth->authen_method = (u_char)method;
    auth->priv_lvl = (u_char)priv_lvl;
    auth->authen_type = (u_char)authen_type;
    auth->service = (u_char)authen_service;
    auth->user_len = (u_char)ulen;
    auth->port_len = (u_char)plen;
    auth->rem_addr_len = (u_char)alen;
    auth->arg_cnt = (u_char)i;

    for (i = 0; avpair[i] != NULL && strlen(avpair[i]) > 0; i++) {
        *lens = (u_char) strlen(avpair[i]);
        lens += 1;
    }

    /* now filling some data */
    if (ulen > 0) {
        strcpy(lens, user);
        lens += ulen;
    }
    if (plen > 0) {
        strcpy(lens, port);
        lens += plen;
    }
    if (alen > 0) {
        strcpy(lens, rem_addr);
        lens += alen;
    }
    for (i = 0; avpair[i] != NULL && strlen(avpair[i]) > 0; i++) {
        strcpy(lens, avpair[i]);
        lens += (u_char)strlen(avpair[i]);
    }
    /* now send */
    if (tac_write_packet(session, (u_char *)buf)) {
        return 1;
    }
    return 0;
}


/***************************************
     get request (server function)

 return 0 if fails
 1 - if sussess
***************************************/
#ifdef TAC_SERVER_INCLUDE
int tac_author_get_request_s(char *buf,
                             struct session *session,
                             int *method,
                             int *priv_lvl,
                             int *authen_type,
                             int *authen_service,
                             char *user,
                             char *port,
                             char *rem_addr,
                             char **avpair)
{
    int arglens = 0;
    int i;
    int l[255];       /* I think, not more 255 AV-pairs can be requested */
    char ss[255];

    /* header */
    HDR *hdr = (HDR *)buf;
    /* data */
    struct author *auth = (struct author *)
                          (buf + TAC_PLUS_HDR_SIZE);
    char *lens = (char *)(buf + TAC_PLUS_HDR_SIZE +
                          TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE);

    /* Do some sanity checks */
    if (hdr->type != TAC_PLUS_AUTHOR) {
        (void)tac_error("This is no AUTHOR request\n");
        return 0;
    }
    if (hdr->seq_no != 1) {
        (void)tac_error("Error in sequence in AUTHOR/REQUEST\n");
        return 0;
    }
    session->session_id = ntohl(hdr->session_id);

    /* count length */
    for (i = 0; i < auth->arg_cnt; i++) {
        arglens += (int) * (lens + i);
    }

    if (hdr->datalength != htonl(TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE +
                                 auth->arg_cnt + auth->user_len + auth->port_len +
                                 auth->rem_addr_len + arglens)) {
        (void)tac_error("Error in AUTHOR/REQUEST packet, check keys\n");
        return 0;
    }
    *method = auth->authen_method;
    *priv_lvl = auth->priv_lvl;
    *authen_type = auth->authen_type;
    *authen_service = auth->service;

    /* count length */
    for (i = 0; i < auth->arg_cnt; i++) {
        l[i] = (int) * lens;
        lens++;
    }

    strncpy(user, lens, auth->user_len);
    user[auth->user_len + 1] = 0;
    lens += auth->user_len;

    strncpy(port, lens, auth->port_len);
    user[auth->port_len + 1] = 0;
    lens += auth->port_len;

    strncpy(rem_addr, lens, auth->rem_addr_len);
    user[auth->rem_addr_len + 1] = 0;
    lens += auth->rem_addr_len;

    /* reviewing avpairs */
    for (i = 0 ; i < auth->arg_cnt; i++) {
        strncpy(ss, lens, l[i]);
        lens += l[i];
        ss[l[i] + 1] = 0; /* set 0 */
        avpair[i] = tac_callout_strdup(ss);
        avpair[i + 1] = NULL;
    }
    /* hmmm, this is strange, but... I think, all... */
    return 1;
}

int tac_author_get_request(struct session *session,
                           int *method,
                           int *priv_lvl,
                           int *authen_type,
                           int *authen_service,
                           char *user,
                           char *port,
                           char *rem_addr,
                           char **avpair)
{
    char *buf = (char *)tac_read_packet(session);
    if (buf == NULL) {
        return 0;
    }
    return (tac_author_get_request_s(buf, session, method, priv_lvl,
                                     authen_type, authen_service, user, port, rem_addr, avpair));
}
#endif /* TAC_SERVER_INCLUDE */


/* RESPONSEs processing *
status =
TAC_PLUS_AUTHOR_STATUS_PASS_ADD  := 0x01
TAC_PLUS_AUTHOR_STATUS_PASS_REPL := 0x02
TAC_PLUS_AUTHOR_STATUS_FAIL      := 0x10
TAC_PLUS_AUTHOR_STATUS_ERROR     := 0x11
TAC_PLUS_AUTHOR_STATUS_FOLLOW    := 0x21
*/
#define TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE 6
/* An authorization reply packet */
struct author_reply {
    u_char status;
    u_char arg_cnt;
    u_short msg_len;
    u_short data_len;

    /* <arg_cnt u_chars containing the lengths of arg 1 to arg n> */
    /* <msg_len bytes of char data> */
    /* <data_len bytes of char data> */
    /* <char data for each arg> */
};

/****************************************
 * send RESPONSE (server function) *

     0 - error
     1 - success

 ****************************************/
#ifdef TAC_SERVER_INCLUDE
int tac_author_send_response(struct session *session,
                             const int status,
                             const char *server_msg,
                             const char *data,
                             const char **avpair)
{
    char buf[256];
    /* header */
    HDR *hdr = (HDR *)buf;
    /* data */
    struct author_reply *auth = (struct author_reply *)
                                (buf + TAC_PLUS_HDR_SIZE);
    char *lens = (char *)(buf + TAC_PLUS_HDR_SIZE +
                          TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE);
    int arglens = 0;
    int i;

    bzero(buf, sizeof(buf));

    hdr->version = TAC_PLUS_VER_0;
    hdr->type = TAC_PLUS_AUTHOR;
    hdr->seq_no = ++session->seq_no;
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
    hdr->session_id = htonl(session->session_id);

    /* count length */
    for (i = 0; avpair[i] != NULL; i++) {
        arglens += strlen(avpair[i]);
        *lens = (u_char)strlen(avpair[i]);
        lens++;
    }
    hdr->datalength = htonl(TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE +
                            i + strlen(server_msg) + strlen(data) + arglens);

    auth->status = (u_char)status;
    auth->arg_cnt = (u_char)i;
    auth->msg_len = (u_int)strlen(server_msg);
    auth->data_len = (u_int)strlen(data);

    /* lens we are filled above */

    strcpy(lens, server_msg);
    lens += strlen(server_msg);

    strcpy(lens, data);
    lens += strlen(data);

    /* process avpairs */
    for (i = 0; avpair[i] != NULL; i++) {
        strcpy(lens, avpair[i]);
        lens += strlen(avpair[i]);
    }
    /* now we can send to NAS */
    if (tac_write_packet(session, (u_char *)buf)) {
        return 1;
    }
    return 0;
}
#endif /* TAC_SERVER_INCLUDE */


/*********************************************************
*     get RESPONSE (client function)  return status      *
**********************************************************/
int tac_author_get_response(struct session *session,
                            char *server_msg,
                            size_t server_msg_len,
                            char *data,
                            size_t data_len,
                            char **avpair)
{
    int status;
    char *buf = (char *)tac_read_packet(session);
    struct author_reply *auth = (struct author_reply *)(buf + TAC_PLUS_HDR_SIZE);
    HDR *hdr = (HDR *)buf;
    int i;
    char *lens = (char *)(buf + TAC_PLUS_HDR_SIZE +  TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE);
    size_t l[255]; /* No more than 255 avpairs can be processed */
    size_t arglens = 0;
    size_t mlen;
    size_t dlen;
    char ss[256]; /* Each avpair can be up to 255 bytes - make room for '\0' */

    if (buf == NULL) {
        return 0;
    }
    /* Do some checks */
    if (session == NULL) {
        tac_callout_free(buf);
        return -1;
    }
    if (hdr->type != TAC_PLUS_AUTHOR) {
        (void)tac_error("This is not AUTHOR request\n");
        tac_callout_free(buf);
        return 0;
    }
    if (hdr->seq_no != 2) {
        (void)tac_error("Error in sequence in AUTHOR/RESPONSE packet\n");
        tac_callout_free(buf);
        return 0;
    }
    session->session_id = ntohl(hdr->session_id);

    status = auth->status;

    avpair[0] = NULL;
    if (status == TAC_PLUS_AUTHOR_STATUS_ERROR) {
        tac_callout_free(buf);
        return (status);
    }

    /* count length */
    for (i = 0; i < auth->arg_cnt ; i++) {
        arglens += (u_char)(*lens);
        l[i] = (u_char)(*lens);
        lens++;
    }

    /* fields length */
    mlen = ntohs(auth->msg_len);
    dlen = ntohs(auth->data_len);

    if (hdr->datalength != htonl(TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE + auth->arg_cnt + mlen + dlen + arglens)) {
        (void)tac_error("Error in AUTHOR/RESPONSE packet, check keys\n");
        tac_callout_free(buf);
        return (status);
    }

    strncpy(server_msg, lens, MIN(mlen, server_msg_len)); /* Don't copy outside server_msg */
    server_msg[MIN(mlen, server_msg_len - 1)] = '\0';
    lens += mlen;

    strncpy(data, lens, MIN(dlen, data_len)); /* Don't copy outside data */
    data[MIN(dlen, data_len - 1)] = '\0';
    lens += dlen;

    /* write avpairs */
    for (i = 0; i < auth->arg_cnt ; i++) {
        strncpy(ss, lens, MIN(l[i], sizeof(ss))); /* Don't copy outside ss */
        ss[MIN(l[i], sizeof(ss) - 1)] = '\0';
        lens += l[i];
        avpair[i] = tac_callout_strdup(ss);
        avpair[i + 1] = NULL;
    }

    tac_callout_free(buf);
    return (status);
}
