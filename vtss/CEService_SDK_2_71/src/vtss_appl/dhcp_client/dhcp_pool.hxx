/*

 Vitesse software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/

#ifndef _DHCP_POOL_H_
#define _DHCP_POOL_H_

#include "dhcp_client.hxx"

inline void *operator new(size_t size, void *ptr) { return ptr; };
#define VTSS_NEW(DST, TYPE, ...)           \
    DST = (TYPE*)VTSS_CALLOC_MODID(VTSS_MODULE_ID_DHCP_CLIENT, 1, sizeof(TYPE)); \
    if (DST) {                             \
        new(DST) TYPE(__VA_ARGS__);        \
    }                                      \

#define VTSS_DELETE(DST, DESTRUCTOR)       \
    if (DST) {                             \
        DST-> DESTRUCTOR();                \
        VTSS_FREE(DST);                    \
    }


extern "C" {
#include "ip2_utils.h"
}


#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_CLIENT

#define OK_OR_RETURN_ERR(VID)     \
    do {                          \
        VTSS_ASSERT(VID < MAX);   \
        if (client[VID] == 0) {   \
            return VTSS_RC_ERROR; \
        }                         \
    }while(0)

#define ACK_OR_RETURN_ERR(VID, FRAME)                                    \
    OK_OR_RETURN_ERR(VID);                                               \
    typename DhcpClient_t::AckConfPacket FRAME(client[VID]->ack_conf()); \
    do {                                                                 \
        if (! FRAME.valid()) {                                           \
            return VTSS_RC_ERROR;                                        \
        }                                                                \
    } while(0);


namespace VTSS {
namespace Dhcp {

template<typename FrameTxService,
         typename TimerService,
         typename Lock,
         unsigned MAX>
class DhcpPool
{
public:
    typedef Client<FrameTxService, TimerService, Lock> DhcpClient_t;

    struct CStyleCB : public VTSS::AbstractTriggerHandler
    {
        enum { MAX_CALLEES = 1 };

        CStyleCB(DhcpClient_t *c, vtss_vid_t v) :
            trigger(this), client_(c), vlan_(v)
        {
            for (int i = 0; i < MAX_CALLEES; ++i) {
                callees[i] = 0;
            }

            client_->ack_conf(trigger);
        }

        vtss_rc callback_add(vtss_dhcp_client_callback_t cb) {
            int free = -1;

            for (int i = 0; i < MAX_CALLEES; ++i) {
                if (callees[i] == 0) {
                    free = i;
                }

                if (callees[i] == cb) {
                    T_I("Callback already exists");
                    return VTSS_RC_OK;
                }
            }

            if (free != -1) {
                callees[free] = cb;
                return VTSS_RC_OK;
            } else {
                return VTSS_RC_ERROR;
            }
        }

        vtss_rc callback_del(vtss_dhcp_client_callback_t cb) {
            for (int i = 0; i < MAX_CALLEES; ++i) {
                if (callees[i] != cb) {
                    continue;
                }
                callees[i] = 0;
                return VTSS_RC_OK;
            }
            return VTSS_RC_ERROR;
        }

        void execute(VTSS::Trigger & t, VTSS::Trigger::subject_status_t status)
        {
            if (status != VTSS::Trigger::success) {
                return;
            }

            client_->ack_conf(t);

            for (int i = 0; i < MAX_CALLEES; ++i) {
                vtss_dhcp_client_callback_t cb = callees[i];
                if (cb == 0) {
                    continue;
                }
                cb(vlan_);
            }
        }

        ~CStyleCB() {
            trigger.unlink();
        }

        Trigger trigger;
        DhcpClient_t *client_;
        const vtss_vid_t vlan_;
        vtss_dhcp_client_callback_t callees[MAX_CALLEES];
    };


    DhcpPool(FrameTxService & tx, TimerService & ts, Lock & l) :
        tx_(tx), ts_(ts), lock_(l)
    {
        for (unsigned i = 0; i < MAX; ++i) {
            client[i] = 0;
            c_style_cb[i] = 0;
        }
    }

    vtss_rc start(vtss_vid_t vid) {
        VTSS_ASSERT(vid < MAX);

        if (client[vid] == 0) {
            T_I("Creating new client");
            VTSS_NEW(client[vid], DhcpClient_t, tx_, ts_, lock_, vid);
            VTSS_NEW(c_style_cb[vid], CStyleCB, client[vid], vid);
        } else {
            T_I("Restarting existing client");
        }

        if (client[vid] == 0 || c_style_cb[vid] == 0) {
            VTSS_DELETE(c_style_cb[vid], ~CStyleCB);
            VTSS_DELETE(client[vid], ~DhcpClient_t);
            c_style_cb[vid] = 0;
            client[vid] = 0;
            return VTSS_RC_ERROR;
        }

        return client[vid]->start();
    }

    vtss_rc kill(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);

        vtss_rc rc;
        rc = client[vid]->stop();

        VTSS_DELETE(c_style_cb[vid], ~CStyleCB);
        VTSS_DELETE(client[vid], ~DhcpClient_t);
        c_style_cb[vid] = 0;
        client[vid] = 0;

        client[vid] = 0;
        c_style_cb[vid] = 0;

        return rc;
    }

    vtss_rc stop(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->stop();
    }

    vtss_rc fallback(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->fallback();
    }

    vtss_rc if_down(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->if_down();
    }

    vtss_rc if_up(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->if_up();
    }

    vtss_rc release(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->release();
    }

    vtss_rc decline(vtss_vid_t vid) {
        OK_OR_RETURN_ERR(vid);
        return client[vid]->decline();
    }

    DhcpClient_t * get(vtss_vid_t vid) {
        return client[vid];
    }

    BOOL bound_get(vtss_vid_t vid)
    {
        OK_OR_RETURN_ERR(vid);
        vtss_dhcp4c_state_t s = client[vid]->state();

        return s == DHCP4C_STATE_BOUND ||
            s == DHCP4C_STATE_RENEWING ||
            s == DHCP4C_STATE_REBINDING;
    }

    vtss_rc offers_get(vtss_vid_t vid,
                       vtss_dhcp_client_offer_list_t *list)
    {
        OK_OR_RETURN_ERR(vid);
        vtss_dhcp_client_offer_list_t o = client[vid]->offers();
        *list = o;
        return VTSS_RC_OK;
    }

    vtss_rc accept(vtss_vid_t vid, unsigned   idx)
    {
        OK_OR_RETURN_ERR(vid);

        if (client[vid]->accept(idx)) {
            return VTSS_RC_OK;
        } else {
            return VTSS_RC_ERROR;
        }
    }

    vtss_rc status(vtss_vid_t vid,
                   vtss_dhcp_client_status_t *status)
    {
        OK_OR_RETURN_ERR(vid);
        vtss_dhcp_client_status_t s = client[vid]->status();
        *status = s;
        return VTSS_RC_OK;
    }

    vtss_rc callback_add(vtss_vid_t vid, vtss_dhcp_client_callback_t cb)
    {
        OK_OR_RETURN_ERR(vid);
        return c_style_cb[vid]->callback_add(cb);
    }

    vtss_rc callback_del(vtss_vid_t vid, vtss_dhcp_client_callback_t cb)
    {
        OK_OR_RETURN_ERR(vid);
        return c_style_cb[vid]->callback_del(cb);
    }

    vtss_rc fields_get(vtss_vid_t vid, vtss_dhcp_fields_t *fields)
    {
        OK_OR_RETURN_ERR(vid);

        typename DhcpClient_t::AckConfPacket p(client[vid]->ack_conf());

        if (p.valid()) {
            *fields = p.get().data;
            return VTSS_RC_OK;
        } else {
            return VTSS_RC_ERROR;
        }
    }

    vtss_rc dns_option_any_get(vtss_ipv4_t  prefered,
                               vtss_ipv4_t *ip)
    {
        vtss_ipv4_t some_ip = 0;

        for (int i = 0; i < VTSS_VIDS; ++i) {
            vtss_dhcp_fields_t fields;

            if (fields_get(i, &fields) == VTSS_RC_OK && fields.has_domain_name_server) {
                if (fields.domain_name_server == prefered) {
                    *ip = fields.domain_name_server;
                    return VTSS_RC_OK;
                } else {
                    some_ip = fields.domain_name_server;
                }
            }
        }

        if (some_ip != 0) {
            *ip = some_ip;
            return VTSS_RC_OK;
        }

        return VTSS_RC_ERROR;
    }

    ~DhcpPool() {
        for (unsigned i = 0; i < MAX; ++i) {
            VTSS_DELETE(c_style_cb[i], ~CStyleCB);
            c_style_cb[i] = 0;
            VTSS_DELETE(client[i], ~DhcpClient_t);
            client[i] = 0;
        }
    }

private:
    FrameTxService & tx_;
    TimerService & ts_;
    Lock & lock_;
    DhcpClient_t *client[MAX];
    CStyleCB *c_style_cb[MAX];
};
};
};

#undef OK_OR_RETURN_ERR
#undef ACK_OR_RETURN_ERR
#undef VTSS_TRACE_MODULE_ID
#endif /* _DHCP_POOL_H_ */
