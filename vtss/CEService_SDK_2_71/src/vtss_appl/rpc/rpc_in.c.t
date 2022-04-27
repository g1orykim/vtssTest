/* -*- mode: C -*- */
[% INCLUDE copyright -%]

#include "rpc_api.h"

[% FOREACH group = groups %]
/*
 * Request Decoding: Group '[% group.name %]'
 */
[% FOREACH func = group.entry -%]
[% plist = paramlist(func, "in") -%]
[% IF nonempty(plist) -%]
static BOOL [% func.name %]_decode_req([% decllist("rpc_msg_t *msg", plist, "ptr") %])
{
    return 
[% FOREACH param = paramlist(func, "in") -%]
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
 * Response Encoding: Group '[% group.name %]'
 */
[% FOREACH func = group.entry -%]
[% IF nonempty(paramlist(func, "out")) -%]
static BOOL [% func.name %]_encode_rsp([% decllist("rpc_msg_t *msg", paramlist(func, "out")) %])
{
    init_rsp(msg);
    return
[% FOREACH param = paramlist(func, "out") -%]
[% IF param.array -%]
        encode_[% param.type %]_array(msg, [% param.name %], [% param.array %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% ELSE -%]
        encode_[% param.type %](msg, [% "*" IF !get_type(param.type).struct %][% param.name %])[% " &&" IF loop.next %][% ";" IF loop.last %]
[% END -%]
[% END -%]
}

[% END -%]
[% END -%]
[% END -%]

void rpc_message_dispatch(rpc_msg_t *msg)
{
    vtss_rc rc = VTSS_RC_INCOMPLETE;
    RPC_IOTRACE("%s: message %d\n", __FUNCTION__, msg->id);
    switch(msg->id) {
[% FOREACH group = groups %]
[% FOREACH func = group.entry -%]
    case [% msgid(func.name) %]:
        {
[% FOREACH param = paramlist(func) -%]
            [% param.type %] [% param.name -%][% IF param.array %][[% param.array %]][% END %];
[% END -%]
            RPC_IOTRACE("Processing %s\n", "[% msgid(func.name) %]");
            if(![% func.name %]_decode_req([% arglist("msg", paramlist(func, "in")) %])) {
                RPC_IOTRACE("Decode %s - failed\n", "[% msgid(func.name) %]");
            } else {
                rc = [% func.name %]([% namelist_p(paramlist(func)) %]);
                RPC_IOTRACE("Execute %s = %d\n", "[% msgid(func.name) %]", rc);
[% IF nonempty(paramlist(func, "out")) -%]
               if(![% func.name %]_encode_rsp([% arglist("msg", paramlist(func, "out")) %])) {
                    RPC_IOTRACE("Encode response %s - failed\n", "[% msgid(func.name) %]");
                    rc = VTSS_RC_INCOMPLETE;
                }
[% END -%]
            }
        }
        break;
[% END %]
[% END %]
    default:
        RPC_IOERROR("Illegal message type: %d\n", msg->id);
    }
    msg->rc = rc;
}
