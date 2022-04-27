/*

 Vitesse API software.

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

#include "main.h"
#include "vtss_types.h"
#include "packet_api.h"

#include "critd_api.h"
#include "ip2_types.h"
#include "ip2_trace.h"
#include "ip2_utils.h"
#include "ip2_os_api.h" // should not be needed, move type to types
#include "ip2_chip_api.h"
#include "vtss_simple_fifo.h"
#include "vlan_api.h"
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#include "ce_max_api.h"
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_IP2
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP2
#define __GRP VTSS_TRACE_IP2_GRP_OS_DRIVER
#define E(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define W(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define I(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define D(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define N(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define R(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)

#ifndef _KERNEL
#define _KERNEL /* Must have _KERNEL sys/mbuf.h */
#endif
#include <sys/param.h>
#include <sys/mbuf.h>

#include <stdlib.h>
#include <network.h>
#include <sys/sysctl.h>

#ifndef _KERNEL
#define _KERNEL /* Must have _KERNEL defined for sc_arpcom / cyg/io/eth/eth_drv.h */
#endif

#include <net/if_dl.h>
#include <net/if_var.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <net/if_types.h>
#include <netinet/ip_var.h>
#include <cyg/io/eth/eth_drv.h>

typedef struct {
    char                    ifname[16];
    BOOL                    started;
    BOOL                    reject_ioctl;
    vtss_vid_t              vid;
    u32                     if_index;
    vtss_mac_t              mac;
    struct eth_drv_mc_list  mclist;  // Current multicast list
    const void             *rx_frm;  // Latest received frame,
    unsigned long           rx_len;  // Latest received frame's length
} priv_t;

// This structure can be used to map if_index to vlan number after the interface
// has been deleted.
// The table is therefor only updated when new interfaces are added!
struct {
    vtss_vid_t              vid;
    int                     if_index;
} if_id_map[IP2_MAX_INTERFACES];

typedef struct {
    struct eth_drv_sc sc;
    priv_t            priv;
} driver_pair_t;

typedef enum  {
    MAC_ADD,
    MAC_DEL
} mac_opr_t;

typedef struct {
    vtss_mac_t mac;
    vtss_vid_t vid;
    mac_opr_t  opr;
} mac_rec_t;

struct m_rtmsg {
    struct rt_msghdr m_rtm;
    char m_space[128];
};

#define ROUNDUP(a) \
    ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

FIFO_DECL_STATIC(mac_subscriptions_fifo, mac_rec_t, 1024);
static u32 mac_subscriptions_fifo_overflow = 0;

static cyg_handle_t thread_handle;
static cyg_thread   thread_block;
static char         thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define WAKEUP VTSS_BIT(0)
static cyg_flag_t   thread_control_flags;

static driver_pair_t *drivers[VTSS_VIDS],
       *driver_memory[VTSS_VIDS];  /* Cache - we never free interface structures - (blame eCos) */

static const vtss_mac_t broadcast_mac = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

static void new_mapping(vtss_vid_t vid, int if_index_)
{
    int i;
    int free_index = -1;

#define LATCH_FREE(I) \
    if (free_index == -1) free_index = I

    for (i = 0; i < (int)IP2_MAX_INTERFACES; ++i) {
        if (if_id_map[i].vid == 0) {
            LATCH_FREE(i);
        }

        // delete existing vid
        if (if_id_map[i].vid == vid) {
            LATCH_FREE(i);
            if_id_map[i].vid = 0;
            if_id_map[i].if_index = 0;
        }

        if (if_id_map[i].if_index == if_index_ ) {
            LATCH_FREE(i);
            if_id_map[i].vid = 0;
            if_id_map[i].if_index = 0;
        }
    }

    VTSS_ASSERT(free_index >= 0 && free_index < (int)IP2_MAX_INTERFACES);
    if_id_map[free_index].vid = vid;
    if_id_map[free_index].if_index = if_index_;
}

vtss_vid_t vtss_ip2_ecos_driver_if_index_to_vid(int idx)
{
    int i, res = 0;

    cyg_scheduler_lock();
    for (i = 0; i < (int)IP2_MAX_INTERFACES; ++i) {
        if (if_id_map[i].vid == 0) {
            continue;
        }

        if (if_id_map[i].if_index == idx) {
            res = if_id_map[i].vid;
            break;
        }
    }
    cyg_scheduler_unlock();

    return res;
}

int vtss_ip2_ecos_driver_vid_to_if_index(vtss_vid_t idx)
{
    int i, ifidx;

    ifidx = -1;
    if (idx < VLAN_ID_MIN || idx > VLAN_ID_MAX) {
        return ifidx;
    }

    cyg_scheduler_lock();
    for (i = 0; i < (int)IP2_MAX_INTERFACES; ++i) {
        if (if_id_map[i].vid == idx) {
            ifidx = if_id_map[i].if_index;
            break;
        }
    }
    cyg_scheduler_unlock();

    return ifidx;
}

static vtss_rc async_mac_sub(vtss_vid_t vlan, const vtss_mac_t *mac)
{
    vtss_rc rc;
    mac_rec_t rec;
    rec.mac = *mac;
    rec.vid = vlan;
    rec.opr = MAC_ADD;

    I("Async subscribe %u "VTSS_MAC_FORMAT, vlan, VTSS_MAC_ARGS(*mac));

    cyg_scheduler_lock();
    if (FIFO_FULL(mac_subscriptions_fifo)) {
        mac_subscriptions_fifo_overflow ++;
        rc = VTSS_RC_ERROR;
    } else {
        FIFO_PUT(mac_subscriptions_fifo, rec);
        rc = VTSS_RC_OK;
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return rc;
}

static vtss_rc async_mac_unsub(vtss_vid_t vlan, const vtss_mac_t *mac)
{
    vtss_rc rc;
    mac_rec_t rec;
    rec.mac = *mac;
    rec.vid = vlan;
    rec.opr = MAC_DEL;

    I("Async unsubscribe %u "VTSS_MAC_FORMAT, vlan, VTSS_MAC_ARGS(*mac));

    cyg_scheduler_lock();
    if (FIFO_FULL(mac_subscriptions_fifo)) {
        mac_subscriptions_fifo_overflow ++;
        rc = VTSS_RC_ERROR;
    } else {
        FIFO_PUT(mac_subscriptions_fifo, rec);
        rc = VTSS_RC_OK;
    }
    cyg_flag_setbits(&thread_control_flags, WAKEUP);
    cyg_scheduler_unlock();

    return rc;
}

static void mclist_clean(vtss_vid_t vlan, struct eth_drv_mc_list *list)
{
    int i;

    /* Remove old list */
    for (i = 0; i < list->len; i++) {
        (void) async_mac_unsub(vlan, (vtss_mac_t *)list->addrs[i]);
    }

    list->len = 0;
}

static BOOL mclist_find(const struct eth_drv_mc_list *pmc,
                        const unsigned char *mac)
{
    int i;

    for (i = 0; i < pmc->len; i++) {
        if (memcmp(pmc->addrs[i], mac, ETHER_ADDR_LEN) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static void mclist_update(vtss_vid_t vlan,
                          struct eth_drv_mc_list *pMCcur,
                          const struct eth_drv_mc_list *pMCnew)
{
    int i;

    /* Add new */
    for (i = 0; i < pMCnew->len; i++) {
        if (!mclist_find(pMCcur, pMCnew->addrs[i])) {
            (void) async_mac_sub(vlan, (vtss_mac_t *)pMCnew->addrs[i]);
        }
    }

    /* Purge removed */
    for (i = 0; i < pMCcur->len; i++) {
        if (!mclist_find(pMCnew, pMCcur->addrs[i])) {
            (void) async_mac_unsub(vlan, (vtss_mac_t *)pMCcur->addrs[i]);
        }
    }

    /* Update MC list */
    *pMCcur = *pMCnew;
}

static void eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    priv_t *priv;

    priv = sc->driver_private;
    if (!priv->started) {
        I("Start interface %u", priv->vid);
        priv->mclist.len = 0;
        priv->rx_len = 0;
        priv->rx_frm = NULL;
        (void) async_mac_sub(priv->vid, &priv->mac);
        (void) async_mac_sub(priv->vid, &broadcast_mac);
        priv->reject_ioctl = FALSE;
        priv->started = TRUE;
    }
}

static void _stop(struct eth_drv_sc *sc)
{
    priv_t *priv;

    priv = sc->driver_private;
    if (priv->started) {
        /* Tear down current MC list */
        I("Stop interface %u", priv->vid);
        mclist_clean(priv->vid, &priv->mclist);
        (void) async_mac_unsub(priv->vid, &broadcast_mac);
        (void) async_mac_unsub(priv->vid, &priv->mac);
        priv->started = FALSE;

        /* The IP stack insist on deleting the multicast addresses it has
         * subscribed on one by one. The problem with this approch is that it
         * does this by setting the entierer list of multicast addresses where
         * one address is removed at a time, and it does not remove all the
         * addresses. To clean this up we manually remove all the multicast
         * addresses when an interface is deleted, and from  this point we
         * ignore all IOCTL requests on this interface. */
        priv->reject_ioctl = TRUE;
    }
}

static void eth_stop(struct eth_drv_sc *sc)
{
    I("%s", __FUNCTION__);
    _stop(sc);
}

static int eth_ioctl(struct eth_drv_sc *sc, unsigned long key,
                     void *data, int len)
{
    int res = 0; // Expect success
    priv_t *priv;

    priv = sc->driver_private;

    if (priv->reject_ioctl) {
        I("IOCTL ignore %u", priv->vid);
        return 0;
    }

    switch (key) {
#ifdef ETH_DRV_GET_MAC_ADDRESS
    case ETH_DRV_GET_MAC_ADDRESS:
        I("IOCTL ETH_DRV_GET_MAC_ADDRESS %u", priv->vid);
        memcpy((uchar *)data, priv->mac_address.addr, 6);
        break;
#endif

    case ETH_DRV_SET_MAC_ADDRESS:
        res = 1;
        break;

    case ETH_DRV_SET_MC_LIST: {
        // TODO, check that len matches pMC->len
        I("IOCTL ETH_DRV_SET_MC_LIST %u", priv->vid);
        struct eth_drv_mc_list *pMC = data;
        mclist_update(priv->vid, &priv->mclist, pMC);
        break;
    }

    case ETH_DRV_SET_MC_ALL:
        res = 1;
        break;

    case ETH_DRV_GET_IF_STATS_UD:
    case ETH_DRV_GET_IF_STATS:
        // Not supported anymore
        res = 1;
        break;

    default:
        res = 1;
    }

    return res;
}

/****************************************************************************/
// eth_tx_done()
// Called by packet module when the frame has been transmitted (or attempted
// transmitted)
/****************************************************************************/
static void eth_tx_done(void *contxt, packet_tx_done_props_t *props)
{
    unsigned char *extra_ptr;

    extra_ptr = (unsigned char *)contxt;
    // extra_ptr points to the vid+key that we should callback the upper layer with.
    VTSS_ASSERT(extra_ptr);
    if (extra_ptr) {
        vtss_vid_t vid = (vtss_vid_t) ((unsigned long *)extra_ptr)[0];
        if (vid < VTSS_VIDS) {
            driver_pair_t *driver;
            cyg_scheduler_lock();
            driver = drivers[vid];
            if (driver) {
                // Tell upper layer that we're done with this sglist
                driver->sc.funs->eth_drv->tx_done(&driver->sc, ((unsigned long *)extra_ptr)[1], !props->tx);
            } else {
                /* Driver gone, free this ourselves */
                W("tx_complete: VID %d gone", vid);
                m_freem((struct mbuf *) ((unsigned long *)extra_ptr)[1]);
            }
            cyg_scheduler_unlock();
        } else {
            E("Bogus VID: %d", vid);
        }
        // Release buffer
        N("eth_tx_done %x", (unsigned)extra_ptr);
        packet_tx_free_extra(extra_ptr);
    }
}

/****************************************************************************/
// eth_send()
// Called by IP stack when a scatter/gather list is ready to be transmitted.
// The scatter/gather list is simply an array of buf and len items.
/****************************************************************************/
static void eth_send(struct eth_drv_sc *sc,
                     struct eth_drv_sg *sg_list,
                     int sg_len,
                     int total_len,
                     unsigned long key)
{
    priv_t *priv;
    unsigned char *frm, *dst, *extra_ptr;
    packet_tx_props_t tx_props;
    vtss_rc rc;
    int alloc_len;

    priv = sc->driver_private;
    alloc_len = MAX(60, total_len);

    // total_len doesn't include room for the FCS, which is fine, since that's what the API expects.

    // Allocate a TX packet.
    // Besides reserving room for IFH, CMD, and FCS, packet_tx_alloc_extra()
    // reserved room for a user-defined number of 32-bit words, which we use
    // to save off the sc+key so that we can have several outstanding packets
    // and still callback the IP stack when eth_tx_done() is called.
    if ((frm = packet_tx_alloc_extra(alloc_len, 2, &extra_ptr)) == NULL) {
        sc->funs->eth_drv->tx_done(sc, key, -1);
        return;
    }

    // Copy data from scatter/gather list into dst
    dst = frm;
    while (sg_len) {
        memcpy(dst, (unsigned char *)sg_list->buf, sg_list->len);
        dst += sg_list->len;
        sg_list++;
        sg_len--;
    }

    // Pad with zeros if total_len < alloc_len to avoid the Etherleak vulnerability.
    if (total_len < alloc_len) {
        memset(&frm[total_len], 0, alloc_len - total_len);
    }

    // Save the vid+key, so that eth_tx_done() can find them.
    ((unsigned long *)extra_ptr)[0] = (unsigned long) priv->vid;;
    ((unsigned long *)extra_ptr)[1] = key;

    T_NG    (TRACE_GRP_TXPKT_DUMP, "Tx Packet len = %d", total_len);
    T_NG_HEX(TRACE_GRP_TXPKT_DUMP, frm, MIN(60, total_len));

    packet_tx_props_init(&tx_props);
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    tx_props.tx_info.switch_frm = FALSE;
    if ((rc = ce_max_mgmt_ip_mgmt_active_port_mask_get(priv->vid, &tx_props.tx_info.dst_port_mask)) != VTSS_RC_OK) {
        E("CE_MAX: No IP mgmt ports, %s", error_txt(rc));
        goto CLEANUP;
    }
#else
    tx_props.tx_info.switch_frm = TRUE;
#endif
    tx_props.packet_info.modid              = VTSS_MODULE_ID_IP_STACK_GLUE;
    tx_props.packet_info.frm[0]             = frm;
    tx_props.packet_info.len[0]             = alloc_len;
    tx_props.tx_info.tag.vid                = priv->vid;
    tx_props.packet_info.tx_done_cb         = eth_tx_done;
    tx_props.packet_info.tx_done_cb_context = extra_ptr;
    if ((rc = packet_tx(&tx_props)) != VTSS_RC_OK) {
        /* Explicit cleanup */
        D("%u: Frame transmit error - %s", priv->vid, error_txt(rc));
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
CLEANUP:
#endif
        sc->funs->eth_drv->tx_done(sc, key, -1);
        packet_tx_free_extra(extra_ptr);
    }
}

static void eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    int len;
    priv_t *priv;
    const unsigned char *src;
    int rem_len_in_sg_item;

    priv = sc->driver_private;
    rem_len_in_sg_item = sg_list->len;

    if (!priv->rx_len) {
        T_W("Zero length packet");
        return;
    }

    if (!priv->rx_frm) {
        T_W("Data null ptr");
        return;
    }

    len = priv->rx_len;
    src = priv->rx_frm; // We're not allowed to change priv->rx_frm.

    T_NG    (TRACE_GRP_RXPKT_DUMP, "Rx Packet len = %u", len);
    T_NG_HEX(TRACE_GRP_RXPKT_DUMP, src, MIN(60, len));

    while (len && sg_len && sg_list->buf) {
        int chunk = MIN(rem_len_in_sg_item, len);
        if (chunk)  {
            memcpy((unsigned char *)sg_list->buf, src, chunk);
            len -= chunk;
            src += chunk;
            rem_len_in_sg_item -= chunk;

            if (rem_len_in_sg_item == 0) {
                sg_len--;
                sg_list++;
                rem_len_in_sg_item = sg_list->len;
            }
        } else {
            VTSS_ASSERT(FALSE);
            break;
        }
    }
}

static void eth_dummy(struct eth_drv_sc *sc)
{
}

static int eth_can_send(struct eth_drv_sc *sc)
{
    return TRUE;
}


static int eth_int_vector(struct eth_drv_sc *sc)
{
#if defined(CYGNUM_HAL_INT_FDMA)
    return CYGNUM_HAL_INT_FDMA;
#elif defined(CYGNUM_HAL_INTERRUPT_FDMA)
    return CYGNUM_HAL_INTERRUPT_FDMA;
#else
#error Unsupported platform
#endif
}

static struct eth_hwr_funs eth_fun = {
    .start      = eth_start,
    .stop       = eth_stop,
    .control    = eth_ioctl,
    .can_send   = eth_can_send,
    .send       = eth_send,
    .recv       = eth_recv,
    .deliver    = eth_dummy,
    .poll       = eth_dummy,
    .int_vector = eth_int_vector,
    .eth_drv    = &eth_drv_funs,
};

int vtss_ip2_ecos_driver_inject(vtss_vid_t vlan, u32 length, const u8 *const data)
{
    int res;
    priv_t *priv;
    struct eth_drv_sc *sc;

    N("vtss_ip2_ecos_driver_inject vid=%u, length=%u", vlan, length);
    VTSS_ASSERT(vlan < VTSS_VIDS);

    cyg_scheduler_lock();
    if (drivers[vlan] == 0) {
        // no souch interface
        res = -1;
        goto ERR;

    }
    sc = &drivers[vlan]->sc;
    priv = &drivers[vlan]->priv;

    priv->rx_frm = data;
    priv->rx_len = length;
    sc->funs->eth_drv->recv(sc, length); // Frame is injected here
    priv->rx_frm = 0;
    priv->rx_len = 0;
    res = 0;

ERR:
    cyg_scheduler_unlock();
    return res;
}

// returns if_index
int vtss_ip2_ecos_driver_if_add(vtss_vid_t vlan, const vtss_mac_t *mac)
{
    int if_no = -1;
    driver_pair_t *_driver;

    VTSS_ASSERT(vlan >= VLAN_ID_MIN && vlan <= VLAN_ID_MAX);

    cyg_scheduler_lock();

    if (drivers[vlan] != 0) {
        E("Vlan device already exists");
        goto ERR;
    }

    if ((_driver = driver_memory[vlan])) {
        D("Reusing driver memory for vid %d - pointer %p", vlan, _driver);
    } else {
        _driver = driver_memory[vlan] = VTSS_CALLOC(1, sizeof(driver_pair_t));
        if (!_driver) {
            E("Alloc error");
            if_no = -1;
            goto ERR;
        } else {
            D("Allocated driver memory for vid %d - pointer %p", vlan, _driver);
        }
    }

    (void) snprintf(_driver->priv.ifname, sizeof(_driver->priv.ifname),
                    "eth%d", vlan);

    _driver->sc.funs = &eth_fun;
    _driver->sc.dev_name = _driver->priv.ifname;
    _driver->sc.driver_private = &(_driver->priv);

    _driver->priv.vid = vlan;
    _driver->priv.mac = *mac;
    _driver->priv.mclist.len = 0;
    _driver->priv.rx_len = 0;
    _driver->priv.rx_frm = NULL;
    _driver->priv.reject_ioctl = FALSE;

    // Initialize upper layer
    I("if_add: %s", _driver->priv.ifname);
    _driver->sc.funs->eth_drv->init(&(_driver->sc), _driver->priv.mac.addr);
    I("if_add-post: %s if_no=%d", _driver->priv.ifname, if_no);
    if_no = _driver->sc.sc_arpcom.ac_if.if_index;
    _driver->priv.if_index = if_no;
    drivers[vlan] = _driver;
    new_mapping(vlan, if_no);

ERR:
    cyg_scheduler_unlock();
    return if_no;
}

int vtss_ip2_ecos_driver_if_del(vtss_vid_t vid)
{
    int res;
    struct ifnet *ifp;
    struct eth_drv_sc *sc;

    VTSS_ASSERT(vid <= VLAN_ID_MAX);
    I("if_del: if_no=%u", vid);

    cyg_scheduler_lock();
    if (drivers[vid] == 0) {
        E("No such interface %u", vid);
        res = -1;
        goto ERR;

    }

    sc = &(drivers[vid]->sc);
    ifp = &sc->sc_arpcom.ac_if;
    _stop(sc);
    /* Ensure no queued tx are leaked */
    IF_PURGE(&ifp->if_snd);
    /* Detach interface in stack */
    ether_ifdetach(ifp, 0);
    /* NB: We explicitly *don't* free interface memory because of
     * missing cleanup hooks in eCos. A reference to the interface is
     * still kept in the driver_memory[] array, and may be re-used if
     * the vlan interface is added again later. See bz 12264 for
     * reference.
     */
    drivers[vid] = 0;
    res = 0;
    I("if_del-post: if_no=%u", vid);

ERR:
    cyg_scheduler_unlock();
    return res;
}

static bool ip2_driver_fifo_pop(mac_rec_t *msg)
{
    u32 o;
    bool res;

    cyg_scheduler_lock();
    o = mac_subscriptions_fifo_overflow;
    mac_subscriptions_fifo_overflow = 0;
    cyg_scheduler_unlock();

    if (o) {
        E("Fifo overflow: %u", o);
    }

    cyg_scheduler_lock();
    if (FIFO_EMPTY(mac_subscriptions_fifo)) {
        res = FALSE;
    } else {
        res = TRUE;
        *msg = FIFO_HEAD(mac_subscriptions_fifo);
        FIFO_DEL(mac_subscriptions_fifo);
    }
    cyg_scheduler_unlock();

    return res;
}

static BOOL ip2_driver_mac_is_ipmc_ctrl(const vtss_mac_t *const m)
{
    // 01-00-5e-00-00-xx
    return m->addr[0] == 0x01 &&
           m->addr[1] == 0x00 &&
           m->addr[2] == 0x5e &&
           m->addr[3] == 0x00 &&
           m->addr[4] == 0x00;
}

static int IP2_DRIVER_mac_ipmc_cnt = 0;
static void ip2_driver_mac_ipmc_sub(void)
{
    int ref;

    cyg_scheduler_lock();
    ref = IP2_DRIVER_mac_ipmc_cnt++;
    cyg_scheduler_unlock();

    if (ref == 0) {
        vtss_rc rc;
        vtss_packet_rx_conf_t conf;

        I("Subscribe to all 224.0.0.x dips");
        memset(&conf, 0, sizeof(vtss_packet_rx_conf_t));

        vtss_appl_api_lock();
        rc = vtss_packet_rx_conf_get(NULL, &conf);
        if (rc == VTSS_RC_OK) {
            conf.reg.ipmc_ctrl_cpu_copy = TRUE;
            rc = vtss_packet_rx_conf_set(NULL, &conf);
            if (rc != VTSS_RC_OK) {
                E("vtss_packet_rx_conf_set: Failed");
            }
        } else {
            E("vtss_packet_rx_conf_get: Failed");
        }
        vtss_appl_api_unlock();
    }
}

static void ip2_driver_mac_ipmc_unsub(void)
{
    int ref;

    cyg_scheduler_lock();
    ref = --IP2_DRIVER_mac_ipmc_cnt;
    cyg_scheduler_unlock();

    if (ref == 0) {
        vtss_rc rc;
        vtss_packet_rx_conf_t conf;

        I("Unsubscribe from all 224.0.0.x dips");
        memset(&conf, 0, sizeof(vtss_packet_rx_conf_t));

        vtss_appl_api_lock();
        rc = vtss_packet_rx_conf_get(NULL, &conf);
        if (rc == VTSS_RC_OK) {
            conf.reg.ipmc_ctrl_cpu_copy = FALSE;
            rc = vtss_packet_rx_conf_set(NULL, &conf);
            if (rc != VTSS_RC_OK) {
                E("vtss_packet_rx_conf_set: Failed");
            }
        } else {
            E("vtss_packet_rx_conf_get: Failed");
        }
        vtss_appl_api_unlock();
    }
}

inline static void ip2_driver_thread_body(cyg_addrword_t data)
{
    mac_rec_t head;

    while (ip2_driver_fifo_pop(&head)) {
        vtss_vid_t vid = head.vid;
        vtss_mac_t mac = head.mac;

        switch (head.opr) {
        case MAC_ADD:
            if (ip2_driver_mac_is_ipmc_ctrl(&mac)) {
                ip2_driver_mac_ipmc_sub();
                D("Ignoring subscription to VLAN:%u "VTSS_MAC_FORMAT,
                  vid, VTSS_MAC_ARGS(mac));
            } else {
                I("sync subscribe VLAN:%u "VTSS_MAC_FORMAT,
                  vid, VTSS_MAC_ARGS(mac));
                (void) vtss_ip2_chip_mac_subscribe(vid, &mac);
            }

            break;
        case MAC_DEL:
            if (ip2_driver_mac_is_ipmc_ctrl(&mac)) {
                ip2_driver_mac_ipmc_unsub();
                D("Ignoring unsubscription to VLAN:%u "VTSS_MAC_FORMAT,
                  vid, VTSS_MAC_ARGS(mac));
            } else {
                I("unsync subscribe VLAN:%u "VTSS_MAC_FORMAT,
                  vid, VTSS_MAC_ARGS(mac));
                (void) vtss_ip2_chip_mac_unsubscribe(vid, &mac);
            }
            break;
        }
    }
}

static void ip2_driver_thread(cyg_addrword_t data)
{
    while (cyg_flag_wait(&thread_control_flags, WAKEUP,
                         CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR)) {
        ip2_driver_thread_body(data);
    }
}

void vtss_ip2_ecos_driver_init(void)
{
    I("Initiliazing IP2-OS-DRIVER");

    cyg_flag_init(&thread_control_flags);
    cyg_thread_create(THREAD_HIGH_PRIO,
                      ip2_driver_thread,
                      0,
                      "IP2.driver",
                      thread_stack,
                      sizeof(thread_stack),
                      &thread_handle,
                      &thread_block);
    cyg_thread_resume(thread_handle);
}

////////////////////////////////////////////////////////////////////////////

static void _ecos_drv_stat_intf_cntr_convert(vtss_if_status_ip_stat_t *target,
                                             struct if_rfc4293_stats *stat)
{
    vtss_ip_stat_data_t *entry;

    if (!target || !stat) {
        return;
    }

    entry = &target->data;
    entry->InReceives = (u32)((stat->ifi_InReceives << 32) >> 32);
    entry->HCInReceives = stat->ifi_InReceives;
    entry->InOctets = (u32)((stat->ifi_InOctets << 32) >> 32);
    entry->HCInOctets = stat->ifi_InOctets;
    entry->InHdrErrors = stat->ifi_InHdrErrors;
    entry->InNoRoutes = stat->ifi_InNoRoutes;
    entry->InAddrErrors = stat->ifi_InAddrErrors;
    entry->InUnknownProtos = stat->ifi_InUnknownProtos;
    entry->InTruncatedPkts = stat->ifi_InTruncatedPkts;
    entry->InForwDatagrams = (u32)((stat->ifi_InForwDatagrams << 32) >> 32);
    entry->HCInForwDatagrams = stat->ifi_InForwDatagrams;
    entry->ReasmReqds = stat->ifi_ReasmReqds;
    entry->ReasmOKs = stat->ifi_ReasmOKs;
    entry->ReasmFails = stat->ifi_ReasmFails;
    entry->InDiscards = stat->ifi_InDiscards;
    entry->InDelivers = (u32)((stat->ifi_InDelivers << 32) >> 32);
    entry->HCInDelivers = stat->ifi_InDelivers;
    entry->OutRequests = (u32)((stat->ifi_OutRequests << 32) >> 32);
    entry->HCOutRequests = stat->ifi_OutRequests;
    entry->OutForwDatagrams = (u32)((stat->ifi_OutForwDatagrams << 32) >> 32);;
    entry->HCOutForwDatagrams = stat->ifi_OutForwDatagrams;
    entry->OutDiscards = stat->ifi_OutDiscards;
    entry->OutFragReqds = stat->ifi_OutFragReqds;
    entry->OutFragOKs = stat->ifi_OutFragOKs;
    entry->OutFragFails = stat->ifi_OutFragFails;
    entry->OutFragCreates = stat->ifi_OutFragCreates;
    entry->OutTransmits = (u32)((stat->ifi_OutTransmits << 32) >> 32);
    entry->HCOutTransmits = stat->ifi_OutTransmits;
    entry->OutOctets = (u32)((stat->ifi_OutOctets << 32) >> 32);
    entry->HCOutOctets = stat->ifi_OutOctets;
    entry->InMcastPkts = (u32)((stat->ifi_InMcastPkts << 32) >> 32);
    entry->HCInMcastPkts = stat->ifi_InMcastPkts;
    entry->InMcastOctets = (u32)((stat->ifi_InMcastOctets << 32) >> 32);
    entry->HCInMcastOctets = stat->ifi_InMcastOctets;
    entry->OutMcastPkts = (u32)((stat->ifi_OutMcastPkts << 32) >> 32);
    entry->HCOutMcastPkts = stat->ifi_OutMcastPkts;
    entry->OutMcastOctets = (u32)((stat->ifi_OutMcastOctets << 32) >> 32);
    entry->HCOutMcastOctets = stat->ifi_OutMcastOctets;
    entry->InBcastPkts = (u32)((stat->ifi_InBcastPkts << 32) >> 32);
    entry->HCInBcastPkts = stat->ifi_InBcastPkts;
    entry->OutBcastPkts = (u32)((stat->ifi_OutBcastPkts << 32) >> 32);
    entry->HCOutBcastPkts = stat->ifi_OutBcastPkts;

    entry->DiscontinuityTime.sec_msb = 0;
    entry->DiscontinuityTime.seconds = stat->ifi_DiscontinuityTime.tv_sec;
    entry->DiscontinuityTime.nanoseconds = (stat->ifi_DiscontinuityTime.tv_usec * 1000);
    entry->RefreshRate = IP2_STAT_REFRESH_RATE;
}

extern int vtss_ip2_arp_prune_get(void);

static inline
u32 IP2_ECOS_DRIVER_if_status(vtss_if_status_type_t  type,
                              const u32              max,
                              struct ifnet          *ifp,
                              vtss_if_status_t      *status)
{
    u32 cnt = 0;
    struct ifaddr *ifa;
    int arp_prune_size = 0;

    arp_prune_size = vtss_ip2_arp_prune_get();

    TAILQ_FOREACH(ifa, &ifp->if_addrlist, ifa_list) {
        if (cnt >= max) {
            break;
        }

        if (ifa->ifa_addr->sa_family == AF_LINK &&
            (type == VTSS_IF_STATUS_TYPE_ANY ||
             type == VTSS_IF_STATUS_TYPE_LINK)) {
            u_char *ll;
            status->type = VTSS_IF_STATUS_TYPE_LINK;

            if (ifp->if_type == IFT_ETHER) {
                ll = ((struct eth_drv_sc *)ifp->if_softc)->sc_arpcom.ac_enaddr;
            } else {
                ll = (u_char *)LLADDR((struct sockaddr_dl *)ifa->ifa_addr);
            }

            status->u.link.os_if_index = ifp->if_index;
            status->u.link.mtu = ifp->if_mtu;
            memcpy(status->u.link.mac.addr, ll, 6);

#define FLAG(X) \
            if ((ifp->if_flags & IFF_ ##X)) { \
                status->u.link.flags |= VTSS_IF_LINK_FLAG_ ##X; \
            }
            status->u.link.flags = 0;
            FLAG(UP);
            FLAG(BROADCAST);
            FLAG(LOOPBACK);
            FLAG(RUNNING);
            FLAG(NOARP);
            FLAG(PROMISC);
            FLAG(MULTICAST);
#undef FLAG
            cnt ++;
            status ++;

        } else if (ifa->ifa_addr->sa_family == AF_INET &&
                   (type == VTSS_IF_STATUS_TYPE_ANY ||
                    type == VTSS_IF_STATUS_TYPE_IPV4)) {
            struct sockaddr_in *a = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *n = (struct sockaddr_in *)ifa->ifa_netmask;
            struct sockaddr_in *b = (struct sockaddr_in *)ifa->ifa_broadaddr;
            vtss_ipv4_t mask = ntohl(n->sin_addr.s_addr);
            u32 prefix_size = 0;

            (void)vtss_conv_ipv4mask_to_prefix(mask, &prefix_size);
            status->type = VTSS_IF_STATUS_TYPE_IPV4;
            status->u.ipv4.net.address = ntohl(a->sin_addr.s_addr);
            status->u.ipv4.net.prefix_size = prefix_size;
            status->u.ipv4.broadcast = ntohl(b->sin_addr.s_addr);
            status->u.ipv4.reasm_max_size = IP_MAXPACKET;
            status->u.ipv4.arp_retransmit_time = arp_prune_size;

            cnt ++;
            status ++;

        } else if (ifa->ifa_addr->sa_family == AF_INET6 &&
                   (type == VTSS_IF_STATUS_TYPE_ANY ||
                    type == VTSS_IF_STATUS_TYPE_IPV6)) {
            struct sockaddr_in6 *a = (struct sockaddr_in6 *)ifa->ifa_addr;
            struct sockaddr_in6 *n = (struct sockaddr_in6 *)ifa->ifa_netmask;
            vtss_ipv6_t mask;
            u32 prefix_size = 0;
            status->type = VTSS_IF_STATUS_TYPE_IPV6;
            memcpy(mask.addr, &n->sin6_addr, 16);

            (void)vtss_conv_ipv6mask_to_prefix(&mask, &prefix_size);
            memcpy(status->u.ipv6.net.address.addr, &a->sin6_addr, 16);
            status->u.ipv6.net.prefix_size = prefix_size;
            status->u.ipv6.os_if_index = ifp->if_index;

#define FLAG(X) \
            if ((ifp->if_flags & IN6_IFF_ ##X)) { \
                status->u.ipv6.flags |= VTSS_IF_IPV6_FLAG_ ##X; \
            }
            status->u.ipv6.flags = 0;
            FLAG(ANYCAST);
            FLAG(TENTATIVE);
            FLAG(DUPLICATED);
            FLAG(DETACHED);
            FLAG(DEPRECATED);
            FLAG(NODAD);
            FLAG(AUTOCONF);
            FLAG(TEMPORARY);
            FLAG(HOME);
#undef FLAG

            cnt ++;
            status ++;
        } else if (ifa->ifa_addr->sa_family == AF_INET &&
                   (type == VTSS_IF_STATUS_TYPE_ANY ||
                    type == VTSS_IF_STATUS_TYPE_STAT_IPV4)) {
            status->type = VTSS_IF_STATUS_TYPE_STAT_IPV4;
            status->u.ip_stat.IPVersion = VTSS_IP_TYPE_IPV4;
            _ecos_drv_stat_intf_cntr_convert(&status->u.ip_stat, &ifp->if_data.statistics4);

            cnt ++;
            status ++;
        } else if (ifa->ifa_addr->sa_family == AF_INET6 &&
                   (type == VTSS_IF_STATUS_TYPE_ANY ||
                    type == VTSS_IF_STATUS_TYPE_STAT_IPV6)) {
            status->type = VTSS_IF_STATUS_TYPE_STAT_IPV6;
            status->u.ip_stat.IPVersion = VTSS_IP_TYPE_IPV6;
            _ecos_drv_stat_intf_cntr_convert(&status->u.ip_stat, &ifp->if_data.statistics6);

            cnt ++;
            status ++;
        }
    }

    return cnt;
}

vtss_rc vtss_ip2_ecos_driver_if_status(vtss_if_status_type_t   type,
                                       const u32               max,
                                       u32                    *cnt,
                                       vtss_vid_t              vid,
                                       vtss_if_status_t       *status)
{
    u32 i;
    u32 res;
    u32 _cnt = 0;
    struct ifnet *ifp;
    vtss_rc rc = VTSS_RC_OK;

    cyg_scheduler_lock();
    if (drivers[vid] == 0) {
        rc = VTSS_RC_ERROR;
        goto ERR;
    }

    ifp = &(drivers[vid]->sc.sc_arpcom.ac_if);
    res = IP2_ECOS_DRIVER_if_status(type, max - _cnt, ifp, status);

    for (i = 0; i < res; ++i) {
        status->if_id.type = VTSS_ID_IF_TYPE_VLAN;
        status->if_id.u.vlan = vid;

        if (status->type == VTSS_IF_STATUS_TYPE_STAT_IPV4) {
            memcpy(&status->u.ip_stat.IfIndex, &status->if_id, sizeof(vtss_if_id_t));
        }
        if (status->type == VTSS_IF_STATUS_TYPE_STAT_IPV6) {
            memcpy(&status->u.ip_stat.IfIndex, &status->if_id, sizeof(vtss_if_id_t));
        }

        status ++;
        _cnt ++;
    }
    *cnt = _cnt;

ERR:
    cyg_scheduler_unlock();
    return rc;
}

extern struct ifnethead cyg_ifnet;
vtss_rc vtss_ip2_ecos_driver_if_status_all(vtss_if_status_type_t   type,
                                           const u32               max,
                                           u32                    *cnt,
                                           vtss_if_status_t       *status)
{
    u32 res, j;
    u32 _cnt = 0;
    struct ifnet *ifp;
    vtss_rc rc = VTSS_RC_OK;

    cyg_scheduler_lock();
    for (ifp = cyg_ifnet.tqh_first; ifp; ifp = ifp->if_link.tqe_next) {
        if (_cnt >= max) {
            break;
        }

        res = IP2_ECOS_DRIVER_if_status(type, max - _cnt, ifp, status);
        for (j = 0; j < res; ++j) {
            vtss_vid_t vid = vtss_ip2_ecos_driver_if_index_to_vid(ifp->if_index);
            if (vid) {
                status->if_id.type = VTSS_ID_IF_TYPE_VLAN;
                status->if_id.u.vlan = vid;

            } else {
                status->if_id.type = VTSS_ID_IF_TYPE_OS_ONLY;
                status->if_id.u.os.ifno = ifp->if_index;
                strcpy(status->if_id.u.os.name, ifp->if_name);
            }

            if (status->type == VTSS_IF_STATUS_TYPE_STAT_IPV4) {
                memcpy(&status->u.ip_stat.IfIndex, &status->if_id, sizeof(vtss_if_id_t));
            }
            if (status->type == VTSS_IF_STATUS_TYPE_STAT_IPV6) {
                memcpy(&status->u.ip_stat.IfIndex, &status->if_id, sizeof(vtss_if_id_t));
            }

            status ++;
            _cnt ++;
        }
    }
    cyg_scheduler_unlock();

    *cnt = _cnt;
    return rc;
}

typedef struct {
    u32 max;
    u32 cnt;
    vtss_routing_status_t *data;
} __handle_t;

/*lint -sem(_copy_route_entry, thread_protected) */
static int
_copy_route_entry(struct radix_node *rn, void *_handle)
{
    int if_idx;
    __handle_t *handle = (__handle_t *)_handle;
    struct rtentry *rt = (struct rtentry *)rn;
    struct sockaddr *dst, *gate, *netmask;
    vtss_vid_t vid;

    // Warning, this code does not make any sense... Before you change it have a
    // look at _dumpentry in bsd_tcpip/current/src/ecos/support.c and make sure
    // that it does the exactly the same thing.

    dst = rt_key(rt);
    gate = rt->rt_gateway;
    netmask = rt_mask(rt);

#define HANDLE handle->data[handle->cnt]
#define RT HANDLE.rt
    if (handle->cnt >= handle->max) {
        return 0;
    }

    if (!((rt->rt_flags & (RTF_UP | RTF_WASCLONED)) == RTF_UP)) {
        return 0;
    }

    if (dst->sa_family == AF_INET) {
        u32 prefix_size = 0;
        vtss_ipv4_t address = 0;
        struct sockaddr_in *a = (struct sockaddr_in *)dst;
        struct sockaddr_in *n = (struct sockaddr_in *)netmask;
        struct sockaddr_in *g = (struct sockaddr_in *)gate;

        RT.type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;

        // address
        address = ntohl(a->sin_addr.s_addr);
        RT.route.ipv4_uc.network.address = address;

        // netmask
        if (n != NULL) {
            vtss_ipv4_t _netmask = ntohl(n->sin_addr.s_addr);
            (void)vtss_conv_ipv4mask_to_prefix(_netmask, &prefix_size);
        } else {
            prefix_size = 32;
        }
        RT.route.ipv4_uc.network.prefix_size = prefix_size;

        // gateway
        if (g != NULL && g->sin_family == AF_INET) {
            vtss_ipv4_t _gateway = ntohl(g->sin_addr.s_addr);
            RT.route.ipv4_uc.destination = _gateway;
        } else {
            RT.route.ipv4_uc.destination = 0;
        }

        HANDLE.lifetime = -1;
        HANDLE.preference = 0;

    } else if (dst->sa_family == AF_INET6) {
        u32 prefix_size = 0;
        vtss_ipv6_t address;
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)dst;
        struct sockaddr_in6 *n = (struct sockaddr_in6 *)netmask;
        struct sockaddr_in6 *g = (struct sockaddr_in6 *)gate;

        RT.type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;

        // address
        memcpy(address.addr, &(a->sin6_addr), 16);
        RT.route.ipv6_uc.network.address = address;

        // netmask
        if ( n != NULL ) {
            vtss_ipv6_t _netmask;
            memcpy(_netmask.addr, &(n->sin6_addr), 16);
            (void)vtss_conv_ipv6mask_to_prefix(&_netmask, &prefix_size);
        } else {
            prefix_size = 128;
        }

        RT.route.ipv6_uc.network.prefix_size = prefix_size;

        // gateway
        if (g != NULL) {
            vtss_ipv6_t _gateway;
            memcpy(_gateway.addr, &(g->sin6_addr), 16);
            RT.route.ipv6_uc.destination = _gateway;
        } else {
            memset(RT.route.ipv6_uc.destination.addr, 0, 16);
        }

        HANDLE.lifetime = -1;
        HANDLE.preference = 0;
    }

    if (dst->sa_family == AF_INET ||
        dst->sa_family == AF_INET6) {
#define FLAG(X) \
        if ((rt->rt_flags & RTF_  ##X)) { \
            HANDLE.flags |= VTSS_ROUTING_FLAG_ ##X; \
        }
        HANDLE.flags = 0;
        FLAG(UP);
        FLAG(HOST);
        FLAG(GATEWAY);
        FLAG(REJECT);
        FLAG(HW_RT);
#undef FLAG

        if_idx = rt->rt_ifp->if_index;
        vid = vtss_ip2_ecos_driver_if_index_to_vid(if_idx);
        if (vid) {
            HANDLE.interface.type = VTSS_ID_IF_TYPE_VLAN;
            HANDLE.interface.u.vlan = vid;

        } else {
            HANDLE.interface.type = VTSS_ID_IF_TYPE_OS_ONLY;
            HANDLE.interface.u.os.ifno = if_idx;
            strcpy(HANDLE.interface.u.os.name, rt->rt_ifp->if_name);

        }

        handle->cnt ++;
    }
#undef RT
#undef HANDLE
    return 0;
}

extern struct radix_node_head *cyg_rt_tables[AF_MAX + 1];
vtss_rc vtss_ip2_ecos_driver_route_get( vtss_routing_entry_type_t type,
                                        u32                       max,
                                        vtss_routing_status_t     *rt,
                                        u32                       *const cnt)
{
    struct radix_node_head *rnh;
    __handle_t handle;
    int af_type;

    if (type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
        af_type = AF_INET;
    } else if (type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
        af_type = AF_INET6;
    } else {
        return VTSS_INVALID_PARAMETER;
    }

    handle.cnt = 0;
    handle.max = max;
    handle.data = rt;

    cyg_scheduler_lock();
    if ((rnh = cyg_rt_tables[af_type]) != NULL) {
        (void)rnh->rnh_walktree(rnh, _copy_route_entry, &handle);
    }
    cyg_scheduler_unlock();

    *cnt = handle.cnt;

    return VTSS_RC_OK;
}

/*
 * Get or delete an ARP entry.
 * Get initializes the rtmsg and must always be called first!
 * Returns TRUE on success.
 */
static BOOL rtcmd(int s, int cmd, struct sockaddr_inarp *addr, struct m_rtmsg *rtmsg)
{
    static int seq;
    struct rt_msghdr *rtm = &rtmsg->m_rtm;
    char *cp = rtmsg->m_space;
    int len;

    if ((cmd != RTM_GET) && (cmd != RTM_DELETE)) {
        W("wrong cmd (%d)\n", cmd);
        return FALSE;
    }
    if (cmd == RTM_GET) {
        memset((char *)rtmsg, 0, sizeof(*rtmsg));
        rtm->rtm_version = RTM_VERSION;
        rtm->rtm_addrs |= RTA_DST;
        memcpy(cp, (char *)addr, sizeof(*addr));
        /*lint -e{506} Disable 'Constant value Boolean' warning */
        cp += ROUNDUP(sizeof(*addr));
        rtm->rtm_msglen = cp - (char *)rtmsg;
    }

    if (++seq == 0) {
        seq++; /* Avoid zero (used by kernel) */
    }
    rtm->rtm_seq = seq;
    rtm->rtm_type = cmd;
    len = rtm->rtm_msglen;
    if (write(s, (char *)rtmsg, len) < 0) {
        if ((errno != ESRCH) || (cmd == RTM_GET)) {
            W("cannot write to routing socket\n");
            return FALSE;
        }
    }
    do {
        len = read(s, (char *)rtmsg, sizeof(*rtmsg));
    } while (len > 0 && (rtm->rtm_seq != seq));

    if (len < 0) {
        W("cannot read from routing socket\n");
        return FALSE;
    } else {
        return TRUE;
    }
}

static const char *ipv6_neighbour_state_txt(int state)
{
    const char *txt;

    switch ( state ) {
    case ND6_LLINFO_NOSTATE:
        txt = "NONE";
        break;
    case ND6_LLINFO_INCOMPLETE:
        txt = "INCMP";
        break;
    case ND6_LLINFO_REACHABLE:
        txt = "REACH";
        break;
    case ND6_LLINFO_STALE:
        txt = "STALE";
        break;
    case ND6_LLINFO_DELAY:
        txt = "DELAY";
        break;
    case ND6_LLINFO_PROBE:
        txt = "PROBE";
        break;
    default:
        txt = "ERROR";
        break;
    }

    return txt;
}

static void get_nb_entry(struct sockaddr_dl *sdl, struct sockaddr_inarp *addr,
                         struct rt_msghdr *rtm, vtss_neighbour_status_t *status,
                         u32 nbtstamp)
{
    int if_idx;
    vtss_vid_t vid;

    memset(status, 0, sizeof(vtss_neighbour_status_t));

    if (addr->sin_family == AF_INET) {
        status->ip_address.type = VTSS_IP_TYPE_IPV4;
        status->ip_address.addr.ipv4 = ntohl(addr->sin_addr.s_addr);
    } else {
        status->ip_address.type = VTSS_IP_TYPE_NONE;
    }

    if_idx = sdl->sdl_index;
    vid = vtss_ip2_ecos_driver_if_index_to_vid(if_idx);
    if (vid) {
        status->interface.type = VTSS_ID_IF_TYPE_VLAN;
        status->interface.u.vlan = vid;
    } else {
        status->interface.type = VTSS_ID_IF_TYPE_OS_ONLY;
        status->interface.u.os.ifno = if_idx;
        (void) if_indextoname(sdl->sdl_index, status->interface.u.os.name);
    }

    if (status->ip_address.type == VTSS_IP_TYPE_IPV4) {
        if (sdl->sdl_alen) {
            status->flags |= VTSS_NEIGHBOUR_FLAG_VALID;
            memcpy(status->mac_address.addr, (char *)LLADDR(sdl), 6);
        }
        if (!rtm->rtm_rmx.rmx_expire) {
            status->flags |= VTSS_NEIGHBOUR_FLAG_PERMANENT;
        }
    }
    if (rtm->rtm_flags & RTF_HW_NB) {
        status->flags |= VTSS_NEIGHBOUR_FLAG_HARDWARE;
    }
}

static void clear_entry(int s, struct sockaddr_inarp *addr)
{
    struct m_rtmsg rtmsg;
    struct rt_msghdr *rtm = &rtmsg.m_rtm;
    struct sockaddr_inarp *sin2;
    struct sockaddr_dl *sdl;

    while (1) {
        if (!rtcmd(s, RTM_GET, addr, &rtmsg)) {
            W("cannot get %s\n", inet_ntoa(addr->sin_addr));
            return;
        }
        sin2 = (struct sockaddr_inarp *)(rtm + 1);
        sdl = (struct sockaddr_dl *)(ROUNDUP(sin2->sin_len) + (char *)sin2);
        if (sin2->sin_addr.s_addr == addr->sin_addr.s_addr) {
            if ((sdl->sdl_family == AF_LINK) && (sdl->sdl_type == IFT_ETHER) &&
                (rtm->rtm_flags & RTF_LLINFO) && !(rtm->rtm_flags & RTF_GATEWAY)) {

                if (!rtcmd(s, RTM_DELETE, addr, &rtmsg)) {
                    W("cannot delete %s\n", inet_ntoa(addr->sin_addr));
                }
                return;
            }
        }
        if (addr->sin_other & SIN_PROXY) {
            W("cannot locate %s\n", inet_ntoa(addr->sin_addr));
            return;
        } else {
            addr->sin_other = SIN_PROXY;
        }
    }
}

vtss_rc vtss_ip2_ecos_driver_nb_clear_type(int af_type)
{
    int mib[6];
    size_t needed;
    char *lim, *buf, *next;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin2;
    int s = -1;
    vtss_rc rc = VTSS_RC_OK;

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = af_type;
    mib[4] = NET_RT_FLAGS;
    mib[5] = RTF_LLINFO;

    cyg_scheduler_lock();
    s = socket(PF_ROUTE, SOCK_RAW, 0);
    if (s < 0) {
        W("cannot open socket\n");
        rc = VTSS_RC_ERROR;
        goto ERROR;
    }

    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
        W("cannot get estimate\n");
        rc = VTSS_RC_ERROR;
        goto ERROR_SOCKET;
    }

    if ((buf = VTSS_MALLOC(needed)) == NULL) {
        W("malloc\n");
        rc = VTSS_RC_ERROR;
        goto ERROR_SOCKET;
    }

    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
        W("cannot get routing table\n");
        rc = VTSS_RC_ERROR;
        goto ERROR_ALLOC;
    }

    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
        rtm = (struct rt_msghdr *)next;
        sin2 = (struct sockaddr_inarp *)(rtm + 1);
        clear_entry(s, sin2);
    }

ERROR_ALLOC:
    VTSS_FREE(buf);

ERROR_SOCKET:
    if (close(s) != 0) {
        W("cannot close socket\n");
    }

ERROR:
    cyg_scheduler_unlock();
    return rc;
}

BOOL ipv6_if_table_op(CYG_ADDRWORD cmd)
{
    int s_rt6;
    if ((s_rt6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        E("failed to open socket");
        return FALSE;
    } else {
        int i, vid;
        driver_pair_t *d;
        cyg_scheduler_lock();
        for (i = 0; i < (int)IP2_MAX_INTERFACES; ++i) {
            if ((vid = if_id_map[i].vid) != VTSS_VID_NULL &&
                (d = drivers[vid]) != NULL) {
                struct in6_ifreq entry;
                memset(&entry, 0x0, sizeof(struct in6_ifreq));
                strcpy(entry.ifr_name, d->priv.ifname);
                if (ioctl(s_rt6, cmd, &entry) < 0) {
                    D("ioctl 0x%08x: %s", cmd, strerror(errno));
                }
            }
        }
        cyg_scheduler_unlock();
    }
    close(s_rt6);
    return TRUE;
}

vtss_rc vtss_ip2_ecos_driver_nb_clear(vtss_ip_type_t type)
{
    if (type == VTSS_IP_TYPE_IPV4) {
        return vtss_ip2_ecos_driver_nb_clear_type(AF_INET);
    } else if (type == VTSS_IP_TYPE_IPV6) {
        return ipv6_if_table_op(SIOCSNDFLUSH_IN6) ? VTSS_OK : VTSS_UNSPECIFIED_ERROR;
    }
    return VTSS_INVALID_PARAMETER;
}

vtss_rc vtss_ip2_ecos_driver_nb_status_get_ipv4(const u32 max,
                                                u32 *cnt,
                                                vtss_neighbour_status_t *status)
{
    int mib[6];
    size_t needed;
    char *lim, *buf, *next;
    struct rt_msghdr *rtm;
    struct sockaddr_inarp *sin2;
    struct sockaddr_dl *sdl;
    struct timespec curr_time;
    u32     nbtstamp;
    vtss_rc rc = VTSS_RC_OK;

    cyg_scheduler_lock();
    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
    mib[5] = RTF_LLINFO;

    if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
        W("cannot get estimate\n");
        rc = VTSS_RC_ERROR;
        goto ERROR;
    }

    if ((buf = VTSS_MALLOC(needed)) == NULL) {
        W("malloc\n");
        rc = VTSS_RC_ERROR;
        goto ERROR;
    }

    if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
        W("cannot get routing table\n");
        rc = VTSS_RC_ERROR;
        goto ERROR_ALLOC;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &curr_time)) {
        nbtstamp = (cyg_current_time() * ECOS_MSECS_PER_HWTICK) / 1000;
    } else {
        nbtstamp = curr_time.tv_sec;
    }
    lim = buf + needed;
    for (*cnt = 0, next = buf; next < lim && *cnt < max; next +=
             rtm->rtm_msglen, *cnt += 1, status += 1) {
        rtm = (struct rt_msghdr *)next;
        sin2 = (struct sockaddr_inarp *)(rtm + 1);
        sdl = (struct sockaddr_dl *)((char *)sin2 + ROUNDUP(sin2->sin_len));
        get_nb_entry(sdl, sin2, rtm, status, nbtstamp);
    }

ERROR_ALLOC:
    VTSS_FREE(buf);

ERROR:
    cyg_scheduler_unlock();
    return rc;
}

vtss_rc vtss_ip2_ecos_driver_nb_status_get_ipv6(const u32 max,
                                                u32 *cnt,
                                                vtss_neighbour_status_t *status)
{
    int i, s, vid, entries = 0;
    driver_pair_t *d;
    vtss_rc rc = VTSS_OK;

    if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) >= 0) {
        cyg_scheduler_lock();
        for (i = 0; i < (int)IP2_MAX_INTERFACES; ++i) {
            if ((vid = if_id_map[i].vid) != VTSS_VID_NULL &&
                (d = drivers[vid]) != NULL) {
                struct in6_nbrinfo nbi;
                memset(&nbi, 0, sizeof(nbi));
                nbi.next = 1;   /* Getnext */
                strncpy(nbi.ifname, d->priv.ifname, sizeof(nbi.ifname));
                while (entries < max &&
                       (ioctl(s, SIOCGNBRINFO_IN6, (caddr_t)&nbi) >= 0)) {
                    memset(status, 0, sizeof(*status));
                    status->interface.type = VTSS_ID_IF_TYPE_VLAN;
                    status->interface.u.vlan = vid;
                    status->ip_address.type = VTSS_IP_TYPE_IPV6;
                    memcpy(status->ip_address.addr.ipv6.addr, nbi.addr.s6_addr, 16);
                    memcpy(status->mac_address.addr, nbi.ether, sizeof(nbi.ether));
                    strncpy(status->state, ipv6_neighbour_state_txt(nbi.state), sizeof(status->state));
                    if (nbi.state != ND6_LLINFO_NOSTATE && nbi.state != ND6_LLINFO_INCOMPLETE) {
                        status->flags |= VTSS_NEIGHBOUR_FLAG_VALID;
                    }
                    if (!nbi.expire) {
                        status->flags |= VTSS_NEIGHBOUR_FLAG_PERMANENT;
                    }
                    if (nbi.isrouter) {
                        status->flags |= VTSS_NEIGHBOUR_FLAG_ROUTER;
                    }
                    entries++;
                    status++;
                }
            }
        }
        cyg_scheduler_unlock();
        close(s);
        *cnt = entries;
    } else {
        rc = VTSS_RC_ERROR;
    }
    return rc;
}

vtss_rc vtss_ip2_ecos_driver_nb_status_get(vtss_ip_type_t type,
                                           const u32 max,
                                           u32 *cnt,
                                           vtss_neighbour_status_t *status)
{
    if (type == VTSS_IP_TYPE_IPV4) {
        return vtss_ip2_ecos_driver_nb_status_get_ipv4(max, cnt, status);
    } else if (type == VTSS_IP_TYPE_IPV6) {
        return vtss_ip2_ecos_driver_nb_status_get_ipv6(max, cnt, status);
    }
    return VTSS_INVALID_PARAMETER;
}

/* Statistics Section: RFC-4293 */
vtss_rc vtss_ip2_ecos_driver_stat_ipoutnoroute_get(     vtss_ip_type_t          version,
                                                        u32                     *val)
{
    u32 ipoutnoroute;

    if (!val) {
        return VTSS_RC_ERROR;
    }

    ipoutnoroute = 0;
    cyg_scheduler_lock();
    /* ipSystemStatsOutNoRoutes */
    IP_STAT_OUT_NO_ROUTES_GET((version == VTSS_IP_TYPE_IPV4) ? 1 :
                              ((version == VTSS_IP_TYPE_IPV6) ? -1 : 0),
                              ipoutnoroute);
    cyg_scheduler_unlock();
    *val = ipoutnoroute;

    return VTSS_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_syst_cntr_clear(vtss_ip_type_t version)
{
    int             ips_ver;
    u32             idx;
    struct ifnet    *ifp;

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_RC_ERROR;
#endif /* defined(INET) || defined(INET6) */

    cyg_scheduler_lock();
    for (idx = 0; idx < IP2_MAX_INTERFACES; idx++) {
        if (!if_id_map[idx].vid || (drivers[if_id_map[idx].vid] == NULL)) {
            continue;
        }

        ifp = &drivers[if_id_map[idx].vid]->sc.sc_arpcom.ac_if;
        ips_ver = 0;
        switch ( version ) {
        case VTSS_IP_TYPE_IPV4:
            ips_ver = 1;
            break;
        case VTSS_IP_TYPE_IPV6:
            ips_ver = -1;
            break;
        default:
            break;
        }
        IFSTAT_CNTR_CLEAR_VER(ips_ver, ifp);
    }
    /* ipSystemStatsOutNoRoutes */
    IP_STAT_OUT_NO_ROUTES_CLEAR((version == VTSS_IP_TYPE_IPV4) ? 1 :
                                ((version == VTSS_IP_TYPE_IPV6) ? -1 : 0));
    cyg_scheduler_unlock();

    return VTSS_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx)
{
    int             ips_ver;
    struct ifnet    *ifp;
    vtss_os_if_t    *ifid;
    vtss_vid_t      ifvid;

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_RC_ERROR;
#endif /* defined(INET) || defined(INET6) */

    if (!ifidx || (ifidx->type != VTSS_ID_IF_TYPE_OS_ONLY)) {
        D("Invalid %s(%d)!", ifidx ?  "ifidx->type:" : "ifidx", ifidx ? ifidx->type : -1);
        return VTSS_RC_ERROR;
    }

    ifid = &ifidx->u.os;
    ifvid = vtss_ip2_ecos_driver_if_index_to_vid(ifid->ifno);
    cyg_scheduler_lock();
    if (!drivers[ifvid]) {
        cyg_scheduler_unlock();
        D("Invalid drivers[ifvid:%u/ifid->ifno:%d]!", ifvid, ifid->ifno);
        return VTSS_RC_ERROR;
    }

    ifp = &drivers[ifvid]->sc.sc_arpcom.ac_if;
    ips_ver = 0;
    switch ( version ) {
    case VTSS_IP_TYPE_IPV4:
        ips_ver = 1;
        break;
    case VTSS_IP_TYPE_IPV6:
        ips_ver = -1;
        break;
    default:
        break;
    }
    IFSTAT_CNTR_CLEAR_VER(ips_ver, ifp);
    cyg_scheduler_unlock();

    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get(        vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    int                     ver_get;
    vtss_icmp_stat_data_t   *stat;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_INVALID_PARAMETER;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_INVALID_PARAMETER;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_INVALID_PARAMETER;
#endif /* defined(INET) || defined(INET6) */

    ver_get = 0;
    if (version == VTSS_IP_TYPE_IPV4) {
        ver_get = 1;
    } else if (version == VTSS_IP_TYPE_IPV6) {
        ver_get = -1;
    } else {
        D("Incorrect IP version: %d!", version);
        return VTSS_INVALID_PARAMETER;
    }

    entry->IPVersion = version;
    entry->Type = ICMP_STAT_MSG_TYPE_MAX;
    stat = &entry->data;
    memset(stat, 0x0, sizeof(vtss_icmp_stat_data_t));
    cyg_scheduler_lock();
    ICMP_STAT_SYST_GET(ver_get,
                       stat->InMsgs, stat->InErrors,
                       stat->OutMsgs, stat->OutErrors);
    cyg_scheduler_unlock();

    return VTSS_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get_first(  vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    vtss_ip_type_t  nxt_vers;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

#if (defined(INET) && defined(INET6))
    nxt_vers = VTSS_IP_TYPE_IPV4;
#else
#if defined(INET)
    nxt_vers = VTSS_IP_TYPE_IPV4;
#endif /* defined(INET) */
#if defined(INET6)
    nxt_vers = VTSS_IP_TYPE_IPV6;
#endif /* defined(INET6) */
#endif /* defined(INET) && defined(INET6) */

    return vtss_ip2_ecos_driver_stat_icmp_cntr_get(nxt_vers, entry);
}

vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get_next(   vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    vtss_ip_type_t  nxt_vers;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    nxt_vers = version;
    if (nxt_vers < VTSS_IP_TYPE_IPV4) {
        nxt_vers = VTSS_IP_TYPE_IPV4;
    } else {
        if (nxt_vers < VTSS_IP_TYPE_IPV6) {
            nxt_vers = VTSS_IP_TYPE_IPV6;
        } else {
            return VTSS_INVALID_PARAMETER;
        }
    }

    return vtss_ip2_ecos_driver_stat_icmp_cntr_get(nxt_vers, entry);
}

vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_clear(vtss_ip_type_t version)
{
    int ver_set;

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_RC_ERROR;
#endif /* defined(INET) || defined(INET6) */

    ver_set = 0;
    if (version == VTSS_IP_TYPE_IPV4) {
        ver_set = 1;
    } else if (version == VTSS_IP_TYPE_IPV6) {
        ver_set = -1;
    } else {
        D("Incorrect IP version: %d!", version);
        return VTSS_RC_ERROR;
    }

    ICMP_STAT_MSGTYPE_CNTR_CLR(ver_set, ICMP_STAT_MSG_TYPE_ALL);

    return VTSS_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get(        vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    int                     ver_get;
    vtss_icmp_stat_data_t   *stat;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_INVALID_PARAMETER;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_INVALID_PARAMETER;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_INVALID_PARAMETER;
#endif /* defined(INET) || defined(INET6) */

    ver_get = 0;
    if (version == VTSS_IP_TYPE_IPV4) {
        ver_get = 1;
    } else if (version == VTSS_IP_TYPE_IPV6) {
        ver_get = -1;
    } else {
        D("Incorrect IP version: %d!", version);
        return VTSS_INVALID_PARAMETER;
    }
    if (!(type < ICMP_STAT_MSG_TYPE_MAX)) {
        char    buf[IP2_MAX_ICMP_TXT_LEN];

        if (vtss_ip2_stat_icmp_type_txt(buf, IP2_MAX_ICMP_TXT_LEN, version, type)) {
            D("Incorrect ICMP MSG-Type:%s!", buf);
        }

        return VTSS_INVALID_PARAMETER;
    }

    entry->IPVersion = version;
    entry->Type = type;
    stat = &entry->data;
    memset(stat, 0x0, sizeof(vtss_icmp_stat_data_t));
    cyg_scheduler_lock();
    ICMP_STAT_TYPE_GET(ver_get, entry->Type,
                       stat->InMsgs, stat->OutMsgs);
    cyg_scheduler_unlock();

    return VTSS_OK;
}

vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get_first(  vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    vtss_ip_type_t  nxt_vers;
    u32             nxt_type;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

#if (defined(INET) && defined(INET6))
    nxt_vers = VTSS_IP_TYPE_IPV4;
#else
#if defined(INET)
    nxt_vers = VTSS_IP_TYPE_IPV4;
#endif /* defined(INET) */
#if defined(INET6)
    nxt_vers = VTSS_IP_TYPE_IPV6;
#endif /* defined(INET6) */
#endif /* defined(INET) && defined(INET6) */
    nxt_type = 0;

    return vtss_ip2_ecos_driver_stat_imsg_cntr_get(nxt_vers, nxt_type, entry);
}

vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get_next(   vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry)
{
    vtss_ip_type_t  nxt_vers;
    u32             nxt_type;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    /* 0 is a valid message type */
    nxt_vers = version;
    nxt_type = type;
    if (version < VTSS_IP_TYPE_IPV4) {
        version = VTSS_IP_TYPE_IPV4;
        nxt_type = 0;
    } else {
        if (version > VTSS_IP_TYPE_IPV6) {
            return VTSS_INVALID_PARAMETER;
        }

        if ((nxt_type + 1) < ICMP_STAT_MSG_TYPE_MAX) {
            nxt_type++;
        } else {
            if (nxt_vers < VTSS_IP_TYPE_IPV6) {
                nxt_vers = VTSS_IP_TYPE_IPV6;
                nxt_type = 0;
            } else {
                return VTSS_INVALID_PARAMETER;
            }
        }
    }

    return vtss_ip2_ecos_driver_stat_imsg_cntr_get(nxt_vers, nxt_type, entry);
}

vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type)
{
    int ver_set;

#if (defined(INET) || defined(INET6))
#if !defined(INET)
    if (version == VTSS_IP_TYPE_IPV4) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV4!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET) */
#if !defined(INET6)
    if (version == VTSS_IP_TYPE_IPV6) {
        D("Incorrect IP version: VTSS_IP_TYPE_IPV6!");
        return VTSS_RC_ERROR;
    }
#endif /* !defined(INET6) */
#else
    D("Neither INET nor INET6!");
    return VTSS_RC_ERROR;
#endif /* defined(INET) || defined(INET6) */

    ver_set = 0;
    if (version == VTSS_IP_TYPE_IPV4) {
        ver_set = 1;
    } else if (version == VTSS_IP_TYPE_IPV6) {
        ver_set = -1;
    } else {
        D("Incorrect IP version: %d!", version);
        return VTSS_RC_ERROR;
    }

    ICMP_STAT_MSGTYPE_CNTR_CLR(ver_set, (type & 0xFFFF));

    return VTSS_OK;
}

