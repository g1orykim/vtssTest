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


#include <linux/module.h>	/* can't do without it */
#include <linux/version.h>	/* and this too */

#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "vtss_switch.h"

int debug = false;

#define DEBUG(args...) do {if(debug) printk(args); } while(0)

/* Internal glue */
extern void vtss_ioctl_ext_init(void);
extern void vtss_ioctl_ext_exit(void);

static long
switch_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static struct file_operations switch_fops = {
  owner:THIS_MODULE,
  unlocked_ioctl:switch_ioctl,
};

static struct miscdevice switch_dev = {
    0,
    "switch",
    &switch_fops
};

static vtss_inst_t chipset;

 /************************************
  * port group 
  */

static long
switch_ioctl_port(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {



    case SWIOC_vtss_port_map_set:
	{
	    struct vtss_port_map_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_map_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_map_set(chipset, ioc.port_map);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_map_set", ret);
	}
	break;



    case SWIOC_vtss_port_map_get:
	{
	    struct vtss_port_map_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_map_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_port_map_get(chipset, ioc.port_map);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_map_get", ret);
	}
	break;


#ifdef VTSS_FEATURE_10G
    case SWIOC_vtss_port_mmd_read:
	{
	    struct vtss_port_mmd_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_mmd_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_mmd_read(chipset, ioc.port_no, ioc.mmd,
				       ioc.addr, &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_mmd_read", ret);
	}
	break;
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
    case SWIOC_vtss_port_mmd_write:
	{
	    struct vtss_port_mmd_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_mmd_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_mmd_write(chipset, ioc.port_no, ioc.mmd,
					ioc.addr, ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_mmd_write", ret);
	}
	break;
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
    case SWIOC_vtss_port_mmd_masked_write:
	{
	    struct vtss_port_mmd_masked_write_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_mmd_masked_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_mmd_masked_write(chipset, ioc.port_no,
					       ioc.mmd, ioc.addr,
					       ioc.value, ioc.mask);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_mmd_masked_write", ret);
	}
	break;
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
    case SWIOC_vtss_mmd_read:
	{
	    struct vtss_mmd_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mmd_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mmd_read(chipset, ioc.chip_no,
				  ioc.miim_controller, ioc.miim_addr,
				  ioc.mmd, ioc.addr, &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mmd_read", ret);
	}
	break;
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
    case SWIOC_vtss_mmd_write:
	{
	    struct vtss_mmd_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mmd_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mmd_write(chipset, ioc.chip_no,
				   ioc.miim_controller, ioc.miim_addr,
				   ioc.mmd, ioc.addr, ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mmd_write", ret);
	}
	break;
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_CLAUSE_37
    case SWIOC_vtss_port_clause_37_control_get:
	{
	    struct vtss_port_clause_37_control_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_clause_37_control_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_clause_37_control_get(chipset, ioc.port_no,
						    &ioc.control);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_clause_37_control_get",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_CLAUSE_37 */

#ifdef VTSS_FEATURE_CLAUSE_37
    case SWIOC_vtss_port_clause_37_control_set:
	{
	    struct vtss_port_clause_37_control_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_clause_37_control_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_clause_37_control_set(chipset, ioc.port_no,
						    &ioc.control);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_clause_37_control_set",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_CLAUSE_37 */


    case SWIOC_vtss_port_conf_set:
	{
	    struct vtss_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_conf_set(chipset, ioc.port_no, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_conf_set", ret);
	}
	break;



    case SWIOC_vtss_port_conf_get:
	{
	    struct vtss_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_conf_get(chipset, ioc.port_no, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_conf_get", ret);
	}
	break;



    case SWIOC_vtss_port_status_get:
	{
	    struct vtss_port_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_status_get(chipset, ioc.port_no,
					 &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_status_get", ret);
	}
	break;



    case SWIOC_vtss_port_counters_update:
	{
	    struct vtss_port_counters_update_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_counters_update");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_counters_update(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_counters_update", ret);
	}
	break;



    case SWIOC_vtss_port_counters_clear:
	{
	    struct vtss_port_counters_clear_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_counters_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_counters_clear(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_counters_clear", ret);
	}
	break;



    case SWIOC_vtss_port_counters_get:
	{
	    struct vtss_port_counters_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_counters_get(chipset, ioc.port_no,
					   &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_counters_get", ret);
	}
	break;



    case SWIOC_vtss_port_basic_counters_get:
	{
	    struct vtss_port_basic_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_basic_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_basic_counters_get(chipset, ioc.port_no,
						 &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_basic_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_port_forward_state_get:
	{
	    struct vtss_port_forward_state_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_forward_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_forward_state_get(chipset, ioc.port_no,
						&ioc.forward);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_forward_state_get",
		  ret);
	}
	break;



    case SWIOC_vtss_port_forward_state_set:
	{
	    struct vtss_port_forward_state_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_forward_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_forward_state_set(chipset, ioc.port_no,
						ioc.forward);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_forward_state_set",
		  ret);
	}
	break;



    case SWIOC_vtss_miim_read:
	{
	    struct vtss_miim_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_miim_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_miim_read(chipset, ioc.chip_no,
				   ioc.miim_controller, ioc.miim_addr,
				   ioc.addr, &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_miim_read", ret);
	}
	break;



    case SWIOC_vtss_miim_write:
	{
	    struct vtss_miim_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_miim_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_miim_write(chipset, ioc.chip_no,
				    ioc.miim_controller, ioc.miim_addr,
				    ioc.addr, ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_miim_write", ret);
	}
	break;



    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * misc group 
  */

static long
switch_ioctl_misc(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {



    case SWIOC_vtss_debug_info_print:
	{
	    struct vtss_debug_info_print_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_debug_info_print");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_debug_info_print(chipset,
					  (vtss_debug_printf_t) printk,
					  &ioc.info);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_debug_info_print", ret);
	}
	break;



    case SWIOC_vtss_reg_read:
	{
	    struct vtss_reg_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_reg_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_reg_read(chipset, ioc.chip_no, ioc.addr,
				  &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_reg_read", ret);
	}
	break;



    case SWIOC_vtss_reg_write:
	{
	    struct vtss_reg_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_reg_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_reg_write(chipset, ioc.chip_no, ioc.addr,
				   ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_reg_write", ret);
	}
	break;



    case SWIOC_vtss_reg_write_masked:
	{
	    struct vtss_reg_write_masked_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_reg_write_masked");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_reg_write_masked(chipset, ioc.chip_no, ioc.addr,
					  ioc.value, ioc.mask);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_reg_write_masked", ret);
	}
	break;



    case SWIOC_vtss_chip_id_get:
	{
	    struct vtss_chip_id_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_chip_id_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_chip_id_get(chipset, &ioc.chip_id);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_chip_id_get", ret);
	}
	break;



    case SWIOC_vtss_poll_1sec:
	{
	    DEBUG("Calling %s\n", "vtss_poll_1sec");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_poll_1sec(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_poll_1sec", ret);
	}
	break;



    case SWIOC_vtss_ptp_event_poll:
	{
	    struct vtss_ptp_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ptp_event_poll");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ptp_event_poll(chipset, &ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ptp_event_poll", ret);
	}
	break;



    case SWIOC_vtss_ptp_event_enable:
	{
	    struct vtss_ptp_event_enable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ptp_event_enable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ptp_event_enable(chipset, ioc.ev_mask,
					  ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ptp_event_enable", ret);
	}
	break;



    case SWIOC_vtss_dev_all_event_poll:
	{
	    struct vtss_dev_all_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_dev_all_event_poll");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_dev_all_event_poll(chipset, ioc.poll_type,
					    &ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_dev_all_event_poll", ret);
	}
	break;



    case SWIOC_vtss_dev_all_event_enable:
	{
	    struct vtss_dev_all_event_enable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_dev_all_event_enable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_dev_all_event_enable(chipset, ioc.port,
					      ioc.ev_mask, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_dev_all_event_enable", ret);
	}
	break;



    case SWIOC_vtss_gpio_mode_set:
	{
	    struct vtss_gpio_mode_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_mode_set(chipset, ioc.chip_no, ioc.gpio_no,
				       ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_mode_set", ret);
	}
	break;



    case SWIOC_vtss_gpio_direction_set:
	{
	    struct vtss_gpio_direction_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_direction_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_direction_set(chipset, ioc.chip_no,
					    ioc.gpio_no, ioc.output);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_direction_set", ret);
	}
	break;



    case SWIOC_vtss_gpio_read:
	{
	    struct vtss_gpio_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_read(chipset, ioc.chip_no, ioc.gpio_no,
				   &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_read", ret);
	}
	break;



    case SWIOC_vtss_gpio_write:
	{
	    struct vtss_gpio_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_write(chipset, ioc.chip_no, ioc.gpio_no,
				    ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_write", ret);
	}
	break;



    case SWIOC_vtss_gpio_event_poll:
	{
	    struct vtss_gpio_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_event_poll");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_event_poll(chipset, ioc.chip_no,
					 &ioc.events);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_event_poll", ret);
	}
	break;



    case SWIOC_vtss_gpio_event_enable:
	{
	    struct vtss_gpio_event_enable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_event_enable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_event_enable(chipset, ioc.chip_no,
					   ioc.gpio_no, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_event_enable", ret);
	}
	break;


#ifdef VTSS_ARCH_B2
    case SWIOC_vtss_gpio_clk_set:
	{
	    struct vtss_gpio_clk_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_gpio_clk_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_gpio_clk_set(chipset, ioc.gpio_no, ioc.port_no,
				      ioc.clk);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_gpio_clk_set", ret);
	}
	break;
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_FEATURE_SERIAL_LED
    case SWIOC_vtss_serial_led_set:
	{
	    struct vtss_serial_led_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_serial_led_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_serial_led_set(chipset, ioc.port, ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_serial_led_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_LED
    case SWIOC_vtss_serial_led_intensity_set:
	{
	    struct vtss_serial_led_intensity_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_serial_led_intensity_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_serial_led_intensity_set(chipset, ioc.port,
						  ioc.intensity);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_serial_led_intensity_set",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_LED
    case SWIOC_vtss_serial_led_intensity_get:
	{
	    struct vtss_serial_led_intensity_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_serial_led_intensity_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret =
		    vtss_serial_led_intensity_get(chipset, &ioc.intensity);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_serial_led_intensity_get",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    case SWIOC_vtss_sgpio_conf_get:
	{
	    struct vtss_sgpio_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sgpio_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sgpio_conf_get(chipset, ioc.chip_no, ioc.group,
					&ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sgpio_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    case SWIOC_vtss_sgpio_conf_set:
	{
	    struct vtss_sgpio_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sgpio_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sgpio_conf_set(chipset, ioc.chip_no, ioc.group,
					&ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sgpio_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    case SWIOC_vtss_sgpio_read:
	{
	    struct vtss_sgpio_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sgpio_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sgpio_read(chipset, ioc.chip_no, ioc.group,
				    ioc.data);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sgpio_read", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    case SWIOC_vtss_sgpio_event_poll:
	{
	    struct vtss_sgpio_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sgpio_event_poll");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sgpio_event_poll(chipset, ioc.chip_no, ioc.group,
					  ioc.bit, ioc.events);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sgpio_event_poll", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    case SWIOC_vtss_sgpio_event_enable:
	{
	    struct vtss_sgpio_event_enable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sgpio_event_enable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sgpio_event_enable(chipset, ioc.chip_no,
					    ioc.group, ioc.port, ioc.bit,
					    ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sgpio_event_enable", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_temp_sensor_init:
	{
	    struct vtss_temp_sensor_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_temp_sensor_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_temp_sensor_init(chipset, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_temp_sensor_init", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_temp_sensor_get:
	{
	    struct vtss_temp_sensor_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_temp_sensor_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_temp_sensor_get(chipset, &ioc.temperature);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_temp_sensor_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_fan_rotation_get:
	{
	    struct vtss_fan_rotation_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_fan_rotation_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret =
		    vtss_fan_rotation_get(chipset, &ioc.fan_spec,
					  &ioc.rotation_count);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_fan_rotation_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_fan_cool_lvl_set:
	{
	    struct vtss_fan_cool_lvl_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_fan_cool_lvl_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_fan_cool_lvl_set(chipset, ioc.lvl);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_fan_cool_lvl_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_fan_controller_init:
	{
	    struct vtss_fan_controller_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_fan_controller_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_fan_controller_init(chipset, &ioc.spec);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_fan_controller_init", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
    case SWIOC_vtss_fan_cool_lvl_get:
	{
	    struct vtss_fan_cool_lvl_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_fan_cool_lvl_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_fan_cool_lvl_get(chipset, &ioc.lvl);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_fan_cool_lvl_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_EEE
    case SWIOC_vtss_eee_port_conf_set:
	{
	    struct vtss_eee_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_eee_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eee_port_conf_set(chipset, ioc.port_no,
					   &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eee_port_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_FEATURE_EEE
    case SWIOC_vtss_eee_port_state_set:
	{
	    struct vtss_eee_port_state_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_eee_port_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eee_port_state_set(chipset, ioc.port_no,
					    &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eee_port_state_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_FEATURE_EEE
    case SWIOC_vtss_eee_port_counter_get:
	{
	    struct vtss_eee_port_counter_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_eee_port_counter_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eee_port_counter_get(chipset, ioc.port_no,
					      &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eee_port_counter_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_EEE */


    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * init group 
  */

static long
switch_ioctl_init(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {


#ifdef VTSS_FEATURE_WARM_START
    case SWIOC_vtss_restart_status_get:
	{
	    struct vtss_restart_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_restart_status_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_restart_status_get(chipset, &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_restart_status_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_WARM_START */

#ifdef VTSS_FEATURE_WARM_START
    case SWIOC_vtss_restart_conf_get:
	{
	    struct vtss_restart_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_restart_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_restart_conf_get(chipset, &ioc.restart);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_restart_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_WARM_START */

#ifdef VTSS_FEATURE_WARM_START
    case SWIOC_vtss_restart_conf_set:
	{
	    struct vtss_restart_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_restart_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_restart_conf_set(chipset, ioc.restart);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_restart_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_WARM_START */


    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * phy group 
  */

static long
switch_ioctl_phy(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {



    case SWIOC_vtss_phy_pre_reset:
	{
	    struct vtss_phy_pre_reset_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_pre_reset");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_pre_reset(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_pre_reset", ret);
	}
	break;



    case SWIOC_vtss_phy_post_reset:
	{
	    struct vtss_phy_post_reset_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_post_reset");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_post_reset(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_post_reset", ret);
	}
	break;



    case SWIOC_vtss_phy_reset:
	{
	    struct vtss_phy_reset_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_reset");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_reset(chipset, ioc.port_no, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_reset", ret);
	}
	break;



    case SWIOC_vtss_phy_chip_temp_get:
	{
	    struct vtss_phy_chip_temp_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_chip_temp_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_chip_temp_get(chipset, ioc.port_no,
					   &ioc.temp);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_chip_temp_get", ret);
	}
	break;



    case SWIOC_vtss_phy_chip_temp_init:
	{
	    struct vtss_phy_chip_temp_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_chip_temp_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_chip_temp_init(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_chip_temp_init", ret);
	}
	break;



    case SWIOC_vtss_phy_conf_get:
	{
	    struct vtss_phy_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_conf_get(chipset, ioc.port_no, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_conf_get", ret);
	}
	break;



    case SWIOC_vtss_phy_conf_set:
	{
	    struct vtss_phy_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_conf_set(chipset, ioc.port_no, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_conf_set", ret);
	}
	break;



    case SWIOC_vtss_phy_status_get:
	{
	    struct vtss_phy_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_status_get(chipset, ioc.port_no, &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_status_get", ret);
	}
	break;



    case SWIOC_vtss_phy_conf_1g_get:
	{
	    struct vtss_phy_conf_1g_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_conf_1g_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_conf_1g_get(chipset, ioc.port_no, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_conf_1g_get", ret);
	}
	break;



    case SWIOC_vtss_phy_conf_1g_set:
	{
	    struct vtss_phy_conf_1g_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_conf_1g_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_conf_1g_set(chipset, ioc.port_no, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_conf_1g_set", ret);
	}
	break;



    case SWIOC_vtss_phy_status_1g_get:
	{
	    struct vtss_phy_status_1g_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_status_1g_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_status_1g_get(chipset, ioc.port_no,
					   &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_status_1g_get", ret);
	}
	break;



    case SWIOC_vtss_phy_power_conf_get:
	{
	    struct vtss_phy_power_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_power_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_power_conf_get(chipset, ioc.port_no,
					    &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_power_conf_get", ret);
	}
	break;



    case SWIOC_vtss_phy_power_conf_set:
	{
	    struct vtss_phy_power_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_power_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_power_conf_set(chipset, ioc.port_no,
					    &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_power_conf_set", ret);
	}
	break;



    case SWIOC_vtss_phy_power_status_get:
	{
	    struct vtss_phy_power_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_power_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_power_status_get(chipset, ioc.port_no,
					      &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_power_status_get", ret);
	}
	break;



    case SWIOC_vtss_phy_clock_conf_set:
	{
	    struct vtss_phy_clock_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_clock_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_clock_conf_set(chipset, ioc.port_no,
					    ioc.clock_port, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_clock_conf_set", ret);
	}
	break;



    case SWIOC_vtss_phy_read:
	{
	    struct vtss_phy_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_read(chipset, ioc.port_no, ioc.addr,
				  &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_read", ret);
	}
	break;



    case SWIOC_vtss_phy_write:
	{
	    struct vtss_phy_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_write(chipset, ioc.port_no, ioc.addr,
				   ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_write", ret);
	}
	break;



    case SWIOC_vtss_phy_mmd_read:
	{
	    struct vtss_phy_mmd_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_mmd_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_mmd_read(chipset, ioc.port_no, ioc.devad,
				      ioc.addr, &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_mmd_read", ret);
	}
	break;



    case SWIOC_vtss_phy_mmd_write:
	{
	    struct vtss_phy_mmd_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_mmd_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_mmd_write(chipset, ioc.port_no, ioc.devad,
				       ioc.addr, ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_mmd_write", ret);
	}
	break;



    case SWIOC_vtss_phy_write_masked:
	{
	    struct vtss_phy_write_masked_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_write_masked");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_write_masked(chipset, ioc.port_no, ioc.addr,
					  ioc.value, ioc.mask);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_write_masked", ret);
	}
	break;



    case SWIOC_vtss_phy_write_masked_page:
	{
	    struct vtss_phy_write_masked_page_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_write_masked_page");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_write_masked_page(chipset, ioc.port_no,
					       ioc.page, ioc.addr,
					       ioc.value, ioc.mask);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_write_masked_page", ret);
	}
	break;



    case SWIOC_vtss_phy_veriphy_start:
	{
	    struct vtss_phy_veriphy_start_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_veriphy_start");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_veriphy_start(chipset, ioc.port_no, ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_veriphy_start", ret);
	}
	break;



    case SWIOC_vtss_phy_veriphy_get:
	{
	    struct vtss_phy_veriphy_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_veriphy_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_veriphy_get(chipset, ioc.port_no,
					 &ioc.result);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_veriphy_get", ret);
	}
	break;


#ifdef VTSS_FEATURE_LED_POW_REDUC
    case SWIOC_vtss_phy_led_intensity_set:
	{
	    struct vtss_phy_led_intensity_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_led_intensity_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_led_intensity_set(chipset, ioc.port_no,
					       ioc.intensity);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_led_intensity_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_LED_POW_REDUC */

#ifdef VTSS_FEATURE_LED_POW_REDUC
    case SWIOC_vtss_phy_enhanced_led_control_init:
	{
	    struct vtss_phy_enhanced_led_control_init_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_enhanced_led_control_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_enhanced_led_control_init(chipset,
						       ioc.port_no,
						       ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_phy_enhanced_led_control_init", ret);
	}
	break;
#endif				/* VTSS_FEATURE_LED_POW_REDUC */


    case SWIOC_vtss_phy_coma_mode_disable:
	{
	    struct vtss_phy_coma_mode_disable_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_coma_mode_disable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_coma_mode_disable(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_coma_mode_disable", ret);
	}
	break;


#ifdef VTSS_FEATURE_EEE
    case SWIOC_vtss_phy_eee_ena:
	{
	    struct vtss_phy_eee_ena_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_eee_ena");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_eee_ena(chipset, ioc.port_no, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_eee_ena", ret);
	}
	break;
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_CHIP_CU_PHY
    case SWIOC_vtss_phy_event_enable_set:
	{
	    struct vtss_phy_event_enable_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_event_enable_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_event_enable_set(chipset, ioc.port_no,
					      ioc.ev_mask, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_event_enable_set", ret);
	}
	break;
#endif				/* VTSS_CHIP_CU_PHY */

#ifdef VTSS_CHIP_CU_PHY
    case SWIOC_vtss_phy_event_enable_get:
	{
	    struct vtss_phy_event_enable_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_event_enable_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_event_enable_get(chipset, ioc.port_no,
					      &ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_event_enable_get", ret);
	}
	break;
#endif				/* VTSS_CHIP_CU_PHY */

#ifdef VTSS_CHIP_CU_PHY
    case SWIOC_vtss_phy_event_poll:
	{
	    struct vtss_phy_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_event_poll");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_event_poll(chipset, ioc.port_no,
					&ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_event_poll", ret);
	}
	break;
#endif				/* VTSS_CHIP_CU_PHY */


    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * 10gphy group 
  */

static long
switch_ioctl_10gphy(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_10G

    case SWIOC_vtss_phy_10g_mode_get:
	{
	    struct vtss_phy_10g_mode_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_mode_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_mode_get(chipset, ioc.port_no, &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_mode_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_mode_set:
	{
	    struct vtss_phy_10g_mode_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_mode_set(chipset, ioc.port_no, &ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_mode_set", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_synce_clkout_get:
	{
	    struct vtss_phy_10g_synce_clkout_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_synce_clkout_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_synce_clkout_get(chipset, ioc.port_no,
						  &ioc.synce_clkout);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_synce_clkout_get",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_synce_clkout_set:
	{
	    struct vtss_phy_10g_synce_clkout_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_synce_clkout_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_synce_clkout_set(chipset, ioc.port_no,
						  ioc.synce_clkout);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_synce_clkout_set",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_xfp_clkout_get:
	{
	    struct vtss_phy_10g_xfp_clkout_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_xfp_clkout_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_xfp_clkout_get(chipset, ioc.port_no,
						&ioc.xfp_clkout);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_xfp_clkout_get",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_xfp_clkout_set:
	{
	    struct vtss_phy_10g_xfp_clkout_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_xfp_clkout_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_xfp_clkout_set(chipset, ioc.port_no,
						ioc.xfp_clkout);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_xfp_clkout_set",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_status_get:
	{
	    struct vtss_phy_10g_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_status_get(chipset, ioc.port_no,
					    &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_status_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_reset:
	{
	    struct vtss_phy_10g_reset_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_reset");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_10g_reset(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_reset", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_loopback_set:
	{
	    struct vtss_phy_10g_loopback_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_loopback_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_loopback_set(chipset, ioc.port_no,
					      &ioc.loopback);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_loopback_set", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_loopback_get:
	{
	    struct vtss_phy_10g_loopback_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_loopback_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_loopback_get(chipset, ioc.port_no,
					      &ioc.loopback);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_loopback_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_cnt_get:
	{
	    struct vtss_phy_10g_cnt_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_cnt_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_phy_10g_cnt_get(chipset, ioc.port_no, &ioc.cnt);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_cnt_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_power_get:
	{
	    struct vtss_phy_10g_power_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_power_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_power_get(chipset, ioc.port_no,
					   &ioc.power);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_power_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_power_set:
	{
	    struct vtss_phy_10g_power_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_power_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_power_set(chipset, ioc.port_no,
					   &ioc.power);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_power_set", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_failover_set:
	{
	    struct vtss_phy_10g_failover_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_failover_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_failover_set(chipset, ioc.port_no,
					      &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_failover_set", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_failover_get:
	{
	    struct vtss_phy_10g_failover_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_failover_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_failover_get(chipset, ioc.port_no,
					      &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_failover_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_id_get:
	{
	    struct vtss_phy_10g_id_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_id_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_id_get(chipset, ioc.port_no, &ioc.phy_id);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_id_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_gpio_mode_set:
	{
	    struct vtss_phy_10g_gpio_mode_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_gpio_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_gpio_mode_set(chipset, ioc.port_no,
					       ioc.gpio_no, &ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_gpio_mode_set", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_gpio_mode_get:
	{
	    struct vtss_phy_10g_gpio_mode_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_gpio_mode_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_gpio_mode_get(chipset, ioc.port_no,
					       ioc.gpio_no, &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_gpio_mode_get", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_gpio_read:
	{
	    struct vtss_phy_10g_gpio_read_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_gpio_read");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_gpio_read(chipset, ioc.port_no,
					   ioc.gpio_no, &ioc.value);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_gpio_read", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_gpio_write:
	{
	    struct vtss_phy_10g_gpio_write_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_gpio_write");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_gpio_write(chipset, ioc.port_no,
					    ioc.gpio_no, ioc.value);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_gpio_write", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_event_enable_set:
	{
	    struct vtss_phy_10g_event_enable_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_event_enable_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_event_enable_set(chipset, ioc.port_no,
						  ioc.ev_mask, ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_event_enable_set",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_event_enable_get:
	{
	    struct vtss_phy_10g_event_enable_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_event_enable_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_event_enable_get(chipset, ioc.port_no,
						  &ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_event_enable_get",
		  ret);
	}
	break;



    case SWIOC_vtss_phy_10g_event_poll:
	{
	    struct vtss_phy_10g_event_poll_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_event_poll");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_event_poll(chipset, ioc.port_no,
					    &ioc.ev_mask);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_event_poll", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_poll_1sec:
	{
	    DEBUG("Calling %s\n", "vtss_phy_10g_poll_1sec");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_phy_10g_poll_1sec(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_poll_1sec", ret);
	}
	break;



    case SWIOC_vtss_phy_10g_edc_fw_status_get:
	{
	    struct vtss_phy_10g_edc_fw_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_phy_10g_edc_fw_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_phy_10g_edc_fw_status_get(chipset, ioc.port_no,
						   &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_phy_10g_edc_fw_status_get",
		  ret);
	}
	break;


#endif				/* VTSS_FEATURE_10G */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * qos group 
  */

static long
switch_ioctl_qos(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_QOS
#ifdef VTSS_ARCH_CARACAL
    case SWIOC_vtss_mep_policer_conf_get:
	{
	    struct vtss_mep_policer_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mep_policer_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mep_policer_conf_get(chipset, ioc.port_no,
					      ioc.prio, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mep_policer_conf_get", ret);
	}
	break;
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
    case SWIOC_vtss_mep_policer_conf_set:
	{
	    struct vtss_mep_policer_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mep_policer_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mep_policer_conf_set(chipset, ioc.port_no,
					      ioc.prio, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mep_policer_conf_set", ret);
	}
	break;
#endif				/* VTSS_ARCH_CARACAL */


    case SWIOC_vtss_qos_conf_get:
	{
	    struct vtss_qos_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qos_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_qos_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qos_conf_get", ret);
	}
	break;



    case SWIOC_vtss_qos_conf_set:
	{
	    struct vtss_qos_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qos_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_qos_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qos_conf_set", ret);
	}
	break;



    case SWIOC_vtss_qos_port_conf_get:
	{
	    struct vtss_qos_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qos_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_qos_port_conf_get(chipset, ioc.port_no,
					   &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qos_port_conf_get", ret);
	}
	break;



    case SWIOC_vtss_qos_port_conf_set:
	{
	    struct vtss_qos_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qos_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_qos_port_conf_set(chipset, ioc.port_no,
					   &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qos_port_conf_set", ret);
	}
	break;


#ifdef VTSS_FEATURE_QCL
    case SWIOC_vtss_qce_init:
	{
	    struct vtss_qce_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qce_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_qce_init(chipset, ioc.type, &ioc.qce);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qce_init", ret);
	}
	break;
#endif				/* VTSS_FEATURE_QCL */

#ifdef VTSS_FEATURE_QCL
    case SWIOC_vtss_qce_add:
	{
	    struct vtss_qce_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qce_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_qce_add(chipset, ioc.qcl_id, ioc.qce_id,
				 &ioc.qce);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qce_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_QCL */

#ifdef VTSS_FEATURE_QCL
    case SWIOC_vtss_qce_del:
	{
	    struct vtss_qce_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_qce_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_qce_del(chipset, ioc.qcl_id, ioc.qce_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_qce_del", ret);
	}
	break;
#endif				/* VTSS_FEATURE_QCL */

#endif				/* VTSS_FEATURE_QOS */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * packet group 
  */

static long
switch_ioctl_packet(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_PACKET
#ifdef VTSS_FEATURE_NPI
    case SWIOC_vtss_npi_conf_get:
	{
	    struct vtss_npi_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_npi_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_npi_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_npi_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_NPI */

#ifdef VTSS_FEATURE_NPI
    case SWIOC_vtss_npi_conf_set:
	{
	    struct vtss_npi_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_npi_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_npi_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_npi_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_NPI */


    case SWIOC_vtss_packet_rx_conf_get:
	{
	    struct vtss_packet_rx_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_rx_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_packet_rx_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_rx_conf_get", ret);
	}
	break;



    case SWIOC_vtss_packet_rx_conf_set:
	{
	    struct vtss_packet_rx_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_rx_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_packet_rx_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_rx_conf_set", ret);
	}
	break;


#ifdef VTSS_FEATURE_PACKET_PORT_REG
    case SWIOC_vtss_packet_rx_port_conf_get:
	{
	    struct vtss_packet_rx_port_conf_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_rx_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_packet_rx_port_conf_get(chipset, ioc.port_no,
						 &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_rx_port_conf_get",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */

#ifdef VTSS_FEATURE_PACKET_PORT_REG
    case SWIOC_vtss_packet_rx_port_conf_set:
	{
	    struct vtss_packet_rx_port_conf_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_rx_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_packet_rx_port_conf_set(chipset, ioc.port_no,
						 &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_rx_port_conf_set",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */


    case SWIOC_vtss_packet_frame_filter:
	{
	    struct vtss_packet_frame_filter_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_frame_filter");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_packet_frame_filter(chipset, &ioc.info,
					     &ioc.filter);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_frame_filter", ret);
	}
	break;



    case SWIOC_vtss_packet_port_info_init:
	{
	    struct vtss_packet_port_info_init_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_port_info_init");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_packet_port_info_init(&ioc.info);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_port_info_init", ret);
	}
	break;



    case SWIOC_vtss_packet_port_filter_get:
	{
	    struct vtss_packet_port_filter_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_port_filter_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_packet_port_filter_get(chipset, &ioc.info,
						ioc.filter);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_port_filter_get",
		  ret);
	}
	break;


#ifdef VTSS_FEATURE_AFI_SWC
    case SWIOC_vtss_afi_alloc:
	{
	    struct vtss_afi_alloc_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_afi_alloc");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_afi_alloc(chipset, &ioc.dscr, &ioc.id);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_afi_alloc", ret);
	}
	break;
#endif				/* VTSS_FEATURE_AFI_SWC */

#ifdef VTSS_FEATURE_AFI_SWC
    case SWIOC_vtss_afi_free:
	{
	    struct vtss_afi_free_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_afi_free");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_afi_free(chipset, ioc.id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_afi_free", ret);
	}
	break;
#endif				/* VTSS_FEATURE_AFI_SWC */


    case SWIOC_vtss_packet_rx_hdr_decode:
	{
	    struct vtss_packet_rx_hdr_decode_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_rx_hdr_decode");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_packet_rx_hdr_decode(chipset, &ioc.meta,
					      &ioc.info);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_rx_hdr_decode", ret);
	}
	break;



    case SWIOC_vtss_packet_tx_info_init:
	{
	    struct vtss_packet_tx_info_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_packet_tx_info_init");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_packet_tx_info_init(chipset, &ioc.info);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_packet_tx_info_init", ret);
	}
	break;


#endif				/* VTSS_FEATURE_PACKET */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * security group 
  */

static long
switch_ioctl_security(struct file *filp, unsigned int cmd,
		      unsigned long arg)
{
    int ret = 0;

    switch (cmd) {


#ifdef VTSS_FEATURE_LAYER2
    case SWIOC_vtss_auth_port_state_get:
	{
	    struct vtss_auth_port_state_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_auth_port_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_auth_port_state_get(chipset, ioc.port_no,
					     &ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_auth_port_state_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_LAYER2 */

#ifdef VTSS_FEATURE_LAYER2
    case SWIOC_vtss_auth_port_state_set:
	{
	    struct vtss_auth_port_state_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_auth_port_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_auth_port_state_set(chipset, ioc.port_no,
					     ioc.state);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_auth_port_state_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_LAYER2 */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_policer_conf_get:
	{
	    struct vtss_acl_policer_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_policer_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_acl_policer_conf_get(chipset, ioc.policer_no,
					      &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_policer_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_policer_conf_set:
	{
	    struct vtss_acl_policer_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_policer_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_acl_policer_conf_set(chipset, ioc.policer_no,
					      &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_policer_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_port_conf_get:
	{
	    struct vtss_acl_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_acl_port_conf_get(chipset, ioc.port_no,
					   &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_port_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_port_conf_set:
	{
	    struct vtss_acl_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_acl_port_conf_set(chipset, ioc.port_no,
					   &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_port_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_port_counter_get:
	{
	    struct vtss_acl_port_counter_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_port_counter_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_acl_port_counter_get(chipset, ioc.port_no,
					      &ioc.counter);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_port_counter_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_acl_port_counter_clear:
	{
	    struct vtss_acl_port_counter_clear_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_acl_port_counter_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_acl_port_counter_clear(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_acl_port_counter_clear",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_ace_init:
	{
	    struct vtss_ace_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ace_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ace_init(chipset, ioc.type, &ioc.ace);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ace_init", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_ace_add:
	{
	    struct vtss_ace_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ace_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ace_add(chipset, ioc.ace_id, &ioc.ace);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ace_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_ace_del:
	{
	    struct vtss_ace_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ace_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ace_del(chipset, ioc.ace_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ace_del", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_ace_counter_get:
	{
	    struct vtss_ace_counter_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ace_counter_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ace_counter_get(chipset, ioc.ace_id,
					 &ioc.counter);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ace_counter_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
    case SWIOC_vtss_ace_counter_clear:
	{
	    struct vtss_ace_counter_clear_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ace_counter_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ace_counter_clear(chipset, ioc.ace_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ace_counter_clear", ret);
	}
	break;
#endif				/* VTSS_FEATURE_ACL */


    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * layer2 group 
  */

static long
switch_ioctl_layer2(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_LAYER2

    case SWIOC_vtss_mac_table_add:
	{
	    struct vtss_mac_table_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_add(chipset, &ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_add", ret);
	}
	break;



    case SWIOC_vtss_mac_table_del:
	{
	    struct vtss_mac_table_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_del(chipset, &ioc.vid_mac);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_del", ret);
	}
	break;



    case SWIOC_vtss_mac_table_get:
	{
	    struct vtss_mac_table_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_get(chipset, &ioc.vid_mac, &ioc.entry);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_get", ret);
	}
	break;



    case SWIOC_vtss_mac_table_get_next:
	{
	    struct vtss_mac_table_get_next_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_get_next");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_get_next(chipset, &ioc.vid_mac,
					    &ioc.entry);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_get_next", ret);
	}
	break;


#ifdef VTSS_FEATURE_MAC_AGE_AUTO
    case SWIOC_vtss_mac_table_age_time_get:
	{
	    struct vtss_mac_table_age_time_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_age_time_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mac_table_age_time_get(chipset, &ioc.age_time);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_age_time_get",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */

#ifdef VTSS_FEATURE_MAC_AGE_AUTO
    case SWIOC_vtss_mac_table_age_time_set:
	{
	    struct vtss_mac_table_age_time_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_age_time_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_age_time_set(chipset, ioc.age_time);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_age_time_set",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */


    case SWIOC_vtss_mac_table_age:
	{
	    DEBUG("Calling %s\n", "vtss_mac_table_age");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mac_table_age(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_age", ret);
	}
	break;



    case SWIOC_vtss_mac_table_vlan_age:
	{
	    struct vtss_mac_table_vlan_age_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_vlan_age");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_vlan_age(chipset, ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_vlan_age", ret);
	}
	break;



    case SWIOC_vtss_mac_table_flush:
	{
	    DEBUG("Calling %s\n", "vtss_mac_table_flush");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mac_table_flush(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_flush", ret);
	}
	break;



    case SWIOC_vtss_mac_table_port_flush:
	{
	    struct vtss_mac_table_port_flush_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_port_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_port_flush(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_port_flush", ret);
	}
	break;



    case SWIOC_vtss_mac_table_vlan_flush:
	{
	    struct vtss_mac_table_vlan_flush_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_vlan_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_vlan_flush(chipset, ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_vlan_flush", ret);
	}
	break;



    case SWIOC_vtss_mac_table_vlan_port_flush:
	{
	    struct vtss_mac_table_vlan_port_flush_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_vlan_port_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_vlan_port_flush(chipset, ioc.port_no,
						   ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_vlan_port_flush",
		  ret);
	}
	break;


#ifdef VTSS_FEATURE_VSTAX_V2
    case SWIOC_vtss_mac_table_upsid_flush:
	{
	    struct vtss_mac_table_upsid_flush_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_upsid_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_upsid_flush(chipset, ioc.upsid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_upsid_flush", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_VSTAX_V2
    case SWIOC_vtss_mac_table_upsid_upspn_flush:
	{
	    struct vtss_mac_table_upsid_upspn_flush_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_upsid_upspn_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_upsid_upspn_flush(chipset, ioc.upsid,
						     ioc.upspn);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_upsid_upspn_flush",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_AGGR_GLAG
    case SWIOC_vtss_mac_table_glag_add:
	{
	    struct vtss_mac_table_glag_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_glag_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_glag_add(chipset, &ioc.entry,
					    ioc.glag_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_glag_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_AGGR_GLAG
    case SWIOC_vtss_mac_table_glag_flush:
	{
	    struct vtss_mac_table_glag_flush_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_glag_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mac_table_glag_flush(chipset, ioc.glag_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_glag_flush", ret);
	}
	break;
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_AGGR_GLAG
    case SWIOC_vtss_mac_table_vlan_glag_flush:
	{
	    struct vtss_mac_table_vlan_glag_flush_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_vlan_glag_flush");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mac_table_vlan_glag_flush(chipset, ioc.glag_no,
						   ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_vlan_glag_flush",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_AGGR_GLAG */


    case SWIOC_vtss_mac_table_status_get:
	{
	    struct vtss_mac_table_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mac_table_status_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mac_table_status_get(chipset, &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mac_table_status_get", ret);
	}
	break;



    case SWIOC_vtss_learn_port_mode_get:
	{
	    struct vtss_learn_port_mode_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_learn_port_mode_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_learn_port_mode_get(chipset, ioc.port_no,
					     &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_learn_port_mode_get", ret);
	}
	break;



    case SWIOC_vtss_learn_port_mode_set:
	{
	    struct vtss_learn_port_mode_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_learn_port_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_learn_port_mode_set(chipset, ioc.port_no,
					     &ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_learn_port_mode_set", ret);
	}
	break;



    case SWIOC_vtss_port_state_get:
	{
	    struct vtss_port_state_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_port_state_get(chipset, ioc.port_no, &ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_state_get", ret);
	}
	break;



    case SWIOC_vtss_port_state_set:
	{
	    struct vtss_port_state_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_port_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_port_state_set(chipset, ioc.port_no, ioc.state);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_port_state_set", ret);
	}
	break;



    case SWIOC_vtss_stp_port_state_get:
	{
	    struct vtss_stp_port_state_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_stp_port_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_stp_port_state_get(chipset, ioc.port_no,
					    &ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_stp_port_state_get", ret);
	}
	break;



    case SWIOC_vtss_stp_port_state_set:
	{
	    struct vtss_stp_port_state_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_stp_port_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_stp_port_state_set(chipset, ioc.port_no,
					    ioc.state);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_stp_port_state_set", ret);
	}
	break;



    case SWIOC_vtss_mstp_vlan_msti_get:
	{
	    struct vtss_mstp_vlan_msti_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mstp_vlan_msti_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mstp_vlan_msti_get(chipset, ioc.vid, &ioc.msti);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mstp_vlan_msti_get", ret);
	}
	break;



    case SWIOC_vtss_mstp_vlan_msti_set:
	{
	    struct vtss_mstp_vlan_msti_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mstp_vlan_msti_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mstp_vlan_msti_set(chipset, ioc.vid, ioc.msti);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mstp_vlan_msti_set", ret);
	}
	break;



    case SWIOC_vtss_mstp_port_msti_state_get:
	{
	    struct vtss_mstp_port_msti_state_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mstp_port_msti_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mstp_port_msti_state_get(chipset, ioc.port_no,
						  ioc.msti, &ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mstp_port_msti_state_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mstp_port_msti_state_set:
	{
	    struct vtss_mstp_port_msti_state_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mstp_port_msti_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mstp_port_msti_state_set(chipset, ioc.port_no,
						  ioc.msti, ioc.state);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mstp_port_msti_state_set",
		  ret);
	}
	break;



    case SWIOC_vtss_vlan_conf_get:
	{
	    struct vtss_vlan_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_vlan_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_conf_get", ret);
	}
	break;



    case SWIOC_vtss_vlan_conf_set:
	{
	    struct vtss_vlan_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vlan_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_conf_set", ret);
	}
	break;



    case SWIOC_vtss_vlan_port_conf_get:
	{
	    struct vtss_vlan_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_port_conf_get(chipset, ioc.port_no,
					    &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_port_conf_get", ret);
	}
	break;



    case SWIOC_vtss_vlan_port_conf_set:
	{
	    struct vtss_vlan_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_port_conf_set(chipset, ioc.port_no,
					    &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_port_conf_set", ret);
	}
	break;



    case SWIOC_vtss_vlan_port_members_get:
	{
	    struct vtss_vlan_port_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_port_members_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_port_members_get(chipset, ioc.vid,
					       ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_port_members_get", ret);
	}
	break;



    case SWIOC_vtss_vlan_port_members_set:
	{
	    struct vtss_vlan_port_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_port_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_port_members_set(chipset, ioc.vid,
					       ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_port_members_set", ret);
	}
	break;


#ifdef VTSS_FEATURE_VLAN_COUNTERS
    case SWIOC_vtss_vlan_counters_get:
	{
	    struct vtss_vlan_counters_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_counters_get(chipset, ioc.vid,
					   &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_counters_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */

#ifdef VTSS_FEATURE_VLAN_COUNTERS
    case SWIOC_vtss_vlan_counters_clear:
	{
	    struct vtss_vlan_counters_clear_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_counters_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vlan_counters_clear(chipset, ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_counters_clear", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */


    case SWIOC_vtss_vce_init:
	{
	    struct vtss_vce_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vce_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vce_init(chipset, ioc.type, &ioc.vce);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vce_init", ret);
	}
	break;



    case SWIOC_vtss_vce_add:
	{
	    struct vtss_vce_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vce_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vce_add(chipset, ioc.vce_id, &ioc.vce);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vce_add", ret);
	}
	break;



    case SWIOC_vtss_vce_del:
	{
	    struct vtss_vce_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vce_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vce_del(chipset, ioc.vce_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vce_del", ret);
	}
	break;


#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    case SWIOC_vtss_vlan_trans_group_add:
	{
	    struct vtss_vlan_trans_group_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_trans_group_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_trans_group_add(chipset, ioc.group_id,
					      ioc.vid, ioc.trans_vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_trans_group_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    case SWIOC_vtss_vlan_trans_group_del:
	{
	    struct vtss_vlan_trans_group_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_trans_group_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_trans_group_del(chipset, ioc.group_id,
					      ioc.vid);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_trans_group_del", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    case SWIOC_vtss_vlan_trans_group_get:
	{
	    struct vtss_vlan_trans_group_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_trans_group_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_trans_group_get(chipset, &ioc.conf,
					      ioc.next);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vlan_trans_group_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    case SWIOC_vtss_vlan_trans_group_to_port_set:
	{
	    struct vtss_vlan_trans_group_to_port_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_trans_group_to_port_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_trans_group_to_port_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_vlan_trans_group_to_port_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
    case SWIOC_vtss_vlan_trans_group_to_port_get:
	{
	    struct vtss_vlan_trans_group_to_port_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_vlan_trans_group_to_port_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vlan_trans_group_to_port_get(chipset, &ioc.conf,
						      ioc.next);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_vlan_trans_group_to_port_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */


    case SWIOC_vtss_isolated_vlan_get:
	{
	    struct vtss_isolated_vlan_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_isolated_vlan_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_isolated_vlan_get(chipset, ioc.vid,
					   &ioc.isolated);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_isolated_vlan_get", ret);
	}
	break;



    case SWIOC_vtss_isolated_vlan_set:
	{
	    struct vtss_isolated_vlan_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_isolated_vlan_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_isolated_vlan_set(chipset, ioc.vid, ioc.isolated);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_isolated_vlan_set", ret);
	}
	break;



    case SWIOC_vtss_isolated_port_members_get:
	{
	    struct vtss_isolated_port_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_isolated_port_members_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_isolated_port_members_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_isolated_port_members_get",
		  ret);
	}
	break;



    case SWIOC_vtss_isolated_port_members_set:
	{
	    struct vtss_isolated_port_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_isolated_port_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_isolated_port_members_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_isolated_port_members_set",
		  ret);
	}
	break;


#ifdef VTSS_FEATURE_PVLAN
    case SWIOC_vtss_pvlan_port_members_get:
	{
	    struct vtss_pvlan_port_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_pvlan_port_members_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_pvlan_port_members_get(chipset, ioc.pvlan_no,
						ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_pvlan_port_members_get",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_PVLAN */

#ifdef VTSS_FEATURE_PVLAN
    case SWIOC_vtss_pvlan_port_members_set:
	{
	    struct vtss_pvlan_port_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_pvlan_port_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_pvlan_port_members_set(chipset, ioc.pvlan_no,
						ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_pvlan_port_members_set",
		  ret);
	}
	break;
#endif				/* VTSS_FEATURE_PVLAN */

#ifdef VTSS_FEATURE_SFLOW
    case SWIOC_vtss_sflow_port_conf_get:
	{
	    struct vtss_sflow_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sflow_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sflow_port_conf_get(chipset, ioc.port_no,
					     &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sflow_port_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SFLOW */

#ifdef VTSS_FEATURE_SFLOW
    case SWIOC_vtss_sflow_port_conf_set:
	{
	    struct vtss_sflow_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_sflow_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_sflow_port_conf_set(chipset, ioc.port_no,
					     &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_sflow_port_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_SFLOW */


    case SWIOC_vtss_aggr_port_members_get:
	{
	    struct vtss_aggr_port_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_port_members_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_aggr_port_members_get(chipset, ioc.aggr_no,
					       ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_port_members_get", ret);
	}
	break;



    case SWIOC_vtss_aggr_port_members_set:
	{
	    struct vtss_aggr_port_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_port_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_aggr_port_members_set(chipset, ioc.aggr_no,
					       ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_port_members_set", ret);
	}
	break;



    case SWIOC_vtss_aggr_mode_get:
	{
	    struct vtss_aggr_mode_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_mode_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_aggr_mode_get(chipset, &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_mode_get", ret);
	}
	break;



    case SWIOC_vtss_aggr_mode_set:
	{
	    struct vtss_aggr_mode_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_aggr_mode_set(chipset, &ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_mode_set", ret);
	}
	break;


#ifdef VTSS_FEATURE_AGGR_GLAG
    case SWIOC_vtss_aggr_glag_members_get:
	{
	    struct vtss_aggr_glag_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_glag_members_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_aggr_glag_members_get(chipset, ioc.glag_no,
					       ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_glag_members_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_VSTAX_V1
    case SWIOC_vtss_aggr_glag_set:
	{
	    struct vtss_aggr_glag_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_aggr_glag_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_aggr_glag_set(chipset, ioc.glag_no, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_aggr_glag_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX_V1 */

#ifdef VTSS_FEATURE_VSTAX_V2
    case SWIOC_vtss_vstax_glag_get:
	{
	    struct vtss_vstax_glag_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_glag_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vstax_glag_get(chipset, ioc.glag_no, ioc.entry);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_glag_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_VSTAX_V2
    case SWIOC_vtss_vstax_glag_set:
	{
	    struct vtss_vstax_glag_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_glag_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vstax_glag_set(chipset, ioc.glag_no, ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_glag_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX_V2 */


    case SWIOC_vtss_mirror_monitor_port_get:
	{
	    struct vtss_mirror_monitor_port_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_monitor_port_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mirror_monitor_port_get(chipset, &ioc.port_no);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_monitor_port_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_monitor_port_set:
	{
	    struct vtss_mirror_monitor_port_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_monitor_port_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mirror_monitor_port_set(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_monitor_port_set",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_ingress_ports_get:
	{
	    struct vtss_mirror_ingress_ports_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_ingress_ports_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mirror_ingress_ports_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_ingress_ports_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_ingress_ports_set:
	{
	    struct vtss_mirror_ingress_ports_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_ingress_ports_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mirror_ingress_ports_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_ingress_ports_set",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_egress_ports_get:
	{
	    struct vtss_mirror_egress_ports_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_egress_ports_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mirror_egress_ports_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_egress_ports_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_egress_ports_set:
	{
	    struct vtss_mirror_egress_ports_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_egress_ports_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mirror_egress_ports_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_egress_ports_set",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_cpu_ingress_get:
	{
	    struct vtss_mirror_cpu_ingress_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_cpu_ingress_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mirror_cpu_ingress_get(chipset, &ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_cpu_ingress_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_cpu_ingress_set:
	{
	    struct vtss_mirror_cpu_ingress_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_cpu_ingress_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mirror_cpu_ingress_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_cpu_ingress_set",
		  ret);
	}
	break;



    case SWIOC_vtss_mirror_cpu_egress_get:
	{
	    struct vtss_mirror_cpu_egress_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_cpu_egress_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mirror_cpu_egress_get(chipset, &ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_cpu_egress_get", ret);
	}
	break;



    case SWIOC_vtss_mirror_cpu_egress_set:
	{
	    struct vtss_mirror_cpu_egress_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mirror_cpu_egress_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mirror_cpu_egress_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mirror_cpu_egress_set", ret);
	}
	break;



    case SWIOC_vtss_uc_flood_members_get:
	{
	    struct vtss_uc_flood_members_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_uc_flood_members_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_uc_flood_members_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_uc_flood_members_get", ret);
	}
	break;



    case SWIOC_vtss_uc_flood_members_set:
	{
	    struct vtss_uc_flood_members_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_uc_flood_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_uc_flood_members_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_uc_flood_members_set", ret);
	}
	break;



    case SWIOC_vtss_mc_flood_members_get:
	{
	    struct vtss_mc_flood_members_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mc_flood_members_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mc_flood_members_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mc_flood_members_get", ret);
	}
	break;



    case SWIOC_vtss_mc_flood_members_set:
	{
	    struct vtss_mc_flood_members_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mc_flood_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mc_flood_members_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mc_flood_members_set", ret);
	}
	break;



    case SWIOC_vtss_ipv4_mc_flood_members_get:
	{
	    struct vtss_ipv4_mc_flood_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv4_mc_flood_members_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ipv4_mc_flood_members_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv4_mc_flood_members_get",
		  ret);
	}
	break;



    case SWIOC_vtss_ipv4_mc_flood_members_set:
	{
	    struct vtss_ipv4_mc_flood_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv4_mc_flood_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ipv4_mc_flood_members_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv4_mc_flood_members_set",
		  ret);
	}
	break;


#ifdef VTSS_FEATURE_IPV4_MC_SIP
    case SWIOC_vtss_ipv4_mc_add:
	{
	    struct vtss_ipv4_mc_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv4_mc_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ipv4_mc_add(chipset, ioc.vid, ioc.sip, ioc.dip,
				     ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv4_mc_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */

#ifdef VTSS_FEATURE_IPV4_MC_SIP
    case SWIOC_vtss_ipv4_mc_del:
	{
	    struct vtss_ipv4_mc_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv4_mc_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ipv4_mc_del(chipset, ioc.vid, ioc.sip, ioc.dip);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv4_mc_del", ret);
	}
	break;
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */


    case SWIOC_vtss_ipv6_mc_flood_members_get:
	{
	    struct vtss_ipv6_mc_flood_members_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_flood_members_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ipv6_mc_flood_members_get(chipset, ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_flood_members_get",
		  ret);
	}
	break;



    case SWIOC_vtss_ipv6_mc_flood_members_set:
	{
	    struct vtss_ipv6_mc_flood_members_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_flood_members_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ipv6_mc_flood_members_set(chipset, ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_flood_members_set",
		  ret);
	}
	break;



    case SWIOC_vtss_ipv6_mc_ctrl_flood_get:
	{
	    struct vtss_ipv6_mc_ctrl_flood_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_ctrl_flood_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ipv6_mc_ctrl_flood_get(chipset, &ioc.scope);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_ctrl_flood_get",
		  ret);
	}
	break;



    case SWIOC_vtss_ipv6_mc_ctrl_flood_set:
	{
	    struct vtss_ipv6_mc_ctrl_flood_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_ctrl_flood_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ipv6_mc_ctrl_flood_set(chipset, ioc.scope);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_ctrl_flood_set",
		  ret);
	}
	break;


#ifdef VTSS_FEATURE_IPV6_MC_SIP
    case SWIOC_vtss_ipv6_mc_add:
	{
	    struct vtss_ipv6_mc_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ipv6_mc_add(chipset, ioc.vid, ioc.sip, ioc.dip,
				     ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_add", ret);
	}
	break;
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */

#ifdef VTSS_FEATURE_IPV6_MC_SIP
    case SWIOC_vtss_ipv6_mc_del:
	{
	    struct vtss_ipv6_mc_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ipv6_mc_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ipv6_mc_del(chipset, ioc.vid, ioc.sip, ioc.dip);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ipv6_mc_del", ret);
	}
	break;
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */


    case SWIOC_vtss_eps_port_conf_get:
	{
	    struct vtss_eps_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_eps_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eps_port_conf_get(chipset, ioc.port_no,
					   &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eps_port_conf_get", ret);
	}
	break;



    case SWIOC_vtss_eps_port_conf_set:
	{
	    struct vtss_eps_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_eps_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eps_port_conf_set(chipset, ioc.port_no,
					   &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eps_port_conf_set", ret);
	}
	break;



    case SWIOC_vtss_eps_port_selector_get:
	{
	    struct vtss_eps_port_selector_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_eps_port_selector_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eps_port_selector_get(chipset, ioc.port_no,
					       &ioc.selector);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eps_port_selector_get", ret);
	}
	break;



    case SWIOC_vtss_eps_port_selector_set:
	{
	    struct vtss_eps_port_selector_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_eps_port_selector_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_eps_port_selector_set(chipset, ioc.port_no,
					       ioc.selector);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_eps_port_selector_set", ret);
	}
	break;



    case SWIOC_vtss_erps_vlan_member_get:
	{
	    struct vtss_erps_vlan_member_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_erps_vlan_member_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_erps_vlan_member_get(chipset, ioc.erpi, ioc.vid,
					      &ioc.member);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_erps_vlan_member_get", ret);
	}
	break;



    case SWIOC_vtss_erps_vlan_member_set:
	{
	    struct vtss_erps_vlan_member_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_erps_vlan_member_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_erps_vlan_member_set(chipset, ioc.erpi, ioc.vid,
					      ioc.member);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_erps_vlan_member_set", ret);
	}
	break;



    case SWIOC_vtss_erps_port_state_get:
	{
	    struct vtss_erps_port_state_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_erps_port_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_erps_port_state_get(chipset, ioc.erpi,
					     ioc.port_no, &ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_erps_port_state_get", ret);
	}
	break;



    case SWIOC_vtss_erps_port_state_set:
	{
	    struct vtss_erps_port_state_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_erps_port_state_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_erps_port_state_set(chipset, ioc.erpi,
					     ioc.port_no, ioc.state);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_erps_port_state_set", ret);
	}
	break;


#ifdef VTSS_ARCH_B2
    case SWIOC_vtss_vid2port_set:
	{
	    struct vtss_vid2port_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vid2port_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vid2port_set(chipset, ioc.vid, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vid2port_set", ret);
	}
	break;
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_ARCH_B2
    case SWIOC_vtss_vid2lport_get:
	{
	    struct vtss_vid2lport_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vid2lport_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vid2lport_get(chipset, ioc.vid, &ioc.lport_no);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vid2lport_get", ret);
	}
	break;
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_ARCH_B2
    case SWIOC_vtss_vid2lport_set:
	{
	    struct vtss_vid2lport_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vid2lport_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vid2lport_set(chipset, ioc.vid, ioc.lport_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vid2lport_set", ret);
	}
	break;
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_FEATURE_VSTAX
    case SWIOC_vtss_vstax_conf_get:
	{
	    struct vtss_vstax_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_vstax_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
    case SWIOC_vtss_vstax_conf_set:
	{
	    struct vtss_vstax_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_vstax_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
    case SWIOC_vtss_vstax_port_conf_get:
	{
	    struct vtss_vstax_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vstax_port_conf_get(chipset, ioc.chip_no,
					     ioc.stack_port_a, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_port_conf_get", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
    case SWIOC_vtss_vstax_port_conf_set:
	{
	    struct vtss_vstax_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vstax_port_conf_set(chipset, ioc.chip_no,
					     ioc.stack_port_a, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_port_conf_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
    case SWIOC_vtss_vstax_topology_set:
	{
	    struct vtss_vstax_topology_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_vstax_topology_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_vstax_topology_set(chipset, ioc.chip_no,
					    &ioc.table);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_vstax_topology_set", ret);
	}
	break;
#endif				/* VTSS_FEATURE_VSTAX */

#endif				/* VTSS_FEATURE_LAYER2 */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * evc group 
  */

static long
switch_ioctl_evc(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_EVC

    case SWIOC_vtss_evc_port_conf_get:
	{
	    struct vtss_evc_port_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_port_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_port_conf_get(chipset, ioc.port_no,
					   &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_port_conf_get", ret);
	}
	break;



    case SWIOC_vtss_evc_port_conf_set:
	{
	    struct vtss_evc_port_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_port_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_port_conf_set(chipset, ioc.port_no,
					   &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_port_conf_set", ret);
	}
	break;



    case SWIOC_vtss_evc_add:
	{
	    struct vtss_evc_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_evc_add(chipset, ioc.evc_id, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_add", ret);
	}
	break;



    case SWIOC_vtss_evc_del:
	{
	    struct vtss_evc_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_evc_del(chipset, ioc.evc_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_del", ret);
	}
	break;



    case SWIOC_vtss_evc_get:
	{
	    struct vtss_evc_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_evc_get(chipset, ioc.evc_id, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_get", ret);
	}
	break;



    case SWIOC_vtss_ece_init:
	{
	    struct vtss_ece_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ece_init");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ece_init(chipset, ioc.type, &ioc.ece);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ece_init", ret);
	}
	break;



    case SWIOC_vtss_ece_add:
	{
	    struct vtss_ece_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ece_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ece_add(chipset, ioc.ece_id, &ioc.ece);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ece_add", ret);
	}
	break;



    case SWIOC_vtss_ece_del:
	{
	    struct vtss_ece_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ece_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ece_del(chipset, ioc.ece_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ece_del", ret);
	}
	break;


#ifdef VTSS_ARCH_CARACAL
    case SWIOC_vtss_mce_init:
	{
	    struct vtss_mce_init_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mce_init");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mce_init(chipset, &ioc.mce);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mce_init", ret);
	}
	break;
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
    case SWIOC_vtss_mce_add:
	{
	    struct vtss_mce_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mce_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mce_add(chipset, ioc.mce_id, &ioc.mce);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mce_add", ret);
	}
	break;
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
    case SWIOC_vtss_mce_del:
	{
	    struct vtss_mce_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mce_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mce_del(chipset, ioc.mce_id);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mce_del", ret);
	}
	break;
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_JAGUAR_1
    case SWIOC_vtss_evc_counters_get:
	{
	    struct vtss_evc_counters_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_counters_get(chipset, ioc.evc_id, ioc.port_no,
					  &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_counters_get", ret);
	}
	break;
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
    case SWIOC_vtss_evc_counters_clear:
	{
	    struct vtss_evc_counters_clear_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_counters_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_counters_clear(chipset, ioc.evc_id,
					    ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_counters_clear", ret);
	}
	break;
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
    case SWIOC_vtss_ece_counters_get:
	{
	    struct vtss_ece_counters_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ece_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ece_counters_get(chipset, ioc.ece_id, ioc.port_no,
					  &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ece_counters_get", ret);
	}
	break;
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
    case SWIOC_vtss_ece_counters_clear:
	{
	    struct vtss_ece_counters_clear_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ece_counters_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ece_counters_clear(chipset, ioc.ece_id,
					    ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ece_counters_clear", ret);
	}
	break;
#endif				/* VTSS_ARCH_JAGUAR_1 */

#endif				/* VTSS_FEATURE_EVC */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * qos_policer_dlb group 
  */

static long
switch_ioctl_qos_policer_dlb(struct file *filp, unsigned int cmd,
			     unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_QOS_POLICER_DLB

    case SWIOC_vtss_evc_policer_conf_get:
	{
	    struct vtss_evc_policer_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_policer_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_policer_conf_get(chipset, ioc.policer_id,
					      &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_policer_conf_get", ret);
	}
	break;



    case SWIOC_vtss_evc_policer_conf_set:
	{
	    struct vtss_evc_policer_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_evc_policer_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_evc_policer_conf_set(chipset, ioc.policer_id,
					      &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_evc_policer_conf_set", ret);
	}
	break;


#endif				/* VTSS_FEATURE_QOS_POLICER_DLB */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * timestamp group 
  */

static long
switch_ioctl_timestamp(struct file *filp, unsigned int cmd,
		       unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_TIMESTAMP

    case SWIOC_vtss_ts_timeofday_set:
	{
	    struct vtss_ts_timeofday_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_timeofday_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ts_timeofday_set(chipset, &ioc.ts);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_timeofday_set", ret);
	}
	break;



    case SWIOC_vtss_ts_adjtimer_one_sec:
	{
	    struct vtss_ts_adjtimer_one_sec_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_adjtimer_one_sec");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret =
		    vtss_ts_adjtimer_one_sec(chipset,
					     &ioc.ongoing_adjustment);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_adjtimer_one_sec", ret);
	}
	break;



    case SWIOC_vtss_ts_timeofday_get:
	{
	    struct vtss_ts_timeofday_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_timeofday_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ts_timeofday_get(chipset, &ioc.ts, &ioc.tc);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_timeofday_get", ret);
	}
	break;



    case SWIOC_vtss_ts_adjtimer_set:
	{
	    struct vtss_ts_adjtimer_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_adjtimer_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_ts_adjtimer_set(chipset, ioc.adj);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_adjtimer_set", ret);
	}
	break;



    case SWIOC_vtss_ts_adjtimer_get:
	{
	    struct vtss_ts_adjtimer_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_adjtimer_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ts_adjtimer_get(chipset, &ioc.adj);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_adjtimer_get", ret);
	}
	break;



    case SWIOC_vtss_ts_freq_offset_get:
	{
	    struct vtss_ts_freq_offset_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_freq_offset_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_ts_freq_offset_get(chipset, &ioc.adj);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_freq_offset_get", ret);
	}
	break;



    case SWIOC_vtss_ts_external_clock_mode_set:
	{
	    struct vtss_ts_external_clock_mode_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_external_clock_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_external_clock_mode_set(chipset,
						    &ioc.ext_clock_mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_external_clock_mode_set",
		  ret);
	}
	break;



    case SWIOC_vtss_ts_external_clock_mode_get:
	{
	    struct vtss_ts_external_clock_mode_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_external_clock_mode_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret =
		    vtss_ts_external_clock_mode_get(chipset,
						    &ioc.ext_clock_mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_external_clock_mode_get",
		  ret);
	}
	break;



    case SWIOC_vtss_ts_ingress_latency_set:
	{
	    struct vtss_ts_ingress_latency_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_ingress_latency_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_ingress_latency_set(chipset, ioc.port_no,
						&ioc.ingress_latency);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_ingress_latency_set",
		  ret);
	}
	break;



    case SWIOC_vtss_ts_ingress_latency_get:
	{
	    struct vtss_ts_ingress_latency_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_ingress_latency_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_ingress_latency_get(chipset, ioc.port_no,
						&ioc.ingress_latency);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_ingress_latency_get",
		  ret);
	}
	break;



    case SWIOC_vtss_ts_p2p_delay_set:
	{
	    struct vtss_ts_p2p_delay_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_p2p_delay_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_p2p_delay_set(chipset, ioc.port_no,
					  &ioc.p2p_delay);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_p2p_delay_set", ret);
	}
	break;



    case SWIOC_vtss_ts_p2p_delay_get:
	{
	    struct vtss_ts_p2p_delay_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_p2p_delay_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_p2p_delay_get(chipset, ioc.port_no,
					  &ioc.p2p_delay);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_p2p_delay_get", ret);
	}
	break;



    case SWIOC_vtss_ts_egress_latency_set:
	{
	    struct vtss_ts_egress_latency_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_egress_latency_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_egress_latency_set(chipset, ioc.port_no,
					       &ioc.egress_latency);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_egress_latency_set", ret);
	}
	break;



    case SWIOC_vtss_ts_egress_latency_get:
	{
	    struct vtss_ts_egress_latency_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_egress_latency_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_egress_latency_get(chipset, ioc.port_no,
					       &ioc.egress_latency);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_egress_latency_get", ret);
	}
	break;



    case SWIOC_vtss_ts_operation_mode_set:
	{
	    struct vtss_ts_operation_mode_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_operation_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_operation_mode_set(chipset, ioc.port_no,
					       &ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_operation_mode_set", ret);
	}
	break;



    case SWIOC_vtss_ts_operation_mode_get:
	{
	    struct vtss_ts_operation_mode_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_ts_operation_mode_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_ts_operation_mode_get(chipset, ioc.port_no,
					       &ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_ts_operation_mode_get", ret);
	}
	break;



    case SWIOC_vtss_tx_timestamp_update:
	{
	    DEBUG("Calling %s\n", "vtss_tx_timestamp_update");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_tx_timestamp_update(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_tx_timestamp_update", ret);
	}
	break;



    case SWIOC_vtss_rx_timestamp_get:
	{
	    struct vtss_rx_timestamp_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_rx_timestamp_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_rx_timestamp_get(chipset, &ioc.ts_id, &ioc.ts);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_rx_timestamp_get", ret);
	}
	break;



    case SWIOC_vtss_rx_master_timestamp_get:
	{
	    struct vtss_rx_master_timestamp_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_rx_master_timestamp_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_rx_master_timestamp_get(chipset, ioc.port_no,
						 &ioc.ts);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_rx_master_timestamp_get",
		  ret);
	}
	break;


#ifdef NOTDEF
    case SWIOC_vtss_tx_timestamp_idx_alloc:
	{
	    struct vtss_tx_timestamp_idx_alloc_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_tx_timestamp_idx_alloc");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_tx_timestamp_idx_alloc(chipset, &ioc.alloc_parm,
						&ioc.ts_id);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_tx_timestamp_idx_alloc",
		  ret);
	}
	break;
#endif				/* NOTDEF */


    case SWIOC_vtss_timestamp_age:
	{
	    DEBUG("Calling %s\n", "vtss_timestamp_age");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_timestamp_age(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_timestamp_age", ret);
	}
	break;


#endif				/* VTSS_FEATURE_TIMESTAMP */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * synce group 
  */

static long
switch_ioctl_synce(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_SYNCE

    case SWIOC_vtss_synce_clock_out_set:
	{
	    struct vtss_synce_clock_out_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_synce_clock_out_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_synce_clock_out_set(chipset, ioc.clk_port,
					     &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_synce_clock_out_set", ret);
	}
	break;



    case SWIOC_vtss_synce_clock_out_get:
	{
	    struct vtss_synce_clock_out_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_synce_clock_out_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_synce_clock_out_get(chipset, ioc.clk_port,
					     &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_synce_clock_out_get", ret);
	}
	break;



    case SWIOC_vtss_synce_clock_in_set:
	{
	    struct vtss_synce_clock_in_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_synce_clock_in_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_synce_clock_in_set(chipset, ioc.clk_port,
					    &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_synce_clock_in_set", ret);
	}
	break;



    case SWIOC_vtss_synce_clock_in_get:
	{
	    struct vtss_synce_clock_in_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_synce_clock_in_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_synce_clock_in_get(chipset, ioc.clk_port,
					    &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_synce_clock_in_get", ret);
	}
	break;


#endif				/* VTSS_FEATURE_SYNCE */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * ccm group 
  */

static long
switch_ioctl_ccm(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {



    case SWIOC_vtssx_ccm_start:
	{
	    struct vtssx_ccm_start_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtssx_ccm_start");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtssx_ccm_start(chipset, ioc.frame, ioc.length,
				    &ioc.opt, &ioc.sess);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtssx_ccm_start", ret);
	}
	break;



    case SWIOC_vtssx_ccm_status_get:
	{
	    struct vtssx_ccm_status_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtssx_ccm_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtssx_ccm_status_get(chipset, ioc.sess, &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtssx_ccm_status_get", ret);
	}
	break;



    case SWIOC_vtssx_ccm_cancel:
	{
	    struct vtssx_ccm_cancel_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtssx_ccm_cancel");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtssx_ccm_cancel(chipset, ioc.sess);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtssx_ccm_cancel", ret);
	}
	break;



    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * l3 group 
  */

static long
switch_ioctl_l3(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_SW_OPTION_L3RT

    case SWIOC_vtss_l3_flush:
	{
	    DEBUG("Calling %s\n", "vtss_l3_flush");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_l3_flush(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_flush", ret);
	}
	break;



    case SWIOC_vtss_l3_rleg_common_get:
	{
	    struct vtss_l3_rleg_common_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_rleg_common_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_l3_rleg_common_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_rleg_common_get", ret);
	}
	break;



    case SWIOC_vtss_l3_rleg_common_set:
	{
	    struct vtss_l3_rleg_common_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_rleg_common_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_rleg_common_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_rleg_common_set", ret);
	}
	break;



    case SWIOC_vtss_l3_rleg_get:
	{
	    struct vtss_l3_rleg_get_ioc ioc, *uioc = (void *) arg;
	    vtss_l3_rleg_conf_t *buf = kmalloc(GFP_KERNEL, sizeof(*buf) * VTSS_RLEG_CNT);	/* Separate IO */
	    DEBUG("Calling %s\n", "vtss_l3_rleg_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_rleg_get(chipset, &ioc.cnt, buf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
		if (!ret && buf) {
		    ret =
			copy_to_user(ioc.buf, buf,
				     sizeof(*buf) * ioc.cnt) ? -EFAULT : 0;
		}
		if (buf)
		    kfree(buf);

	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_rleg_get", ret);
	}
	break;



    case SWIOC_vtss_l3_rleg_add:
	{
	    struct vtss_l3_rleg_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_rleg_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_rleg_add(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_rleg_add", ret);
	}
	break;



    case SWIOC_vtss_l3_rleg_del:
	{
	    struct vtss_l3_rleg_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_rleg_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_rleg_del(chipset, ioc.vlan);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_rleg_del", ret);
	}
	break;



    case SWIOC_vtss_l3_route_get:
	{
	    struct vtss_l3_route_get_ioc ioc, *uioc = (void *) arg;
	    vtss_routing_entry_t *buf = kmalloc(GFP_KERNEL, sizeof(*buf) * VTSS_LPM_CNT);	/* Separate IO */
	    DEBUG("Calling %s\n", "vtss_l3_route_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_route_get(chipset, &ioc.cnt, buf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
		if (!ret && buf) {
		    ret =
			copy_to_user(ioc.buf, buf,
				     sizeof(*buf) * ioc.cnt) ? -EFAULT : 0;
		}
		if (buf)
		    kfree(buf);

	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_route_get", ret);
	}
	break;



    case SWIOC_vtss_l3_route_add:
	{
	    struct vtss_l3_route_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_route_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_route_add(chipset, &ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_route_add", ret);
	}
	break;



    case SWIOC_vtss_l3_route_del:
	{
	    struct vtss_l3_route_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_route_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_route_del(chipset, &ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_route_del", ret);
	}
	break;



    case SWIOC_vtss_l3_neighbour_get:
	{
	    struct vtss_l3_neighbour_get_ioc ioc, *uioc = (void *) arg;
	    vtss_l3_neighbour_t *buf = kmalloc(GFP_KERNEL, sizeof(*buf) * VTSS_ARP_CNT);	/* Separate IO */
	    DEBUG("Calling %s\n", "vtss_l3_neighbour_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_neighbour_get(chipset, &ioc.cnt, buf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
		if (!ret && buf) {
		    ret =
			copy_to_user(ioc.buf, buf,
				     sizeof(*buf) * ioc.cnt) ? -EFAULT : 0;
		}
		if (buf)
		    kfree(buf);

	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_neighbour_get", ret);
	}
	break;



    case SWIOC_vtss_l3_neighbour_add:
	{
	    struct vtss_l3_neighbour_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_neighbour_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_neighbour_add(chipset, &ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_neighbour_add", ret);
	}
	break;



    case SWIOC_vtss_l3_neighbour_del:
	{
	    struct vtss_l3_neighbour_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_neighbour_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_l3_neighbour_del(chipset, &ioc.entry);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_neighbour_del", ret);
	}
	break;



    case SWIOC_vtss_l3_counters_reset:
	{
	    DEBUG("Calling %s\n", "vtss_l3_counters_reset");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_l3_counters_reset(chipset);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_counters_reset", ret);
	}
	break;



    case SWIOC_vtss_l3_counters_system_get:
	{
	    struct vtss_l3_counters_system_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_counters_system_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_l3_counters_system_get(chipset, &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_counters_system_get",
		  ret);
	}
	break;



    case SWIOC_vtss_l3_counters_rleg_get:
	{
	    struct vtss_l3_counters_rleg_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_l3_counters_rleg_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_l3_counters_rleg_get(chipset, ioc.rleg,
					      &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_l3_counters_rleg_get", ret);
	}
	break;


#endif				/* VTSS_SW_OPTION_L3RT */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * mpls group 
  */

static long
switch_ioctl_mpls(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_MPLS

    case SWIOC_vtss_mpls_l2_alloc:
	{
	    struct vtss_mpls_l2_alloc_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_alloc");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mpls_l2_alloc(chipset, &ioc.idx);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_alloc", ret);
	}
	break;



    case SWIOC_vtss_mpls_l2_free:
	{
	    struct vtss_mpls_l2_free_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_free");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_l2_free(chipset, ioc.idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_free", ret);
	}
	break;



    case SWIOC_vtss_mpls_l2_get:
	{
	    struct vtss_mpls_l2_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_l2_get(chipset, ioc.idx, &ioc.l2);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_get", ret);
	}
	break;



    case SWIOC_vtss_mpls_l2_set:
	{
	    struct vtss_mpls_l2_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_l2_set(chipset, ioc.idx, &ioc.l2);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_set", ret);
	}
	break;



    case SWIOC_vtss_mpls_l2_segment_attach:
	{
	    struct vtss_mpls_l2_segment_attach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_segment_attach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_l2_segment_attach(chipset, ioc.idx,
						ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_segment_attach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_l2_segment_detach:
	{
	    struct vtss_mpls_l2_segment_detach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_l2_segment_detach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_l2_segment_detach(chipset, ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_l2_segment_detach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_alloc:
	{
	    struct vtss_mpls_segment_alloc_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_alloc");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_segment_alloc(chipset, ioc.is_in, &ioc.idx);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_alloc", ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_free:
	{
	    struct vtss_mpls_segment_free_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_free");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_segment_free(chipset, ioc.idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_free", ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_get:
	{
	    struct vtss_mpls_segment_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_segment_get(chipset, ioc.idx, &ioc.seg);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_get", ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_set:
	{
	    struct vtss_mpls_segment_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_segment_set(chipset, ioc.idx, &ioc.seg);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_set", ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_state_get:
	{
	    struct vtss_mpls_segment_state_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_state_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_segment_state_get(chipset, ioc.idx,
						&ioc.state);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_state_get",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_server_attach:
	{
	    struct vtss_mpls_segment_server_attach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_server_attach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_segment_server_attach(chipset, ioc.idx,
						    ioc.srv_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_server_attach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_segment_server_detach:
	{
	    struct vtss_mpls_segment_server_detach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_segment_server_detach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_segment_server_detach(chipset, ioc.idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_segment_server_detach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_alloc:
	{
	    struct vtss_mpls_xc_alloc_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_alloc");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_xc_alloc(chipset, ioc.type, &ioc.idx);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_alloc", ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_free:
	{
	    struct vtss_mpls_xc_free_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_free");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_xc_free(chipset, ioc.idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_free", ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_get:
	{
	    struct vtss_mpls_xc_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_xc_get(chipset, ioc.idx, &ioc.xc);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_get", ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_set:
	{
	    struct vtss_mpls_xc_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_xc_set(chipset, ioc.idx, &ioc.xc);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_set", ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_segment_attach:
	{
	    struct vtss_mpls_xc_segment_attach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_segment_attach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_xc_segment_attach(chipset, ioc.xc_idx,
						ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_segment_attach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_segment_detach:
	{
	    struct vtss_mpls_xc_segment_detach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_segment_detach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_xc_segment_detach(chipset, ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_segment_detach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_mc_segment_attach:
	{
	    struct vtss_mpls_xc_mc_segment_attach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_mc_segment_attach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_xc_mc_segment_attach(chipset, ioc.xc_idx,
						   ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_mc_segment_attach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_xc_mc_segment_detach:
	{
	    struct vtss_mpls_xc_mc_segment_detach_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_xc_mc_segment_detach");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_mpls_xc_mc_segment_detach(chipset, ioc.xc_idx,
						   ioc.seg_idx);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_xc_mc_segment_detach",
		  ret);
	}
	break;



    case SWIOC_vtss_mpls_tc_conf_get:
	{
	    struct vtss_mpls_tc_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_tc_conf_get");
	    /* No inputs except ioctl ordinal */
	    if (!ret) {
		ret = vtss_mpls_tc_conf_get(chipset, &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_tc_conf_get", ret);
	}
	break;



    case SWIOC_vtss_mpls_tc_conf_set:
	{
	    struct vtss_mpls_tc_conf_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_mpls_tc_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_mpls_tc_conf_set(chipset, &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_mpls_tc_conf_set", ret);
	}
	break;


#endif				/* VTSS_FEATURE_MPLS */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

 /************************************
  * macsec group 
  */

static long
switch_ioctl_macsec(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (cmd) {

#ifdef VTSS_FEATURE_MACSEC

    case SWIOC_vtss_macsec_init_set:
	{
	    struct vtss_macsec_init_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_init_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_init_set(chipset, ioc.port_no, &ioc.init);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_init_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_init_get:
	{
	    struct vtss_macsec_init_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_init_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_init_get(chipset, ioc.port_no, &ioc.init);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_init_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_conf_add:
	{
	    struct vtss_macsec_secy_conf_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_conf_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_conf_add(chipset, ioc.port,
					      &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_conf_add", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_conf_get:
	{
	    struct vtss_macsec_secy_conf_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_conf_get(chipset, ioc.port,
					      &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_conf_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_conf_del:
	{
	    struct vtss_macsec_secy_conf_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_conf_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_secy_conf_del(chipset, ioc.port);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_conf_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_controlled_set:
	{
	    struct vtss_macsec_secy_controlled_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_controlled_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_controlled_set(chipset, ioc.port,
						    ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_controlled_set",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_controlled_get:
	{
	    struct vtss_macsec_secy_controlled_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_controlled_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_controlled_get(chipset, ioc.port,
						    &ioc.enable);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_controlled_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_controlled_get:
	{
	    struct vtss_macsec_secy_controlled_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_controlled_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_controlled_get(chipset, ioc.port,
						    &ioc.enable);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_controlled_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_port_status_get:
	{
	    struct vtss_macsec_secy_port_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_port_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_port_status_get(chipset, ioc.port,
						     &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_port_status_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sc_add:
	{
	    struct vtss_macsec_rx_sc_add_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sc_add");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_rx_sc_add(chipset, ioc.port, &ioc.sci);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sc_add", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sc_get_next:
	{
	    struct vtss_macsec_rx_sc_get_next_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sc_get_next");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sc_get_next(chipset, ioc.port,
					       &ioc.search_sci,
					       &ioc.found_sci);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sc_get_next", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sc_del:
	{
	    struct vtss_macsec_rx_sc_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sc_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_rx_sc_del(chipset, ioc.port, &ioc.sci);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sc_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sc_status_get:
	{
	    struct vtss_macsec_rx_sc_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sc_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sc_status_get(chipset, ioc.port,
						 &ioc.sci, &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sc_status_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sc_set:
	{
	    struct vtss_macsec_tx_sc_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sc_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_tx_sc_set(chipset, ioc.port);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sc_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sc_del:
	{
	    struct vtss_macsec_tx_sc_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sc_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_tx_sc_del(chipset, ioc.port);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sc_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sc_status_get:
	{
	    struct vtss_macsec_tx_sc_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sc_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sc_status_get(chipset, ioc.port,
						 &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sc_status_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_set:
	{
	    struct vtss_macsec_rx_sa_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_set(chipset, ioc.port, &ioc.sci,
					  ioc.an, ioc.lowest_pn, &ioc.sak);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_get:
	{
	    struct vtss_macsec_rx_sa_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_get(chipset, ioc.port, &ioc.sci,
					  ioc.an, &ioc.lowest_pn, &ioc.sak,
					  &ioc.active);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_activate:
	{
	    struct vtss_macsec_rx_sa_activate_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_activate");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_activate(chipset, ioc.port, &ioc.sci,
					       ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_activate", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_disable:
	{
	    struct vtss_macsec_rx_sa_disable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_disable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_disable(chipset, ioc.port, &ioc.sci,
					      ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_disable", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_del:
	{
	    struct vtss_macsec_rx_sa_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_del(chipset, ioc.port, &ioc.sci,
					  ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_lowest_pn_update:
	{
	    struct vtss_macsec_rx_sa_lowest_pn_update_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_lowest_pn_update");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_lowest_pn_update(chipset, ioc.port,
						       &ioc.sci, ioc.an,
						       ioc.lowest_pn);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_macsec_rx_sa_lowest_pn_update", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_status_get:
	{
	    struct vtss_macsec_rx_sa_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_status_get(chipset, ioc.port,
						 &ioc.sci, ioc.an,
						 &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_status_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_set:
	{
	    struct vtss_macsec_tx_sa_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sa_set(chipset, ioc.port, ioc.an,
					  ioc.next_pn, ioc.confidentiality,
					  &ioc.sak);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_get:
	{
	    struct vtss_macsec_tx_sa_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sa_get(chipset, ioc.port, ioc.an,
					  &ioc.next_pn,
					  &ioc.confidentiality, &ioc.sak,
					  &ioc.active);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_activate:
	{
	    struct vtss_macsec_tx_sa_activate_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_activate");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sa_activate(chipset, ioc.port, ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_activate", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_disable:
	{
	    struct vtss_macsec_tx_sa_disable_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_disable");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_tx_sa_disable(chipset, ioc.port, ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_disable", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_del:
	{
	    struct vtss_macsec_tx_sa_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_tx_sa_del(chipset, ioc.port, ioc.an);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_status_get:
	{
	    struct vtss_macsec_tx_sa_status_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_status_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sa_status_get(chipset, ioc.port, ioc.an,
						 &ioc.status);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_status_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_port_counters_get:
	{
	    struct vtss_macsec_secy_port_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_port_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_port_counters_get(chipset, ioc.port,
						       &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_macsec_secy_port_counters_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_cap_get:
	{
	    struct vtss_macsec_secy_cap_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_cap_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_cap_get(chipset, ioc.port, &ioc.cap);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_cap_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_secy_counters_get:
	{
	    struct vtss_macsec_secy_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_secy_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_secy_counters_get(chipset, ioc.port,
						  &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_secy_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_counters_update:
	{
	    struct vtss_macsec_counters_update_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_counters_update");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_counters_update(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_counters_update",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_counters_clear:
	{
	    struct vtss_macsec_counters_clear_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_counters_clear");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret = vtss_macsec_counters_clear(chipset, ioc.port_no);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_counters_clear", ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sc_counters_get:
	{
	    struct vtss_macsec_rx_sc_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sc_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sc_counters_get(chipset, ioc.port,
						   &ioc.sci,
						   &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sc_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sc_counters_get:
	{
	    struct vtss_macsec_tx_sc_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sc_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sc_counters_get(chipset, ioc.port,
						   &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sc_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_tx_sa_counters_get:
	{
	    struct vtss_macsec_tx_sa_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_tx_sa_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_tx_sa_counters_get(chipset, ioc.port,
						   ioc.an, &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_tx_sa_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_rx_sa_counters_get:
	{
	    struct vtss_macsec_rx_sa_counters_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_rx_sa_counters_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_rx_sa_counters_get(chipset, ioc.port,
						   &ioc.sci, ioc.an,
						   &ioc.counters);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_rx_sa_counters_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_control_frame_match_conf_set:
	{
	    struct vtss_macsec_control_frame_match_conf_set_ioc ioc,
		*uioc = (void *) arg;
	    DEBUG("Calling %s\n",
		  "vtss_macsec_control_frame_match_conf_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_control_frame_match_conf_set(chipset,
							     ioc.port,
							     &ioc.conf);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_macsec_control_frame_match_conf_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_control_frame_match_conf_get:
	{
	    struct vtss_macsec_control_frame_match_conf_get_ioc ioc,
		*uioc = (void *) arg;
	    DEBUG("Calling %s\n",
		  "vtss_macsec_control_frame_match_conf_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_control_frame_match_conf_get(chipset,
							     ioc.port,
							     &ioc.conf);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n",
		  "vtss_macsec_control_frame_match_conf_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_pattern_set:
	{
	    struct vtss_macsec_pattern_set_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_pattern_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_pattern_set(chipset, ioc.port,
					    ioc.direction, ioc.action,
					    &ioc.pattern);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_pattern_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_pattern_del:
	{
	    struct vtss_macsec_pattern_del_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_pattern_del");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_pattern_del(chipset, ioc.port,
					    ioc.direction, ioc.action);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_pattern_del", ret);
	}
	break;



    case SWIOC_vtss_macsec_pattern_get:
	{
	    struct vtss_macsec_pattern_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_pattern_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_pattern_get(chipset, ioc.port,
					    ioc.direction, ioc.action,
					    &ioc.pattern);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_pattern_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_default_action_set:
	{
	    struct vtss_macsec_default_action_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_default_action_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_default_action_set(chipset, ioc.port,
						   &ioc.pattern);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_default_action_set",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_default_action_get:
	{
	    struct vtss_macsec_default_action_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_default_action_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_default_action_get(chipset, ioc.port,
						   &ioc.pattern);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_default_action_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_bypass_mode_set:
	{
	    struct vtss_macsec_bypass_mode_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_bypass_mode_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_bypass_mode_set(chipset, ioc.port_no,
						ioc.mode);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_bypass_mode_set",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_bypass_mode_get:
	{
	    struct vtss_macsec_bypass_mode_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_bypass_mode_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_bypass_mode_get(chipset, ioc.port_no,
						&ioc.mode);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_bypass_mode_get",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_bypass_tag_set:
	{
	    struct vtss_macsec_bypass_tag_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_bypass_tag_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_bypass_tag_set(chipset, ioc.port, ioc.tag);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_bypass_tag_set", ret);
	}
	break;



    case SWIOC_vtss_macsec_bypass_tag_get:
	{
	    struct vtss_macsec_bypass_tag_get_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_bypass_tag_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_bypass_tag_get(chipset, ioc.port,
					       &ioc.tag);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_bypass_tag_get", ret);
	}
	break;



    case SWIOC_vtss_macsec_frame_capture_set:
	{
	    struct vtss_macsec_frame_capture_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_frame_capture_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_frame_capture_set(chipset, ioc.port_no,
						  ioc.capture);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_frame_capture_set",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_port_loopback_set:
	{
	    struct vtss_macsec_port_loopback_set_ioc ioc, *uioc =
		(void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_port_loopback_set");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_port_loopback_set(chipset, ioc.port_no,
						  ioc.enable);
		/* No outputs except return value */
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_port_loopback_set",
		  ret);
	}
	break;



    case SWIOC_vtss_macsec_frame_get:
	{
	    struct vtss_macsec_frame_get_ioc ioc, *uioc = (void *) arg;
	    DEBUG("Calling %s\n", "vtss_macsec_frame_get");
	    ret = copy_from_user(&ioc, uioc, sizeof(ioc)) ? -EFAULT : 0;
	    if (!ret) {
		ret =
		    vtss_macsec_frame_get(chipset, ioc.port_no,
					  ioc.buf_length,
					  &ioc.return_length, &ioc.frame);
		ret =
		    copy_to_user(uioc, &ioc, sizeof(ioc)) ? -EFAULT : ret;
	    }
	    DEBUG("Done %s - ret %d\n", "vtss_macsec_frame_get", ret);
	}
	break;


#endif				/* VTSS_FEATURE_MACSEC */
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}


static long
switch_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch (_IOC_TYPE(cmd)) {
    case 'A':
	ret = switch_ioctl_port(filp, cmd, arg);
	break;
    case 'B':
	ret = switch_ioctl_misc(filp, cmd, arg);
	break;
    case 'C':
	ret = switch_ioctl_init(filp, cmd, arg);
	break;
    case 'D':
	ret = switch_ioctl_phy(filp, cmd, arg);
	break;
    case 'E':
	ret = switch_ioctl_10gphy(filp, cmd, arg);
	break;
    case 'F':
	ret = switch_ioctl_qos(filp, cmd, arg);
	break;
    case 'G':
	ret = switch_ioctl_packet(filp, cmd, arg);
	break;
    case 'H':
	ret = switch_ioctl_security(filp, cmd, arg);
	break;
    case 'I':
	ret = switch_ioctl_layer2(filp, cmd, arg);
	break;
    case 'J':
	ret = switch_ioctl_evc(filp, cmd, arg);
	break;
    case 'K':
	ret = switch_ioctl_qos_policer_dlb(filp, cmd, arg);
	break;
    case 'L':
	ret = switch_ioctl_timestamp(filp, cmd, arg);
	break;
    case 'M':
	ret = switch_ioctl_synce(filp, cmd, arg);
	break;
    case 'N':
	ret = switch_ioctl_ccm(filp, cmd, arg);
	break;
    case 'O':
	ret = switch_ioctl_l3(filp, cmd, arg);
	break;
    case 'P':
	ret = switch_ioctl_mpls(filp, cmd, arg);
	break;
    case 'Q':
	ret = switch_ioctl_macsec(filp, cmd, arg);
	break;
    default:
	printk(KERN_ERR "Invalid ioctl(%x)\n", cmd);
	ret = -ENOIOCTLCMD;
    }

    return (long) ret;
}

EXPORT_SYMBOL(switch_ioctl);

static int __init vtss_ioctl_init(void)
{
    vtss_ioctl_ext_init();

    if (misc_register(&switch_dev))
	printk(KERN_ERR "switch: can't misc_register on minor %d\n",
	       switch_dev.minor);

    return 0;
}

static void vtss_ioctl_exit(void)
{
    vtss_ioctl_ext_exit();

    misc_deregister(&switch_dev);

    printk(KERN_NOTICE "Uninstalled\n");
}

module_init(vtss_ioctl_init);
module_exit(vtss_ioctl_exit);

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug");

MODULE_AUTHOR("Lars Povlsen <lpovlsen@vitesse.com>");
MODULE_DESCRIPTION("Vitesse Gigabit Switch Core Module");
MODULE_LICENSE("(c) Vitesse Semiconductor Inc.");
