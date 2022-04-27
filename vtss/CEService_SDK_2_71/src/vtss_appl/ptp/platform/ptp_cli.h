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

#ifndef __PTP_CLI_H__
#define __PTP_CLI_H__

void ptp_cli_req_init (void);

typedef int vtss_ptp_cli_pr(        const char                  *fmt,
                                    ...);

void ptp_show_clock_default_ds(int inst, vtss_ptp_cli_pr *pr);
void ptp_show_clock_current_ds(int inst, vtss_ptp_cli_pr *pr);
void ptp_show_clock_parent_ds(int inst, vtss_ptp_cli_pr *pr);
void ptp_show_clock_time_property_ds(int inst, vtss_ptp_cli_pr *pr);
void ptp_show_clock_filter_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_servo_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_clk_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_ho_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_uni_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_master_table_unicast_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_slave_ds(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_port_state_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr);

void ptp_show_clock_port_ds_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr);

void ptp_show_clock_wireless_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr);

void ptp_show_clock_foreign_master_record_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr);

void ptp_show_ext_clock_mode(vtss_ptp_cli_pr *pr);

void ptp_show_local_clock(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_slave_cfg(int inst, vtss_ptp_cli_pr *pr);

void ptp_show_clock_slave_table_unicast_ds(int inst, vtss_ptp_cli_pr *pr);

#if defined(VTSS_ARCH_SERVAL)
void ptp_show_rs422_clock_mode(vtss_ptp_cli_pr *pr);
#endif


#endif /* __PTP_CLI_H__ */
