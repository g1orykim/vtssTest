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

 $Id$
 $Revision$

*/

#ifndef _VTSS_SERVAL_CIL_H_
#define _VTSS_SERVAL_CIL_H_

/* Use relative DWORD addresses for registers - must be first */
#define VTSS_IOADDR(t,o) ((((t) - VTSS_IO_ORIGIN1_OFFSET) >> 2) + (o))
#define VTSS_IOREG(t,o)  (VTSS_IOADDR(t,o))

// Avoid "vtss_serval_cil.h not used in module vtss_serval.c"
/*lint --e{766} */

#include "vtss_api.h"

#if defined(VTSS_ARCH_SERVAL)
#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL
#include "../ail/vtss_state.h"
#include "../ail/vtss_common.h"
#include "../ail/vtss_util.h"
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
#include "vtss_serval_fdma.h"
#endif
#include "vtss_serval.h"
#include "vtss_serval_reg.h"

#if defined(VTSS_ARCH_SEVILLE)
#define VTSS_CHIP_PORTS      10    /* Port 0-9 */
#else
#define VTSS_CHIP_PORTS      11    /* Port 0-10 */
#endif /* VTSS_ARCH_SEVILLE */
#define VTSS_CHIP_PORT_CPU   VTSS_CHIP_PORTS /* Next port is CPU port */
#define VTSS_CHIP_PORT_CPU_0 (VTSS_CHIP_PORT_CPU + 0) /* Aka. CPU Port 11 */
#define VTSS_CHIP_PORT_CPU_1 (VTSS_CHIP_PORT_CPU + 1) /* Aka. CPU Port 12 */
#define VTSS_CHIP_PORT_MASK  VTSS_BITMASK(VTSS_CHIP_PORTS) /* Chip port mask */

/* Policers */
#define SRVL_POLICER_PORT    0    /* 0-11    : Port policers (0-10 used, 11 unused) */
#define SRVL_POLICER_ACL     12   /* 12-31   : ACL policers (12-27 used, 28-31 unused) */
#define SRVL_POLICER_QUEUE   32   /* 32-127  : Queue policers (32-119 used, 120-127 unused) */ 
#if defined(VTSS_CHIP_SERVAL)
#define SRVL_POLICER_EVC     129  /* 129-1150: EVC policers (128 unused) */
#define SRVL_POLICER_DISCARD 1151 /* 1151    : Discard policer */
#define SRVL_POLICER_CNT     1152 /* Total number of policers */
#elif defined(VTSS_CHIP_SERVAL_LITE)
#define SRVL_POLICER_EVC     129  /* 129-384 : EVC policers (128 unused) */
#define SRVL_POLICER_DISCARD 385  /* 385     : Discard policer */
#define SRVL_POLICER_CNT     386  /* Total number of policers */
#else
#define SRVL_POLICER_EVC     129  /* 129-192 : EVC policers (128 unused) */
#define SRVL_POLICER_DISCARD 193  /* 193     : Discard policer */
#define SRVL_POLICER_CNT     194  /* Total number of policers */
#endif /* VTSS_CHIP_SERVAL */

/* Buffer constants */
#define SRVL_BUFFER_MEMORY 1024000
#define SRVL_BUFFER_REFERENCE 11000
#define SRVL_BUFFER_CELL_SZ 60

/* Number of full entries */
#define SRVL_IS0_CNT 384
#define SRVL_IS1_CNT 256
#define SRVL_IS2_CNT 256
#define SRVL_ES0_CNT 1024

#define SRVL_ACS          16  /* Number of aggregation masks */
#define SRVL_PRIOS        8   /* Number of priorities */
#define SRVL_GPIOS        32  /* Number of GPIOs */
#define SRVL_SGPIO_GROUPS 1   /* Number of SGPIO groups */

/* Reserved PGIDs */
#define PGID_UC      (VTSS_PGID_LUTON26 - 4)
#define PGID_MC      (VTSS_PGID_LUTON26 - 3)
#define PGID_MCIPV4  (VTSS_PGID_LUTON26 - 2)
#define PGID_MCIPV6  (VTSS_PGID_LUTON26 - 1)
#define PGID_AGGR    (VTSS_PGID_LUTON26)
#define PGID_SRC     (PGID_AGGR + SRVL_ACS)

#if defined(VTSS_CHIP_SERVAL)
#define SRVL_EVC_CNT 1024
#else
#define SRVL_EVC_CNT 256
#endif /* VTSS_CHIP_SERVAL */

#define SRVL_DIV_ROUND_UP(x, base) ((x + base - 1)/base)

typedef struct {
    BOOL frame_rate; /* Enable frame rate policing (always single bucket) */
    BOOL dual;       /* Enable dual leaky bucket mode */
    BOOL data_rate;  /* Enable data rate policing */
    u32  cir;        /* CIR in kbps/fps (ignored in single bucket mode) */
    u32  cbs;        /* CBS in bytes/frames (ignored in single bucket mode) */
    u32  eir;        /* EIR (PIR) in kbps/fps */
    u32  ebs;        /* EBS (PBS) in bytes/frames */
    BOOL cf;         /* Coupling flag (ignored in single bucket mode) */
} vtss_srvl_policer_conf_t;

#define SEG_I(idx) vtss_state->mpls.segment_conf[idx]
#define XC_I(idx)  vtss_state->mpls.xc_conf[idx]
#define VP_I(idx)  vtss_state->mpls.vprofile_conf[idx]
#define L2_I(idx)  vtss_state->mpls.l2_conf[idx]

#define SEG_P(idx) SEG_I(idx).pub
#define XC_P(idx)  XC_I(idx).pub
#define VP_P(idx)  VP_I(idx).pub
#define L2_P(idx)  L2_I(idx).pub

/* ================================================================= *
 *  Register access
 * ================================================================= */
extern vtss_rc (*vtss_srvl_wr)(vtss_state_t *vtss_state, u32 addr, u32 value);
extern vtss_rc (*vtss_srvl_rd)(vtss_state_t *vtss_state, u32 addr, u32 *value);
vtss_rc vtss_srvl_wrm(vtss_state_t *vtss_state, u32 reg, u32 value, u32 mask);

#define SRVL_RD(p, value)                 \
    {                                     \
        vtss_rc __rc = vtss_srvl_rd(vtss_state, p, value);    \
        if (__rc != VTSS_RC_OK)           \
            return __rc;                  \
    }

#define SRVL_WR(p, value)                 \
    {                                     \
        vtss_rc __rc = vtss_srvl_wr(vtss_state, p, value);    \
        if (__rc != VTSS_RC_OK)           \
            return __rc;                  \
    }

#define SRVL_WRM(p, value, mask)                 \
    {                                            \
        vtss_rc __rc = vtss_srvl_wrm(vtss_state, p, value, mask);     \
        if (__rc != VTSS_RC_OK)                  \
            return __rc;                         \
    }

#define SRVL_WRM_SET(p, mask) SRVL_WRM(p, mask, mask)
#define SRVL_WRM_CLR(p, mask) SRVL_WRM(p, 0,    mask)
#define SRVL_WRM_CTL(p, _cond_, mask) SRVL_WRM(p, (_cond_) ? mask : 0, mask)

/* Decode register bit field */
#define SRVL_BF(name, value) ((VTSS_F_##name & value) ? 1 : 0)

/* Common functions */
vtss_rc vtss_srvl_init_groups(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
u32 vtss_srvl_port_mask(vtss_state_t *vtss_state, const BOOL member[]);
vtss_rc vtss_srvl_isdx_update_es0(vtss_state_t *vtss_state,
                                  BOOL isdx_ena, u32 isdx, u32 isdx_mask);
void vtss_srvl_debug_cnt(const vtss_debug_printf_t pr, const char *col1, const char *col2, 
                         vtss_chip_counter_t *c1, vtss_chip_counter_t *c2);
void vtss_srvl_debug_reg_header(const vtss_debug_printf_t pr, const char *name);
void vtss_srvl_debug_reg(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr, u32 addr, const char *name);
void vtss_srvl_debug_reg_inst(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name);
void vtss_srvl_debug_print_port_header(vtss_state_t *vtss_state,
                                       const vtss_debug_printf_t pr, const char *txt);
void vtss_srvl_debug_print_mask(const vtss_debug_printf_t pr, u32 mask);

/* Port functions */
vtss_rc vtss_srvl_port_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_port_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info);
vtss_rc vtss_srvl_port_max_tags_set(vtss_state_t *vtss_state, vtss_port_no_t port_no);
vtss_rc vtss_srvl_port_policer_fc_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_srvl_counter_update(vtss_state_t *vtss_state,
                                 u32 *addr, vtss_chip_counter_t *counter, BOOL clear);
u32 vtss_srvl_wm_dec(u32 value);

/* Miscellaneous functions */
vtss_rc vtss_srvl_misc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id);
vtss_rc vtss_srvl_gpio_mode(vtss_state_t *vtss_state,
                            const vtss_chip_no_t   chip_no,
                            const vtss_gpio_no_t   gpio_no,
                            const vtss_gpio_mode_t mode);
vtss_rc vtss_srvl_misc_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info);

/* QoS functions */
vtss_rc vtss_srvl_qos_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_policer_conf_set(vtss_state_t *vtss_state,
                                   u32 policer, vtss_srvl_policer_conf_t *conf);
vtss_rc vtss_srvl_qos_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);

/* L2 functions */
vtss_rc vtss_srvl_l2_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_l2_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info);

/* Packet functions */
vtss_rc vtss_srvl_packet_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_packet_debug_print(vtss_state_t *vtss_state,
                                     const vtss_debug_printf_t pr,
                                     const vtss_debug_info_t   *const info);

#if defined(VTSS_FEATURE_AFI_SWC)
vtss_rc srvl_afi_pause_resume(vtss_state_t *vtss_state, vtss_port_no_t port_no, BOOL resume);
vtss_rc vtss_srvl_afi_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);
#endif /* VTSS_FEATURE_AFI_SWC */

#if 1
/* ----------------------------------------------------------------- *
 * Raw MPLS encapsulation.
 *
 * WARNING: Special use, not used by other MPLS functionality.
 *
 * WARNING: Serval-specific!
 * ----------------------------------------------------------------- */

#define VTSS_SRVL_MPLS_OUT_ENCAP_RAW_MAX_BYTE_CNT  (272/8)

typedef struct {
    u8  length;                                              // In bytes, [0;SRVL_MPLS_OUT_ENCAP_RAW_MAX_BYTE_CNT]
    u8  data[VTSS_SRVL_MPLS_OUT_ENCAP_RAW_MAX_BYTE_CNT];     // Network order; [0] is first on the wire
} vtss_srvl_mpls_out_encap_raw_t;

vtss_rc vtss_srvl_mpls_out_encap_raw_set(vtss_state_t *vtss_state,
                                         const u32 idx,
                                         const vtss_srvl_mpls_out_encap_raw_t *const entry,
                                         vtss_es0_mpls_encap_len_t *length);

#endif   /* Raw MPLS encapsulation */

#if defined(VTSS_FEATURE_MPLS)
/* MPLS functions */
vtss_rc vtss_srvl_mpls_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_mpls_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info);
void vtss_mpls_ece_is1_update(vtss_state_t *vtss_state,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece,
                              vtss_sdx_entry_t *isdx, vtss_is1_key_t *key);
void vtss_mpls_ece_es0_update(vtss_state_t *vtss_state,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece,
                              vtss_sdx_entry_t *esdx, vtss_es0_action_t *action);
vtss_rc vtss_srvl_evc_mpls_update(vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd);
#endif /* VTSS_FEATURE_MPLS */

#if defined(VTSS_FEATURE_OAM)
/* OAM functions */
vtss_rc vtss_srvl_oam_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_oam_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);
#endif /* VTSS_FEATURE_OAM */

/* EVC functions */
vtss_rc vtss_srvl_evc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
#if defined(VTSS_FEATURE_EVC)
BOOL vtss_srvl_ece_is1_needed(BOOL nni, vtss_ece_dir_t dir, vtss_ece_rule_t rule);
vtss_rc vtss_srvl_ece_update(vtss_state_t *vtss_state,
                             vtss_ece_entry_t *ece, vtss_res_t *res, vtss_res_cmd_t cmd);
vtss_rc vtss_srvl_mce_is1_add(vtss_state_t *vtss_state, vtss_mce_entry_t *mce);
#endif /* VTSS_FEATURE_EVC */
vtss_rc vtss_srvl_evc_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);

/* VCAP functions */
vtss_rc vtss_srvl_vcap_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_vcap_port_key_addr_set(vtss_state_t         *vtss_state,
                                         const vtss_port_no_t port_no,
                                         u8                   lookup,
                                         vtss_vcap_key_type_t key_new,
                                         vtss_vcap_key_type_t key_old,
                                         BOOL                 dmac_dip_new);
vtss_rc vtss_srvl_vcap_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info);
vtss_rc vtss_srvl_debug_is0_all(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_srvl_debug_is1_all(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_srvl_debug_es0_all(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_srvl_debug_range_checkers(vtss_state_t *vtss_state,
                                       const vtss_debug_printf_t pr,
                                       const vtss_debug_info_t   *const info);

#if defined(VTSS_FEATURE_TIMESTAMP)
/* Timestamp functions */
vtss_rc vtss_srvl_ts_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_srvl_ts_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info);
#endif /* VTSS_FEATURE_TIMESTAMP */

#endif /* VTSS_ARCH_SERVAL */
#endif /* _VTSS_SERVAL_CIL_H_ */
