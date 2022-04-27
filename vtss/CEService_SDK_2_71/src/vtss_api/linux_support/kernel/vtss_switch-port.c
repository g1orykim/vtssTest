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

#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/ioport.h>

#include <linux/io.h>
#include <linux/semaphore.h>

#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <linux/netlink.h>
#include <net/sock.h>

#include "vtss_switch.h"
#include "vtss_switch-port.h"
#include "vtss_switch-netlink.h"

#define PFX "vtss_port: "

#define TRACE(args...) do {if(debug) printk(args); } while(0)

#define DEBUG
int debug, poll;

extern struct proc_dir_entry *switch_proc_dir;
static struct proc_dir_entry *switch_proc_status;

static void poll_port(struct work_struct *ignored);
static DECLARE_DELAYED_WORK(ppoll_work, poll_port);
static struct work_struct work_q_ext0 __attribute__((unused));
static struct work_struct work_q_ext1 __attribute__((unused));
static struct work_struct work_q_devall __attribute__((unused));

static int irqs_used;

struct {
    int irq;
    void *data;
} irq_hooked[NR_IRQS];

static unsigned long poll_interval;
static vtss_inst_t chipset;
#ifdef VTSS_ARCH_LUTON26
static u32 chip_part, chip_revision;
#endif
static u32 pollmask;

static long
port_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static struct file_operations port_fops = {
owner         : THIS_MODULE,
unlocked_ioctl: port_ioctl,
};

static struct miscdevice port_dev = {
    1,
    "swport",
    &port_fops
};

static struct sock *netlsock;

static struct {
    vtss_port_status_t   status[VTSS_PORT_ARRAY_SIZE];
    port_conf_t          config[VTSS_PORT_ARRAY_SIZE];
    vtss_port_counters_t counters[VTSS_PORT_ARRAY_SIZE];
} port;

static int i2uport(vtss_port_no_t port_no)
{
    return (int) (port_no+1);
}

/* Determine if port has a PHY */
static inline BOOL port_phy(vtss_port_no_t port_no)
{
    return 
        port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_SGMII ||
        port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_QSGMII;
}

/* Determine if port has a PHY - and is on a specific chip */
static inline BOOL port_phy_chip(vtss_port_no_t port_no, vtss_chip_no_t chip_no)
{
    return port_phy(port_no) &&
        port_custom_table[port_no].map.chip_no == chip_no;
}

static BOOL port_internal_phy(vtss_port_no_t port_no)
{
    return port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_SGMII &&
        port_custom_table[port_no].map.miim_controller == VTSS_MIIM_CONTROLLER_0;
}

static BOOL port_external_phy(vtss_port_no_t port_no)
{
    return port_phy(port_no) &&
        port_custom_table[port_no].map.miim_controller == VTSS_MIIM_CONTROLLER_1;
}

static BOOL port_sfp(vtss_port_no_t port_no)
{
    return port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_SERDES;
}

static BOOL port_10g_phy(vtss_port_no_t port_no)
{
#if defined(VTSS_CHIP_10G_PHY)
    return port_custom_table[port_no].mac_if == VTSS_PORT_INTERFACE_XAUI;
#else
    return FALSE;
#endif
}

/* Determine port MAC interface */
static vtss_port_interface_t port_mac_interface(vtss_port_no_t port_no)
{
    return port_custom_table[port_no].mac_if;
}

static void send_port_event(vtss_port_no_t port_no, 
                            const vtss_port_status_t *status)
{
    size_t len = NLMSG_SPACE(sizeof(vtss_netlink_portevent_t));
    struct sk_buff *skb = alloc_skb(len, GFP_KERNEL);
    if (skb) {
        static unsigned seq;
	struct nlmsghdr	*nlh = __nlmsg_put(skb, 0, seq++, NLMSG_DONE, (len - sizeof(*nlh)), 0);
        vtss_netlink_portevent_t *pev = NLMSG_DATA(nlh);
        pev->port_no = port_no;
        pev->status = *status;
        NETLINK_CB(skb).dst_group = 1;
        printk(KERN_DEBUG PFX "port event on port_no: %u (link %d link_down %d)\n", 
               i2uport(port_no), status->link, status->link_down);
        netlink_broadcast(netlsock, skb, 0, 1, GFP_KERNEL);
    }
}

/* Setup port based on configuration and auto negotiation status */
static vtss_rc port_setup(vtss_port_no_t port_no, BOOL config)
{
    vtss_phy_conf_t               phy_setup, *phy;
    vtss_port_conf_t              port_setup, *ps;
    vtss_port_clause_37_control_t control;
    vtss_port_clause_37_adv_t     *adv;
    vtss_rc                       rc = VTSS_RC_OK, rc2;
    port_cap_t                    cap;
    vtss_port_status_t            *status;
    port_custom_conf_t            *conf;
    
    status = &port.status[port_no];
    conf = &port.config[port_no];
    cap = port_custom_table[port_no].cap;

    if (config) {
        /* Configure port */
        if (port_phy(port_no)) {
            phy = &phy_setup;
            phy->mdi = VTSS_PHY_MDIX_AUTO; // always enable auto detection of crossed/non-crossed cables
            if (conf->enable) {
                if ((cap & PORT_CAP_AUTONEG) &&
                    (conf->autoneg || conf->speed == VTSS_SPEED_1G)) {
                    /* Auto negotiation */
                    phy->mode = VTSS_PHY_MODE_ANEG;
                    phy->aneg.speed_10m_hdx = (conf->autoneg && (cap & PORT_CAP_10M_HDX) &&
                                               !(conf->adv_dis & PORT_ADV_DIS_10M_HDX));
                    phy->aneg.speed_10m_fdx = (conf->autoneg && (cap & PORT_CAP_10M_FDX) &&
                                               !(conf->adv_dis & PORT_ADV_DIS_10M_FDX));
                    phy->aneg.speed_100m_hdx = (conf->autoneg && (cap & PORT_CAP_100M_HDX) &&
                                                !(conf->adv_dis & PORT_ADV_DIS_100M_HDX));
                    phy->aneg.speed_100m_fdx = (conf->autoneg && (cap & PORT_CAP_100M_FDX) &&
                                                !(conf->adv_dis & PORT_ADV_DIS_100M_FDX));
                    phy->aneg.speed_1g_fdx = ((cap & PORT_CAP_1G_FDX) &&
                                              !(conf->autoneg &&
                                                (conf->adv_dis & PORT_ADV_DIS_1G_FDX)));
                    phy->aneg.symmetric_pause = conf->flow_control;
                    phy->aneg.asymmetric_pause = conf->flow_control;
                } else {
                    /* Forced mode */
                    phy->mode = VTSS_PHY_MODE_FORCED;
                    phy->forced.speed = conf->speed;
                    phy->forced.fdx = conf->fdx;
                }
            } else {
                /* Power down */
                phy->mode = VTSS_PHY_MODE_POWER_DOWN;
            }
            if ((rc = vtss_phy_conf_set(NULL, port_no, phy)) != VTSS_RC_OK) {
                printk(PFX "vtss_phy_conf_set failed, port_no: %u\n", port_no);
            }
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
            {
                vtss_phy_power_conf_t power;

                power.mode = conf->power_mode;
                if ((rc2 = vtss_phy_power_conf_set(NULL, port_no, &power)) != VTSS_RC_OK) {
                    printk(PFX "vtss_phy_power_conf_set failed, port_no: %u\n", port_no);
                    rc = rc2;
                }
            }
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
        } else if (cap & PORT_CAP_AUTONEG) {
            /* PCS auto negotiation */
            control.enable = conf->autoneg;
            adv = &control.advertisement;
            adv->fdx = 1;
            adv->hdx = 0;
            adv->symmetric_pause = conf->flow_control;
            adv->asymmetric_pause = conf->flow_control;
            adv->remote_fault = (conf->enable ? VTSS_PORT_CLAUSE_37_RF_LINK_OK :
                                 VTSS_PORT_CLAUSE_37_RF_OFFLINE);
            adv->acknowledge = 0;
            adv->next_page = 0;
            if ((rc2 = vtss_port_clause_37_control_set(NULL, port_no, &control)) != VTSS_RC_OK) {
                printk(PFX "vtss_port_clause_37_control_set failed, port_no: %u\n", port_no);
                rc = rc2;
            }
        }
    } 

    /* Port setup */
    ps = &port_setup;
    memset(ps, 0, sizeof(*ps));
    ps->if_type = port_mac_interface(port_no);
    ps->power_down = (conf->enable ? 0 : 1);
    //conf_mgmt_mac_addr_get(ps->flow_control.smac.addr, i2uport(port_no));
    ps->max_frame_length = conf->max_length;
    ps->max_tags = (conf->max_tags == PORT_MAX_TAGS_ONE ? VTSS_PORT_MAX_TAGS_ONE :
                    conf->max_tags == PORT_MAX_TAGS_NONE ? VTSS_PORT_MAX_TAGS_NONE :
                    VTSS_PORT_MAX_TAGS_TWO);
    ps->exc_col_cont = conf->exc_col_cont;
    ps->sd_enable = (cap & PORT_CAP_SD_ENABLE ? 1 : 0);
    ps->sd_active_high = (cap & PORT_CAP_SD_HIGH ? 1 : 0);
    ps->sd_internal = (cap & PORT_CAP_SD_INTERNAL ? 1 : 0);
    ps->xaui_rx_lane_flip = (cap & PORT_CAP_XAUI_LANE_FLIP ? 1 : 0);
    ps->xaui_tx_lane_flip = (cap & PORT_CAP_XAUI_LANE_FLIP ? 1 : 0);
    
    if (conf->autoneg && status->link) {
        /* If autoneg and link up, status values are used */
        ps->speed = status->speed;
        ps->fdx = status->fdx;
        ps->flow_control.obey = status->aneg.obey_pause;
        ps->flow_control.generate = status->aneg.generate_pause;
    } else {
        /* If forced mode or link down, configured values are used */
        ps->speed = (conf->autoneg ? VTSS_SPEED_1G : conf->speed);
        if (ps->if_type == VTSS_PORT_INTERFACE_SERDES) {
            /* Change interface type for 100FX and 2.5G */
            if (ps->speed == VTSS_SPEED_2500M)
                ps->if_type = VTSS_PORT_INTERFACE_VAUI;
            if (ps->speed == VTSS_SPEED_100M)
                ps->if_type = VTSS_PORT_INTERFACE_100FX;
        }
        ps->fdx = conf->fdx;
        ps->flow_control.obey = conf->flow_control;
        ps->flow_control.generate = conf->flow_control;
    }
    
    if ((rc2 = vtss_port_conf_set(NULL, port_no, ps)) != VTSS_RC_OK) {
        printk(PFX "vtss_port_conf_set failed, port_no: %u\n", port_no);
        rc = rc2;
    }

    // Do configuration stuff that are board specific.
    port_custom_conf(port_no,conf, status);

    return rc;
}


static const char *port_mode_txt(vtss_port_speed_t speed, BOOL fdx)
{
    switch (speed) {
    case VTSS_SPEED_10M:
        return (fdx ? "10fdx" : "10hdx");
    case VTSS_SPEED_100M:
        return (fdx ? "100fdx" : "100hdx");
    case VTSS_SPEED_1G:
        return (fdx ? "1Gfdx" : "1Ghdx");
    case VTSS_SPEED_2500M:
        return (fdx ? "2.5Gfdx" : "2.5Ghdx");
    case VTSS_SPEED_5G:
        return (fdx ? "5Gfdx" : "5Ghdx");
    case VTSS_SPEED_10G:
        return (fdx ? "10Gfdx" : "10Ghdx");
    case VTSS_SPEED_12G:
        return (fdx ? "12Gfdx" : "12Ghdx");
    default:
        return "?";
    }
}

static int SWITCH_proc_read_status(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    vtss_port_no_t port_no;
    int bytes_written = 0;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORTS; port_no++) {
        vtss_port_status_t *ps = &port.status[port_no];
        bytes_written += snprintf(buf + bytes_written, count - bytes_written - 1, "%2d %-10s\n", 
                                  i2uport(port_no), 
                                  ps->link ? port_mode_txt(ps->speed, ps->fdx) : "Down");
    }

    return bytes_written;
}

static void poll_port_number(vtss_port_no_t port_no)
{
    vtss_port_status_t old_status, *ps = &port.status[port_no];
    old_status = *ps;
    if (vtss_port_status_get(chipset, port_no, ps) == VTSS_RC_OK) {
        /* Detect link down and disable port */
        if ((!ps->link || ps->link_down) && old_status.link) {
            if(debug)
                printk(KERN_NOTICE PFX "link down event on port_no: %u (link %d link_down %d old_link %d)\n", 
                       i2uport(port_no), ps->link, ps->link_down, old_status.link);
            vtss_port_state_set(chipset, port_no, FALSE);
            vtss_mac_table_port_flush(chipset, port_no);
            old_status.link = FALSE;   /* Force link down to redo setup */
            send_port_event(port_no, ps);
        }

        /* Read port counters */
        (void) vtss_port_counters_get(NULL, port_no, &port.counters[port_no]);

        /* Update port LED */
        port_custom_led_update(port_no, ps, &port.counters[port_no], &port.config[port_no]);

        if (ps->link && !old_status.link) { 
            if(debug)
                printk(KERN_NOTICE PFX "link up event on port_no: %u - speed %s\n", i2uport(port_no), 
                       port_mode_txt(ps->speed, ps->fdx));
            if (port.config[port_no].autoneg)
                port_setup(port_no, FALSE);
            vtss_port_state_set(chipset, port_no, TRUE);
            send_port_event(port_no, ps);
        }
    }
}

static void poll_phy_port_number(vtss_port_no_t port_no)
{
    vtss_phy_event_t phy_events;
    if(vtss_phy_event_poll(NULL, port_no, &phy_events) == VTSS_RC_OK && phy_events) {
        if(debug) printk("phy(%d): events 0x%0x\n", port_no, phy_events);
        poll_port_number (port_no);
    }
}

static void poll_port(struct work_struct *ignored)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if(poll || (pollmask & (1 << port_no)) || port_sfp(port_no) || port_10g_phy(port_no)) {
            poll_port_number (port_no);
        }
    }
    schedule_delayed_work(&ppoll_work, poll_interval);
}

#ifdef EXT0_IRQ
static void work_ext0_jag1(struct work_struct *ignored)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        if(port_phy(port_no))
            poll_phy_port_number(port_no);
    enable_irq(EXT0_IRQ);
}

static void work_ext0_l26(struct work_struct *ignored)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        if(port_external_phy(port_no))
            poll_phy_port_number(port_no);
    enable_irq(EXT0_IRQ);
}

static void work_ext0_serval(struct work_struct *ignored)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        if(port_phy(port_no))
            poll_phy_port_number(port_no);
    enable_irq(EXT0_IRQ);
}
#endif

static void work_ext1_jag1_irq(int irq)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if(port_10g_phy(port_no)) {
#if defined(VTSS_CHIP_10G_PHY)
            vtss_phy_10g_event_t phy_events;
            if(vtss_phy_10g_event_poll(NULL, port_no, &phy_events) == VTSS_RC_OK && phy_events) {
                if(debug) printk("10g(%d): events 0x%0x\n", port_no, phy_events);
                poll_port_number (port_no);
            }
#endif
        }
    }
    enable_irq(irq);
}

#ifdef EXT1_IRQ
static void work_ext1_jag1(struct work_struct *ignored)
{
    work_ext1_jag1_irq(EXT1_IRQ);
}
#endif

#ifdef SLV_EXT1_IRQ
static void work_ext1_jag1d(struct work_struct *ignored)
{
    work_ext1_jag1_irq(SLV_EXT1_IRQ);
}
#endif

#ifdef DEV_ALL_IRQ
static void work_devall(struct work_struct *ignored)
{
    vtss_port_no_t port_no;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if(port_internal_phy(port_no)) {
            poll_phy_port_number(port_no);
#if defined(VTSS_ARCH_LUTON26)
            /* Mitigation for DEV_ALL interrupt problem */
            if(chip_revision > 0) {
                u32 chip_mask = VTSS_BIT(port_custom_table[port_no].map.chip_port);
                vtss_port_status_t *ps = &port.status[port_no];
                /* Invert fastlink polarity */
                if(ps->link)
                    vcoreiii_io_clr(VTSS_ICPU_CFG_INTR_DEV_POL, chip_mask); /* IRQ when *NOT* link */
                else
                    vcoreiii_io_set(VTSS_ICPU_CFG_INTR_DEV_POL, chip_mask); /* IRQ when link */
            }
#endif
        }
    }
    enable_irq(DEV_ALL_IRQ);
}
#endif

static int check_config(vtss_port_no_t port_no, const port_conf_t *conf)
{
    port_cap_t cap = port_custom_table[port_no].cap;
    if(conf->enable) {
        if(!conf->autoneg) {
            vtss_port_speed_t speed = conf->speed;
            if(conf->fdx) {
                if((speed == VTSS_SPEED_10M && !(cap & PORT_CAP_10M_FDX)) ||
                   (speed == VTSS_SPEED_100M && !(cap & PORT_CAP_100M_FDX)) ||
                   (speed == VTSS_SPEED_1G && !(cap & PORT_CAP_1G_FDX)) ||
                   (speed == VTSS_SPEED_2500M && !(cap & PORT_CAP_2_5G_FDX)) ||
                   (speed == VTSS_SPEED_5G && !(cap & PORT_CAP_5G_FDX)) ||
                   (speed == VTSS_SPEED_10G && !(cap & PORT_CAP_10G_FDX)))
                    return 0;
            } else {
                if(speed != VTSS_SPEED_10M && speed != VTSS_SPEED_100M)
                    return 0;
                if((speed == VTSS_SPEED_10M && !(cap & PORT_CAP_10M_HDX)) ||
                   (speed == VTSS_SPEED_100M && !(cap & PORT_CAP_100M_HDX)))
                    return 0;
            }
            if(conf->flow_control && !(cap & PORT_CAP_FLOW_CTRL))
                return 0;
        }
    }
    return 1;
}

static long
port_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

    case POIOC_vtss_port_conf_set:
	{
	    struct _port_conf_ioc ioc, *uioc = (void *) arg;
	    TRACE("Calling %s\n", "port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
                if(ioc.port_no < VTSS_PORT_ARRAY_SIZE) {
                    if(check_config(ioc.port_no, &ioc.conf)) {
                        port.config[ioc.port_no] = ioc.conf;
                        ret = port_setup(ioc.port_no, TRUE);
                    } else {
                        ret = -EINVAL;
                    }
                } else
                    ret = -ENOENT;
		/* No outputs except return value */
	    }
	    TRACE("Done %s - ret %d\n", "port_conf_set", ret);
	}
	break;



    case POIOC_vtss_port_conf_get:
	{
	    struct _port_conf_ioc ioc, *uioc = (void *) arg;
	    TRACE("Calling %s\n", "port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
                if(ioc.port_no < VTSS_PORT_ARRAY_SIZE) {
                    ioc.conf = port.config[ioc.port_no];
                    ret =
                        copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
                } else
                    ret = -ENOENT;
	    }
	    TRACE("Done %s - ret %d\n", "port_conf_get", ret);
	}
	break;

    case POIOC_vtss_port_cap_get:
	{
	    struct _port_cap_ioc ioc, *uioc = (void *) arg;
	    TRACE("Calling %s\n", "port_cap_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
                if(ioc.port_no < VTSS_PORT_ARRAY_SIZE) {
                    ioc.cap = port_custom_table[ioc.port_no].cap;
                    ret =
                        copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
                } else
                    ret = -ENOENT;
	    }
	    TRACE("Done %s - ret %d\n", "port_cap_get", ret);
	}
	break;

    default:
        printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
        ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

/* Set defaults for port */
static void __init port_conf_default(vtss_port_no_t port_no, port_conf_t *conf)
{
    port_cap_t cap;

    cap = port_custom_table[port_no].cap;

    conf->enable = 1;
    conf->autoneg = (cap & PORT_CAP_AUTONEG ? 1 : 0);
    if (cap & PORT_CAP_10G_FDX) {
        conf->speed = VTSS_SPEED_10G;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_5G_FDX) {
        conf->speed = VTSS_SPEED_5G;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_2_5G_FDX) {
        conf->speed = VTSS_SPEED_2500M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_1G_FDX) {
        conf->speed = VTSS_SPEED_1G;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_100M_FDX) {
        conf->speed = VTSS_SPEED_100M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_100M_HDX) {
        conf->speed = VTSS_SPEED_100M;
        conf->fdx = 0;
    } else if (cap & PORT_CAP_10M_FDX) {
        conf->speed = VTSS_SPEED_10M;
        conf->fdx = 1;
    } else if (cap & PORT_CAP_10M_HDX) {
        conf->speed = VTSS_SPEED_10M;
        conf->fdx = 0;
    } else {
        printk(PFX "No speed capabilities on port_no %u, cap: 0x%08x\n", port_no, cap);
        conf->speed = VTSS_SPEED_10M;
        conf->fdx = 1;
    }
    conf->flow_control = 0;
    conf->max_length = VTSS_MAX_FRAME_LENGTH_MAX;
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    conf->power_mode = VTSS_PHY_POWER_NOMINAL;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */
    conf->exc_col_cont = 0;
}

/* Initialize ports */
static void __init port_init_ports(void)
{
    vtss_rc               rc;
    vtss_port_no_t        port_no, port_no_phy = VTSS_PORT_NO_NONE;
    vtss_phy_reset_conf_t phy_reset;
    vtss_port_status_t    *status;
    port_cap_t            cap;

#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t     oper_mode;

    /* Configure the 10G phy operating mode */    
    memset(&oper_mode, 0, sizeof(oper_mode));
    oper_mode.xfi_pol_invert = 1; /* Invert the XFI data polarity */
#endif /* VTSS_CHIP_10G_PHY */

    /* Release ports from reset */
    (void) port_custom_reset();

    /* Initialize all ports */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        cap = port_custom_table[port_no].cap;
        if (port_phy(port_no)) {
            /* Reset PHY */
            phy_reset.mac_if = port_mac_interface(port_no);
            if (cap & PORT_CAP_COPPER)
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_CU;
            else if (cap & PORT_CAP_FIBER)
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_FI_1000BX;
            else if (cap & PORT_CAP_DUAL_COPPER)
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_CU_1000BX;
            else if (cap & PORT_CAP_DUAL_FIBER)
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_FI_1000BX;
            else if (cap & PORT_CAP_DUAL_FIBER_100FX)
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_AMS_FI_100FX;
            else
                printk(PFX "unknown media interface on port_no %u\n", port_no);
            phy_reset.i_cpu_en = false;
            if ((rc = vtss_phy_reset(NULL, port_no, &phy_reset)) == VTSS_RC_OK) {
                if (port_no_phy == VTSS_PORT_NO_NONE)
                    port_no_phy = port_no;
            } else {
                printk(PFX "vtss_phy_reset failed, port_no: %u\n", port_no);
            }
        } else if (port_10g_phy(port_no)) {
#if defined(VTSS_CHIP_10G_PHY)
            if (cap & PORT_CAP_VTSS_10G_PHY) {
                if (vtss_phy_10g_mode_set(0, port_no, &oper_mode) != VTSS_RC_OK) {
                    printk(PFX "Could not set the 10g Phy operating mode (port %u)\n", port_no);
                }                                
            }
#endif /* VTSS_CHIP_10G_PHY */
        }

        /* Set port status down */
        status = &port.status[port_no];
        status->link_down = 0;
        status->link = 0;

        /* Setup port with default configuration */
        port_conf_default(port_no, &port.config[port_no]);
        port_setup(port_no, TRUE);
    } /* Port loop */

    // Do post reset
    (void) post_port_custom_reset();
    
    /* Initialize port LEDs */
    (void) port_custom_led_init();
}

irqreturn_t port_irq_handler(int irq, void *data)
{
    /* Queue the bh. Don't worry about multiple enqueueing */
    disable_irq(irq);
    schedule_work(data);
    return IRQ_HANDLED;
}

static int port_request_irq(unsigned int irq, void *data)
{
    int ret = request_irq(irq, port_irq_handler, 0, "vtss_port", data);
    if(ret)
        printk(KERN_ERR PFX "Cannot assign IRQ number %d\n", irq);
    else {
        irq_hooked[irqs_used].irq = irq;
        irq_hooked[irqs_used].data = data;
        irqs_used++;
    }
    return ret;
}

static int __init vtss_switch_init(void)
{
    vtss_port_no_t     port_no;
    vtss_port_map_t    port_map[VTSS_PORT_ARRAY_SIZE];
    const char         *board_name = vtss_board_name();
    int		       board_type = vtss_board_type();
#ifdef VTSS_ARCH_LUTON26
    u32                chip_id = readl(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID);
    chip_part = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(chip_id);
    chip_revision = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(chip_id);
#endif

    printk(KERN_DEBUG PFX "Loaded port module on board %s, type %d\n", board_name, board_type);

    if((netlsock = netlink_kernel_create(&init_net, NETLINK_VTSS_PORTEVENT, 1,
                                         NULL, NULL, THIS_MODULE)) == NULL) {
        printk(KERN_ERR PFX "Cannot create netlink socket");
        return -ENOMEM;
    }

    /* Port custom initialization */
    port_custom_init();

    /* Setup port map for board */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        port_map[port_no] = port_custom_table[port_no].map;

    if(vtss_port_map_set(chipset, port_map) != VTSS_RC_OK) {
        printk(KERN_ERR PFX "Port map setting failed\n");
        return -ENXIO;
    }

    /* State/Config init */
    memset(&port, 0, sizeof(port));

    /* Slow poll needy ports each 1 second */
    poll_interval = msecs_to_jiffies(1000);

    /* Initial port setup */
    port_init_ports();

    switch_proc_status = NULL;

    if(switch_proc_dir) {
        if((switch_proc_status = create_proc_entry("status", S_IFREG | S_IRUGO, switch_proc_dir)))
            switch_proc_status->read_proc = SWITCH_proc_read_status;
    }

    /* Enable IRQ on PHY's */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if(port_10g_phy(port_no)) {
#if defined(VTSS_CHIP_10G_PHY)
            vtss_phy_10g_event_t phy10g_events = VTSS_PHY_10G_LINK_LOS_EV;
            (void) vtss_phy_10g_event_enable_set(NULL, port_no, phy10g_events, TRUE);
#endif
        } else if(port_phy(port_no)) {
            vtss_phy_event_t phy_events = VTSS_PHY_LINK_LOS_EV | VTSS_PHY_LINK_FFAIL_EV | VTSS_PHY_LINK_AMS_EV;
            (void) vtss_phy_event_enable_set(NULL, port_no, phy_events, TRUE);
        }
    }

    /* EXT0 */
#ifdef EXT0_IRQ
    if(board_type == VTSS_BOARD_JAG_CU24_REF || 
       board_type == VTSS_BOARD_JAG_SFP24_REF ||
       board_type == VTSS_BOARD_JAG_CU48_REF) {
        (void) vtss_gpio_mode_set(NULL, 0, 6, VTSS_GPIO_ALT_0);
        INIT_WORK(&work_q_ext0, work_ext0_jag1);
        port_request_irq(EXT0_IRQ, &work_q_ext0);
    } else if(board_type == VTSS_BOARD_LUTON26_REF) {
        (void) vtss_gpio_mode_set(NULL, 0, 8, VTSS_GPIO_ALT_0);
        INIT_WORK(&work_q_ext0, work_ext0_l26);
        port_request_irq(EXT0_IRQ, &work_q_ext0);
    } else if(board_type == VTSS_BOARD_SERVAL_REF) {
        pollmask |= (1 << 10);  /* NPI port must be polled */
        (void) vtss_gpio_mode_set(NULL, 0, 28, VTSS_GPIO_ALT_0);
        INIT_WORK(&work_q_ext0, work_ext0_serval);
        port_request_irq(EXT0_IRQ, &work_q_ext0);
    }
#endif  /* EXT0_IRQ */

    /* EXT1 */
#ifdef EXT1_IRQ
    if(board_type == VTSS_BOARD_JAG_CU24_REF || board_type == VTSS_BOARD_JAG_SFP24_REF) {
        (void) vtss_gpio_mode_set(NULL, 0, 7, VTSS_GPIO_ALT_0);
        INIT_WORK(&work_q_ext1, work_ext1_jag1);
        port_request_irq(EXT1_IRQ, &work_q_ext1);
    }
#endif /* EXT1_IRQ */
    
    /* SLV_EXT1_IRQ */
#ifdef SLV_EXT1_IRQ
    if(board_type == VTSS_BOARD_JAG_CU48_REF) {
        (void) vtss_gpio_mode_set(NULL, 1, 7, VTSS_GPIO_ALT_0); /* EXT1 is alternate GPIO7 */
        INIT_WORK(&work_q_ext1, work_ext1_jag1d);
        port_request_irq(SLV_EXT1_IRQ, &work_q_ext1);
    }
#endif

    /* DEV_ALL */
#ifdef DEV_ALL_IRQ
    if(board_type == VTSS_BOARD_LUTON10_REF || board_type == VTSS_BOARD_LUTON26_REF) {
        // Setup interrupt from PHY.
#if defined(VTSS_ARCH_LUTON26)
        (void) vtss_intr_cfg(NULL, VTSS_BIT(28), TRUE, TRUE); /* DEV_ALL */
        if(chip_revision > 0) {
            /* Workaround for REV B - DEV_ALL defunct */
            writel(0x000, VTSS_ICPU_CFG_INTR_DEV_ENA); /* Disable all */
            writel(0xfff, VTSS_ICPU_CFG_INTR_DEV_POL); /* IRQ when LINK */
            writel(0xfff, VTSS_ICPU_CFG_INTR_DEV_ENA); /* Enable 0-11: Internal PHYs */
        }
#endif
        INIT_WORK(&work_q_devall, work_devall);
        port_request_irq(DEV_ALL_IRQ, &work_q_devall);
    } else if(board_type == VTSS_BOARD_JAG_CU48_REF) {
        INIT_WORK(&work_q_devall, work_devall);
        port_request_irq(DEV_ALL_IRQ, &work_q_devall);
    }
#endif  /* DEV_ALL_IRQ */

    while(misc_register(&port_dev)) {
        if(port_dev.minor > 16) {
            printk(KERN_ERR PFX "Can't misc_register on minor 1-%d\n", port_dev.minor);
            break;
        }
        port_dev.minor++;
    }

#if !defined(CONFIG_VTSS_VCOREIII)
    poll = 1;                   /* Periodic polling, no well-defined IRQ */
#endif

    /* Must poll SFP+10G ports */
    schedule_delayed_work(&ppoll_work, poll_interval);

    return 0;
}

static void vtss_switch_exit(void)
{
    int i;
    cancel_delayed_work_sync(&ppoll_work);

    for(i = 0; i < irqs_used; i++)
        (void) free_irq(irq_hooked[i].irq, irq_hooked[i].data);

    if(switch_proc_status) {
        remove_proc_entry(switch_proc_status->name, switch_proc_status->parent);
        switch_proc_status = NULL;
    }

    netlink_kernel_release(netlsock);

    misc_deregister(&port_dev);
 
    printk(KERN_NOTICE PFX "Uninstalled\n");
}

module_init(vtss_switch_init);
module_exit(vtss_switch_exit);

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug");

module_param(poll, int, 0644);
MODULE_PARM_DESC(poll, "Enable polling of all ports");

MODULE_AUTHOR("Lars Povlsen <lpovlsen@vitesse.com>");
MODULE_DESCRIPTION("Vitesse Gigabit Switch Port Module");
MODULE_LICENSE("(c) Vitesse Semiconductor Inc.");
