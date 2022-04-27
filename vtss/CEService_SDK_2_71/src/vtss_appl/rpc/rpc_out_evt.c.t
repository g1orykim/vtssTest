/* -*- mode: C -*- */
[% INCLUDE copyright %]

#include "rpc_api.h"

/*
 * Event generation
 */

[% FOREACH evt = events.event -%]

static BOOL [% evt.name %]_encode([% decllist("rpc_msg_t *msg", paramlist(evt)) %])
{
    init_evt(msg, [% msgid(evt.name) %]);
    return
[% FOREACH param = paramlist(evt) -%]
        encode_[% param.type %](msg, [% param.name %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% END -%]
}

vtss_rc [% evt.name %]([% decllist("", paramlist(evt)) %])
{
    rpc_msg_t *msg = rpc_message_alloc();
    if(!msg)
        return VTSS_RC_INCOMPLETE;
    if(![% evt.name %]_encode([% namelist("msg", paramlist(evt)) %]))
        return VTSS_RC_INCOMPLETE;
    return rpc_message_send(msg);
}
[% END %]
