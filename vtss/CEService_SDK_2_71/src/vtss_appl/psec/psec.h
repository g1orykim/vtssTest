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

#ifndef _PSEC_H_
#define _PSEC_H_

#include <time.h>            /* For time_t                              */
#include "psec_rate_limit.h" /* For definition of psec_rate_limit_cfg_t */

/**
  * \brief Define this to correct for E-StaX-34 BPDU CPU Copy chip error
  *
  * \details When ANA::LEARNDROP[port] is set (kill of frames on @port from unknown SMACs)
  *          and  ANA::LEARNCPU[port] is cleared (don't even send learn frames from @port to the CPU)
  *          and  ANA::CAPENAB.BPDU is set, then BPDUs received on @port from unknown
  *          SMACs are also killed, instead of copied to the CPU, as expected
  *          and as the datasheet says (Rev. 02-05).
  *          By defining PSEC_FIX_GNATS_6935, this is remedied by opening up
  *          for CPU copy even if a user module says: "Don't allow any frames but BPDUs".
  *          The PSEC module then silently discards all frames, and expects them
  *          to be added through the psec_mgmt_mac_addr_add() function.
  */
#ifdef VTSS_CHIP_E_STAX_34
#define PSEC_FIX_GNATS_6935
#endif

// Interface between psec_cli.c and psec.c - mainly for debugging purposes.

/**
  * \brief Macro to set 2-bit per-user module forwarding decision.
  */
#define PSEC_FORWARD_DECISION_SET(_mac_state_, _user_, _add_method_)                                          \
  do {                                                                                                        \
    VTSS_BF_SET((_mac_state_)->forward_decision_mask, 2 * (int)(_user_) + 0, ((int)(_add_method_) >> 0) & 1); \
    VTSS_BF_SET((_mac_state_)->forward_decision_mask, 2 * (int)(_user_) + 1, ((int)(_add_method_) >> 1) & 1); \
  } while (0)

/**
  * \brief Macro to get 2-bit per-user module forwarding decision.
  */
#define PSEC_FORWARD_DECISION_GET(_mac_state_, _user_)                                \
  ((psec_add_method_t)(                                                               \
    (VTSS_BF_GET((_mac_state_)->forward_decision_mask, 2 * (int)(_user_) + 0) << 0) | \
    (VTSS_BF_GET((_mac_state_)->forward_decision_mask, 2 * (int)(_user_) + 1) << 1)))

/**
  * \brief Macro to set user-enabledness.
  */
#define PSEC_USER_ENA_SET(_port_state_, _user_, _enable_) \
  do {                                                    \
    if (_enable_) {                                       \
      (_port_state_)->ena_mask |=  (1U << (int)(_user_)); \
    } else {                                              \
      (_port_state_)->ena_mask &= ~(1U << (int)(_user_)); \
    }                                                     \
  } while (0)

/**
  * \brief Macro to get a user's enabledness.
  */
#define PSEC_USER_ENA_GET(_port_state_, _user_)            \
  (((_port_state_)->ena_mask & (1U << (u32)(_user_))) ? TRUE : FALSE)

/**
  * \brief Macro for setting the user's preferred Secure Learning CPU copy method
  */
#define PSEC_PORT_MODE_SET(_port_state_, _user_, _port_mode_) \
  do {                                                        \
    if ((_port_mode_) == PSEC_PORT_MODE_KEEP_BLOCKED) {       \
      (_port_state_)->mode_mask |=  (1U << (int)(_user_));    \
    } else {                                                  \
      (_port_state_)->mode_mask &= ~(1U << (int)(_user_));    \
    }                                                         \
  } while (0)

/**
  * \brief Macro for getting the user's preferred Secure Learning CPU copy method
  */
#define PSEC_PORT_MODE_GET(_port_state_, _user_) \
  (((_port_state_)->mode_mask & (1U << (u32)(_user_))) ? PSEC_PORT_MODE_KEEP_BLOCKED : PSEC_PORT_MODE_NORMAL)

/**
  * \brief Macro for setting a user's Force CPU-copy bit.
  */
#ifdef PSEC_FIX_GNATS_6935
#define PSEC_FORCE_CPU_COPY_SET(_port_state_, _user_, _force_cpu_copy_) \
    do {                                                                  \
      if ((_force_cpu_copy_)) {                                           \
        (_port_state_)->force_cpu_copy_mask |=  (1U << (int)(_user_));    \
      } else {                                                            \
        (_port_state_)->force_cpu_copy_mask &= ~(1U << (int)(_user_));    \
      }                                                                   \
    } while (0)
#endif

/**
  * \brief Macro for getting a user's Force CPU-copy bit.
  */
#ifdef PSEC_FIX_GNATS_6935
#define PSEC_FORCE_CPU_COPY_GET(_port_state_, _user_) \
    (((_port_state_)->force_cpu_copy_mask & (1U << (u32)(_user_))) ? TRUE : FALSE)
#endif

/**
  * \brief State flags for one MAC Address entry
  */
enum {
    PSEC_MAC_STATE_FLAGS_BLOCKED        = (1 << 0), /**< This entry is added blocked to the MAC table.                      */
    PSEC_MAC_STATE_FLAGS_KEEP_BLOCKED   = (1 << 1), /**< Not subject to hold-time expiration. BLOCKED bit will also be set. */
    PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE  = (1 << 2), /**< The entry is present in the MAC module.                            */
    PSEC_MAC_STATE_FLAGS_CPU_COPYING    = (1 << 3), /**< The entry is marked as CPU copying due to aging                    */
    PSEC_MAC_STATE_FLAGS_AGE_FRAME_SEEN = (1 << 4), /**< An age frame has been seen since CPU copying was set.              */
    PSEC_MAC_STATE_FLAGS_HW_ADD_FAILED  = (1 << 5), /**< This entry couldn't get added to the MAC table (H/W failure).      */
    PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED  = (1 << 6), /**< This entry couldn't get added to the MAC table (S/W failure).      */
}; // To satisfy Lint, we make this enum anonymous and whereever it's used, we declare an u32. I would've liked to call this "psec_mac_state_flags_t".

/**
  * \brief MAC Address state
  *
  * Each instance of this structure is used to manage one MAC address.
  */
typedef struct tag_psec_mac_state_t {

    /**
      * The VID and MAC address that this is all about
      */
    vtss_vid_mac_t vid_mac;

    /**
      * And it was originally added at this time
      */
    time_t creation_time_secs;

    /**
      * Here is when it was last changed in the MAC table
      */
    time_t changed_time_secs;

    /**
      * Properties of this entry.
      * To satisfy Lint, declare an u32, which can take
      * a combination of the PSEC_MAC_STATE_FLAGS_xxx above
      */
    u32 flags;

    /** During "MAC add" callbacks we let go of the crit that
      * protects our internal state. We use this field to ensure
      * that nothing has happened to the internal state during the
      * callbacks. This number is a unique, ever-increasing
      * number that is only zero when this mac_state is in
      * the free pool.
      */
    u32 unique;

    /**
      * Down-counter used in block and aging process
      */
    u32 age_or_hold_time_secs;

    /**
      * Points to previous entry on this port.
      *
      * This is an internal parameter only, and will
      * always be NULL if a list of MAC addresses
      * is obtained with psec_mgmt_port_state_get().
      */
    struct tag_psec_mac_state_t *prev;

    /**
      * Points to next entry on this port
      */
    struct tag_psec_mac_state_t *next;

    /**
      * Forward decision per user module.
      * Unfortunately, we have to do it this way, because
      * we want to save space, and we need to know all
      * modules' decision, and the decision takes more than one
      * bit.
      * Each user module takes two bits, which are encoded
      * as follows:
      * 00 = Forward
      * 01 = Block and use hold timer
      * 10 = Block and don't use hold timer
      * 11 = Unused.
      */
    u8 forward_decision_mask[VTSS_BF_SIZE(2 * (int)PSEC_USER_CNT)];
} psec_mac_state_t;

/**
  * \brief Port state flags.
  */
enum {
    PSEC_PORT_STATE_FLAGS_SEC_LEARNING  = (1 << 0), /**< Determines whether we're currently secure learning (with or w/o CPU copy) */
    PSEC_PORT_STATE_FLAGS_CPU_COPYING   = (1 << 1), /**< Determines whether the port is currently copying learn frames to the CPU. */
    PSEC_PORT_STATE_FLAGS_LINK_UP       = (1 << 2), /**< Determines whether there is link on the port.                             */
    PSEC_PORT_STATE_FLAGS_SHUT_DOWN     = (1 << 3), /**< Determines whether the port is shut down by the PSEC LIMIT module.        */
    PSEC_PORT_STATE_FLAGS_LIMIT_REACHED = (1 << 4), /**< Determines whether the port's max client limit is reached.                */
    PSEC_PORT_STATE_FLAGS_HW_ADD_FAILED = (1 << 5), /**< Determines whether a H/W add failure was detected on this port.           */
    PSEC_PORT_STATE_FLAGS_SW_ADD_FAILED = (1 << 6), /**< Determines whether a S/W add failure was detected on this port.           */
};  // To satisfy Lint, we make this enum anonymous and whereever it's used, we declare an u32. I would've liked to call this "psec_port_state_flags_t".

/**
  * \brief Port state
  *
  * Each instance of this structure is used to manage the MAC addresses
  * on one port.
  */
typedef struct {
    /**
      * Number of MAC addresses currently assigned to this port.
      * The number does NOT include the number of entries that
      * are held due to a H/W failure or S/W failure.
      */
    u32 mac_cnt;

    /**
      * The MAC addresses attached to this port. It also includes those entries
      * that couldn't be saved to the MAC table due to H/W limitations or
      * MAC Module S/W limitations.
      * These entries are marked with PSEC_MAC_STATE_FLAGS_HW_ADD_FAIL/
      * PSEC_MAC_STATE_FLAGS_SW_ADD_FAILED.
      * The user-modules have no knowledge about entries marked as such.
      *
      * If psec_mgmt_port_state_get() is called with #get_macs == FALSE,
      * then this will be NULL, otherwise it will point to an area that
      * is dynamically allocated with VTSS_MALLOC(), and which must be freed
      * by caller (if non-NULL).
      */
    psec_mac_state_t *macs;

    /**
      * Determines whether the port is currently administratively shut down.
      * To satisfy Lint, declare an u32, which can take
      * a combination of the PSEC_PORT_STATE_FLAGS_xxx above
      */
    u32 flags;

    /**
      * This is a bitmask with bits set for those modules in psec_users_t that are enabled.
      * The reason that it's not an u8[VTSS_BF_SIZE(bla-bla)] is that we want to quickly
      * be able to determine if there's at least one user on the port. So rather than
      * going through an u8 array (or even worse: bit-by-bit), we can simply do an
      * if (ena_mask) ...
      * Of course, this sets an upper limit on the number of user-modules we can support,
      * but I guess 32 is enough for now. There is a runtime check on the number of
      * users in psec_init().
      */
    u32 ena_mask;

    /**
      * This is a bitmask with bits set for those user modules that require the port to be kept
      * in non-CPU-copying state.
      * 0 = Normal (let PSEC control whether CPU copying can get enabled)
      * 1 = Keep blocked (User module dictates that this port is kept blocked).
      */
    u32 mode_mask;

    /**
      * This is a bitmask with bits set for those user modules that require the port to
      * be kept in CPU-copy-force state. It is only needed if BPDUs are not copied
      * to the CPU if secure learning is enabled and CPU-copying is disabled.
      * 0 = User module doesn't care.
      * 1 = Force CPU-copy enabled (unless port is shut down).
      */
#ifdef PSEC_FIX_GNATS_6935
    u32 force_cpu_copy_mask;
#endif
} psec_port_state_t;

vtss_rc psec_mgmt_port_state_get(vtss_isid_t isid, vtss_port_no_t port, psec_port_state_t *state, BOOL crit_already_obtained);
vtss_rc psec_dbg_shaper_cfg_set(psec_rate_limit_cfg_t *cfg);
vtss_rc psec_dbg_shaper_cfg_get(psec_rate_limit_cfg_t *cfg);
vtss_rc psec_dbg_shaper_stat_get(psec_rate_limit_stat_t *stat, u64 *cur_fill_level);
vtss_rc psec_dbg_shaper_stat_clr(void);
char *psec_user_name(psec_users_t user);
char  psec_user_abbr(psec_users_t user);

#endif /* _PSEC_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
