#ifndef _VTSS_JAGUAR2_REGS_KR_DEV7_H_
#define _VTSS_JAGUAR2_REGS_KR_DEV7_H_

/*
 *
 * VCore-III Register Definitions
 *
 * Copyright (C) 2012 Vitesse Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "vtss_jaguar2_regs_common.h"

#define VTSS_KR_DEV7_KR_7x0000_KR_7x0000(target)  VTSS_IOREG(target,0x84)
#define  VTSS_F_KR_DEV7_KR_7x0000_KR_7x0000_an_reset  VTSS_BIT(15)
#define  VTSS_F_KR_DEV7_KR_7x0000_KR_7x0000_npctl  VTSS_BIT(13)
#define  VTSS_F_KR_DEV7_KR_7x0000_KR_7x0000_an_enable  VTSS_BIT(12)
#define  VTSS_F_KR_DEV7_KR_7x0000_KR_7x0000_an_restart  VTSS_BIT(9)

#define VTSS_KR_DEV7_KR_7x0001_KR_7x0001(target)  VTSS_IOREG(target,0x85)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_pardetflt  VTSS_BIT(9)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_npstat  VTSS_BIT(7)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_pg_rcvd  VTSS_BIT(6)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_an_complete  VTSS_BIT(5)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_rem_flt  VTSS_BIT(4)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_an_able  VTSS_BIT(3)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_linkstat  VTSS_BIT(2)
#define  VTSS_F_KR_DEV7_KR_7x0001_KR_7x0001_an_lp_able  VTSS_BIT(0)

#define VTSS_KR_DEV7_LD_adv_KR_7x0010(target)  VTSS_IOREG(target,0x8b)
#define  VTSS_F_KR_DEV7_LD_adv_KR_7x0010_adv0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_adv_KR_7x0010_adv0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_adv_KR_7x0010_adv0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LD_adv_KR_7x0011(target)  VTSS_IOREG(target,0x8c)
#define  VTSS_F_KR_DEV7_LD_adv_KR_7x0011_adv1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_adv_KR_7x0011_adv1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_adv_KR_7x0011_adv1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LD_adv_KR_7x0012(target)  VTSS_IOREG(target,0x8d)
#define  VTSS_F_KR_DEV7_LD_adv_KR_7x0012_adv2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_adv_KR_7x0012_adv2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_adv_KR_7x0012_adv2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_base_page_0_KR_7x0013(target)  VTSS_IOREG(target,0x86)
#define  VTSS_F_KR_DEV7_LP_base_page_0_KR_7x0013_lp_bp_adv0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_base_page_0_KR_7x0013_lp_bp_adv0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_base_page_0_KR_7x0013_lp_bp_adv0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_base_page_1_KR_7x0014(target)  VTSS_IOREG(target,0x87)
#define  VTSS_F_KR_DEV7_LP_base_page_1_KR_7x0014_lp_bp_adv1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_base_page_1_KR_7x0014_lp_bp_adv1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_base_page_1_KR_7x0014_lp_bp_adv1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_base_page_2_KR_7x0015(target)  VTSS_IOREG(target,0x88)
#define  VTSS_F_KR_DEV7_LP_base_page_2_KR_7x0015_lp_bp_adv2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_base_page_2_KR_7x0015_lp_bp_adv2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_base_page_2_KR_7x0015_lp_bp_adv2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LD_next_page_KR_7x0016(target)  VTSS_IOREG(target,0x8e)
#define  VTSS_F_KR_DEV7_LD_next_page_KR_7x0016_np_tx0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_next_page_KR_7x0016_np_tx0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_next_page_KR_7x0016_np_tx0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LD_next_page_KR_7x0017(target)  VTSS_IOREG(target,0x8f)
#define  VTSS_F_KR_DEV7_LD_next_page_KR_7x0017_np_tx1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_next_page_KR_7x0017_np_tx1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_next_page_KR_7x0017_np_tx1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LD_next_page_KR_7x0018(target)  VTSS_IOREG(target,0x90)
#define  VTSS_F_KR_DEV7_LD_next_page_KR_7x0018_np_tx2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LD_next_page_KR_7x0018_np_tx2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LD_next_page_KR_7x0018_np_tx2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_next_page_KR_7x0019(target)  VTSS_IOREG(target,0x80)
#define  VTSS_F_KR_DEV7_LP_next_page_KR_7x0019_lp_np_adv0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_next_page_KR_7x0019_lp_np_adv0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_next_page_KR_7x0019_lp_np_adv0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_next_page_KR_7x001A(target)  VTSS_IOREG(target,0x81)
#define  VTSS_F_KR_DEV7_LP_next_page_KR_7x001A_lp_np_adv1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_next_page_KR_7x001A_lp_np_adv1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_next_page_KR_7x001A_lp_np_adv1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_LP_next_page_KR_7x001B(target)  VTSS_IOREG(target,0x82)
#define  VTSS_F_KR_DEV7_LP_next_page_KR_7x001B_lp_np_adv2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_LP_next_page_KR_7x001B_lp_np_adv2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_LP_next_page_KR_7x001B_lp_np_adv2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_KR_7x0030_KR_7x0030(target)  VTSS_IOREG(target,0x91)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_cr10  VTSS_BIT(8)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_cr4  VTSS_BIT(6)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_kr4  VTSS_BIT(5)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_fec  VTSS_BIT(4)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_kr  VTSS_BIT(3)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_kx4  VTSS_BIT(2)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_neg_kx  VTSS_BIT(1)
#define  VTSS_F_KR_DEV7_KR_7x0030_KR_7x0030_an_bp_able  VTSS_BIT(0)

#define VTSS_KR_DEV7_an_cfg0_an_cfg0(target)  VTSS_IOREG(target,0x92)
#define  VTSS_F_KR_DEV7_an_cfg0_an_cfg0_an_sm_hist_clr  VTSS_BIT(5)
#define  VTSS_F_KR_DEV7_an_cfg0_an_cfg0_clkg_disable  VTSS_BIT(4)
#define  VTSS_F_KR_DEV7_an_cfg0_an_cfg0_tr_disable  VTSS_BIT(3)
#define  VTSS_F_KR_DEV7_an_cfg0_an_cfg0_sync10g_sel  VTSS_BIT(2)
#define  VTSS_F_KR_DEV7_an_cfg0_an_cfg0_sync8b10b_sel  VTSS_BIT(1)

#define VTSS_KR_DEV7_bl_tmr_bl_lsw(target)   VTSS_IOREG(target,0x93)
#define  VTSS_F_KR_DEV7_bl_tmr_bl_lsw_bl_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_bl_tmr_bl_lsw_bl_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_bl_tmr_bl_lsw_bl_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_bl_tmr_bl_msw(target)   VTSS_IOREG(target,0x94)
#define  VTSS_F_KR_DEV7_bl_tmr_bl_msw_bl_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_bl_tmr_bl_msw_bl_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_bl_tmr_bl_msw_bl_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_aw_tmr_aw_lsw(target)   VTSS_IOREG(target,0x95)
#define  VTSS_F_KR_DEV7_aw_tmr_aw_lsw_aw_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_aw_tmr_aw_lsw_aw_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_aw_tmr_aw_lsw_aw_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_aw_tmr_aw_msw(target)   VTSS_IOREG(target,0x96)
#define  VTSS_F_KR_DEV7_aw_tmr_aw_msw_aw_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_aw_tmr_aw_msw_aw_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_aw_tmr_aw_msw_aw_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lflong_tmr_lflong_lsw(target)  VTSS_IOREG(target,0x97)
#define  VTSS_F_KR_DEV7_lflong_tmr_lflong_lsw_lflong_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lflong_tmr_lflong_lsw_lflong_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lflong_tmr_lflong_lsw_lflong_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lflong_tmr_lflong_msw(target)  VTSS_IOREG(target,0x98)
#define  VTSS_F_KR_DEV7_lflong_tmr_lflong_msw_lflong_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lflong_tmr_lflong_msw_lflong_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lflong_tmr_lflong_msw_lflong_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lfshort_tmr_lfshort_lsw(target)  VTSS_IOREG(target,0x99)
#define  VTSS_F_KR_DEV7_lfshort_tmr_lfshort_lsw_lfshort_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lfshort_tmr_lfshort_lsw_lfshort_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lfshort_tmr_lfshort_lsw_lfshort_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lfshort_tmr_lfshort_msw(target)  VTSS_IOREG(target,0x9a)
#define  VTSS_F_KR_DEV7_lfshort_tmr_lfshort_msw_lfshort_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lfshort_tmr_lfshort_msw_lfshort_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lfshort_tmr_lfshort_msw_lfshort_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lp_tmr_lp_lsw(target)   VTSS_IOREG(target,0x9b)
#define  VTSS_F_KR_DEV7_lp_tmr_lp_lsw_lp_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lp_tmr_lp_lsw_lp_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lp_tmr_lp_lsw_lp_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_lp_tmr_lp_msw(target)   VTSS_IOREG(target,0x9c)
#define  VTSS_F_KR_DEV7_lp_tmr_lp_msw_lp_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_lp_tmr_lp_msw_lp_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_lp_tmr_lp_msw_lp_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_pd_tmr_pd_lsw(target)   VTSS_IOREG(target,0x9d)
#define  VTSS_F_KR_DEV7_pd_tmr_pd_lsw_pd_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_pd_tmr_pd_lsw_pd_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_pd_tmr_pd_lsw_pd_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_pd_tmr_pd_msw(target)   VTSS_IOREG(target,0x9e)
#define  VTSS_F_KR_DEV7_pd_tmr_pd_msw_pd_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_pd_tmr_pd_msw_pd_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_pd_tmr_pd_msw_pd_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr10g_tmr_kr10g_lsw(target)  VTSS_IOREG(target,0x9f)
#define  VTSS_F_KR_DEV7_kr10g_tmr_kr10g_lsw_kr10g_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr10g_tmr_kr10g_lsw_kr10g_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr10g_tmr_kr10g_lsw_kr10g_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr10g_tmr_kr10g_msw(target)  VTSS_IOREG(target,0xa0)
#define  VTSS_F_KR_DEV7_kr10g_tmr_kr10g_msw_kr10g_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr10g_tmr_kr10g_msw_kr10g_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr10g_tmr_kr10g_msw_kr10g_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr3g_tmr_kr3g_lsw(target)  VTSS_IOREG(target,0xa1)
#define  VTSS_F_KR_DEV7_kr3g_tmr_kr3g_lsw_kr3g_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr3g_tmr_kr3g_lsw_kr3g_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr3g_tmr_kr3g_lsw_kr3g_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr3g_tmr_kr3g_msw(target)  VTSS_IOREG(target,0xa2)
#define  VTSS_F_KR_DEV7_kr3g_tmr_kr3g_msw_kr3g_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr3g_tmr_kr3g_msw_kr3g_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr3g_tmr_kr3g_msw_kr3g_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr1g_tmr_kr1g_lsw(target)  VTSS_IOREG(target,0xa3)
#define  VTSS_F_KR_DEV7_kr1g_tmr_kr1g_lsw_kr1g_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr1g_tmr_kr1g_lsw_kr1g_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr1g_tmr_kr1g_lsw_kr1g_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_kr1g_tmr_kr1g_msw(target)  VTSS_IOREG(target,0xa4)
#define  VTSS_F_KR_DEV7_kr1g_tmr_kr1g_msw_kr1g_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_kr1g_tmr_kr1g_msw_kr1g_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_kr1g_tmr_kr1g_msw_kr1g_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_an_hist_an_hist(target)  VTSS_IOREG(target,0xa5)
#define  VTSS_F_KR_DEV7_an_hist_an_hist_an_sm_hist(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_KR_DEV7_an_hist_an_hist_an_sm_hist     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_KR_DEV7_an_hist_an_hist_an_sm_hist(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

#define VTSS_KR_DEV7_an_sm_an_sm(target)     VTSS_IOREG(target,0x89)
#define  VTSS_F_KR_DEV7_an_sm_an_sm_an_sm(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_KR_DEV7_an_sm_an_sm_an_sm     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_KR_DEV7_an_sm_an_sm_an_sm(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_KR_DEV7_an_sts0_an_sts0(target)  VTSS_IOREG(target,0x8a)
#define  VTSS_F_KR_DEV7_an_sts0_an_sts0_nonce_match  VTSS_BIT(8)
#define  VTSS_F_KR_DEV7_an_sts0_an_sts0_incp_link  VTSS_BIT(7)
#define  VTSS_F_KR_DEV7_an_sts0_an_sts0_link_hcd(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_KR_DEV7_an_sts0_an_sts0_link_hcd     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_KR_DEV7_an_sts0_an_sts0_link_hcd(x)  VTSS_EXTRACT_BITFIELD(x,4,3)
#define  VTSS_F_KR_DEV7_an_sts0_an_sts0_link_ctl(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_KR_DEV7_an_sts0_an_sts0_link_ctl     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_KR_DEV7_an_sts0_an_sts0_link_ctl(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_KR_DEV7_an_sts0_an_sts0_line_rate(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_KR_DEV7_an_sts0_an_sts0_line_rate     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_KR_DEV7_an_sts0_an_sts0_line_rate(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_KR_DEV7_irom_irom_lsw(target,ri)  VTSS_IOREG(target,0x0 + (ri))
#define  VTSS_F_KR_DEV7_irom_irom_lsw_irom_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_irom_irom_lsw_irom_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_irom_irom_lsw_irom_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_irom_irom_msw(target,ri)  VTSS_IOREG(target,0x20 + (ri))
#define  VTSS_F_KR_DEV7_irom_irom_msw_irom_msw(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_KR_DEV7_irom_irom_msw_irom_msw     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_KR_DEV7_irom_irom_msw_irom_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_KR_DEV7_drom_drom_lsw(target,ri)  VTSS_IOREG(target,0x40 + (ri))
#define  VTSS_F_KR_DEV7_drom_drom_lsw_drom_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_drom_drom_lsw_drom_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_drom_drom_lsw_drom_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV7_drom_drom_msw(target,ri)  VTSS_IOREG(target,0x60 + (ri))
#define  VTSS_F_KR_DEV7_drom_drom_msw_drom_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV7_drom_drom_msw_drom_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV7_drom_drom_msw_drom_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR2_REGS_KR_DEV7_H_ */
