/* -*- mode: C -*- */
[% INCLUDE copyright -%]

#include "rpc_api.h"

[% FOREACH group = groups %]
/*
 * Request Encoding: Group '[% group.name %]'
 */
[% FOREACH func = group.entry -%]
[% plist = paramlist(func, "in") -%]
[% IF nonempty(plist) -%]
static BOOL [% func.name %]_encode_req([% decllist("rpc_msg_t *msg", plist) %])
{
    init_req(msg, [% msgid(func.name) %]);
    return
[% FOREACH param = paramlist(func, "in") -%]
[% IF param.array -%]
        encode_[% param.type %]_array(msg, [% param.name %], [% param.array %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% ELSE -%]
        encode_[% param.type %](msg, [% param.name %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% END -%]
[% END -%]
}
[% END -%]

[% END %]
[% END %]

[% FOREACH group = groups %]
/*
 * Request Decoding: Group '[% group.name %]'
 */
[% FOREACH func = group.entry -%]
[% plist = paramlist(func, "out") -%]
[% IF nonempty(plist) -%]
static BOOL [% func.name %]_decode_rsp([% decllist("rpc_msg_t *msg", plist) %])
{
     return
[% FOREACH param = plist -%]
[% IF param.array -%]
         decode_[% param.type %]_array(msg, [% param.name %], [% param.array %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% ELSE -%]
         decode_[% param.type %](msg, [% param.name %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% END -%]
[% END -%]
}
[% END -%]

[% END %]
[% END %]

[% FOREACH group = groups %]

/*
 * Shim API: Group '[% group.name %]'
 */
[% FOREACH func = group.entry -%]
vtss_rc [% prefix %][% func.name %]([% decllist("", paramlist(func)) %])
{
    rpc_msg_t *msg = rpc_message_alloc();
    vtss_rc rc;
    if(!msg)
        return VTSS_RC_INCOMPLETE;
    if(![% func.name %]_encode_req([% namelist("msg", paramlist(func, "in")) %]))
        return VTSS_RC_INCOMPLETE;
    rc = rpc_message_exec(msg);
[% IF nonempty(paramlist(func, "out")) -%]
    if(rc == VTSS_RC_OK) {
        // Copy out return values, rc, ...
        if(msg->rsp == NULL ||
           ![% func.name %]_decode_rsp([% namelist("msg->rsp", paramlist(func, "out")) -%]))
            rc = VTSS_RC_INCOMPLETE;
    }
[% END -%]
    // Dispose message buffers
    if(msg->rsp != NULL)
        rpc_message_dispose(msg->rsp);
    rpc_message_dispose(msg);
    return rc;
}

[% END -%]
[% END -%]

