/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _IPMC_LIB_H_
#define _IPMC_LIB_H_

#include "l2proto_api.h"
#include "netdb.h"
#include "ipmc_lib_base.h"
#include "ipmc_lib_profile.h"

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_trace_api.h>
#define VTSS_TRACE_GRP_DEFAULT          0
#define TRACE_GRP_CRIT                  1
#define TRACE_GRP_CNT                   2
/* ============== */

#define IPMC_LIB_PKT_SZ_VAL             1516
#define IPMC_LIB_PKT_BUF_SZ             (sizeof(int) * ((IPMC_LIB_PKT_SZ_VAL + 3) / sizeof(int)))
#define IPMC_LIB_PKT_LOG_MSG_SZ_VAL     (128 + 1)
#define IPMC_PACK_STRUCT_STRUCT         __attribute__((packed))
#define IPMCLIB_IS_LINKLOCAL(a)         (((a).addr[0] == 0xFE) && (((a).addr[1] & 0xC0) == 0x80))
#define VTSS_IPMC_SFM_OP_PORT_ANY       0xFFFFFFFF

#define IPMC_SFM_OPERAND_BLOCK          0
#define IPMC_SFM_OPERAND_ALLOW          1
#define IPMC_SFM_OPERAND_ALL            2

#define VTSS_IPMC_SFM_OP_OR             1
#define VTSS_IPMC_SFM_OP_AND            2
#define VTSS_IPMC_SFM_OP_DIFF           3

#define IPMC_SFM_RECORD_TYPE_NONE       0
#define IPMC_SFM_MODE_IS_INCLUDE        1
#define IPMC_SFM_MODE_IS_EXCLUDE        2
#define IPMC_SFM_CHANGE_TO_INCLUDE      3
#define IPMC_SFM_CHANGE_TO_EXCLUDE      4
#define IPMC_SFM_ALLOW_NEW_SOURCES      5
#define IPMC_SFM_BLOCK_OLD_SOURCES      6
#define IPMC_SFM_RECORD_TYPE_ERR        0xFF

#define IPMC_SFM_INCLUDE_MODE           1
#define IPMC_SFM_EXCLUDE_MODE           2

#define VTSS_IPMC_SSM6_RANGE_PREFIX     0xFF3E0000  /**< MLD Default Address Range(Prefix) for SSM */
#define VTSS_IPMC_SSM6_RANGE_LEN        0x60        /**< MLD Default Address Range(Length) for SSM */
#define VTSS_IPMC_SSM4_RANGE_PREFIX     0xE8000000  /**< IGMP Default Address Range(Prefix) for SSM */
#define VTSS_IPMC_SSM4_RANGE_LEN        0x8         /**< IGMP Default Address Range(Length) for SSM */

#define IPMC_ADDR_MIN_BIT_LEN           8
#define IPMC_ADDR_MAX_BIT_LEN           128
#define IPMC_ADDR_BYTE_LEN              (IPMC_ADDR_MAX_BIT_LEN / IPMC_ADDR_MIN_BIT_LEN)
#define IPMC_ADDR_V6_MIN_BIT_LEN        IPMC_ADDR_MIN_BIT_LEN
#define IPMC_ADDR_V6_MAX_BIT_LEN        IPMC_ADDR_MAX_BIT_LEN
#define IPMC_ADDR_V4_MIN_BIT_LEN        4
#define IPMC_ADDR_V4_MAX_BIT_LEN        32
#define IPMC_ADDR_4IN6_SHIFT_BIT_LEN    (IPMC_ADDR_V6_MAX_BIT_LEN - IPMC_ADDR_V4_MAX_BIT_LEN)

#define IPMC_REPORT_THROTTLED           (-1)
#define IPMC_REPORT_NORMAL              0

#define IPMC_INTF_MVR(x)                ((x) ? ((x)->param.mvr) : VTSS_IPMC_FALSE)
#define IPMC_INTF_IS_MVR_VAL(x)         ((IPMC_INTF_MVR((x))) ? 1 : 0)
#define IPMC_COMPATIBILITY(x)           ((x) ? ((x)->param.cfg_compatibility) : VTSS_IPMC_COMPAT_MODE_AUTO)

#define IPMC_LIB_ISID_VALID(x)          (((x) == VTSS_ISID_GLOBAL) || ((x) == VTSS_ISID_LOCAL) || VTSS_ISID_LEGAL((x)))
#define IPMC_LIB_ISID_EXIST(x)          (VTSS_ISID_LEGAL((x)) ? msg_switch_exists((x)) : FALSE)
#define IPMC_LIB_ISID_CHECK(x)          ((((x) == VTSS_ISID_GLOBAL) || ((x) == VTSS_ISID_LOCAL)) ? TRUE : (IPMC_LIB_ISID_EXIST((x))))
#define IPMC_LIB_ISID_PASS(x, y)        (((x) == VTSS_ISID_GLOBAL) ? IPMC_LIB_ISID_EXIST((y)) : \
                                         (((x) == VTSS_ISID_LOCAL) ? ipmc_lib_isid_is_local((y)) : ((x) == (y))))

#define IPMC_LIB_BFS_HAS_MEMBER(x)      (ipmc_lib_bf_status_check((x)) != IPMC_BF_EMPTY)
#define IPMC_LIB_BFS_EMPTY(x)           (ipmc_lib_bf_status_check((x)) == IPMC_BF_EMPTY)
#define IPMC_LIB_BFS_SEMI_EMPTY(x)      (ipmc_lib_bf_status_check((x)) != IPMC_BF_HASMEMBER)
#define IPMC_LIB_BFS_PROT_EMPTY(x)      IPMC_LIB_BFS_EMPTY((x))
#define IPMC_LIB_BFS_ONLY_STACK(x)      (ipmc_lib_bf_status_check((x)) == IPMC_BF_SEMIEMPTY)

#define IPMC_LIB_SRCT_ADD_ECK(a, x, y, z)       (!ipmc_lib_srclist_add((x), (y), (z), (a)))
#define IPMC_LIB_SRCT_ADD_EPM(a, x, y, z)       if (!ipmc_lib_srclist_add((x), (y), (z), (a))) {T_W("ipmc_lib_srclist_add failed!!!");}
#define IPMC_LIB_SRCT_ADD_ERT(a, w, x, y, z)    if (!ipmc_lib_srclist_add((x), (y), (z), (a))) {T_W("ipmc_lib_srclist_add failed!!!"); return (w);}

/* Based on RFC-3810 Chapter 9 (in second); Also refer to RFC-5519 */
/* 9.1 */
#define IPMC_TIMER_RV(x)                ((x) ? ((x)->param.querier.RobustVari) : 0)
/* 9.2 */
#define IPMC_TIMER_QI(x)                ((x) ? ((x)->param.querier.QueryIntvl) : 0)
/* 9.3 */
#define IPMC_TIMER_QRI(x)               ((x) ? ((x)->param.querier.MaxResTime / 10) : 0)
/* 9.4 */
#define IPMC_TIMER_MALI(x)              ((x) ? (((x)->param.querier.RobustVari * (x)->param.querier.QueryIntvl) + ((x)->param.querier.MaxResTime / 10)) : 0)
/* 9.5 */
#define IPMC_TIMER_OQPT(x)              ((x) ? (((x)->param.querier.RobustVari * (x)->param.querier.QueryIntvl) + ((x)->param.querier.MaxResTime / (10 * 2))) : 0)
/* 9.6 */
#define IPMC_TIMER_SQI(x)               ((x) ? ((x)->param.querier.QueryIntvl / 4) : 0)
/* 9.7 */
#define IPMC_TIMER_SQC(x)               ((x) ? ((x)->param.querier.RobustVari) : 0)
/* 9.8 */
#define IPMC_TIMER_LLQI(x)              ((x) ? ((x)->param.querier.LastQryItv / 10) : 0)
/* 9.9 */
#define IPMC_TIMER_LLQC(x)              ((x) ? ((x)->param.querier.LastQryCnt) : 0)
/* 9.10 */
#define IPMC_TIMER_LLQT(x)              ((x) ? (((x)->param.querier.LastQryItv / 10) * (x)->param.querier.RobustVari) : 0)
/* 9.11 */
#define IPMC_TIMER_URI(x)               ((x) ? ((x)->param.querier.UnsolicitR) : 0)
/* 9.12 */
#define IPMC_TIMER_OVQPT(x)             ((x) ? (((x)->param.querier.RobustVari * (x)->param.querier.QueryIntvl) + ((x)->param.querier.MaxResTime / 10)) : 0)
/* 9.13 */
#define IPMC_TIMER_OVHPT(x)             ((x) ? (((x)->param.querier.RobustVari * (x)->param.querier.QueryIntvl) + ((x)->param.querier.MaxResTime / 10)) : 0)

#define IPMC_SRCLIST_WALK(x, y)         while (((y) = ipmc_lib_srclist_adr_get_next((x), (y))) != NULL)

#define IPMC_TIMER_GREATER(x, y)        (ipmc_lib_time_cmp((x), (y)) == IPMC_LIB_TIME_CMP_GREATER)
#define IPMC_TIMER_EQUAL(x, y)          (ipmc_lib_time_cmp((x), (y)) == IPMC_LIB_TIME_CMP_EQUAL)
#define IPMC_TIMER_LESS(x, y)           (ipmc_lib_time_cmp((x), (y)) == IPMC_LIB_TIME_CMP_LESS)
#define IPMC_TIMER_ZERO(x)              (!((x)->sec) && !((x)->msec) && !((x)->usec))
#define IPMC_TIMER_RESET(x)             do {    \
  if ((x)) {                                    \
    (x)->sec = 0;                               \
    (x)->msec = (x)->usec = 0;                  \
  }                                             \
} while (0)

#define IPMC_TIMER_UNLINK(x, y, z)      do {    \
  *(z) = FALSE;                                 \
  if ((x) && (y))                               \
    if (IPMC_LIB_DB_GET((x), (y)))              \
      *(z) = IPMC_LIB_DB_DEL((x), (y));         \
  if (*(z)) {};                                 \
} while (0)

#define IPMC_RTIMER_RELINK(w, x, y, z)  do {                                \
  *(z) = FALSE;                                                             \
  if ((x) && (y))                                                           \
    if (IPMC_LIB_DB_GET((x), (y)))                                          \
      *(z) = IPMC_LIB_DB_DEL((x), (y));                                     \
  if (*(z) && (y) && (x)) {                                                 \
    IPMC_TIMER_RESET(&(y)->min_tmr);                                        \
    for (; (w); (w)--) {                                                    \
      if ((y)->rxmt_count[(w) - 1] &&                                       \
          !IPMC_TIMER_ZERO(&(y)->rxmt_timer[(w) - 1])) {                    \
        if (IPMC_TIMER_ZERO(&(y)->min_tmr))                                 \
          ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->rxmt_timer[(w) - 1]);      \
        else                                                                \
          if (IPMC_TIMER_GREATER(&(y)->min_tmr, &(y)->rxmt_timer[(w) - 1])) \
            ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->rxmt_timer[(w) - 1]);    \
      }                                                                     \
    }                                                                       \
    if (!IPMC_TIMER_ZERO(&(y)->min_tmr))                                    \
      *(z) = IPMC_LIB_DB_ADD((x), (y));                                     \
    if (*(z)) {};                                                           \
  }                                                                         \
} while (0)

#define IPMC_FTIMER_RELINK(w, x, y, z)  do {                                        \
  *(z) = FALSE;                                                                     \
  if ((x) && (y))                                                                   \
    if (IPMC_LIB_DB_GET((x), (y)))                                                  \
      *(z) = IPMC_LIB_DB_DEL((x), (y));                                             \
  if (*(z) && (y) && (x)) {                                                         \
    IPMC_TIMER_RESET(&(y)->min_tmr);                                                \
    for (; (w); (w)--) {                                                            \
      if (!IPMC_TIMER_ZERO(&(y)->tmr.fltr_timer.t[(w) - 1])) {                      \
        if (IPMC_TIMER_ZERO(&(y)->min_tmr))                                         \
          ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->tmr.fltr_timer.t[(w) - 1]);        \
        else                                                                        \
          if (IPMC_TIMER_GREATER(&(y)->min_tmr, &(y)->tmr.fltr_timer.t[(w) - 1]))   \
            ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->tmr.fltr_timer.t[(w) - 1]);      \
      }                                                                             \
    }                                                                               \
    if (!IPMC_TIMER_ZERO(&(y)->min_tmr))                                            \
      *(z) = IPMC_LIB_DB_ADD((x), (y));                                             \
    if (*(z)) {};                                                                   \
  }                                                                                 \
} while (0)

#define IPMC_STIMER_RELINK(w, x, y, z)  do {                                        \
  *(z) = FALSE;                                                                     \
  if ((x) && (y))                                                                   \
    if (IPMC_LIB_DB_GET((x), (y)))                                                  \
      *(z) = IPMC_LIB_DB_DEL((x), (y));                                             \
  if (*(z) && (y) && (x)) {                                                         \
    IPMC_TIMER_RESET(&(y)->min_tmr);                                                \
    for (; (w); (w)--) {                                                            \
      if (!IPMC_TIMER_ZERO(&(y)->tmr.srct_timer.t[(w) - 1])) {                      \
        if (IPMC_TIMER_ZERO(&(y)->min_tmr))                                         \
          ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->tmr.srct_timer.t[(w) - 1]);        \
        else                                                                        \
          if (IPMC_TIMER_GREATER(&(y)->min_tmr, &(y)->tmr.srct_timer.t[(w) - 1]))   \
            ipmc_lib_time_cpy(&(y)->min_tmr, &(y)->tmr.srct_timer.t[(w) - 1]);      \
      }                                                                             \
    }                                                                               \
    if (!IPMC_TIMER_ZERO(&(y)->min_tmr))                                            \
      *(z) = IPMC_LIB_DB_ADD((x), (y));                                             \
    if (*(z)) {};                                                                   \
  }                                                                                 \
} while (0)

#define IPMC_TIMER_SRCT_SET(v, w, x, y, z)      do {                                \
  if ((w) && (x) && (y)) {                                                          \
    if ((z)) {                                                                      \
      (void) ipmc_lib_time_cpy(&(x)->tmr.srct_timer.t[(y)], (z));                   \
      if (IPMC_TIMER_ZERO(&(x)->min_tmr) ||                                         \
          IPMC_TIMER_GREATER(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(y)])) {         \
        if (IPMC_LIB_DB_GET((w), (x)))                                              \
          (void) IPMC_LIB_DB_DEL((w), (x));                                         \
        ipmc_lib_time_cpy(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(y)]);              \
        (void) IPMC_LIB_DB_ADD((w), (x));                                           \
      }                                                                             \
    } else {                                                                        \
      if (IPMC_TIMER_GREATER(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(y)])) {         \
        if (IPMC_LIB_DB_GET((w), (x)))                                              \
          (void) IPMC_LIB_DB_DEL((w), (x));                                         \
        IPMC_TIMER_RESET(&(x)->min_tmr);                                            \
        for (; (v); (v)--) {                                                        \
          if ((v - 1) == (y)) continue;                                             \
          if (IPMC_TIMER_ZERO(&(x)->tmr.srct_timer.t[(v) - 1])) continue;           \
          if (IPMC_TIMER_ZERO(&(x)->min_tmr))                                       \
            ipmc_lib_time_cpy(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(v) - 1]);      \
          else                                                                      \
            if (IPMC_TIMER_GREATER(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(v) - 1])) \
              ipmc_lib_time_cpy(&(x)->min_tmr, &(x)->tmr.srct_timer.t[(v) - 1]);    \
        }                                                                           \
        if (!IPMC_TIMER_ZERO(&(x)->min_tmr))                                        \
          (void) IPMC_LIB_DB_ADD((w), (x));                                         \
      }                                                                             \
      IPMC_TIMER_RESET(&(x)->tmr.srct_timer.t[(y)]);                                \
    }                                                                               \
  }                                                                                 \
} while (0)

#define IPMC_RXMT_TIMER_RESET(v, w, x, y, z)    do {                        \
  if ((x) && (y) && (z)) {                                                  \
    if (IPMC_LIB_DB_GET((x), (z)))                                          \
      (void) IPMC_LIB_DB_DEL((x), (z));                                     \
    if ((z)->rxmt_count[(w)]) {                                             \
      if (--(z)->rxmt_count[(w)] == 0) {                                    \
        IPMC_TIMER_RESET(&(z)->rxmt_timer[(w)]);                            \
      } else {                                                              \
        ipmc_time_t offset;                                                 \
        offset.sec = (y)->param.querier.LastQryItv / 10;                    \
        offset.msec = ((y)->param.querier.LastQryItv % 10) * 100;           \
        offset.usec = 0;                                                    \
        (void) ipmc_lib_time_stamp(&(z)->rxmt_timer[(w)], &offset);         \
      }                                                                     \
    } else {                                                                \
      IPMC_TIMER_RESET(&(z)->rxmt_timer[(w)]);                              \
    }                                                                       \
    IPMC_TIMER_RESET(&(z)->min_tmr);                                        \
    for (; (v); (v)--) {                                                    \
      if (!IPMC_TIMER_ZERO(&(z)->rxmt_timer[(v) - 1])) {                    \
        if (IPMC_TIMER_ZERO(&(z)->min_tmr))                                 \
          ipmc_lib_time_cpy(&(z)->min_tmr, &(z)->rxmt_timer[(v) - 1]);      \
        else                                                                \
          if (IPMC_TIMER_GREATER(&(z)->min_tmr, &(z)->rxmt_timer[(v) - 1])) \
            ipmc_lib_time_cpy(&(z)->min_tmr, &(z)->rxmt_timer[(v) - 1]);    \
      }                                                                     \
    }                                                                       \
    if (!IPMC_TIMER_ZERO(&(z)->min_tmr))                                    \
      (void) IPMC_LIB_DB_ADD((x), (z));                                     \
  }                                                                         \
} while (0)

#define IPMC_TIMER_MALI_GET(w, x)       do {                                        \
  if ((w) && (x)) {                                                                 \
    ipmc_time_t offset;                                                             \
    offset.sec = ((w)->param.querier.MaxResTime / 10);                              \
    offset.sec += ((w)->param.querier.QueryIntvl) * (w)->param.querier.RobustVari;  \
    offset.msec = ((w)->param.querier.MaxResTime % 10) * 100;                       \
    offset.usec = 0;                                                                \
    (void) ipmc_lib_time_stamp((x), &offset);                                       \
  }                                                                                 \
} while (0)

#define IPMC_TIMER_MALI_SET(v, w, x, y, z)      do {                                    \
  if ((w) && (x) && (y) && (z)) {                                                       \
    ipmc_time_t offset;                                                                 \
    offset.sec = ((w)->param.querier.MaxResTime / 10);                                  \
    offset.sec += ((w)->param.querier.QueryIntvl) * (w)->param.querier.RobustVari;      \
    offset.msec = ((w)->param.querier.MaxResTime % 10) * 100;                           \
    offset.usec = 0;                                                                    \
    if (!IPMC_TIMER_ZERO(&(z)->min_tmr) && IPMC_TIMER_LESS(&(z)->min_tmr, (x))) {       \
      (void) ipmc_lib_time_stamp((x), &offset);                                         \
    } else {                                                                            \
      (void) ipmc_lib_time_stamp((x), &offset);                                         \
      if (IPMC_TIMER_ZERO(&(z)->min_tmr)) {                                             \
        ipmc_lib_time_cpy(&(z)->min_tmr, (x));                                          \
      } else {                                                                          \
        if (IPMC_LIB_DB_GET((y), (z)))                                                  \
          (void) IPMC_LIB_DB_DEL((y), (z));                                             \
        IPMC_TIMER_RESET(&(z)->min_tmr);                                                \
        for (; (v); (v)--) {                                                            \
          if (!IPMC_TIMER_ZERO(&(z)->tmr.fltr_timer.t[(v) - 1])) {                      \
            if (IPMC_TIMER_ZERO(&(z)->min_tmr))                                         \
              ipmc_lib_time_cpy(&(z)->min_tmr, &(z)->tmr.fltr_timer.t[(v) - 1]);        \
            else                                                                        \
              if (IPMC_TIMER_GREATER(&(z)->min_tmr, &(z)->tmr.fltr_timer.t[(v) - 1]))   \
                ipmc_lib_time_cpy(&(z)->min_tmr, &(z)->tmr.fltr_timer.t[(v) - 1]);      \
          }                                                                             \
        }                                                                               \
      }                                                                                 \
      if (!IPMC_TIMER_ZERO(&(z)->min_tmr))                                              \
        (void) IPMC_LIB_DB_ADD((y), (z));                                               \
    }                                                                                   \
  }                                                                                     \
} while (0)

#define IPMC_TIMER_LLQT_GET(w, x)       do {                                                    \
  if ((w) && (x)) {                                                                             \
    ipmc_time_t offset;                                                                         \
    offset.sec = ((w)->param.querier.LastQryItv / 10) * (w)->param.querier.RobustVari;          \
    offset.msec = ((w)->param.querier.LastQryItv % 10) * (w)->param.querier.RobustVari * 100;   \
    offset.usec = 0;                                                                            \
    (void) ipmc_lib_time_stamp((x), &offset);                                                   \
  }                                                                                             \
} while (0)

#define IPMC_TIMER_LLQT_GSET(w, x, y, z)        do {                                            \
  if ((w) && (x)) {                                                                             \
    ipmc_time_t llqt, offset;                                                                   \
    offset.sec = ((w)->param.querier.LastQryItv / 10) * (w)->param.querier.RobustVari;          \
    offset.msec = ((w)->param.querier.LastQryItv % 10) * (w)->param.querier.RobustVari * 100;   \
    offset.usec = 0;                                                                            \
    if (ipmc_lib_time_stamp(&llqt, &offset) && (y) && (z)) {                                    \
      if (IPMC_TIMER_ZERO((x)) || IPMC_TIMER_GREATER((x), &llqt)) {                             \
        if (IPMC_LIB_DB_GET((y), (z)))                                                          \
          (void) IPMC_LIB_DB_DEL((y), (z));                                                     \
        ipmc_lib_time_cpy((x), &llqt);                                                          \
        if (IPMC_TIMER_ZERO(&(z)->min_tmr)) {                                                   \
          ipmc_lib_time_cpy(&(z)->min_tmr, (x));                                                \
        } else {                                                                                \
          if (IPMC_TIMER_GREATER(&(z)->min_tmr, (x)))                                           \
            ipmc_lib_time_cpy(&(z)->min_tmr, (x));                                              \
        }                                                                                       \
        (void) IPMC_LIB_DB_ADD((y), (z));                                                       \
      }                                                                                         \
    }                                                                                           \
  }                                                                                             \
} while (0)

#define IPMC_TIMER_LLQI_SET(w, x, y, z) do {                    \
  if ((w) && (x)) {                                             \
    ipmc_time_t offset;                                         \
    offset.sec = (w)->param.querier.LastQryItv / 10;            \
    offset.msec = ((w)->param.querier.LastQryItv % 10) * 100;   \
    offset.usec = 0;                                            \
    if (ipmc_lib_time_stamp((x), &offset) && (y) && (z)) {      \
      if (IPMC_LIB_DB_GET((y), (z)))                            \
        (void) IPMC_LIB_DB_DEL((y), (z));                       \
      if (IPMC_TIMER_ZERO(&(z)->min_tmr)) {                     \
        ipmc_lib_time_cpy(&(z)->min_tmr, (x));                  \
      } else {                                                  \
        if (IPMC_TIMER_GREATER(&(z)->min_tmr, (x)))             \
          ipmc_lib_time_cpy(&(z)->min_tmr, (x));                \
      }                                                         \
      (void) IPMC_LIB_DB_ADD((y), (z));                         \
    }                                                           \
  }                                                             \
} while (0)

#define IPMC_FLTR_TIMER_DELTA_GET(x)    do {                        \
  if ((x)) {                                                        \
    ipmc_time_t now_t;                                              \
                                                                    \
    if (ipmc_lib_time_curr_get(&now_t)) {                           \
      u8    idx, local_port_cnt;                                    \
                                                                    \
      local_port_cnt = ipmc_lib_get_system_local_port_cnt();        \
      for (idx = 0; idx < local_port_cnt; idx++) {                  \
        if (IPMC_TIMER_LESS(&now_t, &((x)->tmr.fltr_timer.t[idx]))) \
          (x)->tmr.delta_time.v[idx].sec -= now_t.sec;              \
        else                                                        \
          IPMC_TIMER_RESET(&((x)->tmr.delta_time.v[idx]));          \
      }                                                             \
    }                                                               \
  }                                                                 \
} while (0)

#define IPMC_SRCT_TIMER_DELTA_GET(x)    do {                        \
  if ((x)) {                                                        \
    ipmc_time_t now_t;                                              \
                                                                    \
    if (ipmc_lib_time_curr_get(&now_t)) {                           \
      u8    idx, local_port_cnt;                                    \
                                                                    \
      local_port_cnt = ipmc_lib_get_system_local_port_cnt();        \
      for (idx = 0; idx < local_port_cnt; idx++) {                  \
        if (IPMC_TIMER_LESS(&now_t, &((x)->tmr.srct_timer.t[idx]))) \
          (x)->tmr.delta_time.v[idx].sec -= now_t.sec;              \
        else                                                        \
          IPMC_TIMER_RESET(&((x)->tmr.delta_time.v[idx]));          \
      }                                                             \
    }                                                               \
  }                                                                 \
} while (0)

#define IPMC_MEM_H_MTAKE(x)             (((x) = (ipmc_db_ctrl_hdr_t *)ipmc_lib_mem_alloc(IPMC_MEM_CTRL_HDR, sizeof(ipmc_db_ctrl_hdr_t), 0xFF)) != NULL)
#define IPMC_MEM_H_MGIVE(x, y)          do {BOOL mflag = (x) ? (x)->mflag : FALSE; if (((*(y)) = ipmc_lib_mem_free(IPMC_MEM_CTRL_HDR, (u8 *)(x), 0xFF)) == FALSE) T_W("Free[IPMC_MEM_CTRL_HDR] %s(mflag=%u)!!!", (*(y)) ? "OK" : "NG", mflag);} while (0)
#define IPMC_MEM_SL_MTAKE(x, y)         (((x) = (ipmc_sfm_srclist_t *)ipmc_lib_mem_alloc(IPMC_MEM_SRC_LIST, sizeof(ipmc_sfm_srclist_t), (y))) != NULL)
#define IPMC_MEM_SL_MGIVE(x, y, z)      do {BOOL mflag = (x) ? (x)->mflag : FALSE; if (((*(y)) = ipmc_lib_mem_free(IPMC_MEM_SRC_LIST, (u8 *)(x), (z))) == FALSE) T_D("Free[IPMC_MEM_SRC_LIST] %s(mflag=%u)!!!", (*(y)) ? "OK" : "NG", mflag);} while (0)
#define IPMC_MEM_GRP_MTAKE(x)           (((x) = (ipmc_group_entry_t *)ipmc_lib_mem_alloc(IPMC_MEM_GROUP, sizeof(ipmc_group_entry_t), 0xFF)) != NULL)
#define IPMC_MEM_GRP_MGIVE(x, y)        do {BOOL mflag = (x) ? (x)->mflag : FALSE; if (((*(y)) = ipmc_lib_mem_free(IPMC_MEM_GROUP, (u8 *)(x), 0xFF)) == FALSE) T_W("Free[IPMC_MEM_GROUP] %s(mflag=%u)!!!", (*(y)) ? "OK" : "NG", mflag);} while (0)
#define IPMC_MEM_INFO_MTAKE(x)          (((x) = (ipmc_group_info_t *)ipmc_lib_mem_alloc(IPMC_MEM_GRP_INFO, sizeof(ipmc_group_info_t), 0xFF)) != NULL)
#define IPMC_MEM_INFO_MGIVE(x, y)       do {BOOL mflag = (x) ? (x)->mflag : FALSE; if (((*(y)) = ipmc_lib_mem_free(IPMC_MEM_GRP_INFO, (u8 *)(x), 0xFF)) == FALSE) T_W("Free[IPMC_MEM_GRP_INFO] %s(mflag=%u)!!!", (*(y)) ? "OK" : "NG", mflag);} while (0)
#define IPMC_MEM_RULES_MTAKE(x)         (((x) = (ipmc_profile_rule_t *)ipmc_lib_mem_alloc(IPMC_MEM_RULES, sizeof(ipmc_profile_rule_t), 0xFF)) != NULL)
#define IPMC_MEM_RULES_MGIVE(x)         do {if (!ipmc_lib_mem_free(IPMC_MEM_RULES, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#define IPMC_MEM_JUMBO_MTAKE(x)         (((x) = (u8 *)ipmc_lib_mem_alloc(IPMC_MEM_JUMBO, IPMC_LIB_PKT_BUF_SZ, 0xFF)) != NULL)
#define IPMC_MEM_JUMBO_MGIVE(x)         do {if (!ipmc_lib_mem_free(IPMC_MEM_JUMBO, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#if 1 /* For VTSS_MALLOC/VTSS_FREE */
#define IPMC_MEM_AVLTND_MTAKE(x, y)     (((x) = (vtss_avl_tree_node_t *)ipmc_lib_mem_alloc(IPMC_MEM_OS_MALLOC, (sizeof(vtss_avl_tree_node_t) * (y)), 0xFF)) != NULL)
#define IPMC_MEM_AVLTND_MGIVE(x)        do {if (!ipmc_lib_mem_free(IPMC_MEM_OS_MALLOC, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#define IPMC_MEM_SYSTEM_MTAKE(x, y)     (((x) = (void *)ipmc_lib_mem_alloc(IPMC_MEM_OS_MALLOC, (y), 0xFF)) != NULL)
#define IPMC_MEM_SYSTEM_MGIVE(x)        do {if (!ipmc_lib_mem_free(IPMC_MEM_OS_MALLOC, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#else
#define IPMC_MEM_AVLTND_MTAKE(x, y)     (((x) = (vtss_avl_tree_node_t *)ipmc_lib_mem_alloc(IPMC_MEM_AVLT_NODE, (sizeof(vtss_avl_tree_node_t) * (y)), 0xFF)) != NULL)
#define IPMC_MEM_AVLTND_MGIVE(x)        do {if (!ipmc_lib_mem_free(IPMC_MEM_AVLT_NODE, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#define IPMC_MEM_SYSTEM_MTAKE(x, y)     (((x) = (void *)ipmc_lib_mem_alloc(IPMC_MEM_SYS, (y), 0xFF)) != NULL)
#define IPMC_MEM_SYSTEM_MGIVE(x)        do {if (!ipmc_lib_mem_free(IPMC_MEM_SYS, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)
#endif /* For VTSS_MALLOC/VTSS_FREE */
#define IPMC_MEM_PROFILE_MTAKE(x)       (((x) = (ipmc_lib_profile_mem_t *)ipmc_lib_mem_alloc(IPMC_MEM_PROFILE, sizeof(ipmc_lib_profile_mem_t), 0xFF)) != NULL)
#define IPMC_MEM_PROFILE_MGIVE(x)       do {if (!ipmc_lib_mem_free(IPMC_MEM_PROFILE, (u8 *)(x), 0xFF)) T_W("IPMC_LIB_MEM_FREE fail !!!");} while (0)

#define IPMC_LIB_CHAR_ASCII_MIN         33  /* ! */
#define IPMC_LIB_CHAR_ASCII_MAX         126 /* ~ */
#define IPMC_LIB_CHAR_ASCII_SPACE       32  /* SPACE */

#define IPMC_LIB_NAME_IDX_CHECK(x, y)   (ipmc_lib_instance_name_check((x), (y), TRUE))
#define IPMC_LIB_NAME_CHECK(x)          (ipmc_lib_instance_name_check((x), VTSS_IPMC_NAME_MAX_LEN, FALSE))
#define IPMC_LIB_DESC_CHECK(x)          (ipmc_lib_instance_desc_check((x), VTSS_IPMC_DESC_MAX_LEN))

/* Packet format declaration */
#define IPMC_PKT_PRIVATE_PAD_LEN        sizeof(u32)
#define IPMC_PKT_PRIVATE_PAD_MAGIC      0x19794600

#define IP_MULTICAST_V4_ETHER_TYPE      0x0800
#define IP_MULTICAST_V6_ETHER_TYPE      0x86DD
#define IP_MULTICAST_V4_IP_VERSION      4
#define IP_MULTICAST_V6_IP_VERSION      6

#define IPMC_IP2MAC_V4MAC_ARRAY0        0x1
#define IPMC_IP2MAC_V4MAC_ARRAY1        0x0
#define IPMC_IP2MAC_V4MAC_ARRAY2        0x5E
#define IPMC_IP2MAC_V6MAC_ARRAY0        0x33
#define IPMC_IP2MAC_V6MAC_ARRAY1        0x33
#define IPMC_IP2MAC_V4SHIFT_LEN         9
#define IPMC_IP2MAC_V6SHIFT_LEN         0
#define IPMC_IP2MAC_ARRAY_MASK          0xFF
#define IPMC_IP2MAC_ARRAY2_SHIFT_LEN    24
#define IPMC_IP2MAC_ARRAY3_SHIFT_LEN    16
#define IPMC_IP2MAC_ARRAY4_SHIFT_LEN    8
#define IPMC_IP2MAC_ARRAY5_SHIFT_LEN    0

#define IP_MULTICAST_IGMP_PROTO_ID      2
#define IPMC_IGMP_MSG_TYPE_QUERY        0x11    /* 17 */
#define IPMC_IGMP_MSG_TYPE_V1JOIN       0x12    /* 18 */
#define IPMC_IGMP_MSG_TYPE_V2JOIN       0x16    /* 22 */
#define IPMC_IGMP_MSG_TYPE_V3JOIN       0x22    /* 34 */
#define IPMC_IGMP_MSG_TYPE_LEAVE        0x17    /* 23 */

#define IPMC_IPHDR_HOPLIMIT             1

#define IPMC_IPV4_RTR_ALERT_PREFIX1     0x94    /* 1 0 0 1 0 1 0 0 0 0 0 0 0 1 0 0 + Value(2 octets) */
#define IPMC_IPV4_RTR_ALERT_PREFIX2     0x04    /* 1 0 0 1 0 1 0 0 0 0 0 0 0 1 0 0 + Value(2 octets) */

#define IGMP_MIN_PAYLOAD_LEN            8
#define IGMP_SFM_MIN_PAYLOAD_LEN        12

#define IPV6_HDR_FIXED_LEN              40
#define MLD_MIN_HBH_LEN                 8
#define MLD_MIN_OFFSET                  (IPV6_HDR_FIXED_LEN + MLD_MIN_HBH_LEN)

#define MLD_GEN_MIN_PAYLOAD_LEN         24
#define MLD_SFM_MIN_PAYLOAD_LEN         28

#define MLD_IPV6_NEXTHDR_OPT_HBH        0x0     /* Hop-By-Hop Option */
#define MLD_IPV6_NEXTHDR_ICMP           0x3A    /* MLD is a subprotocol of ICMPv6 (58) */
#define IPMC_IPV6_RTR_ALERT_PREFIX1     0x05    /* 0 0 0 0 0 1 0 1 0 0 0 0 0 0 1 0 + Value(2 octets) */
#define IPMC_IPV6_RTR_ALERT_PREFIX2     0x02    /* 0 0 0 0 0 1 0 1 0 0 0 0 0 0 1 0 + Value(2 octets) */

#define IPMC_MLD_MSG_TYPE_QUERY         0x82    /* 130 */
#define IPMC_MLD_MSG_TYPE_V1REPORT      0x83    /* 131 */
#define IPMC_MLD_MSG_TYPE_V2REPORT      0x8F    /* 143 */
#define IPMC_MLD_MSG_TYPE_DONE          0x84    /* 132 */

typedef enum {
    IPMC_LIB_TIME_CMP_INVALID = -2,
    IPMC_LIB_TIME_CMP_LESS,
    IPMC_LIB_TIME_CMP_EQUAL,
    IPMC_LIB_TIME_CMP_GREATER
} ipmc_time_cmp_t;

typedef enum {
    IPMC_MEM_OS_MALLOC = 0,
    IPMC_MEM_JUMBO,
    IPMC_MEM_SYS,
    IPMC_MEM_AVLT_NODE,
    IPMC_MEM_GROUP,
    IPMC_MEM_GRP_INFO,
    IPMC_MEM_CTRL_HDR,
    IPMC_MEM_SRC_LIST,
    IPMC_MEM_RULES,
    IPMC_MEM_PROFILE,
    IPMC_MEM_TYPE_MAX
} ipmc_mem_t;

typedef enum {
    VTSS_IPMC_SF_STATUS_DISABLED = 0,           /**< Source Filtering Status is Disabled */
    VTSS_IPMC_SF_STATUS_ENABLED,                /**< Source Filtering Status is Enabled */
    VTSS_IPMC_SF_STATUS_TRANSIT                 /**< Source Filtering Status is Changed */
} ipmc_sfm_status_t;

typedef enum {
    VTSS_IPMC_SF_MODE_EXCLUDE = 0,              /**< Source Filtering Mode is Exclude */
    VTSS_IPMC_SF_MODE_INCLUDE,                  /**< Source Filtering Mode is Include */
    VTSS_IPMC_SF_MODE_NONE                      /**< Source Filtering Mode is NONE */
} ipmc_sfm_mode_t;

typedef enum {
    VTSS_IPMC_SF_FWD_OP_INIT = 0,
    VTSS_IPMC_SF_FWD_OP_NONE_TO_NONE,
    VTSS_IPMC_SF_FWD_OP_NONE_TO_INCL,
    VTSS_IPMC_SF_FWD_OP_NONE_TO_EXCL,
    VTSS_IPMC_SF_FWD_OP_INCL_TO_NONE,
    VTSS_IPMC_SF_FWD_OP_INCL_TO_INCL,
    VTSS_IPMC_SF_FWD_OP_INCL_TO_EXCL,
    VTSS_IPMC_SF_FWD_OP_EXCL_TO_NONE,
    VTSS_IPMC_SF_FWD_OP_EXCL_TO_INCL,
    VTSS_IPMC_SF_FWD_OP_EXCL_TO_EXCL
} ipmc_sfm_fwd_op_t;

typedef enum {
    IPMC_BF_EMPTY = 0,
    IPMC_BF_SEMIEMPTY,
    IPMC_BF_HASMEMBER
} ipmc_bf_status;

typedef enum {
    PROC4RCV = 0,
    PROC4TICK,
    PROC4PROXY,
    PROC4INIT
} proc_grp_tmp_t;

typedef enum {
    IPMC_GRP_CALC_FOR_INTF = 0,
    IPMC_GRP_CALC_INTF_GRP,
    IPMC_GRP_CALC_GRP_TMR,
    IPMC_GRP_CALC_FOR_TMRS,
    IPMC_GRP_CALC_FOR_GRPS,
    IPMC_GRP_CALC_MAX
} ipmc_group_calculation_t;

typedef struct {
    BOOL                                valid;
    BOOL                                fixed;
    u8                                  idx;
    size_t                              sz_pool;
    size_t                              sz_partition;
} ipmc_mem_status_t;

typedef struct ipmc_grp_tmrlist_t {
    ipmc_time_t                         fire;
    u8                                  port;

    union {
        struct {
            ipmc_group_entry_t          *grp;
        } fltr;

        struct {
            ipmc_group_entry_t          *grp;
        } rxmt;

        struct {
            ipmc_sfm_srclist_t          *src;
        } srct;
    } tmr_info;

    struct ipmc_grp_tmrlist_t           *next;
} ipmc_grp_tmrlist_t;

/**
 * Representation of a 48-bit Ethernet address.
 */
typedef struct {
    uchar                       addr[6];
} IPMC_PACK_STRUCT_STRUCT ipmc_eth_addr;

/**
 * The Ethernet header.
 */
typedef struct {
    ipmc_eth_addr               dest;
    ipmc_eth_addr               src;
    ushort                      type;
} IPMC_PACK_STRUCT_STRUCT ipmc_ip_eth_hdr;


typedef struct {
    /* IP header. */
    uchar                       vhl;
    uchar                       tos;
    ushort                      PayloadLen;
    ushort                      seq_id;
    ushort                      offset;
    uchar                       ttl;
    uchar                       proto;
    ushort                      ip_chksum;
    ipmcv4addr                  ip4_src;
    ipmcv4addr                  ip4_dst;
    uchar                       router_option[4];
} IPMC_PACK_STRUCT_STRUCT igmp_ip4_hdr;

typedef struct {
    uchar                       type;
    uchar                       max_resp_time;
    ushort                      checksum;
} IPMC_PACK_STRUCT_STRUCT ipmc_igmp_packet_common_t;

typedef struct {
    ipmc_igmp_packet_common_t   common;

    union {
        struct {
            ipmcv4addr          group_address;
            uint                cannot_use[2];
        } usual;

        struct {
            ipmcv4addr          group_address;

            uchar               resv_s_qrv;
            uchar               qqic;
            ushort              no_of_sources;

            uint                dont_care;
        } sfm_query;

        struct {
            ushort              reserved;
            ushort              number_of_record;

            uchar               record_type;
            uchar               aux_len;
            ushort              no_of_sources;

            ipmcv4addr          group_address;
        } sfm_report;
    } sfminfo;
} IPMC_PACK_STRUCT_STRUCT ipmc_igmp_packet_t;

/* The IGMP and IPv4 headers. */
typedef struct {
    /* IP header. */
    uchar                       vhl;
    uchar                       tos;
    ushort                      PayloadLen;
    ushort                      seq_id;
    ushort                      offset;
    uchar                       ttl;
    uchar                       proto;
    ushort                      ip_chksum;
    ipmcv4addr                  ip4_src;
    ipmcv4addr                  ip4_dst;
    uchar                       router_option[4];

    /* IGMP header. */
    uchar                       type;
    uchar                       max_resp_time;
    ushort                      checksum;
    ipmcv4addr                  group;
} IPMC_PACK_STRUCT_STRUCT ipmc_ip_igmp_hdr;

/* The IGMPv3 and IPv4 headers. */
typedef struct {
    /* IP header. */
    uchar                       vhl;
    uchar                       tos;
    ushort                      PayloadLen;
    ushort                      seq_id;
    ushort                      offset;
    uchar                       ttl;
    uchar                       proto;
    ushort                      ip_chksum;
    ipmcv4addr                  ip4_src;
    ipmcv4addr                  ip4_dst;
    uchar                       router_option[4];

    /* IGMP header. */
    uchar                       type;
    uchar                       code;
    ushort                      checksum;

    union {
        struct {
            ipmcv4addr          group_address;

            uchar               resv_s_qrv;
            uchar               qqic;
            ushort              no_of_sources;

            ipmcv4addr          source_addr[IPMC_NO_OF_PKT_SRCLIST];    /* Max. 366 for MTU 1500 */
        } query;

        struct {
            ushort              reserved;
            ushort              number_of_record;

            uchar               record_type;
            uchar               aux_len;
            ushort              no_of_sources;

            ipmcv4addr          group_address;

            ipmcv4addr          source_addr[IPMC_NO_OF_PKT_SRCLIST];    /* Max. 366 for MTU 1500 */
        } report;
    } sfminfo;
} IPMC_PACK_STRUCT_STRUCT ipmc_ip_igmpv3_hdr;

typedef struct {
    ulong                       VerTcFl;    /* Version, Traffic Class, Flow Label */
    ushort                      PayloadLen; /* Payload Length */
    uchar                       NxtHdr;     /* Next Header */
    uchar                       HopLimit;   /* Hop Limit */
    vtss_ipv6_t                 ip6_src;    /* source address */
    vtss_ipv6_t                 ip6_dst;    /* destination address */
} IPMC_PACK_STRUCT_STRUCT mld_ip6_hdr;

typedef struct {
    uchar                       NextHdr;    /* Next Header */
    uchar                       HdrExtLen;  /* Hdr Ext Len */
    uchar                       OptNPad[6]; /* Options and Padding */
    uchar                       MoreOpt[8]; /* Optional: more Options and Padding */
} IPMC_PACK_STRUCT_STRUCT mld_ip6_hbh_hdr;

typedef struct {
    uchar                       type;
    uchar                       code;
    ushort                      checksum;
    ushort                      max_resp_time;
    ushort                      number_of_record;
} IPMC_PACK_STRUCT_STRUCT ipmc_mld_packet_common_t;

typedef struct {
    ipmc_mld_packet_common_t    common;

    union {
        struct {
            vtss_ipv6_t         group_address;
            uint                cannot_use;
        } usual;

        struct {
            vtss_ipv6_t         group_address;

            uchar               resv_s_qrv;
            uchar               qqic;
            ushort              no_of_sources;
        } sfm_query;

        struct {
            uchar               record_type;
            uchar               aux_len;
            ushort              no_of_sources;

            vtss_ipv6_t         group_address;
        } sfm_report;
    } sfminfo;
} IPMC_PACK_STRUCT_STRUCT ipmc_mld_packet_t;

typedef struct {
    ipmc_mld_packet_t           pkt;
    vtss_ipv6_t                 buf[IPMC_NO_OF_MAX_PKT_SRCLIST6];
} IPMC_PACK_STRUCT_STRUCT ipmc_pseudo_mld_t;

typedef struct {
    vtss_ipv6_t                 pseudoSrc;
    vtss_ipv6_t                 pseudoDst;
    ulong                       pseudoLen;
    uchar                       pseudoZero[3];
    uchar                       pseudoNextHdr;

    ipmc_pseudo_mld_t           ctrl;
} IPMC_PACK_STRUCT_STRUCT mld_icmp_pseudo_t;

/* The MLDv1 and IPv6 headers. */
typedef struct {
    /* IP header. */
    ulong                       VerTcFl;
    ushort                      PayloadLen;
    uchar                       NxtHdr;
    uchar                       HopLimit;
    vtss_ipv6_t                 ip6_src;
    vtss_ipv6_t                 ip6_dst;

    /* Hop-By-Hop */
    uchar                       HBHNxtHdr;
    uchar                       HdrExtLen;
    uchar                       OptNPad[6];

    /* MLDv1 */
    uchar                       type;
    uchar                       code;
    ushort                      checksum;
    ushort                      max_resp_time;
    uchar                       reserved[2];
    vtss_ipv6_t                 group;
} IPMC_PACK_STRUCT_STRUCT ipmc_ip_mld_hdr;

/* The MLDv2 and IPv6 headers. */
typedef struct {
    /* IP header. */
    ulong                       VerTcFl;
    ushort                      PayloadLen;
    uchar                       NxtHdr;
    uchar                       HopLimit;
    vtss_ipv6_t                 ip6_src;
    vtss_ipv6_t                 ip6_dst;

    /* Hop-By-Hop */
    uchar                       HBHNxtHdr;
    uchar                       HdrExtLen;
    uchar                       OptNPad[6];

    /* MLDv2 */
    uchar                       type;
    uchar                       code;
    ushort                      checksum;
    union {
        struct {
            ushort              max_resp_time;
            uchar               reserved[2];

            vtss_ipv6_t         group_address;

            uchar               resv_s_qrv;
            uchar               qqic;
            ushort              no_of_sources;

            vtss_ipv6_t         source_addr[IPMC_NO_OF_PKT_SRCLIST];    /* Max. 89 for MTU 1500 */
        } query;

        struct {
            uchar               reserved[2];
            ushort              number_of_record;

            uchar               record_type;
            uchar               aux_len;
            ushort              no_of_sources;

            vtss_ipv6_t         group_address;

            vtss_ipv6_t         source_addr[IPMC_NO_OF_PKT_SRCLIST];    /* Max. 89 for MTU 1500 */
        } report;
    } sfminfo;
} IPMC_PACK_STRUCT_STRUCT ipmc_ip_mldv2_hdr;

typedef struct {
    uchar       record_type;
    uchar       aux_len;
    ushort      no_of_sources;

    ipmcv4addr  group_address;
    ipmcv4addr  source_addr[IPMC_NO_OF_PKT_SRCLIST];
} IPMC_PACK_STRUCT_STRUCT igmp_group_record_t;

typedef struct {
    uchar       record_type;
    uchar       aux_len;
    ushort      no_of_sources;

    vtss_ipv6_t group_address;
    vtss_ipv6_t source_addr[IPMC_NO_OF_PKT_SRCLIST];
} IPMC_PACK_STRUCT_STRUCT mld_group_record_t;

/* Dedicated Memory Allocate/Free for IPMC */
typedef struct {
    int                 totalmem;
    int                 freemem;
    void                *base;
    int                 size;
    int                 blocksize;
    int                 maxfree;
} ipmc_memory_info_t;


u8 *ipmc_lib_mem_alloc(ipmc_mem_t type, size_t size, u8 alcid);
BOOL ipmc_lib_mem_free(ipmc_mem_t type, u8 *ptr, u8 freid);
BOOL ipmc_lib_instance_name_check(char *inst_name, i32 name_len, BOOL as_idx);
BOOL ipmc_lib_instance_desc_check(char *inst_desc, i32 desc_len);
/* Internal Debug Usage */
char *ipmc_lib_mem_id_txt(ipmc_mem_t mem_id);
int ipmc_lib_mem_debug_get_cnt(void);
void ipmc_lib_mem_debug_get_info(ipmc_mem_t idx, ipmc_memory_info_t *info);

u8 ipmc_lib_get_system_local_port_cnt(void);
BOOL ipmc_lib_get_next_system_local_port(u8 *port);

BOOL ipmc_lib_system_mgmt_info_set(ipmc_lib_mgmt_info_t *mgmt_sys);
BOOL ipmc_lib_system_mgmt_info_chg(ipmc_lib_mgmt_info_t *mgmt_sys);
BOOL ipmc_lib_system_mgmt_info_cpy(ipmc_lib_mgmt_info_t *mgmt_sys);
BOOL ipmc_lib_get_system_mgmt_macx(u8 *mgmt_mac);
BOOL ipmc_lib_get_ipintf_igmp_adrs(ipmc_intf_entry_t *intf, vtss_ipv4_t *ip4addr);
BOOL ipmc_lib_get_system_mgmt_intf(ipmc_intf_entry_t *intf, ipmc_mgmt_ipif_t *ipif);

BOOL ipmc_lib_time_curr_get(ipmc_time_t *now_t);
BOOL ipmc_lib_time_diff_get(BOOL prt, BOOL info_prt, char *str, ipmc_time_t *base_t, ipmc_time_t *diff_t);
ipmc_time_cmp_t ipmc_lib_time_cmp(ipmc_time_t *time_a, ipmc_time_t *time_b);
void ipmc_lib_time_cpy(ipmc_time_t *time_a, ipmc_time_t *time_b);
BOOL ipmc_lib_time_stamp(ipmc_time_t *time_target, ipmc_time_t *time_offset);

u8 ipmc_lib_calc_thread_tick(u32 *tick, u32 diff, u32 unit, u32 *overflow);
u32 ipmc_lib_diff_u32_wrap_around(u32 start, u32 end);
u16 ipmc_lib_diff_u16_wrap_around(u16 start, u16 end);
u8 ipmc_lib_diff_u8_wrap_around(u8 start, u8 end);

/* Cannot convert VTSS_ISID_LOCAL in Zero-Based ISID */
vtss_isid_t ipmc_lib_isid_convert(BOOL local_valid, vtss_isid_t isid_in);

i32 ipmc_lib_addrs_cmp_func(void *elm1, void *elm2);
i32 ipmc_lib_srclist_cmp_func(void *elm1, void *elm2);

void ipmc_lib_srclist_prepare(ipmc_intf_entry_t *intf,
                              ipmc_sfm_srclist_t *srclist,
                              void *group_record,
                              u16 idx,
                              u8 port);
BOOL ipmc_lib_srclist_struct_copy(ipmc_group_entry_t *grp, ipmc_db_ctrl_hdr_t *dest_list, ipmc_db_ctrl_hdr_t *src_list, u32 port);
BOOL ipmc_lib_sfm_logical_op(ipmc_group_entry_t *grp, u32 intf, ipmc_db_ctrl_hdr_t *res_list, ipmc_db_ctrl_hdr_t *a_list, ipmc_db_ctrl_hdr_t *b_list, int action, ipmc_db_ctrl_hdr_t *srct);
BOOL ipmc_lib_srclist_logical_op_pkt(ipmc_group_entry_t *grp, ipmc_intf_entry_t *intf, u8 port, int action, ipmc_db_ctrl_hdr_t *list, void *group_record, u16 src_num, BOOL unlnk, ipmc_db_ctrl_hdr_t *srct);
BOOL ipmc_lib_srclist_logical_op_set(ipmc_group_entry_t *grp, u8 port, int action, BOOL unlnk, ipmc_db_ctrl_hdr_t *srct, ipmc_db_ctrl_hdr_t *list, ipmc_db_ctrl_hdr_t *operand);
void ipmc_lib_srclist_logical_op_cmp(BOOL *chg, u8 port, ipmc_db_ctrl_hdr_t *list, ipmc_db_ctrl_hdr_t *operand);
BOOL ipmc_lib_grp_src_list_del4port(ipmc_db_ctrl_hdr_t *srct, u32 intf, ipmc_db_ctrl_hdr_t *head);
ipmc_bf_status ipmc_lib_bf_status_check(u8 *dst_ptr);

void ipmc_lib_set_ssm_range(ipmc_ip_version_t version, ipmc_prefix_t *prefix);
BOOL ipmc_lib_get_ssm_range(ipmc_ip_version_t version, ipmc_prefix_t *prefix);
/* Address Masking and Sanity Checking for IPMC Prefix */
BOOL ipmc_lib_prefix_maskingNchecking(ipmc_ip_version_t version, BOOL convert, ipmc_prefix_t *in, ipmc_prefix_t *out);
/* Address Matching for two (IPMC) Prefixes */
BOOL ipmc_lib_prefix_matching(ipmc_ip_version_t version, BOOL mc, ipmc_prefix_t *pfx1, ipmc_prefix_t *pfx2);
BOOL ipmc_lib_get_port_rpstatus(ipmc_ip_version_t version, u32 port);
void ipmc_lib_get_discovered_router_port_mask(ipmc_ip_version_t version, ipmc_port_bfs_t *port_mask);
void ipmc_lib_set_discovered_router_port_mask(ipmc_ip_version_t version, u32 port, BOOL state);
BOOL ipmc_lib_get_changed_rpstatus(ipmc_ip_version_t version, u32 port);
void ipmc_lib_set_changed_router_port(BOOL init, ipmc_ip_version_t version, u32 port);
BOOL ipmc_lib_get_eui64_linklocal_addr(vtss_ipv6_t *ipv6_addr);

char *ipmc_lib_op_action_txt(ipmc_operation_action_t op);
char *ipmc_lib_fltr_action_txt(ipmc_action_t action, ipmc_text_cap_t cap);
char *ipmc_lib_version_txt(ipmc_ip_version_t version, ipmc_text_cap_t cap);
char *ipmc_lib_severity_txt(ipmc_log_severity_t severity, ipmc_text_cap_t cap);
char *ipmc_lib_port_role_txt(mvr_port_role_t role, ipmc_text_cap_t cap);
char *ipmc_lib_ip_txt(ipmc_ip_version_t version, ipmc_text_cap_t cap);
char *ipmc_lib_frm_tagtype_txt(vtss_tag_type_t type, ipmc_text_cap_t cap);

ipmc_ip_version_t ipmc_lib_grp_adrs_version(vtss_ipv6_t *grps);
BOOL ipmc_lib_grp_adrs_boundary(ipmc_ip_version_t version, BOOL floor, vtss_ipv6_t *grps);
BOOL ipmc_lib_isaddr4_all_zero(ipmcv4addr *addr);
BOOL ipmc_lib_isaddr4_all_ones(ipmcv4addr *addr);
BOOL ipmc_lib_isaddr4_all_node(ipmcv4addr *addr);
BOOL ipmc_lib_isaddr4_all_router(ipmcv4addr *addr);
void ipmc_lib_get_all_zero_ipv4_addr(ipmcv4addr *addr);
void ipmc_lib_get_all_ones_ipv4_addr(ipmcv4addr *addr);
void ipmc_lib_get_all_node_ipv4_addr(ipmcv4addr *addr);
void ipmc_lib_get_all_router_ipv4_addr(ipmcv4addr *addr);

BOOL ipmc_lib_isaddr6_all_zero(vtss_ipv6_t *addr);
BOOL ipmc_lib_isaddr6_all_ones(vtss_ipv6_t *addr);
BOOL ipmc_lib_isaddr6_all_node(vtss_ipv6_t *addr);
BOOL ipmc_lib_isaddr6_all_router(vtss_ipv6_t *addr);
void ipmc_lib_get_all_zero_ipv6_addr(vtss_ipv6_t *addr);
void ipmc_lib_get_all_ones_ipv6_addr(vtss_ipv6_t *addr);
void ipmc_lib_get_all_node_ipv6_addr(vtss_ipv6_t *addr);
void ipmc_lib_get_all_router_ipv6_addr(vtss_ipv6_t *addr);

ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info);
ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info);
ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info, ipmc_time_t *current);
ipmc_group_db_t *ipmc_lib_fltr_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db);
ipmc_group_db_t *ipmc_lib_fltr_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db);
ipmc_group_db_t *ipmc_lib_fltr_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db, ipmc_time_t *current);
ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);
ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);
ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist, ipmc_time_t *current);

ipmc_group_entry_t *ipmc_lib_group_ptr_walk_start(void);
ipmc_group_entry_t *ipmc_lib_group_ptr_get_first(ipmc_db_ctrl_hdr_t *p);
ipmc_group_entry_t *ipmc_lib_group_ptr_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp);
ipmc_group_entry_t *ipmc_lib_group_ptr_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp);

BOOL ipmc_lib_group_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp);
BOOL ipmc_lib_group_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp);
ipmc_group_entry_t *ipmc_lib_group_init(ipmc_intf_entry_t *intf,
                                        ipmc_db_ctrl_hdr_t *p,
                                        ipmc_group_entry_t *grp);
ipmc_group_entry_t *ipmc_lib_group_sync(ipmc_db_ctrl_hdr_t *p,
                                        ipmc_intf_entry_t *grp_intf,
                                        ipmc_group_entry_t *grp,
                                        BOOL asm_only,
                                        proc_grp_tmp_t type);
BOOL ipmc_lib_group_delete(ipmc_intf_entry_t *ipmc_intf,
                           ipmc_db_ctrl_hdr_t *p,
                           ipmc_db_ctrl_hdr_t *rxmt,
                           ipmc_db_ctrl_hdr_t *fltr,
                           ipmc_db_ctrl_hdr_t *srct,
                           ipmc_group_entry_t *grp,
                           BOOL proxy, BOOL force);
BOOL ipmc_lib_group_update(ipmc_db_ctrl_hdr_t *p,
                           ipmc_group_entry_t *grp);

BOOL ipmc_lib_srclist_add(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp, ipmc_sfm_srclist_t *srclist, u8 alcid);
BOOL ipmc_lib_srclist_del(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist, u8 freid);
BOOL ipmc_lib_srclist_clear(ipmc_db_ctrl_hdr_t *p, u8 freid);
ipmc_sfm_srclist_t *ipmc_lib_srclist_adr_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);
ipmc_sfm_srclist_t *ipmc_lib_srclist_adr_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);
BOOL ipmc_lib_srclist_buf_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);
BOOL ipmc_lib_srclist_buf_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist);

void ipmc_lib_proc_grp_sfm_tmp4rcv(u8 is_mvr, BOOL clear, BOOL dosf, ipmc_group_entry_t *grp);
void ipmc_lib_proc_grp_sfm_tmp4tick(u8 is_mvr, BOOL clear, BOOL dosf, ipmc_group_entry_t *grp);
BOOL ipmc_lib_get_grp_sfm_tmp4rcv(u8 is_mvr, ipmc_group_entry_t **grp);
BOOL ipmc_lib_get_grp_sfm_tmp4tick(u8 is_mvr, ipmc_group_entry_t **grp);
ipmc_db_ctrl_hdr_t *ipmc_lib_get_sf_permit_srclist(u8 is_mvr, u32 port);
ipmc_db_ctrl_hdr_t *ipmc_lib_get_sf_deny_srclist(u8 is_mvr, u32 port);

vtss_rc ipmc_lib_protocol_intf_tmr(BOOL proxy_active, ipmc_intf_entry_t *ipmc_intf);
vtss_rc ipmc_lib_protocol_group_tmr(BOOL from_mvr,
                                    ipmc_db_ctrl_hdr_t *p,
                                    ipmc_db_ctrl_hdr_t *rxmt,
                                    ipmc_db_ctrl_hdr_t *fltr,
                                    ipmc_db_ctrl_hdr_t *srct,
                                    ipmc_port_throttling_t *g_throttling,
                                    BOOL *g_proxy,
                                    BOOL *l_proxy);
vtss_rc ipmc_lib_protocol_intf_rxmt(ipmc_db_ctrl_hdr_t *p,
                                    ipmc_db_ctrl_hdr_t *rxmt,
                                    ipmc_db_ctrl_hdr_t *fltr);
vtss_rc ipmc_lib_protocol_suppression(u16 *timer, u16 timeout, u16 *fld_cnt);
vtss_rc ipmc_lib_protocol_do_sfm_report(ipmc_db_ctrl_hdr_t *p,
                                        ipmc_db_ctrl_hdr_t *rxmt,
                                        ipmc_db_ctrl_hdr_t *fltr,
                                        ipmc_db_ctrl_hdr_t *srct,
                                        ipmc_intf_entry_t *entry,
                                        u8 *content,
                                        u8 src_port,
                                        u8 msgType,
                                        u32 ipmc_pkt_len,
                                        specific_grps_fltr_t *grps_fltr,
                                        int *throttling,
                                        BOOL proxy,
                                        ipmc_operation_action_t *op);
vtss_rc ipmc_lib_protocol_lower_filter_timer(ipmc_db_ctrl_hdr_t *fltr, ipmc_group_entry_t *grp, ipmc_intf_entry_t *entry, u8 ndx);
BOOL ipmc_lib_listener_set_reporting_timer(u16 *out_timer, u16 in_timer, u16 ref_timeout);
ipmc_send_act_t ipmc_lib_get_sq_ssq_action(BOOL proxy, ipmc_intf_entry_t *entry, u32 port);

/* Used to RX/TX IPMC frames */
vtss_rc ipmc_lib_packet_register(ipmc_owner_t owner, void *cb);
vtss_rc ipmc_lib_packet_unregister(ipmc_owner_t owner);
void ipmc_lib_packet_strip_vtag(const u8 *const frame, u8 *frm, vtss_tag_type_t tag_type, vtss_packet_rx_info_t *rx_info);
vtss_rc ipmc_lib_packet_rx(ipmc_intf_entry_t *entry, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info, ipmc_pkt_attribute_t *atr);
vtss_rc ipmc_lib_packet_tx(ipmc_port_bfs_t *dst_port_mask,
                           BOOL force_untag,
                           BOOL fast_leave,
                           u32 src_port,
                           ipmc_pkt_src_port_t src_type,
                           vtss_vid_t vid,
                           BOOL cfi,
                           u8 uprio,
                           vtss_glag_no_t glag_id,
                           const u8 *const frame,
                           size_t len);
BOOL ipmc_lib_packet_rx_mxrc_qqic(ipmc_pkt_exp_t type, void *input, u32 *output);
BOOL ipmc_lib_packet_tx_mxrc_qqic(ipmc_pkt_exp_t type, ipmc_intf_entry_t *input, void *output);

void ipmc_lib_packet_max_time_reset(void);
BOOL ipmc_lib_packet_max_time_get(ipmc_time_t *pkt_time);
vtss_rc ipmc_lib_packet_tx_gq(ipmc_intf_entry_t *entry, vtss_ipv6_t *query_group_addr, BOOL force_untag, BOOL debug);
vtss_rc ipmc_lib_packet_tx_sq(ipmc_db_ctrl_hdr_t *fltr, ipmc_send_act_t snd_act, ipmc_group_entry_t *grp, ipmc_intf_entry_t *entry, u8 src_port, BOOL force_untag);
vtss_rc ipmc_lib_packet_tx_ssq(ipmc_db_ctrl_hdr_t *fltr, ipmc_send_act_t snd_act, ipmc_group_entry_t *grp, ipmc_intf_entry_t *entry, u8 src_port, ipmc_db_ctrl_hdr_t *src_list, BOOL force_untag);
vtss_rc ipmc_lib_packet_tx_group_leave(ipmc_intf_entry_t *entry, vtss_ipv6_t *leave_group_addr, ipmc_port_bfs_t *dst_port_mask, BOOL force_untag, BOOL debug);
vtss_rc ipmc_lib_packet_tx_join_report(BOOL is_mvr, ipmc_compat_mode_t compat, ipmc_intf_entry_t *entry, vtss_ipv6_t *join_group_addr, ipmc_port_bfs_t *dst_port_mask, ipmc_ip_version_t version, BOOL force_untag, BOOL proxy, BOOL debug);
vtss_rc ipmc_lib_packet_tx_proxy_query(ipmc_intf_entry_t *entry, vtss_ipv6_t *query_group_addr, BOOL force_untag);
vtss_rc ipmc_lib_packet_tx_helping_query(ipmc_intf_entry_t *entry, vtss_ipv6_t *query_group_addr, ipmc_port_bfs_t *dst_port_mask, BOOL force_untag, BOOL debug);

vtss_rc ipmc_lib_forward_process_group_sfm(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *old_grp, ipmc_group_entry_t *new_grp);
vtss_rc ipmc_lib_porting_set_chip(BOOL op, ipmc_db_ctrl_hdr_t *p,
                                  ipmc_group_entry_t *grp_op,
                                  ipmc_ip_version_t version, vtss_vid_t ifid,
                                  vtss_ipv4_t ip4sip, vtss_ipv4_t ip4dip,
                                  vtss_ipv6_t ip6sip, vtss_ipv6_t ip6dip,
                                  BOOL fwd_map[VTSS_PORT_ARRAY_SIZE]);

BOOL ipmc_lib_isid_is_local(vtss_isid_t idx);

/* Set Global Filtering Profile State */
vtss_rc ipmc_lib_mgmt_profile_state_set(BOOL profiling);

/* Get Global Filtering Profile State */
vtss_rc ipmc_lib_mgmt_profile_state_get(BOOL *profiling);

/* Add/Delete/Update IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_entry_t *fltr_entry);

/* Get IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_get(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name);

/* GetNext IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_get_next(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name);

/* Add/Delete/Update IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_profile_t *fltr_profile);

/* Get IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_get(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name);

/* GetNext IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_get_next(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name);

/* Add/Delete/Update IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_set(ipmc_operation_action_t action, u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* Search IPMC Profile Rule by Entry Index */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_search(u32 profile_index, u32 entry_index, ipmc_lib_rule_t *fltr_rule);

/* Get IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get(u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* GetFirst IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get_first(u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* GetNext IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get_next(u32 profile_index, ipmc_lib_rule_t *fltr_rule);

/* Clear all the profile settings and running databases */
vtss_rc ipmc_lib_mgmt_clear_profile(vtss_isid_t isid_add);

/* Get Internal IPMC Profile Tree */
BOOL ipmc_lib_mgmt_profile_tree_get(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl);

/* GetNext Internal IPMC Profile Tree */
BOOL ipmc_lib_mgmt_profile_tree_get_next(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl);

/* Get Specific Internal IPMC Profile Tree VID */
BOOL ipmc_lib_mgmt_profile_tree_vid_get(u32 tdx, vtss_vid_t *pf_vid);

/* Get delta time for timers (filter timer & source timer) */
int ipmc_lib_mgmt_calc_delta_time(vtss_isid_t isid, ipmc_time_t *time_v);

#endif /* _IPMC_LIB_H_ */
