/*

 Vitesse Switch Software.

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
/*
 *
 * Serval1 UIO driver.
 *
 * Copyright (C) 2012 Vitesse Semiconductor Inc.
 * Author: Lars Povlsen (lpovlsen@vitesse.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * VITESSE SEMICONDUCTOR INC SHALL HAVE NO LIABILITY WHATSOEVER OF ANY
 * KIND ARISING OUT OF OR RELATED TO THE PROGRAM OR THE OPEN SOURCE
 * MATERIALS UNDER ANY THEORY OF LIABILITY.
 *
 */

#include <linux/device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/uio_driver.h>
#include <linux/delay.h>

#define DEVICE_NAME "serval1"

#define PCIE_VENDOR_ID 0x101B
#define PCIE_DEVICE_ID 0xB002

struct uio_serval {
    struct uio_info uio;
    /* Private data */
    spinlock_t lock;
    unsigned long flags;
    struct pci_dev *pdev;
};

static void serval1_softreset(struct pci_dev *pdev, const struct uio_info *info)
{
    void __iomem *regaddr_reset = info->mem[0].internal_addr + (0x01070000) + (2 * sizeof(u32));
    void __iomem *regaddr_chipid = info->mem[0].internal_addr + (0x01070000);

    dev_info(&pdev->dev, "Start reset of %s\n", info->name);

    pci_save_state(pdev);

    iowrite32(1, regaddr_reset);
    msleep(100);

    pci_restore_state(pdev);

    ioread32(regaddr_reset);

    dev_info(&pdev->dev, "Reset %s done, id 0x%08x.\n", info->name, ioread32(regaddr_chipid));
}

static ssize_t serval1_sr_store(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf,
                                size_t n)
{
    struct pci_dev *pdev = to_pci_dev(dev);
    struct uio_info *info = pci_get_drvdata(pdev);

    serval1_softreset(pdev, info);
    
    return n;
}

static DEVICE_ATTR(softreset, 0200, NULL, serval1_sr_store);

static irqreturn_t serval1_handler(int irq, struct uio_info *dev_info)
{
    struct uio_serval *priv = dev_info->priv;

    //printk("Serval1: Got IRQ, disabling\n");
        
    if (!test_and_set_bit(0, &priv->flags))
        disable_irq_nosync(irq);

    return IRQ_HANDLED;
}

static int serval1_irqcontrol(struct uio_info *dev_info, s32 irq_on)
{
    struct uio_serval *priv = dev_info->priv;
    unsigned long flags;

    spin_lock_irqsave(&priv->lock, flags);
    if (irq_on) {
        if (test_and_clear_bit(0, &priv->flags)) {
            //printk("Serval1: Enable IRQ\n");
            enable_irq(dev_info->irq);
        }
    } else {
        if (!test_and_set_bit(0, &priv->flags)) {
            //printk("Serval1: Disable IRQ\n");
            disable_irq(dev_info->irq);
        }
    }
    spin_unlock_irqrestore(&priv->lock, flags);

    return 0;
}

static int serval1_pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct uio_serval *priv;
    struct uio_info *info;
    void __iomem *chipid_reg;

    priv = kzalloc(sizeof(struct uio_serval), GFP_KERNEL);
    if (!priv)
        return -ENOMEM;
    info = &priv->uio;
    info->priv = priv;

    if (pci_enable_device(dev))
        goto out_free;

    if (pci_request_regions(dev, DEVICE_NAME))
        goto out_disable;

    /* BAR0 = registers, BAR1 = CONFIG, BAR2 = DDR (unused) */
    info->mem[0].addr = pci_resource_start(dev, 0);
    if (!info->mem[0].addr)
        goto out_release;
    info->mem[0].size = pci_resource_len(dev, 0);
    info->mem[0].memtype = UIO_MEM_PHYS;
    info->mem[0].internal_addr = ioremap(info->mem[0].addr, info->mem[0].size);

#if 1
    info->mem[1].addr = pci_resource_start(dev, 1);
    if (!info->mem[1].addr)
        goto out_release;
    info->mem[1].size = pci_resource_len(dev, 1);
    info->mem[1].memtype = UIO_MEM_PHYS;
#endif

    info->name = "Serval1 RefBoard";
    info->version = "1.0.0";
    info->irq = dev->irq;
    info->handler = serval1_handler;
    info->irqcontrol = serval1_irqcontrol;

    spin_lock_init(&priv->lock);
    priv->flags = 0; /* interrupt is enabled to begin with */
    priv->pdev = dev;

    if (uio_register_device(&dev->dev, info))
        goto out_unmap;

    pci_set_drvdata(dev, info);
    chipid_reg = info->mem[0].internal_addr + (0x01070000);
    dev_info(&dev->dev, "Found %s, UIO device - IRQ %ld, id 0x%08x.\n", info->name, info->irq, ioread32(chipid_reg));

    if(sysfs_create_file(&dev->dev.kobj, &dev_attr_softreset.attr)) {
        dev_info(&dev->dev, "Error creating softreset file\n");
    }

    return 0;

out_unmap:
    iounmap(info->mem[0].internal_addr);
out_release:
    pci_release_regions(dev);
out_disable:
    pci_disable_device(dev);
out_free:
    kfree(info);
    return -ENODEV;
}


static void serval1_pci_remove(struct pci_dev *dev)
{
    struct uio_info *info = pci_get_drvdata(dev);

    sysfs_remove_file(&dev->dev.kobj, &dev_attr_softreset.attr);
    uio_unregister_device(info);
    iounmap(info->mem[0].internal_addr);
    pci_release_regions(dev);
    pci_disable_device(dev);
    pci_set_drvdata(dev, NULL);

    kfree(info->priv);
}

static pci_ers_result_t serval1_error_detected(struct pci_dev *dev,
                                             enum pci_channel_state state)
{
    printk(KERN_ERR DEVICE_NAME ": PCI error\n");
    if (state == pci_channel_io_perm_failure)
        return PCI_ERS_RESULT_DISCONNECT;
    if (state == pci_channel_io_frozen)
        return PCI_ERS_RESULT_NEED_RESET;
    return PCI_ERS_RESULT_CAN_RECOVER;
}

static pci_ers_result_t serval1_link_reset(struct pci_dev *dev)
{
    printk(KERN_INFO DEVICE_NAME ": link reset\n");
    return 0;
}

static pci_ers_result_t serval1_slot_reset(struct pci_dev *dev)
{
    printk(KERN_INFO DEVICE_NAME ": slot reset\n");
    return 0;
}

static void serval1_resume(struct pci_dev *dev)
{
    printk(KERN_INFO DEVICE_NAME ": resume\n");
}

static struct pci_error_handlers serval1_errors = {
    .error_detected = serval1_error_detected,
    .link_reset = serval1_link_reset,
    .slot_reset = serval1_slot_reset,
    .resume = serval1_resume,
};


static struct pci_device_id serval1_pci_ids[] = {
    {
        .vendor =	PCIE_VENDOR_ID,
        .device =	PCIE_DEVICE_ID,
        .subvendor =	PCI_ANY_ID,
        .subdevice =	PCI_ANY_ID,
    },
    { 0, }
};

static struct pci_driver serval1_pci_driver = {
    .name = DEVICE_NAME,
    .id_table = serval1_pci_ids,
    .probe = serval1_pci_probe,
    .remove = serval1_pci_remove,
    .err_handler = &serval1_errors,
};

static int __init serval1_init_module(void)
{
    return pci_register_driver(&serval1_pci_driver);
}

static void __exit serval1_exit_module(void)
{
    pci_unregister_driver(&serval1_pci_driver);
}

module_init(serval1_init_module);
module_exit(serval1_exit_module);

MODULE_DEVICE_TABLE(pci, serval1_pci_ids);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Lars Povlsen <lpovlsen@vitesse.com>");
