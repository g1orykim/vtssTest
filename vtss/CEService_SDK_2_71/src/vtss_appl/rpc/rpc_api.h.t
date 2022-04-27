/* -*- mode: C -*- */
[% INCLUDE copyright -%]

#ifndef _RPC_API_H_
#define _RPC_API_H_

/*
 * Include files
 */
[% FOREACH inc = includes.include -%]
#include "[% inc.file %]"
[% END -%]

typedef void *rpc_pointer;

enum {
    MSG_TYPE_REQ,               /* Request */
    MSG_TYPE_RSP,               /* Response */
    MSG_TYPE_EVT,               /* Event (unsolicited indication) */
};

typedef struct rpc_msg {
    /* Local householding */
    struct rpc_msg *next;     /* Link pointer, for request queueing use */
    struct rpc_msg *rsp;      /* Link pointer, for response message */
    void *syncro;             /* Synchronization object pointer */
    u32 timeout;              /* For implementing message timeouts */
    /* Communication Message Fields */
    u16		   id;          /* Request or event ordinal */
    u16		   type;        /* Message type */
    u32		   rc;          /* Return code from request execution */
    u32		   seq;         /* Message sequence number for correlation */
    /* Buffer management */
    size_t         size;        /* Total (max) length of buffer */
    size_t         available;   /* Current bytes available in buffer */
    size_t         length;      /* Current bytes in buffer */
    u8             *data;       /* Start of buffer */
    u8             *head;       /* Current start of buffer */
    u8             *tail;       /* Current end of buffer */
} rpc_msg_t;

static inline void rpc_msg_init(rpc_msg_t *msg, u8 *ptr, size_t size)
{
    msg->size = msg->available = size;
    msg->length = 0;
    msg->data = msg->head = msg->tail = ptr;
}

static inline void rpc_msg_put(rpc_msg_t *msg, size_t adj)
{
    msg->length += adj;
    msg->tail += adj;
}

static inline u8 *rpc_msg_push(rpc_msg_t *msg, size_t adj)
{
    if(adj <= msg->available) {
        u8 *ptr = msg->tail;
        msg->available -= adj;
        msg->length += adj;
        msg->tail += adj;
        return ptr;
    }
    return NULL;
}

static inline u8 *rpc_msg_pull(rpc_msg_t *msg, size_t adj)
{
    if(adj <= msg->length) {
        u8 *ptr = msg->head;
        msg->length -= adj;
        msg->head += adj;
        return ptr;
    }
    return NULL;
}

/*
 * Marshalling options
 *
 * RPC_PACK: Set if data objects are byte-packed in a consistent,
 * CPU-independent manner.
 *
 * Otherwise, data objects are 32-bit aligned and the byteorder
 * defined by the RPC_NTOHL(), RPC_HTONL, macros.
 */
//#define RPC_PACK 1
#ifdef RPC_NETWORK_CODING
 #if defined(VTSS_OPSYS_ECOS)
  #include "network.h"
 #endif
 #include <arpa/inet.h>
 #define RPC_NTOHL(x) ntohl(x)
 #define RPC_HTONL(x) htonl(x)
#else
 #define RPC_NTOHL(x) (x)
 #define RPC_HTONL(x) (x)
#endif

/*
 * Trace and error reporting
 */
#define RPC_IOERROR(...) printf(__VA_ARGS__)
#if 0
#define RPC_IOTRACE(...) printf(__VA_ARGS__)
#else
#define RPC_IOTRACE(...)
#endif

void       rpc_message_dispatch(rpc_msg_t *msg);
rpc_msg_t *rpc_message_alloc(void);
void       rpc_message_dispose(rpc_msg_t *msg);
vtss_rc    rpc_message_exec(rpc_msg_t *msg);
vtss_rc    rpc_message_send(rpc_msg_t *msg);

/*
 * Enumerator
 */
enum {
[% FOREACH group = groups -%]
    /* Group  [% group.name %] */
[% FOREACH entry = group.entry -%]
    [% msgid(entry.name) %],
[% END -%]
[% END -%]
    /* Events */
[% FOREACH evt = events.event -%]
    [% msgid(evt.name) %],
[% END -%]
};

/***********************************************************************
 * Utility functions
 */

void init_req(rpc_msg_t *msg, u32 type);
void init_rsp(rpc_msg_t *msg);
void init_evt(rpc_msg_t *msg, u32 type);

/***********************************************************************
 * Base encoders
 */
BOOL encode_u8(rpc_msg_t *msg, u8 arg);
BOOL encode_u16(rpc_msg_t *msg, u16 arg);
BOOL encode_u32(rpc_msg_t *msg, u32 arg);
BOOL encode_rpc_pointer(rpc_msg_t *msg, rpc_pointer arg);
BOOL encode_u64(rpc_msg_t *msg, u64 arg);

BOOL encode_u8_array (rpc_msg_t *msg, const u8  *arg, size_t nelm);
BOOL encode_u16_array(rpc_msg_t *msg, const u16 *arg, size_t nelm);
BOOL encode_u32_array(rpc_msg_t *msg, const u32 *arg, size_t nelm);
BOOL encode_u64_array(rpc_msg_t *msg, const u64 *arg, size_t nelm);

BOOL decode_u8(rpc_msg_t *msg, u8 *arg);
BOOL decode_u16(rpc_msg_t *msg, u16 *arg);
BOOL decode_u32(rpc_msg_t *msg, u32 *arg);
BOOL decode_u64(rpc_msg_t *msg, u64 *arg);
BOOL decode_rpc_pointer(rpc_msg_t *msg, rpc_pointer *arg);

BOOL decode_u8_array (rpc_msg_t *msg, u8  *arg, size_t nelm);
BOOL decode_u16_array(rpc_msg_t *msg, u16 *arg, size_t nelm);
BOOL decode_u32_array(rpc_msg_t *msg, u32 *arg, size_t nelm);
BOOL decode_u64_array(rpc_msg_t *msg, u64 *arg, size_t nelm);

/***********************************************************************
 * Application Type Encoders
 */

/* Scalar coding */
[% FOREACH t = types.type -%]
BOOL decode_[% t.name %](rpc_msg_t *msg, [% t.name %] *arg);
[% IF t.struct -%]
BOOL encode_[% t.name %](rpc_msg_t *msg, const [% t.name %] *arg);
[% ELSE -%]
BOOL encode_[% t.name %](rpc_msg_t *msg, [% t.name %] arg);
[% END -%]
[% END -%]

/* Array coding */
[% FOREACH t = types.type -%]
BOOL encode_[% t.name %]_array(rpc_msg_t *msg, const [% t.name %] *arg, size_t nelm);
BOOL decode_[% t.name %]_array(rpc_msg_t *msg, [% t.name %] *arg, size_t nelm);
[% END -%]

/***********************************************************************
 * RPC 'wrapped' API
 */

[% FOREACH group = groups %]
/* Group '[% group.name %]' */
[% FOREACH func = group.entry -%]
vtss_rc [% prefix %][% func.name %]([% decllist("", paramlist(func)) %]);
[% END %]
[% END %]

/***********************************************************************
 * Event generation and reception
 */

[% FOREACH evt = events.event -%]
/* 
 * Generate '[% evt.name %]' event (output side)
 */
vtss_rc [% evt.name %]([% decllist("", paramlist(evt)) %]);

/* 
 * Consume '[% evt.name %]' event (input side - application defined)
 */
void [% evt.name %]_receive([% decllist("", paramlist(evt)) %]);

[% END %]

void rpc_event_receive(rpc_msg_t *msg); /* Main event sink - RPC module provided */

#endif // _RPC_API_H_

// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
