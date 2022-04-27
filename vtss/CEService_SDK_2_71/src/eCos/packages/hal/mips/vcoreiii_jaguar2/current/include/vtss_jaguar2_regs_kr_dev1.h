#ifndef _VTSS_JAGUAR2_REGS_KR_DEV1_H_
#define _VTSS_JAGUAR2_REGS_KR_DEV1_H_

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

#define VTSS_KR_DEV1_KR_1x0096_KR_1x0096(target)  VTSS_IOREG(target,0x6)
#define  VTSS_F_KR_DEV1_KR_1x0096_KR_1x0096_tr_enable  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_KR_1x0096_KR_1x0096_tr_restart  VTSS_BIT(0)

#define VTSS_KR_DEV1_KR_1x0097_KR_1x0097(target)  VTSS_IOREG(target,0xf)
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_tr_fail  VTSS_BIT(3)
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_stprot  VTSS_BIT(2)
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_frlock  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_rcvr_rdy  VTSS_BIT(0)

#define VTSS_KR_DEV1_KR_1x0098_KR_1x0098(target)  VTSS_IOREG(target,0x7)
#define  VTSS_F_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_KR_1x0099_KR_1x0099(target)  VTSS_IOREG(target,0x8)
#define  VTSS_F_KR_DEV1_KR_1x0099_KR_1x0099_lpstat(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x0099_KR_1x0099_lpstat     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x0099_KR_1x0099_lpstat(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_KR_1x009A_KR_1x009A(target)  VTSS_IOREG(target,0x9)
#define  VTSS_F_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_KR_1x009B_KR_1x009B(target)  VTSS_IOREG(target,0xa)
#define  VTSS_F_KR_DEV1_KR_1x009B_KR_1x009B_ldstat(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x009B_KR_1x009B_ldstat     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x009B_KR_1x009B_ldstat(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_cfg0_tr_cfg0(target)  VTSS_IOREG(target,0x10)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr(x)  VTSS_EXTRACT_BITFIELD(x,12,4)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_apc_drct_en  VTSS_BIT(11)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_rx_inv  VTSS_BIT(10)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tx_inv  VTSS_BIT(9)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_ld_pre_init  VTSS_BIT(4)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_lp_pre_init  VTSS_BIT(3)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_nosum  VTSS_BIT(2)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_part_cfg_en  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tapctl_en  VTSS_BIT(0)

#define VTSS_KR_DEV1_tr_cfg1_tr_cfg1(target)  VTSS_IOREG(target,0x11)
#define  VTSS_F_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

#define VTSS_KR_DEV1_tr_cfg2_tr_cfg2(target)  VTSS_IOREG(target,0x12)
#define  VTSS_F_KR_DEV1_tr_cfg2_tr_cfg2_vp_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg2_tr_cfg2_vp_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg2_tr_cfg2_vp_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg2_tr_cfg2_v2_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg2_tr_cfg2_v2_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg2_tr_cfg2_v2_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg3_tr_cfg3(target)  VTSS_IOREG(target,0x13)
#define  VTSS_F_KR_DEV1_tr_cfg3_tr_cfg3_cp_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg3_tr_cfg3_cp_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg3_tr_cfg3_cp_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg3_tr_cfg3_cp_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg3_tr_cfg3_cp_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg3_tr_cfg3_cp_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg4_tr_cfg4(target)  VTSS_IOREG(target,0x14)
#define  VTSS_F_KR_DEV1_tr_cfg4_tr_cfg4_c0_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg4_tr_cfg4_c0_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg4_tr_cfg4_c0_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg4_tr_cfg4_c0_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg4_tr_cfg4_c0_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg4_tr_cfg4_c0_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg5_tr_cfg5(target)  VTSS_IOREG(target,0x15)
#define  VTSS_F_KR_DEV1_tr_cfg5_tr_cfg5_cm_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg5_tr_cfg5_cm_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg5_tr_cfg5_cm_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg5_tr_cfg5_cm_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg5_tr_cfg5_cm_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg5_tr_cfg5_cm_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg6_tr_cfg6(target)  VTSS_IOREG(target,0x16)
#define  VTSS_F_KR_DEV1_tr_cfg6_tr_cfg6_cp_init(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg6_tr_cfg6_cp_init     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg6_tr_cfg6_cp_init(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg6_tr_cfg6_c0_init(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg6_tr_cfg6_c0_init     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg6_tr_cfg6_c0_init(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg7_tr_cfg7(target)  VTSS_IOREG(target,0x17)
#define  VTSS_F_KR_DEV1_tr_cfg7_tr_cfg7_cm_init(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg7_tr_cfg7_cm_init     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg7_tr_cfg7_cm_init(x)  VTSS_EXTRACT_BITFIELD(x,6,6)
#define  VTSS_F_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_cfg8_tr_cfg8(target)  VTSS_IOREG(target,0x18)
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt1(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt1     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt1(x)  VTSS_EXTRACT_BITFIELD(x,6,2)
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt2(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt2     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt2(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt3(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt3     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt3(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt4(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt4     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt4(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_KR_DEV1_tr_cfg9_tr_cfg9(target)  VTSS_IOREG(target,0x19)
#define  VTSS_F_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_gain_tr_gain(target)  VTSS_IOREG(target,0x1a)
#define  VTSS_F_KR_DEV1_tr_gain_tr_gain_gain_marg(x)  VTSS_ENCODE_BITFIELD(x,10,6)
#define  VTSS_M_KR_DEV1_tr_gain_tr_gain_gain_marg     VTSS_ENCODE_BITMASK(10,6)
#define  VTSS_X_KR_DEV1_tr_gain_tr_gain_gain_marg(x)  VTSS_EXTRACT_BITFIELD(x,10,6)
#define  VTSS_F_KR_DEV1_tr_gain_tr_gain_gain_targ(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_KR_DEV1_tr_gain_tr_gain_gain_targ     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_KR_DEV1_tr_gain_tr_gain_gain_targ(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

#define VTSS_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd(target)  VTSS_IOREG(target,0x1b)
#define  VTSS_F_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd(target)  VTSS_IOREG(target,0x1c)
#define  VTSS_F_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_ovrd_tr_ovrd(target)  VTSS_IOREG(target,0xb)
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_ovrd_en  VTSS_BIT(4)
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_rxtrained_ovrd  VTSS_BIT(3)
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_ber_en_ovrd  VTSS_BIT(2)
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_coef_ovrd_vld  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_stat_ovrd_vld  VTSS_BIT(0)

#define VTSS_KR_DEV1_tr_step_tr_step(target)  VTSS_IOREG(target,0xc)
#define  VTSS_F_KR_DEV1_tr_step_tr_step_step  VTSS_BIT(0)

#define VTSS_KR_DEV1_tr_mthd_tr_mthd(target)  VTSS_IOREG(target,0x1d)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_cp(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_cp     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_cp(x)  VTSS_EXTRACT_BITFIELD(x,10,2)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_c0(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_c0     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_c0(x)  VTSS_EXTRACT_BITFIELD(x,8,2)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_cm(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_cm     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_cm(x)  VTSS_EXTRACT_BITFIELD(x,6,2)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord1(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord1     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord1(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord2(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord2     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord2(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord3(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord3     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord3(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_KR_DEV1_tr_ber_thr_tr_ber_thr(target)  VTSS_IOREG(target,0x1e)
#define  VTSS_F_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th(x)  VTSS_EXTRACT_BITFIELD(x,8,8)
#define  VTSS_F_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_KR_DEV1_tr_ber_ofs_tr_ber_ofs(target)  VTSS_IOREG(target,0x1f)
#define  VTSS_F_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_KR_DEV1_tr_lutsel_tr_lutsel(target)  VTSS_IOREG(target,0x20)
#define  VTSS_F_KR_DEV1_tr_lutsel_tr_lutsel_lut_row(x)  VTSS_ENCODE_BITFIELD(x,3,6)
#define  VTSS_M_KR_DEV1_tr_lutsel_tr_lutsel_lut_row     VTSS_ENCODE_BITMASK(3,6)
#define  VTSS_X_KR_DEV1_tr_lutsel_tr_lutsel_lut_row(x)  VTSS_EXTRACT_BITFIELD(x,3,6)
#define  VTSS_F_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_KR_DEV1_tr_brkmask_brkmask_lsw(target)  VTSS_IOREG(target,0x21)
#define  VTSS_F_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_brkmask_tr_brkmask_msw(target)  VTSS_IOREG(target,0x22)
#define  VTSS_F_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_romadr_romadr1(target)  VTSS_IOREG(target,0x0)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr1_romadr_gain1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr1_romadr_gain1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr1_romadr_gain1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr1_romadr_gain2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr1_romadr_gain2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr1_romadr_gain2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_romadr_romadr2(target)  VTSS_IOREG(target,0x1)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr2_romadr_dfe1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr2_romadr_dfe1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr2_romadr_dfe1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr2_romadr_dfe2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr2_romadr_dfe2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr2_romadr_dfe2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_romadr_romadr3(target)  VTSS_IOREG(target,0x2)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr3_romadr_ber1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr3_romadr_ber1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr3_romadr_ber1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr3_romadr_ber2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr3_romadr_ber2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr3_romadr_ber2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_romadr_romadr4(target)  VTSS_IOREG(target,0x3)
#define  VTSS_F_KR_DEV1_tr_romadr_romadr4_romadr_end(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr4_romadr_end     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr4_romadr_end(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_obcfg_addr_obcfg_addr(target)  VTSS_IOREG(target,0x23)
#define  VTSS_F_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_apc_tmr_apc_tmr(target)  VTSS_IOREG(target,0x24)
#define  VTSS_F_KR_DEV1_apc_tmr_apc_tmr_apc_tmr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_apc_tmr_apc_tmr_apc_tmr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_apc_tmr_apc_tmr_apc_tmr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_wt_tmr_wt_tmr(target)   VTSS_IOREG(target,0x25)
#define  VTSS_F_KR_DEV1_wt_tmr_wt_tmr_wt_tmr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_wt_tmr_wt_tmr_wt_tmr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_wt_tmr_wt_tmr_wt_tmr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_mw_tmr_mw_tmr_lsw(target)  VTSS_IOREG(target,0x26)
#define  VTSS_F_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_mw_tmr_mw_tmr_msw(target)  VTSS_IOREG(target,0x27)
#define  VTSS_F_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_sts1_tr_sts1(target)  VTSS_IOREG(target,0xd)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_ber_busy  VTSS_BIT(12)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_tr_sm(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_KR_DEV1_tr_sts1_tr_sts1_tr_sm     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_KR_DEV1_tr_sts1_tr_sts1_tr_sm(x)  VTSS_EXTRACT_BITFIELD(x,9,3)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm(x)  VTSS_ENCODE_BITFIELD(x,4,5)
#define  VTSS_M_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm     VTSS_ENCODE_BITMASK(4,5)
#define  VTSS_X_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm(x)  VTSS_EXTRACT_BITFIELD(x,4,5)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_gain_fail  VTSS_BIT(3)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_training  VTSS_BIT(2)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_dme_viol  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_tr_done  VTSS_BIT(0)

#define VTSS_KR_DEV1_tr_sts2_tr_sts2(target)  VTSS_IOREG(target,0xe)
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_cp_range_err  VTSS_BIT(2)
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_c0_range_err  VTSS_BIT(1)
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_cm_range_err  VTSS_BIT(0)

#define VTSS_KR_DEV1_tr_tapval_tr_cmval(target)  VTSS_IOREG(target,0x28)
#define  VTSS_F_KR_DEV1_tr_tapval_tr_cmval_cm_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_cmval_cm_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_cmval_cm_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_tapval_tr_c0val(target)  VTSS_IOREG(target,0x29)
#define  VTSS_F_KR_DEV1_tr_tapval_tr_c0val_c0_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_c0val_c0_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_c0val_c0_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_tapval_tr_cpval(target)  VTSS_IOREG(target,0x2a)
#define  VTSS_F_KR_DEV1_tr_tapval_tr_cpval_cp_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_cpval_cp_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_cpval_cp_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_KR_DEV1_tr_frames_sent_frsent_lsw(target)  VTSS_IOREG(target,0x4)
#define  VTSS_F_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_frames_sent_frsent_msw(target)  VTSS_IOREG(target,0x5)
#define  VTSS_F_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_lut_lut_lsw(target)  VTSS_IOREG(target,0x2b)
#define  VTSS_F_KR_DEV1_tr_lut_lut_lsw_lut_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_lut_lut_lsw_lut_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_lut_lut_lsw_lut_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_lut_lut_msw(target)  VTSS_IOREG(target,0x2c)
#define  VTSS_F_KR_DEV1_tr_lut_lut_msw_lut_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_lut_lut_msw_lut_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_lut_lut_msw_lut_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_KR_DEV1_tr_errcnt_tr_errcnt(target)  VTSS_IOREG(target,0x2d)
#define  VTSS_F_KR_DEV1_tr_errcnt_tr_errcnt_errcnt(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_errcnt_tr_errcnt_errcnt     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_errcnt_tr_errcnt_errcnt(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR2_REGS_KR_DEV1_H_ */
