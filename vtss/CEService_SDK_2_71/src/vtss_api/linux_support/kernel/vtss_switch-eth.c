/*

 Vitesse Switch API software.

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

#include <linux/module.h>  /* can't do without it */
#include <linux/version.h> /* and this too */

#include <linux/io.h>
#include <linux/semaphore.h>

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/ethtool.h>
#include <linux/completion.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/if_vlan.h>
#include <asm/io.h>

#include <asm/unaligned.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "vtss_switch.h"
#include "vtss_switch-netlink.h"

#if !defined(VTSS_FEATURE_FDMA) || VTSS_OPT_FDMA == 0
#error Need to have FDMA enabled!
#endif

#define DBG(x...) do { if(debug) printk(x); } while(0)

#define FCS_SIZE_BYTES 4

#define MAX_STATIC_MAC 128


/*****************************************************************************/
/*****************************************************************************/

static int vlan = ETH_VLAN, debug = FALSE, rxbuffers = 64;

// Intrinsically, the device doesn't have it's own MAC address, so we assign a bogus one.
static const unsigned char bogus_mac_addr[]={0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
static const unsigned char bcast_mac_addr[]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static struct sock *netlsock __read_mostly;

static int already_initialized;

static vtss_fdma_inj_props_t inj_opts  = 
{
#ifdef VTSS_ARCH_JAGUAR_1
    .inj_grp_auto = TRUE,
#endif
    .switch_frm = TRUE,
    .masquerade_port = VTSS_PORT_NO_NONE,
};

typedef struct
{
    struct net_device *dev;
    vtss_fdma_list_t *xmit_desc;
    struct net_device_stats net_stats;
    struct vlan_group *vlgrp;

    struct work_struct netif_work;
    struct sk_buff_head rx_list;

    unsigned char mac_entries[6 * MAX_STATIC_MAC];
    int mac_cnt;

    /* LP Filter */
    int                lpf_flen; /* Filter length */
    struct sock_filter *lpf_fprog; /* Filter instructions */

} linux_fdma_priv_t;

static struct net_device *this_device;

/*****************************************************************************/
/*****************************************************************************/

static void fdma_free_xmit(linux_fdma_priv_t *priv, vtss_fdma_list_t *list)
{
    vtss_fdma_list_t *oldhead;

    oldhead = priv->xmit_desc;
    priv->xmit_desc = list;
    list->next = oldhead;
}

static vtss_fdma_list_t *fdma_alloc_xmit(linux_fdma_priv_t *priv)
{
    vtss_fdma_list_t *oldhead;

    oldhead = priv->xmit_desc;
    if(oldhead) {
        priv->xmit_desc = oldhead->next;
        return oldhead;
    }

    /* Get a fresh */
    return kmalloc(sizeof(vtss_fdma_list_t), GFP_KERNEL);
}

/*****************************************************************************
 * netlink plumming
 *****************************************************************************/

void linux_fdma_inject_done(void *cntxt, 
                            struct tag_vtss_fdma_list *list,
                            vtss_fdma_ch_t ch,
                            BOOL dropped)
{
    linux_fdma_priv_t *priv = netdev_priv(this_device);
    void *frame = list->user;

    kfree(frame);
    fdma_free_xmit(priv, list);

    DBG("oninjectpacket(list %p, skb %p)\n", list, frame);
}

static inline void netlsock_receive_process(struct sk_buff *skb)
{
    linux_fdma_priv_t *priv = netdev_priv(this_device);
    struct nlmsghdr *nlh = nlmsg_hdr(skb);
    vtss_netlink_inject_t *nli = nlmsg_data(nlh);
    int raw_frm_sz_bytes =  nli->length + FCS_SIZE_BYTES;
    vtss_fdma_list_t *l = NULL;
    u8 *tx_buf = NULL;
    
    if((tx_buf = kmalloc(raw_frm_sz_bytes + VTSS_FDMA_HDR_SIZE_BYTES, GFP_KERNEL)) &&
       (l = fdma_alloc_xmit(priv))) {
        vtss_fdma_inj_props_t netinj_opts;

        DBG("tx inject list %p frame %p len %zu\n", l, tx_buf, nli->length);
    
        /* jot down tx buffer */
        l->user = tx_buf;

        /* data and length */
        l->data = tx_buf;
        l->act_len = raw_frm_sz_bytes + VTSS_FDMA_HDR_SIZE_BYTES;

        // copy data - reserve fdma header
#if 1
        memcpy(tx_buf + VTSS_FDMA_HDR_SIZE_BYTES, nli->frame, nli->length);
#else
        skb_copy_bits(skb, ((unsigned char*) nli->frame) - skb->data, 
                      tx_buf + VTSS_FDMA_HDR_SIZE_BYTES, nli->length);
#endif

        // One fragment
        l->next = NULL;

        // Tell it which function to callback when done.
        l->inj_post_cb = linux_fdma_inject_done;

        // Inject props
        memset(&netinj_opts, 0, sizeof(netinj_opts));
        netinj_opts.switch_frm = nli->switch_frm;
        netinj_opts.qos_class = nli->qos_class;
        netinj_opts.vlan = nli->vlan;
        netinj_opts.port_mask = nli->port_mask;
#ifdef vtss_arch_jaguar_1
        netinj_opts.inj_grp_auto = true;
#endif

        // Initiate the injection. this also initializes the IFH and CMD fields.
        CHECK(vtss_fdma_inj(NULL, l, DMACH_TX, raw_frm_sz_bytes, &netinj_opts));
    } else {
        DBG("Dropped frame, no dma desc\n");
        if(tx_buf)
            kfree(tx_buf);
        netlink_ack(skb, nlh, -ENOBUFS);
    }
}

/*****************************************************************************/
/* Flush driver MAC table entries                                            */
/*****************************************************************************/
static void linux_fdma_flush_mactable(struct net_device *dev, unsigned short vid)
{
    linux_fdma_priv_t *priv = netdev_priv(dev);
    if(priv->mac_cnt) {
        vtss_vid_mac_t mac_entry;
        unsigned char *buf;
        int mac_cnt;

        DBG("Flush vlan %d, %d entries\n", vid, priv->mac_cnt);
        memset(&mac_entry, 0, sizeof(mac_entry));
        mac_entry.vid = vid;

        for(mac_cnt = priv->mac_cnt, buf = &priv->mac_entries[0]; mac_cnt > 0; mac_cnt--, buf += 6) {
            /* Delete entry */
            memcpy(mac_entry.mac.addr, buf, 6);
            DBG("Delete, vid: %d, mac: %02x-%02x-%02x-%02x-%02x-%02x\n",
                mac_entry.vid,
                mac_entry.mac.addr[0], mac_entry.mac.addr[1], mac_entry.mac.addr[2],
                mac_entry.mac.addr[3], mac_entry.mac.addr[4], mac_entry.mac.addr[5]);
            vtss_mac_table_del(NULL, &mac_entry);
        }
        priv->mac_cnt = 0;
    }
}

/*****************************************************************************/
/* Add single MAC table entry                                                */
/*****************************************************************************/
static void linux_fdma_add_static_mac(struct net_device *dev, 
                                      unsigned char *mac,  
                                      unsigned short vid)
{
    vtss_mac_table_entry_t mac_entry;

    memset(&mac_entry, 0, sizeof(mac_entry));
    if(mac[0] & 0x1) {
        memset(&mac_entry.destination, 1, sizeof(mac_entry.destination));
    }
    memcpy(mac_entry.vid_mac.mac.addr, mac, 6);
    mac_entry.vid_mac.vid = vid;
    mac_entry.copy_to_cpu = 1;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
    mac_entry.cpu_queue = PACKET_XTR_QU_MGMT_MAC;
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
    mac_entry.locked = 1;
    mac_entry.aged = 0;
    DBG("Add, vid: %d, mac: %02x-%02x-%02x-%02x-%02x-%02x\n",
        mac_entry.vid_mac.vid,
        mac_entry.vid_mac.mac.addr[0], mac_entry.vid_mac.mac.addr[1], mac_entry.vid_mac.mac.addr[2],
        mac_entry.vid_mac.mac.addr[3], mac_entry.vid_mac.mac.addr[4], mac_entry.vid_mac.mac.addr[5]);
    vtss_mac_table_add(NULL, &mac_entry);
}

/*****************************************************************************/
/* Update driver MAC table entries                                           */
/*****************************************************************************/
static void linux_fdma_update_mactable(struct net_device *dev, unsigned short vid)
{
    linux_fdma_priv_t *priv = netdev_priv(dev);
#define ADD_MAC(__a) do { memcpy(buf, __a, 6); linux_fdma_add_static_mac(dev, buf, vid); buf += 6; } while(0)

    /* Start from fresh */
    linux_fdma_flush_mactable(dev, vid);

    /* This many entris (per VLAN) */
    priv->mac_cnt = 2 + dev->mc_count + dev->uc_count;

    DBG("MAC table vlan %d, add %d entries\n", vid, priv->mac_cnt);

    if(priv->mac_cnt < MAX_STATIC_MAC) {
        unsigned char *buf = &priv->mac_entries[0];
        struct dev_addr_list *alist;
        int i;

        /* Device address */
        ADD_MAC(dev->dev_addr);

        /* Broadcast */
        ADD_MAC(bcast_mac_addr);
            
        for(i = 0, alist = dev->mc_list; i < dev->mc_count && alist; i++, alist = alist->next) {
            ADD_MAC(alist->da_addr);
        }
        for(i = 0, alist = dev->uc_list; i < dev->uc_count && alist; i++, alist = alist->next) {
            ADD_MAC(alist->da_addr);
        }
    } else {
        printk(KERN_ERR "Unable to allocate %d entries for MAC table, max is %d", priv->mac_cnt, MAX_STATIC_MAC);
        priv->mac_cnt = 0;  /*  */
    }
}

/*****************************************************************************
 * Update Packet registration
 *****************************************************************************/
static void setup_registration(void)
{
    vtss_packet_rx_conf_t conf;

    // Get Rx packet configuration */
    CHECK(vtss_packet_rx_conf_get(0, &conf));

    // Setup Rx queue mapping */
    conf.map.bpdu_queue      = PACKET_XTR_QU_BPDU;
    conf.map.garp_queue      = PACKET_XTR_QU_BPDU;
    conf.map.learn_queue     = PACKET_XTR_QU_LEARN;
    conf.map.igmp_queue      = PACKET_XTR_QU_IGMP;
    conf.map.ipmc_ctrl_queue = PACKET_XTR_QU_IGMP;
    conf.map.mac_vid_queue   = PACKET_XTR_QU_MAC;
    conf.map.stack_queue     = PACKET_XTR_QU_STACK;
    conf.map.lrn_all_queue   = PACKET_XTR_QU_LRN_ALL;
#ifdef VTSS_SW_OPTION_SFLOW
    conf.map.sflow_queue     = PACKET_XTR_QU_SFLOW;
#else
    // Do not change the sflow_queue.
#endif

    // Setup CPU rx registration
    memset(&conf.reg, FALSE, sizeof(conf.reg));
    conf.reg.bpdu_cpu_only = TRUE;

    // Setup CPU queue sizes
    conf.queue[PACKET_XTR_QU_LOWEST - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    conf.queue[PACKET_XTR_QU_LOWER  - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    conf.queue[PACKET_XTR_QU_LOW    - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    conf.queue[PACKET_XTR_QU_NORMAL - VTSS_PACKET_RX_QUEUE_START].size =  8 * 1024;
    conf.queue[PACKET_XTR_QU_MEDIUM - VTSS_PACKET_RX_QUEUE_START].size = 12 * 1024;
    conf.queue[PACKET_XTR_QU_HIGH   - VTSS_PACKET_RX_QUEUE_START].size = 16 * 1024;

    // Set Rx packet configuration */
    CHECK(vtss_packet_rx_conf_set(0, &conf));
}

/*****************************************************************************/
/*****************************************************************************/
static int linux_fdma_set_mac_address(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;

    /* Set new MAC */
    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    /* Update hw address date */
    linux_fdma_update_mactable(dev, vlan);

    return 0;
}

/*****************************************************************************/
/*****************************************************************************/

static struct sk_buff *fdma_alloc_skb(struct net_device *dev,
                                      unsigned int length,
                                      vtss_fdma_list_t *list)
{
    struct sk_buff *skb = netdev_alloc_skb(dev, length);
    if(skb) {
        DBG("Alloc %d bytes, len = %d, headroom = %zd\n", length, skb->len, skb_headroom(skb));

        // Prepare the FDMA with SKB
        list->data = skb->data; /* Real start of DMA buffer (IFH) */
        list->user = skb;
        list->alloc_len = length;
        list->next = NULL;

    } else {
        if(printk_ratelimit())
            printk(KERN_ERR "FAIL: RX Alloc %d bytes\n", length);
    }
    return skb;
}

static int fdma_netif_receive(struct sk_buff *skb,
                              const vtss_fdma_xtr_props_t *xtrprops)
{
    struct net_device *dev = skb->dev, *vlandev;
    linux_fdma_priv_t *priv = netdev_priv(dev);
    u16 rx_vlan = xtrprops->vid;

    /* If ACL and *no* filter, return frame */
    if(xtrprops->acl_hit && !priv->lpf_flen) {
        DBG("netlink: ACL hit, rule# = %d\n", xtrprops->acl_idx);
        return 0;
    }

    if(priv->lpf_flen > 0) {    /* Process packet filter, possibly trimming frame len */
        int fres = sk_run_filter(skb, priv->lpf_fprog, priv->lpf_flen);
        if(fres > 0) {
            DBG("sk_run_filter ret %d, frame len %d\n", fres, skb->len);
            skb_trim(skb, fres);
            return 0;               /* NETLINK wants this */
        }
    }

    // Update skb->protocol, skb->pkt_type, and skb->mac.raw
    skb->protocol = eth_type_trans(skb, dev);

    /* Look for VLAN tagging present */
    if(skb->protocol == __constant_htons(ETH_P_8021Q) ||
       skb->protocol == __constant_htons(0x88a8)) {
        /* CPU extract does not always strip tags, must do so 'manually' */
	struct vlan_hdr *vhdr = (struct vlan_hdr *)skb->data;
        __be16 proto;
        skb_pull(skb, VLAN_HLEN); /* Strip 802.1Q */
        proto = vhdr->h_vlan_encapsulated_proto;
        if(ntohs(proto) >= 1536)
            skb->protocol = proto;
        else
            skb->protocol = __constant_htons(ETH_P_802_2);
    }

    DBG("%s: rcv %d bytes on vlan '%d' - proto 0x%0x\n", __FUNCTION__, skb->len, 
        xtrprops->was_tagged ? rx_vlan : -1, ntohs(skb->protocol));
    if(rx_vlan != vlan &&
       priv->vlgrp && (vlandev = vlan_group_get_device(priv->vlgrp, rx_vlan))) {
        /* Received on non-default vlan, for which we have a VLAN network interface */
        DBG("%s: vlan rcv on vlan '%d'\n", vlandev->name, rx_vlan);
        vlan_hwaccel_rx(skb, priv->vlgrp, rx_vlan);
    } else {
        dev->last_rx = jiffies;
        priv->net_stats.rx_packets++;
        priv->net_stats.rx_bytes += skb->len;
        DBG("%s: Normal receive\n", dev->name);
        netif_rx(skb);
    }
    
    /* Was consumed by netlink */
    return 1;
}

/* 
 * Netif RX queue
 */
static void netlink_deliver(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = (struct nlmsghdr *) skb->head;

    /* Initliaze NL header */
    nlh->nlmsg_type = 0;
    nlh->nlmsg_len = skb->len;
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_pid = 0;
    nlh->nlmsg_seq = 0;

    /* to mcast group 1<<0 */
    NETLINK_CB(skb).dst_group = 1;
        
    /* multicast the message to all listening processes */
    netlink_broadcast(netlsock, skb, 0, 1, GFP_KERNEL);
}

static void do_netif_work(struct work_struct *work)
{
    linux_fdma_priv_t *priv = container_of(work, linux_fdma_priv_t, netif_work);
    struct sk_buff *skb;

    /*
     * Fish out each received SKB, along with the xtr props decoded
     * after the frame data.
     */
    while ((skb = skb_dequeue(&priv->rx_list)) != NULL)
        netlink_deliver(skb);
}

static vtss_fdma_list_t *
linux_fdma_on_rx_packet(void *cntxt, 
                        vtss_fdma_list_t *list, 
                        vtss_packet_rx_queue_t qu)
{
    struct sk_buff *new_skb, *skb = list->user;
    struct net_device *dev = skb->dev;
    linux_fdma_priv_t *priv = netdev_priv(dev);

    DBG("Rx frame len %zd dat @ %p\n", list->act_len, list->data);

    if(list->next == NULL) { // The frame only takes one entry in the list.
        vtss_fdma_xtr_props_t xtrprops;

        /* Get props - BEFORE fiddling with list */
        if (vtss_fdma_xtr_hdr_decode(NULL, list, &xtrprops) != VTSS_RC_OK) {
            u32 i, len;
            len = list->frm_ptr - list->ifh_ptr;
            printk(KERN_ERR "Failed to decode the following packet:\nIFH (%u bytes):\n", len);
            len = len > 64 ? 64 : len;
            for (i = 0; i < len; i++) {
                printk("%02x ", list->ifh_ptr[i]);
            }
            len = list->act_len - (list->frm_ptr - list->ifh_ptr) - FCS_SIZE_BYTES;
            printk(KERN_ERR "\nFrame (%u bytes):\n", len + FCS_SIZE_BYTES);
            len = len > 64 ? 64 : len;
            for (i = 0; i < len; i++) {
                printk("%02x ", list->frm_ptr[i]);
            }
            return list;
        }

        /* We allocate a new SKB in place of the one received data herein */
        if(!(new_skb = fdma_alloc_skb(dev, list->alloc_len, list))) {
            priv->net_stats.rx_dropped++;
        } else {
            u8 *data = list->frm_ptr;
            u32 frame_length;

            VTSS_ASSERT(skb->len == 0); /* We start with an empty SKB */

            // Set SOF
            skb->tail = skb->data = data;
            // Calc frame len (len - ifh_len - fcs)
            frame_length = list->act_len - (list->frm_ptr - list->ifh_ptr) - FCS_SIZE_BYTES;
            skb_put(skb, frame_length);
                
            /* Normal IP/ICMP on mgmt VLANs ? */
            if(!fdma_netif_receive(skb, &xtrprops)) { /* - else feed to netlink */
                size_t copied = skb->len;             /* May be trimmed */
                u32 gap_align = XTR_PROPS_ALIGN(copied) - copied;
                vtss_fdma_xtr_props_t *props;
                struct nlmsghdr *nlh;
                vtss_netlink_extract_t *xtr;
                
                /* Add Netlink  */
                nlh = (struct nlmsghdr *) skb_push(skb, sizeof(vtss_netlink_extract_t) + NLMSG_HDRLEN);
                xtr = NLMSG_DATA(nlh);
                xtr->length = frame_length;
                xtr->copied = copied;
                
                /* Add vtss_fdma_xtr_props_t aligned after frame data */
                (void) skb_put(skb, gap_align); /* Alignment */
                props = (vtss_fdma_xtr_props_t *) skb_put(skb, sizeof(vtss_fdma_xtr_props_t));
                *props = xtrprops; /* Copy */

                DBG("Netlink: %02x:%02x:%02x:%02x:%02x:%02x, len %d (copied %d), port %d, vid %d\n", 
                    data[0], data[1], data[2], data[3], data[4], data[5],
                    frame_length, copied, props->src_port, props->vid);
                
                /* Delayed delivery */
                skb_queue_tail(&priv->rx_list, skb);
                schedule_work(&priv->netif_work);
            }
        }
    } else {
        vtss_fdma_list_t *l = list;
        int nr_frags = 0;
        size_t totlen = 0;
        while(l) {
            totlen += l->act_len;
            nr_frags++;
            l = l->next;
        }
        if(printk_ratelimit())
            printk(KERN_ERR "%s: XTR: Jumbo Frame not supported (%d frags, len %zu)\n",
                   dev->name, nr_frags, totlen);
    }

    return list;                /* List is re-used, possibly with a new SKB attached */
}

/*****************************************************************************/
/*****************************************************************************/
static int
linux_fdma_load_rx_ring(struct net_device *dev, int ch, int chip)
{
    int i;
    unsigned data_len_bytes;
    vtss_fdma_ch_cfg_t xtr_cfg;
    vtss_fdma_list_t *xtr_list;

    // Allocate the ring structures needed by fdma.c
    xtr_list = kmalloc(rxbuffers*sizeof(vtss_fdma_list_t), GFP_KERNEL);

    // The length of a DCB's associated data area is the MTU + ETHhdr + the IFH size + 
    // the start and end gap sizes, but it cannot exceed FDMA_MAX_DATA_PER_DCB_BYTES.
    // The extra slack caters for VLAN tags etc.
    data_len_bytes = min(dev->mtu + 64 + dev->hard_header_len + (2*sizeof(vtss_fdma_xtr_props_t)) + FCS_SIZE_BYTES, 
                         (unsigned)VTSS_FDMA_MAX_DATA_PER_DCB_BYTES);
    data_len_bytes = max(data_len_bytes, (unsigned)VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES);

    // Chain it together while allocating skbs for the data.
    for(i = 0; i < rxbuffers; i++) {
        struct sk_buff *skb = fdma_alloc_skb(dev, data_len_bytes, &xtr_list[i]);
        if(skb == NULL) {
            // Deallocate it all again
            for(; i >= 0; i--)
                kfree_skb(xtr_list[i].user);
            // Also deallocate the list itself
            kfree(xtr_list);
            return -ENOBUFS;
        }
        xtr_list[i].next = &xtr_list[i+1];
    }

    // The last entry's next pointer must point to NULL
    xtr_list[rxbuffers-1].next = NULL;
    
    // Initialize the FDMA with this list, and tell it to
    // call linux_fdma_on_rx_packet() back when a frame has arrived.
    memset(&xtr_cfg, 0, sizeof(xtr_cfg));
    xtr_cfg.usage = VTSS_FDMA_CH_USAGE_XTR;
    xtr_cfg.chip_no = chip;
    xtr_cfg.xtr_grp = 0;
#if !defined(VTSS_ARCH_SERVAL)
    xtr_cfg.prio    = 7;
#endif  /* !VTSS_ARCH_SERVAL */
    xtr_cfg.xtr_cb = linux_fdma_on_rx_packet;
    xtr_cfg.list = xtr_list;
    CHECK(vtss_fdma_ch_cfg(NULL, ch, &xtr_cfg));

    return 0;
}

void fdma_ccm_init(struct net_device *dev)
{
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD
    int ch;
    for(ch = DMACH_CCM_START; ch <= DMACH_CCM_END; ch++) {
        vtss_fdma_ch_cfg_t ch_cfg;
        // FDMA channel cfg
        memset(&ch_cfg, 0, sizeof(ch_cfg));
        ch_cfg.usage              = VTSS_FDMA_CH_USAGE_CCM;
        ch_cfg.inj_grp_mask       = 1 << 1; /* Grp 1 */
        ch_cfg.prio               = 3;     /* XX */
        ch_cfg.ccm_quotient_max = VTSS_FDMA_CCM_QUOTIENT_MAX;
        if(vtss_fdma_ch_cfg(NULL, ch, &ch_cfg) != VTSS_RC_OK)
            printk("vtss_fdma_ch_cfg(%d): failed\n", ch);
    }
#endif  /* VTSS_OPT_CCM_OFFLOAD */
}

static void fdma_init(struct net_device *dev)
{
    vtss_fdma_ch_cfg_t inj_cfg;

    linux_fdma_update_mactable(dev, vlan);

    memset(&inj_cfg, 0, sizeof(inj_cfg));
    inj_cfg.usage = VTSS_FDMA_CH_USAGE_INJ;
#if defined(VTSS_ARCH_LUTON26)
    inj_cfg.inj_grp_mask = 1 << 0; // Only inject to group 0.
#elif defined(VTSS_ARCH_SERVAL)
    /* don't care */
#elif defined(VTSS_ARCH_JAGUAR_1)
    // Take over all injection groups.
    inj_cfg.inj_grp_mask = (1 << VTSS_PACKET_TX_GRP_CNT) - 1;
#else
 #error "Architecture not supported"
#endif
    inj_cfg.prio         = DMACH_TX; // For now.
    CHECK(vtss_fdma_ch_cfg(NULL, DMACH_TX, &inj_cfg));

    linux_fdma_load_rx_ring(dev, DMACH_RX, 0);
#if defined(CONFIG_VTSS_VCOREIII_JAGUAR_DUAL)
    linux_fdma_load_rx_ring(dev, DMACH_RX_SLV, 1);
#endif

    fdma_ccm_init(dev);

    setup_registration();
}

static void fdma_uninit(struct net_device *dev)
{
    vtss_fdma_ch_cfg_t fdma_cfg;

    linux_fdma_flush_mactable(dev, vlan);

    memset(&fdma_cfg, 0, sizeof(fdma_cfg));
    fdma_cfg.usage = VTSS_FDMA_CH_USAGE_UNUSED;
    CHECK(vtss_fdma_ch_cfg(NULL, DMACH_TX, &fdma_cfg));
    CHECK(vtss_fdma_ch_cfg(NULL, DMACH_RX, &fdma_cfg));
#if defined(CONFIG_VTSS_VCOREIII_JAGUAR_DUAL)
    CHECK(vtss_fdma_ch_cfg(NULL, DMACH_RX_SLV, &fdma_cfg));
#endif
}

/*
 * The set_rx_mode entry point is called whenever the unicast or multicast
 * address lists or the network interface flags are updated. This routine is
 * responsible for configuring the hardware for proper unicast, multicast,
 * promiscuous mode, and all-multi behavior.
 */
static void linux_fdma_set_rx_mode(struct net_device *dev)
{
    linux_fdma_update_mactable(dev, vlan);
}

/*****************************************************************************/
//  linux_fdma_close()
//  User configuring the FDMA down
//  @dev: Device card to shut down
/*****************************************************************************/
static int linux_fdma_close(struct net_device *dev)
{
    DBG("linux_fdma_close()\n");

    netif_stop_queue(dev);

    return 0;
}

/*****************************************************************************/
/*****************************************************************************/
static irqreturn_t linux_fdma_irq_handler(int irq, void *dev_id)
{
#if defined(SLV_FDMA_IRQ)
    /*
     * Make sure vtss_fdma_irq_handler() doesn't get
     * recursively invoked. This can only happen if two
     * different interrupts can cause it to be invoked,
     * which is the case on a dual-Jaguar platform.
     */
    unsigned long flags;
    local_irq_save(flags);
#endif
    vtss_fdma_irq_handler(NULL, dev_id);
#if defined(SLV_FDMA_IRQ)
    local_irq_restore(flags);
#endif
    return IRQ_HANDLED;
}

#if defined(SLV_FDMA_IRQ)
static irqreturn_t linux_fdma_irq_slv_handler(int irq, void *dev_id)
{
    unsigned long flags;
    local_irq_save(flags);
    vtss_fdma_irq_handler(NULL, dev_id);
    local_irq_restore(flags);
    return IRQ_HANDLED;
}
#endif  /* SLV_FDMA_IRQ */

static int eth_request_irq(unsigned int irq, irq_handler_t handler, struct net_device *dev)
{
    int ret = request_irq(irq, handler, 0, dev->name, dev);
    if(ret)        
        printk(KERN_ERR "Cannot assign IRQ number %d\n", irq);
    else
        DBG("Interrupt %d requested\n", irq);
    return ret;
}

/*****************************************************************************/
//  linux_fdma_open()
//  Handle 'up' of card
//  @dev: Device to open
/*****************************************************************************/
static int linux_fdma_open(struct net_device *dev)
{
  DBG("linux_fdma_open()\n");

  netif_start_queue(dev);
  return 0;
}

/*****************************************************************************/
// linux_fdma_timeout()
// Handle a timeout from the network layer.
// @dev: Device that timed out
//
// Handle a timeout on transmit from the dma. This normally means
// bad things.
/*****************************************************************************/
static void linux_fdma_timeout(struct net_device *dev)
{
    if(printk_ratelimit())
        printk(KERN_ERR "%s: transmit timed out?\n", dev->name);
    /* Try to restart the adaptor. */
    netif_wake_queue(dev);
}

void linux_fdma_on_tx_packet(void *cntxt, 
                             vtss_fdma_list_t *list, 
                             vtss_fdma_ch_t ch, 
                             BOOL dropped)
{
    struct sk_buff *skb;
    linux_fdma_priv_t *priv;
    int act_frm_len;
  
    VTSS_ASSERT(list);
    VTSS_ASSERT(list->user);
    DBG("Inject frame done dat @ %p\n", list->data);
    act_frm_len = list->act_len - VTSS_FDMA_HDR_SIZE_BYTES;
    VTSS_ASSERT(act_frm_len >= VTSS_FDMA_MIN_FRAME_SIZE_BYTES);

    skb = list->user;
    priv = netdev_priv(skb->dev);

    if(dropped) {
        priv->net_stats.tx_dropped++;
    } else {
        priv->net_stats.tx_packets++;
        priv->net_stats.tx_bytes += act_frm_len;
    }
    dev_kfree_skb_irq(skb);
    fdma_free_xmit(priv, list);
    DBG("OnTxPacket(#=%lu)\n", priv->net_stats.tx_packets);
}

/*****************************************************************************/
// linux_fdma_hard_start_xmit()
/*****************************************************************************/
static int linux_fdma_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    linux_fdma_priv_t *priv = netdev_priv(dev);
    vtss_fdma_list_t *l;
    int raw_frm_sz_bytes; // The size of the frame without IFH and CMD fields, but including FCS.

    // We do not handle s/g, since we dont have IPv4/IPv6 HW CSUM
    VTSS_ASSERT(skb_shinfo(skb)->nr_frags==0);
    
    if (skb_padto(skb, ETH_ZLEN))
        return NETDEV_TX_OK;

    // l is the buffer we will be loading
    l = fdma_alloc_xmit(priv);
    if(l == NULL) {
        if(printk_ratelimit()) printk(KERN_NOTICE "Dropped frame, no DMA desc\n");
        priv->net_stats.tx_dropped++;
        return NETDEV_TX_BUSY;
    }

    /* The upper layers don't reserve space for the FCS, and the
     * skb->len doesn't include it, but the FDMA includes it, so to
     * find the full frame size, we need to add it.  Since the frame
     * contained in the SKB (excluding the FCS) may end on the last
     * allocated byte, we may inject uninitialized data - or rather -
     * data that belongs to another field within the skb.  But I don't
     * see this as a security risk, since the chip will overwrite the
     * FCS on its way out of it.  ETH_ZLEN is defined as 60 bytes.
     */
    raw_frm_sz_bytes = unlikely(skb->len < ETH_ZLEN) ? ETH_ZLEN : skb->len;
    raw_frm_sz_bytes += FCS_SIZE_BYTES;

    /* We'll need this to free the memory when transmitted, so we save
     * a pointer to the SKB in the FDMA internal list.
     */
    l->user = skb;

    /* The FDMA expects a pointer to the first byte of the IFH, so we
     * reserve space in front of the packet for the IFH and the CMD
     * fields.  The skb_push(skb, len) function subtracts 'len' from
     * skb->data and adds 'len' to skb->len and returns the new
     * skb->data.
     */
    l->data = skb_push(skb, VTSS_FDMA_HDR_SIZE_BYTES);
    l->act_len = raw_frm_sz_bytes + VTSS_FDMA_HDR_SIZE_BYTES;

    // Only one item is used to describe this frame (currently we don't support jumbo frames or multi-fragment frames).
    l->next = NULL;

    // Tell it which port to inject the frame on.
#ifdef VTSS_CPU_PM_NUMBER
    l->phys_port = VTSS_CPU_PM_NUMBER;
#endif

    // Tell it which function to callback when done.
    l->inj_post_cb = linux_fdma_on_tx_packet;

    if (vlan_tx_tag_present(skb))
        inj_opts.vlan = vlan_tx_tag_get(skb) & VTSS_BITMASK(12);
    else
        inj_opts.vlan = vlan;

    DBG("Inject frame vlan %d, length %zd, actlen %zd, skb len %zd, dat @ %p\n", 
        inj_opts.vlan, raw_frm_sz_bytes, l->act_len, skb->len, l->data);

    dev->trans_start = jiffies;

    // Initiate the injection. This also initializes the IFH and CMD fields.
    CHECK(vtss_fdma_inj(NULL, l, DMACH_TX, raw_frm_sz_bytes, &inj_opts));

    return NETDEV_TX_OK;
}

/*****************************************************************************/
// linux_fdma_get_stats()
/*****************************************************************************/
static struct net_device_stats *linux_fdma_get_stats(struct net_device *dev)
{
  linux_fdma_priv_t *priv = netdev_priv(dev);
  DBG("linux_fdma_get_stats()\n");

  if(!priv) {
      printk(KERN_NOTICE "linux_fdma_get_stats() - no device\n");
      return ERR_PTR(-ENODEV);
  }

  return &priv->net_stats;
}  

/*****************************************************************************/
// linux_fdma_do_ioctl()
// Configure FDMA through ioctl calls
// @dev: The device
/*****************************************************************************/
static int linux_fdma_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    linux_fdma_priv_t *priv = netdev_priv(dev);
    int status = 0;

    DBG("linux_fdma_do_ioctl()\n");

    switch(cmd) {
    case SIOCETHDRVSETFILT: {
        struct SIOCETHDRV_filter ioc_flt;
	void __user *arg = ifr->ifr_data;
        if (!arg || 
            copy_from_user(&ioc_flt, arg, sizeof(ioc_flt)) ||
            ioc_flt.length <= 0 || ioc_flt.length > BPF_MAXINSNS) {
            status = -EINVAL;
        } else {
            size_t sz = ioc_flt.length * sizeof(struct sock_filter);
            if(priv->lpf_fprog) {
                kfree(priv->lpf_fprog); /* Cleanup old filter */
            }
            if((priv->lpf_fprog = kmalloc(sz, GFP_KERNEL)) == NULL) {
                printk(KERN_ERR "Unable to allocate %d netfilter ops.\n", ioc_flt.length);
                status = -ENOMEM;
            } else {
                if(copy_from_user(priv->lpf_fprog, ioc_flt.netlink_filter, sz) ||
                   sk_chk_filter(priv->lpf_fprog, ioc_flt.length)) {
                    printk(KERN_ERR "Invalid packet filter, ignored.\n");
                    kfree(priv->lpf_fprog);
                    priv->lpf_fprog = NULL;
                    priv->lpf_flen = 0;
                } else {
                    priv->lpf_flen = ioc_flt.length;
                    printk(KERN_INFO "Extraction filter set, %d ops.\n", ioc_flt.length);
                }
            }
        }
        break;
    }
    case SIOCETHDRVCLRFILT:
        if(priv->lpf_fprog) {
            kfree(priv->lpf_fprog);
            priv->lpf_fprog = NULL;
            priv->lpf_flen = 0;
        }
        printk(KERN_INFO "Extraction filter cleared.\n");
        break;
    default:
        status = -EINVAL;
        break;
    }
    return status;
}

static void linux_fdma_vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
    DBG("%s: kill %d\n", dev->name, vid);
    linux_fdma_flush_mactable(dev, vid);
}

static void linux_fdma_vlan_rx_add_vid(struct net_device *dev, uint16_t vid)
{
    DBG("%s: add %d\n", dev->name, vid);
    linux_fdma_update_mactable(dev, vid);
}

static void
linux_fdma_vlan_rx_register(struct net_device *dev, struct vlan_group *grp)
{
    linux_fdma_priv_t *priv = netdev_priv(dev);
    DBG("device='%s', group='%p'\n", dev->name, grp);
    priv->vlgrp = grp;
}

/*****************************************************************************/
/*****************************************************************************/
static int linux_fdma_change_mtu(struct net_device *dev, int new_mtu)
{
    DBG("linux_fdma_change_mtu(%d => %d)\n", dev->mtu, new_mtu);
    if(dev->mtu == new_mtu)
        return 0;

    if(new_mtu < 68 || new_mtu > VTSS_FDMA_MAX_FRAME_SIZE_BYTES)
        return -EINVAL;

    return 0;
}

/*****************************************************************************/
// linux_fdma_probe()
// Search for FDMA
// @unit: interface number to use
/*****************************************************************************/
struct net_device *__init linux_fdma_probe(void)
{
    struct net_device *dev;
    linux_fdma_priv_t *priv;
    int err;

    // We only support one single instance.
    if(already_initialized)
        return ERR_PTR(-ENODEV);

    /* Allocate std ethernet device */
    if((dev = alloc_etherdev(sizeof(linux_fdma_priv_t))) == NULL)
        return ERR_PTR(-ENOMEM);

    priv = netdev_priv(dev);

    memcpy(dev->dev_addr, bogus_mac_addr, 6);  // Set initial, bogus MAC address

    dev->features           = NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX | NETIF_F_HW_VLAN_FILTER;
    dev->open               = linux_fdma_open;
    dev->stop               = linux_fdma_close;
    dev->hard_start_xmit    = linux_fdma_hard_start_xmit;
    dev->set_rx_mode        = linux_fdma_set_rx_mode;
    dev->set_mac_address    = linux_fdma_set_mac_address;
    dev->get_stats          = linux_fdma_get_stats;
    dev->tx_timeout         = linux_fdma_timeout;
    dev->watchdog_timeo     = HZ;
    dev->do_ioctl           = linux_fdma_do_ioctl;
    dev->change_mtu         = linux_fdma_change_mtu; // In order to support jumbo frames
    dev->hard_header_len    = ETH_HLEN + VTSS_FDMA_HDR_SIZE_BYTES + sizeof(vtss_netlink_extract_t) + NLMSG_HDRLEN;
    dev->vlan_rx_register   = linux_fdma_vlan_rx_register;
    dev->vlan_rx_add_vid    = linux_fdma_vlan_rx_add_vid;
    dev->vlan_rx_kill_vid   = linux_fdma_vlan_rx_kill_vid;

    if((err = register_netdev(dev)) != 0) {
        printk(KERN_ERR "Failed to register net device\n");
        free_netdev(dev);
        return ERR_PTR(err);
    }

    /* Initialize NETIF workqueue */
    INIT_WORK(&priv->netif_work, do_netif_work);
    skb_queue_head_init(&priv->rx_list);

    already_initialized=1;
    return dev;
}

/*****************************************************************************/
// linux_fdma_module_init()
// Entry point. Probe for existense of VTSS FDMA. 
/*****************************************************************************/
int __init linux_fdma_module_init(void)
{
    struct net_device *dev;
    int ret;

    dev = this_device = linux_fdma_probe();
    if(IS_ERR(this_device))
        return PTR_ERR(this_device);

    if ((ret = eth_request_irq(FDMA_IRQ, linux_fdma_irq_handler, dev)))
        goto error;
#if defined(SLV_FDMA_IRQ)
    if ((ret = eth_request_irq(SLV_XTR_RDY0_IRQ, linux_fdma_irq_slv_handler, dev)))
        goto error;
#endif  /* SLV_FDMA_IRQ */

    /* Create netlink interface */
    if((netlsock = netlink_kernel_create(&init_net, NETLINK_VTSS_FRAMEIO, 0,
                                         netlsock_receive_process, NULL, THIS_MODULE)) == NULL) {
        printk(KERN_ERR "Cannot create netlink socket");
        ret = -ENOMEM;
        goto error;
    }

    CHECK(vtss_fdma_init(NULL));
    DBG("FDMA Core initialized\n");

    fdma_init(dev);
    DBG("FDMA Setup done\n");

    if(debug) printk(KERN_NOTICE "Debug: Loaded Frame-DMA network interface on VLAN %d\n", vlan);

    return ret;

error:
    if(this_device)
        free_netdev(this_device);
    this_device = NULL;

    printk(KERN_ERR "Driver load failed: rc = %d\n", ret);

    return ret;
}

/*****************************************************************************/
// linux_fdma_module_exit()
// Free resources for an unload.
/*****************************************************************************/
void __exit linux_fdma_module_exit(void)
{
    struct net_device *dev = this_device;
    linux_fdma_priv_t *priv = netdev_priv(this_device);
    vtss_fdma_list_t *head, *next;

    fdma_uninit(dev);

    // Disable FDMA
    vtss_fdma_uninit(NULL);

    // Let go of the interrupt
    free_irq(FDMA_IRQ, dev);
#if defined(SLV_FDMA_IRQ)
    free_irq(SLV_XTR_RDY0_IRQ, dev);
#endif  /* SLV_FDMA_IRQ */

    for(head = priv->xmit_desc; head != NULL; head = next) {
        next = head->next;
        kfree(head);
    }
    flush_scheduled_work();     /* Flush possible queued NETIF data */
    unregister_netdev(this_device);
    free_netdev(this_device);
    if (netlsock != NULL) {
        netlink_kernel_release(netlsock);
        netlsock = NULL;
    }
    if(priv->lpf_fprog) {
        kfree(priv->lpf_fprog);
        priv->lpf_fprog = NULL;
        priv->lpf_flen = 0;
    }
    already_initialized = 0;
}

module_init(linux_fdma_module_init);
module_exit(linux_fdma_module_exit);

module_param(vlan, int, 0444);
MODULE_PARM_DESC(vlan, "Port VLAN for Ethernet interface");

module_param(rxbuffers, int, 0444);
MODULE_PARM_DESC(rxbuffers, "Receive DMA Buffers per channel");

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug");

MODULE_AUTHOR("Lars Povlsen <lpovlsen@vitesse.com>");
MODULE_DESCRIPTION("Vitesse Gigabit Switch Ethernet Driver");
MODULE_LICENSE("(c) Vitesse Semiconductor Inc.");

/*****************************************************************************/
//
// End of file
//
//****************************************************************************/
