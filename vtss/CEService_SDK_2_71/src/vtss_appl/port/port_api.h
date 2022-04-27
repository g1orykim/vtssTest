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
 
 $Id$
 $Revision$

*/
#ifndef _VTSS_APPL_PORT_API_H_
#define _VTSS_APPL_PORT_API_H_

#include "main.h"
#include "port_custom_api.h"
#include "vtss_mgmt_port_api.h"

/* ================================================================= *
 *  Stack port information
 * ================================================================= */

/* Stack ports */
#if VTSS_SWITCH_STACKABLE
#define PORT_NO_STACK_0 port_no_stack(0)
#define PORT_NO_STACK_1 port_no_stack(1)
#endif /* VTSS_SWITCH_STACKABLE */

#define PORT_NO_IS_STACK(port_no) port_no_is_stack(port_no)

#if VTSS_SWITCH_STACKABLE
#define VTSS_FRONT_PORT_COUNT (VTSS_PORTS - 2)
#else
#define VTSS_FRONT_PORT_COUNT (VTSS_PORTS)
#endif

/* Port number is stack port? */
BOOL port_no_is_stack(vtss_port_no_t port_no);

/* Port number of stack port 0 or 1 */
vtss_port_no_t port_no_stack(BOOL port_1);

/* Get port capability */
vtss_rc port_cap_get(vtss_port_no_t port_no, port_cap_t *cap);



// Returns if a port is PHY (if the PHY is in pass through mode, it shall act as it is not a PHY)
BOOL port_phy(vtss_port_no_t port_no);

/* Determine if port has a PHY */
    BOOL is_port_phy(vtss_port_no_t port_no);

/* Wait until all PHYs have been reset */
void port_phy_wait_until_ready(void);

/* ================================================================= *
 *  Port information per ISID
 * ================================================================= */

/* Port related information per switch */
typedef struct {
    int            board_type;   /* Board type */
    u32            port_count;   /* Number of ports */
    vtss_port_no_t stack_port_0; /* Stack port 0 or VTSS_PORT_NO_NONE */
    vtss_port_no_t stack_port_1; /* Stack port 1 or VTSS_PORT_NO_NONE */
    port_cap_t     cap;          /* Combined port capabilities */
} port_isid_info_t;


/* Function returning the board type for a given isid
   In : isid - The switch id for which to return the board type.
   Return - Board type. */
vtss_board_type_t port_isid_info_board_type_get(vtss_isid_t isid);

/* Get port related information for ISID */
vtss_rc port_isid_info_get(vtss_isid_t isid, port_isid_info_t *info);

/* Port information per ISID and port */
typedef struct {
    port_cap_t     cap;       /* Capability */
    uint           chip_port; /* Chip port number */
    vtss_chip_no_t chip_no;   /* Chip number */
} port_isid_port_info_t;

/* Get information for ISID and port */
vtss_rc port_isid_port_info_get(vtss_isid_t isid, vtss_port_no_t port_no, 
                                port_isid_port_info_t *info);

/* Get number of ports for ISID including the stack ports */
u32 port_isid_port_count(vtss_isid_t isid);

/* Determine if ISID port is a stack port */
BOOL port_isid_port_no_is_stack(vtss_isid_t isid, vtss_port_no_t port_no);

/* Determine if ISID port is a front port 
   In : isid - The switch id for the switch to check.
      : port_no - Port number to check.
   Return : TRUE if the port at the given switch is a front port else FALSE*/
BOOL port_isid_port_no_is_front_port(vtss_isid_t isid, vtss_port_no_t port_no);

/* Get current maximum port count */
u32 port_count_max(void);

/* ================================================================= *
 *  Switch iteration.
 * ================================================================= */
/**
 * \brief Private declaration used internally
 **/
typedef enum {
    SWITCH_ITER_STATE_FIRST, /**< This is the first call of switch_iter_getnext(). */
    SWITCH_ITER_STATE_NEXT,  /**< This is one of the following calls of switch_iter_getnext(). */
    SWITCH_ITER_STATE_LAST,  /**< This is the last call of switch_iter_getnext(). Next time switch_iter_getnext() returns FALSE. */
    SWITCH_ITER_STATE_DONE   /**< switch_iter_getnext() has returned FALSE and we are done. */
} switch_iter_state_t;

typedef enum {
    SWITCH_ITER_SORT_ORDER_ISID,        /**< Return the existing switches in isid order. */
    SWITCH_ITER_SORT_ORDER_USID,        /**< Return the existing switches in usid order. */
    SWITCH_ITER_SORT_ORDER_ISID_CFG,    /**< Return the configurable switches in isid order. */
    SWITCH_ITER_SORT_ORDER_USID_CFG,    /**< Return the configurable switches in usid order. */
    SWITCH_ITER_SORT_ORDER_ISID_ALL,    /**< Return the existing and non-existing switches in isid order. */
    SWITCH_ITER_SORT_ORDER_END = SWITCH_ITER_SORT_ORDER_ISID_ALL  /* Must be last value in enum */
} switch_iter_sort_order_t;

typedef struct {
// private: Do not use these variables!
    u32                      m_switch_mask;        /**< Bitmask of switches indexed with isid or usid. */
    u32                      m_exists_mask;        /**< Bitmask of existing switches indexed with isid or usid. */
    switch_iter_state_t      m_state;              /**< Internal state of iterator. */
    switch_iter_sort_order_t m_order;              /**< Configured sort_order. */
    vtss_isid_t              m_sid;                /**< Internal sid variable. */
// public: These variables are read-only and are valid after each switch_iter_getnext()
    vtss_isid_t              isid;                 /**< The current isid. */
    vtss_usid_t              usid;                 /**< The current usid. Not valid with SWITCH_ITER_SORT_ORDER_ISID_ALL (always zero). */
    BOOL                     first;                /**< The current switch is the first one. */
    BOOL                     last;                 /**< The current switch is the last one. The next call to switch_iter_getnext() will return FALSE. */
    BOOL                     exists;               /**< The current switch exists. */
    uint                     remaining;            /**< The remaining number of times a call to switch_iter_getnext() will return TRUE. */
} switch_iter_t;

/**
 * \brief Initialize a switch iterator.
 *
 * If any of the parameters are invalid, it'll return VTSS_INVALID_PARAMETER.
 * Otherwise it'll return VTSS_OK.
 *
 * \param sit        [IN] Switch iterator.
 * \param isid       [IN] isid selector. Valid values are VTSS_ISID_START to (VTSS_ISID_END - 1) or VTSS_ISID_GLOBAL.
 * \param sort_order [IN] Sorting order.
 *
 * \return Return code.
 **/
vtss_rc switch_iter_init(switch_iter_t *sit, vtss_isid_t isid, switch_iter_sort_order_t sort_order);

/**
 * \brief Get the next switch.
 *
 * The number of times switch_iter_getnext() returns a switch is dependant on how the iterator was initialized.
 *
 * If isid == VTSS_ISID_GLOBAL, the returned number of switches is 0 to VTSS_ISID_CNT:
 *   If sort_order != SWITCH_ITER_SORT_ORDER_ISID_ALL, all existing (or configurable) switches are returned by switch_iter_getnext().
 *   If sort_order == SWITCH_ITER_SORT_ORDER_ISID_ALL, all existing and non-existing switches are returned by switch_iter_getnext().
 *
 * If isid != VTSS_ISID_GLOBAL, the returned number of switches is 0 to 1:
 *   If sort_order != SWITCH_ITER_SORT_ORDER_ISID_ALL and the switch exists (or is configurable) it is returned by switch_iter_getnext().
 *   If sort_order == SWITCH_ITER_SORT_ORDER_ISID_ALL the switch is always returned by switch_iter_getnext().
 *
 * If switch_iter_getnext() is called on a slave or if switch_iter_init() is called with invalid parameters, it silently returns 0 switches.
 *
 * \param sit  [IN] Switch iterator.
 *
 * \return TRUE if a switch is found, otherwise FALSE.
 **/
BOOL switch_iter_getnext(switch_iter_t *sit);

/* ================================================================= *
 *  Port iteration.
 * ================================================================= */
/**
 * \brief Private declaration used internally
 **/
typedef enum {
    PORT_ITER_STATE_FIRST, /**< This is the first call of port_iter_getnext(). */
    PORT_ITER_STATE_NEXT,  /**< This is one of the following calls of port_iter_getnext(). */
    PORT_ITER_STATE_LAST,  /**< This is the last call of port_iter_getnext(). Next time port_iter_getnext() returns FALSE. */
    PORT_ITER_STATE_DONE,  /**< port_iter_getnext() has returned FALSE and we are done. */
    PORT_ITER_STATE_INIT   /**< Get next switch and reinitialize port iterator with the new isid. */
} port_iter_state_t;

typedef enum {
    PORT_ITER_SORT_ORDER_IPORT,    /**< Return the existing ports in iport order */
    PORT_ITER_SORT_ORDER_UPORT,    /**< Return the existing ports in uport order */
    PORT_ITER_SORT_ORDER_IPORT_ALL /**< Return the existing and non-existing ports in iport order */
} port_iter_sort_order_t;

typedef enum {
    PORT_ITER_TYPE_FRONT,    /**< This is a front port. */
    PORT_ITER_TYPE_STACK,    /**< This is a stack port. */
    PORT_ITER_TYPE_LOOPBACK, /**< This is a loopback port. */
    PORT_ITER_TYPE_TRUNK,    /**< This is a trunk port. */
    PORT_ITER_TYPE_NPI,      /**< This is a NPI port. */
    PORT_ITER_TYPE_LAST      /**< This must always be the last one. */
} port_iter_type_t;

typedef enum {
    PORT_ITER_FLAGS_FRONT    = (1 << PORT_ITER_TYPE_FRONT),      /**< Return front ports. */
    PORT_ITER_FLAGS_STACK    = (1 << PORT_ITER_TYPE_STACK),      /**< Return stack ports. */
    PORT_ITER_FLAGS_LOOPBACK = (1 << PORT_ITER_TYPE_LOOPBACK),   /**< Return loopback ports. */
    PORT_ITER_FLAGS_TRUNK    = (1 << PORT_ITER_TYPE_TRUNK),      /**< Return trunk ports. */
    PORT_ITER_FLAGS_NPI      = (1 << PORT_ITER_TYPE_NPI),        /**< Return NPI ports. */
    PORT_ITER_FLAGS_NORMAL   =  PORT_ITER_FLAGS_FRONT,           /**< This is for normal use. */
    PORT_ITER_FLAGS_ALL      = (PORT_ITER_FLAGS_FRONT    |
                                PORT_ITER_FLAGS_STACK    |
                                PORT_ITER_FLAGS_LOOPBACK |
                                PORT_ITER_FLAGS_TRUNK    |
                                PORT_ITER_FLAGS_NPI),            /**< All port types. */
    PORT_ITER_FLAGS_UP       = (1 << PORT_ITER_TYPE_LAST),       /**< Only return ports with link. */
    PORT_ITER_FLAGS_DOWN     = (1 << (PORT_ITER_TYPE_LAST + 1))  /**< Only return ports without link. */
} port_iter_flags_t;

typedef struct {
// private: Do not use these variables!
    u64                    m_port_mask;        /**< Bitmask of ports indexed with iport or uport. */
    u64                    m_exists_mask;      /**< Bitmask of existing ports indexed with iport or uport. */
    port_iter_state_t      m_state;            /**< Internal state of iterator. */
    switch_iter_t         *m_sit;              /**< Configured switch iterator. */
    vtss_isid_t            m_isid;             /**< Configured isid. */
    port_iter_sort_order_t m_order;            /**< Configured sort_order. */
    port_iter_flags_t      m_flags;            /**< Configured port flags. */
    vtss_port_no_t         m_port;             /**< Internal port variable. */
// public: These variables are read-only and are valid after each port_iter_getnext()
    vtss_port_no_t         iport;              /**< The current iport */
    vtss_port_no_t         uport;              /**< The current uport */
    BOOL                   first;              /**< The current port is the first one */
    BOOL                   last;               /**< The current port is the last one. The next call to port_iter_getnext() will return FALSE. */
    BOOL                   exists;             /**< The current port exists. */
    BOOL                   link;               /**< The current link state. */
    port_iter_type_t       type;               /**< The current port type. Will never contain more than one type (only one bit set). */
} port_iter_t;

/**
 * \brief Initialize a port iterator.
 *
 * If this function is called on a slave and sit == NULL and isid != VTSS_ISID_LOCAL, it'll return VTSS_UNSPECIFIED_ERROR.
 * If this function is called on a slave and sit != NULL, it'll return VTSS_UNSPECIFIED_ERROR.
 * If any of the parameters are invalid, it'll return VTSS_INVALID_PARAMETER.
 * Otherwise it'll return VTSS_OK.
 *
 * \param pit        [IN] Port iterator.
 * \param sit        [IN] Switch iterator. If sit != NULL, the port iterator iterates over all isid's returned from the switch iterator.
 *                        If sit == NULL, the port iterator iterates over the isid in the isid selector parameter.
 * \param isid       [IN] isid selector. Valid values are VTSS_ISID_START to (VTSS_ISID_END - 1) or VTSS_ISID_LOCAL when sit == NULL.
 *                        Not used if sit != NULL.
 * \param sort_order [IN] Sorting order.
 * \param flags      [IN] Port type(s) to be returned. Several port flags can be or'ed together.
 *
 * \return Return code.
 **/
vtss_rc port_iter_init(port_iter_t *pit, switch_iter_t *sit, vtss_isid_t isid, port_iter_sort_order_t sort_order, port_iter_flags_t flags);

/* Initialize port iterator for VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT and PORT_ITER_FLAGS_NORMAL */
void port_iter_init_local(port_iter_t *pit);

/* Initialize port iterator for VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT_ALL and PORT_ITER_FLAGS_NORMAL */
void port_iter_init_local_all(port_iter_t *pit);

/**
 * \brief Get the next port.
 *
 * The number of times port_iter_getnext() returns a port is dependant on how the iterator was initialized.
 *
 * If isid == VTSS_ISID_LOCAL, we iterate over the local ports. This is valid on both slave and master.
 * If isid != VTSS_ISID_LOCAL, we iterate over the ports on the specific switch. This is only valid on master.
 *
 * If sort_order == PORT_ITER_SORT_ORDER_IPORT_ALL, we iterate over all ports. Even if we know that the actual port count is 
 * less than the maximum port count or the switch doesn't exist, we will return them all.
 * If sort_order != PORT_ITER_SORT_ORDER_IPORT_ALL, we iterate over the actual port count if the switch exist. Otherwise we
 * return the maximum number of ports.
 *
 * If PORT_ITER_TYPE_FRONT is excluded from type, port_iter_getnext() may return 0 ports.
 *
 * \param pit  [IN] Port iterator.
 *
 * \return TRUE if a port is found, otherwise FALSE.
 **/
BOOL port_iter_getnext(port_iter_t *pit);

/* ================================================================= *
 *  Local port change events
 * ================================================================= */

/* Port change callback */
typedef void (*port_change_callback_t)(vtss_port_no_t port_no, port_info_t *info);

/* Port change callback registration */
vtss_rc port_change_register(vtss_module_id_t module_id, port_change_callback_t callback);

/* Port change registration */
typedef struct {
    uchar                  port_done[VTSS_PORT_BF_SIZE]; /* Port mask for initial event done */
    port_change_callback_t callback;                     /* User callback function */
    vtss_module_id_t       module_id;                    /* Module ID */
    cyg_tick_count_t       max_ticks;                    /* Maximum ticks */
} port_change_reg_t;

/* Get/clear port change registration */
vtss_rc port_change_reg_get(port_change_reg_t *reg, BOOL clear);

/* ================================================================= *
 *  Global port change events
 * ================================================================= */

/* Port global change callback */
typedef void (*port_global_change_callback_t)(
    vtss_isid_t    isid,
    vtss_port_no_t port_no,
    port_info_t    *info);

/* Port change callback registration - global (stack) version */
vtss_rc port_global_change_register(vtss_module_id_t module_id, 
                                    port_global_change_callback_t callback);

/* Port global change registration */
typedef struct {
    port_global_change_callback_t callback;  /* User callback function */
    vtss_module_id_t              module_id; /* Module ID */
    cyg_tick_count_t              max_ticks; /* Maximum ticks */
} port_global_change_reg_t;

/* Get/clear port change registration */
vtss_rc port_global_change_reg_get(port_global_change_reg_t *reg, BOOL clear);

/* ================================================================= *
 *  Local port shutdown events
 * ================================================================= */

/* Port shutdown callback */
typedef void (*port_shutdown_callback_t)(vtss_port_no_t port_no);

/* Port shutdown registration */
vtss_rc port_shutdown_register(vtss_module_id_t module_id, port_shutdown_callback_t callback);

/* Port shutdown registration entry */
typedef struct {
    port_shutdown_callback_t callback;  /* User callback function */
    vtss_module_id_t         module_id; /* Module ID */
    cyg_tick_count_t         max_ticks; /* Maximum ticks */
} port_shutdown_reg_t;

/* Get/clear port change registration */
vtss_rc port_shutdown_reg_get(port_shutdown_reg_t *reg, BOOL clear);

/* ================================================================= *
 *  Local port information
 * ================================================================= */

/* Convert chip port to logical port */
vtss_port_no_t port_physical2logical(vtss_chip_no_t chip_no, uint chip_port, vtss_glag_no_t *glag_no);

/* ================================================================= *
 *  Volatile port configuration APIs
 * ================================================================= */

/* Volatile port configuration users */
typedef enum {
    PORT_USER_STATIC,

#ifdef VTSS_SW_OPTION_ACL
    PORT_USER_ACL,
#endif /* VTSS_SW_OPTION_ACL */

#ifdef VTSS_SW_OPTION_THERMAL_PROTECT
    PORT_USER_THERMAL_PROTECT,
#endif /* VTSS_SW_OPTION_THERMAL_PROTECT */

#ifdef VTSS_SW_OPTION_LOOP_PROTECT
    PORT_USER_LOOP_PROTECT,
#endif /* VTSS_SW_OPTION_LOOP_PROTECT */

#if defined(VTSS_SW_OPTION_MEP)
    PORT_USER_MEP,
#endif /* VTSS_SW_OPTION_MEP */

#if defined(VTSS_SW_OPTION_EFF)
    PORT_USER_EFF,
#endif /* VTSS_SW_OPTION_EFF */

    PORT_USER_CNT
} port_user_t;

/* Volatile port configuration */
typedef struct {
    BOOL             disable; /* Administrative disable port */
    vtss_port_loop_t loop;    /* Port loopback */
    BOOL             oper_up; /* Force operational mode up */
} port_vol_conf_t;

/* Get volatile port configuration */
vtss_rc port_vol_conf_get(port_user_t user, vtss_isid_t isid, 
                          vtss_port_no_t port_no, port_vol_conf_t *conf);

/* Set volatile port configuration */
vtss_rc port_vol_conf_set(port_user_t user, vtss_isid_t isid, 
                          vtss_port_no_t port_no, const port_vol_conf_t *conf);

/* Volatile port status */
typedef struct {
    port_vol_conf_t conf;
    port_user_t     user;
    char            name[64];
} port_vol_status_t;

/* Get volatile port status (PORT_USER_CNT means combined) */
vtss_rc vtss_port_vol_status_get(port_user_t user, vtss_isid_t isid,
                                 vtss_port_no_t port_no, port_vol_status_t *status);
#define port_vol_status_get(...) vtss_port_vol_status_get(__VA_ARGS__)

/* ================================================================= *
 *  Port Module Web Alert APIs
 * ================================================================= */

/**
 * \brief Callback function pointer type to be passed to
 *        port_web_set_notice_callback() and returned by
 *        port_web_get_notice_callback().
 */
typedef ssize_t (*notice_callback_t)(char *buffer, int maxlen);

/**
 * \brief Register callback function to "port" module. The callback
 *        function is called when main page of port status is shown.
 * \param new_callback_function [IN] The function pointer of callback function.
 */
void port_web_set_notice_callback(notice_callback_t new_callback_function);

/**
 * \brief Returns currently registered callback function pointer, which was
 *        registered by port_web_set_notice_callback() previously.
 * \return Currently registered callback function pointer. NULL if non is
 *         registered.
 */
notice_callback_t port_web_get_notice_callback(void);

/* ================================================================= *
 *  Management APIs
 * ================================================================= */

/* Port configuration */
typedef port_custom_conf_t port_conf_t;

/* Detected SFP tranceiver types */
typedef enum {
    PORT_SFP_NONE,
    PORT_SFP_NOT_SUPPORTED,
    PORT_SFP_100FX,
    PORT_SFP_1000BASE_T,
    PORT_SFP_1000BASE_CX,
    PORT_SFP_1000BASE_SX,
    PORT_SFP_1000BASE_LX,
    PORT_SFP_1000BASE_X,
    PORT_SFP_2G5,
    PORT_SFP_5G,
    PORT_SFP_10G,
} sfp_tranceiver_t;

typedef struct {
    char  vendor_name[20]; /* MSA vendor name (20-35) */
    char  vendor_pn[20];   /* MSA vendor pn   (40-55) */
    char  vendor_rev[6];   /* MSA vendor rev  (56-59) */
    sfp_tranceiver_t type;
} port_sfp_t;

/* Port power status */
typedef struct {
    BOOL                  actiphy_capable; // TRUE when port is able to use actiphy
    BOOL                  perfectreach_capable; // TRUE when port is able to use perfectreach
    BOOL                  actiphy_power_savings; // TRUE when port is saving power due to actiphy
    BOOL                  perfectreach_power_savings; // TRUE when port is saving power due to perfectreach
} port_power_status_t;


/* Port status and abilities */
typedef struct {
    vtss_port_status_t status;
    port_cap_t            cap;
    BOOL                  fiber;
    port_sfp_t            sfp;
    vtss_port_interface_t mac_if;   
    vtss_chip_no_t        chip_no;  
    uint                  chip_port;
    u32                   port_up_count;
    u32                   port_down_count;
    port_power_status_t   power; // Power savings
} port_status_t;

/* Port mode text string */
char *vtss_port_mgmt_mode_txt(vtss_port_no_t port_no, vtss_port_speed_t speed, BOOL fdx, BOOL fiber);
#define port_mgmt_mode_txt(...) vtss_port_mgmt_mode_txt(__VA_ARGS__)

/* Fiber port mode text string */
char *vtss_port_fiber_mgmt_mode_txt(vtss_fiber_port_speed_t speed, BOOL auto_neg);
#define port_fiber_mgmt_mode_txt(...) vtss_port_fiber_mgmt_mode_txt(__VA_ARGS__)

/* Get port configuration */
vtss_rc vtss_port_mgmt_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf);
#define port_mgmt_conf_get(...) vtss_port_mgmt_conf_get(__VA_ARGS__)

/* Set port configuration */
vtss_rc vtss_port_mgmt_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *conf);
#define port_mgmt_conf_set(...) vtss_port_mgmt_conf_set(__VA_ARGS__)

/* Get port status */
vtss_rc vtss_port_mgmt_status_get(vtss_isid_t isid, vtss_port_no_t port_no, port_status_t *status);
#define port_mgmt_status_get(...) vtss_port_mgmt_status_get(__VA_ARGS__)

/* Get all port status (including power status) */
vtss_rc vtss_port_mgmt_status_get_all(vtss_isid_t isid, vtss_port_no_t port_no, 
                                      port_status_t *status);
#define port_mgmt_status_get_all(...) vtss_port_mgmt_status_get_all(__VA_ARGS__)

/* Get port counters */
vtss_rc vtss_port_mgmt_counters_get(vtss_isid_t isid, 
                               vtss_port_no_t port_no, vtss_port_counters_t *counters);
#define port_mgmt_counters_get(...) vtss_port_mgmt_counters_get(__VA_ARGS__)

/* Clear port counters */
vtss_rc vtss_port_mgmt_counters_clear(vtss_isid_t isid, vtss_port_no_t port_no);
#define port_mgmt_counters_clear(...) vtss_port_mgmt_counters_clear(__VA_ARGS__)

/* Set defaults for "port_conf"  */
vtss_rc vtss_port_conf_default(vtss_isid_t isid, vtss_port_no_t port_no, port_conf_t *port_conf);

/* VeriPHY state text string */
const char *vtss_port_mgmt_veriphy_txt(vtss_phy_veriphy_status_t status);
#define port_mgmt_veriphy_txt(...) vtss_port_mgmt_veriphy_txt(__VA_ARGS__)

/* VeriPHY operation mode */
typedef enum {
    PORT_VERIPHY_MODE_NONE,      /* No VeriPHY at all */
    PORT_VERIPHY_MODE_BASIC,     /* No length or cross pair search */
    PORT_VERIPHY_MODE_NO_LENGTH, /* No length search */
    PORT_VERIPHY_MODE_FULL       /* Full VeriPHY process */
} port_veriphy_mode_t;

/* Start VeriPHY */
vtss_rc vtss_port_mgmt_veriphy_start(vtss_isid_t isid, 
                                     port_veriphy_mode_t mode[VTSS_PORT_ARRAY_SIZE]);
#define port_mgmt_veriphy_start(...) vtss_port_mgmt_veriphy_start(__VA_ARGS__)

/* Get VeriPHY result */
vtss_rc vtss_port_mgmt_veriphy_get(vtss_isid_t isid, vtss_port_no_t port_no,
                              vtss_phy_veriphy_result_t *result, uint timeout);
#define port_mgmt_veriphy_get(...) vtss_port_mgmt_veriphy_get(__VA_ARGS__)

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
typedef struct {
    BOOL channel_status;
    BOOL xtend_reach;
} port_oobfc_conf_t;

/* Set host port OOBFC configuration */
vtss_rc port_mgmt_oobfc_conf_set(vtss_port_no_t port_no, const port_oobfc_conf_t *const conf);

/* Get host port OOBFC configuration */
vtss_rc port_mgmt_oobfc_conf_get(vtss_port_no_t port_no, port_oobfc_conf_t *const conf);

#endif

/* Initialize module */
vtss_rc port_init(vtss_init_data_t *data);

 // Bugzila#8911 - VeriPhy sometimes give wrong result, so we repeat VeriPhy VERIPHY_REPEAT_CNT times
#ifdef VTSS_SW_OPTION_POE 
#define VERIPHY_REPEAT_CNT 5
#else
#define VERIPHY_REPEAT_CNT 2
#endif
/**
 * \brief Callback type for i2c read function for doing SFP detect */
typedef vtss_rc (*vtss_i2c_callback_t)(u8 i2c_addr, vtss_port_no_t port_no, u8 addr, u8 *const data, u8 cnt);


/**
 * \brief Function that determines the SFP type by reading the SFP ROM via i2c  
 * \param port_no [IN]        The port number at which to detect SFP
 * \param ptr_to_i2c_rd_function [IN]   Pointer to the i2c read function to be used to read the SFP ROM
 * \return SFP type.
*/
vtss_rc sfp_detect(vtss_port_no_t port_no, port_sfp_t *sfp, BOOL *sgmii_cisco, BOOL *approved);


/**
 * \brief Function converting sfp_tranceiver_t to printable string
 * \param sfp [IN] SFP type to print
 * \return SFP type af printable string.
*/
char *sfp_if2txt(sfp_tranceiver_t sfp);

/**
 * \brief Function for getting port capabilities.
 * \param isid [IN] Switch id 
 * \param port_no [IN] Port id
 * \return port capabilities
*/
port_cap_t port_isid_port_cap(vtss_isid_t isid, vtss_port_no_t port_no);


/**
 * \brief Function that can check if at least one port in a stack contains a specific capability.
 * \param ap [IN] The capability to check for.
 * \return TRUE if at least one port support the specified capability, else FALSE
*/
BOOL port_stack_support_cap(u32 cap);

/**
 * \brief Function for converting return code to printable string
 * \param rc [IN] The return code
 * \return Printable string 
*/
char *port_error_txt(vtss_rc rc);

/**
 * \brief Function for resetting the PHY for specific port 
 * \param port_no [IN] Port in question
*/
vtss_rc do_phy_reset(vtss_port_no_t port_no);

#endif /* _VTSS_APPL_PORT_API_H_ */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
