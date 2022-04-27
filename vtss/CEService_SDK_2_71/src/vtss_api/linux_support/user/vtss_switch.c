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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include "vtss_switch_usermode.h"

static int fd;
int __portfd;

void __attribute__ ((constructor)) __vtss_switch_load(void);
void __attribute__ ((destructor)) __vtss_switch_unload(void);

// Called when the library is loaded and before dlopen() returns
void __vtss_switch_load(void)
{
    if ((fd = open("/dev/switch", O_RDWR)) < 0) {
	perror("No kernel interface: open(/dev/switch)");
	exit(-1);
    }
    if ((__portfd = open("/dev/swport", O_RDWR)) < 0) {
	perror("No kernel interface: open(/dev/swport)");
	exit(-1);
    }
}

// Called when the library is unloaded and before dlclose()
// returns
void __vtss_switch_unload(void)
{
    close(fd);
    close(__portfd);
}

 /************************************
  * port group 
  */



vtss_rc vtss_port_map_set(const vtss_inst_t inst,
			  const vtss_port_map_t
			  port_map[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_port_map_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.port_map, port_map,
	   sizeof(port_map[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_map_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_map_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_map_get(const vtss_inst_t inst,
			  vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_port_map_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_port_map_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_map_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(port_map, ioc.port_map,
	   sizeof(port_map[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_10G
vtss_rc vtss_port_mmd_read(const vtss_inst_t inst,
			   const vtss_port_no_t port_no, const u8 mmd,
			   const u16 addr, u16 * value)
{
    struct vtss_port_mmd_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mmd = mmd;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_mmd_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_mmd_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
vtss_rc vtss_port_mmd_write(const vtss_inst_t inst,
			    const vtss_port_no_t port_no, const u8 mmd,
			    const u16 addr, const u16 value)
{
    struct vtss_port_mmd_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mmd = mmd;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_mmd_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_mmd_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
vtss_rc vtss_port_mmd_masked_write(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const u8 mmd, const u16 addr,
				   const u16 value, const u16 mask)
{
    struct vtss_port_mmd_masked_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mmd = mmd;
    ioc.addr = addr;
    ioc.value = value;
    ioc.mask = mask;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_mmd_masked_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_mmd_masked_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
vtss_rc vtss_mmd_read(const vtss_inst_t inst, const vtss_chip_no_t chip_no,
		      const vtss_miim_controller_t miim_controller,
		      const u8 miim_addr, const u8 mmd, const u16 addr,
		      u16 * value)
{
    struct vtss_mmd_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.miim_controller = miim_controller;
    ioc.miim_addr = miim_addr;
    ioc.mmd = mmd;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mmd_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mmd_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_10G
vtss_rc vtss_mmd_write(const vtss_inst_t inst,
		       const vtss_chip_no_t chip_no,
		       const vtss_miim_controller_t miim_controller,
		       const u8 miim_addr, const u8 mmd, const u16 addr,
		       const u16 value)
{
    struct vtss_mmd_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.miim_controller = miim_controller;
    ioc.miim_addr = miim_addr;
    ioc.mmd = mmd;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mmd_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mmd_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_10G */

#ifdef VTSS_FEATURE_CLAUSE_37
vtss_rc vtss_port_clause_37_control_get(const vtss_inst_t inst,
					const vtss_port_no_t port_no,
					vtss_port_clause_37_control_t *
					control)
{
    struct vtss_port_clause_37_control_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_port_clause_37_control_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_clause_37_control_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *control = ioc.control;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_CLAUSE_37 */

#ifdef VTSS_FEATURE_CLAUSE_37
vtss_rc vtss_port_clause_37_control_set(const vtss_inst_t inst,
					const vtss_port_no_t port_no,
					const vtss_port_clause_37_control_t
					* control)
{
    struct vtss_port_clause_37_control_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.control = *control;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_port_clause_37_control_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_clause_37_control_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_CLAUSE_37 */


vtss_rc vtss_port_conf_set(const vtss_inst_t inst,
			   const vtss_port_no_t port_no,
			   const vtss_port_conf_t * conf)
{
    struct vtss_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_conf_get(const vtss_inst_t inst,
			   const vtss_port_no_t port_no,
			   vtss_port_conf_t * conf)
{
    struct vtss_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_status_get(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     vtss_port_status_t * status)
{
    struct vtss_port_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_counters_update(const vtss_inst_t inst,
				  const vtss_port_no_t port_no)
{
    struct vtss_port_counters_update_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_counters_update, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_counters_update", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_counters_clear(const vtss_inst_t inst,
				 const vtss_port_no_t port_no)
{
    struct vtss_port_counters_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_counters_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_counters_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_counters_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_port_counters_t * counters)
{
    struct vtss_port_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_basic_counters_get(const vtss_inst_t inst,
				     const vtss_port_no_t port_no,
				     vtss_basic_counters_t * counters)
{
    struct vtss_port_basic_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_basic_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_basic_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_forward_state_get(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    vtss_port_forward_t * forward)
{
    struct vtss_port_forward_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_forward_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_forward_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *forward = ioc.forward;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_forward_state_set(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    const vtss_port_forward_t forward)
{
    struct vtss_port_forward_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.forward = forward;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_forward_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_forward_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_miim_read(const vtss_inst_t inst,
		       const vtss_chip_no_t chip_no,
		       const vtss_miim_controller_t miim_controller,
		       const u8 miim_addr, const u8 addr, u16 * value)
{
    struct vtss_miim_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.miim_controller = miim_controller;
    ioc.miim_addr = miim_addr;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_miim_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_miim_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_miim_write(const vtss_inst_t inst,
			const vtss_chip_no_t chip_no,
			const vtss_miim_controller_t miim_controller,
			const u8 miim_addr, const u8 addr, const u16 value)
{
    struct vtss_miim_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.miim_controller = miim_controller;
    ioc.miim_addr = miim_addr;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_miim_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_miim_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}




 /************************************
  * misc group 
  */



vtss_rc vtss_debug_info_print(const vtss_inst_t inst,
			      const vtss_debug_printf_t prntf,
			      const vtss_debug_info_t * info)
{
    struct vtss_debug_info_print_ioc ioc;
    int ret;
    /* Input params start */
    ioc.info = *info;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_debug_info_print, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_debug_info_print", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_reg_read(const vtss_inst_t inst, const vtss_chip_no_t chip_no,
		      const u32 addr, u32 * value)
{
    struct vtss_reg_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_reg_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_reg_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_reg_write(const vtss_inst_t inst,
		       const vtss_chip_no_t chip_no, const u32 addr,
		       const u32 value)
{
    struct vtss_reg_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_reg_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_reg_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_reg_write_masked(const vtss_inst_t inst,
			      const vtss_chip_no_t chip_no, const u32 addr,
			      const u32 value, const u32 mask)
{
    struct vtss_reg_write_masked_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.addr = addr;
    ioc.value = value;
    ioc.mask = mask;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_reg_write_masked, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_reg_write_masked", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_chip_id_get(const vtss_inst_t inst, vtss_chip_id_t * chip_id)
{
    struct vtss_chip_id_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_chip_id_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_chip_id_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *chip_id = ioc.chip_id;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_poll_1sec(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_poll_1sec, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_poll_1sec", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ptp_event_poll(const vtss_inst_t inst,
			    vtss_ptp_event_type_t * ev_mask)
{
    struct vtss_ptp_event_poll_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ptp_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ptp_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ptp_event_enable(const vtss_inst_t inst,
			      const vtss_ptp_event_type_t ev_mask,
			      const BOOL enable)
{
    struct vtss_ptp_event_enable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ev_mask = ev_mask;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ptp_event_enable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ptp_event_enable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_dev_all_event_poll(const vtss_inst_t inst,
				const vtss_dev_all_event_poll_t poll_type,
				vtss_dev_all_event_type_t * ev_mask)
{
    struct vtss_dev_all_event_poll_ioc ioc;
    int ret;
    /* Input params start */
    ioc.poll_type = poll_type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_dev_all_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_dev_all_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_dev_all_event_enable(const vtss_inst_t inst,
				  const vtss_port_no_t port,
				  const vtss_dev_all_event_type_t ev_mask,
				  const BOOL enable)
{
    struct vtss_dev_all_event_enable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.ev_mask = ev_mask;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_dev_all_event_enable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_dev_all_event_enable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_mode_set(const vtss_inst_t inst,
			   const vtss_chip_no_t chip_no,
			   const vtss_gpio_no_t gpio_no,
			   const vtss_gpio_mode_t mode)
{
    struct vtss_gpio_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.gpio_no = gpio_no;
    ioc.mode = mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_direction_set(const vtss_inst_t inst,
				const vtss_chip_no_t chip_no,
				const vtss_gpio_no_t gpio_no,
				const BOOL output)
{
    struct vtss_gpio_direction_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.gpio_no = gpio_no;
    ioc.output = output;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_direction_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_direction_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_read(const vtss_inst_t inst,
		       const vtss_chip_no_t chip_no,
		       const vtss_gpio_no_t gpio_no, BOOL * value)
{
    struct vtss_gpio_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.gpio_no = gpio_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_write(const vtss_inst_t inst,
			const vtss_chip_no_t chip_no,
			const vtss_gpio_no_t gpio_no, const BOOL value)
{
    struct vtss_gpio_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.gpio_no = gpio_no;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_event_poll(const vtss_inst_t inst,
			     const vtss_chip_no_t chip_no, BOOL * events)
{
    struct vtss_gpio_event_poll_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *events = ioc.events;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_gpio_event_enable(const vtss_inst_t inst,
			       const vtss_chip_no_t chip_no,
			       const vtss_gpio_no_t gpio_no,
			       const BOOL enable)
{
    struct vtss_gpio_event_enable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.gpio_no = gpio_no;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_event_enable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_event_enable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_ARCH_B2
vtss_rc vtss_gpio_clk_set(const vtss_inst_t inst,
			  const vtss_gpio_no_t gpio_no,
			  const vtss_port_no_t port_no,
			  const vtss_recovered_clock_t clk)
{
    struct vtss_gpio_clk_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.gpio_no = gpio_no;
    ioc.port_no = port_no;
    ioc.clk = clk;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_gpio_clk_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_gpio_clk_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_FEATURE_SERIAL_LED
vtss_rc vtss_serial_led_set(const vtss_inst_t inst,
			    const vtss_led_port_t port,
			    const vtss_led_mode_t mode[3])
{
    struct vtss_serial_led_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    memcpy(ioc.mode, mode, sizeof(mode[0]) * 3);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_serial_led_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_serial_led_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_LED
vtss_rc vtss_serial_led_intensity_set(const vtss_inst_t inst,
				      const vtss_led_port_t port,
				      const i8 intensity)
{
    struct vtss_serial_led_intensity_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.intensity = intensity;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_serial_led_intensity_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_serial_led_intensity_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_LED
vtss_rc vtss_serial_led_intensity_get(const vtss_inst_t inst,
				      i8 * intensity)
{
    struct vtss_serial_led_intensity_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_serial_led_intensity_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_serial_led_intensity_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *intensity = ioc.intensity;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_LED */

#ifdef VTSS_FEATURE_SERIAL_GPIO
vtss_rc vtss_sgpio_conf_get(const vtss_inst_t inst,
			    const vtss_chip_no_t chip_no,
			    const vtss_sgpio_group_t group,
			    vtss_sgpio_conf_t * conf)
{
    struct vtss_sgpio_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.group = group;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sgpio_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sgpio_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
vtss_rc vtss_sgpio_conf_set(const vtss_inst_t inst,
			    const vtss_chip_no_t chip_no,
			    const vtss_sgpio_group_t group,
			    const vtss_sgpio_conf_t * conf)
{
    struct vtss_sgpio_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.group = group;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sgpio_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sgpio_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
vtss_rc vtss_sgpio_read(const vtss_inst_t inst,
			const vtss_chip_no_t chip_no,
			const vtss_sgpio_group_t group,
			vtss_sgpio_port_data_t data[VTSS_SGPIO_PORTS])
{
    struct vtss_sgpio_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.group = group;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sgpio_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sgpio_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(data, ioc.data, sizeof(data[0]) * VTSS_SGPIO_PORTS);
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
vtss_rc vtss_sgpio_event_poll(const vtss_inst_t inst,
			      const vtss_chip_no_t chip_no,
			      const vtss_sgpio_group_t group,
			      const u32 bit, BOOL events[VTSS_SGPIO_PORTS])
{
    struct vtss_sgpio_event_poll_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.group = group;
    ioc.bit = bit;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sgpio_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sgpio_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(events, ioc.events, sizeof(events[0]) * VTSS_SGPIO_PORTS);
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
vtss_rc vtss_sgpio_event_enable(const vtss_inst_t inst,
				const vtss_chip_no_t chip_no,
				const vtss_sgpio_group_t group,
				const u32 port, const u32 bit,
				const BOOL enable)
{
    struct vtss_sgpio_event_enable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.group = group;
    ioc.port = port;
    ioc.bit = bit;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sgpio_event_enable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sgpio_event_enable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SERIAL_GPIO */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_temp_sensor_init(const vtss_inst_t inst, const BOOL enable)
{
    struct vtss_temp_sensor_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_temp_sensor_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_temp_sensor_init", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_temp_sensor_get(const vtss_inst_t inst, i16 * temperature)
{
    struct vtss_temp_sensor_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_temp_sensor_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_temp_sensor_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *temperature = ioc.temperature;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_fan_rotation_get(const vtss_inst_t inst,
			      vtss_fan_conf_t * fan_spec,
			      u32 * rotation_count)
{
    struct vtss_fan_rotation_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_fan_rotation_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_fan_rotation_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *fan_spec = ioc.fan_spec;
    *rotation_count = ioc.rotation_count;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_fan_cool_lvl_set(const vtss_inst_t inst, const u8 lvl)
{
    struct vtss_fan_cool_lvl_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.lvl = lvl;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_fan_cool_lvl_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_fan_cool_lvl_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_fan_controller_init(const vtss_inst_t inst,
				 const vtss_fan_conf_t * spec)
{
    struct vtss_fan_controller_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.spec = *spec;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_fan_controller_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_fan_controller_init", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_FAN
vtss_rc vtss_fan_cool_lvl_get(const vtss_inst_t inst, u8 * lvl)
{
    struct vtss_fan_cool_lvl_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_fan_cool_lvl_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_fan_cool_lvl_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *lvl = ioc.lvl;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_FAN */

#ifdef VTSS_FEATURE_EEE
vtss_rc vtss_eee_port_conf_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_eee_port_conf_t * conf)
{
    struct vtss_eee_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eee_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eee_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_FEATURE_EEE
vtss_rc vtss_eee_port_state_set(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_eee_port_state_t * conf)
{
    struct vtss_eee_port_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eee_port_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eee_port_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_FEATURE_EEE
vtss_rc vtss_eee_port_counter_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_eee_port_counter_t * conf)
{
    struct vtss_eee_port_counter_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eee_port_counter_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eee_port_counter_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_EEE */



 /************************************
  * init group 
  */


#ifdef VTSS_FEATURE_WARM_START
vtss_rc vtss_restart_status_get(const vtss_inst_t inst,
				vtss_restart_status_t * status)
{
    struct vtss_restart_status_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_restart_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_restart_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_WARM_START */

#ifdef VTSS_FEATURE_WARM_START
vtss_rc vtss_restart_conf_get(const vtss_inst_t inst,
			      vtss_restart_t * restart)
{
    struct vtss_restart_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_restart_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_restart_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *restart = ioc.restart;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_WARM_START */

#ifdef VTSS_FEATURE_WARM_START
vtss_rc vtss_restart_conf_set(const vtss_inst_t inst,
			      const vtss_restart_t restart)
{
    struct vtss_restart_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.restart = restart;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_restart_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_restart_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_WARM_START */



 /************************************
  * phy group 
  */



vtss_rc vtss_phy_pre_reset(const vtss_inst_t inst,
			   const vtss_port_no_t port_no)
{
    struct vtss_phy_pre_reset_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_pre_reset, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_pre_reset", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_post_reset(const vtss_inst_t inst,
			    const vtss_port_no_t port_no)
{
    struct vtss_phy_post_reset_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_post_reset, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_post_reset", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_reset(const vtss_inst_t inst,
		       const vtss_port_no_t port_no,
		       const vtss_phy_reset_conf_t * conf)
{
    struct vtss_phy_reset_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_reset, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_reset", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_chip_temp_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no, i16 * temp)
{
    struct vtss_phy_chip_temp_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_chip_temp_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_chip_temp_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *temp = ioc.temp;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_chip_temp_init(const vtss_inst_t inst,
				const vtss_port_no_t port_no)
{
    struct vtss_phy_chip_temp_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_chip_temp_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_chip_temp_init", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_conf_get(const vtss_inst_t inst,
			  const vtss_port_no_t port_no,
			  vtss_phy_conf_t * conf)
{
    struct vtss_phy_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_conf_set(const vtss_inst_t inst,
			  const vtss_port_no_t port_no,
			  const vtss_phy_conf_t * conf)
{
    struct vtss_phy_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_status_get(const vtss_inst_t inst,
			    const vtss_port_no_t port_no,
			    vtss_port_status_t * status)
{
    struct vtss_phy_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_conf_1g_get(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     vtss_phy_conf_1g_t * conf)
{
    struct vtss_phy_conf_1g_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_conf_1g_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_conf_1g_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_conf_1g_set(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     const vtss_phy_conf_1g_t * conf)
{
    struct vtss_phy_conf_1g_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_conf_1g_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_conf_1g_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_status_1g_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_phy_status_1g_t * status)
{
    struct vtss_phy_status_1g_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_status_1g_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_status_1g_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_power_conf_get(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				vtss_phy_power_conf_t * conf)
{
    struct vtss_phy_power_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_power_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_power_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_power_conf_set(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_phy_power_conf_t * conf)
{
    struct vtss_phy_power_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_power_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_power_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_power_status_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_phy_power_status_t * status)
{
    struct vtss_phy_power_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_power_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_power_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_clock_conf_set(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_phy_recov_clk_t clock_port,
				const vtss_phy_clock_conf_t * conf)
{
    struct vtss_phy_clock_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.clock_port = clock_port;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_clock_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_clock_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_read(const vtss_inst_t inst, const vtss_port_no_t port_no,
		      const u32 addr, u16 * value)
{
    struct vtss_phy_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_write(const vtss_inst_t inst,
		       const vtss_port_no_t port_no, const u32 addr,
		       const u16 value)
{
    struct vtss_phy_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_mmd_read(const vtss_inst_t inst,
			  const vtss_port_no_t port_no, const u16 devad,
			  const u32 addr, u16 * value)
{
    struct vtss_phy_mmd_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.devad = devad;
    ioc.addr = addr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_mmd_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_mmd_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_mmd_write(const vtss_inst_t inst,
			   const vtss_port_no_t port_no, const u16 devad,
			   const u32 addr, const u16 value)
{
    struct vtss_phy_mmd_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.devad = devad;
    ioc.addr = addr;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_mmd_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_mmd_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_write_masked(const vtss_inst_t inst,
			      const vtss_port_no_t port_no, const u32 addr,
			      const u16 value, const u16 mask)
{
    struct vtss_phy_write_masked_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.addr = addr;
    ioc.value = value;
    ioc.mask = mask;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_write_masked, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_write_masked", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_write_masked_page(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const u16 page, const u16 addr,
				   const u16 value, const u16 mask)
{
    struct vtss_phy_write_masked_page_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.page = page;
    ioc.addr = addr;
    ioc.value = value;
    ioc.mask = mask;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_write_masked_page, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_write_masked_page", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_veriphy_start(const vtss_inst_t inst,
			       const vtss_port_no_t port_no, const u8 mode)
{
    struct vtss_phy_veriphy_start_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mode = mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_veriphy_start, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_veriphy_start", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_veriphy_get(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     vtss_phy_veriphy_result_t * result)
{
    struct vtss_phy_veriphy_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_veriphy_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_veriphy_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *result = ioc.result;
    /* Output params end */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_LED_POW_REDUC
vtss_rc vtss_phy_led_intensity_set(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_phy_led_intensity intensity)
{
    struct vtss_phy_led_intensity_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.intensity = intensity;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_led_intensity_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_led_intensity_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_LED_POW_REDUC */

#ifdef VTSS_FEATURE_LED_POW_REDUC
vtss_rc vtss_phy_enhanced_led_control_init(const vtss_inst_t inst,
					   const vtss_port_no_t port_no,
					   const
					   vtss_phy_enhanced_led_control_t
					   conf)
{
    struct vtss_phy_enhanced_led_control_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = conf;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_phy_enhanced_led_control_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_enhanced_led_control_init",
	       ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_LED_POW_REDUC */


vtss_rc vtss_phy_coma_mode_disable(const vtss_inst_t inst,
				   const vtss_port_no_t port_no)
{
    struct vtss_phy_coma_mode_disable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_coma_mode_disable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_coma_mode_disable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_EEE
vtss_rc vtss_phy_eee_ena(const vtss_inst_t inst,
			 const vtss_port_no_t port_no, const BOOL enable)
{
    struct vtss_phy_eee_ena_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_eee_ena, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_eee_ena", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_EEE */

#ifdef VTSS_CHIP_CU_PHY
vtss_rc vtss_phy_event_enable_set(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  const vtss_phy_event_t ev_mask,
				  const BOOL enable)
{
    struct vtss_phy_event_enable_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.ev_mask = ev_mask;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_event_enable_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_event_enable_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_CHIP_CU_PHY */

#ifdef VTSS_CHIP_CU_PHY
vtss_rc vtss_phy_event_enable_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_phy_event_t * ev_mask)
{
    struct vtss_phy_event_enable_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_event_enable_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_event_enable_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_CHIP_CU_PHY */

#ifdef VTSS_CHIP_CU_PHY
vtss_rc vtss_phy_event_poll(const vtss_inst_t inst,
			    const vtss_port_no_t port_no,
			    vtss_phy_event_t * ev_mask)
{
    struct vtss_phy_event_poll_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_CHIP_CU_PHY */



 /************************************
  * 10gphy group 
  */

#ifdef VTSS_FEATURE_10G

vtss_rc vtss_phy_10g_mode_get(const vtss_inst_t inst,
			      const vtss_port_no_t port_no,
			      vtss_phy_10g_mode_t * mode)
{
    struct vtss_phy_10g_mode_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_mode_set(const vtss_inst_t inst,
			      const vtss_port_no_t port_no,
			      const vtss_phy_10g_mode_t * mode)
{
    struct vtss_phy_10g_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mode = *mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_synce_clkout_get(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      BOOL * synce_clkout)
{
    struct vtss_phy_10g_synce_clkout_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_synce_clkout_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_synce_clkout_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *synce_clkout = ioc.synce_clkout;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_synce_clkout_set(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const BOOL synce_clkout)
{
    struct vtss_phy_10g_synce_clkout_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.synce_clkout = synce_clkout;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_synce_clkout_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_synce_clkout_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_xfp_clkout_get(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    BOOL * xfp_clkout)
{
    struct vtss_phy_10g_xfp_clkout_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_xfp_clkout_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_xfp_clkout_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *xfp_clkout = ioc.xfp_clkout;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_xfp_clkout_set(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    const BOOL xfp_clkout)
{
    struct vtss_phy_10g_xfp_clkout_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.xfp_clkout = xfp_clkout;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_xfp_clkout_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_xfp_clkout_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_status_get(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				vtss_phy_10g_status_t * status)
{
    struct vtss_phy_10g_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_reset(const vtss_inst_t inst,
			   const vtss_port_no_t port_no)
{
    struct vtss_phy_10g_reset_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_reset, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_reset", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_loopback_set(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  const vtss_phy_10g_loopback_t * loopback)
{
    struct vtss_phy_10g_loopback_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.loopback = *loopback;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_loopback_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_loopback_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_loopback_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_phy_10g_loopback_t * loopback)
{
    struct vtss_phy_10g_loopback_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_loopback_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_loopback_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *loopback = ioc.loopback;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_cnt_get(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     vtss_phy_10g_cnt_t * cnt)
{
    struct vtss_phy_10g_cnt_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_cnt_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_cnt_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cnt = ioc.cnt;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_power_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_phy_10g_power_t * power)
{
    struct vtss_phy_10g_power_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_power_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_power_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *power = ioc.power;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_power_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_phy_10g_power_t * power)
{
    struct vtss_phy_10g_power_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.power = *power;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_power_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_power_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_failover_set(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_phy_10g_failover_mode_t * mode)
{
    struct vtss_phy_10g_failover_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_failover_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_failover_set", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_failover_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_phy_10g_failover_mode_t * mode)
{
    struct vtss_phy_10g_failover_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_failover_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_failover_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_id_get(const vtss_inst_t inst,
			    const vtss_port_no_t port_no,
			    vtss_phy_10g_id_t * phy_id)
{
    struct vtss_phy_10g_id_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_id_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_id_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *phy_id = ioc.phy_id;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_gpio_mode_set(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_gpio_10g_no_t gpio_no,
				   const vtss_gpio_10g_gpio_mode_t * mode)
{
    struct vtss_phy_10g_gpio_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.gpio_no = gpio_no;
    ioc.mode = *mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_gpio_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_gpio_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_gpio_mode_get(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_gpio_10g_no_t gpio_no,
				   vtss_gpio_10g_gpio_mode_t * mode)
{
    struct vtss_phy_10g_gpio_mode_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.gpio_no = gpio_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_gpio_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_gpio_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_gpio_read(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_gpio_10g_no_t gpio_no,
			       BOOL * value)
{
    struct vtss_phy_10g_gpio_read_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.gpio_no = gpio_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_gpio_read, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_gpio_read", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *value = ioc.value;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_gpio_write(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_gpio_10g_no_t gpio_no,
				const BOOL value)
{
    struct vtss_phy_10g_gpio_write_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.gpio_no = gpio_no;
    ioc.value = value;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_gpio_write, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_gpio_write", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_event_enable_set(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const vtss_phy_10g_event_t ev_mask,
				      const BOOL enable)
{
    struct vtss_phy_10g_event_enable_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.ev_mask = ev_mask;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_event_enable_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_event_enable_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_event_enable_get(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      vtss_phy_10g_event_t * ev_mask)
{
    struct vtss_phy_10g_event_enable_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_event_enable_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_event_enable_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_event_poll(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				vtss_phy_10g_event_t * ev_mask)
{
    struct vtss_phy_10g_event_poll_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_event_poll, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_event_poll", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ev_mask = ioc.ev_mask;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_poll_1sec(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_poll_1sec, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_poll_1sec", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_phy_10g_edc_fw_status_get(const vtss_inst_t inst,
				       const vtss_port_no_t port_no,
				       vtss_phy_10g_fw_status_t * status)
{
    struct vtss_phy_10g_edc_fw_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_phy_10g_edc_fw_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_phy_10g_edc_fw_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_10G */

 /************************************
  * qos group 
  */

#ifdef VTSS_FEATURE_QOS
#ifdef VTSS_ARCH_CARACAL
vtss_rc vtss_mep_policer_conf_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  const vtss_prio_t prio,
				  vtss_dlb_policer_conf_t * conf)
{
    struct vtss_mep_policer_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.prio = prio;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mep_policer_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mep_policer_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
vtss_rc vtss_mep_policer_conf_set(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  const vtss_prio_t prio,
				  const vtss_dlb_policer_conf_t * conf)
{
    struct vtss_mep_policer_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.prio = prio;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mep_policer_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mep_policer_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_CARACAL */


vtss_rc vtss_qos_conf_get(const vtss_inst_t inst, vtss_qos_conf_t * conf)
{
    struct vtss_qos_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_qos_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qos_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_qos_conf_set(const vtss_inst_t inst,
			  const vtss_qos_conf_t * conf)
{
    struct vtss_qos_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qos_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qos_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_qos_port_conf_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_qos_port_conf_t * conf)
{
    struct vtss_qos_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qos_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qos_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_qos_port_conf_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_qos_port_conf_t * conf)
{
    struct vtss_qos_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qos_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qos_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_QCL
vtss_rc vtss_qce_init(const vtss_inst_t inst, const vtss_qce_type_t type,
		      vtss_qce_t * qce)
{
    struct vtss_qce_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.type = type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qce_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qce_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *qce = ioc.qce;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_QCL */

#ifdef VTSS_FEATURE_QCL
vtss_rc vtss_qce_add(const vtss_inst_t inst, const vtss_qcl_id_t qcl_id,
		     const vtss_qce_id_t qce_id, const vtss_qce_t * qce)
{
    struct vtss_qce_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.qcl_id = qcl_id;
    ioc.qce_id = qce_id;
    ioc.qce = *qce;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qce_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qce_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_QCL */

#ifdef VTSS_FEATURE_QCL
vtss_rc vtss_qce_del(const vtss_inst_t inst, const vtss_qcl_id_t qcl_id,
		     const vtss_qce_id_t qce_id)
{
    struct vtss_qce_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.qcl_id = qcl_id;
    ioc.qce_id = qce_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_qce_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_qce_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_QCL */

#endif				/* VTSS_FEATURE_QOS */

 /************************************
  * packet group 
  */

#ifdef VTSS_FEATURE_PACKET
#ifdef VTSS_FEATURE_NPI
vtss_rc vtss_npi_conf_get(const vtss_inst_t inst, vtss_npi_conf_t * conf)
{
    struct vtss_npi_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_npi_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_npi_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_NPI */

#ifdef VTSS_FEATURE_NPI
vtss_rc vtss_npi_conf_set(const vtss_inst_t inst,
			  const vtss_npi_conf_t * conf)
{
    struct vtss_npi_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_npi_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_npi_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_NPI */


vtss_rc vtss_packet_rx_conf_get(const vtss_inst_t inst,
				vtss_packet_rx_conf_t * conf)
{
    struct vtss_packet_rx_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_rx_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_rx_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_packet_rx_conf_set(const vtss_inst_t inst,
				const vtss_packet_rx_conf_t * conf)
{
    struct vtss_packet_rx_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_rx_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_rx_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_PACKET_PORT_REG
vtss_rc vtss_packet_rx_port_conf_get(const vtss_inst_t inst,
				     const vtss_port_no_t port_no,
				     vtss_packet_rx_port_conf_t * conf)
{
    struct vtss_packet_rx_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_rx_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_rx_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */

#ifdef VTSS_FEATURE_PACKET_PORT_REG
vtss_rc vtss_packet_rx_port_conf_set(const vtss_inst_t inst,
				     const vtss_port_no_t port_no,
				     const vtss_packet_rx_port_conf_t *
				     conf)
{
    struct vtss_packet_rx_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_rx_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_rx_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_PACKET_PORT_REG */


vtss_rc vtss_packet_frame_filter(const vtss_inst_t inst,
				 const vtss_packet_frame_info_t * info,
				 vtss_packet_filter_t * filter)
{
    struct vtss_packet_frame_filter_ioc ioc;
    int ret;
    /* Input params start */
    ioc.info = *info;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_frame_filter, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_frame_filter", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *filter = ioc.filter;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_packet_port_info_init(vtss_packet_port_info_t * info)
{
    struct vtss_packet_port_info_init_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_port_info_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_port_info_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *info = ioc.info;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_packet_port_filter_get(const vtss_inst_t inst,
				    const vtss_packet_port_info_t * info,
				    vtss_packet_port_filter_t
				    filter[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_packet_port_filter_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.info = *info;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_port_filter_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_port_filter_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(filter, ioc.filter, sizeof(filter[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_AFI_SWC
vtss_rc vtss_afi_alloc(const vtss_inst_t inst,
		       const vtss_afi_frm_dscr_t * dscr,
		       vtss_afi_id_t * id)
{
    struct vtss_afi_alloc_ioc ioc;
    int ret;
    /* Input params start */
    ioc.dscr = *dscr;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_afi_alloc, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_afi_alloc", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *id = ioc.id;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AFI_SWC */

#ifdef VTSS_FEATURE_AFI_SWC
vtss_rc vtss_afi_free(const vtss_inst_t inst, const vtss_afi_id_t id)
{
    struct vtss_afi_free_ioc ioc;
    int ret;
    /* Input params start */
    ioc.id = id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_afi_free, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_afi_free", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AFI_SWC */


vtss_rc vtss_packet_rx_hdr_decode(const vtss_inst_t inst,
				  const vtss_packet_rx_meta_t * meta,
				  vtss_packet_rx_info_t * info)
{
    struct vtss_packet_rx_hdr_decode_ioc ioc;
    int ret;
    /* Input params start */
    ioc.meta = *meta;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_rx_hdr_decode, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_rx_hdr_decode", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *info = ioc.info;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_packet_tx_info_init(const vtss_inst_t inst,
				 vtss_packet_tx_info_t * info)
{
    struct vtss_packet_tx_info_init_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_packet_tx_info_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_packet_tx_info_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *info = ioc.info;
    /* Output params end */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_PACKET */

 /************************************
  * security group 
  */


#ifdef VTSS_FEATURE_LAYER2
vtss_rc vtss_auth_port_state_get(const vtss_inst_t inst,
				 const vtss_port_no_t port_no,
				 vtss_auth_state_t * state)
{
    struct vtss_auth_port_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_auth_port_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_auth_port_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_LAYER2 */

#ifdef VTSS_FEATURE_LAYER2
vtss_rc vtss_auth_port_state_set(const vtss_inst_t inst,
				 const vtss_port_no_t port_no,
				 const vtss_auth_state_t state)
{
    struct vtss_auth_port_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.state = state;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_auth_port_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_auth_port_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_LAYER2 */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_policer_conf_get(const vtss_inst_t inst,
				  const vtss_acl_policer_no_t policer_no,
				  vtss_acl_policer_conf_t * conf)
{
    struct vtss_acl_policer_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.policer_no = policer_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_policer_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_policer_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_policer_conf_set(const vtss_inst_t inst,
				  const vtss_acl_policer_no_t policer_no,
				  const vtss_acl_policer_conf_t * conf)
{
    struct vtss_acl_policer_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.policer_no = policer_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_policer_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_policer_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_port_conf_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_acl_port_conf_t * conf)
{
    struct vtss_acl_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_port_conf_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_acl_port_conf_t * conf)
{
    struct vtss_acl_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_port_counter_get(const vtss_inst_t inst,
				  const vtss_port_no_t port_no,
				  vtss_acl_port_counter_t * counter)
{
    struct vtss_acl_port_counter_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_port_counter_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_port_counter_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counter = ioc.counter;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_acl_port_counter_clear(const vtss_inst_t inst,
				    const vtss_port_no_t port_no)
{
    struct vtss_acl_port_counter_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_acl_port_counter_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_acl_port_counter_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_ace_init(const vtss_inst_t inst, const vtss_ace_type_t type,
		      vtss_ace_t * ace)
{
    struct vtss_ace_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.type = type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ace_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ace_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ace = ioc.ace;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_ace_add(const vtss_inst_t inst, const vtss_ace_id_t ace_id,
		     const vtss_ace_t * ace)
{
    struct vtss_ace_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ace_id = ace_id;
    ioc.ace = *ace;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ace_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ace_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_ace_del(const vtss_inst_t inst, const vtss_ace_id_t ace_id)
{
    struct vtss_ace_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ace_id = ace_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ace_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ace_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_ace_counter_get(const vtss_inst_t inst,
			     const vtss_ace_id_t ace_id,
			     vtss_ace_counter_t * counter)
{
    struct vtss_ace_counter_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ace_id = ace_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ace_counter_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ace_counter_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counter = ioc.counter;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */

#ifdef VTSS_FEATURE_ACL
vtss_rc vtss_ace_counter_clear(const vtss_inst_t inst,
			       const vtss_ace_id_t ace_id)
{
    struct vtss_ace_counter_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ace_id = ace_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ace_counter_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ace_counter_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_ACL */



 /************************************
  * layer2 group 
  */

#ifdef VTSS_FEATURE_LAYER2

vtss_rc vtss_mac_table_add(const vtss_inst_t inst,
			   const vtss_mac_table_entry_t * entry)
{
    struct vtss_mac_table_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_del(const vtss_inst_t inst,
			   const vtss_vid_mac_t * vid_mac)
{
    struct vtss_mac_table_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid_mac = *vid_mac;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_get(const vtss_inst_t inst,
			   const vtss_vid_mac_t * vid_mac,
			   vtss_mac_table_entry_t * entry)
{
    struct vtss_mac_table_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid_mac = *vid_mac;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *entry = ioc.entry;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_get_next(const vtss_inst_t inst,
				const vtss_vid_mac_t * vid_mac,
				vtss_mac_table_entry_t * entry)
{
    struct vtss_mac_table_get_next_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid_mac = *vid_mac;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_get_next, &ioc)) != 0) {
	/* Silent */ return (vtss_rc) ret;
    }
    /* Output params start */
    *entry = ioc.entry;
    /* Output params end */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_MAC_AGE_AUTO
vtss_rc vtss_mac_table_age_time_get(const vtss_inst_t inst,
				    vtss_mac_table_age_time_t * age_time)
{
    struct vtss_mac_table_age_time_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_age_time_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_age_time_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *age_time = ioc.age_time;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */

#ifdef VTSS_FEATURE_MAC_AGE_AUTO
vtss_rc vtss_mac_table_age_time_set(const vtss_inst_t inst,
				    const vtss_mac_table_age_time_t
				    age_time)
{
    struct vtss_mac_table_age_time_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.age_time = age_time;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_age_time_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_age_time_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_MAC_AGE_AUTO */


vtss_rc vtss_mac_table_age(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_age, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_age", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_vlan_age(const vtss_inst_t inst,
				const vtss_vid_t vid)
{
    struct vtss_mac_table_vlan_age_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_vlan_age, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_vlan_age", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_flush(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_flush, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_port_flush(const vtss_inst_t inst,
				  const vtss_port_no_t port_no)
{
    struct vtss_mac_table_port_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_port_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_port_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_vlan_flush(const vtss_inst_t inst,
				  const vtss_vid_t vid)
{
    struct vtss_mac_table_vlan_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_vlan_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_vlan_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mac_table_vlan_port_flush(const vtss_inst_t inst,
				       const vtss_port_no_t port_no,
				       const vtss_vid_t vid)
{
    struct vtss_mac_table_vlan_port_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_vlan_port_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_vlan_port_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_mac_table_upsid_flush(const vtss_inst_t inst,
				   const vtss_vstax_upsid_t upsid)
{
    struct vtss_mac_table_upsid_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.upsid = upsid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_upsid_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_upsid_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_mac_table_upsid_upspn_flush(const vtss_inst_t inst,
					 const vtss_vstax_upsid_t upsid,
					 const vtss_vstax_upspn_t upspn)
{
    struct vtss_mac_table_upsid_upspn_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.upsid = upsid;
    ioc.upspn = upspn;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_mac_table_upsid_upspn_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_upsid_upspn_flush",
	       ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_AGGR_GLAG
vtss_rc vtss_mac_table_glag_add(const vtss_inst_t inst,
				const vtss_mac_table_entry_t * entry,
				const vtss_glag_no_t glag_no)
{
    struct vtss_mac_table_glag_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    ioc.glag_no = glag_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_glag_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_glag_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_AGGR_GLAG
vtss_rc vtss_mac_table_glag_flush(const vtss_inst_t inst,
				  const vtss_glag_no_t glag_no)
{
    struct vtss_mac_table_glag_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_glag_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_glag_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_AGGR_GLAG
vtss_rc vtss_mac_table_vlan_glag_flush(const vtss_inst_t inst,
				       const vtss_glag_no_t glag_no,
				       const vtss_vid_t vid)
{
    struct vtss_mac_table_vlan_glag_flush_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_vlan_glag_flush, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_vlan_glag_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AGGR_GLAG */


vtss_rc vtss_mac_table_status_get(const vtss_inst_t inst,
				  vtss_mac_table_status_t * status)
{
    struct vtss_mac_table_status_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mac_table_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mac_table_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_learn_port_mode_get(const vtss_inst_t inst,
				 const vtss_port_no_t port_no,
				 vtss_learn_mode_t * mode)
{
    struct vtss_learn_port_mode_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_learn_port_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_learn_port_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_learn_port_mode_set(const vtss_inst_t inst,
				 const vtss_port_no_t port_no,
				 const vtss_learn_mode_t * mode)
{
    struct vtss_learn_port_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mode = *mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_learn_port_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_learn_port_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_state_get(const vtss_inst_t inst,
			    const vtss_port_no_t port_no, BOOL * state)
{
    struct vtss_port_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_port_state_set(const vtss_inst_t inst,
			    const vtss_port_no_t port_no, const BOOL state)
{
    struct vtss_port_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.state = state;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_port_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_stp_port_state_get(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				vtss_stp_state_t * state)
{
    struct vtss_stp_port_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_stp_port_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_stp_port_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_stp_port_state_set(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_stp_state_t state)
{
    struct vtss_stp_port_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.state = state;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_stp_port_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_stp_port_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mstp_vlan_msti_get(const vtss_inst_t inst,
				const vtss_vid_t vid, vtss_msti_t * msti)
{
    struct vtss_mstp_vlan_msti_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mstp_vlan_msti_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mstp_vlan_msti_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *msti = ioc.msti;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mstp_vlan_msti_set(const vtss_inst_t inst,
				const vtss_vid_t vid,
				const vtss_msti_t msti)
{
    struct vtss_mstp_vlan_msti_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.msti = msti;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mstp_vlan_msti_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mstp_vlan_msti_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mstp_port_msti_state_get(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const vtss_msti_t msti,
				      vtss_stp_state_t * state)
{
    struct vtss_mstp_port_msti_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.msti = msti;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mstp_port_msti_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mstp_port_msti_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mstp_port_msti_state_set(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const vtss_msti_t msti,
				      const vtss_stp_state_t state)
{
    struct vtss_mstp_port_msti_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.msti = msti;
    ioc.state = state;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mstp_port_msti_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mstp_port_msti_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_conf_get(const vtss_inst_t inst, vtss_vlan_conf_t * conf)
{
    struct vtss_vlan_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_conf_set(const vtss_inst_t inst,
			   const vtss_vlan_conf_t * conf)
{
    struct vtss_vlan_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_port_conf_get(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				vtss_vlan_port_conf_t * conf)
{
    struct vtss_vlan_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_port_conf_set(const vtss_inst_t inst,
				const vtss_port_no_t port_no,
				const vtss_vlan_port_conf_t * conf)
{
    struct vtss_vlan_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_port_members_get(const vtss_inst_t inst,
				   const vtss_vid_t vid,
				   BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_vlan_port_members_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_port_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_port_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_vlan_port_members_set(const vtss_inst_t inst,
				   const vtss_vid_t vid,
				   const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_vlan_port_members_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_port_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_port_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_VLAN_COUNTERS
vtss_rc vtss_vlan_counters_get(const vtss_inst_t inst,
			       const vtss_vid_t vid,
			       vtss_vlan_counters_t * counters)
{
    struct vtss_vlan_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */

#ifdef VTSS_FEATURE_VLAN_COUNTERS
vtss_rc vtss_vlan_counters_clear(const vtss_inst_t inst,
				 const vtss_vid_t vid)
{
    struct vtss_vlan_counters_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_counters_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_counters_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_COUNTERS */


vtss_rc vtss_vce_init(const vtss_inst_t inst, const vtss_vce_type_t type,
		      vtss_vce_t * vce)
{
    struct vtss_vce_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.type = type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vce_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vce_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *vce = ioc.vce;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_vce_add(const vtss_inst_t inst, const vtss_vce_id_t vce_id,
		     const vtss_vce_t * vce)
{
    struct vtss_vce_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vce_id = vce_id;
    ioc.vce = *vce;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vce_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vce_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_vce_del(const vtss_inst_t inst, const vtss_vce_id_t vce_id)
{
    struct vtss_vce_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vce_id = vce_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vce_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vce_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_VLAN_TRANSLATION
vtss_rc vtss_vlan_trans_group_add(const vtss_inst_t inst,
				  const u16 group_id, const vtss_vid_t vid,
				  const vtss_vid_t trans_vid)
{
    struct vtss_vlan_trans_group_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.group_id = group_id;
    ioc.vid = vid;
    ioc.trans_vid = trans_vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_trans_group_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_trans_group_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
vtss_rc vtss_vlan_trans_group_del(const vtss_inst_t inst,
				  const u16 group_id, const vtss_vid_t vid)
{
    struct vtss_vlan_trans_group_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.group_id = group_id;
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_trans_group_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_trans_group_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
vtss_rc vtss_vlan_trans_group_get(const vtss_inst_t inst,
				  vtss_vlan_trans_grp2vlan_conf_t * conf,
				  const BOOL next)
{
    struct vtss_vlan_trans_group_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.next = next;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vlan_trans_group_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_trans_group_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
vtss_rc vtss_vlan_trans_group_to_port_set(const vtss_inst_t inst,
					  const
					  vtss_vlan_trans_port2grp_conf_t *
					  conf)
{
    struct vtss_vlan_trans_group_to_port_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_vlan_trans_group_to_port_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_trans_group_to_port_set",
	       ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */

#ifdef VTSS_FEATURE_VLAN_TRANSLATION
vtss_rc vtss_vlan_trans_group_to_port_get(const vtss_inst_t inst,
					  vtss_vlan_trans_port2grp_conf_t *
					  conf, const BOOL next)
{
    struct vtss_vlan_trans_group_to_port_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.next = next;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_vlan_trans_group_to_port_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vlan_trans_group_to_port_get",
	       ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VLAN_TRANSLATION */


vtss_rc vtss_isolated_vlan_get(const vtss_inst_t inst,
			       const vtss_vid_t vid, BOOL * isolated)
{
    struct vtss_isolated_vlan_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_isolated_vlan_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_isolated_vlan_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *isolated = ioc.isolated;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_isolated_vlan_set(const vtss_inst_t inst,
			       const vtss_vid_t vid, const BOOL isolated)
{
    struct vtss_isolated_vlan_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.isolated = isolated;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_isolated_vlan_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_isolated_vlan_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_isolated_port_members_get(const vtss_inst_t inst,
				       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_isolated_port_members_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_isolated_port_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_isolated_port_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_isolated_port_members_set(const vtss_inst_t inst,
				       const BOOL
				       member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_isolated_port_members_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_isolated_port_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_isolated_port_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_PVLAN
vtss_rc vtss_pvlan_port_members_get(const vtss_inst_t inst,
				    const vtss_pvlan_no_t pvlan_no,
				    BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_pvlan_port_members_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.pvlan_no = pvlan_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_pvlan_port_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_pvlan_port_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_PVLAN */

#ifdef VTSS_FEATURE_PVLAN
vtss_rc vtss_pvlan_port_members_set(const vtss_inst_t inst,
				    const vtss_pvlan_no_t pvlan_no,
				    const BOOL
				    member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_pvlan_port_members_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.pvlan_no = pvlan_no;
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_pvlan_port_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_pvlan_port_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_PVLAN */

#ifdef VTSS_FEATURE_SFLOW
vtss_rc vtss_sflow_port_conf_get(const vtss_inst_t inst, const u32 port_no,
				 vtss_sflow_port_conf_t * conf)
{
    struct vtss_sflow_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sflow_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sflow_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SFLOW */

#ifdef VTSS_FEATURE_SFLOW
vtss_rc vtss_sflow_port_conf_set(const vtss_inst_t inst, const u32 port_no,
				 const vtss_sflow_port_conf_t * conf)
{
    struct vtss_sflow_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_sflow_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_sflow_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_SFLOW */


vtss_rc vtss_aggr_port_members_get(const vtss_inst_t inst,
				   const vtss_aggr_no_t aggr_no,
				   BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_aggr_port_members_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.aggr_no = aggr_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_port_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_port_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_aggr_port_members_set(const vtss_inst_t inst,
				   const vtss_aggr_no_t aggr_no,
				   const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_aggr_port_members_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.aggr_no = aggr_no;
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_port_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_port_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_aggr_mode_get(const vtss_inst_t inst, vtss_aggr_mode_t * mode)
{
    struct vtss_aggr_mode_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_aggr_mode_set(const vtss_inst_t inst,
			   const vtss_aggr_mode_t * mode)
{
    struct vtss_aggr_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.mode = *mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_AGGR_GLAG
vtss_rc vtss_aggr_glag_members_get(const vtss_inst_t inst,
				   const vtss_glag_no_t glag_no,
				   BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_aggr_glag_members_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_glag_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_glag_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_AGGR_GLAG */

#ifdef VTSS_FEATURE_VSTAX_V1
vtss_rc vtss_aggr_glag_set(const vtss_inst_t inst,
			   const vtss_glag_no_t glag_no,
			   const vtss_port_no_t
			   member[VTSS_GLAG_PORT_ARRAY_SIZE])
{
    struct vtss_aggr_glag_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    memcpy(ioc.member, member,
	   sizeof(member[0]) * VTSS_GLAG_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_aggr_glag_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_aggr_glag_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX_V1 */

#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_vstax_glag_get(const vtss_inst_t inst,
			    const vtss_glag_no_t glag_no,
			    vtss_vstax_glag_entry_t
			    entry[VTSS_GLAG_PORT_ARRAY_SIZE])
{
    struct vtss_vstax_glag_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_glag_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_glag_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(entry, ioc.entry, sizeof(entry[0]) * VTSS_GLAG_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX_V2 */

#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_vstax_glag_set(const vtss_inst_t inst,
			    const vtss_glag_no_t glag_no,
			    const vtss_vstax_glag_entry_t
			    entry[VTSS_GLAG_PORT_ARRAY_SIZE])
{
    struct vtss_vstax_glag_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.glag_no = glag_no;
    memcpy(ioc.entry, entry, sizeof(entry[0]) * VTSS_GLAG_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_glag_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_glag_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX_V2 */


vtss_rc vtss_mirror_monitor_port_get(const vtss_inst_t inst,
				     vtss_port_no_t * port_no)
{
    struct vtss_mirror_monitor_port_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_monitor_port_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_monitor_port_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *port_no = ioc.port_no;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_monitor_port_set(const vtss_inst_t inst,
				     const vtss_port_no_t port_no)
{
    struct vtss_mirror_monitor_port_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_monitor_port_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_monitor_port_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_ingress_ports_get(const vtss_inst_t inst,
				      BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mirror_ingress_ports_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_ingress_ports_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_ingress_ports_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_ingress_ports_set(const vtss_inst_t inst,
				      const BOOL
				      member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mirror_ingress_ports_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_ingress_ports_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_ingress_ports_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_egress_ports_get(const vtss_inst_t inst,
				     BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mirror_egress_ports_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_egress_ports_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_egress_ports_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_egress_ports_set(const vtss_inst_t inst,
				     const BOOL
				     member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mirror_egress_ports_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_egress_ports_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_egress_ports_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_cpu_ingress_get(const vtss_inst_t inst, BOOL * member)
{
    struct vtss_mirror_cpu_ingress_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_cpu_ingress_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_cpu_ingress_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *member = ioc.member;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_cpu_ingress_set(const vtss_inst_t inst,
				    const BOOL member)
{
    struct vtss_mirror_cpu_ingress_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.member = member;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_cpu_ingress_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_cpu_ingress_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_cpu_egress_get(const vtss_inst_t inst, BOOL * member)
{
    struct vtss_mirror_cpu_egress_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_cpu_egress_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_cpu_egress_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *member = ioc.member;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mirror_cpu_egress_set(const vtss_inst_t inst,
				   const BOOL member)
{
    struct vtss_mirror_cpu_egress_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.member = member;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mirror_cpu_egress_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mirror_cpu_egress_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_uc_flood_members_get(const vtss_inst_t inst,
				  BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_uc_flood_members_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_uc_flood_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_uc_flood_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_uc_flood_members_set(const vtss_inst_t inst,
				  const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_uc_flood_members_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_uc_flood_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_uc_flood_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mc_flood_members_get(const vtss_inst_t inst,
				  BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mc_flood_members_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mc_flood_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mc_flood_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mc_flood_members_set(const vtss_inst_t inst,
				  const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_mc_flood_members_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mc_flood_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mc_flood_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ipv4_mc_flood_members_get(const vtss_inst_t inst,
				       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv4_mc_flood_members_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv4_mc_flood_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv4_mc_flood_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ipv4_mc_flood_members_set(const vtss_inst_t inst,
				       const BOOL
				       member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv4_mc_flood_members_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv4_mc_flood_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv4_mc_flood_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_IPV4_MC_SIP
vtss_rc vtss_ipv4_mc_add(const vtss_inst_t inst, const vtss_vid_t vid,
			 const vtss_ip_t sip, const vtss_ip_t dip,
			 const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv4_mc_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.sip = sip;
    ioc.dip = dip;
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv4_mc_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv4_mc_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */

#ifdef VTSS_FEATURE_IPV4_MC_SIP
vtss_rc vtss_ipv4_mc_del(const vtss_inst_t inst, const vtss_vid_t vid,
			 const vtss_ip_t sip, const vtss_ip_t dip)
{
    struct vtss_ipv4_mc_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.sip = sip;
    ioc.dip = dip;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv4_mc_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv4_mc_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_IPV4_MC_SIP */


vtss_rc vtss_ipv6_mc_flood_members_get(const vtss_inst_t inst,
				       BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv6_mc_flood_members_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_flood_members_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_flood_members_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    memcpy(member, ioc.member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ipv6_mc_flood_members_set(const vtss_inst_t inst,
				       const BOOL
				       member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv6_mc_flood_members_set_ioc ioc;
    int ret;
    /* Input params start */
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_flood_members_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_flood_members_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ipv6_mc_ctrl_flood_get(const vtss_inst_t inst, BOOL * scope)
{
    struct vtss_ipv6_mc_ctrl_flood_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_ctrl_flood_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_ctrl_flood_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *scope = ioc.scope;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ipv6_mc_ctrl_flood_set(const vtss_inst_t inst,
				    const BOOL scope)
{
    struct vtss_ipv6_mc_ctrl_flood_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.scope = scope;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_ctrl_flood_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_ctrl_flood_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_FEATURE_IPV6_MC_SIP
vtss_rc vtss_ipv6_mc_add(const vtss_inst_t inst, const vtss_vid_t vid,
			 const vtss_ipv6_t sip, const vtss_ipv6_t dip,
			 const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    struct vtss_ipv6_mc_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.sip = sip;
    ioc.dip = dip;
    memcpy(ioc.member, member, sizeof(member[0]) * VTSS_PORT_ARRAY_SIZE);
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */

#ifdef VTSS_FEATURE_IPV6_MC_SIP
vtss_rc vtss_ipv6_mc_del(const vtss_inst_t inst, const vtss_vid_t vid,
			 const vtss_ipv6_t sip, const vtss_ipv6_t dip)
{
    struct vtss_ipv6_mc_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.sip = sip;
    ioc.dip = dip;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ipv6_mc_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ipv6_mc_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_IPV6_MC_SIP */


vtss_rc vtss_eps_port_conf_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_eps_port_conf_t * conf)
{
    struct vtss_eps_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eps_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eps_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_eps_port_conf_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_eps_port_conf_t * conf)
{
    struct vtss_eps_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eps_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eps_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_eps_port_selector_get(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   vtss_eps_selector_t * selector)
{
    struct vtss_eps_port_selector_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eps_port_selector_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eps_port_selector_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *selector = ioc.selector;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_eps_port_selector_set(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_eps_selector_t selector)
{
    struct vtss_eps_port_selector_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.selector = selector;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_eps_port_selector_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_eps_port_selector_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_erps_vlan_member_get(const vtss_inst_t inst,
				  const vtss_erpi_t erpi,
				  const vtss_vid_t vid, BOOL * member)
{
    struct vtss_erps_vlan_member_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.erpi = erpi;
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_erps_vlan_member_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_erps_vlan_member_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *member = ioc.member;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_erps_vlan_member_set(const vtss_inst_t inst,
				  const vtss_erpi_t erpi,
				  const vtss_vid_t vid, const BOOL member)
{
    struct vtss_erps_vlan_member_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.erpi = erpi;
    ioc.vid = vid;
    ioc.member = member;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_erps_vlan_member_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_erps_vlan_member_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_erps_port_state_get(const vtss_inst_t inst,
				 const vtss_erpi_t erpi,
				 const vtss_port_no_t port_no,
				 vtss_erps_state_t * state)
{
    struct vtss_erps_port_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.erpi = erpi;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_erps_port_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_erps_port_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_erps_port_state_set(const vtss_inst_t inst,
				 const vtss_erpi_t erpi,
				 const vtss_port_no_t port_no,
				 const vtss_erps_state_t state)
{
    struct vtss_erps_port_state_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.erpi = erpi;
    ioc.port_no = port_no;
    ioc.state = state;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_erps_port_state_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_erps_port_state_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_ARCH_B2
vtss_rc vtss_vid2port_set(const vtss_inst_t inst, const vtss_vid_t vid,
			  const vtss_port_no_t port_no)
{
    struct vtss_vid2port_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vid2port_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vid2port_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_ARCH_B2
vtss_rc vtss_vid2lport_get(const vtss_inst_t inst, const vtss_vid_t vid,
			   vtss_lport_no_t * lport_no)
{
    struct vtss_vid2lport_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vid2lport_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vid2lport_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *lport_no = ioc.lport_no;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_ARCH_B2
vtss_rc vtss_vid2lport_set(const vtss_inst_t inst, const vtss_vid_t vid,
			   const vtss_lport_no_t lport_no)
{
    struct vtss_vid2lport_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vid = vid;
    ioc.lport_no = lport_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vid2lport_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vid2lport_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_B2 */

#ifdef VTSS_FEATURE_VSTAX
vtss_rc vtss_vstax_conf_get(const vtss_inst_t inst,
			    vtss_vstax_conf_t * conf)
{
    struct vtss_vstax_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
vtss_rc vtss_vstax_conf_set(const vtss_inst_t inst,
			    const vtss_vstax_conf_t * conf)
{
    struct vtss_vstax_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
vtss_rc vtss_vstax_port_conf_get(const vtss_inst_t inst,
				 const vtss_chip_no_t chip_no,
				 const BOOL stack_port_a,
				 vtss_vstax_port_conf_t * conf)
{
    struct vtss_vstax_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.stack_port_a = stack_port_a;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
vtss_rc vtss_vstax_port_conf_set(const vtss_inst_t inst,
				 const vtss_chip_no_t chip_no,
				 const BOOL stack_port_a,
				 const vtss_vstax_port_conf_t * conf)
{
    struct vtss_vstax_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.stack_port_a = stack_port_a;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX */

#ifdef VTSS_FEATURE_VSTAX
vtss_rc vtss_vstax_topology_set(const vtss_inst_t inst,
				const vtss_chip_no_t chip_no,
				const vtss_vstax_route_table_t * table)
{
    struct vtss_vstax_topology_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.chip_no = chip_no;
    ioc.table = *table;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_vstax_topology_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_vstax_topology_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_FEATURE_VSTAX */

#endif				/* VTSS_FEATURE_LAYER2 */

 /************************************
  * evc group 
  */

#ifdef VTSS_FEATURE_EVC

vtss_rc vtss_evc_port_conf_get(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       vtss_evc_port_conf_t * conf)
{
    struct vtss_evc_port_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_evc_port_conf_set(const vtss_inst_t inst,
			       const vtss_port_no_t port_no,
			       const vtss_evc_port_conf_t * conf)
{
    struct vtss_evc_port_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_evc_add(const vtss_inst_t inst, const vtss_evc_id_t evc_id,
		     const vtss_evc_conf_t * conf)
{
    struct vtss_evc_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.evc_id = evc_id;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_evc_del(const vtss_inst_t inst, const vtss_evc_id_t evc_id)
{
    struct vtss_evc_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.evc_id = evc_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_evc_get(const vtss_inst_t inst, const vtss_evc_id_t evc_id,
		     vtss_evc_conf_t * conf)
{
    struct vtss_evc_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.evc_id = evc_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ece_init(const vtss_inst_t inst, const vtss_ece_type_t type,
		      vtss_ece_t * ece)
{
    struct vtss_ece_init_ioc ioc;
    int ret;
    /* Input params start */
    ioc.type = type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ece_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ece_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ece = ioc.ece;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ece_add(const vtss_inst_t inst, const vtss_ece_id_t ece_id,
		     const vtss_ece_t * ece)
{
    struct vtss_ece_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ece_id = ece_id;
    ioc.ece = *ece;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ece_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ece_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ece_del(const vtss_inst_t inst, const vtss_ece_id_t ece_id)
{
    struct vtss_ece_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ece_id = ece_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ece_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ece_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#ifdef VTSS_ARCH_CARACAL
vtss_rc vtss_mce_init(const vtss_inst_t inst, vtss_mce_t * mce)
{
    struct vtss_mce_init_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mce_init, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mce_init", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mce = ioc.mce;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
vtss_rc vtss_mce_add(const vtss_inst_t inst, const vtss_mce_id_t mce_id,
		     const vtss_mce_t * mce)
{
    struct vtss_mce_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.mce_id = mce_id;
    ioc.mce = *mce;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mce_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mce_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_CARACAL
vtss_rc vtss_mce_del(const vtss_inst_t inst, const vtss_mce_id_t mce_id)
{
    struct vtss_mce_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.mce_id = mce_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mce_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mce_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_CARACAL */

#ifdef VTSS_ARCH_JAGUAR_1
vtss_rc vtss_evc_counters_get(const vtss_inst_t inst,
			      const vtss_evc_id_t evc_id,
			      const vtss_port_no_t port_no,
			      vtss_evc_counters_t * counters)
{
    struct vtss_evc_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.evc_id = evc_id;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
vtss_rc vtss_evc_counters_clear(const vtss_inst_t inst,
				const vtss_evc_id_t evc_id,
				const vtss_port_no_t port_no)
{
    struct vtss_evc_counters_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.evc_id = evc_id;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_counters_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_counters_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
vtss_rc vtss_ece_counters_get(const vtss_inst_t inst,
			      const vtss_ece_id_t ece_id,
			      const vtss_port_no_t port_no,
			      vtss_evc_counters_t * counters)
{
    struct vtss_ece_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ece_id = ece_id;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ece_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ece_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_JAGUAR_1 */

#ifdef VTSS_ARCH_JAGUAR_1
vtss_rc vtss_ece_counters_clear(const vtss_inst_t inst,
				const vtss_ece_id_t ece_id,
				const vtss_port_no_t port_no)
{
    struct vtss_ece_counters_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ece_id = ece_id;
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ece_counters_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ece_counters_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}
#endif				/* VTSS_ARCH_JAGUAR_1 */

#endif				/* VTSS_FEATURE_EVC */

 /************************************
  * qos_policer_dlb group 
  */

#ifdef VTSS_FEATURE_QOS_POLICER_DLB

vtss_rc vtss_evc_policer_conf_get(const vtss_inst_t inst,
				  const vtss_evc_policer_id_t policer_id,
				  vtss_evc_policer_conf_t * conf)
{
    struct vtss_evc_policer_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.policer_id = policer_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_policer_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_policer_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_evc_policer_conf_set(const vtss_inst_t inst,
				  const vtss_evc_policer_id_t policer_id,
				  const vtss_evc_policer_conf_t * conf)
{
    struct vtss_evc_policer_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.policer_id = policer_id;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_evc_policer_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_evc_policer_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_QOS_POLICER_DLB */

 /************************************
  * timestamp group 
  */

#ifdef VTSS_FEATURE_TIMESTAMP

vtss_rc vtss_ts_timeofday_set(const vtss_inst_t inst,
			      const vtss_timestamp_t * ts)
{
    struct vtss_ts_timeofday_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ts = *ts;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_timeofday_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_timeofday_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_adjtimer_one_sec(const vtss_inst_t inst,
				 BOOL * ongoing_adjustment)
{
    struct vtss_ts_adjtimer_one_sec_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_adjtimer_one_sec, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_adjtimer_one_sec", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ongoing_adjustment = ioc.ongoing_adjustment;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_timeofday_get(const vtss_inst_t inst,
			      vtss_timestamp_t * ts, u32 * tc)
{
    struct vtss_ts_timeofday_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_timeofday_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_timeofday_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ts = ioc.ts;
    *tc = ioc.tc;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_adjtimer_set(const vtss_inst_t inst, const i32 adj)
{
    struct vtss_ts_adjtimer_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.adj = adj;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_adjtimer_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_adjtimer_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_adjtimer_get(const vtss_inst_t inst, i32 * adj)
{
    struct vtss_ts_adjtimer_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_adjtimer_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_adjtimer_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *adj = ioc.adj;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_freq_offset_get(const vtss_inst_t inst, i32 * adj)
{
    struct vtss_ts_freq_offset_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_freq_offset_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_freq_offset_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *adj = ioc.adj;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_external_clock_mode_set(const vtss_inst_t inst,
					const vtss_ts_ext_clock_mode_t *
					ext_clock_mode)
{
    struct vtss_ts_external_clock_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ext_clock_mode = *ext_clock_mode;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_ts_external_clock_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_external_clock_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_external_clock_mode_get(const vtss_inst_t inst,
					vtss_ts_ext_clock_mode_t *
					ext_clock_mode)
{
    struct vtss_ts_external_clock_mode_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_ts_external_clock_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_external_clock_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ext_clock_mode = ioc.ext_clock_mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_ingress_latency_set(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    const vtss_timeinterval_t *
				    ingress_latency)
{
    struct vtss_ts_ingress_latency_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.ingress_latency = *ingress_latency;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_ingress_latency_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_ingress_latency_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_ingress_latency_get(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    vtss_timeinterval_t * ingress_latency)
{
    struct vtss_ts_ingress_latency_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_ingress_latency_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_ingress_latency_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ingress_latency = ioc.ingress_latency;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_p2p_delay_set(const vtss_inst_t inst,
			      const vtss_port_no_t port_no,
			      const vtss_timeinterval_t * p2p_delay)
{
    struct vtss_ts_p2p_delay_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.p2p_delay = *p2p_delay;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_p2p_delay_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_p2p_delay_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_p2p_delay_get(const vtss_inst_t inst,
			      const vtss_port_no_t port_no,
			      vtss_timeinterval_t * p2p_delay)
{
    struct vtss_ts_p2p_delay_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_p2p_delay_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_p2p_delay_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *p2p_delay = ioc.p2p_delay;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_egress_latency_set(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_timeinterval_t *
				   egress_latency)
{
    struct vtss_ts_egress_latency_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.egress_latency = *egress_latency;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_egress_latency_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_egress_latency_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_egress_latency_get(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   vtss_timeinterval_t * egress_latency)
{
    struct vtss_ts_egress_latency_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_egress_latency_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_egress_latency_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *egress_latency = ioc.egress_latency;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_operation_mode_set(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   const vtss_ts_operation_mode_t * mode)
{
    struct vtss_ts_operation_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mode = *mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_operation_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_operation_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_ts_operation_mode_get(const vtss_inst_t inst,
				   const vtss_port_no_t port_no,
				   vtss_ts_operation_mode_t * mode)
{
    struct vtss_ts_operation_mode_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_ts_operation_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_ts_operation_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_tx_timestamp_update(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_tx_timestamp_update, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_tx_timestamp_update", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_rx_timestamp_get(const vtss_inst_t inst,
			      const vtss_ts_id_t * ts_id,
			      vtss_ts_timestamp_t * ts)
{
    struct vtss_rx_timestamp_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.ts_id = *ts_id;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_rx_timestamp_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_rx_timestamp_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ts = ioc.ts;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_rx_master_timestamp_get(const vtss_inst_t inst,
				     const vtss_port_no_t port_no,
				     vtss_ts_timestamp_t * ts)
{
    struct vtss_rx_master_timestamp_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_rx_master_timestamp_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_rx_master_timestamp_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ts = ioc.ts;
    /* Output params end */
    return VTSS_RC_OK;
}


#ifdef NOTDEF
vtss_rc vtss_tx_timestamp_idx_alloc(const vtss_inst_t inst,
				    const vtss_ts_timestamp_alloc_t *
				    alloc_parm, vtss_ts_id_t * ts_id)
{
    struct vtss_tx_timestamp_idx_alloc_ioc ioc;
    int ret;
    /* Input params start */
    ioc.alloc_parm = *alloc_parm;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_tx_timestamp_idx_alloc, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_tx_timestamp_idx_alloc", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *ts_id = ioc.ts_id;
    /* Output params end */
    return VTSS_RC_OK;
}
#endif				/* NOTDEF */


vtss_rc vtss_timestamp_age(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_timestamp_age, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_timestamp_age", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_TIMESTAMP */

 /************************************
  * synce group 
  */

#ifdef VTSS_FEATURE_SYNCE

vtss_rc vtss_synce_clock_out_set(const vtss_inst_t inst,
				 const vtss_synce_clk_port_t clk_port,
				 const vtss_synce_clock_out_t * conf)
{
    struct vtss_synce_clock_out_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.clk_port = clk_port;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_synce_clock_out_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_synce_clock_out_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_synce_clock_out_get(const vtss_inst_t inst,
				 const vtss_synce_clk_port_t clk_port,
				 vtss_synce_clock_out_t * conf)
{
    struct vtss_synce_clock_out_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.clk_port = clk_port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_synce_clock_out_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_synce_clock_out_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_synce_clock_in_set(const vtss_inst_t inst,
				const vtss_synce_clk_port_t clk_port,
				const vtss_synce_clock_in_t * conf)
{
    struct vtss_synce_clock_in_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.clk_port = clk_port;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_synce_clock_in_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_synce_clock_in_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_synce_clock_in_get(const vtss_inst_t inst,
				const vtss_synce_clk_port_t clk_port,
				vtss_synce_clock_in_t * conf)
{
    struct vtss_synce_clock_in_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.clk_port = clk_port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_synce_clock_in_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_synce_clock_in_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_SYNCE */

 /************************************
  * ccm group 
  */



vtss_rc vtssx_ccm_start(const vtss_inst_t inst, const void *frame,
			const size_t length, const vtssx_ccm_opt_t * opt,
			vtssx_ccm_session_t * sess)
{
    struct vtssx_ccm_start_ioc ioc;
    int ret;
    /* Input params start */
    ioc.frame = frame;
    ioc.length = length;
    ioc.opt = *opt;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtssx_ccm_start, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtssx_ccm_start", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *sess = ioc.sess;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtssx_ccm_status_get(const vtss_inst_t inst,
			     const vtssx_ccm_session_t sess,
			     vtssx_ccm_status_t * status)
{
    struct vtssx_ccm_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.sess = sess;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtssx_ccm_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtssx_ccm_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtssx_ccm_cancel(const vtss_inst_t inst,
			 const vtssx_ccm_session_t sess)
{
    struct vtssx_ccm_cancel_ioc ioc;
    int ret;
    /* Input params start */
    ioc.sess = sess;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtssx_ccm_cancel, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtssx_ccm_cancel", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}




 /************************************
  * l3 group 
  */

#ifdef VTSS_SW_OPTION_L3RT

vtss_rc vtss_l3_flush(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_flush, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_flush", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_rleg_common_get(const vtss_inst_t inst,
				vtss_l3_rleg_common_conf_t * conf)
{
    struct vtss_l3_rleg_common_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_rleg_common_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_rleg_common_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_rleg_common_set(const vtss_inst_t inst,
				const vtss_l3_rleg_common_conf_t * conf)
{
    struct vtss_l3_rleg_common_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_rleg_common_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_rleg_common_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_rleg_get(const vtss_inst_t inst, u32 * cnt,
			 vtss_l3_rleg_conf_t buf[VTSS_RLEG_CNT])
{
    struct vtss_l3_rleg_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.buf = buf;		/* Separate copyin/copyout: buf */
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_rleg_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_rleg_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cnt = ioc.cnt;
    /* Note: buf copied directly */
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_rleg_add(const vtss_inst_t inst,
			 const vtss_l3_rleg_conf_t * conf)
{
    struct vtss_l3_rleg_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_rleg_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_rleg_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_rleg_del(const vtss_inst_t inst, const vtss_vid_t vlan)
{
    struct vtss_l3_rleg_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.vlan = vlan;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_rleg_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_rleg_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_route_get(const vtss_inst_t inst, u32 * cnt,
			  vtss_routing_entry_t buf[VTSS_LPM_CNT])
{
    struct vtss_l3_route_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.buf = buf;		/* Separate copyin/copyout: buf */
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_route_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_route_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cnt = ioc.cnt;
    /* Note: buf copied directly */
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_route_add(const vtss_inst_t inst,
			  const vtss_routing_entry_t * entry)
{
    struct vtss_l3_route_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_route_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_route_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_route_del(const vtss_inst_t inst,
			  const vtss_routing_entry_t * entry)
{
    struct vtss_l3_route_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_route_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_route_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_neighbour_get(const vtss_inst_t inst, u32 * cnt,
			      vtss_l3_neighbour_t buf[VTSS_ARP_CNT])
{
    struct vtss_l3_neighbour_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.buf = buf;		/* Separate copyin/copyout: buf */
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_neighbour_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_neighbour_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cnt = ioc.cnt;
    /* Note: buf copied directly */
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_neighbour_add(const vtss_inst_t inst,
			      const vtss_l3_neighbour_t * entry)
{
    struct vtss_l3_neighbour_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_neighbour_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_neighbour_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_neighbour_del(const vtss_inst_t inst,
			      const vtss_l3_neighbour_t * entry)
{
    struct vtss_l3_neighbour_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.entry = *entry;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_neighbour_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_neighbour_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_counters_reset(const vtss_inst_t inst)
{
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_counters_reset, NULL)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_counters_reset", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_counters_system_get(const vtss_inst_t inst,
				    vtss_l3_counters_t * counters)
{
    struct vtss_l3_counters_system_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_counters_system_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_counters_system_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_l3_counters_rleg_get(const vtss_inst_t inst,
				  const vtss_l3_rleg_id_t rleg,
				  vtss_l3_counters_t * counters)
{
    struct vtss_l3_counters_rleg_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.rleg = rleg;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_l3_counters_rleg_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_l3_counters_rleg_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}


#endif				/* VTSS_SW_OPTION_L3RT */

 /************************************
  * mpls group 
  */

#ifdef VTSS_FEATURE_MPLS

vtss_rc vtss_mpls_l2_alloc(const vtss_inst_t inst,
			   vtss_mpls_l2_idx_t * idx)
{
    struct vtss_mpls_l2_alloc_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_alloc, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_alloc", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *idx = ioc.idx;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_l2_free(const vtss_inst_t inst,
			  const vtss_mpls_l2_idx_t idx)
{
    struct vtss_mpls_l2_free_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_free, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_free", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_l2_get(const vtss_inst_t inst,
			 const vtss_mpls_l2_idx_t idx, vtss_mpls_l2_t * l2)
{
    struct vtss_mpls_l2_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *l2 = ioc.l2;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_l2_set(const vtss_inst_t inst,
			 const vtss_mpls_l2_idx_t idx,
			 const vtss_mpls_l2_t * l2)
{
    struct vtss_mpls_l2_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    ioc.l2 = *l2;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_l2_segment_attach(const vtss_inst_t inst,
				    const vtss_mpls_l2_idx_t idx,
				    const vtss_mpls_segment_idx_t seg_idx)
{
    struct vtss_mpls_l2_segment_attach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_segment_attach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_segment_attach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_l2_segment_detach(const vtss_inst_t inst,
				    const vtss_mpls_segment_idx_t seg_idx)
{
    struct vtss_mpls_l2_segment_detach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_l2_segment_detach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_l2_segment_detach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_alloc(const vtss_inst_t inst, const BOOL is_in,
				vtss_mpls_segment_idx_t * idx)
{
    struct vtss_mpls_segment_alloc_ioc ioc;
    int ret;
    /* Input params start */
    ioc.is_in = is_in;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_segment_alloc, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_alloc", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *idx = ioc.idx;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_free(const vtss_inst_t inst,
			       const vtss_mpls_segment_idx_t idx)
{
    struct vtss_mpls_segment_free_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_segment_free, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_free", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_get(const vtss_inst_t inst,
			      const vtss_mpls_segment_idx_t idx,
			      vtss_mpls_segment_t * seg)
{
    struct vtss_mpls_segment_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_segment_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *seg = ioc.seg;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_set(const vtss_inst_t inst,
			      const vtss_mpls_segment_idx_t idx,
			      const vtss_mpls_segment_t * seg)
{
    struct vtss_mpls_segment_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    ioc.seg = *seg;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_segment_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_state_get(const vtss_inst_t inst,
				    const vtss_mpls_segment_idx_t idx,
				    vtss_mpls_segment_state_t * state)
{
    struct vtss_mpls_segment_state_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_segment_state_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_state_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *state = ioc.state;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_server_attach(const vtss_inst_t inst,
					const vtss_mpls_segment_idx_t idx,
					const vtss_mpls_segment_idx_t
					srv_idx)
{
    struct vtss_mpls_segment_server_attach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    ioc.srv_idx = srv_idx;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_mpls_segment_server_attach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_server_attach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_segment_server_detach(const vtss_inst_t inst,
					const vtss_mpls_segment_idx_t idx)
{
    struct vtss_mpls_segment_server_detach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_mpls_segment_server_detach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_segment_server_detach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_alloc(const vtss_inst_t inst,
			   const vtss_mpls_xc_type_t type,
			   vtss_mpls_xc_idx_t * idx)
{
    struct vtss_mpls_xc_alloc_ioc ioc;
    int ret;
    /* Input params start */
    ioc.type = type;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_alloc, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_alloc", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *idx = ioc.idx;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_free(const vtss_inst_t inst,
			  const vtss_mpls_xc_idx_t idx)
{
    struct vtss_mpls_xc_free_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_free, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_free", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_get(const vtss_inst_t inst,
			 const vtss_mpls_xc_idx_t idx, vtss_mpls_xc_t * xc)
{
    struct vtss_mpls_xc_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *xc = ioc.xc;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_set(const vtss_inst_t inst,
			 const vtss_mpls_xc_idx_t idx,
			 const vtss_mpls_xc_t * xc)
{
    struct vtss_mpls_xc_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.idx = idx;
    ioc.xc = *xc;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_segment_attach(const vtss_inst_t inst,
				    const vtss_mpls_xc_idx_t xc_idx,
				    const vtss_mpls_segment_idx_t seg_idx)
{
    struct vtss_mpls_xc_segment_attach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.xc_idx = xc_idx;
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_segment_attach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_segment_attach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_segment_detach(const vtss_inst_t inst,
				    const vtss_mpls_segment_idx_t seg_idx)
{
    struct vtss_mpls_xc_segment_detach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_segment_detach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_segment_detach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_mc_segment_attach(const vtss_inst_t inst,
				       const vtss_mpls_xc_idx_t xc_idx,
				       const vtss_mpls_segment_idx_t
				       seg_idx)
{
    struct vtss_mpls_xc_mc_segment_attach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.xc_idx = xc_idx;
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_mc_segment_attach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_mc_segment_attach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_xc_mc_segment_detach(const vtss_inst_t inst,
				       const vtss_mpls_xc_idx_t xc_idx,
				       const vtss_mpls_segment_idx_t
				       seg_idx)
{
    struct vtss_mpls_xc_mc_segment_detach_ioc ioc;
    int ret;
    /* Input params start */
    ioc.xc_idx = xc_idx;
    ioc.seg_idx = seg_idx;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_xc_mc_segment_detach, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_xc_mc_segment_detach", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_tc_conf_get(const vtss_inst_t inst,
			      vtss_mpls_tc_conf_t * conf)
{
    struct vtss_mpls_tc_conf_get_ioc ioc;
    int ret;
    /* No input args */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_tc_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_tc_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_mpls_tc_conf_set(const vtss_inst_t inst,
			      const vtss_mpls_tc_conf_t * conf)
{
    struct vtss_mpls_tc_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_mpls_tc_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_mpls_tc_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_MPLS */

 /************************************
  * macsec group 
  */

#ifdef VTSS_FEATURE_MACSEC

vtss_rc vtss_macsec_init_set(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     const vtss_macsec_init_t * init)
{
    struct vtss_macsec_init_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.init = *init;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_init_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_init_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_init_get(const vtss_inst_t inst,
			     const vtss_port_no_t port_no,
			     vtss_macsec_init_t * init)
{
    struct vtss_macsec_init_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_init_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_init_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *init = ioc.init;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_conf_add(const vtss_inst_t inst,
				  const vtss_macsec_port_t port,
				  const vtss_macsec_secy_conf_t * conf)
{
    struct vtss_macsec_secy_conf_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_secy_conf_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_conf_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_conf_get(const vtss_inst_t inst,
				  const vtss_macsec_port_t port,
				  vtss_macsec_secy_conf_t * conf)
{
    struct vtss_macsec_secy_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_secy_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_conf_del(const vtss_inst_t inst,
				  const vtss_macsec_port_t port)
{
    struct vtss_macsec_secy_conf_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_secy_conf_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_conf_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_controlled_set(const vtss_inst_t inst,
					const vtss_macsec_port_t port,
					const BOOL enable)
{
    struct vtss_macsec_secy_controlled_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.enable = enable;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_secy_controlled_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_controlled_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_controlled_get(const vtss_inst_t inst,
					const vtss_macsec_port_t port,
					BOOL * enable)
{
    struct vtss_macsec_secy_controlled_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_secy_controlled_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_controlled_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *enable = ioc.enable;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_controlled_get(const vtss_inst_t inst,
					const vtss_macsec_port_t port,
					BOOL * enable)
{
    struct vtss_macsec_secy_controlled_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_secy_controlled_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_controlled_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *enable = ioc.enable;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_port_status_get(const vtss_inst_t inst,
					 const vtss_macsec_port_t port,
					 vtss_macsec_secy_port_status_t *
					 status)
{
    struct vtss_macsec_secy_port_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_secy_port_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_port_status_get",
	       ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sc_add(const vtss_inst_t inst,
			      const vtss_macsec_port_t port,
			      const vtss_macsec_sci_t * sci)
{
    struct vtss_macsec_rx_sc_add_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sc_add, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sc_add", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sc_get_next(const vtss_inst_t inst,
				   const vtss_macsec_port_t port,
				   const vtss_macsec_sci_t * search_sci,
				   vtss_macsec_sci_t * found_sci)
{
    struct vtss_macsec_rx_sc_get_next_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.search_sci = *search_sci;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sc_get_next, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sc_get_next", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *found_sci = ioc.found_sci;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sc_del(const vtss_inst_t inst,
			      const vtss_macsec_port_t port,
			      const vtss_macsec_sci_t * sci)
{
    struct vtss_macsec_rx_sc_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sc_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sc_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sc_status_get(const vtss_inst_t inst,
				     const vtss_macsec_port_t port,
				     const vtss_macsec_sci_t * sci,
				     vtss_macsec_rx_sc_status_t * status)
{
    struct vtss_macsec_rx_sc_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sc_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sc_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sc_set(const vtss_inst_t inst,
			      const vtss_macsec_port_t port)
{
    struct vtss_macsec_tx_sc_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sc_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sc_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sc_del(const vtss_inst_t inst,
			      const vtss_macsec_port_t port)
{
    struct vtss_macsec_tx_sc_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sc_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sc_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sc_status_get(const vtss_inst_t inst,
				     const vtss_macsec_port_t port,
				     vtss_macsec_tx_sc_status_t * status)
{
    struct vtss_macsec_tx_sc_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sc_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sc_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_set(const vtss_inst_t inst,
			      const vtss_macsec_port_t port,
			      const vtss_macsec_sci_t * sci, const u16 an,
			      const u32 lowest_pn,
			      const vtss_macsec_sak_t * sak)
{
    struct vtss_macsec_rx_sa_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    ioc.lowest_pn = lowest_pn;
    ioc.sak = *sak;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_get(const vtss_inst_t inst,
			      const vtss_macsec_port_t port,
			      const vtss_macsec_sci_t * sci, const u16 an,
			      u32 * lowest_pn, vtss_macsec_sak_t * sak,
			      BOOL * active)
{
    struct vtss_macsec_rx_sa_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *lowest_pn = ioc.lowest_pn;
    *sak = ioc.sak;
    *active = ioc.active;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_activate(const vtss_inst_t inst,
				   const vtss_macsec_port_t port,
				   const vtss_macsec_sci_t * sci,
				   const u16 an)
{
    struct vtss_macsec_rx_sa_activate_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_activate, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_activate", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_disable(const vtss_inst_t inst,
				  const vtss_macsec_port_t port,
				  const vtss_macsec_sci_t * sci,
				  const u16 an)
{
    struct vtss_macsec_rx_sa_disable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_disable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_disable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_del(const vtss_inst_t inst,
			      const vtss_macsec_port_t port,
			      const vtss_macsec_sci_t * sci, const u16 an)
{
    struct vtss_macsec_rx_sa_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_lowest_pn_update(const vtss_inst_t inst,
					   const vtss_macsec_port_t port,
					   const vtss_macsec_sci_t * sci,
					   const u16 an,
					   const u32 lowest_pn)
{
    struct vtss_macsec_rx_sa_lowest_pn_update_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    ioc.lowest_pn = lowest_pn;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_rx_sa_lowest_pn_update, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_lowest_pn_update",
	       ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_status_get(const vtss_inst_t inst,
				     const vtss_macsec_port_t port,
				     const vtss_macsec_sci_t * sci,
				     const u16 an,
				     vtss_macsec_rx_sa_status_t * status)
{
    struct vtss_macsec_rx_sa_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_set(const vtss_inst_t inst,
			      const vtss_macsec_port_t port, const u16 an,
			      const u32 next_pn,
			      const BOOL confidentiality,
			      const vtss_macsec_sak_t * sak)
{
    struct vtss_macsec_tx_sa_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    ioc.next_pn = next_pn;
    ioc.confidentiality = confidentiality;
    ioc.sak = *sak;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_get(const vtss_inst_t inst,
			      const vtss_macsec_port_t port, const u16 an,
			      u32 * next_pn, BOOL * confidentiality,
			      vtss_macsec_sak_t * sak, BOOL * active)
{
    struct vtss_macsec_tx_sa_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *next_pn = ioc.next_pn;
    *confidentiality = ioc.confidentiality;
    *sak = ioc.sak;
    *active = ioc.active;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_activate(const vtss_inst_t inst,
				   const vtss_macsec_port_t port,
				   const u16 an)
{
    struct vtss_macsec_tx_sa_activate_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_activate, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_activate", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_disable(const vtss_inst_t inst,
				  const vtss_macsec_port_t port,
				  const u16 an)
{
    struct vtss_macsec_tx_sa_disable_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_disable, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_disable", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_del(const vtss_inst_t inst,
			      const vtss_macsec_port_t port, const u16 an)
{
    struct vtss_macsec_tx_sa_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_status_get(const vtss_inst_t inst,
				     const vtss_macsec_port_t port,
				     const u16 an,
				     vtss_macsec_tx_sa_status_t * status)
{
    struct vtss_macsec_tx_sa_status_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_status_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_status_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *status = ioc.status;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_port_counters_get(const vtss_inst_t inst,
					   const vtss_macsec_port_t port,
					   vtss_macsec_secy_port_counters_t
					   * counters)
{
    struct vtss_macsec_secy_port_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_secy_port_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_port_counters_get",
	       ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_cap_get(const vtss_inst_t inst,
				 const vtss_macsec_port_t port,
				 vtss_macsec_secy_cap_t * cap)
{
    struct vtss_macsec_secy_cap_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_secy_cap_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_cap_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cap = ioc.cap;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_secy_counters_get(const vtss_inst_t inst,
				      const vtss_macsec_port_t port,
				      vtss_macsec_secy_counters_t *
				      counters)
{
    struct vtss_macsec_secy_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_secy_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_secy_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_counters_update(const vtss_inst_t inst,
				    const vtss_port_no_t port_no)
{
    struct vtss_macsec_counters_update_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_counters_update, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_counters_update", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_counters_clear(const vtss_inst_t inst,
				   const vtss_port_no_t port_no)
{
    struct vtss_macsec_counters_clear_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_counters_clear, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_counters_clear", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sc_counters_get(const vtss_inst_t inst,
				       const vtss_macsec_port_t port,
				       const vtss_macsec_sci_t * sci,
				       vtss_macsec_rx_sc_counters_t *
				       counters)
{
    struct vtss_macsec_rx_sc_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sc_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sc_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sc_counters_get(const vtss_inst_t inst,
				       const vtss_macsec_port_t port,
				       vtss_macsec_tx_sc_counters_t *
				       counters)
{
    struct vtss_macsec_tx_sc_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sc_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sc_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_tx_sa_counters_get(const vtss_inst_t inst,
				       const vtss_macsec_port_t port,
				       const u16 an,
				       vtss_macsec_tx_sa_counters_t *
				       counters)
{
    struct vtss_macsec_tx_sa_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_tx_sa_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_tx_sa_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_rx_sa_counters_get(const vtss_inst_t inst,
				       const vtss_macsec_port_t port,
				       const vtss_macsec_sci_t * sci,
				       const u16 an,
				       vtss_macsec_rx_sa_counters_t *
				       counters)
{
    struct vtss_macsec_rx_sa_counters_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.sci = *sci;
    ioc.an = an;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_rx_sa_counters_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_rx_sa_counters_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *counters = ioc.counters;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_control_frame_match_conf_set(const vtss_inst_t inst,
						 const vtss_port_no_t port,
						 const
						 vtss_macsec_control_frame_match_conf_t
						 * conf)
{
    struct vtss_macsec_control_frame_match_conf_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_control_frame_match_conf_set,
	       &ioc)) != 0) {
	printf("%s: Returns %d\n",
	       "vtss_macsec_control_frame_match_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_control_frame_match_conf_get(const vtss_inst_t inst,
						 const vtss_port_no_t port,
						 vtss_macsec_control_frame_match_conf_t
						 * conf)
{
    struct vtss_macsec_control_frame_match_conf_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret =
	 ioctl(fd, SWIOC_vtss_macsec_control_frame_match_conf_get,
	       &ioc)) != 0) {
	printf("%s: Returns %d\n",
	       "vtss_macsec_control_frame_match_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_pattern_set(const vtss_inst_t inst,
				const vtss_macsec_port_t port,
				const vtss_macsec_direction_t direction,
				const vtss_macsec_match_action_t action,
				const vtss_macsec_match_pattern_t *
				pattern)
{
    struct vtss_macsec_pattern_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.direction = direction;
    ioc.action = action;
    ioc.pattern = *pattern;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_pattern_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_pattern_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_pattern_del(const vtss_inst_t inst,
				const vtss_macsec_port_t port,
				const vtss_macsec_direction_t direction,
				const vtss_macsec_match_action_t action)
{
    struct vtss_macsec_pattern_del_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.direction = direction;
    ioc.action = action;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_pattern_del, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_pattern_del", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_pattern_get(const vtss_inst_t inst,
				const vtss_macsec_port_t port,
				const vtss_macsec_direction_t direction,
				const vtss_macsec_match_action_t action,
				vtss_macsec_match_pattern_t * pattern)
{
    struct vtss_macsec_pattern_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.direction = direction;
    ioc.action = action;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_pattern_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_pattern_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *pattern = ioc.pattern;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_default_action_set(const vtss_inst_t inst,
				       const vtss_port_no_t port,
				       const
				       vtss_macsec_default_action_policy_t
				       * pattern)
{
    struct vtss_macsec_default_action_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.pattern = *pattern;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_default_action_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_default_action_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_default_action_get(const vtss_inst_t inst,
				       const vtss_port_no_t port,
				       vtss_macsec_default_action_policy_t
				       * pattern)
{
    struct vtss_macsec_default_action_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_default_action_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_default_action_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *pattern = ioc.pattern;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_bypass_mode_set(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    const vtss_macsec_bypass_mode_t mode)
{
    struct vtss_macsec_bypass_mode_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.mode = mode;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_bypass_mode_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_bypass_mode_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_bypass_mode_get(const vtss_inst_t inst,
				    const vtss_port_no_t port_no,
				    vtss_macsec_bypass_mode_t * mode)
{
    struct vtss_macsec_bypass_mode_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_bypass_mode_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_bypass_mode_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *mode = ioc.mode;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_bypass_tag_set(const vtss_inst_t inst,
				   const vtss_macsec_port_t port,
				   const vtss_macsec_tag_bypass_t tag)
{
    struct vtss_macsec_bypass_tag_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    ioc.tag = tag;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_bypass_tag_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_bypass_tag_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_bypass_tag_get(const vtss_inst_t inst,
				   const vtss_macsec_port_t port,
				   vtss_macsec_tag_bypass_t * tag)
{
    struct vtss_macsec_bypass_tag_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port = port;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_bypass_tag_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_bypass_tag_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *tag = ioc.tag;
    /* Output params end */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_frame_capture_set(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const vtss_macsec_frame_capture_t
				      capture)
{
    struct vtss_macsec_frame_capture_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.capture = capture;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_frame_capture_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_frame_capture_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_port_loopback_set(const vtss_inst_t inst,
				      const vtss_port_no_t port_no,
				      const BOOL enable)
{
    struct vtss_macsec_port_loopback_set_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.enable = enable;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_port_loopback_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_port_loopback_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}



vtss_rc vtss_macsec_frame_get(const vtss_inst_t inst,
			      const vtss_port_no_t port_no,
			      const u32 buf_length, u32 * return_length,
			      u8 * frame)
{
    struct vtss_macsec_frame_get_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.buf_length = buf_length;
    /* Input params end */
    if ((ret = ioctl(fd, SWIOC_vtss_macsec_frame_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_macsec_frame_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *return_length = ioc.return_length;
    *frame = ioc.frame;
    /* Output params end */
    return VTSS_RC_OK;
}


#endif				/* VTSS_FEATURE_MACSEC */
