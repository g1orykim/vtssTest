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

#ifndef _VTSS_APPL_H_
#define _VTSS_APPL_H_

#ifdef VTSS_ARCH_DAYTONA
#define DAYTONA_EVAL_INIT
#ifdef VTSS_SW_OPTION_REMOTE_TS_PHY 
#include "vtss_remote_ts_phy_api.h"
#endif /* VTSS_SW_OPTION_REMOTE_TS_PHY */
#endif /* VTSS_ARCH_DAYTONA */

#ifdef DAYTONA_EVAL_INIT
enum board_t {
    BOARD_INVALID,
    EVAL_BOARD_DAYTONA,
    EVAL_BOARD_TALLADEGA,
};

enum device_t {
    DEVICE_INVALID,
    DEVICE_DAYTONA,
    DEVICE_TALLADEGA,
};

enum interface_t { 
    IF_UPI0 = 0,
    IF_UPI1,
    IF_UPI2,
    IF_UPI3,
    IF_XAUI0,
    IF_XAUI1,
    IF_XAUI2,
    IF_XAUI3,
    IF_FPGA0,
    IF_FPGA1,
    IF_FPGA2,
    IF_FPGA3,
    IF_SMA0,
    IF_DELAY0,
    IF_DELAY1,
    IF_DELAY2,
    IF_DELAY3,
    IF_VIEW,
    IF_MAX,
};

enum protocol_t {
    PROTO_SONET,
    PROTO_10GEWAN,
    PROTO_OTU2,
    PROTO_OTU2E,
    PROTO_OTU1E,
    PROTO_OTU2F,
    PROTO_10GELAN,
    PROTO_10GFC,
    PROTO_8GFC,
    PROTO_250,
    PROTO_OTU2_SFI42,
    PROTO_OTU2E_SFI42,
    PROTO_OTU1E_SFI42,
    PROTO_OTU2F_SFI42,
    PROTO_XAUI_10GE,
    PROTO_XAUI_10GFC,
    PROTO_SONET_X4,
    PROTO_10GEWAN_X4,
    PROTO_OTU2_X4,
    PROTO_OTU2E_X4,
    PROTO_OTU1E_X4,
    PROTO_OTU2F_X4,
    PROTO_MAX,
};

enum block_t {
    BLOCK_SI5326_1,
    BLOCK_SI5326_2,
    BLOCK_SI5338_1,
    BLOCK_SI5338_2,
};

enum block_state_si5326_t{
    BS_64_66,
    BS_66_64,
    BS_15232_16830,
    BS_16830_15232,
    BS_16830_15168,
    BS_15168_16830,
    BS_237_255,
    BS_237_255_X4,
    BS_255_237,
    BS_255_237_X4,
    BS_238_255,
    BS_255_238,
    BS_X4,
    BS_X4_OTU2E,
    BS_CLEANUP,
    BS_BYP_X4_SONET,
    BS_BYP_X4_OTU2,
    BS_BYP_X4_OTU2E,
    BS_BYP_X4_TSOC1,
    BS_BYP_X4_TSOC2,
    BS_BYP_REF,
    BS_MAX,
};

#define SI5338_MAX_FIELDS         15
#define SI5326_MAX_FIELDS         27



#define HS_MODE_NONE                  0
#define MAX_CPU_CMD_LEN               100

#define BASE_ADDR_LBFPGA              0x80000000
#define BASE_ADDR_VSC3172             0x80030000
#define BASE_ADDR_STFPGA_0            0x80010000
#define BASE_ADDR_DUT                 0x80800000
#define BASE_ADDR_STFPGA_1            0x81000000

/* LBFPGA register */
#define LB_MOTOROLA_MODE              0x0C
#define LB_GPIN                       0x18
#define LB_I2C_CONTROLLER_RESET       0x10
#define LB_VSC3172_CTRL               0xa8
#define LB_VSC8492_RESET_CTRL         0x24

/* LBFPGA register */

/* pcf8575 register */
#define PCF8575_I2C_ADDR              0x23
#define PCF8575_PORT                  0x00
#define CLIENT_0_REFCLK_SEL_2         1 << 11
#define CLIENT_1_REFCLK_SEL_2         1 << 12
#define CLIENT_2_REFCLK_SEL_2         1 << 13
#define CLIENT_3_REFCLK_SEL_2         1 << 14
/* pcf8575 register */

/* Si5338 */
#define SI5338_POWERDOWN_0_ADDR       31
#define SI5338_POWERDOWN_1_ADDR       32
#define SI5338_POWERDOWN_2_ADDR       33
#define SI5338_POWERDOWN_3_ADDR       34
#define SI5338_TARGETID_START         27
#define SI5338_I2C_ADDR               0x70
#define SI5338_TGTID_CHAN_0_1         4
#define SI5338_TGTID_CHAN_2_3         5
#define SI5338_TGTID_MISC             6
#define SI5338_TGTID_VSC3406          7
/* Si5338 */

/* Si5326 */
#define SI5326_CLK_GEN_BASE           8

/* Si5326 */

/* VSC3172 */
#define VSC3172_REGISTER_INIT         0x28e
#define VSC3172_SWITCH_CONFIGURATION  0x281
vtss_rc lbfpga_read(u32 reg_addr, u32 *data);
vtss_rc lbfpga_write(u32 reg_addr, u32 lb_data, u32 mask);
vtss_rc pcf8575_read(u32 target_id, u16 *data);
vtss_rc pcf8575_write(u32 target_id, u16 lb_data, u16 mask);
vtss_rc vsc3172_read(u32 reg_addr, u16 *data);
vtss_rc vsc3172_write(u16 reg_addr, u16 lb_data, u16 mask);
vtss_rc si5338_read(u32 target_id, u16 reg_addr, u16 *data);
vtss_rc si5338_write(u32 target_id, u16 reg_addr, u16 lb_data, u16 mask);
vtss_rc si5326_read(u32 target_id, u16 reg_addr, u16 *data);
vtss_rc si5326_write(u32 target_id, u16 reg_addr, u16 lb_data, u16 mask);
vtss_rc daytona_eval_init(int inst,vtss_ev_conf_t ev_conf0, vtss_ev_conf_t conf1);
vtss_rc d_init_channel_clock_sw(int channel, vtss_config_mode_t mode, u32 xfi_clk, u32 x4_case);
vtss_rc d_init_board_conn_sw(int chan, int dutif, int boardif, int client_proto, int swith_loop);
#endif /* DAYTONA_EVAL_INIT */


/* Application instance number */
typedef int vtss_appl_inst_t;

/* Port status */
typedef struct {
    BOOL              link;       /* Link status, remaining fields only valid if link up */
    vtss_port_speed_t speed;      /* Speed */
    BOOL              fdx;        /* Full duplex */
    
    /* Auto negotiation result */
    struct {
        BOOL obey_pause;     /* This port should obey PAUSE frames */
        BOOL generate_pause; /* Link partner obeys PAUSE frames */
    } aneg;

} vtss_appl_port_status_t;

/* Get port status */
vtss_rc vtss_appl_port_status_get(const vtss_appl_inst_t inst,
                                  const vtss_port_no_t    port_no,
                                  vtss_appl_port_status_t *const status);

/* Port configuration */
typedef struct {
    BOOL              enable;       /* Admin enable/disable */
    BOOL              autoneg;      /* Auto negotiation */
    vtss_port_speed_t speed;        /* Forced port speed */
    BOOL              fdx;          /* Forced duplex mode */
    BOOL              flow_control; /* Flow control */
    u32               max_length;   /* Max frame length */
#if defined(VTSS_ARCH_B2)
  vtss_port_ct_t ct;          /* Cut-through settings */
  BOOL           pfci_enable; /* External flow control */ 
#endif /* VTSS_ARCH_B2 */
} vtss_appl_port_conf_t;

/* Get port configuration */
vtss_rc vtss_appl_port_conf_get(const vtss_appl_inst_t inst, 
                                const vtss_port_no_t  port_no,
                                vtss_appl_port_conf_t *const conf);

/* Set port configuration */
vtss_rc vtss_appl_port_conf_set(const vtss_appl_inst_t inst,
                                const vtss_port_no_t        port_no,
                                const vtss_appl_port_conf_t *const conf);

/* Get port interface type */
vtss_rc vtss_appl_port_if_get(const vtss_appl_inst_t inst,
                              const vtss_port_no_t  port_no,
                              vtss_port_interface_t *const if_type);

/* Get init configuration */
vtss_rc vtss_appl_init_conf_get(const vtss_appl_inst_t inst,
                                vtss_init_conf_t * const conf);

#if defined(VTSS_ARCH_B2)
/* Get host configuration */
vtss_rc vtss_appl_host_conf_get(const vtss_appl_inst_t inst,
                                const vtss_port_no_t port_no, 
                                vtss_host_conf_t * const conf);
#endif /* VTSS_ARCH_B2 */
/* Is the port host port */
BOOL vtss_appl_port_is_host(const vtss_appl_inst_t inst,
                            const vtss_port_no_t port_no);

/* Is the interface type SPI4 */
BOOL vtss_appl_iftype_is_spi4(const vtss_appl_inst_t inst,
                              const vtss_port_no_t port_no);

/* Is the interface type XAUI */
BOOL vtss_appl_iftype_is_xaui(const vtss_appl_inst_t inst,
                              const vtss_port_no_t port_no);

#ifdef VTSS_OPT_TRACE

extern vtss_trace_conf_t vtss_appl_trace_conf;

/* Application trace group */
#define VTSS_APPL_TRACE_GROUP VTSS_TRACE_GROUP_COUNT
#define VTSS_APPL_TRACE_LAYER VTSS_TRACE_LAYER_COUNT

/* Application trace macros */
#define T_E(...) { if (vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL] >= VTSS_TRACE_LEVEL_ERROR) vtss_callout_trace_printf(VTSS_APPL_TRACE_LAYER, VTSS_APPL_TRACE_GROUP, VTSS_TRACE_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }
#define T_I(...) { if (vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL] >= VTSS_TRACE_LEVEL_INFO) vtss_callout_trace_printf(VTSS_APPL_TRACE_LAYER, VTSS_APPL_TRACE_GROUP, VTSS_TRACE_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }
#define T_D(...) { if (vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL] >= VTSS_TRACE_LEVEL_DEBUG) vtss_callout_trace_printf(VTSS_APPL_TRACE_LAYER, VTSS_APPL_TRACE_GROUP, VTSS_TRACE_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }
#define T_N(...) { if (vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL] >= VTSS_TRACE_LEVEL_NOISE) vtss_callout_trace_printf(VTSS_APPL_TRACE_LAYER, VTSS_APPL_TRACE_GROUP, VTSS_TRACE_LEVEL_NOISE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }
#else

/* No trace */
#define T_E(...)
#define T_I(...)
#define T_D(...)
#define T_N(...)

#endif
typedef struct {
    vtss_init_conf_t *init_conf; /* Init configuration */
#if defined(VTSS_ARCH_B2)
    vtss_spi4_conf_t *spi4;      /* SPI-4.2 host setup */
    vtss_xaui_conf_t *xaui;      /* XAUI host setup */
#endif /* VTSS_ARCH_B2 */
} vtss_init_t;

#if defined(VTSS_CHIP_ALL) || defined(BOARD_TALLADEGA)
#define VTSS_APPL_INST_CNT 2
#else
#define VTSS_APPL_INST_CNT 1
#endif /* VTSS_CHIP_ALL */

/**< \brief Description: Board structure filled out by vtss_board_init(). */
typedef struct vtss_board_t {
    const char         *descr;     /* Target description */
    vtss_target_type_t target;     /* Target type */
    vtss_inst_t        inst;       /* API instance reference */
    u32                port_count; /* Number of ports */
    BOOL               b2;         /* Barrington-II family */
    BOOL               daytona;    /* Daytona family */

    /* Feature flags */
    struct {
        BOOL port_control;
        BOOL layer2;
        BOOL packet;
    } feature;

    vtss_init_t init; /* Init configuration */

    /* Port status */
    vtss_appl_port_status_t port_status[VTSS_PORT_ARRAY_SIZE];

    /* Port configuration */
    vtss_appl_port_conf_t   port_conf[VTSS_PORT_ARRAY_SIZE];

#if defined(VTSS_FEATURE_PORT_CONTROL)
    vtss_port_map_t *port_map;   /* Board port map */
#endif /* VTSS_FEATURE_PORT_CONTROL */

    /* Port MAC interface */
    vtss_port_interface_t (* port_interface)(vtss_port_no_t port_no);

    /* Post ports reset */
    vtss_rc (*post_reset)(void);

    /* Called when system is reset. */
    void (*pre_reset)(void);

    /* Optional, avoid polling for some ports */
    BOOL (* port_poll)(vtss_port_no_t port_no);

    /* Parse options, initialize I/O, release main chip from reset */
    int  (* board_init)(int argc, const char **argv, struct vtss_board_t *board);

    /* Optional, release PHYs from reset */
    void (* board_init_post)(struct vtss_board_t *board);

    /* Optional, release PHYs from coma */
    void (* board_init_done)(struct vtss_board_t *board);

    /* Optional, IRQ poll */
    void (* board_irq_poll)(struct vtss_board_t *board);
#ifndef VTSS_ARCH_DAYTONA
#if defined(VTSS_FEATURE_PACKET)
    /* Optional, Transmit */
    vtss_rc (* board_transmit)(struct vtss_board_t        *board, 
                               const vtss_packet_tx_ifh_t *const ifh,
                               const u8                   *const frame,
                               const u32                   length);
#endif
#endif
} vtss_board_t;

/* Get API instance */
vtss_board_t *vtss_appl_board_get(vtss_appl_inst_t inst);

void vtss_board_b2_init(vtss_board_t *board);
void vtss_board_estax34_init(vtss_board_t *board);
void vtss_board_daytona_init(vtss_board_t *board);
void vtss_board_l26_eval_init(vtss_board_t *board);
void vtss_board_serval_eval_init(vtss_board_t *board);
void vtss_board_serval_ref_init(vtss_board_t *board);
void vtss_board_jr1_eval_init(vtss_board_t *board);
void vtss_board_jr2_ref_init(vtss_board_t *board);
void vtss_board_daytona_interrupt(void);
void vtss_board_daytona_event_screen(BOOL enable);
void vtss_board_daytona_interrupt_without_mask(vtss_port_no_t port);
#ifdef VTSS_ARCH_DAYTONA
#ifdef VTSS_SW_OPTION_REMOTE_TS_PHY 
vtss_rc vtss_daytona_remote_phy_ts_rx_parse(remote_ts_msg_header_t *msg_hdr, u8 *data);
#endif /* VTSS_SW_OPTION_REMOTE_TS_PHY */
#endif /* VTSS_ARCH_DAYTONA */
#if defined(BOARD_B2_EVAL)
vtss_rc vtss_rabbit_rd_wr_fpga(u32 addr, u32 *value, BOOL read);
#endif /* BOARD_B2_EVAL */

/* ================================================================= *
 *  Conversion between internal and user port numbers.
 *  These functions are intended to be used only in places that
 *  interact with the user, i.e. Web, CLI, SNMP etc..
 * ================================================================= */
typedef vtss_port_no_t vtss_uport_no_t; /* User port is of same type as normal, internal port. */
vtss_uport_no_t iport2uport(vtss_port_no_t iport); // Convert from internal to user port number.
vtss_port_no_t  uport2iport(vtss_uport_no_t uport); // Convert from user to internal port number.

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#endif /* _VTSS_APPL_H_ */
