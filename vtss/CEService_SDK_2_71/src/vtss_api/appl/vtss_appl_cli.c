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

#include "vtss_api.h"

#if defined(VTSS_OPSYS_LINUX)
#include <termios.h>
#include <unistd.h>

#undef __USE_EXTERN_INLINES /* Avoid warning */
#include <sys/socket.h>

#ifndef VTSS_ARCH_DAYTONA
#include <arpa/inet.h>
#endif

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#endif /* VTSS_OPSYS_LINUX */

#if defined(VTSS_OPSYS_ECOS)
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <cyg/kernel/kapi.h>

static cyg_io_handle_t ser_handle;

#endif /* VTSS_OPSYS_ECOS */

#include "vtss_version.h"
#include "vtss_appl.h"
#include "vtss_appl_cli.h"

#if defined(VTSS_FEATURE_OTN)
#include "vtss_otn_api.h"
#endif
#if defined(VTSS_FEATURE_GFP)
#include "vtss_gfp_api.h"
#endif
#if defined(VTSS_FEATURE_WIS)
#include "vtss_wis_api.h"
#endif
#if defined(VTSS_FEATURE_UPI)
#include "vtss_upi_api.h"
#endif
#if defined(VTSS_FEATURE_XAUI)
#include "vtss_xaui_api.h"
#endif
#if defined(VTSS_FEATURE_XFI)
#include "vtss_xfi_api.h"
#endif
#if defined(VTSS_FEATURE_MAC10G)
#include "vtss_mac10g_api.h"
#endif
#if defined(VTSS_FEATURE_PCS_10GBASE_R)
#include "vtss_pcs_10gbase_r_api.h"
#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#if defined(VTSS_FEATURE_RAB)
#include "vtss_rab_api.h"
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_I2C)
#include "vtss_i2c_api.h"
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_FEATURE_OHA)
#include "vtss_oha_api.h"
#endif

typedef unsigned char uchar;

#define MAX_CMD_LEN      (500 + 1)
#define MAX_CMD_HISTORY_LEN 20
#define MAX_WORD_LEN        64

#define CURSOR_UP    0x41
#define CURSOR_DOWN  0x42
#define CURSOR_RIGHT 0x43
#define CURSOR_LEFT  0x44

#define CURSOR_HOME  0x48
#define CURSOR_END   0x4B

#define BS  0x08
#define BEL 0x07
#define LF  0x0a
#define CR  0x0d
#define ESC 0x1b

#define CLI_TST_RC(expr) { vtss_rc rc = (expr); if (rc < VTSS_RC_OK) printf("Return code: %d\n", rc); }


static char cmd_buf[MAX_CMD_LEN];
static u32  cmd_len = 0;
static u32  cmd_cursor = 0;

static BOOL suspended = 0;

struct {
    u32 idx;
    u32 len;
    u32 scroll;
    struct {
        u32  cmd_len;
        char cmd[MAX_CMD_LEN];
    } buf[MAX_CMD_HISTORY_LEN];
} cmd_history;

static vtss_appl_inst_t cli_appl_inst;
static vtss_board_t     *cli_board;
#ifdef VTSS_FEATURE_PHY_TIMESTAMP

#define API_INST_DEFAULT NULL
struct {
    BOOL  init;
    u8    encap_type;
    u8    flow_match_mode;
    u16   flow_st_index;
    u16   flow_end_index;
} engine_init_info[VTSS_PORTS][2][4]; /* 2->ingress/egress, 4->num_engine */
typedef enum {
    TXT2IP6_ADDR_TYPE_INVALID = 0,
    TXT2IP6_ADDR_TYPE_MCAST,
    TXT2IP6_ADDR_TYPE_UCAST,
    TXT2IP6_ADDR_TYPE_LINK_LOCAL,
    TXT2IP6_ADDR_TYPE_UNSPECIFIED,
    TXT2IP6_ADDR_TYPE_LOOPBACK,
    TXT2IP6_ADDR_TYPE_IPV4_MAPPED,
    TXT2IP6_ADDR_TYPE_IPV4_COMPAT,
    TXT2IP6_ADDR_TYPE_GEN
} txt2ip6_addr_type_t;

#define IPV6_ADDR_STR_LEN_LONG  0
#if IPV6_ADDR_STR_LEN_LONG
#define IPV6_ADDR_STR_MAX_LEN   254
#else
#define IPV6_ADDR_STR_MAX_LEN   45
#endif /* IPV6_ADDR_STR_LEN_LONG */
#define IPV6_ADDR_STR_MIN_LEN   2

#define IPV6_ADDR_IBUF_MAX_LEN  (IPV6_ADDR_STR_MAX_LEN + 1)
#define IPV6_ADDR_IBUF_MIN_LEN  (IPV6_ADDR_STR_MIN_LEN + 1)
#define IPV6_ADDR_OBUF_MAX_LEN  40  /* 39 + 1 */
#define IPV6_ADDR_OBUF_MIN_LEN  (IPV6_ADDR_STR_MIN_LEN + 1)
/* These APIs are defined to debug PHY Timestamp features, not intend
   to expose to API customers and hence move to API base folder;
   extern here to access thru Debug CLI */
extern vtss_rc vtss_phy_1588_csr_reg_write(const vtss_inst_t inst,
                                           const vtss_port_no_t port_no,
                                           const u32 blk_id,
                                           const u16 csr_address,
                                           const u32 *const value);
extern vtss_rc vtss_phy_1588_csr_reg_read(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no,
                                          const u32 blk_id,
                                          const u16 csr_address,
                                          u32 *const value);


#endif // VTSS_FEATURE_PHY_TIMESTAMP

static int cli_char_available(unsigned char *c)
{
#if defined(VTSS_OPSYS_ECOS)
    Cyg_ErrNo err;    
    cyg_uint32 len = 1;
    err = cyg_io_read(ser_handle, c, &len);
    if((ENOERR == err) && (len > 0)) {
        return 1;
    }
    return 0;
#else
    fd_set fdset;
    struct timeval tv;

    FD_ZERO(&fdset);
    FD_SET(0, &fdset);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    if(select(1, &fdset, NULL, NULL, &tv) > 0) {
        *c = getchar();        
        return 1;
    }
    return 0;
#endif
}

static void prompt(void)
{
    printf("%s>", cli_board->descr);
    fflush(stdout);
}

static void cursor_left(void)
{
    if (cmd_cursor > 0) {
        cmd_cursor--;
        putchar(ESC);
        putchar(0x5b);
        putchar(CURSOR_LEFT);
        fflush(stdout);
    }
}

static void cursor_right(void)
{
    if (cmd_cursor < cmd_len) {
        cmd_cursor++;
        putchar(ESC);
        putchar(0x5b);
        putchar(CURSOR_RIGHT);
        fflush(stdout);
    }
}

static void cursor_home(void)
{
    while (cmd_cursor > 0) {
        cursor_left();
    }
}

static void delete_to_eol(void)
{
    putchar(ESC);
    putchar(0x5b);
    putchar(CURSOR_END);
    fflush(stdout);
}

static void rewrite_to_eol(void)
{
    while (cmd_cursor < cmd_len) {
        putchar(cmd_buf[cmd_cursor++]);
    }
    fflush(stdout);
}

static void delete_line(void)
{
    cursor_home();
    delete_to_eol();
}

static void append_line(void)
{
    u32 cursor_save;

    cursor_save = cmd_cursor;
    rewrite_to_eol();
    while (cmd_cursor > cursor_save) {
        cursor_left();
    }
}

static void delete_char(int backspace)
{
    u32 j;

    if (cmd_len == 0 || 
        (backspace && cmd_cursor == 0) || 
        (!backspace && cmd_cursor == cmd_len))
        return;
    
    if (backspace)
        cursor_left();
    delete_to_eol();

    /* concatenate command string */
    cmd_len--;
    for (j = cmd_cursor; j < cmd_len; j++) {
        cmd_buf[j] = cmd_buf[j + 1];
    }

    /* rewrite command part to the right of cursor */
    append_line();
}

static void insert_char(char ch)
{
    u32 j;

    delete_to_eol();
    for (j = cmd_len; j > cmd_cursor; j--) {
        cmd_buf[j] = cmd_buf[j - 1];
    }
    cmd_len++;
    cmd_buf[cmd_cursor++] = ch;

    append_line();
}

static void cmd_history_put(void)
{
    memcpy(&cmd_history.buf[cmd_history.idx].cmd, &cmd_buf, cmd_len);
    cmd_history.buf[cmd_history.idx].cmd_len = cmd_len - 1; /* don't include CR */
    if (cmd_history.len < MAX_CMD_HISTORY_LEN) {
        cmd_history.len++;
    }
    if (++cmd_history.idx >= MAX_CMD_HISTORY_LEN) {
        cmd_history.idx = 0;
    }
    cmd_history.scroll = 0;
}

static void cmd_history_get(void)
{
    u32 idx;

    if (cmd_history.idx >= cmd_history.scroll) {
        idx = cmd_history.idx - cmd_history.scroll;
    }
    else {
        idx = MAX_CMD_HISTORY_LEN - (cmd_history.scroll - cmd_history.idx);
    }

    cmd_len = cmd_history.buf[idx].cmd_len;
    memcpy(&cmd_buf, &cmd_history.buf[idx].cmd, cmd_len);

}

static void get_old_cmd(void)
{
    delete_line();
    cmd_history_get();
    rewrite_to_eol();
}

static void process_escape_seq(char ch_1, char ch_2)
{
    if (ch_1 == 0x5b) {
        switch (ch_2) {
        case CURSOR_UP:
            if (cmd_history.scroll < cmd_history.len) {
                cmd_history.scroll++;
                get_old_cmd();
            }
            else {
                putchar(BEL);
                fflush(stdout);
            }
            break;

        case CURSOR_DOWN:
            if (cmd_history.scroll > 0) {
                cmd_history.scroll--;

                if (cmd_history.scroll > 0) {
                    get_old_cmd();
                }
                else {
                    delete_line();
                    cmd_len = 0;
                    cmd_cursor = 0;
                }
            }
            else {
                putchar(BEL);
                fflush(stdout);
            }
            break;

        case CURSOR_RIGHT:
            cursor_right();
            break;

        case CURSOR_LEFT:
            cursor_left();
            break;

        case CURSOR_HOME:
            cursor_home();
            break;

        case CURSOR_END:
            while (cmd_cursor < cmd_len) {
                cursor_right();
            }
            break;

        default:
            break;
        }
    }
}

static int empty_cmd_line(void)
{
    u32 j;

    for (j = 0; j < cmd_len; j++) {
        if ((cmd_buf[j] != ' ') && (cmd_buf[j] != CR)) {
            return 0;
        }
    }
    return 1;
}

static int cmd_ready(void)
{
    unsigned char ch;
    u32 loop_count;
#if defined(VTSS_OPSYS_ECOS)
    static u32 escape_seq_flag = 0;
    static u32 escape_seq_count = 0;
    static char escape_seq [2];
#endif

    loop_count = 0;
    while (cli_char_available(&ch) && (loop_count++ < 20)) {

#if !defined(VTSS_OPSYS_ECOS)
/*         printf("CHAR_1: %02X\n", ch);  */
        /* convert LF into CR for linux terminal usage */
        if(ch==LF) ch=CR;
#endif
        if (ch != LF) { /* discard LF chars */
            
            if (ch == BS) /* Backspace */
                delete_char(1);
            else if (ch == 0x7f) /* DEL */
                delete_char(1); 
#if defined(VTSS_OPSYS_ECOS)
            else if (ch == ESC) {
                escape_seq_flag = 1;
                escape_seq_count = 0;
            }
            else if (escape_seq_flag) {
                escape_seq[escape_seq_count++] = ch;
                if (escape_seq_count >= 2) {
                    process_escape_seq(escape_seq[0], escape_seq[1]);
                    escape_seq_flag = 0;
                }
            }
#else
            else if (ch == ESC) {                
                ch = getc(stdin);
                process_escape_seq(ch, getc(stdin));
            }
#endif

            else {
                
                if (cmd_len < MAX_CMD_LEN) {
                    /* echo */
                    putchar(ch);
                    fflush(stdout);
                }
                
                if (ch != CR) {
                    if (cmd_len < MAX_CMD_LEN) {
                        if (cmd_cursor < cmd_len) {
                            insert_char(ch);
                        }
                        else {
                            cmd_buf[cmd_cursor++] = ch;
                            if (cmd_len < cmd_cursor) {
                                cmd_len++;
                            }
                        }
                    }
                }
                else {
                    putchar(LF);
                    fflush(stdout);

                    /* Ensure that CR is present in buffer in case of buffer overflow */
                    if (cmd_len == MAX_CMD_LEN) {
                        cmd_buf[MAX_CMD_LEN - 1] = CR;
                    }
                    else {
                        cmd_buf[cmd_len++] = CR;
                    }
                    if (!empty_cmd_line()) {
                        cmd_history_put();
                    }
                    cmd_buf[cmd_len++] = '\0';                    

                    fflush(stdout);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static char *cmd_get(void)
{
    cmd_len = 0;
    cmd_cursor = 0;

    return &cmd_buf[0];
}

#if defined(VTSS_OPSYS_LINUX)
static struct termios stored_settings;

static void restore_keypress(void)
{
    tcsetattr(0, TCSANOW, &stored_settings);
}
#endif /* VTSS_OPSYS_LINUX */

static void cli_inst_update(vtss_appl_inst_t inst)
{
    cli_appl_inst = inst;
    cli_board = vtss_appl_board_get(inst);
}

void vtss_appl_cli_init(void)
{
#if defined(VTSS_OPSYS_LINUX)
    struct termios new_settings;

    tcgetattr(0,&stored_settings);

    new_settings = stored_settings;

    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;

    tcsetattr(0,TCSANOW,&new_settings);
    
    atexit(restore_keypress);
#endif /* VTSS_OPSYS_LINUX */

#if defined(VTSS_OPSYS_ECOS)
    cyg_uint32 val = 0, len = sizeof(val);

    if (cyg_io_lookup("/dev/ser1", &ser_handle) != ENOERR) {
        T_E("/dev/ser1 lookup failed");
    } else if (cyg_io_set_config(ser_handle, 
                                 CYG_IO_SET_CONFIG_READ_BLOCKING, &val, &len) != ENOERR) {
        T_E("setting nonblocking read failed");
    }
#endif /* VTSS_OPSYS_ECOS */

    cmd_history.len = 0;
    cmd_history.idx = 0;
    cmd_history.scroll = 0;

    cli_inst_update(0);

    printf("\n\nWelcome to the Vitesse Command Line Interface.\n");
    printf("Press <enter> to get the list of commands.\n");
    prompt();
}

/****************************************************************************/
/*  Command parsing                                                         */
/****************************************************************************/

/* CLI command ID */
typedef enum {
    CLI_CMD_ID_NONE,
    CLI_CMD_ID_HELP,
    CLI_CMD_ID_SYSTEM_CONF,
    CLI_CMD_ID_SYSTEM_INST,
    CLI_CMD_ID_SYSTEM_INIT,
    CLI_CMD_ID_SYSTEM_RESET,
    CLI_CMD_ID_SYSTEM_RESTART,
    CLI_CMD_ID_PORT_CONF,
    CLI_CMD_ID_PORT_MODE,
    CLI_CMD_ID_PORT_FLOW_CONTROL,
    CLI_CMD_ID_PORT_STATE,
    CLI_CMD_ID_PORT_MAX_FRAME,
    CLI_CMD_ID_PORT_STATS,
    CLI_CMD_ID_PORT_MAP,
    CLI_CMD_ID_PORT_PSI,
    CLI_CMD_ID_PORT_CT,
    CLI_CMD_ID_PORT_PFCI,
    CLI_CMD_ID_PORT_FORWARD,
    CLI_CMD_ID_PORT_FILTER,
    CLI_CMD_ID_PORT_TRANSMIT,
    CLI_CMD_ID_QOS_PORT_POLICE,
    CLI_CMD_ID_QOS_PORT_SHAPER,
    CLI_CMD_ID_QOS_LPORT_SHAPER,
    CLI_CMD_ID_QOS_QUEUE_POLICE,
    CLI_CMD_ID_QOS_GLOBAL_DSCP,
    CLI_CMD_ID_QOS_GLOBAL_TCPUDP,
    CLI_CMD_ID_QOS_GLOBAL_TCPUDP_ENABLE,
    CLI_CMD_ID_QOS_GLOBAL_FILTTRIG,
    CLI_CMD_ID_QOS_GLOBAL_FILTACTION,
    CLI_CMD_ID_QOS_GLOBAL_FILTENABLE,
    CLI_CMD_ID_QOS_FILTTRIG,
    CLI_CMD_ID_QOS_FILTACTION,
    CLI_CMD_ID_QOS_PRIORDER,
    CLI_CMD_ID_QOS_CLASS_MISC,
    CLI_CMD_ID_QOS_CLASS_TCPUDP,
    CLI_CMD_ID_QOS_CLASS_IP_PROTO,
    CLI_CMD_ID_QOS_CLASS_ETYPE,
    CLI_CMD_ID_QOS_CLASS_VID,
    CLI_CMD_ID_QOS_CLASS_TAG_PRIO,
    CLI_CMD_ID_QOS_CLASS_MPLS,
    CLI_CMD_ID_QOS_DSCP_TABLE,
    CLI_CMD_ID_QOS_TAG,
    CLI_CMD_ID_QOS_L2_FILTER_ALTORDER,
    CLI_CMD_ID_QOS_L2_FILTER_DMAC,
    CLI_CMD_ID_QOS_L2_FILTER_VID,
    CLI_CMD_ID_QOS_L2_FILTER_ETYPE,
    CLI_CMD_ID_QOS_L2_FILTER_PORT,
    CLI_CMD_ID_QOS_L2_FILTER_CONF,
    CLI_CMD_ID_QOS_RED_THRESHOLD,
    CLI_CMD_ID_QOS_RED_PROBABILITY,
    CLI_CMD_ID_QOS_RED_PROFILE,
    CLI_CMD_ID_QOS_PORT_SCHEDULER,
    CLI_CMD_ID_QOS_LPORT_SCHEDULER,
    CLI_CMD_ID_QOS_QUEUE_SCHEDULER,
    CLI_CMD_ID_MAC_ADD,
    CLI_CMD_ID_MAC_DEL,
    CLI_CMD_ID_MAC_LOOKUP,
    CLI_CMD_ID_MAC_DUMP,
    CLI_CMD_ID_MAC_FLUSH,
    CLI_CMD_ID_MAC_AGE_TIME,
    CLI_CMD_ID_MAP_PORT,
    CLI_CMD_ID_MAP_LPORT,
    CLI_CMD_ID_PHY_10G_MODE,
    CLI_CMD_ID_PHY_10G_STATUS,
    CLI_CMD_ID_PHY_10G_RESET,
    CLI_CMD_ID_PHY_10G_LOOPBACK,
    CLI_CMD_ID_PHY_10G_STATS,
    CLI_CMD_ID_PHY_10G_POWER,
    CLI_CMD_ID_DEBUG_API,
    CLI_CMD_ID_DEBUG_REG_READ_L28,
    CLI_CMD_ID_DEBUG_REG_WRITE_L28,
    CLI_CMD_ID_DEBUG_REG_READ,
    CLI_CMD_ID_DEBUG_REG_WRITE,
    CLI_CMD_ID_DEBUG_PHY_READ,
    CLI_CMD_ID_DEBUG_PHY_WRITE,
    CLI_CMD_ID_DEBUG_TARGETS,
    CLI_CMD_ID_DEBUG_MMD_READ,
    CLI_CMD_ID_DEBUG_MMD_WRITE,
    CLI_CMD_ID_DEBUG_SUSPEND,
    CLI_CMD_ID_DEBUG_CHIP_ID,
    CLI_CMD_ID_DEBUG_RESUME,
    CLI_CMD_ID_DEBUG_TRACE,
    CLI_CMD_ID_DEBUG_FPGA_READ,
    CLI_CMD_ID_DEBUG_FPGA_WRITE,
    CLI_CMD_ID_DEBUG_FPGA_STATUS,
    CLI_CMD_ID_CLK_GPIO,
    CLI_CMD_ID_DEBUG_GPIO_OUTPUT,
    CLI_CMD_ID_DEBUG_GPIO_READ,
    CLI_CMD_ID_DEBUG_GPIO_WRITE,
    CLI_CMD_ID_DEBUG_SPI4,
    CLI_CMD_ID_DEBUG_REG_CHECK,
    CLI_CMD_ID_DEBUG_QSS,
    CLI_CMD_ID_DEBUG_BUF,
    CLI_CMD_ID_DEBUG_RED_CNT,
    CLI_CMD_ID_DEBUG_REG_READ_DAYTONA,
    CLI_CMD_ID_DEBUG_REG_WRITE_DAYTONA,
    CLI_CMD_ID_DEBUG_EVENT_CONFIG,
    CLI_CMD_ID_DEBUG_EVENT_SCREEN,
    CLI_CMD_ID_DEBUG_EVENT_CLEAR,
    CLI_CMD_ID_DEBUG_INTERRUPT_WITHOUT_MASK,
    CLI_CMD_ID_PMTICK_CONFIG,
#if defined(VTSS_FEATURE_WIS)    
    CLI_CMD_ID_WIS_MODE,
    CLI_CMD_ID_WIS_SECTION_ATTI_GET,
    CLI_CMD_ID_WIS_PATH_ATTI_GET,
    CLI_CMD_ID_WIS_SECTION_TXTTI,
    CLI_CMD_ID_WIS_PATH_TXTTI,
    CLI_CMD_ID_WIS_PATH_TXSL,
    CLI_CMD_ID_WIS_STATUS,
    CLI_CMD_ID_WIS_CONS_ACT,
    CLI_CMD_ID_WIS_COUNTERS,
    CLI_CMD_ID_WIS_DEFECTS,    
    CLI_CMD_ID_WIS_OH_CONF,
    CLI_CMD_ID_WIS_PREF_COUNTERS,    
    CLI_CMD_ID_WIS_PASSTHRU,
    CLI_CMD_ID_WIS_FORCED_INSERTION,
    CLI_CMD_ID_WIS_PERF_MODE,
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
    CLI_CMD_ID_WIS_RESET,
    CLI_CMD_ID_WIS_PRBS_ERR_INJ,
    CLI_CMD_ID_WIS_TEST_MODE,
    CLI_CMD_ID_WIS_TEST_STATUS,
    CLI_CMD_ID_WIS_TX_PREF_THRE,
    CLI_CMD_ID_WIS_FORCE_CONF,
    CLI_CMD_ID_WIS_EVENT_FORCE,
#endif
#endif
#if defined VTSS_FEATURE_PHY_TIMESTAMP
    CLI_CMD_ID_1588_BLOCK_INIT,
    CLI_CMD_ID_1588_ENGINE_INIT,
    CLI_CMD_ID_1588_ENGINE_CLEAR,
    CLI_CMD_ID_1588_ENGINE_MODE,
    CLI_CMD_ID_1588_ENGINE_CHANNEL_MAP,
    CLI_CMD_ID_1588_ENGINE_ETH1_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_ETH1_FLOW_CONF,
    CLI_CMD_ID_1588_ENGINE_ETH2_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_ETH2_FLOW_CONF,
    CLI_CMD_ID_1588_ENGINE_IP1_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_IP1_FLOW_CONF,
    CLI_CMD_ID_1588_ENGINE_IP2_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_IP2_FLOW_CONF,
    CLI_CMD_ID_1588_ENGINE_MPLS_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_MPLS_FLOW_CONF,
    CLI_CMD_ID_1588_ENGINE_ACH_COMM_CONF,
    CLI_CMD_ID_1588_ENGINE_ACTION_ADD,
    CLI_CMD_ID_1588_ENGINE_ACTION_DELETE,
    CLI_CMD_ID_1588_ENGINE_ACTION_SHOW,
    CLI_CMD_ID_1588_SIGNATURE_CONF,
    CLI_CMD_ID_1588_STATS_SHOW,
    CLI_CMD_ID_1588_LATENCY,
    CLI_CMD_ID_1588_DELAY,
    CLI_CMD_ID_1588_DELAY_ASYM,
    CLI_CMD_ID_1588_MMD_READ,
    CLI_CMD_ID_1588_MMD_WRITE,
#endif/* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_FEATURE_OTN)
    CLI_CMD_ID_OTN_OTU_ACC_RES,
    CLI_CMD_ID_OTN_OTU_ACC_SMRES,
    CLI_CMD_ID_OTN_OTU_ACC_GCC0,
    CLI_CMD_ID_OTN_OTU_TX_RES,
    CLI_CMD_ID_OTN_OTU_TX_SMRES,
    CLI_CMD_ID_OTN_OTU_TX_GCC0,
    CLI_CMD_ID_OTN_ODU_OH_INSERTION,
    CLI_CMD_ID_OTN_OTU_OH_INSERTION,
    CLI_CMD_ID_OTN_ODU_TX_RES,
    CLI_CMD_ID_OTN_ODU_ACC_RES,
    CLI_CMD_ID_OTN_ODU_TX_EXP,
    CLI_CMD_ID_OTN_ODU_ACC_EXP,
    CLI_CMD_ID_OTN_ODU_TX_FTFL,
    CLI_CMD_ID_OTN_ODU_ACC_FTFL,
    CLI_CMD_ID_OTN_OPU_OH_INSERTION,
    CLI_CMD_ID_OTN_OPU_TEST_INSERTION,
    CLI_CMD_ID_OTN_OCH_PERF,
    CLI_CMD_ID_OTN_OCH_LOOPBACK,
    CLI_CMD_ID_OTN_OCH_FEC,
    CLI_CMD_ID_OTN_OCH_BER_THRESHOLD,
    CLI_CMD_ID_OTN_OCH_DEFECTS,
    CLI_CMD_ID_OTN_OTU_TTI,
    CLI_CMD_ID_OTN_OTU_ACCTTI,
    CLI_CMD_ID_OTN_OTU_CONSACTS,
    CLI_CMD_ID_OTN_OTU_CONSACTS_IAE,
    CLI_CMD_ID_OTN_OTU_DEFECTS,
    CLI_CMD_ID_OTN_OTU_PERF,
    CLI_CMD_ID_OTN_ODU_MODE,
    CLI_CMD_ID_OTN_ODU_TTI,
    CLI_CMD_ID_OTN_ODU_ACCTTI,
    CLI_CMD_ID_OTN_ODU_AIS,
    CLI_CMD_ID_OTN_ODU_CONSACTS_BEI,
    CLI_CMD_ID_OTN_ODU_CONSACTS_BDI,
    CLI_CMD_ID_OTN_ODU_CONSACTS_AIS,
    CLI_CMD_ID_OTN_ODU_PAYLOAD_TYPE,
    CLI_CMD_ID_OTN_ODU_ACC_PAYLOAD_TYPE,
    CLI_CMD_ID_OTN_ODU_APS,
    CLI_CMD_ID_OTN_ODU_ACCAPS,
    CLI_CMD_ID_OTN_ODU_DEFECTS,
    CLI_CMD_ID_OTN_ODU_PERF,
    CLI_CMD_ID_OTN_ODUT_MODE,
    CLI_CMD_ID_OTN_ODUT_TTI,
    CLI_CMD_ID_OTN_ODUT_ACCTTI,
    CLI_CMD_ID_OTN_ODUT_CONSACTS_BEI,
    CLI_CMD_ID_OTN_ODUT_CONSACTS_BDI,
    CLI_CMD_ID_OTN_ODUT_CONSACTS_AIS,
    CLI_CMD_ID_OTN_ODUT_CONSACTS_IAE,
    CLI_CMD_ID_OTN_ODUT_BDI,
    CLI_CMD_ID_OTN_ODUT_STAT,
    CLI_CMD_ID_OTN_ODUT_ACCSTAT,
    CLI_CMD_ID_OTN_ODUT_APS,
    CLI_CMD_ID_OTN_ODUT_ACCAPS,
    CLI_CMD_ID_OTN_ODUT_DEFECTS,
    CLI_CMD_ID_OTN_ODUT_PERF,
#endif    
#if defined(VTSS_FEATURE_GFP)
    CLI_CMD_ID_GFP_SINGLE_ERR_CORR,
    CLI_CMD_ID_GFP_CMF_AUTO_SF_GEN,
    CLI_CMD_ID_GFP_CMF_FORCED_GEN,
    CLI_CMD_ID_GFP_CMF_ACC_UPI,
    CLI_CMD_ID_GFP_CDF_FCS_INSERT,
    CLI_CMD_ID_GFP_CDF_UPI,
    CLI_CMD_ID_GFP_CDF_ACC_UPI,
    CLI_CMD_ID_GFP_8B10B_CONTROL_CODE,
    CLI_CMD_ID_GFP_DEFECTS,
    CLI_CMD_ID_GFP_PERF,
#endif    
    CLI_CMD_ID_UPI_CONFIG,
    CLI_CMD_ID_UPI_STATUS,
    CLI_CMD_ID_UPI_TEST_CONFIG,
    CLI_CMD_ID_UPI_TEST_STATUS,
    CLI_CMD_ID_UPI_TXEQ_MODE,
    CLI_CMD_ID_UPI_TXEQ_COEF,
    CLI_CMD_ID_UPI_TXEQ_AMP,
    CLI_CMD_ID_UPI_TXEQ_SLEW,
    CLI_CMD_ID_UPI_RXEQ_MODE,
    CLI_CMD_ID_XAUI_CONFIG,
    CLI_CMD_ID_XAUI_STATUS,
    CLI_CMD_ID_XAUI_STATS,
    CLI_CMD_ID_XFI_CONFIG,
    CLI_CMD_ID_XFI_STATUS,
    CLI_CMD_ID_XFI_TEST_CONFIG,
    CLI_CMD_ID_XFI_TEST_STATUS,
    CLI_CMD_ID_XFI_REC_CLK_STATUS,
    CLI_CMD_ID_XFI_TXEQ_MODE,
    CLI_CMD_ID_XFI_TXMISC_PARM,
    CLI_CMD_ID_XFI_RXEQ_MODE,
    CLI_CMD_ID_XFI_TXEQ_COEFUPD,
    CLI_CMD_ID_XFI_802a_TXEQ_COEUPD,
    CLI_CMD_ID_XFI_802a_TXEQ_FSM,
#if defined(VTSS_FEATURE_MAC10G)
    CLI_CMD_ID_MAC10G_LB_CONFIG,
    CLI_CMD_ID_MAC10G_TX_FLOW_CTRL_CONFIG,
    CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_DROP_CONFIG,
    CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_ACTION_CONFIG,
    CLI_CMD_ID_MAC10G_FRAME_LEN_CONFIG,
#ifdef VTSS_ARCH_DAYTONA 
    CLI_CMD_ID_MAC10G_PREAMBLE_CHECK_CONFIG,
    CLI_CMD_ID_MAC10G_LF_CONFIG,
    CLI_CMD_ID_MAC10G_RF_CONFIG,
    CLI_CMD_ID_CHANNEL_MODE_CONFIG,
#endif /* VTSS_ARCH_DAYTONA */
    CLI_CMD_ID_MAC10G_STATUS,
    CLI_CMD_ID_MAC10G_RMON_STATS,
#endif /* VTSS_FEATURE_MAC10G */
#if defined (VTSS_FEATURE_PCS_10GBASE_R) 
    CLI_CMD_ID_PCS_10GBASE_R_LB_CONFIG,
    CLI_CMD_ID_PCS_10GBASE_R_STATUS,
    CLI_CMD_ID_PCS_10GBASE_R_STATS,
#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#if defined (VTSS_FEATURE_RAB) 
    CLI_CMD_ID_RAB_RX_THRESH_CONFIG,
    CLI_CMD_ID_RAB_TX_THRESH_CONFIG,
    CLI_CMD_ID_RAB_STATS,
    CLI_CMD_ID_RAB_CUT_THRU_RX,
    CLI_CMD_ID_RAB_CUT_THRU_TX,
#endif /* VTSS_FEATURE_RAB */
#if defined (VTSS_FEATURE_I2C)
    CLI_CMD_ID_I2C_INIT,
    CLI_CMD_ID_I2C_RX,
    CLI_CMD_ID_I2C_TX,
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_FEATURE_INTERRUPTS)
    CLI_CMD_ID_DEBUG_INTERRUPT,
#endif
#ifdef DAYTONA_EVAL_INIT
    CLI_CMD_ID_DEBUG_LBFPGA_READ,
    CLI_CMD_ID_DEBUG_LBFPGA_WRITE,
    CLI_CMD_ID_DEBUG_PCF8575_READ,
    CLI_CMD_ID_DEBUG_PCF8575_WRITE,
    CLI_CMD_ID_DEBUG_VSC3172_READ,
    CLI_CMD_ID_DEBUG_VSC3172_WRITE,
    CLI_CMD_ID_DEBUG_SI5338_READ,
    CLI_CMD_ID_DEBUG_SI5338_WRITE,
    CLI_CMD_ID_DEBUG_SI5326_READ,
    CLI_CMD_ID_DEBUG_SI5326_WRITE,
    CLI_CMD_ID_DEBUG_CLK_RESYNC,
#endif /* DAYTONA_EVAL_INIT */
#if defined(VTSS_FEATURE_OHA)
    CLI_CMD_ID_OHA_CONF,
#endif 
    CLI_CMD_ID_NPI_PORT,
    CLI_CMD_ID_TRANSMIT_NPI,
    CLI_CMD_ID_TRANSMIT_PORT,
    CLI_CMD_ID_TRANSMIT_VLAN,
    CLI_CMD_ID_LAST
} cli_cmd_id_t;

/* CLI command entry */
typedef struct cli_cmd_t {
    const char         *syntax; /* Syntax string */
    const char         *descr;  /* Description string */
    const cli_cmd_id_t id;      /* ID */
} cli_cmd_t;

/* CLI parameter type */
typedef enum {
    /* General */
    CLI_PARM_TYPE_KEYWORD,
    CLI_PARM_TYPE_MAX_FRAME,
    CLI_PARM_TYPE_SYSTEM_INST,
    CLI_PARM_TYPE_PORT_LIST,
    CLI_PARM_TYPE_LPORT_LIST,
    CLI_PARM_TYPE_LPORT_DIS,
    CLI_PARM_TYPE_VID,
    CLI_PARM_TYPE_MAC,
    CLI_PARM_TYPE_MAC_AGE_TIME,
    CLI_PARM_TYPE_TGT_LIST,
    CLI_PARM_TYPE_BLK_LIST,
    CLI_PARM_TYPE_SUB_LIST,
    CLI_PARM_TYPE_ADDR_LIST,
    CLI_PARM_TYPE_VALUE,
    CLI_PARM_TYPE_PHY_ADDR_LIST,
    CLI_PARM_TYPE_PHY_VALUE,
    CLI_PARM_TYPE_MMD_LIST,
    CLI_PARM_TYPE_MMD_ADDR,
    CLI_PARM_TYPE_PAGE,
    CLI_PARM_TYPE_API_LAYER,
    CLI_PARM_TYPE_API_GROUP,
    CLI_PARM_TYPE_TRACE_LAYER,
    CLI_PARM_TYPE_TRACE_GROUP,
    CLI_PARM_TYPE_CLK_FREQ,
    CLI_PARM_TYPE_GPIO_LIST,
    CLI_PARM_TYPE_CLK_GPIO_NO,
    CLI_PARM_TYPE_GPIO_VALUE,
    CLI_PARM_TYPE_FPGA_ADDR_LIST,
    CLI_PARM_TYPE_PORT_DIS,
    CLI_PARM_TYPE_PSI_CLOCK,
    CLI_PARM_TYPE_TABLE_ID,
    CLI_PARM_TYPE_TABLE_INDEX,
    CLI_PARM_TYPE_QUEUE_INDEX,
    CLI_PARM_TYPE_TRIG_INDEX,
    CLI_PARM_TYPE_TCP_UDP_PORT,
    CLI_PARM_TYPE_QOS_ORDER,
    CLI_PARM_TYPE_QOS_ENDPOINT,
    CLI_PARM_TYPE_DSCP_LIST,
    CLI_PARM_TYPE_OFFSET_VALUE,
    CLI_PARM_TYPE_PATTERN_VALUE,
    CLI_PARM_TYPE_CLASS_VALUE,
    CLI_PARM_TYPE_TAG_PRIO,
    CLI_PARM_TYPE_MPLS_EXP,
    CLI_PARM_TYPE_MASK_VALUE,
    CLI_PARM_TYPE_RED_VALUE,
    CLI_PARM_TYPE_DMAC,
    CLI_PARM_TYPE_DMAC_MASK,
    CLI_PARM_TYPE_ETYPE,
    CLI_PARM_TYPE_VLAN,
    CLI_PARM_TYPE_IP_PROTO,
    CLI_PARM_TYPE_QOS_BIT_RATE,
    CLI_PARM_TYPE_QOS_LEVEL,
    CLI_PARM_TYPE_RED_PROB_1,
    CLI_PARM_TYPE_RED_PROB_2,
    CLI_PARM_TYPE_RED_PROB_3,
    CLI_PARM_TYPE_RED_WEIGHT,
    CLI_PARM_TYPE_RED_MIN,
    CLI_PARM_TYPE_RED_MAX,
    CLI_PARM_TYPE_PCT,
    CLI_PARM_TYPE_CT_THRESHOLD,
#if defined(VTSS_FEATURE_WIS)
    CLI_PARM_TYPE_WIS_TTI_MODE,
    CLI_PARM_TYPE_WIS_TTI,
    CLI_PARM_TYPE_WIS_SL,
    CLI_PARM_TYPE_WIS_AISL,
    CLI_PARM_TYPE_WIS_RDIL,
    CLI_PARM_TYPE_WIS_FAULT,
    CLI_PARM_TYPE_WIS_LINERX,
    CLI_PARM_TYPE_WIS_LINETX,
    CLI_PARM_TYPE_WIS_PATH_FORCE,
    CLI_PARM_TYPE_WIS_OH_NAME,
    CLI_PARM_TYPE_WIS_OH_VALUE,
    
    CLI_PARM_TYPE_WIS_NETS,
    CLI_PARM_TYPE_WIS_NETL,
    CLI_PARM_TYPE_WIS_FETL,
    CLI_PARM_TYPE_WIS_NETP,
    CLI_PARM_TYPE_WIS_FETP,
    CLI_PARM_TYPE_WIS_FORCE_EVENTS,
    CLI_PARM_TYPE_WIS_PASSTHRU,
#endif /*  VTSS_FEATURE_WIS  */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    CLI_PARM_TYPE_1588_PORTNO,
    CLI_PARM_TYPE_1588_ENGID,
    CLI_PARM_TYPE_1588_CLK_FREQ,
    CLI_PARM_TYPE_1588_CLK_SRC,
    CLI_PARM_TYPE_1588_MOD_FRM,
    CLI_PARM_TYPE_1588_RX_TS_POS,
    CLI_PARM_TYPE_1588_TX_FIFO_MODE,
    CLI_PARM_TYPE_1588_ACTID,
    CLI_PARM_TYPE_1588_DIRECTION,
    CLI_PARM_TYPE_1588_ENCAP_TYPE,
    CLI_PARM_TYPE_1588_CHNLMAP,
    CLI_PARM_TYPE_1588_FLOW_ID,
    CLI_PARM_TYPE_1588_FLOW_ST_IDX,
    CLI_PARM_TYPE_1588_FLOW_END_IDX,
    CLI_PARM_TYPE_1588_TPID,
    CLI_PARM_TYPE_1588_NUMBER_OF_TAGS,
    CLI_PARM_TYPE_1588_TAG1_TYPE,
    CLI_PARM_TYPE_1588_TAG2_TYPE,
    CLI_PARM_TYPE_1588_TAG1_UPPER,
    CLI_PARM_TYPE_1588_TAG1_LOWER,
    CLI_PARM_TYPE_1588_TAG2_UPPER,
    CLI_PARM_TYPE_1588_TAG2_LOWER,
    CLI_PARM_TYPE_1588_MAC_ADDR,
    CLI_PARM_TYPE_1588_CLK_MODE,
    CLI_PARM_TYPE_1588_DOMAIN_LOWER,
    CLI_PARM_TYPE_1588_DOMAIN_UPPER,
    CLI_PARM_TYPE_1588_OAM_VER,
    CLI_PARM_TYPE_1588_PTP_DELAYM,
    CLI_PARM_TYPE_1588_Y1731_DELAYM,
    CLI_PARM_TYPE_1588_IETF_OAM_DELAYM,
    CLI_PARM_TYPE_1588_MEG_RANGE_LOWER,
    CLI_PARM_TYPE_1588_MEG_RANGE_UPPER,
    CLI_PARM_TYPE_1588_TS_FORMAT,
    CLI_PARM_TYPE_1588_TRAFFIC_CLASS,
    CLI_PARM_TYPE_1588_SIG_MASK,
    CLI_PARM_TYPE_1588_IPV4_KEYWORD,
    CLI_PARM_TYPE_1588_IPV6_KEYWORD,
    CLI_PARM_TYPE_1588_SPORT_VAL,
    CLI_PARM_TYPE_1588_SPORT_MASK,
    CLI_PARM_TYPE_1588_DPORT_VAL,
    CLI_PARM_TYPE_1588_DPORT_MASK,
    CLI_PARM_TYPE_1588_IPV4_ADDR,
    CLI_PARM_TYPE_1588_IPV4_MASK,
    CLI_PARM_TYPE_1588_IPV6_ADDR,
    CLI_PARM_TYPE_1588_IPV6_MASK,
    CLI_PARM_TYPE_1588_STK_DEPTH,
    CLI_PARM_TYPE_1588_STKLVL0,
    CLI_PARM_TYPE_1588_STKLVL1,
    CLI_PARM_TYPE_1588_STKLVL2,
    CLI_PARM_TYPE_1588_STKLVL3,
    CLI_PARM_TYPE_1588_ACH_VER,
    CLI_PARM_TYPE_1588_CHANNEL_TYPE,
    CLI_PARM_TYPE_1588_PROTOCOL_ID,
    CLI_PARM_TYPE_1588_TIME_SEC,
    CLI_PARM_TYPE_1588_LATENCY_VAL,
    CLI_PARM_TYPE_1588_DELAY_VAL,
    CLI_PARM_TYPE_1588_ASYM_VAL,
    CLI_PARM_TYPE_1588_MAC_MATCH,
    CLI_PARM_TYPE_1588_FLOW_MATCH,
    CLI_PARM_TYPE_1588_PBB_EN,
    CLI_PARM_TYPE_1588_ADDR_MATCH,
    CLI_PARM_TYPE_1588_VLAN_CHK,
    CLI_PARM_TYPE_1588_TAG_RANGE,
    CLI_PARM_TYPE_1588_IP_VER,
    CLI_PARM_TYPE_1588_MPLS_STKREF,
    CLI_PARM_TYPE_1588_ACT_PTP,
    CLI_PARM_TYPE_1588_ACT_Y1731,
    CLI_PARM_TYPE_1588_ACT_IETF_OAM,
    CLI_PARM_TYPE_1588_MPLS_CTRLW,
    CLI_PARM_TYPE_1588_BLK_ID,
    CLI_PARM_TYPE_1588_CSR_OFFSET,
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_FEATURE_OTN)
    CLI_PARM_TYPE_OTN_OCH_FEC,
    CLI_PARM_TYPE_OTN_OCH_BER_THRES,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BEIBIAE,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI,
    CLI_PARM_TYPE_OTN_OTU_CONACT_AIS,
    CLI_PARM_TYPE_OTN_OTU_CONACT_IAE,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOS,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOF,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOM,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_AIS,
    CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_TIM,
    CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_LOM,
    CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_TIM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BEI,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOS,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOF,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_AIS,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_TIM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_AIS,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OCI,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LCK,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LTC,
    CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_TIM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_AIS,
    CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_LOM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_OTU_TIM,
    CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_TIMOCILCK,
    CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_PLM,
    CLI_PARM_TYPE_OTN_ODU_MODE,
    CLI_PARM_TYPE_OTN_ODU_PL_PLM,
    CLI_PARM_TYPE_OTN_ODU_PL_TX,
    CLI_PARM_TYPE_OTN_ODU_PL_PAYLOAD_TYPE,
    CLI_PARM_TYPE_OTN_ODU_SIAIS,
    CLI_PARM_TYPE_OTN_ODU_SOAIS,
    CLI_PARM_TYPE_OTN_ODUT_LEVEL,
    CLI_PARM_TYPE_OTN_ODUT_BDI,
    CLI_PARM_TYPE_OTN_ODUT_STAT,
    CLI_PARM_TYPE_OTN_ODUT_APS,
    CLI_PARM_TYPE_OTN_TTI,
    CLI_PARM_TYPE_OTN_SAPI_TIM,
    CLI_PARM_TYPE_OTN_DAPI_TIM,
    CLI_PARM_TYPE_OTN_OS_TIM,
    CLI_PARM_TYPE_OTN_EX_TTI,
    CLI_PARAM_TYPE_OTN_VALUE,
    CLI_PARAM_TYPE_OTN_OH_INSERTION,
    CLI_PARAM_TYPE_OTN_OPU_OH_INSERTION,
    CLI_PARAM_TYPE_OTN_OTU_OH_INSERTION,
    CLI_PARAM_TYPE_OTN_TEST_INSERTION,
    CLI_PARM_TYPE_FTFL_VALUE,
    CLI_PARM_TYPE_FTFL_INDEX,
    CLI_PARM_TYPE_OTN_OPU_PAYLOAD_TYPE,
#endif /* VTSS_FEATURE_OTN */
#if defined(VTSS_FEATURE_GFP)
    CLI_PARAM_TYPE_GFP_CMF_FORCED_INTERVAL,
    CLI_PARAM_TYPE_GFP_CMF_FRAME_LENGTH,
    CLI_PARAM_TYPE_GFP_UPI,
    CLI_PARAM_TYPE_GFP_PTI,
    CLI_PARAM_TYPE_GFP_ORDERED_UPI,
    CLI_PARAM_TYPE_GFP_8B10B_FORCED,
    CLI_PARAM_TYPE_GFP_8B10B_ERROR,
    CLI_PARAM_TYPE_GFP_CMF_AUTO_CONF,
    CLI_PARAM_TYPE_GFP_SINGLE_ERROR_EN_DIS,
#endif /* VTSS_FEATURE_GFP */
    CLI_PARM_TYPE_GEN_MODE,
    CLI_PARM_TYPE_CHK_MODE,
    CLI_PARM_TYPE_WRP_MODE,
    CLI_PARM_TYPE_MAC10G_MAX_FRAME_LEN,
    CLI_PARM_TYPE_MAC10G_MAX_TAGS,
    CLI_PARM_TYPE_RAB_THRESH_HIGH,
    CLI_PARM_TYPE_RAB_THRESH_LOW,
    CLI_PARM_TYPE_RAB_THRESH_ADAPT,
    CLI_PARM_TYPE_Q_CODE,
    CLI_PARM_TYPE_UPI_TXEQ_MODE,
    CLI_PARM_TYPE_UPI_TXEQ_CHNLOSS,
    CLI_PARM_TYPE_UPI_TXEQ_COEF_ID,
    CLI_PARM_TYPE_UPI_TXEQ_COEF_STEP,
    CLI_PARM_TYPE_UPI_TXEQ_COEF_INCR,
    CLI_PARM_TYPE_UPI_TXEQ_COEF_POLARITY,
    CLI_PARM_TYPE_UPI_TXEQ_POWER,
    CLI_PARM_TYPE_UPI_TXEQ_SLEW,
    CLI_PARM_TYPE_UPI_RXEQ_MODE,
    CLI_PARM_TYPE_XFI_TXEQ_MODE,
    CLI_PARM_TYPE_TXEQ_CHNLOSS,
    CLI_PARM_TYPE_TXEQ_PORTNO,
    CLI_PARM_TYPE_XFI_TXEQ_COEF,
    CLI_PARM_TYPE_XFI_TXEQ_STEPSIZE,
    CLI_PARM_TYPE_XFI_TXEQ_INCR,
    CLI_PARM_TYPE_XFI_TXEQ_POLARITY,
    CLI_PARM_TYPE_XFI_TXEQ_PARM_VALUE,
    CLI_PARM_TYPE_XFI_RXEQ_MODE,
    CLI_PARM_TYPE_XFI_802A_TXEQ_UPDREQ,
    CLI_PARM_TYPE_XFI_802A_TXEQ_LANE,
    CLI_PARM_TYPE_XFI_802A_TXEQ_COEF,
    CLI_PARM_TYPE_XFI_802A_FSM_CTL,
#if defined(VTSS_FEATURE_I2C)
    CLI_PARM_TYPE_I2C_CONTROLLER_NO,
    CLI_PARM_TYPE_I2C_DEV_ADDR,
    CLI_PARM_TYPE_I2C_REG_ADDR,
    CLI_PARM_TYPE_I2C_DATA,
#endif /* VTSS_FEATURE_I2C */
    CLI_PARM_TYPE_EVENT,
    CLI_PARM_TYPE_PORT,
    CLI_PARM_TYPE_PMTICK_INTERVAL,
#ifdef VTSS_ARCH_DAYTONA
    CLI_PARM_TYPE_CHANNEL_NUM,
    CLI_PARM_TYPE_CHANNEL_MODE,
    CLI_PARM_TYPE_CHANNEL_XAUIMODE,
    CLI_PARM_TYPE_CHANNEL_CONFIG_EVINIT,
#endif
#if defined(VTSS_FEATURE_OHA)
    CLI_PARM_TYPE_OHA_ROSITOSI,
    CLI_PARM_TYPE_OHA_OTN1_OTN2,
#endif
#if defined(DAYTONA_EVAL_INIT)
    CLI_PARM_TYPE_LBFPGA_REG,
    CLI_PARM_TYPE_PCF8575_TGTID,
    CLI_PARM_TYPE_VSC3172_REG,
    CLI_PARM_TYPE_SI5338_TGTID,
    CLI_PARM_TYPE_SI5338_REG,
    CLI_PARM_TYPE_SI5326_TGTID,
    CLI_PARM_TYPE_SI5326_REG,
    CLI_PARM_TYPE_FPGA_MASK_VALUE,
    CLI_PARM_TYPE_VAL_MASK_VALUE,
#endif
    CLI_PARM_MODIFY_BIT_7,
    CLI_PARM_TYPE_LAST
} cli_parm_type_t;

#define CLI_PARM_FLAG_NONE   0x00000000 /* No flags */
#define CLI_PARM_FLAG_NO_TXT 0x00000001 /* Suppress identification text */
#define CLI_PARM_FLAG_SET    0x00000002 /* Set operation parameter */
#define CLI_PARM_FLAG_DUAL   0x00000004 /* Dual parameter */

/* CLI parameter entry */
typedef struct {
    const char            *txt;  /* Identification text */
    const char            *help; /* Help text */
    const cli_parm_type_t type;  /* Type */
    const cli_cmd_id_t    id;    /* Optional command ID */
    const u32             flags; /* Miscellaneous flags */
} cli_parm_t;

static cli_cmd_t cli_cmd_table[] = {
    {
        "Help",
        "Show CLI general help text",
        CLI_CMD_ID_HELP
    },
    {
        "System Configuration",
        "Show system information",
        CLI_CMD_ID_SYSTEM_CONF
    },
#if (VTSS_APPL_INST_CNT > 1)
    {
        "System Instance [<inst>]",
        "Set or show the system instance",
        CLI_CMD_ID_SYSTEM_INST
    },
#endif /* VTSS_APPL_INST_CNT > 1 */
#if defined(VTSS_ARCH_B2)
    {
        "System Initialization",
        "Show initialization setup",
        CLI_CMD_ID_SYSTEM_INIT,
    },
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON28) ||\
    defined(VTSS_CHIP_CU_PHY) ||\
    defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
                          
    {
        "System Reboot",
        "Reboot the system",
        CLI_CMD_ID_SYSTEM_RESET
    },
#if defined(VTSS_FEATURE_WARM_START)
    {
        "System Restart [warm|cool]",
        "Set or show the restart mode",
        CLI_CMD_ID_SYSTEM_RESTART
    },
#endif /* VTSS_FEATURE_WARM_START */
#ifndef VTSS_ARCH_DAYTONA
    {
        "Port Configuration [<port_list>]",
        "Show port configuration",
        CLI_CMD_ID_PORT_CONF
    },
    {
        "Port State [<port_list>] [enable|disable]",
        "Set or show the port administrative state",
        CLI_CMD_ID_PORT_STATE
    },
    {
        "Port Transmit [<port_list>]",
        "Transmit (broadcast) on port - using vtss_packet_tx_frame_port",
        CLI_CMD_ID_PORT_TRANSMIT
    },
    {
        "Port Mode [<port_list>] [10hdx|10fdx|100hdx|100fdx|1000fdx|auto]",
        "Set or show the port speed and duplex mode",
        CLI_CMD_ID_PORT_MODE
    },
    {
        "Port Flow Control [<port_list>] [enable|disable]",
        "Set or show the port flow control mode",
        CLI_CMD_ID_PORT_FLOW_CONTROL
    },
#endif /* ifndef VTSS_ARCH_DAYTONA */
#endif
#if defined(VTSS_FEATURE_PORT_CONTROL)
#ifndef VTSS_ARCH_DAYTONA
    {
        "Port MaxFrame [<port_list>] [<max_frame>]",
        "Set or show the port maximum frame size",
        CLI_CMD_ID_PORT_MAX_FRAME
    },
    {
        "Port Statistics [<port_list>] [clear|packets|bytes|errors|discards]",
        "Show port statistics",
        CLI_CMD_ID_PORT_STATS
    },
#endif /* ifndef VTSS_ARCH_DAYTONA */
    {
        "Port Map [<port_list>]",
        "Show port map",
        CLI_CMD_ID_PORT_MAP    
    },
#endif /* VTSS_FEATURE_PORT_CONTROL */
#if defined(VTSS_FEATURE_LAYER2)
    {
        "MAC Add <mac_addr> <port_list> [<vid>]",
        "Add MAC address table entry",
        CLI_CMD_ID_MAC_ADD
    },
    {
        "MAC Delete <mac_addr> [<vid>]",
        "Delete MAC address entry",
        CLI_CMD_ID_MAC_DEL
    },
    {
        "MAC Lookup <mac_addr> [<vid>]",
        "Lookup MAC address entry",
        CLI_CMD_ID_MAC_LOOKUP
    },
    {
        "MAC Dump",
        "Show sorted list of MAC address entries",
        CLI_CMD_ID_MAC_DUMP
    },
    {
        "MAC Flush",
        "Flush all learned entries",
        CLI_CMD_ID_MAC_FLUSH
    },
    {
        "MAC Agetime [<age_time>]",
        "Set or show the MAC address age timer",
        CLI_CMD_ID_MAC_AGE_TIME
    },
#endif /* VTSS_FEATURE_LAYER2 */
#if defined(VTSS_ARCH_B2)
    {
        "Port Filter [<port_list>]\n"
        "            [mac_ctrl|mac_zero|dmac_bc|smac_mc|untag|prio_tag|c_tag|s_tag|tags]\n" 
        "            [allow|deny] [any_tag|1_tag|2_tags]",
        "Set or show the filter configuration",
        CLI_CMD_ID_PORT_FILTER
    },
    {
        "Port PSI [<psi_clock>]",
        "Setup Port Status Interface",
        CLI_CMD_ID_PORT_PSI
    },
    {
        "Port Forward [<port_list>] [enable|disable|ingress|egress]",
        "Set or show the port forwarding state",
        CLI_CMD_ID_PORT_FORWARD
    },
    {
        "Port CT [<port_list>] [rx|tx] [<ct_value>]",
        "Set or show the port cut-through mode. Units in bytes.",
        CLI_CMD_ID_PORT_CT
    },
    {
        "Port PFCI [<port_list>] [enable|disable]",
        "Set or show the Port Flow Control Interface mode",
        CLI_CMD_ID_PORT_PFCI
    },
    {
        "Map Port <vid> <port>|disable",
        "Set mapping from VID to port (host mode 0, 1 and 3)",
        CLI_CMD_ID_MAP_PORT
    },
    {
        "Map Lport <vid> <lport>|disable",
        "Set mapping from VID to logical port (host mode 8, 10 and 11)",
        CLI_CMD_ID_MAP_LPORT
    },
    {
        "QoS Shaper lport [<lport_list>] [<bit_rate>] [<level>]",
        "Set or show the logical port/channel shapers",
        CLI_CMD_ID_QOS_LPORT_SHAPER,
    },
    {
        "QoS Global DSCP [<table_id>] [<dscp_list>] [enable|disable] [<class>] [<red>]",
        "Set or show the global DSCP settings",
        CLI_CMD_ID_QOS_GLOBAL_DSCP
    },
    {
        "QoS Global Filter Trigger [<table_index>] [<trig_index>] [l2|l3|l4]\n" 
        "                          [<offset>] [<pattern>] [<mask>]",
        "Set or show the 8 global filters",
        CLI_CMD_ID_QOS_GLOBAL_FILTTRIG,
    },
    {
        "QoS Global Filter Action [<table_index>] [drop|forward] [<class>] [<red>]",
        "Classify the 8 global filters",
        CLI_CMD_ID_QOS_GLOBAL_FILTACTION,
    },
    {
        "QoS Global Filter Port [<port_list>] [<table_index>] [enable|disable]",
        "Port enable / disable one of the 8 global filters",
        CLI_CMD_ID_QOS_GLOBAL_FILTENABLE,
    },
    {
        "QoS Global ip_port table [<table_index>] [range|specific] [<tcp_udp_port>]\n" 
        "                         [<class>] [<red>]",
        "Set or show the 8 global TCP/UDP ports",
        CLI_CMD_ID_QOS_GLOBAL_TCPUDP
    },
    {
        "QoS Global ip_port port [<port_list>] [<table_index>] [enable|disable]",
        "Port enable / disable one of the 8 global TCP/UDP ports",
        CLI_CMD_ID_QOS_GLOBAL_TCPUDP_ENABLE
    },

    {
        "QoS Filter Action [<port_list>] [drop|forward] [<class>] [<red>]",
        "Classify the local filters",
        CLI_CMD_ID_QOS_FILTACTION,
    },
    {
        "QoS Filter Trigger [<port_list>] [<trig_index>] [l2|l3|l4]\n"
        "                   [<offset>] [<pattern>] [<mask>]",
        "Set or show the local filter",
        CLI_CMD_ID_QOS_FILTTRIG,
    },
    {
        "QoS Priorder [<port_list>] [<order_rank>] [<endpoint_id>]",
        "Set or show the order of the 17 classification endpoints",
        CLI_CMD_ID_QOS_PRIORDER,
    },
    {
        "QoS Class misc [<port_list>] [default|l2_ctrl|l3_ctrl|cfi|ipv4|ipv6|llc]\n" 
        "               [<class>] [<red>]",
        "Set or show the order of the 17 classification endpoints",
        CLI_CMD_ID_QOS_CLASS_MISC,
    },
    {
        "QoS Class ip_port [<port_list>] [<table_id>] [range|specific] [<tcp_udp_port>]\n"
        "                  [<class>] [<red>]",
        "Set or show the TCP/UDP class status",
        CLI_CMD_ID_QOS_CLASS_TCPUDP,
    },
    {
        "QoS Class ip_proto [<port_list>] [<ip_proto>] [<class>] [<red>]",
        "Set or show the IP protocol class status",
        CLI_CMD_ID_QOS_CLASS_IP_PROTO,
    },
    {
        "QoS Class etype [<port_list>] [<etype>] [<class>] [<red>]",
        "Set or show the Ethernet type class status",
        CLI_CMD_ID_QOS_CLASS_ETYPE,
    },
    {
        "QoS Class vid [<port_list>] [<table_id>] [<vid>] [<class>] [<red>]",
        "Set or show the VLAN id class status",
        CLI_CMD_ID_QOS_CLASS_VID,
    },
    {
        "QoS Class tag_prio [<port_list>] [<tag_prio>] [<class>] [<red>]",
        "Set or show Tag Priority class status",
        CLI_CMD_ID_QOS_CLASS_TAG_PRIO,
    },
    {
        "QoS Class mpls_exp [<port_list>] [<mpls_exp>] [<class>] [<red>]",
        "Set or show MPLS EXP class status",
        CLI_CMD_ID_QOS_CLASS_MPLS,
    },
    {
        "QoS Police Port [<port_list>] [<table_id>] [mc|bc|uc|all] [<bit_rate>] [<level>]",
        "Set or show the port policers, 2 policers per port",
        CLI_CMD_ID_QOS_PORT_POLICE,
    },
    {
        "QoS Police Queue [<port_list>] [<q_index>] [mc|bc|uc|all] [<bit_rate>] [<level>]",
        "Set or show the queue policers, 8 policers per queue",
        CLI_CMD_ID_QOS_QUEUE_POLICE,
    },
    {
        "QoS Shaper port [<port_list>] [<bit_rate>] [<level>]",
        "Set or show the line/host port shapers",
        CLI_CMD_ID_QOS_PORT_SHAPER,
    },
    {
        "QoS Tag [<port_list>] [ctag|cfi|vlan_aware|tag_inner] [enable|disable]",
        "Set or show the order of the 17 classification endpoints",
        CLI_CMD_ID_QOS_TAG,
    },
    {
        "QoS L2_filter altorder [<port_list>] [<order_rank>] [<endpoint_id>]",
        "Set or show the alternate order of the 17 classification endpoints",
        CLI_CMD_ID_QOS_L2_FILTER_ALTORDER,
    },
    {
        "QoS L2_filter dmac [<port_list>] [<table_id>] [<dmac>] [<dmac_mask>]",
        "Set or show the L2 filter dmac",
        CLI_CMD_ID_QOS_L2_FILTER_DMAC,
    },
    {
        "QoS L2_filter vid [<port_list>] [<table_id>] [<vid>] [<mask>]",
        "Set or show the L2 filter vid",
        CLI_CMD_ID_QOS_L2_FILTER_VID,
    },
    {
        "QoS L2_filter etype [<port_list>] [<table_id>] [<etype>] [<mask>]",
        "Set or show the L2 filter etype",
        CLI_CMD_ID_QOS_L2_FILTER_ETYPE,
    },
    {
        "QoS L2_filter port [<port_list>] [<table_id>] [enable|disable] [prim|alt]",
        "Set or show the L2 filter port enable status",
        CLI_CMD_ID_QOS_L2_FILTER_PORT,
    },
    {
        "QoS L2_filter conf [<port_list>]",
        "show the L2 filter confs",
        CLI_CMD_ID_QOS_L2_FILTER_CONF,
    },

    {
        "QoS Red Threshold [<port_list>] [<q_index>] [<red_min>] [<red_max>] [<red_weight>]",
        "Set or show the Red min/max/weight config",
        CLI_CMD_ID_QOS_RED_THRESHOLD,
    },
    {
        "QoS Red Probability [<port_list>] [<q_index>] [<prob_1>] [<prob_2>] [<prob_3>]",
        "Set or show the Red probabilty profile",
        CLI_CMD_ID_QOS_RED_PROBABILITY,
    },
    {
        "QoS Red Profile [<port_list>] [<q_index>] [enable|disable]",
        "Set or show the red profile status",
        CLI_CMD_ID_QOS_RED_PROFILE,
    },
    {
        "QoS Scheduler queue [<port_list>] [<q_index>] [<pct>]",
        "Set or show the queue scheduler for queues 0-5. 6-7 are strict.",
        CLI_CMD_ID_QOS_QUEUE_SCHEDULER,
    },
    {
        "QoS Scheduler port [<port_list>] [<bit_rate>]",
        "Set or show the port scheduler",
        CLI_CMD_ID_QOS_PORT_SCHEDULER,
    },
    {
        "QoS Scheduler lport [<lport_list>] [rx|tx] [<bit_rate>]",
        "Set or show the logical port scheduler as seen from the SPI4 interface\n"
        "(rx:SPI4  tx:XAUI0/XAUI1)",
        CLI_CMD_ID_QOS_LPORT_SCHEDULER,
    },
    {
        "QoS DSCP table [<port_list>] [<table_id>]",
        "Set or show the DSCP table used",        
        CLI_CMD_ID_QOS_DSCP_TABLE,
    },


#if defined(VTSS_GPIOS)
    {
        "GPIO clk <port>|disable <gpio_no> [<clk_freq>]",
        "Set or show the DSCP table used",        
        CLI_CMD_ID_CLK_GPIO,
    },
#endif /* VTSS_GPIOS */
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_CHIP_10G_PHY)
    {
        "Phy_10g mode [<port_list>]",
        "Show the 10G Phy operating mode",        
        CLI_CMD_ID_PHY_10G_MODE,
    },
    {
        "Phy_10g status [<port_list>]",
        "Show the 10G Phy status",        
        CLI_CMD_ID_PHY_10G_STATUS,
    },
    {
        "Phy_10g reset [<port_list>]",
        "Reset the 10G Phy",        
        CLI_CMD_ID_PHY_10G_RESET,
    },
    {
        "Phy_10g loopback [<port_list>] [a|b|c|d|e|f|g|h|j|k] [enable|disable]",
        "Enable/disable loopback",        
        CLI_CMD_ID_PHY_10G_LOOPBACK,
    },
    {
        "Phy_10g statistics [<port_list>]",
        "Show the statistics",        
        CLI_CMD_ID_PHY_10G_STATS,
    },
    {
        "Phy_10g power [<port_list>] [enable|disable]",
        "Enable/disable power",        
        CLI_CMD_ID_PHY_10G_POWER,
    },
#endif /* VTSS_CHIP_10G_PHY */
    {
        "Debug API [<layer>] [<group>] [<port_list>] [full]",
        "Show API debug information",
        CLI_CMD_ID_DEBUG_API
    },
#if defined(VTSS_ARCH_LUTON28)
    {
        "Debug Register Read <blk_list> <sub_list> <addr_list> [binary|decimal]",
        "Read chip register",
        CLI_CMD_ID_DEBUG_REG_READ_L28
    },
    {
        "Debug Register Write <blk_list> <sub_list> <addr_list> <value>",
        "Write chip register",
        CLI_CMD_ID_DEBUG_REG_WRITE_L28
    },
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        "Debug Register Read <tgt_list> <addr_list> [binary|decimal]",
        "Read chip register",
        CLI_CMD_ID_DEBUG_REG_READ
    },
    {
        "Debug Register Write <tgt_list> <addr_list> <value>",
        "Write chip register",
        CLI_CMD_ID_DEBUG_REG_WRITE
    },
#endif
#if defined(VTSS_ARCH_B2)
    {
        "Debug Targets",
        "Show target ID",
        CLI_CMD_ID_DEBUG_TARGETS,
    },
#if defined(BOARD_B2_EVAL)
    {
        "Debug FPGA Read <addr_list> [binary|decimal]",
        "Read FPGA register",
        CLI_CMD_ID_DEBUG_FPGA_READ,
    },
    {
        "Debug FPGA Write <addr_list> <value>",
        "Write FPGA register",
        CLI_CMD_ID_DEBUG_FPGA_WRITE,
    },
    {
        "Debug FPGA Status",
        "Show FPGA status",
        CLI_CMD_ID_DEBUG_FPGA_STATUS,
    },
#endif /* BOARD_B2_EVAL */
#if defined(VTSS_GPIOS)
    {
        "Debug GPIO Output [<gpio_list>] [enable|disable]",
        "Setup GPIO direction",
        CLI_CMD_ID_DEBUG_GPIO_OUTPUT,
    },
    {
        "Debug GPIO Read [<gpio_list>]",
        "Read value from GPIO input pin",
        CLI_CMD_ID_DEBUG_GPIO_READ,
    },
    {
        "Debug GPIO Write [<gpio_list>] <gpio_value>",
        "Write value to GPIO output pin",
        CLI_CMD_ID_DEBUG_GPIO_WRITE,
    },
#endif /* VTSS_GPIOS */
#if defined(VTSS_FEATURE_INTERRUPTS)
    {
        "Debug Interrupt [<port_list>] [enable|disable]",
        "Set interrupt mode or show interrupt status",
        CLI_CMD_ID_DEBUG_INTERRUPT,
    },
#endif /* VTSS_FEATURE_INTERRUPTS */

    {
        "Debug SPI4 [clear]",
        "Debug SPI4 interface",
        CLI_CMD_ID_DEBUG_SPI4,
    },
    {
        "Debug QSS",
        "Debug Queue Sub System",
        CLI_CMD_ID_DEBUG_QSS,
    },
    {
        "Debug BufCnt <port_list>",
        "Show QSS buffer usage",
        CLI_CMD_ID_DEBUG_BUF,
    },
    {
        "Debug Red <port_list>",
        "Debug Red drop counters",
        CLI_CMD_ID_DEBUG_RED_CNT,
    },

#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON28) ||\
    defined(VTSS_CHIP_CU_PHY) ||\
    defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
#ifndef VTSS_ARCH_DAYTONA
    {
        "Debug PHY Read <port_list> <addr_list> [<page>] [binary|decimal]",
        "Read PHY register",
        CLI_CMD_ID_DEBUG_PHY_READ,
    },
    {
        "Debug PHY Write <port_list> <addr_list> <value> [<page>]",
        "Write PHY register",
        CLI_CMD_ID_DEBUG_PHY_WRITE,
    },
#endif /* ifndef VTSS_ARCH_DAYTONA */
#endif
#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
    {
        "Debug MMD Read <port_list> <mmd_list> <mmd_addr> [binary|decimal]",
        "Read MMD register",
        CLI_CMD_ID_DEBUG_MMD_READ,
    },
    {
        "Debug MMD Write <port_list> <mmd_list> <mmd_addr> <value>",
        "Write MMD register",
        CLI_CMD_ID_DEBUG_MMD_WRITE,
    },
#endif /* VTSS_FEATURE_10G */
#if defined(VTSS_FEATURE_MISC)
    {
        "Debug Chip ID",
        "Read chip ID",
        CLI_CMD_ID_DEBUG_CHIP_ID,
    },
#endif /* VTSS_FEATURE_MISC */
    {
        "Debug Suspend",
        "Suspend application",
        CLI_CMD_ID_DEBUG_SUSPEND,
    },
    {
        "Debug Resume",
        "Resume application",
        CLI_CMD_ID_DEBUG_RESUME,
    },
#if VTSS_OPT_TRACE
    {
        "Debug Trace [<layer>] [<group>] [off|error|info|debug|noise]",
        "Set or show the trace level for group",
        CLI_CMD_ID_DEBUG_TRACE
    },
#endif /* VTSS_OPT_TRACE */
#ifdef VTSS_ARCH_DAYTONA
    {
        "Debug Register Read <tgt_list> <addr_list> [binary|decimal]",
        "Read chip register",
        CLI_CMD_ID_DEBUG_REG_READ_DAYTONA
    },
    {
        "Debug Register Write <tgt_list> <addr_list> <value>",
        "Write chip register",
        CLI_CMD_ID_DEBUG_REG_WRITE_DAYTONA
    },
#if defined(VTSS_GPIOS)
    {
        "Debug GPIO Output [<gpio_list>] [enable|disable]",
        "Setup GPIO direction",
        CLI_CMD_ID_DEBUG_GPIO_OUTPUT,
    },
    {
        "Debug GPIO Read [<gpio_list>]",
        "Read value from GPIO input pin",
        CLI_CMD_ID_DEBUG_GPIO_READ,
    },
    {
        "Debug GPIO Write [<gpio_list>] <gpio_value>",
        "Write value to GPIO output pin",
        CLI_CMD_ID_DEBUG_GPIO_WRITE,
    },
#endif /* VTSS_GPIOS */
#ifdef DAYTONA_EVAL_INIT
    {
        "Debug Val_board lbfpga Read <reg_addr> [binary|decimal]",
        "Read Validation Board LBFPGA register",
        CLI_CMD_ID_DEBUG_LBFPGA_READ,
    },
    {
        "Debug Val_board lbfpga Write <reg_addr> <value> <fpga_mask>",
        "Write Validation Board LBFPGA register",
        CLI_CMD_ID_DEBUG_LBFPGA_WRITE,
    },
    {
        "Debug Val_board pcf8575 Read <target_id> [binary|decimal]",
        "Read Validation Board pcf8575",
        CLI_CMD_ID_DEBUG_PCF8575_READ,
    },
    {
        "Debug Val_board pcf8575 Write <target_id> <value> <val_mask>",
        "Read Validation Board pcf8575",
        CLI_CMD_ID_DEBUG_PCF8575_WRITE,
    },
    {
        "Debug Val_board vsc3172 Read <reg_addr> [binary|decimal]",
        "Read Validation Board vsc3172 register",
        CLI_CMD_ID_DEBUG_VSC3172_READ,
    },
    {
        "Debug Val_board vsc3172 Write <reg_addr> <value> <val_mask>",
        "Write Validation Board vsc3172 register",
        CLI_CMD_ID_DEBUG_VSC3172_WRITE,
    },
    {
        "Debug Val_board si5338 Read <target_id> <reg_addr> [binary|decimal]",
        "Read Validation Board si5338 register",
        CLI_CMD_ID_DEBUG_SI5338_READ,
    },
    {
        "Debug Val_board si5338 Write <target_id> <reg_addr> <value> <val_mask>",
        "Write Validation Board si5338 register",
        CLI_CMD_ID_DEBUG_SI5338_WRITE,
    },
    {
        "Debug Val_board si5326 Read <target_id> <reg_addr> [binary|decimal]",
        "Read Validation Board si5326 register",
        CLI_CMD_ID_DEBUG_SI5326_READ,
    },
    {
        "Debug Val_board si5326 Write <target_id> <reg_addr> <value> <val_mask>",
        "Write Validation Board si5326 register",
        CLI_CMD_ID_DEBUG_SI5326_WRITE,
    },
    {
        "Debug Val_board clock_sync <channel_num>",
        "Resynchronize the clocks on the line side and client side for a channel",
        CLI_CMD_ID_DEBUG_CLK_RESYNC,
    },
#endif
#if defined(VTSS_FEATURE_OTN)
    {
        "OTN_OTU acc_res <port_list>",
        "Get OTN OTU ACC_RES",
        CLI_CMD_ID_OTN_OTU_ACC_RES,
    },
    {
        "OTN_OTU acc_gcc0 <port_list>",
        "Get OTN OTU ACC_GCC0",
        CLI_CMD_ID_OTN_OTU_ACC_GCC0,
    },
    {
        "OTN_OTU acc_smres <port_list>",
        "Get OTN OTU ACC_SEMRES",
        CLI_CMD_ID_OTN_OTU_ACC_SMRES,
    },
    {
        "OTN_OTU tx_res <port_list> [enable|disable] [<otn_value>]",
        "Get or Set OTN OTU TX_RES",
        CLI_CMD_ID_OTN_OTU_TX_RES,
    },
    {
        "OTN_OTU tx_smres <port_list> [enable|disable] [<otn_value>]",
        "Get or Set OTN_OTU TX_SMRES",
        CLI_CMD_ID_OTN_OTU_TX_SMRES,
    },
    {
        "OTN_OTU tx_gcc0 <port_list> [enable|disable] [<otn_value>]",
        "Get or Set OTN_OTU TX_GCC0",
        CLI_CMD_ID_OTN_OTU_TX_GCC0,
    },
    {
        "OTN_OTU oh_insertion <port_list> [<mode>]",
        "Get or Set OTN_OTU OH_INSERTION",
        CLI_CMD_ID_OTN_OTU_OH_INSERTION,
    },
    {
        "OTN_ODU oh_insertion <port_list> [<mode>]",    
        "Get or Set OTN_ODU OH_INSERTION",
        CLI_CMD_ID_OTN_ODU_OH_INSERTION,
    },
    {
        "OTN_ODU tx_res <port_list> [enable|disable] [<otn_value>]",
        "Get or Set OTN_ODU TX_RES",
        CLI_CMD_ID_OTN_ODU_TX_RES,
    },
    {
        "OTN_ODU acc_res <port_list>",      
        "Get OTN_ODU ACC_RES",
        CLI_CMD_ID_OTN_ODU_ACC_RES,
    },
    {
        "OTN_ODU tx_exp <port_list> [enable|disable] [<otn_value>]",
        "Get or Set OTN_ODU TX_EXP",
        CLI_CMD_ID_OTN_ODU_TX_EXP,
    },
    {
        "OTN_ODU acc_exp <port_list>",     
        "Get OTN_ODU ACC_EXP",
        CLI_CMD_ID_OTN_ODU_ACC_EXP,
    },
    {
        "OTN_ODU tx_ftfl <port_list> [enable|disable] [<ftfl_index>] [<ftfl_value>]", 
        "Get or Set OTN_ODU TX_FTFL",
        CLI_CMD_ID_OTN_ODU_TX_FTFL,
    },
    {
        "OTN_ODU acc_ftfl <port_list>", 
        "Get OTN_ODU ACC_FTFL",
        CLI_CMD_ID_OTN_ODU_ACC_FTFL,
    },
    {
        "OTN_OPU oh_insertion <port_list> [<mode>]", 
        "Get or Set OTN_OPU OH_INSERTION",
        CLI_CMD_ID_OTN_OPU_OH_INSERTION,
    },
    {
        "OTN_OPU test_insertion <port_list> [enable|disable] [<test_payload_type>] [<otn_value>]", 
        "Get or Set OTN_OPU TEST_INSERTION",
        CLI_CMD_ID_OTN_OPU_TEST_INSERTION,
    },
    {
        "OTN_OCH loopback [<port_list>] [line_side_equip|line_side_line|sys_side_equip|sys_side_line|no_loopback]",
        "Set/Get OCH Loopback",
        CLI_CMD_ID_OTN_OCH_LOOPBACK,
    },
    {
        "OTN_OCH fec [<port_list>] [none_fec|rs_fec|i4_fec|i7_fec]",
        "Set/Get OTN OCH FEC mode",
        CLI_CMD_ID_OTN_OCH_FEC,
    },
    {
        "OTN_OCH ber_threshold [<port_list>] [none_fec|rs_fec|i4_fec|i7_fec] [ber_threshold]]",
        "Set/Get OTN OCH FEC Ber Threshold",
        CLI_CMD_ID_OTN_OCH_BER_THRESHOLD,
    },
    {
        "OTN_OCH defects [<port_list>]",
        "Get OTN OCH Defects",
        CLI_CMD_ID_OTN_OCH_DEFECTS,
    },
    {
        "OTN_OCH performance [<port_list>]",
        "Get OTN OCH Performance",
        CLI_CMD_ID_OTN_OCH_PERF,
    },
    {
        "OTN_OTU tti <port> [enable|disable] [<otn_tti>] [sapi_tim_en|sapi_tim_dis] [dapi_tim_en|dapi_tim_dis] [os_tim_en|os_tim_dis] [<exp_tti>]",
        "Set or get OTN OTU SM tti",
        CLI_CMD_ID_OTN_OTU_TTI,
    },
    {
       "OTN_OTU Acc_tti [<port_list>]",
       "Get OTN OTU Acc_TTI",
       CLI_CMD_ID_OTN_OTU_ACCTTI,
    },
    {
       "OTN_OTU Cons_Actions [<port_list>] [<beibiae>] [<bdi>] [<ais>] [<bdi_los>] [<bdi_lof>] [<bdi_lom>] [<bdi_ais>] [<bdi_tim>] [<ais_lom>] [<ais_tim>]",
       "Set or Get OTN OTU Consequent Actions",
       CLI_CMD_ID_OTN_OTU_CONSACTS,
    },
    {
       "OTN_OTU Cons_Action_iae [<port_list>] [<iae>]",
       "Set or Get OTN OTU IAE Consequent Action",
       CLI_CMD_ID_OTN_OTU_CONSACTS_IAE,
    },
    {
        "OTN_OTU defects [<port_list>]",
        "Get OTN OTU Defects",
        CLI_CMD_ID_OTN_OTU_DEFECTS,
    },
    {
       "OTN_OTU performance [<port_list>]",
       "Get OTN OTU Perfomance",
       CLI_CMD_ID_OTN_OTU_PERF,
    },
    {
        "OTN_ODU mode [<port_list>] [termination|monitor]",
        "Set/Get OTN ODU mode",
        CLI_CMD_ID_OTN_ODU_MODE,
    },
    {
        "OTN_ODU tti <port> [enable|disable] [<otn_tti>] [sapi_tim_en|sapi_tim_dis] [dapi_tim_en|dapi_tim_dis] [os_tim_en|os_tim_dis] [<exp_tti>]",
        "Set or get OTN ODU TTI",
        CLI_CMD_ID_OTN_ODU_TTI,
    },
    {
       "OTN_ODU Acc_TTI [<port_list>]",
       "Get OTN ODU Acc_TTI",
       CLI_CMD_ID_OTN_ODU_ACCTTI,
    },
    {
       "OTN_ODU ais [<port_list>] [<si_ais>] [<so_ais>]",
       "Set or Get OTN ODU Forced AIS insertion",
       CLI_CMD_ID_OTN_ODU_AIS,
    },
    {
       "OTN_ODU Cons_Actions_bei [<port_list>] [<bei>]",
       "Set or Get OTN ODU Consequent Actions",
       CLI_CMD_ID_OTN_ODU_CONSACTS_BEI,
    },
    {
       "OTN_ODU Cons_Actions_bdi [<port_list>] [<odu_bdi>] [<bdilos>] [<bdilof>] [<bdilom>] [<bdiotuais>] [<bdiotutim>] [<bdiais>] [<bdioci>] [<bdilck>] [<bditim>]",
       "Set or Get OTN ODU Consequent Actions",
       CLI_CMD_ID_OTN_ODU_CONSACTS_BDI,
    },
    {
       "OTN_ODU Cons_Actions_ais [<port_list>] [<odu_ais>] [<ais_timocilck>] [<ais_plm>]",
       "Set or Get OTN ODU Consequent Actions",
       CLI_CMD_ID_OTN_ODU_CONSACTS_AIS,
    },
    {
       "OTN_ODU Payload_type [<port_list>] [<pl_plm>] [<pl_tx>] [<payload_type>]",
       "Set or Get OTN ODU Payload Type",
       CLI_CMD_ID_OTN_ODU_PAYLOAD_TYPE,
    },
    {
       "OTN_ODU Acc_Payload_type [<port_list>]",
       "Get OTN ODU Accepted Payload Type",
       CLI_CMD_ID_OTN_ODU_ACC_PAYLOAD_TYPE,
    },
    {
        "OTN_ODU aps [<port_list>] [enable|disable] [<aps>] [<aps>] [<aps>] [<aps>]",
        "Set/Get OTN ODU APS values",
        CLI_CMD_ID_OTN_ODU_APS,
    },
    {
        "OTN_ODU acc_aps [<port_list>]",
        "Get OTN ODU Accepted APS values",
        CLI_CMD_ID_OTN_ODU_ACCAPS,
    },
    {
        "OTN_ODU defects [<port_list>]",
        "Get OTN ODU Defects",
        CLI_CMD_ID_OTN_ODU_DEFECTS,
    },
    {
       "OTN_ODU performance [<port_list>]",
       "Get OTN ODU Perfomance",
       CLI_CMD_ID_OTN_ODU_PERF,
    },
    {
        "OTN_ODT mode <odut_level> [<port_list>] [termination|monitor]",
        "Set/Get OTN ODUT mode",
        CLI_CMD_ID_OTN_ODUT_MODE,
    },
    {
        "OTN_ODT tti <odut_level> <port> [enable|disable] [<otn_tti>] [sapi_tim_en|sapi_tim_dis] [dapi_tim_en|dapi_tim_dis] [os_tim_en|os_tim_dis] [<exp_tti>]",
        "Set or get OTN ODUT TTI",
        CLI_CMD_ID_OTN_ODUT_TTI,
    },
    {
       "OTN_ODT Acc_TTI <odut_level> [<port_list>]",
       "Get OTN ODUT Acc_TTI",
       CLI_CMD_ID_OTN_ODUT_ACCTTI,
    },
    {
       "OTN_ODT ca_beibiae <odut_level> [<port_list>] [<beibiae>]",
       "Set or Get OTN ODUT BEIBIAE Consequent Actions",
       CLI_CMD_ID_OTN_ODUT_CONSACTS_BEI,
    },
    {
       "OTN_ODT ca_bdi <odut_level> [<port_list>] [<bdi>] [<bdilos>] [<bdilof>] [<bdilom>] [<bdiotuais>] [<bdiotutim>] [<bdiais>] [<bdioci>] [<bdilck>] [<bdiltc>] [<bditim>]",
       "Set or Get OTN ODUT BDI Consequent Actions",
       CLI_CMD_ID_OTN_ODUT_CONSACTS_BDI,
    },
    {
       "OTN_ODT ca_ais <odut_level> [<port_list>] [<ais>] [<ais_timocilck>]",
       "Set or Get OTN ODUT AIS Consequent Actions",
       CLI_CMD_ID_OTN_ODUT_CONSACTS_AIS,
    },
    {
       "OTN_ODT ca_iae <odut_level> [<port_list>] [<iae>]",
       "Set or Get OTN ODUT IAE Consequent Actions",
       CLI_CMD_ID_OTN_ODUT_CONSACTS_IAE,
    },
    {
        "OTN_ODT bdi <odut_level> [<port_list>] [unforced_bdi|one_bdi|zero_bdi]",
        "Set/Get OTN ODUT BDI Configuration",
        CLI_CMD_ID_OTN_ODUT_BDI,
    },
    {
        "OTN_ODT stat <odut_level> [<port_list>] [enable|disable] [<stat>]",
        "Set/Get OTN ODUT STAT value",
        CLI_CMD_ID_OTN_ODUT_STAT,
    },
    {
        "OTN_ODT acc_stat <odut_level> [<port_list>]",
        "Get OTN ODUT Accepted STAT value",
        CLI_CMD_ID_OTN_ODUT_ACCSTAT,
    },
    {
        "OTN_ODT aps <odut_level> [<port_list>] [enable|disable] [<aps>] [<aps>] [<aps>] [<aps>]",
        "Set/Get OTN ODUT APS values",
        CLI_CMD_ID_OTN_ODUT_APS,
    },
    {
        "OTN_ODT acc_aps <odut_level> [<port_list>]",
        "Get OTN ODUT Accepted APS values",
        CLI_CMD_ID_OTN_ODUT_ACCAPS,
    },
    {
        "OTN_ODT defects <odut_level> [<port_list>]",
        "Get OTN ODU Defects",
        CLI_CMD_ID_OTN_ODUT_DEFECTS,
    },
    {
       "OTN_ODT performance <odut_level> [<port_list>]",
       "Get OTN ODUT Perfomance",
       CLI_CMD_ID_OTN_ODUT_PERF,
    },
#endif /* VTSS_FEATURE_OTN*/
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    {
        "1588 Block Init <port_no> [<clkfreq>] [<clk_src>] [<rx_ts_pos>] [<tx_fifo_mode>] [<modify_frm>]",
        "To initialize the 1588 Block",
        CLI_CMD_ID_1588_BLOCK_INIT,
    },
    {
        "1588 Engine Init <port_no> (ingress|egress) <engine_id> <encap_type> <flow_st_index> <flow_end_index> [strict|non-strict]",
        "To initialize the engine for a specific encapsulation type,\n"
        "number of flows, and flow match mode.",
        CLI_CMD_ID_1588_ENGINE_INIT,
    },
    {
        "1588 Engine Clear <port_no> (ingress|egress) <engine_id>",
        "To clear the initialized engine. (To re-use the engine with different encapsulation type or number of flows, engine must be cleared first)",
        CLI_CMD_ID_1588_ENGINE_CLEAR,
    },
    {
        "1588 Engine Mode <port_no> (ingress|egress) <engine_id> [enable|disable]",
        "Enable or Disable the engine. By default an initialised engine is disabled.",
        CLI_CMD_ID_1588_ENGINE_MODE,
    },
    {
        "1588 Engine Channel_Map <port_no> (ingress|egress) <engine_id> [<flow_id>] [<channel_map>]",
        "Channel association for each flow in an Engine. Engine configuration is shared by both the channels",
        CLI_CMD_ID_1588_ENGINE_CHANNEL_MAP,
    },
    {
        "1588 Engine Comm_Conf eth1 <port_no> (ingress|egress) <engine_id> [pbb_en|pbb_dis] [<etype>] [<tpid>]",
        "Configure the Ethernet 1 comparator common parameters (common for all 8 flows)",
        CLI_CMD_ID_1588_ENGINE_ETH1_COMM_CONF,
    },
    {
        "1588 Engine Flow_Conf eth1 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
        "       [enable|disable] [match_full|any_uc|any_mc] [<macaddr>]\n"
        "       [match_source|match_dest|match_src_dest] [vlan_chk_en|vlan_chk_dis] [<num_tag>]\n"
        "       [tag_rng_none|tag_rng_outer|tag_rng_inner] [<tag1_type>] [<tag2_type>]\n"
        "       [<tag1_lower>] [<tag1_upper>] [<tag2_lower>] [<tag2_upper>]",
        "Configure (flow parameters, enable/disable) the individual flows for \n"
        "Ethernet 1 comparator. (ETH1 classification parameters)",
        CLI_CMD_ID_1588_ENGINE_ETH1_FLOW_CONF,
    },
    {
        "1588 Engine Comm_Conf eth2 <port_no> (ingress|egress) <engine_id> [<etype>] [<tpid>]",
        "Configure the Ethernet 2 comparator common parameters (common for all 8 flows)",
        CLI_CMD_ID_1588_ENGINE_ETH2_COMM_CONF,
    },
    {
        "1588 Engine Flow_Conf eth2 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
        "       [enable|disable] [match_full|any_uc|any_mc] [<macaddr>]\n"
        "       [match_source|match_dest|match_src_dest] [vlan_chk_en|vlan_chk_dis] [<num_tag>]\n"
        "       [tag_rng_none|tag_rng_outer|tag_rng_inner] [<tag1_type>] [<tag2_type>]\n"
        "       [<tag1_lower>] [<tag1_upper>] [<tag2_lower>] [<tag2_upper>]",
        "Configure (flow parameters, enable/disable) the individual flows for \n"
        "Ethernet 2 comparator. (ETH2 classification parameters)",
        CLI_CMD_ID_1588_ENGINE_ETH2_FLOW_CONF,
    },
    {
        "1588 Engine Comm_Conf ip1 <port_no> (ingress|egress) <engine_id>\n"
        "       [ipv4|ipv6] [<sport_val>] [<sport_mask>] [<dport_val>] [<dport_mask>]",
        "Configure IP comparator 1 for the common part. (common for all 8 flows)\n"
        "The common part of flows includes IP version, Src port, dest port",
        CLI_CMD_ID_1588_ENGINE_IP1_COMM_CONF,
    },
    {
        "1588 Engine Flow_Conf ip1 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
        "       [enable|disable] [match_source|match_dest|match_src_dest]\n"
        "       [(ipv4 <ip_addr> <ip_mask>) | (ipv6 <ipv6_addr> <ipv6_mask>)]",
        "Configure IP comparator 1 individual flows of supported 8 flows.\n"
        "And enable or disable the configured flows.",
        CLI_CMD_ID_1588_ENGINE_IP1_FLOW_CONF,
    },
    {
        "1588 Engine Comm_Conf ip2 <port_no> (ingress|egress) <engine_id>\n"
        "       [ipv4|ipv6] [<sport_val>] [<sport_mask>] [<dport_val>] [<dport_mask>]",
        "Configure IP comparator 2 for the common part (common for all 8 flows)\n"
        "The common part of flows includes IP version, Src port, dest port",
        CLI_CMD_ID_1588_ENGINE_IP2_COMM_CONF,
    },
    {
        "1588 Engine Flow_Conf ip2 <port_no> (ingress|egress) <engine_id> <flow_id>\n"
        "       [enable|disable] [match_source|match_dest|match_src_dest]\n"
        "       [(ipv4 <ip_addr> <ip_mask>) | (ipv6 <ipv6_addr> <ipv6_mask>)]",
        "Configure IP comparator 2 individual flows of supported 8 flows.\n"
        "And enable or disable the configured flows.",
        CLI_CMD_ID_1588_ENGINE_IP2_FLOW_CONF,
    },
    {
        "1588 Engine Comm_Conf mpls <port_no> (ingress|egress) <engine_id> [cw_en|cw_dis]",
        "Configure the MPLS Comparator common part (common for supported 8 MPLS flows)",
        CLI_CMD_ID_1588_ENGINE_MPLS_COMM_CONF,
    },
    {
        "1588 Engine Flow_Conf mpls <port_no> (ingress|egress) <engine_id> <flow_id>\n"
        "       [enable|disable] [<stk_depth>] [stk_ref_top|stk_ref_bottom]\n"
        "       [<stk_lvl_0>] [<stk_lvl_1>] [<stk_lvl_2>] [<stk_lvl_3>]",
        "Configure the MPLS Comparator,the supported 8 MPLS flows individually.\n"
        "Enable or disable the configured flows",
        CLI_CMD_ID_1588_ENGINE_MPLS_FLOW_CONF,
    },
    {
        "1588 Engine Comm_Conf ACH <port_no> (ingress|egress) <engine_id>\n"
        "       [<ach_ver>] [<channel_type>] [<proto_id>]",
        "Configure the IP1/ACH Comparator, for common part of the supported 8 ACH flows",
        CLI_CMD_ID_1588_ENGINE_ACH_COMM_CONF,
    },
    {
        "1588 Engine Action Add <port_no> (ingress|egress) <engine_id>\n"
        "       <action_id> <channel_map>\n"
        "       [(ptp <clock_mode> <pdm> <domain_lower> <domain_upper>) |\n"
        "        (y1731_oam <version> <yodm> <meg_lower> <meg_upper>) |\n"
        "        (ietf_oam <version> <iodm> <tf> <tc>)]",
        "Configure the action for PTP/OAM comparator ( Action for identified PTP flows)",
        CLI_CMD_ID_1588_ENGINE_ACTION_ADD,
    },
    {
        "1588 Engine Action Delete <port_no> (ingress|egress) <engine_id> <action_id>",
        "Delete the actions configured.",
        CLI_CMD_ID_1588_ENGINE_ACTION_DELETE,
    },
    {
        "1588 Engine Action Show <port_no> (ingress|egress) <engine_id>",
        "Display the Actions configured for the engine",
        CLI_CMD_ID_1588_ENGINE_ACTION_SHOW,
    },
    {
        "1588 Signature <port_no> [<signature_mask>]",
        "Configure the signature mask for egress TX FIFO, to capture the tx timestamps\n"
        "used in two step synchronization",
        CLI_CMD_ID_1588_SIGNATURE_CONF,
    },
    {
        "1588 Stats [<time_sec>]",
        "PHY TS stats show",
        CLI_CMD_ID_1588_STATS_SHOW,
    },
    {
        "1588 Latency <port_no> (ingress|egress) [<latency_val>]",
        "Get/Set the ingress/egress latency for a port",
        CLI_CMD_ID_1588_LATENCY,
    },
    {
        "1588 Path_Delay <port_no> [<delay_val>]",
        "Get/Set the Path delay for a port",
        CLI_CMD_ID_1588_DELAY,
    },
    {
        "1588 Delay_Asym <port_no> [<asym_val>]",
        "Get/Set the Delay Asymmetry for a port",
        CLI_CMD_ID_1588_DELAY_ASYM,
    },
    {
        "1588 Read <port_list> <blk_id_1588> <csr_reg_offset> [binary|decimal]",
        "Read PHY 1588 register",
        CLI_CMD_ID_1588_MMD_READ,
    },
    {
        "1588 Write <port_list> <blk_id_1588> <csr_reg_offset> <value>",
        "Write PHY 1588 register",
        CLI_CMD_ID_1588_MMD_WRITE,
    },
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_FEATURE_GFP)
    {
        "GFP single_err_corr <port_list> [core_en|core_dis] [pyld_en|pyld_dis] [trans_en|trans_dis]",
        "Get or set GFP single error correction Configuration",
        CLI_CMD_ID_GFP_SINGLE_ERR_CORR,
    },
    {
        "GFP cmf_auto_sf_gen <port_list> [los_none|los_hw|los_on|los_off] [lsync_none|lsync_hw|lsync_on|lsync_off]",
        "Get or set GFP automatic CMF SF generation Configuration",
        CLI_CMD_ID_GFP_CMF_AUTO_SF_GEN,
    },
    {
        "GFP cmf_forced_gen <port_list> [<cmf_int>] [<cmf_fr_len>] [<upi>] [<pti>] [enable|disable]",
        "Get or set GFP forced CMF generation Configuration",
        CLI_CMD_ID_GFP_CMF_FORCED_GEN,
    },
    {
        "GFP cmf_acc_upi <port_list>",
        "Get of accepted CMF UPI",
        CLI_CMD_ID_GFP_CMF_ACC_UPI,
    },
    {
        "GFP cdf_fcs_insert <port_list> [enable|disable]",
        "Get or set GFP FCS insertion configuration",
        CLI_CMD_ID_GFP_CDF_FCS_INSERT,
    },
    {
        "GFP cdf_upi <port_list> [<upi>] [<ordered_upi>]",
        "Get or set GFP CDF transmitted and expected UPI configuration",
        CLI_CMD_ID_GFP_CDF_UPI,
    },
    {
        "GFP cdf_acc_upi <port_list>",
        "Get of accepted (not matching) CDF UPI",
        CLI_CMD_ID_GFP_CDF_ACC_UPI,
    },
    {
        "GFP 8b10b_control_code <port_list> [los_force_on|los_force_off] [lsync_force_on|lsync_force_off] [10b_err|k30_7]",
        "Get or set GFP 8B10B error control code configuration",
        CLI_CMD_ID_GFP_8B10B_CONTROL_CODE,
    },
    {
        "GFP defects [<port_list>]",
        "Get GFP Defects",
        CLI_CMD_ID_GFP_DEFECTS,
    },
    {
        "GFP performance [<port_list>]",
        "Get GFP Performance",
        CLI_CMD_ID_GFP_PERF,
    },
#endif /* VTSS_FEATURE_GFP */
#if defined(VTSS_FEATURE_WIS)
    {
        "WIS mode <port_list> [enable|disable]",
        "Get or Set  WIS Mode",
        CLI_CMD_ID_WIS_MODE
    },
    {
        "WIS stxti <port_list> [(<tti_mode> <tti>)]",
        "Set or get Section Tx tti ",
        CLI_CMD_ID_WIS_SECTION_TXTTI,
    },
    {
        "WIS ptxti <port_list> [(<tti_mode> <tti>)",
        "Set or get Path Tx tti ",
        CLI_CMD_ID_WIS_PATH_TXTTI,
    },
    {
        "WIS ptxsl [<port_list>] [<sl>]",
        "Set or get Path Tx SL ",
        CLI_CMD_ID_WIS_PATH_TXSL,
    },
    {
        "WIS status [<port_list>]",
        "Get WIS status ",
        CLI_CMD_ID_WIS_STATUS,
    },
    {
        "WIS cons_act [<port_list>] [ais_on_los|ais_on_lof|rdil_on_los|rdil_on_lof|rdil_on_ais] [enable|disable]",
        "Set WIS consequent action ",
        CLI_CMD_ID_WIS_CONS_ACT,
    },
    {
        "WIS defects [<port_list>]",
        "Get WIS Defects",
        CLI_CMD_ID_WIS_DEFECTS,
    },
    {
        "WIS sattiget [<port_list>]",
        "Get Section Rx Aaccepted TTI",
        CLI_CMD_ID_WIS_SECTION_ATTI_GET,
    },
    {
        "WIS pattiget [<port_list>]",
        "Get Path Rx Aaccepted TTI",
        CLI_CMD_ID_WIS_PATH_ATTI_GET,
    },
    {
        "WIS counters [<port_list>]",
        "Get WIS Error counters",
        CLI_CMD_ID_WIS_COUNTERS,
    },
    {
        "WIS tx_oh_conf [<port_list>] [(<oh_name> <oh_value>)]",
        "Set one tx overhead register or get all tx overhead bytes",
        CLI_CMD_ID_WIS_OH_CONF,
    },
    {
        "WIS perf_counters [<port_list>]",
        "Get Performance counters",
        CLI_CMD_ID_WIS_PREF_COUNTERS,
    },
    {
        "WIS passthru [<port_list>] [<pass_thru>]",
        "Set or Get wis overhead passthru configuration",
        CLI_CMD_ID_WIS_PASSTHRU,
    },
    {
        "WIS forced_insertion [<port_list>] [rx_force|tx_force] [ais_l|rdi_l] [enable|disable]",
        "Enable/disable forced insertion of AIS-L or RID-L",
        CLI_CMD_ID_WIS_FORCED_INSERTION,
    },
    {
        "WIS perf_mode [<port_list>] [pn_ebc_mode_s|pn_ebc_mode_l|pf_ebc_mode_l|pn_ebc_mode_p|pf_ebc_mode_p] [bit|block]",
        "Set or get WIS Performance Monitoring Counter Mode",
        CLI_CMD_ID_WIS_PERF_MODE,
    },
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
    {
        "WIS reset <port_list>",
        "Reset WIS block",
        CLI_CMD_ID_WIS_RESET
    },
    {
        "WIS test_status [<port_list>]",
        "Get WIS test status",
        CLI_CMD_ID_WIS_TEST_STATUS,
    },
    {
        "WIS test_mode [<port_list>] [loopback|no_loopback] [gen_dis|gen_sqr|gen_prbs31|gen_mix] [ana_dis|ana_prbs31|ana_mix]",
        "Set or get Test mode",
        CLI_CMD_ID_WIS_TEST_MODE,
    },
    {
        "WIS force_conf <port_list> [(<line_rx> <line_tx> <path_force>)]",
        "Set or get Wis force configuration",
        CLI_CMD_ID_WIS_FORCE_CONF,
    },
    {
        "WIS event_force [<port_list>] [<force_events>]",
        "Force events",
        CLI_CMD_ID_WIS_EVENT_FORCE,
    },
    {
        "WIS perf_thr <port_list> [(<n_ebc_thr_s> <n_ebc_thr_l> <f_ebc_thr_l> <n_ebc_thr_p> <f_ebc_thr_p>)]",
        "Set or get WIS Tx performance counter threshold configuration",
        CLI_CMD_ID_WIS_TX_PREF_THRE,
    },
    {
        "WIS prbs31_err_inj [<port_list>] [single_err|sat_err]",
        "Inject prbs31 errors",
        CLI_CMD_ID_WIS_PRBS_ERR_INJ,
    },
#endif    
    
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_UPI)
    {
        "UPI config [<port_list>] [equip_loopback|line_loopback|no_loopback]",
        "Set or get UPI configuration",
        CLI_CMD_ID_UPI_CONFIG,
    },
    {
        "UPI status [<port_list>]",
        "Get UPI status",
        CLI_CMD_ID_UPI_STATUS,
    },
    {
        "UPI test config [<port_list>] [<gen_mode>] [<chk_mode>] [<wrp_mode>]",
        "Set or get UPI test configuration",
        CLI_CMD_ID_UPI_TEST_CONFIG,
    },
    {
        "UPI test status [<port_list>]",
        "Get UPI test status",
        CLI_CMD_ID_UPI_TEST_STATUS,
    },
    {
        "UPI tx_eq mode <port_list> [<upi_txeq_mode>] [<upi_txeq_chnloss>]",
        "Set/Get UPI Transmit equalization mode",
        CLI_CMD_ID_UPI_TXEQ_MODE,
    },
    {
        "UPI tx_eq coef_update <port_list> [<upi_coef_id>] [<upi_coef_step>] [<upi_incr>] [<upi_polarity>]",
        "Set/Get UPI co-efficients",
        CLI_CMD_ID_UPI_TXEQ_COEF,
    },
    {
        "UPI tx_eq power [<port_list>] [<upi_power>]",
        "Set/get UPI Power amplitude",
        CLI_CMD_ID_UPI_TXEQ_AMP,
    },
    {
        "UPI tx_eq slew [<port_list>] [<upi_slew>]",
        "Set/Get UPI Slew rate",
        CLI_CMD_ID_UPI_TXEQ_SLEW,
    },	
    {
        "UPI rx_eq mode [<port_list>] [<upi_rxeq_mode>]",
        "Set/Get UPI Rx Eq Mode",
        CLI_CMD_ID_UPI_RXEQ_MODE,
    },
#endif /* VTSS_FEATURE_UPI */
    #if defined(VTSS_FEATURE_XAUI)
    {
        "XAUI config [<port_list>] [equip_loopback|line_loopback|no_loopback]",
        "Set or get XAUI configuration",
        CLI_CMD_ID_XAUI_CONFIG,
    },
    {
        "XAUI status [<port_list>]",
        "Get XAUI status",
        CLI_CMD_ID_XAUI_STATUS,
    },
    {
        "XAUI statistics [<port_list>] [clear]",
        "Get XAUI statistics, i.e error counters and byte counters",
        CLI_CMD_ID_XAUI_STATS,
    },
    #endif /* VTSS_FEATURE_XAUI */
#if defined(VTSS_FEATURE_XFI)
    {
        "XFI config [<port_list>] [a_lane_xfi|b_lane_xfi] [equip_loopback|line_loopback|no_loopback]",
        "Set or get XFI configuration",
        CLI_CMD_ID_XFI_CONFIG,
    },
    {
        "XFI status [<port_list>]",
        "Get XFI status",
        CLI_CMD_ID_XFI_STATUS,
    },
    {
        "XFI test config [<port_list>] [<gen_mode>] [<chk_mode>]",
        "Set or get XFI test configuration",
        CLI_CMD_ID_XFI_TEST_CONFIG,
    },
    {
        "XFI test status [<port_list>]",
        "Get XFI test status",
        CLI_CMD_ID_XFI_TEST_STATUS,
    },
    {
        "XFI rec_clock [<port_list>] [lane_a|lane_b] [enable|disable]",
        "Set/Get XFI recovery clock output status",
        CLI_CMD_ID_XFI_REC_CLK_STATUS,
    },
    {
        "XFI tx_eq_mode <port_list> [<txeq_mode>] [<txeq_channel_loss>]",
        "Set/Get XFI Transmit equalization mode",
        CLI_CMD_ID_XFI_TXEQ_MODE,
    },
    {
        "XFI tx_misc_parm [<port_list>] [slew|power] [<xfi_misc_parm_value>]",
        "Set/Get XFI Miscellaneous Parameters slew_rate and power",
        CLI_CMD_ID_XFI_TXMISC_PARM,
    },
    {
        "XFI rx_eq_mode [<port_list>] [<xfi_rxeq_mode>]",
        "Set/Get XFI RX_EQ Mode",
        CLI_CMD_ID_XFI_RXEQ_MODE,
    },
    {
        "XFI tx_eq_coef [<port_list>] [<coef>] [<step_size>] [<incr>] [<polarity>]",
        "Update/Get XFI Transmit equalization coefficient",
        CLI_CMD_ID_XFI_TXEQ_COEFUPD,
    },
    {
        "XFI tx_eq_802a_coef <port_list> [<lane>] [<cof_802a>] [<upd_req>]",
        "Update/Get XFI 802.3ap Transmit equalization coefficient",
        CLI_CMD_ID_XFI_802a_TXEQ_COEUPD,
    },   
    {
        "XFI tx_eq_802a_fsm <port_list> <lane> [<xfi_fsm_ctl>]",
        "Update/Get XFI 802.3ap FSM control register",
        CLI_CMD_ID_XFI_802a_TXEQ_FSM,
    },
#endif /* VTSS_FEATURE_XFI */
#if defined(VTSS_FEATURE_MAC10G)
    {
        "MAC10G Loopback [<port_list>] [tx_rx_loopback|rx_tx_loopback|no_loopback]",
        "Set or get MAC10G Loopback configuration",
        CLI_CMD_ID_MAC10G_LB_CONFIG,
    },
    {
        "MAC10G TxFlowCtrl [<port_list>] [enable|disable] [<mac_addr>]",
        "Set or get MAC10G Tx Flow control configuration",
        CLI_CMD_ID_MAC10G_TX_FLOW_CTRL_CONFIG,
    },
    {
        "MAC10G RxFlowCtrl RxDrop [<port_list>] [enable|disable]",
        "Set or get MAC10G Rx Pause Frames Drop configuration",
        CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_DROP_CONFIG,
    },
    {
        "MAC10G RxFlowCtrl RxAction [<port_list>] [enable|disable]",
        "Set or get MAC10G Rx Flow control Action configuration",
        CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_ACTION_CONFIG,
    },
    {
        "MAC10G FrameLen [<port_list>] [<max_frame_len>] [<max_tags>]",
        "Set or get MAC10G Frame configuration",
        CLI_CMD_ID_MAC10G_FRAME_LEN_CONFIG,
    },
    {
        "MAC10G Preamble Check [<port_list>] [enable|disable]",
        "Set or get Preamble Check configuration [Strict Preamble Check (or) disable Preamble Check]",
        CLI_CMD_ID_MAC10G_PREAMBLE_CHECK_CONFIG,
    },
    {
        "MAC10G LF [<port_list>] [enable|disable]",
        "Set or get Local Fault configuration ",
        CLI_CMD_ID_MAC10G_LF_CONFIG,
    },
    {
        "MAC10G RF [<port_list>] [enable|disable]",
        "Set or get Remote Fault configuration ",
        CLI_CMD_ID_MAC10G_RF_CONFIG,
    },
    {
        "MAC10G Status [<port_list>]",
        "Get MAC10G statistics",
        CLI_CMD_ID_MAC10G_STATUS,
    },
    {
        "MAC10G RMON statistics [<port_list>] [clear]",
        "Get MAC10G RMON statistics",
        CLI_CMD_ID_MAC10G_RMON_STATS,
    },
#endif /* VTSS_FEATURE_MAC10G */
#if defined(VTSS_FEATURE_PCS_10GBASE_R)
    {
        "PCS10G_r Loopback [<port_list>] [enable|disable]",
        "Set or get PCS10G Base R Loopback configuration",
        CLI_CMD_ID_PCS_10GBASE_R_LB_CONFIG,
    },
    {
        "PCS10G_r Status [<port_list>]",
        "Set or get PCS10G Base R Status",
        CLI_CMD_ID_PCS_10GBASE_R_STATUS,
    },
    {
        "PCS10G_r Statistics [<port_list>] [clear]",
        "Set or get PCS10G Base R Statistics",
        CLI_CMD_ID_PCS_10GBASE_R_STATS,
    },
#endif /* VTSS_FEATURE_PCS_10GBASE_R*/
#if defined(VTSS_FEATURE_RAB)
    {
        "RAB Rx_FIFO_Threshold [<port_list>] [<thresh_high>] [<thresh_low>] [<thresh_adapt>]",
        "Set or get RAB Rx Thresholds configuration",
        CLI_CMD_ID_RAB_RX_THRESH_CONFIG,
    },
    {
        "RAB Tx_FIFO_Threshold [<port_list>] [<thresh_high>] [<thresh_low>] [<thresh_adapt>]",
        "Set or get RAB Tx Thresholds configuration",
        CLI_CMD_ID_RAB_TX_THRESH_CONFIG,
    },
    {
        "RAB Tx_Cut_Thru [<port_list>] [enable|disable]",
        "Set or get Tx Cut_Thru configuration",
        CLI_CMD_ID_RAB_CUT_THRU_TX,
    },
    {
        "RAB Rx_Cut_Thru [<port_list>] [enable|disable]",
        "Set or get Rx Cut_Thru configuration",
        CLI_CMD_ID_RAB_CUT_THRU_RX,
    },
    {
        "RAB Statistics [<port_list>]",
        "Display the RAB Statistics",
        CLI_CMD_ID_RAB_STATS,
    },
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_I2C)
    {
        "I2C Init <controller_no>",
        "Initialize the controller",
        CLI_CMD_ID_I2C_INIT,
    },
    {
        "I2C Rx <controller_no> <dev_addr> <reg_addr>",
        "Read from the specified device register",
        CLI_CMD_ID_I2C_RX,
    },
    {
        "I2C Tx <controller_no> <dev_addr> <reg_addr> <data>",
        "Write into the specified device register",
        CLI_CMD_ID_I2C_TX,
    },
#endif /* VTSS_FEATURE_I2C */
    {
        "Event configure <port> [upi|xfi|otn|ewis|pcs|mac|gfp|i1588|efeci4|efeci7] <event> [enable|disable]",
        "Enable or disable an event",
        CLI_CMD_ID_DEBUG_EVENT_CONFIG,
    },
    {
        "Event screen [enable|disable]",
        "Enable or disable of seperate Event screen",
        CLI_CMD_ID_DEBUG_EVENT_SCREEN,
    },
    {
        "Event clear",
        "Clear Event screen",
        CLI_CMD_ID_DEBUG_EVENT_CLEAR,
    },
    {
        "Debug Interrupt Status [<port_list>]",
        "Show interrupt information",
        CLI_CMD_ID_DEBUG_INTERRUPT_WITHOUT_MASK,
    },
    {
        "PMTICK <port_list> [disabled|internal|external] <interval>",
        "Enable or Disable the PMTICK configuration; \n"
        "<interval> is used only when PMTICK selection is internal",
        CLI_CMD_ID_PMTICK_CONFIG,
    },
    {
        "channel_mode <channel_num> <channel_mode> <xaui_mode> <ev_init>",
        "Set the operating mode and XAUI mode both channels\n",
        CLI_CMD_ID_CHANNEL_MODE_CONFIG,
    },
#endif /* VTSS_ARCH_DAYTONA */
#if defined (VTSS_FEATURE_OHA)
    {
        "oha_mux_conf <port_list> [<rosi_tosi>] [<otn1_otn2>]",
        "Set Data MUX for the TOSI/ROSI and OTN1/OTN2 and Enable OH Add/Drop for EWIS2",
        CLI_CMD_ID_OHA_CONF,
    },
#endif  /* VTSS_FEATURE_OHA */
#if defined(NPI_INTERFACE)
    {
        "NPI port [<port>|disable]",
        "Set NPI port",
        CLI_CMD_ID_NPI_PORT
    },
    {
        "transmit NPI <port_list>",
        "Transmit packet on port set - via NPI port",
        CLI_CMD_ID_TRANSMIT_NPI
    },
#endif  /* VTSS_FEATURE_NPI */
#if defined(VTSS_FEATURE_PACKET)
    {
        "transmit port <port_list>",
        "Transmit packet on port set - using vtss_packet_tx_frame() ",
        CLI_CMD_ID_TRANSMIT_PORT
    },
    {
        "transmit vlan <vid>",
        "Transmit packet on vlan - using vtss_packet_tx_frame() ",
        CLI_CMD_ID_TRANSMIT_VLAN
    },
#endif  /* VTSS_FEATURE_NPI */
}; /* cli_cmd_table */


static cli_parm_t cli_parm_table[] = {
#ifdef VTSS_ARCH_DAYTONA
    {
        "<event>",
        "Event identification as in register spec.\n"
        "upi block:\n"
        "   rx_hss_prbssync_0\n"
        "   rx_hss_prbssync_1\n"
        "   rx_hss_prbssync_2\n"
        "   rx_hss_prbssync_3\n"
        "   tx_hss_prbssync_0\n"
        "   tx_hss_prbssync_1\n"
        "   tx_hss_prbssync_2\n"
        "   tx_hss_prbssync_3\n"
        "   hss_resetout\n"
        "   rx_los\n"
        "   rx_sigdet_0\n"
        "   rx_sigdet_1\n"
        "   rx_sigdet_2\n"
        "   rx_sigdet_3\n"
        "   tx_fifo_overflow\n"
        "   tx_fifo_underflow\n"
        "   hss_prtready\n"
        "   rx_prbs_err_0\n"
        "   rx_prbs_err_1\n"
        "   rx_prbs_err_2\n"
        "   rx_prbs_err_3\n"
        "   tx_prbs_err_0\n"
        "   tx_prbs_err_1\n"
        "   tx_prbs_err_2\n"
        "   tx_prbs_err_3\n"
        "   ssf\n"
        "   plllock\n"
        "   eye_quality_err\n"
        "   fas_pos\n"
        "   lom\n"
        "   oom\n"
        "   lof\n"
        "   oof\n"
        "   all\n"
        "\nxfi block:\n"
        "   hssplllock\n"
        "   hssresetout\n"
        "   hsseyequality\n"
        "   hssprtready\n"
        "   rxprbssync\n"
        "   rxprbserr\n"
        "   rxasigdet\n"
        "   rxa_los\n"
        "   ifpll_lock\n"
        "   tx_fifo_los\n"
        "   rxbsigdet\n"
        "   rxb_los\n"
        "   aet_run_b\n"
        "   aet_frm_lck_b\n"
        "   pcs_rx_ready_b\n"
        "   pcs_tx_ready_b\n"
        "   fec_negotiated_b\n"
        "   train_ok_b\n"
        "   train_fail_b\n"
        "   train_local_b\n"
        "   aet_run_a\n"
        "   aet_frm_lck_a\n"
        "   pcs_rx_ready_a\n"
        "   pcs_tx_ready_a\n"
        "   fec_negotiated_a\n"
        "   train_ok_a\n"
        "   train_fail_a\n"
        "   train_local_a\n"
        "   fas_pos\n"
        "   lom\n"
        "   oom\n"
        "   lof\n"
        "   oof\n"
        "   incompatible_link\n"
        "   par_detect_fault\n"
        "   an_good_check\n"
        "   page_rx\n"
        "   aneg_complete\n"
        "   all\n"
        "\notn block:\n"
        "   och_los\n"
        "   och_lof\n"
        "   och_lom\n"
        "   och_ais\n"
        "   och_ber_exceeded\n"
        "   och_uncorrect_code\n"
        "   och_all\n"
        "   otu_tim\n"
        "   otu_acc_tti\n"
        "   otu_iconsist_tti\n"
        "   otu_acc_res\n"
        "   otu_inconsist_res\n"
        "   otu_acc_smres\n"
        "   otu_inconsist_smres\n"
        "   otu_acc_gcc0\n"
        "   otu_inconsist_gcc0\n"
        "   otu_bdi\n"
        "   otu_iae\n"
        "   otu_biae\n"
        "   otu_all\n"
        "   odu_tim\n"
        "   odu_acc_tti\n"
        "   odu_iconsist_tti\n"
        "   odu_plm\n"
        "   odu_acc_pt\n"
        "   odu_iconsist_pt\n"
        "   odu_acc_exp\n"
        "   odu_iconsist_exp\n"
        "   odu_acc_res\n"
        "   odu_iconsist_res\n"
        "   odu_acc_ftfl\n"
        "   odu_iconsist_ftfl\n"
        "   odu_acc_aps\n"
        "   odu_iconsist_aps\n"
        "   odu_oci\n"
        "   odu_lck\n"
        "   odu_bdi\n"
        "   odu_ais\n"
        "   odu_all\n"
        "   odut_tim\n"
        "   odut_acc_tti\n"
        "   odut_iconsist_tti\n"
        "   odut_acc_stat\n"
        "   odut_iconsist_stat\n"
        "   odut_acc_aps\n"
        "   odut_iconsist_aps\n"
        "   odut_oci\n"
        "   odut_lck\n"
        "   odut_bdi\n"
        "   odut_ais\n"
        "   odut_ltc\n"
        "   odut_iae\n"
        "   odut_biae\n"
        "   odut_all\n"
        "   all\n"
        "\ngfp block:\n"
        "   ssf\n"
        "   8b10b_los\n"
        "   8b10b_lsync\n"
        "   8b10b_error\n"
        "   8b10b_rx_illegal\n"
        "   8b10b_rx_disparity\n"
        "   8b10b_tx_illegal\n"
        "   8b10b_tx_disparity\n"
        "   cmf_los_sf\n"
        "   cmf_lsync_sf\n"
        "   cmf_unsupported\n"
        "   cmf_inconsistent\n"
        "   frame_mapped_eth\n"
        "   cmf_accepted\n"
        "   cdf_upm\n"
        "   ppos_cdf_upm\n"
        "   all\n"
        "\newis block:\n"
        "   los\n"
        "   oof\n"
        "   lof\n"
        "   rdil\n"
        "   aisl\n"
        "   s1\n"
        "   lop\n"
        "   aisp\n"
        "   plmp\n"
        "   uneqp\n"
        "   rdip\n"
        "   fe_ais_p\n"
        "   feplmp\n"
        "   fe_uneqp\n"
        "   all\n"
        "\npcs block:\n"
        "   rx_hi_ber\n"
        "   rx_fifo_oflow\n"
        "   rx_fifo_uflow\n"
        "   lock_changed\n"
        "   c64b66b_err\n"
        "   rx_oset\n"
        "   rx_oset_fifo_full\n"
        "   tx_fifo_oflow\n"
        "   tx_fifo_uflow\n"
        "   xgmii_err\n"
        "   rx_fset\n"
        "   rx_fset_fifo_full\n"
        "   tx_fifo_ecc_1bit\n"
        "   tx_fifo_ecc_2bit\n"
        "   rx_fifo_ecc_1bit\n"
        "   rx_fifo_ecc_2bit\n"
        "   all\n"
        "\nmac block:\n"
        "   link_delta\n"
        "   remote_fault_state_delta\n"
        "   local_fault_state_delta\n"
        "   tx_abort\n"
        "   rx_non_std_pream\n"
        "   rx_pream_err\n"
        "   rx_pream_mismatch\n"
        "   rx_pream_shrink\n"
        "   tx_ifg_column_drop\n"
        "   all\n"
        "\n1588 block:\n"
        "   all\n"
        "\ni4 fec block:\n"
        "   i4_ed_uccw\n"
        "   i4_rs_uccw\n"
        "   i4_bch_uccw\n"
        "   all\n"
        "\ni7 fec block:\n"
        "   i7_ber_alarm\n"
        "   all",  
        CLI_PARM_TYPE_EVENT,
        CLI_CMD_ID_DEBUG_EVENT_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "upi|xfi|otn|ewis|pcs|mac|gfp|i1588|efeci4|efeci7",
        "Daytona Block Identifier",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_DEBUG_EVENT_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<port>",
        "port number",
        CLI_PARM_TYPE_PORT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "ais_on_los|ais_on_lof|rdil_on_los|rdil_on_lof|rdil_on_ais",
        "",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_WIS_CONS_ACT,
        CLI_PARM_FLAG_SET
    },
    {
        "rx_force|tx_force",
        "WIS Block force direction of AIS_L/RDI_L",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_WIS_FORCED_INSERTION,
        CLI_PARM_FLAG_NONE
    },
    {
        "ais_l|rdi_l",
        "WIS block force selection",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_WIS_FORCED_INSERTION,
        CLI_PARM_FLAG_NONE
    },
    {
        "pn_ebc_mode_s|pn_ebc_mode_l|pf_ebc_mode_l|pn_ebc_mode_p|pf_ebc_mode_p",
        "\n"
        "pn_ebc_mode_s:   Section BIP error count (B1)\n"
        "pn_ebc_mode_l:   Near end line block (BIP) error count (B2) \n"
        "pf_ebc_mode_l:   Far end line block (BIP) error count (REI-L) \n" 
        "pn_ebc_mode_p:   Path block error count (B3)\n"
        "pf_ebc_mode_p:   Far end path block error count (REI-P)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_WIS_PERF_MODE,
        CLI_PARM_FLAG_SET
    },
    {
        "bit|block",
        "\n"
        "bit:   Counter is set to Bit Mode"
        "block: Counter is set to Block Mode",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_WIS_PERF_MODE,
        CLI_PARM_FLAG_SET
    },
    {
        "disabled|internal|external",
        "PMTICK selection",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_PMTICK_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<interval>",
        "PMTICK interval",
        CLI_PARM_TYPE_PMTICK_INTERVAL,
        CLI_CMD_ID_PMTICK_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<channel_num>",
        "Channel number (0 or 1)",
        CLI_PARM_TYPE_CHANNEL_NUM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<channel_mode>",
        "Channel mode",
        CLI_PARM_TYPE_CHANNEL_MODE,
        CLI_CMD_ID_CHANNEL_MODE_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<xaui_mode>",
        "XAUI mode for channel , 0-SDR, 1-DDR_A, 2-DDR_K, 3-DDR_M\n",
        CLI_PARM_TYPE_CHANNEL_XAUIMODE,
        CLI_CMD_ID_CHANNEL_MODE_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<ev_init>",
        "Enable/Disable Validation board initialization 0-Disable, 1-Enable\n",
        CLI_PARM_TYPE_CHANNEL_CONFIG_EVINIT,
        CLI_CMD_ID_CHANNEL_MODE_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<rosi_tosi>",
        "Mux Selection for ROSI/TOSI interface: 1 - eWIS2, 2 - OTN1",
        CLI_PARM_TYPE_OHA_ROSITOSI,
        CLI_CMD_ID_OHA_CONF,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn1_otn2>",
        "Mux Selection for OTN1_OTN2 interface: 1 - OTN1, 2 - OTN2",
        CLI_PARM_TYPE_OHA_OTN1_OTN2,
        CLI_CMD_ID_OHA_CONF,
        CLI_PARM_FLAG_SET
    },
#endif/* VTSS_ARCH_DAYTONA */
#if (VTSS_APPL_INST_CNT > 1)
    {
        "<inst>",
        "System instance, 0 or 1",
        CLI_PARM_TYPE_SYSTEM_INST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /* VTSS_APPL_INST_CNT > 1 */
    {
        "<port_list>",
        "Port list, default: All ports",
        CLI_PARM_TYPE_PORT_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<port>|disable",
        "Port number or 'disable'",
        CLI_PARM_TYPE_PORT_DIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "enable|disable",
        "enable     : Enable\n"
        "disable    : Disable\n"
        "(default: Show mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "allow|deny",
        "allow   : Allow\n"
        "deny    : Deny\n"
        "(default: Show mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "10hdx|10fdx|100hdx|100fdx|1000fdx|auto",
        "10hdx      : 10 Mbps, half duplex\n"
        "10fdx      : 10 Mbps, full duplex\n"
        "100hdx     : 100 Mbps, half duplex\n"
        "100fdx     : 100 Mbps, full duplex\n"
        "1000fdx    : 1 Gbps, full duplex\n"
        "auto       : Auto negotiation of speed and duplex\n"
        "(default: Show configured and current mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "clear|packets|bytes|errors|discards",
        "clear      : Clear port statistics\n"
        "packets    : Show packet statistics\n"
        "bytes      : Show byte statistics\n"
        "errors     : Show error statistics\n"
        "discards   : Show discard statistics\n"
        "(default: Show all port statistics)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT
    },
    {
        "clear",
        "Clear sticky bits",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_DEBUG_REG_CHECK,
        CLI_PARM_FLAG_NONE
    },
    {
        "clear",
        "Clear counters",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<max_frame>",
        "Port maximum frame size, default: Show maximum frame size",
        CLI_PARM_TYPE_MAX_FRAME,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET
    },
    {
        "<mac_addr>",
        "MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_TYPE_MAC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<vid>",
        "VLAN ID",
        CLI_PARM_TYPE_VID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<age_time>",
        "MAC address age time (10-1000000), default: Show age time",
        CLI_PARM_TYPE_MAC_AGE_TIME,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "warm|cool",
        "warm: Warm restart\n"
        "cool: Cool restart\n"
        "(default: Show restart mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "<etype>",
        "Etype value",
        CLI_PARM_TYPE_ETYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#if defined(VTSS_ARCH_B2)
    {
        "<psi_clock>",
        "Port Status Interface clock (0-10000000) where 0 is disable clock, default: Show PSI clock.",
        CLI_PARM_TYPE_PSI_CLOCK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "enable|disable|ingress|egress",
        "enable     : Enable forwarding\n"
        "disable    : Disable forwarding\n"
        "ingress    : Enable ingress forwarding only\n"
        "egress     : Enable egress forwarding only",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "enable|disable",
        "enable     : Enable trusted DSCP\n"
        "disable    : Disable trusted DSCP",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_QOS_GLOBAL_DSCP,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "<lport_list>",
        "Logical port list, default: All ports (0-47)",
        CLI_PARM_TYPE_LPORT_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<lport>|disable",
        "Logical port (0-47) or 'disable'",
        CLI_PARM_TYPE_LPORT_DIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<clk_freq>",
        "\n1=25Mhz     (1G)\n"
        "2=31.25Mhz  (1G/10G)\n"
        "3=39.06Mhz  (10G)\n"
        "4=62.5Mhz   (1G)\n"
        "5=78.125Mhz (10G)\n"
        "6=125Mhz    (1G)\n"
        "7=156.25Mhz (10G)\n"
        ,
        CLI_PARM_TYPE_CLK_FREQ,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<table_id>",
        "Table 1 or 2",
        CLI_PARM_TYPE_TABLE_ID,
        CLI_CMD_ID_QOS_DSCP_TABLE,
        CLI_PARM_FLAG_SET
    },
    {
        "<table_id>",
        "Table 1 or 2",
        CLI_PARM_TYPE_TABLE_ID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<dscp_list>",
        "DSCP list, 0-63",
        CLI_PARM_TYPE_DSCP_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<ct_value>",
        "Cut through threshold, 1-0x1fff, 0=disable",
        CLI_PARM_TYPE_CT_THRESHOLD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<table_index>",
        "Index in table 1-8",
        CLI_PARM_TYPE_TABLE_INDEX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<q_index>",
        "queue 0-7",
        CLI_PARM_TYPE_QUEUE_INDEX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<pct>",
        "Percentage 0-100%",
        CLI_PARM_TYPE_PCT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ip_proto>",
        "IP protocol number 0-255",
        CLI_PARM_TYPE_IP_PROTO,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<trig_index>",
        "Trigger id 0-6",
        CLI_PARM_TYPE_TRIG_INDEX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<tcp_udp_port>",
        "TCP/UDP port id 0-0xFFFF",
        CLI_PARM_TYPE_TCP_UDP_PORT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag_prio>",
        "User priority / Tag priority 0-7",
        CLI_PARM_TYPE_TAG_PRIO,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<mpls_exp>",
        "MPLS experimental 0-7",
        CLI_PARM_TYPE_MPLS_EXP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "mc|bc|uc|all",
        "mc|bc|uc|all: Traffic type to police",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "<order_rank>",
        "Class order rank (0-16)",
        CLI_PARM_TYPE_QOS_ORDER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<endpoint_id>",
        "Endpoint ID (0-16)\n"
        "0:  Default\n"
        "1:  L2 control frames\n"
        "2:  L3 control frames\n"
        "3:  Tag CFI\n"
        "4:  Trusted DSCP values\n"
        "5:  IPv4/IPv6 TCP/UDP port\n"
        "6:  IPv4 prot or IPv6 next header\n"
        "7:  All DSCP values\n"
        "8:  IPv4 and ARP frames\n"
        "9:  IPv6 frames\n"
        "10: Ethernet Type\n"
        "11: MPLS exp bits\n"
        "12: VLAN ID\n"
        "13: VLAN Tag Priority\n"
        "14: LLC SNAP\n"
        "15: Global custom filter\n"
        "16: Local custom filter\n",
        CLI_PARM_TYPE_QOS_ENDPOINT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "default|l2_ctrl|l3_ctrl|cfi|ipv4|ipv6|llc",
        "\ndefault      : Default endpoint\n"
        "l2_ctrl      : L2 Ctrl endpoint\n"
        "l3_ctrl      : L3 Ctrl endpoint\n"
        "cfi          : TAG CFI endpoint\n"
        "ipv4         : IPv4 and ARP endpoint\n"
        "ipv6         : IPv6 endpoint\n"
        "llc          : LLC SNAP endpoint",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "ctag|cfi|vlan_aware|tag_inner",
        "\nctag           : Stop analyses if C-tag is found\n"
        "cfi              : Stop analyses if C-tag CFI is found\n"
        "vlan-aware       : Enable VLAN awareness\n"
        "tag_inner        : Use inner tag if present\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },

    {
        "range|specific",
        "Range of ports / Specific port\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "l2|l3|l4",
        "Layer2 / Layer3 / Layer4 position\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "a|b|c|d|e|f|g|h|j|k",
        "10G Phy Loopbacks system/network\n"
        "b: XAUI -> XS -> XAUI - shallow\n"
        "c: XAUI -> XS -> XAUI - deep\n"
        "e: XAUI -> PCS FIFO -> XAUI\n"
        "g: XAUI -> PCS -> XAUI\n"
        "j: XAUI -> PMA -> XAUI \n"
        "d: XFI  -> XS  -> XFI - shallow\n"
        "a: XFI  -> XS  -> XFI - deep\n"
        "f: XFI  -> PCS -> XFI\n"
        "h: XFI  -> WIS -> XFI\n"
        "k: XFI  -> PMA -> XFI",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<offset>",
        "Offset in bytes the given Layer, must be even, within the first 104 bytes in the frame.",
        CLI_PARM_TYPE_OFFSET_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<pattern>",
        "Pattern to match, 2 bytes",
        CLI_PARM_TYPE_PATTERN_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<mask>",
        "Mask, 0-0xFFFF",
        CLI_PARM_TYPE_MASK_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },        
    {
        "drop|forward",
        "\ndrop        : Drop the frame if matched\n"
        "forward     : Forward the frame if matched\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<dmac>",
        "Destination MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_TYPE_DMAC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<dmac_mask>",
        "Mask for MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_TYPE_DMAC_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<red_min>",
        "Red min queue fill level in bytes, 0-32640",
        CLI_PARM_TYPE_RED_MIN,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<red_max>",
        "Red max queue fill level in bytes, 0-65280",
        CLI_PARM_TYPE_RED_MAX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<red_weight>",
        "Red weight level (0-31) for avg queue level calc, 9 is typically used.",
        CLI_PARM_TYPE_RED_WEIGHT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<prob_1>",
        "Red probability profile 1 value, 0-100%",
        CLI_PARM_TYPE_RED_PROB_1,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<prob_2>",
        "Red probability profile 2 value, 0-100%",
        CLI_PARM_TYPE_RED_PROB_2,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<prob_3>",
        "Red probability profile 3 value, 0-100%",
        CLI_PARM_TYPE_RED_PROB_3,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<class>",
        "Class 0-7",
        CLI_PARM_TYPE_CLASS_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "prim|alt",
        "Primary / Alternate endpoint table\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "rx|tx",
        "Receive / Transmit direction, as seen from the SPI4 interface\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bit_rate>",
        "Rate in 1000 bits per second. 0 to disable",
        CLI_PARM_TYPE_QOS_BIT_RATE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<level>",
        "Max bucket depth in bytes, 1-256KB",
        CLI_PARM_TYPE_QOS_LEVEL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<red>",
        "Random Early Drop 0-3, 0=disable",
        CLI_PARM_TYPE_RED_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "mac_ctrl|mac_zero|dmac_bc|smac_mc|untag|prio_tag|c_tag|s_tag|tags",
        "mac_ctrl   : Filter mac ctrl frames\n"
        "mac_zero   : Filter frames with zero mac address\n"
        "dmac_bc    : Filter frames with BC DMAC\n"
        "smac_mc    : Filter frames with MC SMAC\n"
        "untag      : Filter untagged frames\n"
        "prio_tag   : Filter priority tagged frames\n"
        "c_tag      : Filter C-tagged frames\n"
        "s_tag      : Filter S-tagged frames\n"
        "(default: Show configured and current mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "any_tag|1_tag|2_tags",
        "any_tag : Any number of tags allowed\n"
        "1_tag   : Max 1 tag allowed\n"
        "2_tags  : Max 2 tags allowed\n"
        "(default: Show configured and current mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "<mmd_address>",
        "MMD register address (0-0xffff)",
        CLI_PARM_TYPE_MMD_ADDR,
        CLI_CMD_ID_DEBUG_MMD_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<mmd_address>",
        "MMD register address (0-0xffff)",
        CLI_PARM_TYPE_MMD_ADDR,
        CLI_CMD_ID_DEBUG_MMD_READ,
        CLI_PARM_FLAG_NONE
    },
#endif /* VTSS_ARCH_B2 */
    {
        "<addr_list>",
        "Register address list (0-255)",
        CLI_PARM_TYPE_FPGA_ADDR_LIST,
        CLI_CMD_ID_DEBUG_FPGA_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<addr_list>",
        "Register address list (0-255)",
        CLI_PARM_TYPE_FPGA_ADDR_LIST,
        CLI_CMD_ID_DEBUG_FPGA_WRITE,
        CLI_PARM_FLAG_NONE
    },
#if defined(VTSS_ARCH_LUTON28)
    {
        "<blk_list>",
        "Block list (0-7)",
        CLI_PARM_TYPE_BLK_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<sub_list>",
        "Sub block list (0-15)",
        CLI_PARM_TYPE_SUB_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
#endif /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        "<tgt_list>",
#if defined(VTSS_ARCH_B2)
        "Target list (0-63)",
#else
        "Target list (0-255)",
#endif /* VTSS_ARCH_B2 */
        CLI_PARM_TYPE_TGT_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
#endif /* VTSS_ARCH_B2/LUTON26 */
    {
        "<addr_list>",
        "Register address list (0-31)",
        CLI_PARM_TYPE_PHY_ADDR_LIST,
        CLI_CMD_ID_DEBUG_PHY_READ,
        CLI_PARM_FLAG_NONE
    },
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        "<addr_list>",
#if defined(VTSS_ARCH_B2)
        "Register address list (0-65535)",
#else
        "Register address list (0-16383)",
#endif /* VTSS_ARCH_B2 */
        CLI_PARM_TYPE_ADDR_LIST,
        CLI_CMD_ID_DEBUG_REG_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<addr_list>",
#if defined(VTSS_ARCH_B2)
        "Register address list (0-65535)",
#else
        "Register address list (0-16383)",
#endif /* VTSS_ARCH_B2 */
        CLI_PARM_TYPE_ADDR_LIST,
        CLI_CMD_ID_DEBUG_REG_WRITE,
        CLI_PARM_FLAG_NONE
    },
#endif /* VTSS_ARCH_B2/LUTON26 */
    {
        "<addr_list>",
        "Register address list (0-255)",
        CLI_PARM_TYPE_ADDR_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<value>",
        "Register value (0-0xffff)",
        CLI_PARM_TYPE_PHY_VALUE,
        CLI_CMD_ID_DEBUG_PHY_WRITE,
        CLI_PARM_FLAG_SET
    },
    {
        "<value>",
        "Register value (0-0xffffffff)",
        CLI_PARM_TYPE_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<page>",
        "Register page (0-0xffff), default: page 0",
        CLI_PARM_TYPE_PAGE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "binary|decimal",
        "binary     : Show value in binary format\n"
        "decimal    : Show value in decimal format\n"
        "(default: Show value in hexadecimal format)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT
    },
#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
    {
        "<mmd_list>",
        "MMD address list (0-31)",
        CLI_PARM_TYPE_MMD_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<mmd_addr>",
        "MMD register address (0-0xffff)",
        CLI_PARM_TYPE_MMD_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
#endif /* VTSS_FEATURE_10G */
    {
        "<layer>",
        "API Layer, 'ail' or 'cil' (default: All layers)",
        CLI_PARM_TYPE_API_LAYER,
        CLI_CMD_ID_DEBUG_API,
        CLI_PARM_FLAG_NONE
    },
    {
        "<group>",
        "API Function Group or 'show' to list groups (default: All groups)",
        CLI_PARM_TYPE_API_GROUP,
        CLI_CMD_ID_DEBUG_API,
        CLI_PARM_FLAG_NONE
    },
    {
        "full",
        "Show full information",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<layer>",
        "Trace layer, 'ail' or 'cil', default: All layers",
        CLI_PARM_TYPE_TRACE_LAYER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<group>",
        "Trace group name, default: All groups",
        CLI_PARM_TYPE_TRACE_GROUP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "off|error|info|debug|noise",
        "off     : No trace\n"
        "error   : Error trace level\n"
        "info    : Information trace level\n"
        "debug   : Debug trace level\n"
        "noise   : Noise trace level\n"
        "(default: Show trace level)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
#if defined(VTSS_GPIOS)
    {
        "<gpio_list>",
        "GPIO list, default: All GPIOs",
        CLI_PARM_TYPE_GPIO_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "enable|disable",
        "enable     : Use output mode\n"
        "disable    : Use input mode\n"
        "(default: Enable output mode)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_DEBUG_GPIO_OUTPUT,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET
    },
    {
        "<gpio_no>",
        "0 or 1",
        CLI_PARM_TYPE_CLK_GPIO_NO,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "gpio_value",
        "\n        : The GPIO pin output value 0/1\n",
        CLI_PARM_TYPE_GPIO_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /* VTSS_GPIOS */
#ifdef VTSS_ARCH_DAYTONA
    {
        "<tgt_list>",
        "Block list (0-63)",
        CLI_PARM_TYPE_TGT_LIST,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<gen_mode>",
        "UPI test pattern generator mode:\n"
        "        0=> DISABLE\n"
        "        1=> PRBS7_P\n"
        "        2=> PRBS7_N\n"
        "        3=> PRBS23_P\n"
        "        4=> PRBS23_N\n"
        "        5=> PRBS31_P\n"
        "        6=> PRBS31_N\n"
        "        7=> HF\n"
        "        8=> LF\n",
        CLI_PARM_TYPE_GEN_MODE,
        CLI_CMD_ID_UPI_TEST_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<chk_mode>",
        "UPI test pattern checker mode:\n"
        "        0=> DISABLE\n"
        "        1=> PRBS7_P\n"
        "        2=> PRBS7_N\n"
        "        3=> PRBS23_P\n"
        "        4=> PRBS23_N\n"
        "        5=> PRBS31_P\n"
        "        6=> PRBS31_N\n",
        CLI_PARM_TYPE_CHK_MODE,
        CLI_CMD_ID_UPI_TEST_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<wrp_mode>",
        "UPI test rx wrap mode:\n"
        "        0=> RX Serial input\n"
        "        1=> Int Loop (from rx generator)\n"
        "        2=> Int Wrap (from Tx path)\n",
        CLI_PARM_TYPE_WRP_MODE,
        CLI_CMD_ID_UPI_TEST_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "equip_loopback|line_loopback|no_loopback",
        "\n"
        "    equip_loopback : equipment loopback\n"
        "    line_loopback  : line loopback\n"
        "    no_loopback    : no loopback\n"
        "    (default       : unchanged)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "line_side_equip|line_side_line|sys_side_equip|sys_side_line|no_loopback",
        "\n"
        "    line_side_equip: TX Terminal Loopback\n"
        "    line_side_line : RX Facility Loopback\n"
        "    sys_side_equip : TX Facility Loopback\n"
        "    sys_side_line  : RX Terminal Loopback\n"
        "    no_loopback    : no loopback\n"
        "    (default       : unchanged)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "a_lane_xfi|b_lane_xfi",
        "\n"
        "    a_lane_xfi  : Select XFI lane A\n"
        "    b_lane_xfi  : Select XFI lane B\n"
        "    (default   : unchanged)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_XFI_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "lane_a|lane_b",
        "\n"
        "    lane_A  : Select XFI lane A\n"
        "    lane_B  : Select XFI lane B\n"
        "    (default   : unchanged)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_XFI_REC_CLK_STATUS,
        CLI_PARM_FLAG_NONE
    },
    {
        "<gen_mode>",
        "XFI test pattern generator mode:\n"
        "        0=> DISABLE\n"
        "        1=> PRBS7_P\n"
        "        2=> PRBS7_N\n"
        "        3=> PRBS23_P\n"
        "        4=> PRBS23_N\n"
        "        5=> PRBS31_P\n"
        "        6=> PRBS31_N\n"
        "        7=> HF\n"
        "        8=> LF\n"
        "        9=> PRBS9_P",
        CLI_PARM_TYPE_GEN_MODE,
        CLI_CMD_ID_XFI_TEST_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<chk_mode>",
        "XFI test pattern checker mode:\n"
        "        0=> DISABLE\n"
        "        1=> PRBS7_P\n"
        "        2=> PRBS7_N\n"
        "        3=> PRBS23_P\n"
        "        4=> PRBS23_N\n"
        "        5=> PRBS31_P\n"
        "        6=> PRBS31_N\n",
        CLI_PARM_TYPE_CHK_MODE,
        CLI_CMD_ID_XFI_TEST_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "tx_rx_loopback|rx_tx_loopback|no_loopback",
        "\n"
        "    tx_rx_loopback  : Transmit to Receive loopback\n"
        "    rx_tx_loopback  : Receive to Transmit loopback\n"
        "    no_loopback     : No Loopback\n"
        "    (default   : unchanged)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_MAC10G_LB_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<txeq_mode>",
        "\n"
        "    0  : Traditional Override\n"
        "    1  : Traditional \n"
        "    2  : 802.3ap Override \n"
        "    3  : 802.3ap \n",
        CLI_PARM_TYPE_XFI_TXEQ_MODE,
        CLI_CMD_ID_XFI_TXEQ_MODE,
        CLI_PARM_FLAG_SET
    },
    {
        "<txeq_channel_loss>",
        "\n"
        "    0  : Less than 10 inch length\n"
        "    1  : Greater than 10 inch length\n"
        "    2  : SFP Plus Channel \n"
        "    3  : Copper cable \n",
        CLI_PARM_TYPE_TXEQ_CHNLOSS,
        CLI_CMD_ID_XFI_TXEQ_MODE,
        CLI_PARM_FLAG_SET
    },
    {
        "<port_no>",
        "port number",
        CLI_PARM_TYPE_TXEQ_PORTNO,
        CLI_CMD_ID_XFI_TXEQ_MODE,
        CLI_PARM_FLAG_SET
    }, 
    {
        "slew|power",
        "\n"
        "    slew    : Set XFI slew value\n"
        "    power   : Select XFI lane B\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_XFI_TXMISC_PARM,
        CLI_PARM_FLAG_NONE
    },
    {
        "<xfi_rxeq_mode>",
        "\n"
        "   The XFI RX_MODE \n"
        "        Channel Loss Type Conf\n"
        "                  1 for < 10 inch\n"
        "                  2 for > 10 inch\n"
        "                  3 for sfp_plus\n"
        "                  4 for copper_cable\n",
        CLI_PARM_TYPE_XFI_RXEQ_MODE,
        CLI_CMD_ID_XFI_RXEQ_MODE,
        CLI_PARM_FLAG_SET
    },
    {
        "<xfi_misc_parm_value>",
        "\n"
        "    The value of the slew/power that needs to be set\n"
        "          Allowed  Slew      :: 0,3,5 or 7 \n"
        "          Allowed  Amp/Power :: 32-127\n",
        CLI_PARM_TYPE_XFI_TXEQ_PARM_VALUE,
        CLI_CMD_ID_XFI_TXMISC_PARM,
        CLI_PARM_FLAG_SET
    },
    {
        "<coef>",
        "coefcient number (0,1,2)",
        CLI_PARM_TYPE_XFI_TXEQ_COEF,
        CLI_CMD_ID_XFI_TXEQ_COEFUPD,
        CLI_PARM_FLAG_SET
    },
    {
        "<step_size>",
        "Step size to increment or decrement the coefficient",
        CLI_PARM_TYPE_XFI_TXEQ_STEPSIZE,
        CLI_CMD_ID_XFI_TXEQ_COEFUPD,
        CLI_PARM_FLAG_SET
    },
    {
        "<incr>",
        "Increment(1) or decrement(0) the coefficient",
        CLI_PARM_TYPE_XFI_TXEQ_INCR,
        CLI_CMD_ID_XFI_TXEQ_COEFUPD,
        CLI_PARM_FLAG_SET
    },
    {
        "<polarity>",
        "Polarity of the coefficient positive(1) or negative(0)",
        CLI_PARM_TYPE_XFI_TXEQ_POLARITY,
        CLI_CMD_ID_XFI_TXEQ_COEFUPD,
        CLI_PARM_FLAG_SET
    },
    {
        "<lane>",
        "XFI lanes 0:lane a  1:lane b",
        CLI_PARM_TYPE_XFI_802A_TXEQ_LANE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<cof_802a>",
        "802.ap Co-efficient status 0:Not updated 1:Updated 2:Minimum 3:Maximum",
        CLI_PARM_TYPE_XFI_802A_TXEQ_COEF,
        CLI_CMD_ID_XFI_802a_TXEQ_COEUPD,
        CLI_PARM_FLAG_SET
    },    
    {
        "<upd_req>",
        "802.3ap update request type 0:Hold 1:Increment 2:Decrement",
        CLI_PARM_TYPE_XFI_802A_TXEQ_UPDREQ,
        CLI_CMD_ID_XFI_802a_TXEQ_COEUPD,
        CLI_PARM_FLAG_SET
    },
    {
        "<xfi_fsm_ctl>",
        "802.3ap FSM control field : 0 - 0xffff",
        CLI_PARM_TYPE_XFI_802A_FSM_CTL,
        CLI_CMD_ID_XFI_802a_TXEQ_FSM,
        CLI_PARM_FLAG_SET
    },
    {
        "<q_code>",
        "XAUI transmitted q_code",
        CLI_PARM_TYPE_Q_CODE,
        CLI_CMD_ID_XAUI_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<max_frame_len>",
        "\n"
        "    0 - 10240 frame length",
        CLI_PARM_TYPE_MAC10G_MAX_FRAME_LEN,
        CLI_CMD_ID_MAC10G_FRAME_LEN_CONFIG,
        CLI_PARM_FLAG_SET
    },
    {
        "<max_tags>",
        "\n"
        "    0 - 2 \n"
        "    0 - Un Tagged \n"
        "    1 - Single Tagged \n"
        "    2 - Double Tagged \n",
        CLI_PARM_TYPE_MAC10G_MAX_TAGS,
        CLI_CMD_ID_MAC10G_FRAME_LEN_CONFIG,
        CLI_PARM_FLAG_SET
    },
     {
        "<thresh_high>",
        "\n"
        "    0 - 8191 Units :: 8 - Byte words \n",
        CLI_PARM_TYPE_RAB_THRESH_HIGH,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
     {
        "<thresh_low>",
        "\n"
        "    0 - 8191 Units :: 8 - Byte Words \n",
        CLI_PARM_TYPE_RAB_THRESH_LOW,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
     {
        "<thresh_adapt>",
        "\n"
        "    0 - 8191 Units :: 8 - Byte Words \n",
        CLI_PARM_TYPE_RAB_THRESH_ADAPT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#if defined(DAYTONA_EVAL_INIT)
    {
        "<reg_addr>",
        "LBFPGA register (0-0xff)",
        CLI_PARM_TYPE_LBFPGA_REG,
        CLI_CMD_ID_DEBUG_LBFPGA_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "LBFPGA register (0-0xff)",
        CLI_PARM_TYPE_LBFPGA_REG,
        CLI_CMD_ID_DEBUG_LBFPGA_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the pcf8575 instance (0-2)",
        CLI_PARM_TYPE_PCF8575_TGTID,
        CLI_CMD_ID_DEBUG_PCF8575_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the pcf8575 instance (0-2)",
        CLI_PARM_TYPE_PCF8575_TGTID,
        CLI_CMD_ID_DEBUG_PCF8575_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of vsc3172 (0-0x28e)",
        CLI_PARM_TYPE_VSC3172_REG,
        CLI_CMD_ID_DEBUG_VSC3172_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of vsc3172 (0-0x28e)",
        CLI_PARM_TYPE_VSC3172_REG,
        CLI_CMD_ID_DEBUG_VSC3172_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of si5338 (0-0xf6)",
        CLI_PARM_TYPE_SI5338_REG,
        CLI_CMD_ID_DEBUG_SI5338_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of si5338 (0-0xf6)",
        CLI_PARM_TYPE_SI5338_REG,
        CLI_CMD_ID_DEBUG_SI5338_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the si5338 instance (0-7)",
        CLI_PARM_TYPE_SI5338_TGTID,
        CLI_CMD_ID_DEBUG_SI5338_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the si5338 instance (0-7)",
        CLI_PARM_TYPE_SI5338_TGTID,
        CLI_CMD_ID_DEBUG_SI5338_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of si5326 (0-0x88)",
        CLI_PARM_TYPE_SI5326_REG,
        CLI_CMD_ID_DEBUG_SI5326_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<reg_addr>",
        "register number of si5326 (0-0x88)",
        CLI_PARM_TYPE_SI5326_REG,
        CLI_CMD_ID_DEBUG_SI5326_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the si5326 instance (0-16)",
        CLI_PARM_TYPE_SI5326_TGTID,
        CLI_CMD_ID_DEBUG_SI5326_WRITE,
        CLI_PARM_FLAG_NONE
    },
    {
        "<target_id>",
        "target_id of the si5326 instance (0-16)",
        CLI_PARM_TYPE_SI5326_TGTID,
        CLI_CMD_ID_DEBUG_SI5326_READ,
        CLI_PARM_FLAG_NONE
    },
    {
        "<fpga_mask>",
        "Mask, 0-0xFFFFFFFF",
        CLI_PARM_TYPE_FPGA_MASK_VALUE,
        CLI_CMD_ID_DEBUG_LBFPGA_WRITE,
        CLI_PARM_FLAG_SET
    },
    {
        "<val_mask>",
        "Mask, 0-0xFFFF",
        CLI_PARM_TYPE_VAL_MASK_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /* DAYTONA_EVAL_INIT */ 
#if defined(VTSS_FEATURE_I2C)
    {
        "<dev_addr>",
        "\n"
        "    Device address \n",
        CLI_PARM_TYPE_I2C_DEV_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },

    {
        "<reg_addr>",
        "\n"
        "    Register address \n",
        CLI_PARM_TYPE_I2C_REG_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<data>",
        "\n"
        "    I2C data \n",
        CLI_PARM_TYPE_I2C_DATA,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<controller_no>",
        "\n"
        "    I2C controller number. The value should be either 1 or 2.\n",
        CLI_PARM_TYPE_I2C_CONTROLLER_NO,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /* VTSS_FEATURE_I2C */
#ifdef VTSS_FEATURE_OTN
    {
        "<otn_value>",
        "Transmitted RES value if enabled",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_OTU_TX_RES,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_value>",
        "Transmitted SM RES value if enabled",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_OTU_TX_SMRES,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_value>",
        "Transmitted GCC0 value if enabled",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_OTU_TX_GCC0,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_value>",
        "Transmitted RES value if enabled",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_ODU_TX_RES,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_value>",
        "Transmitted EXP value if enabled",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_ODU_TX_EXP,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_value>",
        "Test payload fixed value (bit0-bit7)",
        CLI_PARAM_TYPE_OTN_VALUE,
        CLI_CMD_ID_OTN_OPU_TEST_INSERTION,
        CLI_PARM_FLAG_SET
    },
    {
        "<ftfl_value>",
        "Transmitted FTFL value 0-0xFF",
        CLI_PARM_TYPE_FTFL_VALUE,
        CLI_CMD_ID_OTN_ODU_TX_FTFL,
        CLI_PARM_FLAG_SET
    },
    {
        "<ftfl_index>",
        "Index for transmitted FTFL value 1-256",
        CLI_PARM_TYPE_FTFL_INDEX,
        CLI_CMD_ID_OTN_ODU_TX_FTFL,
        CLI_PARM_FLAG_SET
    },
    {
        "<mode>",
        "Overhead Insertion Mode",
        CLI_PARAM_TYPE_OTN_OH_INSERTION,
        CLI_CMD_ID_OTN_ODU_OH_INSERTION,
        CLI_PARM_FLAG_SET
    },
    {
        "<mode>",
        "Overhead Insertion Mode",
        CLI_PARAM_TYPE_OTN_OTU_OH_INSERTION,
        CLI_CMD_ID_OTN_OTU_OH_INSERTION,
        CLI_PARM_FLAG_SET
    },
    {
        "<mode>",
        "Overhead Insertion Mode",
        CLI_PARAM_TYPE_OTN_OPU_OH_INSERTION,
        CLI_CMD_ID_OTN_OPU_OH_INSERTION,
        CLI_PARM_FLAG_SET
    },
    {
        "<test_payload_type>",
        "Test payload insertion mode",
        CLI_PARAM_TYPE_OTN_TEST_INSERTION,
        CLI_CMD_ID_OTN_OPU_TEST_INSERTION,
        CLI_PARM_FLAG_SET
    },
    {
        "<payload_type>",
        "Transmitted and expected Payload type",
        CLI_PARM_TYPE_OTN_ODU_PL_PAYLOAD_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<otn_tti>",
        "tti (text string) size: [64]",
        CLI_PARM_TYPE_OTN_TTI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<exp_tti>",
        "expected tti (text string) size: [64]",
        CLI_PARM_TYPE_OTN_EX_TTI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "sapi_tim_en|sapi_tim_dis",
        "Enable|Disable OTU TIM detection on SAPI",
        CLI_PARM_TYPE_OTN_SAPI_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "dapi_tim_en|dapi_tim_dis",
        "Enable|Disable OTU TIM detection on DAPI",
        CLI_PARM_TYPE_OTN_DAPI_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "os_tim_en|os_tim_dis",
        "Enable|Disable OTU TIM detection on OS",
        CLI_PARM_TYPE_OTN_OS_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<beibiae>",
        "Enable/disable BEI+BIAE consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BEIBIAE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi>",
        "Enable/disable BDI consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<iae>",
        "Enable/disable IAE consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_IAE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ais>",
        "Enable/disable AIS consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_AIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi_los>",
        "Enable/disable LOS constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi_lof>",
        "Enable/disable LOF constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOF,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi_lom>",
        "Enable/disable LOM constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi_ais>",
        "Enable/disable AIS constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_AIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdi_tim>",
        "Enable/disable TIM constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ais_lom>",
        "Enable/disable LOM constribution to AIS\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_LOM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ais_tim>",
        "Enable/disable TIM constribution to AIS\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bei>",
        "Enable/disable BEI consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BEI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<odu_bdi>",
        "Enable/disable BDI consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdilos>",
        "Enable/disable LOS constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdilof>",
        "Enable/disable LOF constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOF,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdilom>",
        "Enable/disable LOM constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdiotuais>",
        "Enable/disable OTU AIS constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_AIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdiotutim>",
        "Enable/disable OTU TIM constribution to BDI (SSF)\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdiais>",
        "Enable/disable ODU AIS constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_AIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdioci>",
        "Enable/disable OCI constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OCI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdilck>",
        "Enable/disable LCK constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LCK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bdiltc>",
        "Enable/disable LTC constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LTC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<bditim>",
        "Enable/disable LCK constribution to BDI\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_TIM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<odu_ais>",
        "Enable/disable AIS consequent action\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_AIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<si_ais>",
        "Enable/disable Forced AIS in sink direction\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_SIAIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<so_ais>",
        "Enable/disable Forced AIS in source direction\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_SOAIS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ais_timocilck>",
        "Enable/disable TIM/OCI/LCK constribution to AIS\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_TIMOCILCK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ais_plm>",
        "Enable/disable PLM constribution to AIS\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_PLM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<pl_plm>",
        "Enable/disable PLM detection\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_PL_PLM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<pl_tx>",
        "Enable/disable Payload type transmission\n"
        "    0: Disable\n"
        "    1: Enable",
        CLI_PARM_TYPE_OTN_ODU_PL_TX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<payload_type>",
        "Transmitted and expected Payload type",
        CLI_PARM_TYPE_OTN_ODU_PL_PAYLOAD_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "none_fec|rs_fec|i4_fec|i7_fec",
        "ODUT FEC mode",
        CLI_PARM_TYPE_OTN_OCH_FEC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ber_threshold",
        "Fec Ber threshold\n"
        "RS Fec \t\t\t: 1 - 10^-3 or 0 - 10^-4\n"
        "I7 Fec \t\t\t: 8356 - 2 * 10^-3 \n",
        CLI_PARM_TYPE_OTN_OCH_BER_THRES,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "termination|monitor",
        "ODU/T operation mode",
        CLI_PARM_TYPE_OTN_ODU_MODE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<odut_level>",
        "ODUT level",
        CLI_PARM_TYPE_OTN_ODUT_LEVEL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_NONE
    },
    {
        "unforced_bdi|one_bdi|zero_bdi",
        "ODUT operation mode",
        CLI_PARM_TYPE_OTN_ODUT_BDI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stat>",
        "ODUT tx stat value",
        CLI_PARM_TYPE_OTN_ODUT_STAT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<aps>",
        "ODUT tx APS value",
        CLI_PARM_TYPE_OTN_ODUT_APS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /* VTSS_FEATURE_OTN */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    {
        "<port_no>",
        "port number",
        CLI_PARM_TYPE_1588_PORTNO,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<engine_id>",
        "Engine Identifier (values are 0,1,2 and 3)",
        CLI_PARM_TYPE_1588_ENGID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<clkfreq>",
        "Clock Frequency; range: 0-4\n"
        "\t\t\t 0 -> 125 MHz\n"
        "\t\t\t 1 -> 156.25 MHz\n"
        "\t\t\t 2 -> 200 MHz\n"
        "\t\t\t 3 -> 250 MHz\n"
        "\t\t\t 4 -> 500 MHz\n",
        CLI_PARM_TYPE_1588_CLK_FREQ,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<clk_src>",
        "Clock Source; range: 0-5\n"
        "\t\t\t 0 -> External source\n"
        "\t\t\t 1 -> 10G: XAUI lane 0 recovered clock. 1G: MAC RX clock\n"
        "\t\t\t 2 -> 10G: XAUI lane 0 recovered clock, 1G: MAC TX clock\n"
        "\t\t\t 3 -> Received line clock\n"
        "\t\t\t 4 -> Transmitted line clock\n"
        "\t\t\t 5 -> 10G: Invalid, 1G: Internal 250 MHz Clock",
        CLI_PARM_TYPE_1588_CLK_SRC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<modify_frm>",
        "modify bit :0 aand 1\n"
        "\t\t\t 0 -> modify bit different PHY\n"
        "\t\t\t 1 -> modify bit  same PHY\n",
        CLI_PARM_TYPE_1588_MOD_FRM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<rx_ts_pos>",
        "RX Time Stamp Position: 0-1\n"
        "\t\t\t 0 -> 4 Reserved bytes in PTP header\n"
        "\t\t\t 1 -> 4 Bytes appended at the end",
        CLI_PARM_TYPE_1588_RX_TS_POS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tx_fifo_mode>",
        "Tx FIFO mode; range: 0-1"
        "\t\t\t 0 -> Read TS from normal CPU interface\n"
        "\t\t\t 1 -> Push TS on the SPI interface",
        CLI_PARM_TYPE_1588_TX_FIFO_MODE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<action_id>",
        "Action ID; range: 0-2 for PTP, 0-5 for OAM",
        CLI_PARM_TYPE_1588_ACTID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ingress|egress",
        "ingress or egress engine",
        CLI_PARM_TYPE_1588_DIRECTION,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<encap_type>",
        "encapsulation type, Following are the encapsulation types supported\n"
        "\t\t\t 0 -> ETH/PTP \n"
        "\t\t\t 1 -> ETH/IP/PTP \n"
        "\t\t\t 2 -> ETH/IP/IP/PTP \n"
        "\t\t\t 3 -> ETH/ETH/PTP \n"
        "\t\t\t 4 -> ETH/ETH/IP/PTP \n"
        "\t\t\t 5 -> ETH/MPLS/IP/PTP \n"
        "\t\t\t 6 -> ETH/MPLS/ETH/PTP \n"
        "\t\t\t 7 -> ETH/MPLS/ETH/IP/PTP \n"
        "\t\t\t 8 -> ETH/MPLS/ACH/PTP \n"
        "\t\t\t 9 -> ETH/OAM \n"
        "\t\t\t 10-> ETH/ETH/OAM \n"
        "\t\t\t 11-> ETH/MPLS/ETH/OAM\n"
        "\t\t\t 12-> ETH/MPLS/ACH/OAM  ",
        CLI_PARM_TYPE_1588_ENCAP_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<channel_map>",
        "flow associated to channel",
        CLI_PARM_TYPE_1588_CHNLMAP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<flow_id>",
        "flow Identifier (0 to 7)",
        CLI_PARM_TYPE_1588_FLOW_ID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<flow_st_index>",
        "Start index of the flow range, i.e., start flow_id",
        CLI_PARM_TYPE_1588_FLOW_ST_IDX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<flow_end_index>",
        "Last index of the flow range, i.e., Last flow_id",
        CLI_PARM_TYPE_1588_FLOW_END_IDX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tpid>",
        "Tag Protocoal identifier value(0x1 ~ 0xFFFF)",
        CLI_PARM_TYPE_1588_TPID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<num_tag>",
        "Number of VLAN tags in the packet(possible values: 0,1 or 2",
        CLI_PARM_TYPE_1588_NUMBER_OF_TAGS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag1_type>",
        "Supported tag1 types are Customer tag, Service provider tag, Internal tag, Bridge tag\n"
        "\t\t\t Possible Values for tag types are(1 to 4)\n"
        "\t\t\t 1->Customer tag\n"
        "\t\t\t 2->Service provider tag\n"
        "\t\t\t 3->Internal tag\n"
        "\t\t\t 4->Bridge tag",
        CLI_PARM_TYPE_1588_TAG1_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag2_type>",
        "Supported tag2 types are Customer tag, Service provider tag, Internal tag, Bridge tag\n"
        "\t\t\t Possible Values for tag types are(1 to 4)\n"
        "\t\t\t 1->Customer tag\n"
        "\t\t\t 2->Service provider tag\n"
        "\t\t\t 3->Internal tag\n"
        "\t\t\t 4->Bridge tag",
        CLI_PARM_TYPE_1588_TAG2_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag1_lower>",
        "For specifying the range of tag values for tag1,tag1_lower:Start index/tag in the tag range",
        CLI_PARM_TYPE_1588_TAG1_LOWER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag1_upper>",
        "For specifying the range of tag values for tag1,tag1_upper:Last index/tag in the tag range",
        CLI_PARM_TYPE_1588_TAG1_UPPER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag2_lower>",
        "For specifying the range of tag values for tag2,tag2_lower:Start index/tag in the tag range",
        CLI_PARM_TYPE_1588_TAG2_LOWER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tag2_upper>",
        "For specifying the range of tag values for tag2,tag2_upper:Last index/tag in the tag range",
        CLI_PARM_TYPE_1588_TAG2_UPPER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<macaddr>",
        "MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_TYPE_1588_MAC_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "match_full|any_uc|any_mc",
        "MAC address match mode\n"
        "\t\t\t match_full: Exact Match\n"
        "\t\t\t any_uc    : any unicact MAC\n"
        "\t\t\t any_mc    : any multicast MAC",
        CLI_PARM_TYPE_1588_MAC_MATCH,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "strict|non-strict",
        "Flow Match configuration \n"
        "\t\t\t strict:Packet should match against Same flow id in all comparators, except MPLS and PTP\n"
        "\t\t\t non-strict: Packet can match any flow with in a comparator",
        CLI_PARM_TYPE_1588_FLOW_MATCH,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "pbb_en|pbb_dis",
        "PBB match configuration \n"
        "\t\t\t pbb_en: To enable PBB encapsulation classification \n"
        "\t\t\t pbb_dis: To disable PBB encapsulation classification",
        CLI_PARM_TYPE_1588_PBB_EN,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "match_source|match_dest|match_src_dest",
        "Address selection: \n"
        "\t\t\t match_src      : Source Address\n"
        "\t\t\t match_dest     : Destination Address\n"
        "\t\t\t match_src_dest : Source Address  or Destination Address",
        CLI_PARM_TYPE_1588_ADDR_MATCH,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "vlan_chk_en|vlan_chk_dis",
        "VLAN check configuration \n"
        "\t\t\t vlan_chk_en : To Enable VLAN header classification\n"
        "\t\t\t vlan_chk_dis: To Disable VLAN header classification",
        CLI_PARM_TYPE_1588_VLAN_CHK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "tag_rng_none|tag_rng_outer|tag_rng_inner",
        "VLAN tag range check\n"
        "\t\t\t tag_rng_none : Tag range check disable\n"
        "\t\t\t tag_rng_outer: Outer Tag range check enable\n"
        "\t\t\t tag_rng_inner: Inner Tag range check enable",
        CLI_PARM_TYPE_1588_TAG_RANGE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ipv4|ipv6",
        "IP version ",
        CLI_PARM_TYPE_1588_IP_VER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ipv4",
        "Keyword to identify ipv4 parameters",
        CLI_PARM_TYPE_1588_IPV4_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ipv6",
        "Keyword to identify ipv6 parameters",
        CLI_PARM_TYPE_1588_IPV6_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "stk_ref_top|stk_ref_bottom",
        "stack ref point",
        CLI_PARM_TYPE_1588_MPLS_STKREF,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ptp",
        "Keyword for PTP action",
        CLI_PARM_TYPE_1588_ACT_PTP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "y1731_oam",
        "keyword for Y1731_OAM action",
        CLI_PARM_TYPE_1588_ACT_Y1731,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ietf_oam",
        "keyword for IETF_OAM action",
        CLI_PARM_TYPE_1588_ACT_IETF_OAM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "cw_en|cw_dis",
        "Control Word configuration\n"
        "\t\t\t cw_en : MPLS packet with control word\n"
        "\t\t\t cw_dis: MPLS packet with out control word.",
        CLI_PARM_TYPE_1588_MPLS_CTRLW,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<clock_mode>",
        "PTP clock mode: \n"
        "\t\t\t 0 -> BC One Step\n"
        "\t\t\t 1 -> BC Two Step\n"
        "\t\t\t 2 -> TC one Step\n"
        "\t\t\t 3 -> TC Two Step",
        CLI_PARM_TYPE_1588_CLK_MODE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<domain_lower>",
        "lower value of domain range",
        CLI_PARM_TYPE_1588_DOMAIN_LOWER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<domain_upper>",
        "upper value of domain range",
        CLI_PARM_TYPE_1588_DOMAIN_UPPER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<version>",
        "OAM Version : only 0 is supported",
        CLI_PARM_TYPE_1588_OAM_VER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<pdm>",
        "Delay method (0 or 1)\n"
        "\t\t\t 0 -> Peer to Peer\n"
        "\t\t\t 1 -> End to End",
        CLI_PARM_TYPE_1588_PTP_DELAYM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<yodm>",
        "Delay method for y1731_oam \n"
        "\t\t\t 0 -> One-way Delay Measurement Method(1DM)\n"
        "\t\t\t 1 -> Two-way Delay Measurement Method(DMM)",
        CLI_PARM_TYPE_1588_Y1731_DELAYM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<iodm>",
        "Delay method for ietf_oam \n"
        "\t\t\t 0 -> Delay Measurement Method(DM)\n"
        "\t\t\t 1 -> Loss and Delay Measurement(LDM), In LDM :: We only update the TimeStamp Fields \n",
        CLI_PARM_TYPE_1588_IETF_OAM_DELAYM,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<meg_lower>",
        "lower value of Maintenence Entity Group (MEG) range",
        CLI_PARM_TYPE_1588_MEG_RANGE_LOWER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<meg_upper>",
        "upper value of Maintenence Entity Group (MEG) range",
        CLI_PARM_TYPE_1588_MEG_RANGE_UPPER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tf>",
        "TimeStamp Format \n"
        "Currently only 3 (PTP TimeStamp Format) is used",
        CLI_PARM_TYPE_1588_TS_FORMAT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tc>",
        "Traffic Class : DSCP Field in the Header",
        CLI_PARM_TYPE_1588_TRAFFIC_CLASS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<signature_mask>",
        "Signature Mask : Signature Mask for extracting the signature bytes from the Packet",
        CLI_PARM_TYPE_1588_SIG_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<sport_val>",
        "source port value( 1 ~ 0xFFFF)",
        CLI_PARM_TYPE_1588_SPORT_VAL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<sport_mask>",
        "source port mask to specify the source port range, value ( 0~0xFFFF)",
        CLI_PARM_TYPE_1588_SPORT_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<dport_val>",
        "destination port value (1 ~ 0xFFFF)",
        CLI_PARM_TYPE_1588_DPORT_VAL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<dport_mask>",
        "destination port mask to specify the destination port range, value ( 0~0xFFFF)",
        CLI_PARM_TYPE_1588_DPORT_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ip_addr>",
        "IPv4 address (in x.x.x.x format, where x is decimal value 0~255)",
        CLI_PARM_TYPE_1588_IPV4_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ip_mask>",
        "IPv4 address mask  (in x.x.x.x format, where x is decimal value 0~255)",
        CLI_PARM_TYPE_1588_IPV4_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ipv6_addr>",
        "IPv6 address (in x:x:x:x:x:x:x:x format, where x is hexa-decimal value 0~0xFFFF) ",
        CLI_PARM_TYPE_1588_IPV6_ADDR,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ipv6_mask>",
        "IPv6 address mask in x:x:x:x:x:x:x:x format, where x is hexa-decimal value 0~0xFFFF",
        CLI_PARM_TYPE_1588_IPV6_MASK,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stk_depth>",
        "Configure stack depth",
        CLI_PARM_TYPE_1588_STK_DEPTH,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stk_lvl_0>",
        "syntax: \"stk_lvl_0 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_TYPE_1588_STKLVL0,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stk_lvl_1>",
        "syntax: \"stk_lvl_1 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_TYPE_1588_STKLVL1,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stk_lvl_2>",
        "syntax: \"stk_lvl_2 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_TYPE_1588_STKLVL2,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<stk_lvl_3>",
        "syntax: \"stk_lvl_3 <low>-<high>\"; high not reqd for fixed MPLS level",
        CLI_PARM_TYPE_1588_STKLVL3,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ach_ver>",
        "ACH version",
        CLI_PARM_TYPE_1588_ACH_VER,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<channel_type>",
        "ACH channel type (0~0xFFFF)",
        CLI_PARM_TYPE_1588_CHANNEL_TYPE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<proto_id>",
        "Protocol Identifier",
        CLI_PARM_TYPE_1588_PROTOCOL_ID,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<time_sec>",
        "How many seconds to collect stats, max 60. default 10 seconds.,collects at every second.",
        CLI_PARM_TYPE_1588_TIME_SEC,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
       "<latency_val>",
       "Value of the ingress/egress latency for the particular port in Nano seconds of the range(0-65536)",
       CLI_PARM_TYPE_1588_LATENCY_VAL,
       CLI_CMD_ID_NONE,
       CLI_PARM_FLAG_SET
    },
    {
       "<delay_val>",
       "Value of the path delay for the particular port in Nano seconds of the range(0-4294967295)",
       CLI_PARM_TYPE_1588_DELAY_VAL,
       CLI_CMD_ID_NONE,
       CLI_PARM_FLAG_SET
    },
    {
       "<asym_val>",
       "Value of the delay aymmetry for the particular port in Nano seconds of the range(-32768 to 32767)",
       CLI_PARM_TYPE_1588_ASYM_VAL,
       CLI_CMD_ID_NONE,
       CLI_PARM_FLAG_SET
    },
   {
       "<blk_id_1588>",
       "1588 internal block id <0 - 7>\n"
       " 0 -  Ingress Analyzer 0 \n"
       " 1 -  Egress  Analyzer 0 \n"
       " 2 -  Ingress Analyzer 1 \n"
       " 3 -  Egress  Analyzer 1 \n"
       " 4 -  Ingress Analyzer 2 \n"
       " 5 -  Egress  Analyzer 2 \n"
       " 6 -  Processor 0 \n"
       " 7 -  Processor 1 \n",
       CLI_PARM_TYPE_1588_BLK_ID,
       CLI_CMD_ID_NONE,
       CLI_PARM_FLAG_SET
   },
   {
       "<csr_reg_offset>",
       "1588 CSR internal block register offset.",
       CLI_PARM_TYPE_1588_CSR_OFFSET,
       CLI_CMD_ID_NONE,
       CLI_PARM_FLAG_SET
   },
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#ifdef VTSS_FEATURE_GFP
    {
        "core_en|core_dis",
        "Core header single error correction enable/disable",
        CLI_PARAM_TYPE_GFP_SINGLE_ERROR_EN_DIS,
        CLI_CMD_ID_GFP_SINGLE_ERR_CORR,
        CLI_PARM_FLAG_SET
    },
    {
        "pyld_en|pyld_dis",
        "Payload header single error correction enable/disable",
        CLI_PARAM_TYPE_GFP_SINGLE_ERROR_EN_DIS,
        CLI_CMD_ID_GFP_SINGLE_ERR_CORR,
        CLI_PARM_FLAG_SET
    },
    {
        "trans_en|trans_dis",
        "Transparent super block single error correction enable/disable",
        CLI_PARAM_TYPE_GFP_SINGLE_ERROR_EN_DIS,
        CLI_CMD_ID_GFP_SINGLE_ERR_CORR,
        CLI_PARM_FLAG_SET
    },
    {
        "los_none|los_hw|los_on|los_off",
        "LOS based CMF SF auto generation configuration",
        CLI_PARAM_TYPE_GFP_CMF_AUTO_CONF,
        CLI_CMD_ID_GFP_CMF_AUTO_SF_GEN,
        CLI_PARM_FLAG_SET
    },
    {
        "lsync_none|lsync_hw|lsync_on|lsync_off",
        "LSYNC based CMF SF auto generation configuration",
        CLI_PARAM_TYPE_GFP_CMF_AUTO_CONF,
        CLI_CMD_ID_GFP_CMF_AUTO_SF_GEN,
        CLI_PARM_FLAG_SET
    },
    {
        "<cmf_int>",
        "Forced CMF generation interval",
        CLI_PARAM_TYPE_GFP_CMF_FORCED_INTERVAL,
        CLI_CMD_ID_GFP_CMF_FORCED_GEN,
        CLI_PARM_FLAG_SET
    },
    {
        "<cmf_fr_len>",
        "CMF frame length",
        CLI_PARAM_TYPE_GFP_CMF_FRAME_LENGTH,
        CLI_CMD_ID_GFP_CMF_FORCED_GEN,
        CLI_PARM_FLAG_SET
    },
    {
        "<upi>",
        "CDF or CMF UPI value",
        CLI_PARAM_TYPE_GFP_UPI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<pti>",
        "CDF or CMF PTI value",
        CLI_PARAM_TYPE_GFP_PTI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<ordered_upi>",
        "CDF ordered set UPI value",
        CLI_PARAM_TYPE_GFP_ORDERED_UPI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
	{
		"<upi_txeq_mode>",
		"\n"
		"	0 : Override \n"
		"	1 : Normal ",
		CLI_PARM_TYPE_UPI_TXEQ_MODE,
		CLI_CMD_ID_UPI_TXEQ_MODE,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_txeq_chnloss>",
		"\n"
		"	0 : Less than 10 inch length\n"
		"	1 : Greater than 10 inch length\n"
		"	2 : Backplane",
		CLI_PARM_TYPE_UPI_TXEQ_CHNLOSS,
		CLI_CMD_ID_UPI_TXEQ_MODE,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_coef_id>",
		"\n"
		"\t0 : C0 (Override:C0 <  64, Normal:C0 < 16)\n"
		"\t1 : C1 (Override:C1 < 256, Normal:C1 < 64)\n"
        "\t2 : C2 (Override:C2 < 128, Normal:C2 < 32)\n"
        "\t3 : C3 (Override:C3 <  64, Normal:C3 < 16)",
		CLI_PARM_TYPE_UPI_TXEQ_COEF_ID,	
		CLI_CMD_ID_UPI_TXEQ_COEF,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_coef_step>",
		"UPI Co-efficient step size to modify",
		CLI_PARM_TYPE_UPI_TXEQ_COEF_STEP,
		CLI_CMD_ID_UPI_TXEQ_COEF,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_incr>",
		"\n"
		"\t0 : Decrement Coefficient\n"
		"\t1 : Increment Coefficient",
		CLI_PARM_TYPE_UPI_TXEQ_COEF_INCR,
		CLI_CMD_ID_UPI_TXEQ_COEF,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_polarity>",
		"\n"
		"	0 : Negative Polarity\n"
		"	1 : Positive Polarity",
		CLI_PARM_TYPE_UPI_TXEQ_COEF_POLARITY,
		CLI_CMD_ID_UPI_TXEQ_COEF,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_power>",
		"\n"
		"	Amplitude range 20-127 mV",
		CLI_PARM_TYPE_UPI_TXEQ_POWER,
		CLI_CMD_ID_UPI_TXEQ_AMP,
		CLI_PARM_FLAG_SET
	},
	{
		"<upi_slew>",
		"\n"
		"\t0: 30  ps min - default\n"
		"\t1: 60  ps min",
		CLI_PARM_TYPE_UPI_TXEQ_SLEW,
		CLI_CMD_ID_UPI_TXEQ_SLEW,
		CLI_PARM_FLAG_SET
	},
    {
        "<upi_rxeq_mode>",
        "\n"
        "UPI RX MODE  \n"
        "Channel Loss Type Conf\n"
        "\t0: < 10 inch\n"
        "\t1: > 10 inch\n"
        "\t2: Backplane\n",
        CLI_PARM_TYPE_UPI_RXEQ_MODE,
        CLI_CMD_ID_UPI_RXEQ_MODE,
        CLI_PARM_FLAG_SET
    },                         
    {
        "los_force_on|los_force_off",
        "Forced LOS in 8B10B configuration",
        CLI_PARAM_TYPE_GFP_8B10B_FORCED,
        CLI_CMD_ID_GFP_8B10B_CONTROL_CODE,
        CLI_PARM_FLAG_SET
    },
    {
        "lsync_force_on|lsync_force_off",
        "Forced LSYNC in 8B10B configuration",
        CLI_PARAM_TYPE_GFP_8B10B_FORCED,
        CLI_CMD_ID_GFP_8B10B_CONTROL_CODE,
        CLI_PARM_FLAG_SET
    },
    {
        "10b_err|k30_7",
        "10B8B error code configuration",
        CLI_PARAM_TYPE_GFP_8B10B_ERROR,
        CLI_CMD_ID_GFP_8B10B_CONTROL_CODE,
        CLI_PARM_FLAG_SET
    },
#endif /* VTSS_FEATURE_GFP */
#ifdef VTSS_FEATURE_WIS
    {
        "<tti_mode>",
        "tti mode (size of tti) : [0=> 1 byte, 1=> 16 bytes, 2=> 64 bytes]",
        CLI_PARM_TYPE_WIS_TTI_MODE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<tti>",
        "text string, without any spaces. Size [1, 16, 64] is based on tti mode",
        CLI_PARM_TYPE_WIS_TTI,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<sl>",
        "Signal Label value 0 - 255",
        CLI_PARM_TYPE_WIS_SL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<wis_aisl>",
        "WIS AIS-L consequent actions mask <0 - 1>\n"
        " 0 bit -  Enable for AIS-L insertion on LOS  \n"
        " 1 bit -  Enable for AIS-L insertion on LOF \n",
        CLI_PARM_TYPE_WIS_AISL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<wis_rdil>",
        "WIS RDI-L consequent actions Mask <0 to 3>\n"
        " 0 bit -  Enable for RDI-L backreporting on LOS \n"
        " 1 bit -  Enable for RDI-L backreporting on LOF \n"
        " 2 bit -  Enable for RDI-L backreporting on LOPC \n"
        " 3 bit -  Eable for RDI-L backreporting on AIS_L \n",
        CLI_PARM_TYPE_WIS_RDIL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<wis_fault>",
        "WIS Fault Condition mask <0 to 10 bits>\n"
        "  0 bit -  Enable fault condition on far-end PLM-P \n"
        "  1 bit -  Enable fault condition on far-end AIS-P \n"
        "  2 bit -  Enable fault condition on RDI-L \n"
        "  3 bit -  Enable fault condition on SEF \n"
        "  4 bit -  Enable fault condition on LOF \n"
        "  5 bit -  Enable fault condition on LOS \n"
        "  6 bit -  Enable fault condition on AIS-L \n"
        "  7 bit -  Enable fault condition on LCD-P \n"
        "  8 bit -  Enable fault condition on PLM-P \n"
        "  9 bit -  Enable fault condition on AIS-P \n"
        " 10 bit -  Enable fault condition on LOP-P \n",
        CLI_PARM_TYPE_WIS_FAULT,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#if defined(VTSS_DAYTONA_WIS_CLI_ALL) 
    {
        "<line_rx>",
        "WIS Line rx force mask <0 to 1 bits>\n"
        "  0 bit -  Force AIS-L configuration \n"
        "  1 bit -  Force RDI-L configuration \n",
        CLI_PARM_TYPE_WIS_LINERX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<line_tx>",
        "WIS Line tx force mask <0 to 1 bits>\n"
        "  0 bit -  Force transmission of AIS-L in the K2 byte \n"
        "  1 bit -  Force transmission of RDI-L in the K2 byte \n",
        CLI_PARM_TYPE_WIS_LINETX,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<path_force>",
        "WIS path_force mask <0 to 1 bits>\n"
        "  0 bit -  Force UNEQ-P configuration \n"
        "  1 bit -  Force RDI-P configuration \n",
        CLI_PARM_TYPE_WIS_PATH_FORCE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "loopback|no_loopback",
        "\nloopback     : Enable loopback\n"
        "no_loopback     : Disable loopback\n"
        "(default: Disable loopback)",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "gen_dis|gen_sqr|gen_prbs31|gen_mix",
        "\tGenerator Mode\n"
        "\tgen_dis    : Disable test\n"
        "\tgen_sqr    : Enable squarevave generator, Only valid for test generator\n"
        "\tgen_prbs31 : Enable prbs31 generator (not supported in Daytona)\n"
        "\tgen_mix    : Enable mixed frequency generator\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "ana_dis|ana_prbs31|ana_mix",
        "Analyser Mode\n"
        "\tana_dis    :Disable test \n"
        "\tana_prbs31 :Enable prbs31 Analyser (not supported in Daytona)\n"
        "\tana_mix    :Enable mixed frequency Analyser\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif    
    {
        "<oh_name>",
        "WIS overhead name keywords, Names are case scencitive.\n"
        "\tALL       :  Key word to configure all overheads in one step.\n" 
        "\tSEC-OH    :  Section overhead \n" 
        "\tLINE-OH   :  Line overhead \n" 
        "\tPATH-OH   :  Path overhead \n" 
        "\tD1-D3     :  Section Data Communications Channel\n" 
        "\tSEC-ORD   :  Section Orderwire\n"
        "\tSUC       :  Section User Channel \n" 
        "\tD4-D12    :  Line Data Communications Channel \n" 
        "\tLINE-ORD  :  Line Orderwire overhead \n"
        "\tAPS-RDIL  :  Automatic protection switch (APS) channel and Line Remote Defect Identifier (RDI-L)\n"
        "\tSYNC      :  Synchronization messaging \n"
        "\tRES-LG    :  Reserved for Line growth  \n"
        "\tC2PL      :  Transmitted C2 path label \n"
        "\tPUC       :  Path User Channel \n"
        "\tPTCM      :  Tandem connection maintenance/Path data channel \n"
        "\tRES-PG    :  Reserved for Path growth \n",
        CLI_PARM_TYPE_WIS_OH_NAME,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<oh_value>",        
        "WIS overheadvalue, Value in hexadecimal format, '0x' prefix is optional\n"
        "\t 1. ALL       :  One Hexadecimal value for all overhead(25 bytes), order is from D1-D3 to RES-PG\n" 
        "\t 2. SEC-OH    :  One Hexadecimal value for Section overhead\n" 
        "\t 3. LINE-OH   :  One Hexadecimal value for Line overhead keyword\n" 
        "\t 4. PATH-OH   :  One Hexadecimal value for Path overhead keyword\n" 
        "\t 5. D1-D3     :  Three bytes hexadecimal value, Ex:0xAABBCC\n" 
        "\t 6. SEC-ORD   :  One byte hexadecimal value, Ex:0xAA\n"
        "\t 6. SUC       :  One byte hexadecimal value, Ex:0xAA\n" 
        "\t 8. D4-D12    :  Nine bytes hexadecimal value, Ex:0xAABBCCDDEEFF112233\n" 
        "\t 9. LINE-ORD  :  One byte hexadecimal value, Ex:0xAA\n"
        "\t10. APS-RDIL  :  Two bytes hexadecimal value, Ex:0xAABB \n"
        "\t11. SYNC      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t12. RES-LG    :  Two bytes hexa decimal value, Ex:0xAABB \n"
        "\t13. C2PL      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t14. PUC       :  One byte hexadecimal value, Ex:0xAA\n"
        "\t15. PTCM      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t16. RES-PG    :  Two bytes hexadecimal value, Ex:0xAABB \n",        
        CLI_PARM_TYPE_WIS_OH_VALUE,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)    
    {
        "<n_ebc_thr_s>",
        "WIS Section error count (B1) threshold\n",
        CLI_PARM_TYPE_WIS_NETS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<n_ebc_thr_l>",
        "WIS Near end line error count (B2) threshold\n",
        CLI_PARM_TYPE_WIS_NETL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<f_ebc_thr_l>",
        "WIS Far end line error count threshold\n",
        CLI_PARM_TYPE_WIS_FETL,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<n_ebc_thr_p>",
        "WIS Path block error count (B3) threshold\n",
        CLI_PARM_TYPE_WIS_NETP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<f_ebc_thr_p>",
        "WIS Far end path error count threshold\n",
        CLI_PARM_TYPE_WIS_FETP,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "single_err|sat_err",
        "WIS Far end path error count threshold\n",
        CLI_PARM_TYPE_KEYWORD,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
    {
        "<force_events>",
        "WIS MASK for Single event or multiple events\n",
        CLI_PARM_TYPE_WIS_FORCE_EVENTS,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif    
    {
        "<pass_thru>",
        "Bit Mask to set Passthrough (16 bits). Section Overhead: (Bit 0 to 6) Line overhead:(Bit 7 to 15)\n"
        "\tBit: 0  -> (tx_j0) j0 Section TTI\n"
        "\tBit: 1  -> (tx_z0) z0  Section growth\n"
        "\tBit: 2  -> (tx_b1) b1 BIP\n"
        "\tBit: 3  -> (tx_e1) e1 order wire\n"
        "\tBit: 4  -> (tx_f1) f1 Section user channel\n"
        "\tBit: 5  -> (tx_dcc_s) Section Data communication channel (D1-D3)\n"
        "\tBit: 6  -> (tx_soh) Section Reserved National and unused bytes\n"
        "\tBit: 7  -> (tx_b2) b2 BIP\n"
        "\tBit: 8  -> (tx_k1) k1 passthrough\n"
        "\tBit: 9  -> (tx_k2) k2 passthrough\n"
        "\tBit: 10 -> (tx_reil) reil\n"
        "\tBit: 11 -> (tx_dcc_l) Section Data communication channel(D4-D12)\n"
        "\tBit: 12 -> (tx_s1) Synchronization messaging\n"
        "\tBit: 13 -> (tx_e2) Order wire\n"
        "\tBit: 14 -> (tx_z1_z2) Reserved for path growth\n"
        "\tBit: 15 -> (tx_loh) Line Reserved National and unused bytes\n",
        CLI_PARM_TYPE_WIS_PASSTHRU,
        CLI_CMD_ID_NONE,
        CLI_PARM_FLAG_SET
    },
#endif /*  VTSS_FEATURE_WIS  */
#endif /* VTSS_ARCH_DAYTONA */
};  /* cli_parm_table */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
#define CLI_TGT_MAX  256   /* Targets */
#else
#define CLI_TGT_MAX  64    /* MAC targets */
#endif

#if defined(VTSS_ARCH_B2)
#define CLI_ADDR_MAX (1<<16) /* 16-bit address space */
#elif defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
#define CLI_ADDR_MAX (1<<14) /* 14-bit address space */
#elif defined(VTSS_ARCH_DAYTONA)
#define CLI_ADDR_MAX (1<<16) /* 16-bit address space */
#else
#define CLI_ADDR_MAX (1<<8)  /* 8-bit address space */
#endif /* VTSS_ARCH_B2 */

#define CLI_BLK_MAX  8   /* Switch blocks */
#define CLI_SUB_MAX  16  /* Switch sub blocks */

#define CLI_PHY_MAX  32  /* PHY register addresses */

#define CLI_PORT_MAX 26  /* Chip ports */

typedef enum {
    daytona_upi,
    daytona_xfi,
    daytona_otn,
    daytona_ewis,
    daytona_pcs,
    daytona_mac,
    daytona_gfp,
    daytona_i1588,
    daytona_efec,
    daytona_efeci7
} daytona_block_id_t;

/* CLI request block */
typedef struct {
    vtss_inst_t      api_inst;  /* API instance */
    vtss_appl_inst_t appl_inst; /* Application instance */
    vtss_board_t     *board;    /* Board */

    int   parm_parsed;

#if defined(VTSS_GPIOS)
    BOOL         gpio_list[VTSS_GPIO_NO_END];
#endif /* VTSS_GPIOS */
    BOOL  set;

  
    BOOL  port_list[VTSS_PORTS+1];

    BOOL  blk_list[CLI_TGT_MAX];
    BOOL  sub_list[CLI_SUB_MAX];
    BOOL  addr_list[CLI_ADDR_MAX];
    BOOL  mmd_list[CLI_ADDR_MAX];
    BOOL  chip_port_list[CLI_PORT_MAX];
#if defined(VTSS_ARCH_B2)
    BOOL  lport_list[VTSS_LPORTS];
    u32   lport;
#endif
    u32   value;
    u32   page;
    u32   gpio_no;
    u32   clk_freq;
    BOOL  modify_frm;
    u32   port;
    u16   mmd_addr;
    u32   addr;


    vtss_port_speed_t speed;
    BOOL              fdx;
    u32               max_length;

    vtss_vid_t vid;
    uchar      mac[6];
    
    vtss_debug_layer_t api_layer;
    vtss_debug_group_t api_group;

    vtss_trace_layer_t trace_layer;
    vtss_trace_group_t group;
    vtss_trace_level_t level;
    BOOL egress;
    BOOL ingress;
    BOOL auto_keyword;
    BOOL binary;
    BOOL bytes;
    BOOL clear;
    BOOL discards;
    BOOL decimal;
    BOOL enable;
    BOOL errors;
    BOOL disable;
    BOOL full;
    BOOL packets;
    BOOL rx;
    BOOL tx;
    BOOL warm;
    u32  etype;
    u32  vlan;
    u32  mmd_address;
    u32  class;
    u32  red;
    u32  table_id;
    u32  table_index;
    u32  queue_index;
    u32  trig_index;
    u32  offset;
    u32  pattern;
    u32  mask;
    u32  tcp_udp_port;
    u32  order;
    u32  endpoint;
    u32  ip_proto;
    u32  tag_prio;
    u32  mpls_exp;
    u32  red_min;
    u32  red_max;
    u32  red_weight;
    u32  red_prob1;
    u32  red_prob2;
    u32  red_prob3;
    u32  rate;
#if defined(VTSS_CHIP_10G_PHY)     
    vtss_phy_10g_loopback_t lb;
#endif /* VTSS_CHIP_10G_PHY */
#if defined(VTSS_ARCH_B2)     
    BOOL  dscp_list[VTSS_DSCP_TABLE_SIZE];
    vtss_port_filter_t filter;
    vtss_policer_t policer;
#endif
    BOOL max_tags;
    BOOL allow;
    BOOL range;
    BOOL forward;
    BOOL def;
    BOOL cfi;
    BOOL ipv4;
    BOOL ipv6;
    BOOL llc;
    BOOL l2_ctrl;
    BOOL l3_ctrl;
    BOOL ctag;
    BOOL vlan_aware;
    BOOL tag_inner;
    BOOL alt;
    BOOL transmit;
    int layer;
    uchar dmac[6];
    uchar dmac_mask[6];
#if defined(VTSS_FEATURE_WIS)
    u8      wis_overhead;
    u8      wis_tti_mode;
    u8      wis_gen_mode;
    u8      wis_ana_mode;
    u8      wis_tti[64];
    u16     wis_aisl;
    u16     wis_rdil;
    u16     wis_fault;
    u8      wis_oh_id;
    u8      wis_oh_val[26];
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
    u8      wis_line_rx;
    u8      wis_line_tx;
    u8      wis_path_force;
    u32     wis_n_ebc_thr_s;
    u32     wis_n_ebc_thr_l;
    u32     wis_f_ebc_thr_l;
    u32     wis_n_ebc_thr_p;
    u32     wis_f_ebc_thr_p;
    u32     wis_force_events;
#endif	
    u8      wis_err_inj;
    u32     sl;
    u32     wis_passthru;
    BOOL    wis_gen_mode_set;
    BOOL    wis_ana_mode_set;
    BOOL    wis_tti_mode_set;
    BOOL    wis_overhead_set;
    BOOL    wis_tti_set;
    BOOL    wis_loopback;
    BOOL    wis_err_inj_set;
    BOOL    wis_loopback_set;
    BOOL    ais_on_los;
    BOOL    ais_on_lof;
    BOOL    rdil_on_los;
    BOOL    rdil_on_lof;
    BOOL    rdil_on_ais;
    BOOL    wis_rx_force;
    BOOL    wis_tx_force;
    BOOL    wis_forced_ais_l;
    BOOL    wis_forced_rdi_l;
    BOOL    wis_pn_ebc_mode_s;
    BOOL    wis_pn_ebc_mode_l;
    BOOL    wis_pf_ebc_mode_l;
    BOOL    wis_pn_ebc_mode_p;
    BOOL    wis_pf_ebc_mode_p;
    BOOL    wis_perf_mode;
    BOOL    wis_perf_mode_bit;
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_UPI) || defined(VTSS_FEATURE_XFI)|| defined(VTSS_FEATURE_XAUI)
    BOOL upi_set_lane;
    BOOL xxx_equip_lb;
    BOOL xxx_line_lb;
    BOOL xxx_set_lb;
    BOOL xfi_sel_lane_b;
    BOOL  xfi_lane_A;
    BOOL  xfi_lane_B;
    u32  upi_gen_mode;
    u32  upi_chk_mode;
    u32  upi_wrp_mode;
    u32  upi_txeq_mode;
    u32  upi_txeq_chnloss;
    u32  upi_coef_id;
    u32  upi_coef_step;
    u32  upi_incr;
    u32  upi_polarity;
    u32  upi_power;
    u32  upi_slew;
    u32  upi_rxeq_mode;
    u32  xfi_txeq_mode;
    u32  xfi_txeq_chnloss;
    u32  xfi_txeq_coef;
    u32  xfi_txeq_incr;
    u32  xfi_txeq_polarity;
    u32  xfi_txeq_step;
    u32  xfi_misc_parm_value;
    u8   xfi_txeq_misc_parm;
    u32  xfi_rxeq_mode;
    u32  xfi_802a_txeq_upd;
    u32  xfi_802a_txeq_lane;
    u32  xfi_802a_txeq_coef;
    u32  xfi_802a_txeq_fsm;
#endif
#if defined(VTSS_FEATURE_OTN)
    u8 otn_tti[64];
    BOOL otn_tti_set;
    BOOL otn_extti_set;
    BOOL otn_sapi_tim;
    BOOL otn_dapi_tim;
    BOOL otn_os_tim;
    u32  otn_value;
    vtss_otn_odu_oh_ins_mode_t  oh_ins_mode;
    vtss_otn_odu_opu_oh_ins_mode_t  opu_oh_ins_mode;
    vtss_otn_odu_opu_test_ins_mode_t  test_ins_mode;
    u32  ftfl_value;
    u32  ftfl_index;
    u8   otn_extti[64];
    u32  si_ais;
    u32  so_ais;
    u32  beibiae;
    u32  bdi;
    u32  ais;
    u32  iae;
    u32  bdi_los;
    u32  bdi_lof;
    u32  bdi_lom;
    u32  bdi_ais;
    u32  bdi_tim;
    u32  ais_lom;
    u32  ais_tim;
    u32  bei;
    u32  odu_bdi;
    u32  odu_bdi_los;
    u32  odu_bdi_lof;
    u32  odu_bdi_lom;
    u32  odu_bdi_otu_ais;
    u32  odu_bdi_otu_tim;
    u32  odu_bdi_ais;
    u32  odu_bdi_oci;
    u32  odu_bdi_lck;
    u32  odu_bdi_ltc;
    u32  odu_bdi_tim;
    u32  odu_ais;
    u32  odu_ais_timocilck;
    u32  odu_ais_plm;
    u32  odu_pl_plm;
    u32  odu_pl_tx;
    u32  odu_pl_type;
    vtss_otn_odu_mode_mode_t  odu_mode;
    vtss_otn_odut_mode_mode_t odut_mode;
    vtss_otn_odut_bdi_mode_t  odut_bdi;
    u32                       odut_level;
    u32                       odut_aps_count;
    u8                        odut_aps[VTSS_OTN_ODUT_APS_NUMBER];
    u32                       odut_stat;
    vtss_otn_och_fec_type_t   fec_type;
    u32                       ber_threshold;  
    BOOL                      xxx_sys_side_equip_lb;
    BOOL                      xxx_line_side_equip_lb;
    BOOL                      xxx_line_side_line_lb;
    BOOL                      xxx_sys_side_line_lb;
#endif /* VTSS_FEATURE_OTN */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    u8          port_no;
    u8          eng_id;
    u8          clkfreq;
    BOOL        clk_freq_spec;
    u8          clk_src;
    BOOL        clk_src_spec;
    u8          rx_ts_pos;
    BOOL        rx_ts_pos_spec;
    u8          tx_fifo_mode;
    BOOL        tx_fifo_mode_spec;
    u8          encap_type;
    u8          flow_st_index;
    u8          flow_end_index;
    u8          flow_match_mode;
    u8          channel_map;
    BOOL        flow_id_spec;
    u8          flow_id;
    BOOL        pbb_spec;
    BOOL        pbb_en;
    u16         tpid;
    BOOL        mac_match_mode_spec;
    u8          mac_match_mode;
    BOOL        ptp_mac_spec;
    u8          ptp_mac[6];
    BOOL        vlan_chk_spec;
    BOOL        vlan_chk;
    BOOL        num_tag_spec;
    u8          num_tag;
    BOOL        tag_rng_mode_spec;
    u8          tag_rng_mode;
    u8          tag1_type;
    u8          tag2_type;
    u16         tag1_lower;
    u16         tag1_upper;
    u16         tag2_lower;
    u16         tag2_upper;
    BOOL        ip_mode_spec;
    u8          ip_mode;
    BOOL        sport_spec;
    u16         sport_val;
    u16         sport_mask;
    BOOL        dport_spec;
    u16         dport_val;
    u16         dport_mask;
    BOOL        addr_match_select_spec;
    u8          addr_match_select;
    BOOL        ipv4_addr_spec;
    vtss_ipv4_t ipv4_addr;
    BOOL        ipv4_mask_spec;
    vtss_ipv4_t ipv4_mask;
    u32         ipv6_addr[4];
    u32         ipv6_mask[4];
    BOOL        stk_depth_spec;
    u8          stk_depth;
    BOOL        stk_ref_point_spec;
    u8          stk_ref_point;
    BOOL        stk_lvl_0;
    u32         stk_lvl_0_lower;
    u32         stk_lvl_0_upper;
    BOOL        stk_lvl_1;
    u32         stk_lvl_1_lower;
    u32         stk_lvl_1_upper;
    BOOL        stk_lvl_2;
    u32         stk_lvl_2_lower;
    u32         stk_lvl_2_upper;
    BOOL        stk_lvl_3;
    u32         stk_lvl_3_lower;
    u32         stk_lvl_3_upper;
    BOOL        cw_en;
    BOOL        ach_ver_spec;
    u8          ach_ver;
    BOOL        channel_type_spec;
    u16         channel_type;
    BOOL        proto_id_spec;
    u16         proto_id;
    u8          action_id;
    BOOL        ptp_spec;
    u8          clk_mode;
    u8          delaym;
    u8          domain_meg_lower;
    u8          domain_meg_upper;
    BOOL        y1731_oam_spec;
    BOOL        ietf_oam_spec;
    u8          ietf_tf;
    u8          ietf_ds;
    u8          sig_mask;
    u8          version;
    u8          time_sec;
    vtss_timeinterval_t  latency_val;
    vtss_timeinterval_t  delay_val; 
    vtss_timeinterval_t  asym_val;
    BOOL    phy_mmd_access; // Indicate if a phy access is a MMD access.
    u32     blk_id_1588;         /* 1588 internal block Id's */
    u32     csr_reg_offset_1588; /* 1588 Internal block's CSR register offset*/
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_FEATURE_GFP)
    u32  forced_interval;
    u32  upi;
    u32  pti;
    u32  cmf_fr_len;
    u32  ordered_upi;
    BOOL core_enable;
    BOOL pay_enable;
    BOOL trans_enable;
    vtss_gfp_cmf_sf_t   los_sf;
    vtss_gfp_cmf_sf_t   lsync_sf;
    vtss_gfp_8b10b_err_code_t   err_code;
    BOOL force_los;
    BOOL force_lsync;
#endif /* VTSS_FEATURE_GFP */

#if defined(VTSS_FEATURE_XAUI)
#if 0
    u32  xaui_q_code;
    BOOL xaui_set_q_code;
#endif
#endif
#if defined(VTSS_FEATURE_MAC10G)
    BOOL mac10g_tx_rx_loopback;
    BOOL mac10g_rx_tx_loopback;
    u32  mac10g_max_frame_len;
    u32  mac10g_max_tags;
#endif /* VTSS_FEATURE_MAC10G */
#if defined(VTSS_FEATURE_RAB)
    u32  fifo_thresh_low;
    u32  fifo_thresh_high;
    u32  fifo_thresh_adapt;
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_I2C)
    u32  i2c_controller_no;
    u32  i2c_dev_addr;
    u32  i2c_reg_addr;
    u32  i2c_data;
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_ARCH_DAYTONA )
    daytona_block_id_t block;
    vtss_upi_event_t upi_event;
    vtss_xfi_event_t xfi_event;
    vtss_gfp_event_t gfp_event;
    vtss_phy_ts_event_t i1588_event;
    vtss_otn_och_event_t otn_och_event;
    vtss_otn_otu_event_t otn_otu_event;
    vtss_otn_odu_event_t otn_odu_event;
    vtss_otn_odu_event_t otn_odut_event;
    vtss_ewis_event_t ewis_event;
    vtss_pcs_10gbase_r_event_t pcs_event;
    vtss_mac10g_event_t mac_event;
    vtss_efec_event_t efec_event;
    vtss_efeci7_event_t efeci7_event;
    vtss_pmtick_mode_t  pmtick_mode;
    u32                 pmtick_interval;
    u32 channel_num;
    u32 channel_mode;
    u32 channel_xauimode;
    u32 ev_init;
#endif /* VTSS_ARCH_DAYTONA */
#if defined(DAYTONA_EVAL_INIT)
   u32 lbfpga_reg;
   u32 pcf8575_tgtid;
   u32 vsc3172_reg;
   u32 si5338_reg;
   u32 si5338_tgtid;
   u32 si5326_reg;
   u32 si5326_tgtid;
   u32 fpga_mask;
   u32 val_mask;
#endif
#if defined(VTSS_FEATURE_OHA)
   u32 oha_rosi_tosi;
   u32 oha_otn;
#endif
} cli_req_t;

static cli_req_t cli_req;

#ifndef VTSS_ARCH_DAYTONA
// Function that returns port interface as a string. 
// Note : It takes the internal port number as argument.
static char *if2str(vtss_appl_inst_t appl_inst, vtss_port_no_t iport) {
  vtss_port_interface_t   if_type;
  static char port_str[25];
  
  // Get the interface type 
  vtss_appl_port_if_get(appl_inst, iport, &if_type);
  switch (if_type) {
  case VTSS_PORT_INTERFACE_SPI4 :
    sprintf(port_str, "SPI4"); 
    break;
  case VTSS_PORT_INTERFACE_XAUI :
    sprintf(port_str, "XAUI");
    break;
  case VTSS_PORT_INTERFACE_LOOPBACK :
    sprintf(port_str, "LOOPBACK");
    break;
  case VTSS_PORT_INTERFACE_RTBI :
    sprintf(port_str, "RTBI");
    break;
  case  VTSS_PORT_INTERFACE_INTERNAL :
    sprintf(port_str, "INTERNAL");
    break;
  case VTSS_PORT_INTERFACE_MII:
    sprintf(port_str, "MII");
    break;
  case VTSS_PORT_INTERFACE_GMII :
    sprintf(port_str, "GMII");
    break;
  case VTSS_PORT_INTERFACE_RGMII:
    sprintf(port_str, "RGMII");
    break;
  case  VTSS_PORT_INTERFACE_TBI :
    sprintf(port_str, "TBI");
    break;
  case VTSS_PORT_INTERFACE_SGMII:
    sprintf(port_str, "SGMII");
    break;
  case VTSS_PORT_INTERFACE_SERDES:
    sprintf(port_str, "SERDES");
    break;
  case VTSS_PORT_INTERFACE_VAUI:
    sprintf(port_str, "VAUI");
    break;
  case VTSS_PORT_INTERFACE_100FX:
    sprintf(port_str, "100FX");
    break;
  case VTSS_PORT_INTERFACE_XGMII:
    sprintf(port_str, "XGMII");
    break;
  default:
    sprintf(port_str, "No CONNECTION");
  }


  return port_str;
}
#endif /* ifndef VTSS_ARCH_DAYTONA */

static char *cli_parse_find(char *cmd, char *stx)
{
    char *p, *found = NULL;
    BOOL parm = 0, start = 1;

    for (p = stx; *p != '\0'; p++) {
        if (parm) {
            /* Look for parameter end character */
            if (*p == '>')
                parm = 0;
        } else if (*p == '<') {
            /* Parameter start character */
            parm = 1;
        } else if (*p == '|' || *p == '[' || *p == '(') {
            /* Next argument character */
            start = 1;
        } else if (start) {
            start = 0;
            if (strstr(p, cmd) == p) {
                if (found != NULL)
                    return NULL;
                found = p;
            }
        }
    }
    return found;
}

static int cli_parse_keyword(char *cmd, cli_req_t *req, char *stx)
{
    char *found;
    
    if ((found = cli_parse_find(cmd, stx)) == NULL)
        return 1;
   
    if (!strncmp(found, "auto", 4))
        req->auto_keyword = 1;
    else if (!strncmp(found, "ail", 3))
        req->api_layer = VTSS_DEBUG_LAYER_AIL;
    else if (!strncmp(found, "binary", 6))
        req->binary = 1;
    else if (!strncmp(found, "bytes", 5))
        req->bytes = 1;
    else if (!strncmp(found, "clear", 5))
        req->clear = 1;
    else if (!strncmp(found, "cool", 4))
        req->warm = 0;
    else if (!strncmp(found, "cil", 3))
        req->api_layer = VTSS_DEBUG_LAYER_CIL;
    else if (!strncmp(found, "debug", 5))
        req->level = VTSS_TRACE_LEVEL_DEBUG;
    else if (!strncmp(found, "discards", 8))
        req->discards = 1;
    else if (!strncmp(found, "disable", 7))
        req->disable = 1;
    else if (!strncmp(found, "decimal", 7))
        req->decimal = 1;
    else if (!strncmp(found, "errors", 6))
        req->errors = 1;
    else if (!strncmp(found, "error", 5))
        req->level = VTSS_TRACE_LEVEL_ERROR;
    else if (!strncmp(found, "enable", 6))
        req->enable = 1;
    else if (!strncmp(found, "full", 4))
        req->full = 1;
    else if (!strncmp(found, "info", 4))
        req->level = VTSS_TRACE_LEVEL_INFO;
    else if (!strncmp(found, "noise", 5))
        req->level = VTSS_TRACE_LEVEL_NOISE;
    else if (!strncmp(found, "off", 3))
        req->level = VTSS_TRACE_LEVEL_NONE;
    else if (!strncmp(found, "packets", 7))
        req->packets = 1;
    else if (!strncmp(found, "warm", 4))
        req->warm = 1;
    else if (!strncmp(found, "10hdx", 5))
        req->speed = VTSS_SPEED_10M;
    else if (!strncmp(found, "10fdx", 5)) {
        req->speed = VTSS_SPEED_10M;
        req->fdx = 1;
    } else if (!strncmp(found, "100hdx", 6))
        req->speed = VTSS_SPEED_100M;
    else if (!strncmp(found, "100fdx", 6)) { 
        req->speed = VTSS_SPEED_100M;
        req->fdx = 1;
    } else if (!strncmp(found, "1000fdx", 7)) {
        req->speed = VTSS_SPEED_1G;
        req->fdx = 1;    
#if defined(VTSS_ARCH_B2)
    } else if (!strncmp(found, "allow", 5)) {
        req->allow = 1;
    } else if (!strncmp(found, "deny", 4)) {
        req->allow = 0;
    } else if (!strncmp(found, "mac_ctrl", 8)) {
        req->filter.mac_ctrl_enable = 1;
    } else if (!strncmp(found, "mac_zero", 8)) {
        req->filter.mac_zero_enable = 1;
    } else if (!strncmp(found, "dmac_bc", 7)) {
        req->filter.dmac_bc_enable = 1;
    } else if (!strncmp(found, "smac_mc", 7)) {
        req->filter.smac_mc_enable = 1;
    } else if (!strncmp(found, "untag", 5)) {
        req->filter.untag_enable = 1;
    } else if (!strncmp(found, "prio_tag", 8)) {
        req->filter.prio_tag_enable = 1;
    } else if (!strncmp(found, "c_tag", 5)) {
        req->filter.ctag_enable = 1;
    } else if (!strncmp(found, "s_tag", 5)) {
        req->filter.stag_enable = 1;
    } else if (!strncmp(found, "tags", 4)) {
        req->max_tags = 1;
    } else if (!strncmp(found, "any_tag", 4)) {
        req->filter.max_tags = VTSS_TAG_ANY;
    } else if (!strncmp(found, "1_tag", 5)) {
        req->filter.max_tags = VTSS_TAG_ONE;
    } else if (!strncmp(found, "2_tags", 6)) {
        req->filter.max_tags = VTSS_TAG_TWO;
    } else if (!strncmp(found, "range", 5)) {
        req->range = 1;
    } else if (!strncmp(found, "specific", 8)) {
        req->range = 0;
    } else if (!strncmp(found, "drop", 4)) {
        req->forward = 0;
    } else if (!strncmp(found, "forward", 7)) {
        req->forward = 1;
    } else if (!strncmp(found, "l2_ctrl", 7)) {
        req->l2_ctrl = 1;
    } else if (!strncmp(found, "l3_ctrl", 7)) {
        req->l3_ctrl = 1;
    } else if (!strncmp(found, "l2", 2)) {
        req->layer = VTSS_HEADER_L2;
    } else if (!strncmp(found, "l3", 2)) {
        req->layer = VTSS_HEADER_L3;
    } else if (!strncmp(found, "l4", 2)) {
        req->layer = VTSS_HEADER_L4;
    } else if (!strncmp(found, "default", 7)) {
        req->def = 1;
    } else if (!strncmp(found, "cfi", 3)) {
        req->cfi = 1;
    } else if (!strncmp(found, "ipv4", 4)) {
        req->ipv4 = 1;
    } else if (!strncmp(found, "ipv6", 4)) {
        req->ipv6 = 1;
    } else if (!strncmp(found, "llc", 3)) {
        req->llc = 1;
    } else if (!strncmp(found, "ctag", 4)) {
        req->ctag = 1;
    } else if (!strncmp(found, "tag_inner", 9)) {
        req->tag_inner = 1;
    } else if (!strncmp(found, "vlan_aware", 10)) {
        req->vlan_aware = 1;
    } else if (!strncmp(found, "prim", 4)) {
        req->alt = 0;
    } else if (!strncmp(found, "rx", 2)) {
        req->rx = 1;
    } else if (!strncmp(found, "tx", 2)) {
        req->tx = 1;
    } else if (!strncmp(found, "egress", 6)) {
        req->egress = 1;
    } else if (!strncmp(found, "ingress", 7)) {
        req->ingress = 1;
    } else if (!strncmp(found, "both", 4)) {
        req->value = 3;
    } else if (!strncmp(found, "alt", 3)) {
        req->alt = 1;
    } else if (!strncmp(found, "uc", 2)) {
        req->policer.unicast = 1;
    } else if (!strncmp(found, "mc", 2)) {
        req->policer.multicast = 1;
    } else if (!strncmp(found, "bc", 2)) {
        req->policer.broadcast = 1;
    } else if (!strncmp(found, "all", 3)) {
        req->policer.broadcast = 1;
        req->policer.unicast = 1;
        req->policer.multicast = 1;
#endif /* B2 */
#if defined(VTSS_CHIP_10G_PHY)
    } else if (!strncmp(found, "a", 1)) {
        req->lb.lb_type = VTSS_LB_NETWORK_XS_DEEP;
    } else if (!strncmp(found, "b", 1)) {
        req->lb.lb_type = VTSS_LB_SYSTEM_XS_SHALLOW;
    } else if (!strncmp(found, "c", 1)) {
        req->lb.lb_type = VTSS_LB_SYSTEM_XS_DEEP;
    } else if (!strncmp(found, "d", 1)) {
        req->lb.lb_type = VTSS_LB_NETWORK_XS_SHALLOW;
    } else if (!strncmp(found, "e", 1)) {
        req->lb.lb_type = VTSS_LB_SYSTEM_PCS_SHALLOW;
    } else if (!strncmp(found, "f", 1)) {
        req->lb.lb_type = VTSS_LB_NETWORK_PCS;
    } else if (!strncmp(found, "g", 1)) {
        req->lb.lb_type = VTSS_LB_SYSTEM_PCS_DEEP;
    } else if (!strncmp(found, "h", 1)) {
        req->lb.lb_type = VTSS_LB_NETWORK_WIS;
    } else if (!strncmp(found, "j", 1)) {
        req->lb.lb_type = VTSS_LB_SYSTEM_PMA;
    } else if (!strncmp(found, "k", 1)) {
        req->lb.lb_type = VTSS_LB_NETWORK_PMA;
#endif /* VTSS_CHIP_10G_PHY */
#if defined(VTSS_ARCH_DAYTONA )
    } else if (!strncmp(found, "equip_loopback", 14)) {
        req->xxx_equip_lb = TRUE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "line_loopback", 13)) {
        req->xxx_line_lb = TRUE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "line_side_equip", 15)) {
        req->xxx_line_side_equip_lb = TRUE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "line_side_line", 14)) {
        req->xxx_line_side_line_lb = TRUE; 
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "sys_side_equip", 14)) {
        req->xxx_sys_side_equip_lb = TRUE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "sys_side_line", 13)) {
        req->xxx_sys_side_line_lb = TRUE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "no_loopback", 11)) {
        req->xxx_equip_lb = FALSE;
        req->xxx_line_lb = FALSE;
        req->xxx_line_side_equip_lb = FALSE;
        req->xxx_line_side_line_lb = FALSE; 
        req->xxx_sys_side_equip_lb = FALSE;
        req->xxx_sys_side_line_lb = FALSE;
        req->xxx_set_lb = TRUE;
    } else if (!strncmp(found, "a_lane_xfi", 10)) {
        req->xfi_sel_lane_b = FALSE;
        req->upi_set_lane = TRUE;
    } else if (!strncmp(found, "b_lane_xfi", 10)) {
        req->xfi_sel_lane_b = TRUE;
        req->upi_set_lane = TRUE;
    } else if (!strncmp(found, "lane_a", 6)) {
        req->xfi_lane_A = TRUE;
    } else if (!strncmp(found, "lane_b", 6)) {
        req->xfi_lane_B = TRUE;
    } else if (!strncmp(found, "slew", 4)) {
        req->xfi_txeq_misc_parm = 1;
    } else if (!strncmp(found, "power", 5)) {
        req->xfi_txeq_misc_parm = 2;
    } else if (!strncmp(found, "rx_tx_loopback", 14)) {
        req->mac10g_rx_tx_loopback = TRUE;
        req->mac10g_tx_rx_loopback = FALSE;
    } else if (!strncmp(found, "tx_rx_loopback", 14)) {
        req->mac10g_rx_tx_loopback = FALSE;
        req->mac10g_tx_rx_loopback = TRUE;
    } else if (!strncmp(found, "no_loopback", 11)) {
        req->mac10g_rx_tx_loopback = FALSE;
        req->mac10g_tx_rx_loopback = FALSE;
    } else if (!strncmp(found, "upi", 3)) {
        req->block = daytona_upi;
    } else if (!strncmp(found, "xfi", 3)) {
        req->block = daytona_xfi;
    } else if (!strncmp(found, "otn", 3)) {
        req->block = daytona_otn;
    } else if (!strncmp(found, "ewis", 4)) {
        req->block = daytona_ewis;
    } else if (!strncmp(found, "pcs", 3)) {
        req->block = daytona_pcs;
    } else if (!strncmp(found, "mac", 3)) {
        req->block = daytona_mac;
    } else if (!strncmp(found, "gfp", 3)) {
        req->block = daytona_gfp;
    } else if (!strncmp(found, "i1588", 5)) {
        req->block = daytona_i1588;
    } else if (!strncmp(found, "efeci4", 6)) {
        req->block = daytona_efec;
    } else if (!strncmp(found, "efeci7", 6)) {
        req->block = daytona_efeci7;    
    } else if (!strncmp(found, "ais_on_los", 10)) {
        req->ais_on_los = TRUE;
    } else if (!strncmp(found, "ais_on_lof", 10)) {
        req->ais_on_lof = TRUE;
    } else if (!strncmp(found, "rdil_on_los", 11)) {
        req->rdil_on_los = TRUE;
    } else if (!strncmp(found, "rdil_on_lof", 11)) {
        req->rdil_on_lof = TRUE;
    } else if (!strncmp(found, "rdil_on_ais", 11)) {
        req->rdil_on_ais = TRUE;
    } else if (!strncmp(found, "rx_force", 8)) {
        req->wis_rx_force = TRUE;
    } else if (!strncmp(found, "tx_force", 8)) {
        req->wis_tx_force = TRUE;
    } else if (!strncmp(found, "ais_l", 5)) {
        req->wis_forced_ais_l = TRUE;
    } else if (!strncmp(found, "rdi_l", 5)) {
        req->wis_forced_rdi_l = TRUE;
    } else if (!strncmp(found, "pn_ebc_mode_s", 13)) {
        req->wis_pn_ebc_mode_s = TRUE;
    } else if (!strncmp(found, "pn_ebc_mode_l", 13)) {
        req->wis_pn_ebc_mode_l = TRUE;
    } else if (!strncmp(found, "pf_ebc_mode_l", 13)) {
        req->wis_pf_ebc_mode_l = TRUE;
    } else if (!strncmp(found, "pn_ebc_mode_p", 13)) {
        req->wis_pn_ebc_mode_p = TRUE;
    } else if (!strncmp(found, "pf_ebc_mode_p", 13)) {
        req->wis_pf_ebc_mode_p = TRUE;
    } else if (!strncmp(found, "bit", 3)) {
        req->wis_perf_mode = TRUE;
        req->wis_perf_mode_bit = FALSE;
    } else if (!strncmp(found, "block", 4)) {
        req->wis_perf_mode = TRUE;
        req->wis_perf_mode_bit = TRUE;
    } else if (!strncmp(found, "disabled", 8)) {
        req->pmtick_mode = VTSS_MISC_PMTICK_DISABLE;
    } else if (!strncmp(found, "internal", 8)) {
        req->pmtick_mode = VTSS_MISC_PMTICK_INTERNAL;
    } else if (!strncmp(found, "external", 8)) {
        req->pmtick_mode = VTSS_MISC_PMTICK_EXTERNAL;
#endif /* VTSS_ARCH_DAYTONA */
#if defined(VTSS_FEATURE_OTN)
    } else if (!strncmp(found, "sapi_tim_en", 11)) {
        req->otn_sapi_tim = TRUE;
    } else if (!strncmp(found, "sapi_tim_dis", 12)) {
        req->otn_sapi_tim = FALSE;
    } else if (!strncmp(found, "dapi_tim_en", 11)) {
        req->otn_dapi_tim = TRUE;
    } else if (!strncmp(found, "dapi_tim_dis", 12)) {
        req->otn_dapi_tim = FALSE;
    } else if (!strncmp(found, "os_tim_en", 9)) {
        req->otn_os_tim = TRUE;
    } else if (!strncmp(found, "os_tim_dis", 10)) {
        req->otn_os_tim = FALSE;
    } else if (!strncmp(found, "none_fec", 8)) {
        req->fec_type = VTSS_OTN_OCH_FEC_NONE;
    } else if (!strncmp(found, "rs_fec", 6)) {
        req->fec_type = VTSS_OTN_OCH_FEC_RS;
    } else if (!strncmp(found, "i4_fec", 6)) {
        req->fec_type = VTSS_OTN_OCH_FEC_I4;
    } else if (!strncmp(found, "i7_fec", 6)) {
        req->fec_type = VTSS_OTN_OCH_FEC_I7;
    } else if (!strncmp(found, "unforced_bdi", 12)) {
        req->odut_bdi = VTSS_OTN_ODUT_BDI_UNFORCED;
    } else if (!strncmp(found, "one_bdi", 7)) {
        req->odut_bdi = VTSS_OTN_ODUT_BDI_FORCED_HIGH;
    } else if (!strncmp(found, "zero_bdi", 8)) {
        req->odut_bdi = VTSS_OTN_ODUT_BDI_FORCED_LOW;
    } else if (!strncmp(found, "termination", 11)) {
        req->odu_mode = VTSS_OTN_ODU_MODE_TERMINATION;
        req->odut_mode = VTSS_OTN_ODUT_MODE_TERMINATION;
    } else if (!strncmp(found, "monitor", 7)) {
        req->odu_mode = VTSS_OTN_ODU_MODE_MONITOR;
        req->odut_mode = VTSS_OTN_ODUT_MODE_MONITOR;
#endif
#if defined(VTSS_FEATURE_GFP)
    } else if (!strncmp(found, "core_en", 7)) {
        req->core_enable = TRUE;
    } else if (!strncmp(found, "core_dis", 8)) {
        req->core_enable = FALSE;
    } else if (!strncmp(found, "pyld_en", 7)) {
        req->pay_enable = TRUE;
    } else if (!strncmp(found, "pyld_dis", 8)) {
        req->pay_enable = FALSE;
    } else if (!strncmp(found, "trans_en", 8)) {
        req->trans_enable = TRUE;
    } else if (!strncmp(found, "trans_dis", 9)) {
        req->trans_enable = FALSE;
    } else if (!strncmp(found, "los_none", 8)) {
        req->los_sf = VTSS_GFP_CMF_SF_NONE;
    } else if (!strncmp(found, "los_hw", 6)) {
        req->los_sf = VTSS_GFP_CMF_SF_HW;
    } else if (!strncmp(found, "los_on", 6)) {
        req->los_sf = VTSS_GFP_CMF_SF_FORCED_ON;
    } else if (!strncmp(found, "los_off", 7)) {
        req->los_sf = VTSS_GFP_CMF_SF_FORCED_OFF;
    } else if (!strncmp(found, "lsync_none", 10)) {
        req->lsync_sf = VTSS_GFP_CMF_SF_NONE;
    } else if (!strncmp(found, "lsync_hw", 8)) {
        req->lsync_sf = VTSS_GFP_CMF_SF_HW;
    } else if (!strncmp(found, "lsync_on", 8)) {
        req->lsync_sf = VTSS_GFP_CMF_SF_FORCED_ON;
    } else if (!strncmp(found, "lsync_off", 9)) {
        req->lsync_sf = VTSS_GFP_CMF_SF_FORCED_OFF;
    } else if (!strncmp(found, "los_force_on", 12)) {
        req->force_los = TRUE;
    } else if (!strncmp(found, "los_force_off", 13)) {
        req->force_los = FALSE;
    } else if (!strncmp(found, "lsync_force_on", 14)) {
        req->force_lsync = TRUE;
    } else if (!strncmp(found, "lsync_force_off", 15)) {
        req->force_lsync = FALSE;
    } else if (!strncmp(found, "10b_err", 7)) {
        req->err_code = VTSS_GFP_10B_ERR_ERROR_CODE;
    } else if (!strncmp(found, "k30_7", 5)) {
        req->err_code = VTSS_GFP_K30_7_ERROR_CODE;
#endif
#if defined(VTSS_FEATURE_WIS)
    } else if (!strncmp(found, "gen_dis", 7)) {
        req->wis_gen_mode_set = TRUE;
        req->wis_gen_mode= VTSS_WIS_TEST_MODE_DISABLE;
    } else if (!strncmp(found, "gen_sqr", 7)) {
        req->wis_gen_mode_set = TRUE;
        req->wis_gen_mode = VTSS_WIS_TEST_MODE_SQUARE_WAVE;
    } else if (!strncmp(found, "gen_prbs31", 10)) {
        req->wis_gen_mode_set = TRUE;
        req->wis_gen_mode = VTSS_WIS_TEST_MODE_PRBS31;
    } else if (!strncmp(found, "gen_mix", 7)) {
        req->wis_gen_mode_set = TRUE;
        req->wis_gen_mode = VTSS_WIS_TEST_MODE_MIXED_FREQUENCY;
    } else if (!strncmp(found, "ana_dis", 7)) {
        req->wis_ana_mode_set = TRUE;
        req->wis_ana_mode = VTSS_WIS_TEST_MODE_DISABLE;
    } else if (!strncmp(found, "ana_prbs31", 10)) {
        req->wis_ana_mode_set = TRUE;
        req->wis_ana_mode = VTSS_WIS_TEST_MODE_PRBS31;
    } else if (!strncmp(found, "ana_mix", 7)) {
        req->wis_ana_mode_set = TRUE;
        req->wis_ana_mode = VTSS_WIS_TEST_MODE_MIXED_FREQUENCY;
    } else if (!strncmp(found, "loopback", 8)) {
        req->wis_loopback = TRUE;
        req->wis_loopback_set = TRUE;
    } else if (!strncmp(found, "no_loopback", 11)) {
        req->wis_loopback_set = TRUE;
        req->wis_loopback = FALSE;
    } else if (!strncmp(found, "single_err", 10)) {
        req->wis_err_inj_set = TRUE;
        req->wis_err_inj = 0;
    } else if (!strncmp(found, "sat_err", 7)) {
        req->wis_err_inj_set = TRUE;
        req->wis_err_inj = 1;
#endif /*  VTSS_FEATURE_WIS  */

    } else
        printf("no match:%s\n",found);
    
    return 0;
}

#if defined(VTSS_ARCH_DAYTONA)
static int cli_parse_event(char *cmd, cli_req_t *req, char *stx)
{
    char *found;

    found = cmd;
    if (req->block == daytona_upi) {
        if (!strncmp(found, "rx_hss_prbssync_0", 17))        req->upi_event = VTSS_UPI_RX_HSS_PRBSSYNC_0_EV;
        else if (!strncmp(found, "rx_hss_prbssync_1", 17))   req->upi_event = VTSS_UPI_RX_HSS_PRBSSYNC_1_EV;
        else if (!strncmp(found, "rx_hss_prbssync_2", 17))   req->upi_event = VTSS_UPI_RX_HSS_PRBSSYNC_2_EV;
        else if (!strncmp(found, "rx_hss_prbssync_3", 17))   req->upi_event = VTSS_UPI_RX_HSS_PRBSSYNC_3_EV;
        else if (!strncmp(found, "tx_hss_prbssync_0", 17))   req->upi_event = VTSS_UPI_TX_HSS_PRBSSYNC_0_EV;
        else if (!strncmp(found, "tx_hss_prbssync_1", 17))   req->upi_event = VTSS_UPI_TX_HSS_PRBSSYNC_1_EV;
        else if (!strncmp(found, "tx_hss_prbssync_2", 17))   req->upi_event = VTSS_UPI_TX_HSS_PRBSSYNC_2_EV;
        else if (!strncmp(found, "tx_hss_prbssync_3", 17))   req->upi_event = VTSS_UPI_TX_HSS_PRBSSYNC_3_EV;
        else if (!strncmp(found, "hss_resetout", 12))        req->upi_event = VTSS_UPI_HSS_RESETOUT_EV;
        else if (!strncmp(found, "rx_los", 6))               req->upi_event = VTSS_UPI_RX_LOS_EV;
        else if (!strncmp(found, "rx_sigdet_0", 11))         req->upi_event = VTSS_UPI_RX_SIGDET_0_EV;
        else if (!strncmp(found, "rx_sigdet_1", 11))         req->upi_event = VTSS_UPI_RX_SIGDET_1_EV;
        else if (!strncmp(found, "rx_sigdet_2", 11))         req->upi_event = VTSS_UPI_RX_SIGDET_2_EV;
        else if (!strncmp(found, "rx_sigdet_3", 11))         req->upi_event = VTSS_UPI_RX_SIGDET_3_EV;
        else if (!strncmp(found, "tx_fifo_overflow", 16))    req->upi_event = VTSS_UPI_TX_FIFO_OVERFLOW_EV;
        else if (!strncmp(found, "tx_fifo_underflow", 17))   req->upi_event = VTSS_UPI_TX_FIFO_UNDERFLOW_EV;
        else if (!strncmp(found, "hss_prtready", 12))        req->upi_event = VTSS_UPI_HSS_PRTREADY_EV;
        else if (!strncmp(found, "rx_prbs_err_0", 13))       req->upi_event = VTSS_UPI_RX_PRBS_ERR_0_EV;
        else if (!strncmp(found, "rx_prbs_err_1", 13))       req->upi_event = VTSS_UPI_RX_PRBS_ERR_1_EV;
        else if (!strncmp(found, "rx_prbs_err_2", 13))       req->upi_event = VTSS_UPI_RX_PRBS_ERR_2_EV;
        else if (!strncmp(found, "rx_prbs_err_3", 13))       req->upi_event = VTSS_UPI_RX_PRBS_ERR_3_EV;
        else if (!strncmp(found, "tx_prbs_err_0", 13))       req->upi_event = VTSS_UPI_TX_PRBS_ERR_0_EV;
        else if (!strncmp(found, "tx_prbs_err_1", 13))       req->upi_event = VTSS_UPI_TX_PRBS_ERR_1_EV;
        else if (!strncmp(found, "tx_prbs_err_2", 13))       req->upi_event = VTSS_UPI_TX_PRBS_ERR_2_EV;
        else if (!strncmp(found, "tx_prbs_err_3", 13))       req->upi_event = VTSS_UPI_TX_PRBS_ERR_3_EV;
        else if (!strncmp(found, "ssf", 3))                  req->upi_event = VTSS_UPI_SSF_EV;
        else if (!strncmp(found, "plllock", 7))              req->upi_event = VTSS_UPI_PLLLOCK_EV;
        else if (!strncmp(found, "eye_quality_err", 15))     req->upi_event = VTSS_UPI_EYE_QUALITY_ERR_EV;
        else if (!strncmp(found, "fas_pos", 7))              req->upi_event = VTSS_UPI_OTN_FAS_POS_EV;
        else if (!strncmp(found, "lom", 3))                  req->upi_event = VTSS_UPI_OTN_LOM_EV;
        else if (!strncmp(found, "oom", 3))                  req->upi_event = VTSS_UPI_OTN_OOM_EV;
        else if (!strncmp(found, "lof", 3))                  req->upi_event = VTSS_UPI_OTN_LOF_EV;
        else if (!strncmp(found, "oof", 3))                  req->upi_event = VTSS_UPI_OTN_OOF_EV;
        else if (!strncmp(found, "all", 3))                  req->upi_event = 0xFFFFFFFFFFFFFFFF;
        else return 1;
    }

    if (req->block == daytona_xfi) {
        if (!strncmp(found, "hssplllock", 10))            req->xfi_event = VTSS_XFI_HSS_HSSPLLLOCK_EV;
        else if (!strncmp(found, "hssresetout", 9))       req->xfi_event = VTSS_XFI_HSS_HSSRESETOUT_EV;
        else if (!strncmp(found, "hsseyequality", 12))    req->xfi_event = VTSS_XFI_HSS_HSSEYEQUALITY_EV;
        else if (!strncmp(found, "hssprtready", 10))      req->xfi_event = VTSS_XFI_HSS_HSSPRTREADY_EV;
        else if (!strncmp(found, "rxprbssync", 10))       req->xfi_event = VTSS_XFI_HSS_RXPRBSSYNC_EV;
        else if (!strncmp(found, "rxprbserr", 9))         req->xfi_event = VTSS_XFI_HSS_RXPRBSERR_EV;
        else if (!strncmp(found, "rxasigdet", 9))         req->xfi_event = VTSS_XFI_HSS_RXASIGDET_EV;
        else if (!strncmp(found, "rxa_los", 7))           req->xfi_event = VTSS_XFI_HSS_RXA_LOS_EV;
        else if (!strncmp(found, "ifpll_lock", 10))       req->xfi_event = VTSS_XFI_HSS_IFPLL_LOCK_EV;
        else if (!strncmp(found, "tx_fifo_los", 11))      req->xfi_event = VTSS_XFI_HSS_TX_FIFO_LOS_EV;
        else if (!strncmp(found, "rxbsigdet", 9))         req->xfi_event = VTSS_XFI_HSS_RXBSIGDET_EV;
        else if (!strncmp(found, "rxb_los", 7))           req->xfi_event = VTSS_XFI_HSS_RXB_LOS_EV;

        else if (!strncmp(found, "aet_run_b", 9))         req->xfi_event = VTSS_XFI_AE_AET_RUN_B_EV;
        else if (!strncmp(found, "aet_frm_lck_b", 13))    req->xfi_event = VTSS_XFI_AE_AET_FRM_LCK_B_EV;
        else if (!strncmp(found, "pcs_rx_ready_b", 14))   req->xfi_event = VTSS_XFI_AE_PCS_RX_READY_B_EV;
        else if (!strncmp(found, "pcs_tx_ready_b", 17))   req->xfi_event = VTSS_XFI_AE_PCS_TX_READY_B_EV;
        else if (!strncmp(found, "fec_negotiated_b", 16)) req->xfi_event = VTSS_XFI_AE_FEC_NEGOTIATED_B_EV;
        else if (!strncmp(found, "train_ok_b", 10))       req->xfi_event = VTSS_XFI_AE_TRAIN_OK_B_EV;
        else if (!strncmp(found, "train_fail_b", 12))     req->xfi_event = VTSS_XFI_AE_TRAIN_FAIL_B_EV;
        else if (!strncmp(found, "train_local_b", 13))    req->xfi_event = VTSS_XFI_AE_TRAIN_LOCAL_B_EV;
        else if (!strncmp(found, "aet_run_a", 9))         req->xfi_event = VTSS_XFI_AE_AET_RUN_A_EV;
        else if (!strncmp(found, "aet_frm_lck_a", 13))    req->xfi_event = VTSS_XFI_AE_AET_FRM_LCK_A_EV;
        else if (!strncmp(found, "pcs_rx_ready_a", 14))   req->xfi_event = VTSS_XFI_AE_PCS_RX_READY_A_EV;
        else if (!strncmp(found, "pcs_tx_ready_a", 14))   req->xfi_event = VTSS_XFI_AE_PCS_TX_READY_A_EV;
        else if (!strncmp(found, "fec_negotiated_a", 16)) req->xfi_event = VTSS_XFI_AE_FEC_NEGOTIATED_A_EV;
        else if (!strncmp(found, "train_ok_a", 10))       req->xfi_event = VTSS_XFI_AE_TRAIN_OK_A_EV;
        else if (!strncmp(found, "train_fail_a", 12))     req->xfi_event = VTSS_XFI_AE_TRAIN_FAIL_A_EV;
        else if (!strncmp(found, "train_local_a", 13))    req->xfi_event = VTSS_XFI_AE_TRAIN_LOCAL_A_EV;

        else if (!strncmp(found, "fas_pos", 7))           req->xfi_event = VTSS_XFI_OTN_FAS_POS_EV;
        else if (!strncmp(found, "lom", 3))               req->xfi_event = VTSS_XFI_OTN_LOM_EV;
        else if (!strncmp(found, "oom", 3))               req->xfi_event = VTSS_XFI_OTN_OOM_EV;
        else if (!strncmp(found, "lof", 3))               req->xfi_event = VTSS_XFI_OTN_LOF_EV;
        else if (!strncmp(found, "oof", 3))               req->xfi_event = VTSS_XFI_OTN_OOF_EV;

        else if (!strncmp(found, "incompatible_link", 17))req->xfi_event = VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV;
        else if (!strncmp(found, "par_detect_fault", 16)) req->xfi_event = VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV;
        else if (!strncmp(found, "an_good_check", 13))    req->xfi_event = VTSS_XFI_ANEG_AN_GOOD_CHECK_EV;
        else if (!strncmp(found, "page_rx", 7))           req->xfi_event = VTSS_XFI_ANEG_PAGE_RX_EV;
        else if (!strncmp(found, "aneg_complete", 13))    req->xfi_event = VTSS_XFI_ANEG_ANEG_COMPLETE_EV;
        else if (!strncmp(found, "all", 3))               req->xfi_event = 0xFFFFFFFFFFFFFFFF;

        else return 1;
    }

    if (req->block == daytona_otn) {
        if (!strncmp(found, "och_los", 7))                    req->otn_och_event = VTSS_OTN_OCH_LOS_EV;
        else if (!strncmp(found, "och_lof", 7))               req->otn_och_event = VTSS_OTN_OCH_LOF_EV;
        else if (!strncmp(found, "och_lom", 7))               req->otn_och_event = VTSS_OTN_OCH_LOM_EV;
        else if (!strncmp(found, "och_ais", 7))               req->otn_och_event = VTSS_OTN_OCH_AIS_EV;
        else if (!strncmp(found, "och_ber_exceeded", 16))     req->otn_och_event = VTSS_OTN_OCH_BER_EXCD_EV;
        else if (!strncmp(found, "och_uncorrect_code", 18))   req->otn_och_event = VTSS_OTN_OCH_UNCRCT_CODE_EV;
        else if (!strncmp(found, "otu_tim", 7))               req->otn_otu_event = VTSS_OTN_OTU_TIM_EV;
        else if (!strncmp(found, "otu_acc_tti", 11))          req->otn_otu_event = VTSS_OTN_OTU_ACC_TTI_EV;
        else if (!strncmp(found, "otu_iconsist_tti", 16))     req->otn_otu_event = VTSS_OTN_OTU_ICONSIST_TTI_EV;
        else if (!strncmp(found, "otu_acc_res", 11))          req->otn_otu_event = VTSS_OTN_OTU_ACC_RES_EV;
        else if (!strncmp(found, "otu_inconsist_res", 17))    req->otn_otu_event = VTSS_OTN_OTU_INCONSIST_RES_EV;
        else if (!strncmp(found, "otu_acc_smres", 13))        req->otn_otu_event = VTSS_OTN_OTU_ACC_SMRES_EV;
        else if (!strncmp(found, "otu_inconsist_smres", 19))  req->otn_otu_event = VTSS_OTN_OTU_INCONSIST_SMRES_EV;
        else if (!strncmp(found, "otu_acc_gcc0", 12))         req->otn_otu_event = VTSS_OTN_OTU_ACC_GCC0_EV;
        else if (!strncmp(found, "otu_inconsist_gcc0", 18))   req->otn_otu_event = VTSS_OTN_OTU_INCONSIST_GCC0_EV;
        else if (!strncmp(found, "otu_bdi", 7))               req->otn_otu_event = VTSS_OTN_OTU_BDI_EV;
        else if (!strncmp(found, "otu_iae", 7))               req->otn_otu_event = VTSS_OTN_OTU_IAE_EV;
        else if (!strncmp(found, "otu_biae", 8))              req->otn_otu_event = VTSS_OTN_OTU_BIAE_EV;
        else if (!strncmp(found, "odu_tim", 7))               req->otn_odu_event = VTSS_OTN_ODU_TIM_EV;
        else if (!strncmp(found, "odu_acc_tti", 11))          req->otn_odu_event = VTSS_OTN_ODU_ACC_TTI_EV;
        else if (!strncmp(found, "odu_iconsist_tti", 16))     req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_TTI_EV;
        else if (!strncmp(found, "odu_plm", 7))               req->otn_odu_event = VTSS_OTN_ODU_PLM_EV;
        else if (!strncmp(found, "odu_acc_pt", 10))           req->otn_odu_event = VTSS_OTN_ODU_ACC_PT_EV;
        else if (!strncmp(found, "odu_iconsist_pt", 15))      req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_PT_EV;
        else if (!strncmp(found, "odu_acc_exp", 11))          req->otn_odu_event = VTSS_OTN_ODU_ACC_EXP_EV;
        else if (!strncmp(found, "odu_iconsist_exp", 16))     req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_EXP_EV;
        else if (!strncmp(found, "odu_acc_res", 11))          req->otn_odu_event = VTSS_OTN_ODU_ACC_RES_EV;
        else if (!strncmp(found, "odu_iconsist_res", 16))     req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_RES_EV;
        else if (!strncmp(found, "odu_acc_ftfl", 12))         req->otn_odu_event = VTSS_OTN_ODU_ACC_FTFL_EV;
        else if (!strncmp(found, "odu_iconsist_ftfl", 17))    req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_FTFL_EV;
        else if (!strncmp(found, "odu_acc_aps", 11))          req->otn_odu_event = VTSS_OTN_ODU_ACC_APS_EV;
        else if (!strncmp(found, "odu_iconsist_aps", 16))     req->otn_odu_event = VTSS_OTN_ODU_ICONSIST_APS_EV;
        else if (!strncmp(found, "odu_oci", 7))               req->otn_odu_event = VTSS_OTN_ODU_OCI_EV;
        else if (!strncmp(found, "odu_lck", 7))               req->otn_odu_event = VTSS_OTN_ODU_LCK_EV;
        else if (!strncmp(found, "odu_bdi", 7))               req->otn_odu_event = VTSS_OTN_ODU_BDI_EV;
        else if (!strncmp(found, "odu_ais", 7))               req->otn_odu_event = VTSS_OTN_ODU_AIS_EV;
        else if (!strncmp(found, "odut_tim", 8))              req->otn_odut_event = VTSS_OTN_ODUT_TIM_EV;
        else if (!strncmp(found, "odut_acc_tti", 12))         req->otn_odut_event = VTSS_OTN_ODUT_ACC_TTI_EV;
        else if (!strncmp(found, "odut_iconsist_tti", 17))    req->otn_odut_event = VTSS_OTN_ODUT_ICONSIST_TTI_EV;
        else if (!strncmp(found, "odut_acc_stat", 13))        req->otn_odut_event = VTSS_OTN_ODUT_ACC_STAT_EV;
        else if (!strncmp(found, "odut_iconsist_stat", 18))   req->otn_odut_event = VTSS_OTN_ODUT_ICONSIST_STAT_EV;
        else if (!strncmp(found, "odut_acc_aps", 12))         req->otn_odut_event = VTSS_OTN_ODUT_ACC_APS_EV;
        else if (!strncmp(found, "odut_iconsist_aps", 17))    req->otn_odut_event = VTSS_OTN_ODUT_ICONSIST_APS_EV;
        else if (!strncmp(found, "odut_oci", 8))              req->otn_odut_event = VTSS_OTN_ODUT_OCI_EV;
        else if (!strncmp(found, "odut_lck", 8))              req->otn_odut_event = VTSS_OTN_ODUT_LCK_EV;
        else if (!strncmp(found, "odut_bdi", 8))              req->otn_odut_event = VTSS_OTN_ODUT_BDI_EV;
        else if (!strncmp(found, "odut_ais", 8))              req->otn_odut_event = VTSS_OTN_ODUT_AIS_EV;
        else if (!strncmp(found, "odut_ltc", 8))              req->otn_odut_event = VTSS_OTN_ODUT_LTC_EV;
        else if (!strncmp(found, "odut_iae", 8))              req->otn_odut_event = VTSS_OTN_ODUT_IAE_EV;
        else if (!strncmp(found, "odut_biae", 8))             req->otn_odut_event = VTSS_OTN_ODUT_BIAE_EV;
        else if (!strncmp(found, "och_all", 7))               req->otn_och_event = 0xFFFFFFFF;
        else if (!strncmp(found, "otu_all", 7))               req->otn_otu_event = 0xFFFFFFFF;
        else if (!strncmp(found, "odu_all", 7))               req->otn_odu_event = 0xFFFFFFFF;
        else if (!strncmp(found, "odut_all", 8))              req->otn_odut_event = 0xFFFFFFFF;
        else if (!strncmp(found, "all", 3))                   {req->otn_odut_event = 0xFFFFFFFF; req->otn_odu_event = 0xFFFFFFFF; req->otn_otu_event = 0xFFFFFFFF; req->otn_och_event = 0xFFFFFFFF;}
        else return 1;
    }

    if (req->block == daytona_ewis) {
        if (!strncmp(found, "los", 3))           req->ewis_event = VTSS_EWIS_LOS_EV;
        else if (!strncmp(found, "oof", 3))      req->ewis_event = VTSS_EWIS_OOF_EV;
        else if (!strncmp(found, "lof", 3))      req->ewis_event = VTSS_EWIS_LOF_EV;
        else if (!strncmp(found, "rdil", 4))     req->ewis_event = VTSS_EWIS_RDIL_EV;
        else if (!strncmp(found, "aisl", 4))     req->ewis_event = VTSS_EWIS_AISL_EV;
        else if (!strncmp(found, "s1", 2))       req->ewis_event = VTSS_EWIS_S1_EV;
        else if (!strncmp(found, "lop", 3))      req->ewis_event = VTSS_EWIS_LOP_EV;
        else if (!strncmp(found, "aisp", 4))     req->ewis_event = VTSS_EWIS_AISP_EV;
        else if (!strncmp(found, "plmp", 4))     req->ewis_event = VTSS_EWIS_PLMP_EV;
        else if (!strncmp(found, "uneqp", 5))    req->ewis_event = VTSS_EWIS_UNEQP_EV;
        else if (!strncmp(found, "rdip", 4))     req->ewis_event = VTSS_EWIS_RDIP_EV;
        else if (!strncmp(found, "fe_ais_p", 8)) req->ewis_event = VTSS_EWIS_FE_AIS_P_EV;
        else if (!strncmp(found, "feplmp", 6))   req->ewis_event = VTSS_EWIS_FEPLMP_EV;
        else if (!strncmp(found, "fe_uneqp", 8)) req->ewis_event = VTSS_EWIS_FE_UNEQP_EV;
        else if (!strncmp(found, "all", 3))      req->ewis_event = 0xFFFFFFFFFFFFFFFF;
        else return 1;
    }

    if (req->block == daytona_pcs) {
        if (!strncmp(found, "rx_hi_ber", 9))                req->pcs_event = VTSS_PCS_10GBASE_R_RX_HI_BER_EV;
        else if (!strncmp(found, "rx_fifo_oflow", 13))      req->pcs_event = VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV;
        else if (!strncmp(found, "rx_fifo_uflow", 13))      req->pcs_event = VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV;
        else if (!strncmp(found, "lock_changed", 12))       req->pcs_event = VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV;
        else if (!strncmp(found, "c64b66b_err", 11))        req->pcs_event = VTSS_PCS_10GBASE_R_C64B66B_ERR_EV;
        else if (!strncmp(found, "rx_oset", 7))             req->pcs_event = VTSS_PCS_10GBASE_R_RX_OSET_EV;
        else if (!strncmp(found, "rx_oset_fifo_full", 17))  req->pcs_event = VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV;
        else if (!strncmp(found, "tx_fifo_oflow", 13))      req->pcs_event = VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV;
        else if (!strncmp(found, "tx_fifo_uflow", 13))      req->pcs_event = VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV;
        else if (!strncmp(found, "xgmii_err", 9))           req->pcs_event = VTSS_PCS_10GBASE_R_XGMII_ERR_EV;
        else if (!strncmp(found, "rx_fset", 7))             req->pcs_event = VTSS_PCS_10GBASE_R_RX_FSET_EV;
        else if (!strncmp(found, "rx_fset_fifo_full", 17))  req->pcs_event = VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV;
        else if (!strncmp(found, "tx_fifo_ecc_1bit", 16))   req->pcs_event = VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV;
        else if (!strncmp(found, "tx_fifo_ecc_2bit", 16))   req->pcs_event = VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV;
        else if (!strncmp(found, "rx_fifo_ecc_1bit", 16))   req->pcs_event = VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV;
        else if (!strncmp(found, "rx_fifo_ecc_2bit", 16))   req->pcs_event = VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV;
        else if (!strncmp(found, "all", 3))                 req->pcs_event = 0xFFFFFFFF;
        else return 1;
    }

    if (req->block == daytona_mac) {
        if (!strncmp(found, "link_delta", 10))                      req->mac_event = VTSS_MAC10G_LINK_DELTA_EV;
        else if (!strncmp(found, "remote_fault_state_delta", 24))   req->mac_event = VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV;
        else if (!strncmp(found, "local_fault_state_delta", 23))    req->mac_event = VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV;
        else if (!strncmp(found, "tx_abort", 8))                    req->mac_event = VTSS_MAC10G_TX_ABORT_EV;
        else if (!strncmp(found, "rx_non_std_pream", 16))           req->mac_event = VTSS_MAC10G_RX_NON_STD_PREAM_EV;
        else if (!strncmp(found, "rx_pream_err", 12))               req->mac_event = VTSS_MAC10G_RX_PREAM_ERR_EV;
        else if (!strncmp(found, "rx_pream_mismatch", 17))          req->mac_event = VTSS_MAC10G_RX_PREAM_MISMATCH_EV;
        else if (!strncmp(found, "rx_pream_shrink", 15))            req->mac_event = VTSS_MAC10G_RX_PREAM_SHRINK_EV;
        else if (!strncmp(found, "tx_ifg_column_drop", 18))         req->mac_event = VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV;
        else if (!strncmp(found, "all", 3))                         req->mac_event = 0xFFFFFFFF;
        else return 1;
    }

    if (req->block == daytona_gfp) {
        if (!strncmp(found, "ssf", 3))                      req->gfp_event = VTSS_GFP_SSF_EV;
        else if (!strncmp(found, "8b10b_los", 9))           req->gfp_event = VTSS_GFP_8B10B_LOS_EV;
        else if (!strncmp(found, "8b10b_lsync", 11))        req->gfp_event = VTSS_GFP_8B10B_LSYNC_EV;
        else if (!strncmp(found, "8b10b_error", 11))        req->gfp_event = VTSS_GFP_8B10B_ERROR_EV;
        else if (!strncmp(found, "8b10b_rx_illegal", 16))   req->gfp_event = VTSS_GFP_8B10B_RX_ILLEGAL_EV;
        else if (!strncmp(found, "8b10b_rx_disparity", 18)) req->gfp_event = VTSS_GFP_8B10B_RX_DISPARITY_EV;
        else if (!strncmp(found, "8b10b_tx_illegal", 16))   req->gfp_event = VTSS_GFP_8B10B_TX_ILLEGAL_EV;
        else if (!strncmp(found, "8b10b_tx_disparity", 18)) req->gfp_event = VTSS_GFP_8B10B_TX_DISPARITY_EV;
        else if (!strncmp(found, "cmf_los_sf", 10))         req->gfp_event = VTSS_GFP_CMF_LOS_SF_EV;
        else if (!strncmp(found, "cmf_lsync_sf", 12))       req->gfp_event = VTSS_GFP_CMF_LSYNC_SF_EV;
        else if (!strncmp(found, "cmf_unsupported", 15))    req->gfp_event = VTSS_GFP_CMF_UNSUPPORTED_EV;
        else if (!strncmp(found, "cmf_inconsistent", 16))   req->gfp_event = VTSS_GFP_CMF_INCONSISTENT_EV;
        else if (!strncmp(found, "frame_mapped_eth", 16))   req->gfp_event = VTSS_GFP_FRAME_MAPPED_ETHERNET_EV;
        else if (!strncmp(found, "cmf_accepted", 12))       req->gfp_event = VTSS_GFP_CMF_ACCEPTED_EV;
        else if (!strncmp(found, "cdf_upm", 7))             req->gfp_event = VTSS_GFP_CDF_UPM_EV;
        else if (!strncmp(found, "ppos_cdf_upm", 12))       req->gfp_event = VTSS_GFP_PPOS_CDF_UPM_EV; 
        else if (!strncmp(found, "all", 3))                 req->gfp_event = 0xFFFFFFFF;
        else return 1;
    }
    if (req->block == daytona_i1588) {
        if (!strncmp(found, "all", 3))                      req->i1588_event =  VTSS_PHY_TS_INGR_ENGINE_ERR    |
                                                                                VTSS_PHY_TS_INGR_RW_PREAM_ERR  |
                                                                                VTSS_PHY_TS_INGR_RW_FCS_ERR    |
                                                                                VTSS_PHY_TS_EGR_ENGINE_ERR     |
                                                                                VTSS_PHY_TS_EGR_RW_FCS_ERR     |
                                                                                VTSS_PHY_TS_EGR_TIMESTAMP_CAPTURED |
                                                                                VTSS_PHY_TS_EGR_FIFO_OVERFLOW ;
        else return 1;
    }
    if (req->block == daytona_efec) {
        if (!strncmp(found, "i4_ed_uccw", 10))        req->efec_event = VTSS_I4_ED_UCCW_EV;
        else if (!strncmp(found, "i4_rs_uccw", 10))   req->efec_event = VTSS_I4_RS_UCCW_EV;
        else if (!strncmp(found, "i4_bch_uccw", 11))  req->efec_event = VTSS_I4_BCH_UCCW_EV;
        else if (!strncmp(found, "all", 3))           req->efec_event = 0xFFFFFFFF;
        else return 1;
    }    
    if (req->block == daytona_efeci7) {
        if (!strncmp(found, "i7_ber_alarm", 12))   req->efeci7_event = VTSS_I7_BER_ALARM_EV;
        else if (!strncmp(found, "all", 3))        req->efeci7_event = 0xFFFFFFFF;
        else return 1;
    }    




    return 0;
}
#endif /* VTSS_ARCH_DAYTONA */

/* Convert text to list */
static int cli_parse_list(char *buf, BOOL *list, u32 min, u32 max, BOOL def)
{
    u32   i, start = 0, n;
    char  *p, *end;
    BOOL  error, range = 0, comma = 0;
    
    /* Clear list by default */
    for (i = min; i <= max; i++)
        list[i] = 0;

    p = buf;
    error = (p == NULL);
    while (p != NULL && *p != '\0') {
        /* Read integer */
        n = strtoul(p, &end, 0);
        if (end == p) {
            error = 1;
            break;
        }
        p = end;
        
        /* Check legal range */
        if (n < min || n > max) {
            error = 1;
            break;
        }
        
        if (range) {
            /* End of range has been read */
            if (n < start) {
                error = 1;
                break;
            }
            for (i = start ; i <= n; i++)
                list[i] = 1;
            range = 0;
        } else if (*p == '-') {
            /* Start of range has been read */
            start = n;
            range = 1;
            p++;
        } else {
            /* Single value has been read */
            list[n] = 1;
        }
        comma = 0;
        if (!range && *p == ',') {
            comma = 1;
            p++;
        }
    }
    
    /* Check for trailing comma/dash */
    if (comma || range)
        error = 1;
    
    /* Restore defaults if error */
    for (i = min; error && i <= max; i++)
        list[i] = def;

    return error;
}

/* Remove unused ports from the list */
static void cli_remove_unused_ports(cli_req_t *req) 
{
#if defined(VTSS_FEATURE_PORT_CONTROL)
    vtss_port_no_t  port_no;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) 
        if (cli_board->port_map[port_no].chip_port < 0)
          req->port_list[iport2uport(port_no)] = 0;
#endif /* VTSS_FEATURE_PORT_CONTROL */
}

static int cli_parse_mac(char *cmd, uchar *mac_addr)
{
    u32 i, mac[6];
    int error = 1;
    
    if (sscanf(cmd, "%2x-%2x-%2x-%2x-%2x-%2x", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
        for (i = 0; i < 6; i++)
            mac_addr[i] = (mac[i] & 0xff);
        error = 0;
    } 
    return error;
}

static int cli_parse_ulong(char *buf, u32 *val, u32 min, u32 max)
{
    u32  n;
    char *end;

    n = strtoul(buf, &end, 0);
    if (*end != '\0' || n < min || n > max)
        return 1;

    *val = n;
    return 0;
}

static int cli_parse_trace_layer(char *cmd, cli_req_t *req)
{
    switch (*cmd) {
    case 'a':
        req->trace_layer = VTSS_TRACE_LAYER_AIL;
        break;
    case 'c':
        req->trace_layer = VTSS_TRACE_LAYER_CIL;
        break;
    default:
        return 1;
    } 
    return 0;
}

#if defined(VTSS_ARCH_DAYTONA) || defined(VTSS_FEATURE_WIS)

static int cli_parse_txt(char *buf, char *val, u32 max_len)
{
    strncpy(val, buf, max_len);
    return 0;
}
#endif /* VTSS_ARCH_DAYTONA || defined(VTSS_FEATURE_WIS)*/

#if defined(VTSS_FEATURE_WIS)
static int32_t cli_misc_wis_overhead_name_parse(char *cmd, char *cmd2, char *stx,
        char *cmd_org, cli_req_t *req)
{
    int32_t        error=0;

    if (!strncmp(cmd_org, "ALL", 3)) {
        req->wis_oh_id = 1;
    } else if (!strncmp(cmd_org, "SEC-OH", 6)) {
        req->wis_oh_id = 2;
    } else if (!strncmp(cmd_org, "LINE-OH", 7)) {
        req->wis_oh_id = 3;
    } else if (!strncmp(cmd_org, "PATH-OH", 7)) {
        req->wis_oh_id = 4;
    } else if (!strncmp(cmd_org, "D1-D3", 5)) {
        req->wis_oh_id = 5;
    } else if (!strncmp(cmd_org, "SEC-ORD", 7)) {
        req->wis_oh_id = 6;
    } else if (!strncmp(cmd_org, "SUC", 3)) {
        req->wis_oh_id = 7;
    } else if (!strncmp(cmd_org, "RES-SG", 6)) {
        req->wis_oh_id = 8;
    } else if (!strncmp(cmd_org, "D4-D12", 6)) {
        req->wis_oh_id = 9;
    } else if (!strncmp(cmd_org, "LINE-ORD", 8)) {
        req->wis_oh_id = 10;
    } else if (!strncmp(cmd_org, "APS-RDIL", 8)) {
        req->wis_oh_id = 11;
    } else if (!strncmp(cmd_org, "SYNC", 4)) {
        req->wis_oh_id = 12;
    } else if (!strncmp(cmd_org, "RES-LG", 6)) {
        req->wis_oh_id = 13;
    } else if (!strncmp(cmd_org, "C2PL", 4)) {
        req->wis_oh_id = 14;
    } else if (!strncmp(cmd_org, "PUC", 3)) {
        req->wis_oh_id = 15;
    } else if (!strncmp(cmd_org, "PTCM", 4)) {
        req->wis_oh_id = 16;
    } else if (!strncmp(cmd_org, "RES-PG", 6)) {
        req->wis_oh_id = 17;
    }  else {        
        error = 1;
    }
    return error;
}
#endif /* defined(VTSS_FEATURE_WIS) */

#if defined(VTSS_ARCH_DAYTONA) || defined(VTSS_FEATURE_WIS) 
static u8 hexa2binary(char c)
{
    switch ( c ) {
        case '0':           return 0x0;
        case '1':           return 0x1;
        case '2':           return 0x2;
        case '3':           return 0x3;
        case '4':           return 0x4;
        case '5':           return 0x5;
        case '6':           return 0x6;
        case '7':           return 0x7;
        case '8':           return 0x8;
        case '9':           return 0x9;
        case 'A': case 'a': return 0xa;
        case 'B': case 'b': return 0xb;
        case 'C': case 'c': return 0xc;
        case 'D': case 'd': return 0xd;
        case 'E': case 'e': return 0xe;
        case 'F': case 'f': return 0xf;
        default:            return (0xFF);
    }
}

#endif /* VTSS_ARCH_DAYTONA || VTSS_FEATURE_WIS */

#if defined(VTSS_FEATURE_WIS)
static int32_t cli_misc_wis_overhead_value_parse(char *cmd, char *cmd2, char *stx,
                                              char *cmd_org, cli_req_t *req)
{
    int32_t        error=1;
    u32             length = 0;
    u8              val,val1,i =0,j;
    
    length = strlen(cmd_org);
    if ((length > 54)||(length < 1)) {
        printf("Overhead value error\n");
        return error;
    }
    if (!strncmp(cmd_org,"0x",2))
        i = 2;
    
    for (j=0;i<length;i+=2,j++){
        val = hexa2binary(cmd_org[i]);
        if (val==0xFF)
            return error;
        val1 = hexa2binary(cmd_org[i+1]);
        if (val1==0xFF)
            return error;
        val = (val<<4)| val1;
        req->wis_oh_val[j]= val;
    }
    
    return 0;
}
#endif /* defined(VTSS_FEATURE_WIS) */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP

static int cli_parm_1588_api_parse_keyword(char *cmd, cli_req_t *req, char *stx)
                                      
{
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ingress", 7)) {
            req->ingress = TRUE;
        } else if (!strncmp(found, "egress", 6)) {
            req->ingress = FALSE;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "strict", 6)) {
            req->flow_match_mode = VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT;
        } else if (!strncmp(found, "non-strict", 10)) {
            req->flow_match_mode = VTSS_PHY_TS_ENG_FLOW_MATCH_ANY;
        } else if (!strncmp(found, "pbb_en", 6)) {
            req->pbb_spec = TRUE;
            req->pbb_en = TRUE;
        } else if (!strncmp(found, "pbb_dis", 7)) {
            req->pbb_spec = TRUE;
            req->pbb_en = FALSE;
        } else if (!strncmp(found, "match_full", 10)) {
            req->mac_match_mode_spec = TRUE;
            req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_48BIT;
        } else if (!strncmp(found, "any_uc", 6)) {
            req->mac_match_mode_spec = TRUE;
            req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_UNICAST;
        } else if (!strncmp(found, "any_mc", 6)) {
            req->mac_match_mode_spec = TRUE;
            req->mac_match_mode = VTSS_PHY_TS_ETH_ADDR_MATCH_ANY_MULTICAST;
        } else if (!strncmp(found, "vlan_chk_en", 11)) {
            req->vlan_chk_spec = TRUE;
            req->vlan_chk = TRUE;
        } else if (!strncmp(found, "vlan_chk_dis", 12)) {
            req->vlan_chk_spec = TRUE;
            req->vlan_chk = FALSE;
        } else if (!strncmp(found, "tag_rng_none", 12)) {
            req->tag_rng_mode_spec = TRUE;
            req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_NONE;
        } else if (!strncmp(found, "tag_rng_outer", 13)) {
            req->tag_rng_mode_spec = TRUE;
            req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_OUTER;
        } else if (!strncmp(found, "tag_rng_inner", 13)) {
            req->tag_rng_mode_spec = TRUE;
            req->tag_rng_mode = VTSS_PHY_TS_TAG_RANGE_INNER;
        } else if (!strncmp(found, "ipv4", 4)) {
            req->ip_mode_spec = TRUE;
            req->ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (!strncmp(found, "ipv6", 4)) {
            req->ip_mode_spec = TRUE;
            req->ip_mode = VTSS_PHY_TS_IP_VER_6;
        } else if (!strncmp(found, "match_source", 12)) {
            req->addr_match_select_spec = TRUE;
            req->addr_match_select = 0;
        } else if (!strncmp(found, "match_dest", 10)) {
            req->addr_match_select_spec = TRUE;
            req->addr_match_select = 1;
        } else if (!strncmp(found, "match_src_dest", 14)) {
            req->addr_match_select_spec = TRUE;
            req->addr_match_select = 2;
        } else if (!strncmp(found, "stk_ref_top", 11)) {
            req->stk_ref_point_spec = TRUE;
            req->stk_ref_point = VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP;
        } else if (!strncmp(found, "stk_ref_bottom", 14)) {
            req->stk_ref_point_spec = TRUE;
            req->stk_ref_point = VTSS_PHY_TS_MPLS_STACK_REF_POINT_END;
        } else if (!strncmp(found, "ptp", 3)) {
            req->ptp_spec = TRUE;
        } else if (!strncmp(found, "y1731_oam", 9)) {
            req->y1731_oam_spec = TRUE;
        } else if (!strncmp(found, "ietf_oam", 8)) {
            req->ietf_oam_spec = TRUE;
        }

    }

    return (found == NULL ? 1 : 0);

}
static int cli_parm_1588_ip_flow_keyword(char *cmd, cli_req_t *req, char *stx)
{
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ipv4", 4)) {
            req->ip_mode = VTSS_PHY_TS_IP_VER_4;
        } else if (!strncmp(found, "ipv6", 4)) {
            req->ip_mode = VTSS_PHY_TS_IP_VER_6;
        }
    }
    return (found == NULL ? 1 : 0);
}


static vtss_rc vtss_phy_ts_base_port_get(const vtss_port_no_t port_no,
                                         vtss_port_no_t     *const base_port_no)
{
    if (port_no == 2 || port_no == 3) {
        *base_port_no = 2;
        return VTSS_RC_OK;
    }
    printf("Invalid port (%d)", port_no);
    return VTSS_RC_ERROR;
}


/* Validate and convert IPv4 address string a.b.c.d
 * a legal unicast/multicast address or 0.0.0.0 is accepted.
 */
static int txt2_ipv4(char *buf, u32 *ip)
{
    u32 a, b, c, d;
    u32 data;
    int items;

    items = sscanf(buf, "%u.%u.%u.%u", &a, &b, &c, &d);
    if ((items != 4) || (a > 255) || (b > 255) || (c > 255) || (d > 255)) {
        /* ERROR: Invalid IP address format */
        return 1;
    }

    data = ((a<<24) + (b<<16) + (c<<8) + d);
    /* Check that it is 0.0.0.0 or a legal address */
    if ((a == 0) && (data != 0)) {
        /* ERROR: Not a legal address or 0.0.0.0 */
        return 1;
    }

    *ip = data;
    return 0;
}

#define TXT2IPV6_INVALID_BIN_VAL    (-1)
#define TXT2IPV6_VAL_BIN_MASK       0xF
#define TXT2IPV6_VAL_MAX_SIGN       0xFFFF
#define TXT2IPV6_MIN_COL_CNT        2
#define TXT2IPV6_MAX_COL_CNT        7
#define TXT2IPV6_MIN_DOT_CNT        3
#define TXT2IPV6_MAX_DOT_CNT        3
#define TXT2IPV6_MAX_ELEMENT_CNT    8
#define TXT2IPV6_MAX_SYMBOL_CNT     9
#define TXT2IPV6_MAX_DOT_VAL        0x9
#if IPV6_ADDR_STR_LEN_LONG
#define TXT2IPV6_MAX_COL_DIGIT_CNT  (IPV6_ADDR_STR_MAX_LEN - IPV6_ADDR_STR_MIN_LEN)
#else
#define TXT2IPV6_MAX_COL_DIGIT_CNT  4
#endif /* IPV6_ADDR_STR_LEN_LONG  */
#define TXT2IPV6_MAX_DOT_DIGIT_CNT  3
#define TXT2IPV6_HEX_VAL_SHIFT_UNIT 4
#define TXT2IPV6_DEC_VAL_SHIFT_UNIT 10
#define TXT2IPV6_V6_ADDR_SHIFT_UNIT 8
#define TXT2IPV6_V6_ADDR_SHIFT_MASK 0xFF
#define TXT2IPV6_V6_ADDR_ZERO_SIGN  0x0
#define TXT2IPV6_IPV4COMPAT_SIGN    0x0
#define TXT2IPV6_IPV4MAPPED_SIGN    0xFFFF
#define TXT2IPV6_V4TOV6_ELECNT_BASE 2
#define TXT2IPV6_V4TOV6_COMPAT_BASE 12
#define TXT2IPV6_V4TOV6_MAPPED_BASE 10
#define TXT2IPV6_V4TOV6_MAPPED_CHK1 11
#define TXT2IPV6_V4TOV6_MAPPED_SIGN 0xFF
#define TXT2IPV6_V4TOV6_MAX_OFFSET  3
#define TXT2IPV6_V4TOV6_MAX_VALUE   0xFF
#define TXT2IPV6_POWER_BASE_VAL     1
#define TXT2IPV6_ADDR_IDX_BASE      0x0
#define TXT2IPV6_ADDR_IDX_SIZE      sizeof(vtss_ipv6_t)
#define TXT2IPV6_ADDR_IDX_END       (TXT2IPV6_ADDR_IDX_SIZE - 1)
#define TXT2IPV6_V4_MCAST_LEADING   0xE0
#define TXT2IPV6_V6_MCAST_LEADING   0xFF
#define TXT2IPV6_ADDR_LL_CHK_IDX0   TXT2IPV6_ADDR_IDX_BASE
#define TXT2IPV6_ADDR_LL_CHK_IDX1   1
#define TXT2IPV6_ADDR_LB_CHK_IDX    TXT2IPV6_ADDR_IDX_END
#define TXT2IPV6_ADDR_LL_SIGN1      0xFE
#define TXT2IPV6_ADDR_LL_SIGN2      0x80
#define TXT2IPV6_ADDR_LB_SIGN       0x1

static int hex2bin(char c)
{
    switch ( c ) {
    case '0':           return 0x0;
    case '1':           return 0x1;
    case '2':           return 0x2;
    case '3':           return 0x3;
    case '4':           return 0x4;
    case '5':           return 0x5;
    case '6':           return 0x6;
    case '7':           return 0x7;
    case '8':           return 0x8;
    case '9':           return 0x9;
    case 'A': case 'a': return 0xa;
    case 'B': case 'b': return 0xb;
    case 'C': case 'c': return 0xc;
    case 'D': case 'd': return 0xd;
    case 'E': case 'e': return 0xe;
    case 'F': case 'f': return 0xf;
    default:            return (-1);
    }
}

/* Convert text to range */
static int mgmt_txt2range(char *buf, u32 *req_min, u32 *req_max, u32 min, u32 max)
{
    u32  x, y = 0;
    char *end;

    x = strtoul(buf, &end, 0);
    if (*end == '\0')
        y = x;
    else if (*end == '-')
        y = strtoul(end + 1, &end, 0);

    if (*end != '\0' || x < min || y > max || x > y ) {
        return 1;
    }

    *req_min = x;
    *req_max = y;
    return 0;
}
/* Parse syntax word */
static int cli_parse_word(char *cmd, const char *stx)
{
    return (strstr(stx, cmd) != stx);
}

static txt2ip6_addr_type_t _mgmt_txt2ipv6(const char *s, vtss_ipv6_t *addr)
{
    /* Follow RFC-4291 & RFC-5952 */
    u8      i, j, k, idx, len, val_cnt, dot_cnt, col_cnt;
    u16     curr_val, last_val, prev_val;
    u32     max_hex_val;
    int     val;
    BOOL    flag, val_chk, hex_val;
    BOOL    ipv4_compat, ipv4_mapped;

    if (!s || !addr) {
        return TXT2IP6_ADDR_TYPE_INVALID;
    }

    len = strlen(s);
    if ((len < IPV6_ADDR_STR_MIN_LEN) || (len > IPV6_ADDR_STR_MAX_LEN)) {
        return TXT2IP6_ADDR_TYPE_INVALID;
    }

    /* syntax parsing first */
    val_cnt = dot_cnt = col_cnt = 0;
    curr_val = last_val = prev_val = 0;
    max_hex_val = 0;
    flag = hex_val = val_chk = FALSE;
    ipv4_compat = ipv4_mapped = FALSE;
    for (i = 0; i < len; i++) {
        if ((val = hex2bin(s[i])) != TXT2IPV6_INVALID_BIN_VAL) {
            if (!val_cnt) {
                if (!prev_val) {
                    prev_val = last_val;
                }
                last_val = curr_val;
                curr_val = 0;
                max_hex_val = 0;

                if (dot_cnt) {
                    hex_val = FALSE;
                    val_chk = TRUE;
                } else {
                    hex_val = TRUE;
                    val_chk = FALSE;
                }
            }

            for (j = i + 1; !val_chk && (j < len); j++) {
                if (s[j] == '.') {
                    hex_val = FALSE;
                    break;
                } else if (s[j] == ':') {
                    hex_val = TRUE;
                    break;
                } else {
                    if (hex2bin(s[j]) == TXT2IPV6_INVALID_BIN_VAL) {
                        return TXT2IP6_ADDR_TYPE_INVALID;
                    }
                }
            }
            val_chk = TRUE;

            val_cnt++;

            if (hex_val) {
                if (val_cnt > TXT2IPV6_MAX_COL_DIGIT_CNT) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }

                if (val_cnt - 1) {
                    max_hex_val = max_hex_val << TXT2IPV6_HEX_VAL_SHIFT_UNIT;
                }

                max_hex_val |= (val & TXT2IPV6_VAL_BIN_MASK);
                if (max_hex_val > TXT2IPV6_VAL_MAX_SIGN) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                } else {
                    curr_val = max_hex_val & TXT2IPV6_VAL_MAX_SIGN;
                }
            } else {
                if (val_cnt > TXT2IPV6_MAX_DOT_DIGIT_CNT) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }

                if (val > TXT2IPV6_MAX_DOT_VAL) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }

                curr_val = (curr_val * TXT2IPV6_DEC_VAL_SHIFT_UNIT) + val;
            }

            continue;
        }

        if (s[i] == ':') {
            if (dot_cnt) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }
            if (!col_cnt && ((i + 1) < len)) {
                if (!val_cnt && (hex2bin(s[i + 1]) != TXT2IPV6_INVALID_BIN_VAL)) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }
            }

            if (((i + 1) < len) && (s[i + 1] == ':')) {
                if (!flag) {
                    flag = TRUE;
                } else {
                    /* :: is only allowed once */
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }
            }

            col_cnt++;

            if (col_cnt > TXT2IPV6_MAX_COL_CNT) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            val_cnt = 0;
            continue;
        }

        if (s[i] == '.') {
            if (!col_cnt || !val_cnt) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            if (!dot_cnt) {
                if (prev_val) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }

                /* Check DOTTED IPv4-Mapped or IPv4-Compatible(Deprecated) */
                if (last_val == TXT2IPV6_IPV4COMPAT_SIGN) {
                    ipv4_compat = TRUE;
                } else if (last_val == TXT2IPV6_IPV4MAPPED_SIGN) {
                    ipv4_mapped = TRUE;
                } else {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }
            }

            if ((i + 1) < len) {
                if (hex2bin(s[i + 1]) == TXT2IPV6_INVALID_BIN_VAL) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }
            } else {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            if (curr_val > TXT2IPV6_V4TOV6_MAX_VALUE) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            dot_cnt++;

            if (dot_cnt > TXT2IPV6_MAX_DOT_CNT) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            val_cnt = 0;
            continue;
        }

        return TXT2IP6_ADDR_TYPE_INVALID;
    }
    if (col_cnt < TXT2IPV6_MIN_COL_CNT) {
        return TXT2IP6_ADDR_TYPE_INVALID;
    }
    if (dot_cnt) {
        if (dot_cnt < TXT2IPV6_MIN_DOT_CNT) {
            return TXT2IP6_ADDR_TYPE_INVALID;
        }

        if ((dot_cnt + col_cnt) > TXT2IPV6_MAX_SYMBOL_CNT) {
            return TXT2IP6_ADDR_TYPE_INVALID;
        }

        if (!flag) {
            j = TXT2IPV6_V4TOV6_ELECNT_BASE;    /* count element */
            for ((val_cnt = 0), (i = 0); i < len; i++) {
                if (s[i] == '.') {
                    break;
                }

                if (s[i] != ':') {
                    val_cnt++;
                } else {
                    if (val_cnt) {
                        j++;
                    }

                    val_cnt = 0;
                }
            }

            if (j != TXT2IPV6_MAX_ELEMENT_CNT) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }
        }
    } else {
        if (!flag && (col_cnt != TXT2IPV6_MAX_COL_CNT)) {
            return TXT2IP6_ADDR_TYPE_INVALID;
        }
    }

    /* assign value (IPv6 Address) second/next */
    if (ipv4_compat || ipv4_mapped) {
        u8  v4_val_set = 0;

        memset(addr, 0x0, sizeof(vtss_ipv6_t));
        j = TXT2IPV6_V4TOV6_MAX_OFFSET;
        for ((val_cnt = 0), (i = len); i > 0; i--) {
            if (s[i - 1] == ':') {
                addr->addr[TXT2IPV6_V4TOV6_COMPAT_BASE] = v4_val_set;

                if (s[i - 2] == ':') {
                    break;
                } else {
                    if (hex2bin(s[i - 2])) {
                        addr->addr[TXT2IPV6_V4TOV6_MAPPED_BASE] = TXT2IPV6_V4TOV6_MAPPED_SIGN;
                        addr->addr[TXT2IPV6_V4TOV6_MAPPED_CHK1] = TXT2IPV6_V4TOV6_MAPPED_SIGN;
                    }
                }

                break;
            }

            if (s[i - 1] == '.') {
                addr->addr[TXT2IPV6_V4TOV6_COMPAT_BASE + j] = v4_val_set;

                if (j == 0) {
                    break;
                }

                j--;
                val_cnt = 0;
                v4_val_set = 0;
            } else {
                val = hex2bin(s[i - 1]);

                k = TXT2IPV6_POWER_BASE_VAL;
                for (idx = 0; idx < val_cnt; idx++) {
                    k = k * TXT2IPV6_DEC_VAL_SHIFT_UNIT;
                }
                v4_val_set = v4_val_set + (val * k);
                val_cnt++;
            }
        }
    } else {
        j = 0;  /* count element */
        for ((val_cnt = 0), (i = 0); i < len; i++) {
            if (s[i] != ':') {
                val_cnt++;

                if ((i + 1) == len) {
                    j++;
                }
            } else {
                if (val_cnt) {
                    j++;
                }

                val_cnt = 0;
            }
        }

        if (!j) {
            memset(addr, 0x0, sizeof(vtss_ipv6_t));
        } else {
            u16 v6_val_set = 0;

            k = TXT2IPV6_MAX_ELEMENT_CNT - j;   /* zero-compress count */
            idx = TXT2IPV6_ADDR_IDX_BASE;
            for ((val_cnt = 0), (i = 0); i < len; i++) {
                if (s[i] == ':') {
                    addr->addr[idx] = v6_val_set >> TXT2IPV6_V6_ADDR_SHIFT_UNIT;
                    idx++;
                    addr->addr[idx] = v6_val_set & TXT2IPV6_V6_ADDR_SHIFT_MASK;
                    idx++;
                    if (idx > TXT2IPV6_ADDR_IDX_END) {
                        break;
                    }

                    if (((i + 1) < len) && (s[i + 1] == ':')) {
                        if (v6_val_set) {
                            j = 0;
                        } else {
                            j = 1;  /* already compress last one */
                        }
                        for (; j < k; j++) {
                            addr->addr[idx] = TXT2IPV6_V6_ADDR_ZERO_SIGN;
                            idx++;
                            addr->addr[idx] = TXT2IPV6_V6_ADDR_ZERO_SIGN;
                            idx++;
                            if (idx > TXT2IPV6_ADDR_IDX_END) {
                                break;
                            }
                        }

                        i++;
                    }

                    val_cnt = 0;
                    v6_val_set = 0;
                } else {
                    val_cnt++;

                    val = hex2bin(s[i]);
                    if (val_cnt - 1) {
                        v6_val_set = v6_val_set << TXT2IPV6_HEX_VAL_SHIFT_UNIT;
                    }

                    v6_val_set |= (val & TXT2IPV6_VAL_BIN_MASK);

                    if ((i + 1) == len) {
                        addr->addr[idx] = v6_val_set >> TXT2IPV6_V6_ADDR_SHIFT_UNIT;
                        idx++;
                        addr->addr[idx] = v6_val_set & TXT2IPV6_V6_ADDR_SHIFT_MASK;
                        idx++;
                        if (idx > TXT2IPV6_ADDR_IDX_END) {
                            break;
                        }
                    }
                }
            }
        }
    }

    /*
        Address Type could be determined precisely here/now
        Unspecified Address (ALL-ZERO)
        Unicast
        Multicast
        Link-Local
        Loopback
        IPv4-Mapped or IPv4-Compatible(Deprecated)
        ...
    */
    if (addr->addr[TXT2IPV6_ADDR_IDX_BASE] == TXT2IPV6_V6_MCAST_LEADING) {
        /* Multicast */
        return TXT2IP6_ADDR_TYPE_MCAST;
    } else {
        /* Unicast */
        if ((addr->addr[TXT2IPV6_ADDR_LL_CHK_IDX0] == TXT2IPV6_ADDR_LL_SIGN1) &&
            (addr->addr[TXT2IPV6_ADDR_LL_CHK_IDX1] == TXT2IPV6_ADDR_LL_SIGN2)) {
            /* Link Local */
            return TXT2IP6_ADDR_TYPE_LINK_LOCAL;
        }

        flag = FALSE;
        i = TXT2IPV6_ADDR_IDX_END;
        for (idx = TXT2IPV6_ADDR_IDX_BASE; idx <= i; idx++) {
            if (addr->addr[idx]) {
                flag = TRUE;
                break;
            }
        }

        if (!flag) {
            /* Unspecified */
            return TXT2IP6_ADDR_TYPE_UNSPECIFIED;
        }

        if ((idx == TXT2IPV6_ADDR_LB_CHK_IDX) &&
            (addr->addr[TXT2IPV6_ADDR_LB_CHK_IDX] == TXT2IPV6_ADDR_LB_SIGN)) {
            /* Loopback */
            return TXT2IP6_ADDR_TYPE_LOOPBACK;
        }

        j = addr->addr[TXT2IPV6_V4TOV6_COMPAT_BASE] & TXT2IPV6_V4_MCAST_LEADING;
        if (idx >= TXT2IPV6_V4TOV6_COMPAT_BASE) {
            /* IPv4 Multicast/Broadcast (Leading: 1110) is not allowed */
            if (j == TXT2IPV6_V4_MCAST_LEADING) {
                return TXT2IP6_ADDR_TYPE_INVALID;
            }

            /* IPv4-Compatible(Deprecated) */
            return TXT2IP6_ADDR_TYPE_IPV4_COMPAT;
        }
        if (idx == TXT2IPV6_V4TOV6_MAPPED_BASE) {
            if ((addr->addr[TXT2IPV6_V4TOV6_MAPPED_BASE] == TXT2IPV6_V4TOV6_MAPPED_SIGN) &&
                (addr->addr[TXT2IPV6_V4TOV6_MAPPED_CHK1] == TXT2IPV6_V4TOV6_MAPPED_SIGN)) {
                /* IPv4 Multicast/Broadcast (Leading: 1110) is not allowed */
                if (j == TXT2IPV6_V4_MCAST_LEADING) {
                    return TXT2IP6_ADDR_TYPE_INVALID;
                }

                /* IPv4-Mapped */
                return TXT2IP6_ADDR_TYPE_IPV4_MAPPED;
            }
        }

        /* General Unicast */
        return TXT2IP6_ADDR_TYPE_UCAST;
    }

    return TXT2IP6_ADDR_TYPE_GEN;
}
static int mgmt_txt2ipv6(const char *s, vtss_ipv6_t *addr)
{
    txt2ip6_addr_type_t retType = _mgmt_txt2ipv6(s, addr);

    if (retType == TXT2IP6_ADDR_TYPE_INVALID) {
        return 1;
    } else {
        if (retType == TXT2IP6_ADDR_TYPE_LOOPBACK) {
            return 1;
        } else {
            return 0;
        }
    }
}

static int cli_parm_1588_api_ipv4_addr_parse(char *cmd, cli_req_t *req, char *stx)
{
    int error;
    error = txt2_ipv4(cmd, &req->ipv4_addr);
    if (!error) {
        req->ipv4_addr_spec = TRUE;
    }
    return error;
}

static int cli_parm_1588_api_ipv4_mask_parse(char *cmd, cli_req_t *req, char *stx)
{
    int error;
    error = txt2_ipv4(cmd, &req->ipv4_mask);
    if (!error) {
        req->ipv4_mask_spec = TRUE;
    }
    return error;
}


static int cli_parm_1588_api_ipv6_addr_parse(char *cmd, cli_req_t *req, char *stx)
{
    int error;
    vtss_ipv6_t addr;
    error = mgmt_txt2ipv6(cmd, &addr);
    if (!error) {
        req->ipv6_addr[3] = addr.addr[ 0] << 24 | addr.addr[ 1] << 16 | addr.addr[ 2] << 8 | addr.addr[ 3];
        req->ipv6_addr[2] = addr.addr[ 4] << 24 | addr.addr[ 5] << 16 | addr.addr[ 6] << 8 | addr.addr[ 7];
        req->ipv6_addr[1] = addr.addr[ 8] << 24 | addr.addr[ 9] << 16 | addr.addr[10] << 8 | addr.addr[11];
        req->ipv6_addr[0] = addr.addr[12] << 24 | addr.addr[13] << 16 | addr.addr[14] << 8 | addr.addr[15];
        req->ipv4_addr_spec = TRUE;
    }
    return error;
}

static int cli_parm_1588_api_ipv6_mask_parse(char *cmd, cli_req_t *req, char *stx)
{
    int error;
    vtss_ipv6_t addr;

    error = mgmt_txt2ipv6(cmd, &addr);
    if (!error) {
        req->ipv6_mask[3] = addr.addr[ 0] << 24 | addr.addr[ 1] << 16 | addr.addr[ 2] << 8 | addr.addr[ 3];
        req->ipv6_mask[2] = addr.addr[ 4] << 24 | addr.addr[ 5] << 16 | addr.addr[ 6] << 8 | addr.addr[ 7];
        req->ipv6_mask[1] = addr.addr[ 8] << 24 | addr.addr[ 9] << 16 | addr.addr[10] << 8 | addr.addr[11];
        req->ipv6_mask[0] = addr.addr[12] << 24 | addr.addr[13] << 16 | addr.addr[14] << 8 | addr.addr[15];
        req->ipv4_mask_spec = TRUE;
    }
    return error;
}

static int cli_parm_1588_api_stk_depth_parse(char *cmd, cli_req_t *req, char *stx)
{
    int error;
    u32 value;

    error = cli_parse_ulong(cmd, &value, 1, 4);
    if (!error) {
        req->stk_depth_spec = TRUE;
        switch(value) {
        case 1:
            req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_1;
            break;
        case 2:
            req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_2;
            break;
        case 3:
            req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_3;
            break;
        case 4:
            req->stk_depth = VTSS_PHY_TS_MPLS_STACK_DEPTH_4;
            break;
        default:
            break; 
        }
    }
    return error;
}

static int cli_parm_1588_api_stk_lvl_0_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    u32 low, high;
    if (!(error = cli_parse_word(cmd, "stk_lvl_0"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        req->stk_lvl_0 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = mgmt_txt2range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                req->stk_lvl_0_lower = low;
                req->stk_lvl_0_upper = high;
            }
        } else {
                req->stk_lvl_0_lower = low;
                req->stk_lvl_0_upper = low;
        }
    }

    return error;
}

static int cli_parm_1588_api_stk_lvl_1_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    u32 low, high;

    if (!(error = cli_parse_word(cmd, "stk_lvl_1"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        req->stk_lvl_1 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = mgmt_txt2range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                req->stk_lvl_1_lower = low;
                req->stk_lvl_1_upper = high;
            }
        } else {
                req->stk_lvl_1_lower = low;
                req->stk_lvl_1_upper = low;
        }
    }

    return error;
}

static int cli_parm_1588_api_stk_lvl_2_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    u32 low, high;

    if (!(error = cli_parse_word(cmd, "stk_lvl_2"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        req->stk_lvl_2 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = mgmt_txt2range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                req->stk_lvl_2_lower = low;
                req->stk_lvl_2_upper = high;
            }
        } else {
                req->stk_lvl_2_lower = low;
                req->stk_lvl_2_upper = low;
        }
    }

    return error;
}

static int cli_parm_1588_api_stk_lvl_3_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int error;
    u32 low, high;

    if (!(error = cli_parse_word(cmd, "stk_lvl_3"))) {
        if (cmd2 == NULL) {
            return 1;
        }
        req->stk_lvl_3 = TRUE;
        req->parm_parsed = 2;

        /* check for specific value */
        error = cli_parse_ulong(cmd2, &low, 0, 0xFFFFF);
        if (error) {
            /* check for range */
            error = mgmt_txt2range(cmd2, &low, &high, 0, 0xFFFFF);
            if (!error) {
                req->stk_lvl_3_lower = low;
                req->stk_lvl_3_upper = high;
            }
        } else {
                req->stk_lvl_3_lower = low;
                req->stk_lvl_3_upper = low;
        }
    }

    return error;
}


static int cli_parm_1588_api_cw_parse(char *cmd, cli_req_t *req, char *stx)
{
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "cw_en", 5)) {
            req->cw_en = TRUE;
        } else if (!strncmp(found, "cw_dis", 6)) {
            req->cw_en = FALSE;
        }
    }
    return (found == NULL ? 1 : 0);
}

#endif /* VTSS_FEATURE_PHY_TIMESTAMP */


static int cli_parse_trace_group(char *cmd, cli_req_t *req)
{
    char c = cmd[1];

    switch (*cmd) {
    case 'm':
        req->group = VTSS_APPL_TRACE_GROUP;
        break;
    case 'd':
        req->group = VTSS_TRACE_GROUP_DEFAULT;
        break;
    case 'p':
        req->group = (c == 'o' ? VTSS_TRACE_GROUP_PORT :
                      c == 'h' ? VTSS_TRACE_GROUP_PHY : VTSS_TRACE_GROUP_PACKET);
        break;
    case 'q':
        req->group = VTSS_TRACE_GROUP_QOS;
        break;
    case 'l':
        req->group = VTSS_TRACE_GROUP_L2;
        break;
#if defined VTSS_ARCH_DAYTONA
    case 'u':
        req->group = VTSS_TRACE_GROUP_UPI;
        break;
    case 'x':
        req->group = VTSS_TRACE_GROUP_XFI;
        break;
#endif /* VTSS_ARCH_DAYTONA */
    default:
        return 1;
    } 
    return 0;
}

static const char * const cli_api_group_table[VTSS_DEBUG_GROUP_COUNT] = {
    [VTSS_DEBUG_GROUP_ALL]       = "all",
    [VTSS_DEBUG_GROUP_INIT]      = "init",
    [VTSS_DEBUG_GROUP_MISC]      = "misc",
    [VTSS_DEBUG_GROUP_PORT]      = "port",
    [VTSS_DEBUG_GROUP_PORT_CNT]  = "counters",
    [VTSS_DEBUG_GROUP_PHY]       = "phy",
    [VTSS_DEBUG_GROUP_VLAN]      = "vlan",
    [VTSS_DEBUG_GROUP_PVLAN]     = "pvlan",
    [VTSS_DEBUG_GROUP_MAC_TABLE] = "mac_table",
    [VTSS_DEBUG_GROUP_ACL]       = "acl",
    [VTSS_DEBUG_GROUP_QOS]       = "qos",
    [VTSS_DEBUG_GROUP_AGGR]      = "aggr",
    [VTSS_DEBUG_GROUP_GLAG]      = "glag",
    [VTSS_DEBUG_GROUP_STP]       = "stp",
    [VTSS_DEBUG_GROUP_MIRROR]    = "mirror",
    [VTSS_DEBUG_GROUP_EVC]       = "evc",
    [VTSS_DEBUG_GROUP_ERPS]      = "erps",
    [VTSS_DEBUG_GROUP_EPS]       = "eps",
    [VTSS_DEBUG_GROUP_PACKET]    = "packet",
    [VTSS_DEBUG_GROUP_FDMA]      = "fdma",
    [VTSS_DEBUG_GROUP_TS]        = "ts",
    [VTSS_DEBUG_GROUP_WM]        = "wm",
    [VTSS_DEBUG_GROUP_LRN]       = "lrn",
    [VTSS_DEBUG_GROUP_IPMC]      = "ipmc",
    [VTSS_DEBUG_GROUP_STACK]     = "stack",
    [VTSS_DEBUG_GROUP_CMEF]      = "cmef",
    [VTSS_DEBUG_GROUP_HOST]      = "host",
    [VTSS_DEBUG_GROUP_MPLS]      = "mpls_base",
    [VTSS_DEBUG_GROUP_MPLS_OAM]  = "mpls_oam",
    [VTSS_DEBUG_GROUP_VXLAT]     = "vxlat",
    [VTSS_DEBUG_GROUP_OAM]       = "oam",
    [VTSS_DEBUG_GROUP_SER_GPIO]  = "sgpio",
    [VTSS_DEBUG_GROUP_L3]        = "l3",
    [VTSS_DEBUG_GROUP_AFI]       = "afi",
    [VTSS_DEBUG_GROUP_MACSEC]    = "macsec",
#ifdef VTSS_ARCH_DAYTONA
    [VTSS_DEBUG_GROUP_XFI]       = "xfi",
    [VTSS_DEBUG_GROUP_UPI]       = "upi",
    [VTSS_DEBUG_GROUP_PCS_10GBASE_R] = "pcs10g",
    [VTSS_DEBUG_GROUP_MAC10G]    = "mac10g",
    [VTSS_DEBUG_GROUP_WIS]       = "wis",
    [VTSS_DEBUG_GROUP_RAB]       = "rab",
    [VTSS_DEBUG_GROUP_XAUI]      = "xaui",
    [VTSS_DEBUG_GROUP_OTN]       = "otn",
    [VTSS_DEBUG_GROUP_GFP]       = "gfp",
#endif /* VTSS_ARCH_DAYTONA */
};
#if defined(VTSS_ARCH_DAYTONA) || defined(VTSS_FEATURE_WIS)
static const char *cli_bool_disp(BOOL b)
{
#if 0
    return (b ? "True" : "False");
#else
    return (b ? "1" : "0");
#endif
}
#endif /* VTSS_ARCH_DAYTONA || VTSS_FEATURE_WIS*/

#ifdef VTSS_ARCH_DAYTONA
static const char *cli_upi_proto_disp(vtss_upi_protocol_t p)
{
    switch (p) {
        case tfi5: return "tfi5";
        case xaui: return "xaui";
        case sfi4: return "sfi4";
        default: return "undef";
    }
}

static const char *cli_upi_test_mode_disp(vtss_upi_test_mode_t p)
{
    switch (p) {
        case VTSS_UPI_TEST_MODE_DISABLE: return "DISABLE";
        case VTSS_UPI_TEST_MODE_PRBS7_P: return "PRBS7_P";
        case VTSS_UPI_TEST_MODE_PRBS7_N: return "PRBS7_N";
        case VTSS_UPI_TEST_MODE_PRBS23_P: return "PRBS23_P";
        case VTSS_UPI_TEST_MODE_PRBS23_N: return "PRBS23_N";
        case VTSS_UPI_TEST_MODE_PRBS31_P: return "PRBS31_P";
        case VTSS_UPI_TEST_MODE_PRBS31_N: return "PRBS31_N";
        case VTSS_UPI_TEST_MODE_HF: return "HF";
        case VTSS_UPI_TEST_MODE_LF: return "LF";
        case VTSS_UPI_TEST_MODE_PRBS9_P: return "PRBS9_P";
        default: return "undef";
    }
}

static const char *cli_upi_wrap_mode_disp(vtss_upi_test_wrap_t p)
{
    switch (p) {
        case VTSS_UPI_TEST_WRAP_RX: return "RxInput";
        case VTSS_UPI_TEST_WRAP_INT_LOOP: return "IntLoop";
        case VTSS_UPI_TEST_WRAP_INT_WRAP: return "IntWrap";
        default: return "undef";
    }
}

static const char *cli_xfi_test_mode_disp(vtss_xfi_test_mode_t p)
{
    switch (p) {
        case VTSS_XFI_TEST_MODE_DISABLE: return "DISABLE";
        case VTSS_XFI_TEST_MODE_PRBS7_P: return "PRBS7_P";
        case VTSS_XFI_TEST_MODE_PRBS7_N: return "PRBS7_N";
        case VTSS_XFI_TEST_MODE_PRBS23_P: return "PRBS23_P";
        case VTSS_XFI_TEST_MODE_PRBS23_N: return "PRBS23_N";
        case VTSS_XFI_TEST_MODE_PRBS31_P: return "PRBS31_P";
        case VTSS_XFI_TEST_MODE_PRBS31_N: return "PRBS31_N";
        case VTSS_XFI_TEST_MODE_HF: return "HF";
        case VTSS_XFI_TEST_MODE_LF: return "LF";
        default: return "undef";
    }
}
#endif /* VTSS_ARCH_DAYTONA */

static int cli_parse_api_layer(char *cmd, cli_req_t *req)
{
    char stx[16];

    strcpy(stx, "ail|cil");
    return cli_parse_keyword(cmd, req, stx);
}
    
static int cli_parse_api_group(char *cmd, cli_req_t *req)
{
    int                error = 1;
    const char         *txt = "show";
    vtss_debug_group_t group;
    
    /* Accept 'show' keyword to display groups */
    if (strstr(txt, cmd) == txt) {
        req->api_group = VTSS_DEBUG_GROUP_COUNT;
        return 0;
    }
    
    for (group = VTSS_DEBUG_GROUP_ALL; group < VTSS_DEBUG_GROUP_COUNT; group++) {
        txt = cli_api_group_table[group];
        if (txt != NULL && strstr(txt, cmd) == txt) {
            /* Found matching group */
            if (error) {
                error = 0;
                req->api_group = group;
            } else {
                error = 1;
                break;
            }
        }
    }
    return error;
}
#ifdef VTSS_ARCH_DAYTONA
static int cli_parse_otn_oh_insertion(char *cmd, cli_req_t *req)
{
    switch (*cmd) {
        case '0':
            req->oh_ins_mode = VTSS_OTN_ODU_OH_INS_NORMAL;
            break;
        case '1':
            req->oh_ins_mode = VTSS_OTN_ODU_OH_INS_PORT;
            break;
        case '2':
            req->oh_ins_mode = VTSS_OTN_ODU_OH_INS_BLANK;
            break;
        case '3':
            req->oh_ins_mode = VTSS_OTN_ODU_OH_INS_MAX;
            break;
        default:
            return 1;
    }
    return 0;
}
static int cli_parse_otn_otu_oh_insertion(char *cmd, cli_req_t *req)
{
    switch (*cmd) {
        case '0':
            req->oh_ins_mode = VTSS_OTN_OTU_OH_INS_NORMAL;
            break;
        case '1':
            req->oh_ins_mode = VTSS_OTN_OTU_OH_INS_PORT;
            break;
        case '2':
            req->oh_ins_mode = VTSS_OTN_OTU_OH_INS_BLANK;
            break;
        case '3':
            req->oh_ins_mode = VTSS_OTN_OTU_OH_INS_MAX;
            break;
        default:
            return 1;
    }
    return 0;
}
static int cli_parse_otn_opu_oh_insertion(char *cmd, cli_req_t *req)
{
    switch (*cmd) {
        case '0':
            req->opu_oh_ins_mode = VTSS_OTN_ODU_OPU_OH_INS_NORMAL;
            break;
        case '1':
            req->opu_oh_ins_mode = VTSS_OTN_ODU_OPU_OH_INS_PORT;
            break;
        case '2':
            req->opu_oh_ins_mode = VTSS_OTN_ODU_OPU_OH_INS_BLANK;
            break;
        case '3':
            req->opu_oh_ins_mode = VTSS_OTN_ODU_OPU_OH_INS_MAX;
            break;
        default:
            return 1;
    }
    return 0;
}
static int cli_parse_otn_test_insertion(char *cmd, cli_req_t *req)
{
    switch (*cmd) {
        case '0':
            req->test_ins_mode = VTSS_OTN_ODU_OPU_TEST_INS_NONE;
            break;
        case '1':
            req->test_ins_mode = VTSS_OTN_ODU_OPU_TEST_INS_FIXED;
            break;
        case '2':
            req->test_ins_mode = VTSS_OTN_ODU_OPU_TEST_INS_PN_23;
            break;
        case '3':
            req->test_ins_mode = VTSS_OTN_ODU_OPU_TEST_INS_PN_31;
            break;
        case '4':
            req->test_ins_mode = VTSS_OTN_ODU_OPU_TEST_INS_MAX;
            break;
        default:
            return 1;
    }
    return 0;
}
#endif /* VTSS_ARCH_DAYTONA */
/* Parse parameter */
static int cli_parse_parm(cli_parm_type_t type, cli_cmd_t *cli_cmd,
                          char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    u32 value;
#if defined(VTSS_FEATURE_WIS)
    u32 maxlen = 1;
#endif /* defined(VTSS_FEATURE_WIS) */

    req->parm_parsed = 1;
    switch (type) {
#ifdef VTSS_ARCH_DAYTONA
    case CLI_PARM_TYPE_EVENT:
        error = cli_parse_event(cmd, req, stx);
        break;
#endif /* VTSS_ARCH_DAYTONA */
    case CLI_PARM_TYPE_PORT:
        if (!(error = cli_parse_ulong(cmd, &value, 1, 4)))
            req->port = value-1;
        break;
    case CLI_PARM_TYPE_SYSTEM_INST:
        if (!(error = cli_parse_ulong(cmd, &value, 0, 1)))
            req->appl_inst = value;
        break;
    case CLI_PARM_TYPE_PORT_LIST:
        error = cli_parse_list(cmd, req->port_list, 1, VTSS_PORTS, 1);
        cli_remove_unused_ports(req);
        break;
    case CLI_PARM_TYPE_PORT_DIS:
        error = (cli_parse_ulong(cmd, &req->port, VTSS_PORT_NO_START, VTSS_PORT_NO_END) &&
                 cli_parse_keyword(cmd, req, stx));
        break;
#if defined(VTSS_FEATURE_PORT_CONTROL)
    case CLI_PARM_TYPE_MAX_FRAME:
        error = cli_parse_ulong(cmd, &req->value, 
                                VTSS_MAX_FRAME_LENGTH_STANDARD, VTSS_MAX_FRAME_LENGTH_MAX);
        break;
#endif /* VTSS_FEATURE_PORT_CONTROL */
    case CLI_PARM_TYPE_VID:
        error = cli_parse_ulong(cmd, &value, VTSS_VID_DEFAULT, VTSS_VID_RESERVED);
        if (!error)
            req->vid = value;
        break;
    case CLI_PARM_TYPE_MAC_AGE_TIME:
        error = (cli_parse_ulong(cmd, &value, 0, 1000000) || (value != 0 && value < 10));
        req->value = value;
        break;
    case CLI_PARM_TYPE_PSI_CLOCK:
        error = (cli_parse_ulong(cmd, &value, 0, 10000000));
        req->value = value;
        break;
    case CLI_PARM_TYPE_MAC:
        error = cli_parse_mac(cmd, req->mac); 
        break;
    case CLI_PARM_TYPE_KEYWORD:
        error = cli_parse_keyword(cmd, req, stx);
        break;
    case CLI_PARM_TYPE_TGT_LIST:
        error = cli_parse_list(cmd, req->blk_list, 0, CLI_TGT_MAX - 1, 0);
        break;
    case CLI_PARM_TYPE_BLK_LIST:
        error = cli_parse_list(cmd, req->blk_list, 0, CLI_BLK_MAX - 1, 0);
        break;
    case CLI_PARM_TYPE_SUB_LIST:
        error = cli_parse_list(cmd, req->sub_list, 0, CLI_SUB_MAX - 1, 0);
        break;
    case CLI_PARM_TYPE_ADDR_LIST:
        error = cli_parse_list(cmd, req->addr_list, 0, CLI_ADDR_MAX - 1, 0);
        break;
    case CLI_PARM_TYPE_VALUE:
        error = cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
        break;
    case CLI_PARM_TYPE_PHY_ADDR_LIST:
        error = cli_parse_list(cmd, req->addr_list, 0, 31, 0);
        break;
    case CLI_PARM_TYPE_PHY_VALUE:
        error = cli_parse_ulong(cmd, &req->value, 0, 0xffff);
        break;
    case CLI_PARM_TYPE_FPGA_ADDR_LIST:
        error = cli_parse_list(cmd, req->addr_list, 0, 255, 0);
        break;
    case CLI_PARM_TYPE_PAGE:
        error = cli_parse_ulong(cmd, &req->page, 0, 0xffff);
        break;
#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
    case CLI_PARM_TYPE_MMD_LIST:
        error = cli_parse_list(cmd, req->mmd_list, 0, 31, 0);
        break;
    case CLI_PARM_TYPE_MMD_ADDR:
        error = cli_parse_ulong(cmd, &value, 0, 0xffff);
        if (!error)
            req->mmd_addr = value;
        break;
#endif /* VTSS_FEATURE_10G */
    case CLI_PARM_TYPE_API_LAYER:
        error = cli_parse_api_layer(cmd, req);
        break;
    case CLI_PARM_TYPE_API_GROUP:
        error = cli_parse_api_group(cmd, req);
        break;
    case CLI_PARM_TYPE_TRACE_LAYER:
        error = cli_parse_trace_layer(cmd, req);
        break;
    case CLI_PARM_TYPE_TRACE_GROUP:
        error = cli_parse_trace_group(cmd, req);
        break;
    case CLI_PARM_TYPE_ETYPE:
        error = cli_parse_ulong(cmd, &req->etype, 0, 0xffff);
        break;
#if defined(VTSS_GPIOS)
    case CLI_PARM_TYPE_GPIO_LIST:
        error = cli_parse_list(cmd, req->gpio_list, 
                               VTSS_GPIO_NO_START, VTSS_GPIO_NO_END - 1, 1);
        break;
    case CLI_PARM_TYPE_GPIO_VALUE:
        error = cli_parse_ulong(cmd, &req->value, 0, 1);
        break;
#if defined(VTSS_ARCH_B2)
    case CLI_PARM_TYPE_CLK_GPIO_NO:
        error = cli_parse_ulong(cmd, &req->gpio_no, 0, 1);
        break;
    case CLI_PARM_TYPE_CLK_FREQ:
        error = cli_parse_ulong(cmd, &req->clk_freq, 1, 7);
        break;
    case CLI_PARM_TYPE_LPORT_LIST:
        error = cli_parse_list(cmd, req->lport_list, 0, VTSS_LPORTS-1, 1);
        break;
    case CLI_PARM_TYPE_LPORT_DIS:
        error = (cli_parse_ulong(cmd, &req->lport, 0, VTSS_LPORTS-1) &&
                 cli_parse_keyword(cmd, req, stx));
        break;
    case CLI_PARM_TYPE_TABLE_ID:        
        error = cli_parse_ulong(cmd, &req->table_id, 1, 2);
        break;
    case CLI_PARM_TYPE_TABLE_INDEX:
        error = cli_parse_ulong(cmd, &req->table_index, 1, 8);
        break;
    case CLI_PARM_TYPE_QUEUE_INDEX:
        error = cli_parse_ulong(cmd, &req->queue_index, 0, 7);
        break;
    case CLI_PARM_TYPE_PCT:
        error = cli_parse_ulong(cmd, &req->value, 0, 100);
        break;
    case CLI_PARM_TYPE_DSCP_LIST:
        error = cli_parse_list(cmd, req->dscp_list, 0, 63, 1);
    case CLI_PARM_TYPE_CT_THRESHOLD:
        error = cli_parse_ulong(cmd, &req->value, 0, 10240);
        break;
    case CLI_PARM_TYPE_TRIG_INDEX:
        error = cli_parse_ulong(cmd, &req->trig_index, 0, 6);
        break;
    case CLI_PARM_TYPE_OFFSET_VALUE:
        error = cli_parse_ulong(cmd, &req->offset, 0, 127);
        break;
    case CLI_PARM_TYPE_PATTERN_VALUE:
        error = cli_parse_ulong(cmd, &req->pattern, 0, 0xFFFF);
        break;
    case CLI_PARM_TYPE_MASK_VALUE:
        error = cli_parse_ulong(cmd, &req->mask, 0, 0xFFFF);
        break;
    case CLI_PARM_TYPE_TCP_UDP_PORT:
        error = cli_parse_ulong(cmd, &req->tcp_udp_port, 0, 0xffff);
        break;
    case CLI_PARM_TYPE_QOS_ORDER:
        error = cli_parse_ulong(cmd, &req->order, 0, 16);
        break;
    case CLI_PARM_TYPE_QOS_ENDPOINT:
        error = cli_parse_ulong(cmd, &req->endpoint, 0, 16);
        break;
    case CLI_PARM_TYPE_TAG_PRIO:
        error = cli_parse_ulong(cmd, &req->tag_prio, 0, 7);
        break;
    case CLI_PARM_TYPE_MPLS_EXP:
        error = cli_parse_ulong(cmd, &req->mpls_exp, 0, 7);
        break;
    case CLI_PARM_TYPE_VLAN:
        error = cli_parse_ulong(cmd, &req->vlan, 0, 4095);
        break;
    case CLI_PARM_TYPE_IP_PROTO:
        error = cli_parse_ulong(cmd, &req->ip_proto, 0, 255);
        break;
    case CLI_PARM_TYPE_DMAC:
        error = cli_parse_mac(cmd, req->dmac); 
        break;
    case CLI_PARM_TYPE_DMAC_MASK:
        error = cli_parse_mac(cmd, req->dmac_mask); 
        break;
    case CLI_PARM_TYPE_CLASS_VALUE:
        error = cli_parse_ulong(cmd, &req->class, 0, 7);
        break;
    case CLI_PARM_TYPE_RED_VALUE:
        error = cli_parse_ulong(cmd, &req->red, 0, 3);
        break;
    case CLI_PARM_TYPE_QOS_BIT_RATE:
        error = cli_parse_ulong(cmd, &req->rate, 0, 10000000);
        break;
    case CLI_PARM_TYPE_QOS_LEVEL:
        error = cli_parse_ulong(cmd, &req->policer.level, 0, 10000000);
        break;
    case CLI_PARM_TYPE_RED_MIN:
        error = cli_parse_ulong(cmd, &req->red_min, 0, 32640);
        break;
    case CLI_PARM_TYPE_RED_MAX:
        error = cli_parse_ulong(cmd, &req->red_max, 0, 65280);
        break;
    case CLI_PARM_TYPE_RED_WEIGHT:
        error = cli_parse_ulong(cmd, &req->red_weight, 0, 31);
        break;
    case CLI_PARM_TYPE_RED_PROB_1:
        error = cli_parse_ulong(cmd, &req->red_prob1, 0, 100);
        break;
    case CLI_PARM_TYPE_RED_PROB_2:
        error = cli_parse_ulong(cmd, &req->red_prob2, 0, 100);
        break;
    case CLI_PARM_TYPE_RED_PROB_3:
        error = cli_parse_ulong(cmd, &req->red_prob3, 0, 100);
        break;
#endif /* VTSS_ARCH_B2 */
#endif /* VTSS_GPIOS */
#if defined(VTSS_FEATURE_WIS)
    case CLI_PARM_TYPE_WIS_TTI_MODE:
        error = cli_parse_ulong(cmd, &value, 0, 2);
        if (!error) { 
            req->wis_tti_mode = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_TTI:
        if (req->wis_tti_mode == 0)
            maxlen= 1;
        else if (req->wis_tti_mode == 1)
            maxlen = 16;
        else if (req->wis_tti_mode == 2)
            maxlen = 64;
        error = cli_parse_txt(cmd, req->wis_tti, maxlen);
        if (!error) {
            req->wis_tti_set = TRUE;
        }
        break;
    case CLI_PARM_TYPE_WIS_SL:
        error = cli_parse_ulong(cmd, &req->sl, 0, 255);
        break;
    case CLI_PARM_TYPE_WIS_AISL:
        error = cli_parse_ulong(cmd, &value, 0, 0x3);
        if (!error) {
            req->wis_aisl = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_RDIL:
        error = cli_parse_ulong(cmd, &value, 0, 0xF);
        if (!error) {
            req->wis_rdil= value;
        }
        break;
    case CLI_PARM_TYPE_WIS_FAULT:
        error = cli_parse_ulong(cmd, &value, 0, 0x7FF);
        if (!error) {
            req->wis_fault = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_OH_NAME:
        cli_misc_wis_overhead_name_parse(cmd, cmd2, stx, cmd_org, req);
        break;
    case CLI_PARM_TYPE_WIS_OH_VALUE:
        cli_misc_wis_overhead_value_parse(cmd, cmd2, stx, cmd_org, req);
        break;
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
    case CLI_PARM_TYPE_WIS_LINERX:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) {
            req->wis_line_rx = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_LINETX:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) { 
            req->wis_line_tx = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_PATH_FORCE:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) {
            req->wis_path_force = value;
        }
        break;
    case CLI_PARM_TYPE_WIS_NETS:
        error = cli_parse_ulong(cmd, &req->wis_n_ebc_thr_s, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_WIS_NETL:
        error = cli_parse_ulong(cmd, &req->wis_n_ebc_thr_l, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_WIS_FETL:
        error = cli_parse_ulong(cmd, &req->wis_f_ebc_thr_l, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_WIS_NETP:
        error = cli_parse_ulong(cmd, &req->wis_n_ebc_thr_p, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_WIS_FETP:
        error = cli_parse_ulong(cmd, &req->wis_f_ebc_thr_p, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_WIS_FORCE_EVENTS:
        error = cli_parse_ulong(cmd, &req->wis_force_events, 0, 0xFFFFFFFF);
        break; 
#endif
    case CLI_PARM_TYPE_WIS_PASSTHRU:
        error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
        if (!error) {
            req->wis_passthru = value;
        }
        break;
#endif
#if defined(VTSS_FEATURE_OTN)
    case CLI_PARM_TYPE_OTN_TTI:
        error = cli_parse_txt(cmd, req->otn_tti, 64);
        req->otn_tti_set = TRUE;
        break;
    case CLI_PARM_TYPE_OTN_SAPI_TIM:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_DAPI_TIM:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_OS_TIM:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_EX_TTI:
        error = cli_parse_txt(cmd, req->otn_extti, 64);
        req->otn_extti_set = TRUE;
        break;
    case CLI_PARM_TYPE_OTN_OCH_FEC:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_OCH_BER_THRES:
        error = cli_parse_ulong(cmd, &req->ber_threshold, 0, 65535);
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BEIBIAE:
        error = cli_parse_ulong(cmd, &req->beibiae, 0, 1);
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI:
        error = cli_parse_ulong(cmd, &req->bdi, 0, 1);   
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_AIS:
        error = cli_parse_ulong(cmd, &req->ais, 0, 1); 
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_IAE:
        error = cli_parse_ulong(cmd, &req->iae, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOS:
        error = cli_parse_ulong(cmd, &req->bdi_los, 0, 1);
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOF:
        error = cli_parse_ulong(cmd, &req->bdi_lof, 0, 1); 
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_LOM:
        error = cli_parse_ulong(cmd, &req->bdi_lom, 0, 1);
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_AIS:
        error = cli_parse_ulong(cmd, &req->bdi_ais, 0, 1); 
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_BDI_TIM:
        error = cli_parse_ulong(cmd, &req->bdi_tim, 0, 1);
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_LOM:
        error = cli_parse_ulong(cmd, &req->ais_lom, 0, 1); 
        break;
    case CLI_PARM_TYPE_OTN_OTU_CONACT_AIS_TIM:
        error = cli_parse_ulong(cmd, &req->ais_tim, 0, 1);
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BEI:
        error = cli_parse_ulong(cmd, &req->bei, 0, 1);   
        break;
    case CLI_PARM_TYPE_OTN_ODU_MODE:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI:
        error = cli_parse_ulong(cmd, &req->odu_bdi, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOS:
        error = cli_parse_ulong(cmd, &req->odu_bdi_los, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOF:
        error = cli_parse_ulong(cmd, &req->odu_bdi_lof, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LOM:
        error = cli_parse_ulong(cmd, &req->odu_bdi_lom, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_AIS:
        error = cli_parse_ulong(cmd, &req->odu_bdi_otu_ais, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OTU_TIM:
        error = cli_parse_ulong(cmd, &req->odu_bdi_otu_tim, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_AIS:
        error = cli_parse_ulong(cmd, &req->odu_bdi_ais, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_OCI:
        error = cli_parse_ulong(cmd, &req->odu_bdi_oci, 0, 1);   
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LCK:
        error = cli_parse_ulong(cmd, &req->odu_bdi_lck, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_LTC:
        error = cli_parse_ulong(cmd, &req->odu_bdi_ltc, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_BDI_TIM:
        error = cli_parse_ulong(cmd, &req->odu_bdi_tim, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_AIS:
        error = cli_parse_ulong(cmd, &req->odu_ais, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_TIMOCILCK:
        error = cli_parse_ulong(cmd, &req->odu_ais_timocilck, 0, 1);   
        break;
    case CLI_PARM_TYPE_OTN_ODU_CONACT_AIS_PLM:
        error = cli_parse_ulong(cmd, &req->odu_ais_plm, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_PL_PLM:
        error = cli_parse_ulong(cmd, &req->odu_pl_plm, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_PL_TX:
        error = cli_parse_ulong(cmd, &req->odu_pl_tx, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_PL_PAYLOAD_TYPE:
        error = cli_parse_ulong(cmd, &req->odu_pl_type, 0, 0xffffffff);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_SIAIS:
        error = cli_parse_ulong(cmd, &req->si_ais, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODU_SOAIS:
        error = cli_parse_ulong(cmd, &req->so_ais, 0, 1);  
        break;
    case CLI_PARM_TYPE_OTN_ODUT_LEVEL:
        error = cli_parse_ulong(cmd, &req->odut_level, 1, 6);
        req->odut_level--;
        break;
    case CLI_PARM_TYPE_OTN_ODUT_BDI:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_OTN_ODUT_STAT:
        error = cli_parse_ulong(cmd, &req->odut_stat, 0, 7);   
        break;
    case CLI_PARM_TYPE_OTN_ODUT_APS:
        if (req->odut_aps_count < VTSS_OTN_ODU_APS_NUMBER)
        {
            u32 value = 0;
            error = cli_parse_ulong(cmd, &value, 0, 0xFF);
            if (!error) {
                req->odut_aps[req->odut_aps_count] = value;
                req->odut_aps_count++;
            }
        }
        break;
    case CLI_PARAM_TYPE_OTN_VALUE:
        error = cli_parse_ulong(cmd, &req->otn_value, 0, 0xffffffff);
        break;
    case CLI_PARAM_TYPE_OTN_OH_INSERTION:
        error = cli_parse_otn_oh_insertion(cmd, req);
        break;
    case CLI_PARAM_TYPE_OTN_OPU_OH_INSERTION:
        error = cli_parse_otn_opu_oh_insertion(cmd, req);
        break;
    case CLI_PARAM_TYPE_OTN_OTU_OH_INSERTION:
        error = cli_parse_otn_otu_oh_insertion(cmd,req);
        break;
    case CLI_PARAM_TYPE_OTN_TEST_INSERTION:
        error = cli_parse_otn_test_insertion(cmd, req);
        break;
    case CLI_PARM_TYPE_FTFL_VALUE:
        error = cli_parse_ulong(cmd, &req->ftfl_value, 0, 0xff);
        break;
    case CLI_PARM_TYPE_FTFL_INDEX:
        error = cli_parse_ulong(cmd, &req->ftfl_index, 1, 256);
        break;
#endif /* VTSS_FEATURE_OTN */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    case CLI_PARM_TYPE_1588_PORTNO:
        error = cli_parse_ulong(cmd, &value, 1, VTSS_PORTS);
        if (!error) {
            req->port_no = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_ENGID:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) {
            req->eng_id = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_CLK_FREQ:
        error = cli_parse_ulong(cmd, &value, 0, 4);
        if (!error) {
            req->clkfreq = (u8)value;
            req->clk_freq_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_CLK_SRC:
        error = cli_parse_ulong(cmd, &value, 0, 5);
        if (!error) {
            req->clk_src = (u8)value;
            req->clk_src_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_RX_TS_POS:
        error = cli_parse_ulong(cmd, &value, 0, 1);
        if (!error) {
            req->rx_ts_pos = (u8)value;
            req->rx_ts_pos_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_TX_FIFO_MODE:
        error = cli_parse_ulong(cmd, &value, 0, 1);
        if (!error) {
            req->tx_fifo_mode = (u8)value;
            req->tx_fifo_mode_spec = TRUE;
        }
        break;
    case CLI_PARM_MODIFY_BIT_7:
        error = cli_parse_ulong(cmd, &value, 0, 1);
        if(!error){
            req->modify_frm = TRUE;
         }
        break;
    case CLI_PARM_TYPE_1588_ACTID:
        error = cli_parse_ulong(cmd, &value, 0, 5);
        if (!error) {
            req->action_id = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_DIRECTION:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_ENCAP_TYPE:
        error = cli_parse_ulong(cmd, &value, VTSS_PHY_TS_ENCAP_ETH_PTP, VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM);
        if (!error) {
            req->encap_type = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_CHNLMAP:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) {
            req->channel_map = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_FLOW_ID:
        error = cli_parse_ulong(cmd, &value, 0, 7);
        if (!error) {
           req->flow_id_spec = TRUE;
           req->flow_id = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_FLOW_ST_IDX:
        error = cli_parse_ulong(cmd, &value, 0, 7);
        if (!error) {
            req->flow_st_index = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_FLOW_END_IDX:
        error = cli_parse_ulong(cmd, &value, 0, 7);
        if (!error) {
            req->flow_end_index = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TPID:
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (!error) {
            req->tpid = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_NUMBER_OF_TAGS:
        error = cli_parse_ulong(cmd, &value, 0, 2);
        if (!error) {
            req->num_tag = (u8)value;
            req->num_tag_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG1_TYPE:
        error = cli_parse_ulong(cmd, &value, 0, 4);
        if (!error) {
            req->tag1_type = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG2_TYPE:
        error = cli_parse_ulong(cmd, &value, 0, 4);
        if (!error) {
            req->tag2_type = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG1_UPPER:
        error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
        if (!error) {
            req->tag1_upper = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG1_LOWER:
        error = cli_parse_ulong(cmd, &value, 1, 4094);
        if (!error) {
            req->tag1_lower = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG2_UPPER:
        error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
        if (!error) {
            req->tag2_upper = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TAG2_LOWER:
        error = cli_parse_ulong(cmd, &value, 1, 4094);
        if (!error) {
            req->tag2_lower = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_MAC_ADDR:
        error = cli_parse_mac(cmd, req->ptp_mac);
        if (!error) req->ptp_mac_spec = TRUE;
        break;
    case CLI_PARM_TYPE_1588_MAC_MATCH:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_FLOW_MATCH:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_PBB_EN:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_ADDR_MATCH:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_VLAN_CHK:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_TAG_RANGE:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_IP_VER:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_MPLS_STKREF:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_ACT_PTP:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_ACT_Y1731:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_ACT_IETF_OAM:
        error = cli_parm_1588_api_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_MPLS_CTRLW:
        error = cli_parm_1588_api_cw_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_CLK_MODE:
        error = cli_parse_ulong(cmd, &value, 0, 3);
        if (!error) {
            req->clk_mode = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_PTP_DELAYM:
    case CLI_PARM_TYPE_1588_Y1731_DELAYM:
    case CLI_PARM_TYPE_1588_IETF_OAM_DELAYM:
        error = cli_parse_ulong(cmd, &value, 0, 1);
        if (!error) {
            req->delaym = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_OAM_VER:
        error = cli_parse_ulong(cmd, &value, 0, 0);
        if (!error) {
            req->version = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_DOMAIN_LOWER:
    case CLI_PARM_TYPE_1588_MEG_RANGE_LOWER:
        error = cli_parse_ulong(cmd, &value, 0, 255);
        if (!error) {
            req->domain_meg_lower = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_DOMAIN_UPPER:
    case CLI_PARM_TYPE_1588_MEG_RANGE_UPPER:
        error = cli_parse_ulong(cmd, &value, 0, 255);
        if (!error) {
            req->domain_meg_upper = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TS_FORMAT:
        error = cli_parse_ulong(cmd, &value, 3, 3);
        if (!error) {
            req->ietf_tf = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_TRAFFIC_CLASS:
        error = cli_parse_ulong(cmd, &value, 0, 7);
        if (!error) {
            req->ietf_ds = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_SIG_MASK:
        error = cli_parse_ulong(cmd, &value, 0, 0x7f);
        req->sig_mask = (u8)value;
        break;
    case CLI_PARM_TYPE_1588_IPV4_KEYWORD:
        error = cli_parm_1588_ip_flow_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_IPV6_KEYWORD:
        error = cli_parm_1588_ip_flow_keyword(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_SPORT_VAL:
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (!error) {
            req->sport_val = (u16)value;
            req->sport_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_SPORT_MASK:
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (!error) {
            req->sport_mask = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_DPORT_VAL:
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (!error) {
            req->dport_val = (u16)value;
            req->dport_spec = TRUE;
        }
        break;
    case CLI_PARM_TYPE_1588_DPORT_MASK:
        error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
        if (!error) {
            req->dport_mask = (u16)value;
        }
        break;
    case CLI_PARM_TYPE_1588_IPV4_ADDR:
        error = cli_parm_1588_api_ipv4_addr_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_IPV4_MASK:
        error = cli_parm_1588_api_ipv4_mask_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_IPV6_ADDR:
        error = cli_parm_1588_api_ipv6_addr_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_IPV6_MASK:
        error = cli_parm_1588_api_ipv6_mask_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_STK_DEPTH:
        error = cli_parm_1588_api_stk_depth_parse(cmd,req,stx);
        break;
    case CLI_PARM_TYPE_1588_STKLVL0:
        error = cli_parm_1588_api_stk_lvl_0_value_parse(cmd, cmd2, stx, cmd_org, req);
        break;
    case CLI_PARM_TYPE_1588_STKLVL1:
        error = cli_parm_1588_api_stk_lvl_1_value_parse(cmd, cmd2, stx, cmd_org, req);
        break;
    case CLI_PARM_TYPE_1588_STKLVL2:
        error = cli_parm_1588_api_stk_lvl_2_value_parse(cmd, cmd2, stx, cmd_org, req);
        break;
    case CLI_PARM_TYPE_1588_STKLVL3:
        error = cli_parm_1588_api_stk_lvl_3_value_parse(cmd, cmd2, stx, cmd_org, req);
        break;
    case CLI_PARM_TYPE_1588_ACH_VER:
        error = cli_parse_ulong(cmd, &value, 0, 0xFF);
        if (!error) { 
            req->ach_ver_spec = TRUE;
            req->ach_ver = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_CHANNEL_TYPE:
        error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
        req->channel_type_spec = TRUE;
        req->channel_type = (u16)value;
        break;
    case CLI_PARM_TYPE_1588_PROTOCOL_ID:
        error = cli_parse_ulong(cmd, &value, 0, 0xFFFF);
         if (!error) {
            req->proto_id_spec = TRUE;
            req->proto_id = (u16)value;
         }
         break;
    case CLI_PARM_TYPE_1588_TIME_SEC:
        error = cli_parse_ulong(cmd, &value, 0, 60);
        if (!error) {
            req->time_sec = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_LATENCY_VAL:
        error = cli_parse_ulong(cmd, &value, 0, 65536);
        if (!error) {
            req->delay_val = (vtss_timeinterval_t)value;
            req->delay_val = req->delay_val << 16;
        }
        break;
    case CLI_PARM_TYPE_1588_DELAY_VAL:
        error = cli_parse_ulong(cmd, &value, 0,4294967295u);
        if (!error) {
            req->delay_val = (vtss_timeinterval_t)value;
            req->delay_val = req->delay_val << 16;
        }
        break;
    case CLI_PARM_TYPE_1588_ASYM_VAL:
        error = cli_parse_ulong(cmd, &value, -32768, 32767);
        if (!error) {
            req->asym_val = (vtss_timeinterval_t)value;
            req->asym_val = req->asym_val << 16;
        }
        break;
    case CLI_PARM_TYPE_1588_BLK_ID:
        error = cli_parse_ulong(cmd, &value, 0, 7);
        if (!error) {
            req->blk_id_1588 = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_1588_CSR_OFFSET:
        error = cli_parse_ulong(cmd, &value, 0, 0x7ff);
        if (!error) {
            req->csr_reg_offset_1588 = (u16)value;
        }
        break;
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_FEATURE_GFP)
    case CLI_PARAM_TYPE_GFP_CMF_FORCED_INTERVAL:
        error = cli_parse_ulong(cmd, &req->forced_interval, 0, 10);
        break;
    case CLI_PARAM_TYPE_GFP_CMF_FRAME_LENGTH:
        error = cli_parse_ulong(cmd, &req->cmf_fr_len, 0, 65535);
        break;
    case CLI_PARAM_TYPE_GFP_UPI:
        error = cli_parse_ulong(cmd, &req->upi, 0, 0xFF);
        break;
    case CLI_PARAM_TYPE_GFP_PTI:
        error = cli_parse_ulong(cmd, &req->pti, 0, 0x7);
        break;
    case CLI_PARAM_TYPE_GFP_ORDERED_UPI:
        error = cli_parse_ulong(cmd, &req->ordered_upi, 0, 0xFF);
        break;
    case CLI_PARAM_TYPE_GFP_8B10B_FORCED:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARAM_TYPE_GFP_8B10B_ERROR:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARAM_TYPE_GFP_CMF_AUTO_CONF:
        error = cli_parse_keyword(cmd,req,stx);
        break;
    case CLI_PARAM_TYPE_GFP_SINGLE_ERROR_EN_DIS:
        error = cli_parse_keyword(cmd,req,stx);
        break;
#endif /* VTSS_FEATURE_GFP */
#if defined(VTSS_FEATURE_UPI)
    case CLI_PARM_TYPE_GEN_MODE:
        error = cli_parse_ulong(cmd, &req->upi_gen_mode, 0, 9);
        break;
    case CLI_PARM_TYPE_CHK_MODE:
        error = cli_parse_ulong(cmd, &req->upi_chk_mode, 0, 9);
        break;
    case CLI_PARM_TYPE_WRP_MODE:
        error = cli_parse_ulong(cmd, &req->upi_wrp_mode, 0, VTSS_UPI_TEST_WRAP_MAX-1);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_MODE:
        error = cli_parse_ulong(cmd, &req->upi_txeq_mode, 0, 1);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_CHNLOSS:
        error = cli_parse_ulong(cmd, &req->upi_txeq_chnloss, 0, 2);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_COEF_ID:
        error = cli_parse_ulong(cmd, &req->upi_coef_id, 0, 3);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_COEF_STEP:
        error = cli_parse_ulong(cmd, &req->upi_coef_step, 0, 255);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_COEF_INCR:
        error = cli_parse_ulong(cmd, &req->upi_incr, 0, 1);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_COEF_POLARITY:
        error = cli_parse_ulong(cmd, &req->upi_polarity, 0, 1);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_POWER:
        error = cli_parse_ulong(cmd, &req->upi_power, 20, 127);
        break;
    case CLI_PARM_TYPE_UPI_TXEQ_SLEW:
        error = cli_parse_ulong(cmd, &req->upi_slew, 0, 1);
        break;
    case CLI_PARM_TYPE_UPI_RXEQ_MODE:
        error = cli_parse_ulong(cmd, &req->upi_rxeq_mode, 0, 2);
#endif
#if defined(VTSS_FEATURE_XFI)
    case CLI_PARM_TYPE_XFI_TXEQ_MODE:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_mode, 0, 3);
        break;
    case CLI_PARM_TYPE_TXEQ_CHNLOSS:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_chnloss, 0, 3);
        break;
    case CLI_PARM_TYPE_TXEQ_PORTNO:
        error = cli_parse_ulong(cmd, &value, 1, VTSS_PORTS);
        if (!error) {
            req->port_no = (u8)value;
        }
        break;
    case CLI_PARM_TYPE_XFI_TXEQ_COEF:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_coef, 0, 2);
        break;
    case CLI_PARM_TYPE_XFI_TXEQ_STEPSIZE:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_step, 0, 127);
        break;
    case CLI_PARM_TYPE_XFI_TXEQ_INCR:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_incr, 0, 1);
        break;
    case CLI_PARM_TYPE_XFI_TXEQ_POLARITY:
        error = cli_parse_ulong(cmd, &req->xfi_txeq_polarity, 0, 1);
        break;
    case CLI_PARM_TYPE_XFI_TXEQ_PARM_VALUE:
        error = cli_parse_ulong(cmd, &req->xfi_misc_parm_value, 0, 200000);
        break;
    case CLI_PARM_TYPE_XFI_RXEQ_MODE:
        error = cli_parse_ulong(cmd, &req->xfi_rxeq_mode, 1, 4);
        break;
    case CLI_PARM_TYPE_XFI_802A_TXEQ_UPDREQ:
        error = cli_parse_ulong(cmd, &req->xfi_802a_txeq_upd, 0, 2);
        break;
    case CLI_PARM_TYPE_XFI_802A_TXEQ_LANE:
        error = cli_parse_ulong(cmd, &req->xfi_802a_txeq_lane, 0, 1);   
        break;
    case CLI_PARM_TYPE_XFI_802A_TXEQ_COEF:
        error = cli_parse_ulong(cmd, &req->xfi_802a_txeq_coef, 0, 3);
        break;
    case CLI_PARM_TYPE_XFI_802A_FSM_CTL:
        error = cli_parse_ulong(cmd, &req->xfi_802a_txeq_fsm, 0, 0xffff);
        break;
#endif
#if defined(VTSS_FEATURE_XAUI)
#if 0
    case CLI_PARM_TYPE_Q_CODE:
        error = cli_parse_ulong(cmd, &req->xaui_q_code, 0, 0xffffffff);
        if (error) {
            req->xaui_set_q_code = FALSE;
        } else {
            req->xaui_set_q_code = TRUE;
        }
        break;
#endif
#endif
#ifdef VTSS_ARCH_DAYTONA
#if defined (VTSS_FEATURE_MAC10G)
    case CLI_PARM_TYPE_MAC10G_MAX_FRAME_LEN:
        error =  cli_parse_ulong(cmd, &req->mac10g_max_frame_len, 0, 10240);
        break;
    case CLI_PARM_TYPE_MAC10G_MAX_TAGS:
        error =  cli_parse_ulong(cmd, &req->mac10g_max_tags, 0, 2);
        break;
#endif /* VTSS_FEATURE_MAC10G */
#if defined (VTSS_FEATURE_RAB)
    case CLI_PARM_TYPE_RAB_THRESH_HIGH:
        error =  cli_parse_ulong(cmd, &req->fifo_thresh_high, 0, 0x1fff);
        break;
    case CLI_PARM_TYPE_RAB_THRESH_LOW:
        error =  cli_parse_ulong(cmd, &req->fifo_thresh_low, 0, 0x1fff);
        break;
    case CLI_PARM_TYPE_RAB_THRESH_ADAPT:
        error =  cli_parse_ulong(cmd, &req->fifo_thresh_adapt, 0, 0x1fff);
        break;
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_I2C)
    case CLI_PARM_TYPE_I2C_CONTROLLER_NO:
        error =  cli_parse_ulong(cmd, &req->i2c_controller_no, 1, 2);
        break;
    case CLI_PARM_TYPE_I2C_DEV_ADDR:
        error =  cli_parse_ulong(cmd, &req->i2c_dev_addr, 0, 0xffff);
        break;
    case CLI_PARM_TYPE_I2C_REG_ADDR:
        error =  cli_parse_ulong(cmd, &req->i2c_reg_addr, 0, 0xffff);
        break;
    case CLI_PARM_TYPE_I2C_DATA:
        error =  cli_parse_ulong(cmd, &req->i2c_data, 0, 0xffff);
        break;
#endif /* VTSS_FEATURE_I2C */
    case CLI_PARM_TYPE_PMTICK_INTERVAL:
        error =  cli_parse_ulong(cmd, &req->pmtick_interval, 0, 0xfffff);
        break;
    case CLI_PARM_TYPE_CHANNEL_NUM:
        error =  cli_parse_ulong(cmd, &req->channel_num, 0, 1);
        break;
    case CLI_PARM_TYPE_CHANNEL_MODE:
        error =  cli_parse_ulong(cmd, &req->channel_mode, 0, 100);
        break;
    case CLI_PARM_TYPE_CHANNEL_XAUIMODE:
        error =  cli_parse_ulong(cmd, &req->channel_xauimode, 0, 3);
        break;
    case CLI_PARM_TYPE_CHANNEL_CONFIG_EVINIT:
        error =  cli_parse_ulong(cmd, &req->ev_init, 0, 1);
        break;
#endif /* VTSS_ARCH_DAYTONA */
#if defined(VTSS_FEATURE_OHA)
    case CLI_PARM_TYPE_OHA_ROSITOSI:
        error =  cli_parse_ulong(cmd, &req->oha_rosi_tosi, 1, 2);
        break;
    case CLI_PARM_TYPE_OHA_OTN1_OTN2:
        error =  cli_parse_ulong(cmd, &req->oha_otn, 1, 2);
        break;
#endif
#if defined(DAYTONA_EVAL_INIT)
    case CLI_PARM_TYPE_LBFPGA_REG:
        error = cli_parse_ulong(cmd, &req->lbfpga_reg, 0, 0xffffff);
        break;
    case CLI_PARM_TYPE_PCF8575_TGTID:
        error = cli_parse_ulong(cmd, &req->pcf8575_tgtid, 0, 2);
        break;
    case CLI_PARM_TYPE_VSC3172_REG:
        error = cli_parse_ulong(cmd, &req->vsc3172_reg, 0, 0x28e);
        break;
    case CLI_PARM_TYPE_SI5338_REG:
        error = cli_parse_ulong(cmd, &req->si5338_reg, 0, 0xf6);
        break;
    case CLI_PARM_TYPE_SI5338_TGTID:
        error = cli_parse_ulong(cmd, &req->si5338_tgtid, 0, 7);
        break;
    case CLI_PARM_TYPE_SI5326_REG:
        error = cli_parse_ulong(cmd, &req->si5326_reg, 0, 0x88);
        break;
    case CLI_PARM_TYPE_SI5326_TGTID:
        error = cli_parse_ulong(cmd, &req->si5326_tgtid, 0, 16);
        break;
    case CLI_PARM_TYPE_FPGA_MASK_VALUE:
        error = cli_parse_ulong(cmd, &req->fpga_mask, 0, 0xFFFFFFFF);
        break;
    case CLI_PARM_TYPE_VAL_MASK_VALUE:
        error = cli_parse_ulong(cmd, &req->val_mask, 0, 0xFFFF);
        break;
#endif /* DAYTONA_EVAL_INIT */
    default:
        printf("Unknown parameter type: %d\n", type); 
        error = 1;
        break;
    }
    return error;
}

static cli_parm_t *cli_parm_lookup(char *stx, cli_cmd_id_t cmd_id, int *idx)
{
    int        i;
    cli_parm_t *parm;
    char stx_buf[MAX_WORD_LEN], *stx_start, *stx_end;

    strcpy(stx_buf, stx);
    stx_start = stx_buf;
    /* Remove the brackets/paranthesis from the beginning and end of syntax */
    while ((*stx_start == '(') || (*stx_start == '['))
        stx_start++;
    if ((stx_end = strchr(stx_start, ')')) != NULL)
        *stx_end = '\0';
    if ((stx_end = strchr(stx_start, ']')) != NULL)
        *stx_end = '\0';

    for (i = 0; i < sizeof(cli_parm_table)/sizeof(*parm); i++) {
        parm = &cli_parm_table[i];
        if ((strcmp(stx_start, parm->txt) == 0) && 
            (parm->id == CLI_CMD_ID_NONE || parm->id == cmd_id)) {
            *idx = i;
            return parm;
        }
    }

    return NULL;
}

#define CLI_TRACE_GROUP_ALL (VTSS_TRACE_GROUP_COUNT + 1)

static void cli_req_default_set(cli_req_t *req)
{
    memset(req, 0, sizeof(*req));
    cli_parse_list(NULL, req->port_list, 0, VTSS_PORTS, 1);
    cli_remove_unused_ports(req);
    req->vid = VTSS_VID_DEFAULT;
    req->trace_layer = VTSS_TRACE_LAYER_COUNT;
    req->group = CLI_TRACE_GROUP_ALL;
#if defined(VTSS_GPIOS)
    cli_parse_list(NULL, req->gpio_list, 0, VTSS_GPIO_NO_END - 1, 1);
#endif /* VTSS_GPIOS */
#if defined(VTSS_ARCH_B2)
    cli_parse_list(NULL, req->dscp_list, 0, 63, 1);
    cli_parse_list(NULL, req->lport_list, 0, VTSS_LPORTS-1, 1);
    req->class = 0;
#endif
    req->api_inst = cli_board->inst;
    req->appl_inst = cli_appl_inst;
    req->board = cli_board;
}

/* Header with optional new line before and after */
static void cli_header_nl_char(const char *txt, BOOL pre, BOOL post, char c)
{
    int i, len;
    
    if (pre)
        printf("\n");
    printf("%s:\n", txt);
    len = (strlen(txt) + 1);
    for (i = 0; i < len; i++)
        printf("%c", c);
    printf("\n");
    if (post)
        printf("\n");
}

/* Underlined header with optional new line before and after */
static void cli_header_nl(const char *txt, BOOL pre, BOOL post)
{
    cli_header_nl_char(txt, pre, post, '-');
}

static void cli_table_header(const char *txt)
{
    int i, j, len, count = 0;

    len = strlen(txt);
    for (i = 0; i < len; i++)
        printf("%c", txt[i] == '*' ? ' ' : txt[i]);
    printf("\n");
    
    while (*txt == ' ') {
        printf(" ");
        txt++;
    }
    for (i = 0; i < len; i++) {
        if (txt[i] == ' ') {
            count++;
        } else {
            for (j = 0; j < count; j++)
                printf("%c", count > 1 && (j >= (count - 2)) ? ' ' : '-');
            printf("-");
            count = 0;
        }
    }
    for (j = 0; j < count; j++)
        printf("%c", count > 1 && (j >= (count - 2)) ? ' ' : '-');
    printf("\n");
}

static void cli_cmd_help(void) 
{
    printf("Type '<group> ?' to get list of group commands, e.g. 'system ?'.\n");
    printf("Type '<command> ?' to get help on a command, e.g. 'system reboot ?'.\n");
    printf("Commands may be abbreviated, e.g. 'sy re' instead of 'system reboot'.\n");
}

static const char *cli_bool_txt(BOOL enabled)
{
    return (enabled ? "Enabled " : "Disabled");
}

static const char *port_mode_txt(vtss_port_speed_t speed, BOOL fdx)
{
    switch (speed) {
    case VTSS_SPEED_10M:
        return (fdx ? "10fdx" : "10hdx");
    case VTSS_SPEED_100M:
        return (fdx ? "100fdx" : "100hdx");
    case VTSS_SPEED_1G:
        return (fdx ? "1Gfdx" : "1Ghdx");
    case VTSS_SPEED_2500M:
        return (fdx ? "2.5Gfdx" : "2.5Ghdx");
    case VTSS_SPEED_5G:
        return (fdx ? "5Gfdx" : "5Ghdx");
    case VTSS_SPEED_10G:
        return (fdx ? "10Gfdx" : "10Ghdx");
    default:
        return "?";
    }
}

#if defined(VTSS_ARCH_B2)
static const char *misc_mac_txt(const uchar *mac, char *buf)
{
    sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_FEATURE_WARM_START)
static const char *cli_restart_txt(vtss_restart_t restart)
{
    return (restart == VTSS_RESTART_COLD ? "Cold" :
            restart == VTSS_RESTART_COOL ? "Cool" :
            restart == VTSS_RESTART_WARM ? "Warm" : "?");
}

static void cli_cmd_system_restart(cli_req_t *req)
{
    if (req->set) {
        vtss_restart_conf_set(req->api_inst, 
                              req->warm ? VTSS_RESTART_WARM : VTSS_RESTART_COOL);
    } else {
        vtss_restart_status_t status;
        vtss_restart_t        restart;
        
        if (vtss_restart_conf_get(req->api_inst, &restart) == VTSS_RC_OK)
            printf("Next Restart    : %s\n", cli_restart_txt(restart));
        if (vtss_restart_status_get(req->api_inst, &status) == VTSS_RC_OK) {
            printf("Previous Restart: %s\n", cli_restart_txt(status.restart));
            printf("Current Version : %u\n", status.cur_version);
            printf("Previous Version: %u\n", status.prev_version);
        }
    } 
}
#endif /* VTSS_FEATURE_WARM_START */

#ifndef VTSS_ARCH_DAYTONA
/* Port configuration */
static void cli_cmd_port_conf(cli_req_t *req, BOOL state, BOOL mode, 
                              BOOL flow_control, BOOL max_length) 
{
    vtss_port_no_t          port, port_no;
    vtss_appl_port_conf_t   conf;
    vtss_appl_port_status_t status;
    vtss_port_interface_t   if_type;
    BOOL                    first = 1;
    char                    buf[80], *p;
    BOOL                    flow_control_enabled = 0;
    BOOL                    rx=0,tx=0;
#if defined(VTSS_ARCH_B2)    
    vtss_host_conf_t        host;
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_CHIP_10G_PHY)                
    vtss_phy_10g_id_t       phy_id;
    BOOL                    found_phy=0;
#endif /* VTSS_CHIP_10G_PHY */
    
#if !defined(VTSS_FEATURE_PORT_CONTROL)
    max_length = 0;
#endif /* VTSS_FEATURE_PORT_CONTROL */

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || 
            vtss_appl_port_conf_get(req->appl_inst, port_no, &conf) != VTSS_RC_OK ||
            vtss_appl_port_status_get(req->appl_inst, port_no, &status) != VTSS_RC_OK ||
            vtss_appl_port_if_get(req->appl_inst, port_no, &if_type) != VTSS_RC_OK)
            continue;
#if defined(VTSS_CHIP_10G_PHY)                
        if (if_type == VTSS_PORT_INTERFACE_XAUI) {
            if (vtss_phy_10G_is_valid(NULL, port_no)) { 
                if (vtss_phy_10g_id_get(req->api_inst, port_no, &phy_id) != VTSS_RC_OK) {
                    continue;
                }
                found_phy = 1;
            }
        }
#endif /* VTSS_CHIP_10G_PHY */

        if (req->set) {
            if (if_type == VTSS_PORT_INTERFACE_SPI4) {
              // Skip - SPI can not be configured by this command
            } else {
              if (mode) {
                conf.autoneg = req->auto_keyword;
                if (!conf.autoneg) {
                  conf.speed = req->speed;
                  conf.fdx = req->fdx;
                }
              } 
              if (state)
                conf.enable = req->enable;
              if (flow_control)
                conf.flow_control = req->enable;
              if (max_length)
                conf.max_length = req->value;
              if (vtss_appl_port_conf_set(req->appl_inst, port_no, &conf) != VTSS_RC_OK) 
                printf("Could not configure port %u\n", iport2uport(port_no));                                                                                            
            }
        } else {
            if (first) {
                p = &buf[0];
                p += sprintf(p, "Port  ");
                if (state)
                    p += sprintf(p, "State     ");
                if (mode)
                    p += sprintf(p, "Mode    ");
                if (flow_control) {
                    p += sprintf(p, "Flow Control  ");
                    if (!state)
                        p += sprintf(p, "Rx Pause  Tx Pause  ");
                }
                if (max_length)
                    p += sprintf(p, "MaxFrame  ");
                if (mode) {
                    p += sprintf(p, "Link      ");
                    p += sprintf(p, "Interface ");
                }
                
                cli_table_header(buf);
                first = 0;
            }
            printf("%-6u", port);
            
            if (if_type == VTSS_PORT_INTERFACE_SPI4) {
                printf("%-10s %-8s %-13s %-8s %-8s %s","SPI4","-","-","-","-",
                       if2str(req->appl_inst, port_no));
            } else {                
                if (state)
                    printf("%-10s", cli_bool_txt(conf.enable));
                if (mode)
                    printf("%-8s", 
                           conf.autoneg ? "Auto" : port_mode_txt(conf.speed, conf.fdx));
                if (flow_control) {


                  if (vtss_appl_iftype_is_xaui(req->appl_inst, port_no)) {
#if defined(VTSS_ARCH_B2)    
                    // Determine if XAUI ports have flow control enabled 
                    vtss_appl_host_conf_get(req->appl_inst, port_no, &host);
                    rx = host.xaui.fc.ib.enable | host.xaui.fc.obey_pause;
                    tx = host.xaui.fc.ib.enable | host.xaui.fc.channel_enable;
                    flow_control_enabled = tx | rx;
#endif /* VTSS_ARCH_B2 */
                    
                  } else {
                    // Determine if 1G line ports have flow control enabled 
                    rx = (conf.autoneg ? (status.link ? status.aneg.obey_pause : 0) :
                          conf.flow_control);
                    tx = (conf.autoneg ? (status.link ? status.aneg.generate_pause : 0) :
                          conf.flow_control);
                    
                    flow_control_enabled = conf.flow_control;
                  }                   

                  printf("%-14s", cli_bool_txt(flow_control_enabled));                    
                
                  if (!state)
                    printf("%s  %s  ", cli_bool_txt(rx), cli_bool_txt(tx));
                }
                if (max_length)
                    printf("%-10u", conf.max_length);
                if (mode) {
                    printf("%-10s", status.link ? port_mode_txt(status.speed, status.fdx) : "Down");
                    printf("%s",if2str(req->appl_inst, port_no));
#if defined(VTSS_CHIP_10G_PHY)    
                    if (found_phy) {
                        printf(" <-> %-10x",phy_id.part_number);
                        found_phy = 0;
                    }
#endif /* VTSS_CHIP_10G_PHY */
                }
            }
            printf("\n");
        }
    }
}
#endif /* ifndef VTSS_ARCH_DAYTONA */

#if defined(VTSS_ARCH_B2)
/* Setup the logical port shapers   */
static void cli_cmd_qos_lport_shaper(cli_req_t *req)
{
    vtss_qos_lport_conf_t  lqos;
    vtss_qos_port_conf_t   qos;
    vtss_init_conf_t       conf;
    int                    port_no,lport_no;
    BOOL                   first = 1;
    
    if (vtss_appl_init_conf_get(req->appl_inst, &conf) != VTSS_RC_OK)
        return;

    if (conf.host_mode < 8) {
        /* Host modes 0-6  */
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            if (req->lport_list[port_no] == 0)
                continue;
            
            if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not get QoS conf\n");
                return;
            }
            
            if (req->set) {
                if (req->rate == 0) {
                    qos.shaper_host.level = 0;
                    qos.shaper_host.rate = VTSS_BITRATE_DISABLED;
                } else {
                    qos.shaper_host.level = req->policer.level;
                    qos.shaper_host.rate = req->rate;
                }
                
                if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                    printf("Could not set QoS conf\n");
                    return;
                }
            } else {
                if (first) {
                    printf("%-10s %-10s %-10s\n","Port","Rate/Kb","Level/KB");
                    first = 0;
                }
                if (qos.shaper_host.rate == VTSS_BITRATE_DISABLED) {
                    printf("%-10d %-10s %-10s\n",port_no,"-","-");
                } else {
                    printf("%-10d %-10u %-10u\n", port_no,
                           qos.shaper_host.rate,
                           qos.shaper_host.level/1024);            
                }
            }
        }
    } else {
        /* Host modes 8-11   */
        for (lport_no = 0; lport_no < VTSS_LPORTS; lport_no++) {
            if (req->lport_list[lport_no] == 0)
                continue;

            if (vtss_qos_lport_conf_get(req->api_inst, lport_no, &lqos) != VTSS_RC_OK) {
                printf("Could not get QoS conf\n");
                return;
            }
                      
            if (req->set) {
                if (req->rate == 0) {
                    lqos.shaper.level = 0;
                    lqos.shaper.rate = VTSS_BITRATE_DISABLED;
                } else {
                    lqos.shaper.level = req->policer.level;
                    lqos.shaper.rate = req->rate;
                }
                
                if (vtss_qos_lport_conf_set(req->api_inst, lport_no, &lqos) != VTSS_RC_OK) {
                    printf("Could not set QoS conf\n");
                    return;
                }
            } else {
                if (first) {
                    printf("%-10s %-10s %-10s\n","Port","Rate/Kb","Level/KB");
                    first = 0;
                }
                if (lqos.shaper.rate == VTSS_BITRATE_DISABLED) {
                    printf("%-10d %-10s %-10s\n",lport_no,"-","-");
                } else {
                    printf("%-10d %-10u %-10u\n",lport_no,
                           lqos.shaper.rate,
                           lqos.shaper.level/1024);            
                }
            }
        }        
    }
}
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_FEATURE_PORT_CONTROL)
/* Print counters in two columns with header */
static void cli_cmd_stat_port(vtss_port_no_t port, BOOL *first, const char *name, 
                              u64 c1, u64 c2)
{
    char buf[80], *p;

    if (*first) {
        *first = 0;
        p = &buf[0];
        p += sprintf(p, "Port  Rx %-17sTx %-17s", name, name);
        cli_table_header(buf);
    }
    printf("%-2u    %-20" PRIu64 "%-20" PRIu64 "\n", port, c1, c2);
}

/* Print two counters in columns */
static void cli_cmd_stats(const char *col1, const char *col2, u64 c1, u64 c2)
{
    char buf[80];
    
    sprintf(buf, "Rx %s:", col1);
    printf("%-19s%19" PRIu64 "   ", buf, c1);
    if (col2 != NULL) {
        sprintf(buf, "Tx %s:", strlen(col2) ? col2 : col1);
        printf("%-19s%19" PRIu64, buf, c2);
    }
    printf("\n");
}

#if defined(VTSS_ARCH_SPARX)
#define RMON_FRAME_MAX 1526
#else
#define RMON_FRAME_MAX 1518
#endif /* VTSS_ARCH_SPARX */

#ifndef VTSS_ARCH_DAYTONA
/* Port statistics */
static void cli_cmd_port_stats(cli_req_t *req)
{
    vtss_port_no_t                     port, port_no, i;
    vtss_port_counters_t               counters;
    vtss_port_rmon_counters_t          *rmon = &counters.rmon;
    vtss_port_if_group_counters_t      *if_group = &counters.if_group;
    vtss_port_ethernet_like_counters_t *elike = &counters.ethernet_like;
    vtss_port_proprietary_counters_t   *prop = &counters.prop;
    BOOL                               first = 1;
    char                               buf[32];

    buf[0] = 0;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        // Do only change setting for ports in the port list and skip SPI4 port
        if (req->port_list[port] == 0 || vtss_appl_iftype_is_spi4(req->appl_inst, port_no)) {
          continue;
        }
        /* Handle 'clear' command */
        if (req->clear) {
            vtss_port_counters_clear(req->api_inst, port_no);
            continue;
        }
            
        /* Get counters */
        if (vtss_port_counters_get(req->api_inst, port_no, &counters) != VTSS_RC_OK)
            continue;
        
        /* Handle 'packet' command */
        if (req->packets) {
            cli_cmd_stat_port(port, &first, "Packets", 
                              rmon->rx_etherStatsPkts, rmon->tx_etherStatsPkts);
            continue;
        } 
        
        /* Handle 'bytes' command */
        if (req->bytes) {
            cli_cmd_stat_port(port, &first, "Octets", 
                              rmon->rx_etherStatsOctets, rmon->tx_etherStatsOctets);
            continue;
        } 
        
        /* Handle 'errors' command */
        if (req->errors) {
            cli_cmd_stat_port(port, &first, "Errors", 
                              if_group->ifInErrors, if_group->ifOutErrors);
            continue;
        } 
        
        /* Handle 'discards' command */
        if (req->discards) {
            cli_cmd_stat_port(port, &first, "Discards", 
                              if_group->ifInDiscards, if_group->ifOutDiscards);
            continue;
        }
        
        /* Handle default command */
        printf("%sPort %u Statistics:\n\n", first ? "" : "\n", port);
        first = 0;
        cli_cmd_stats("Packets", "", rmon->rx_etherStatsPkts, rmon->tx_etherStatsPkts);
        cli_cmd_stats("Octets", "", rmon->rx_etherStatsOctets, rmon->tx_etherStatsOctets);
        cli_cmd_stats("Unicast", "", if_group->ifInUcastPkts, if_group->ifOutUcastPkts);
        cli_cmd_stats("Multicast", "", rmon->rx_etherStatsMulticastPkts, 
                      rmon->tx_etherStatsMulticastPkts);
        cli_cmd_stats("Broadcast", "", rmon->rx_etherStatsBroadcastPkts, 
                      rmon->tx_etherStatsBroadcastPkts);
        cli_cmd_stats("Pause", "", elike->dot3InPauseFrames, elike->dot3OutPauseFrames);
        printf("\n");
        cli_cmd_stats("64", "", rmon->rx_etherStatsPkts64Octets, 
                      rmon->tx_etherStatsPkts64Octets);
        cli_cmd_stats("65-127", "", rmon->rx_etherStatsPkts65to127Octets, 
                      rmon->tx_etherStatsPkts65to127Octets);
        cli_cmd_stats("128-255", "", rmon->rx_etherStatsPkts128to255Octets,
                      rmon->tx_etherStatsPkts128to255Octets);
        cli_cmd_stats("256-511", "", rmon->rx_etherStatsPkts256to511Octets,
                      rmon->tx_etherStatsPkts256to511Octets);
        cli_cmd_stats("512-1023", "", rmon->rx_etherStatsPkts512to1023Octets,
                      rmon->tx_etherStatsPkts512to1023Octets);
        sprintf(buf, "1024-%u", RMON_FRAME_MAX);
        cli_cmd_stats(buf, "", rmon->rx_etherStatsPkts1024to1518Octets,
                      rmon->tx_etherStatsPkts1024to1518Octets);
        sprintf(buf, "%u-    ", RMON_FRAME_MAX + 1);
        cli_cmd_stats(buf, "", rmon->rx_etherStatsPkts1519toMaxOctets,
                      rmon->tx_etherStatsPkts1519toMaxOctets);
        printf("\n");

        // The priority queues does not apply to the host interface
        if (!vtss_appl_port_is_host(req->appl_inst, port_no)) {
          for (i = 0; i < VTSS_PRIOS; i++) {              
            sprintf(buf, "Packets_%u", i);
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || \
    defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_JAGUAR_1)
            cli_cmd_stats(buf, "", prop->rx_prio[i], prop->tx_prio[i]);
#else
            cli_cmd_stats(buf, NULL, prop->rx_prio[i], 0);
#endif /* VTSS_ARCH_LUTON28 */                        
          }
        }
        printf("\n");
        
        cli_cmd_stats("Drops", "", rmon->rx_etherStatsDropEvents,
                      rmon->tx_etherStatsDropEvents);
#if defined(VTSS_FEATURE_PORT_CNT_ETHER_LIKE)
        cli_cmd_stats("CRC/Alignment", "Late Collisions", rmon->rx_etherStatsCRCAlignErrors,
                      elike->dot3StatsLateCollisions);
        cli_cmd_stats("Symbol", "Excessive Coll.", elike->dot3StatsSymbolErrors,
                      elike->dot3StatsExcessiveCollisions);
        cli_cmd_stats("Undersize", "Carrier Sense", rmon->rx_etherStatsUndersizePkts,
                      elike->dot3StatsCarrierSenseErrors);
#else
        cli_cmd_stats("CRC/Alignment", "Late/Exc. Coll.", rmon->rx_etherStatsCRCAlignErrors,
                      if_group->ifOutErrors);
        cli_cmd_stats("Undersize", NULL, rmon->rx_etherStatsUndersizePkts, 0);
        
#endif /* VTSS_FEATURE_PORT_CNT_ETHER_LIKE */
        cli_cmd_stats("Oversize", NULL, rmon->rx_etherStatsOversizePkts, 0);
        cli_cmd_stats("Fragments", NULL, rmon->rx_etherStatsFragments, 0);
        cli_cmd_stats("Jabbers", NULL, rmon->rx_etherStatsJabbers, 0);
#if defined(VTSS_FEATURE_PORT_CNT_BRIDGE)
        cli_cmd_stats("Filtered", NULL, counters.bridge.dot1dTpPortInDiscards, 0);
#endif /* VTSS_FEATURE_PORT_CNT_BRIDGE */
#if defined(VTSS_ARCH_B2)
        printf("\n");
        cli_cmd_stats("Filtered", NULL, prop->rx_filter_drops, 0);
        cli_cmd_stats("Policed", NULL, prop->rx_policer_drops, 0);
        {
            int  i;
            // The priority queues donesn't aplly to the host interface
            if (!vtss_appl_port_is_host(req->appl_inst, port_no)) {

              for (i = 0; i < VTSS_PRIOS; i++) {
                sprintf(buf, "Drops_%u", i);
                cli_cmd_stats(buf, NULL, prop->rx_prio_drops[i], 0);
              }   
            }
        }
#endif /* VTSS_ARCH_B2 */
    } /* Port loop */
}
#endif /* ifndef VTSS_ARCH_DAYTONA */

/* Show port map table */
static void cli_cmd_port_map(cli_req_t *req)
{
    BOOL            first = 1;
    vtss_port_no_t  port, port_no;
    vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE], *map;

    if (vtss_port_map_get(req->api_inst, port_map) != VTSS_RC_OK)
        return;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (first) {
            first = 0;
            cli_table_header("Port  Chip Port  MIIM Bus  MIIM Addr");
        } 
        map = &port_map[port_no];
        printf("%-6u", port);
        if (map->chip_port < 0)
            printf("%-11s", "-");
        else
            printf("%-11d", map->chip_port);
        if (map->miim_controller == VTSS_MIIM_CONTROLLER_NONE)
            printf("%-10s%s", "-", "-");
        else
            printf("%-10d%-2u", map->miim_controller, map->miim_addr);
        printf("\n");
    }
}

#if defined(VTSS_ARCH_B2)
/* Setup the global DSCP tables, 2 tables, 64 entries in each */
static void cli_cmd_qos_global_dscp(cli_req_t *req)
{
    u32                  dscp, first = 1;
    vtss_dscp_entry_t    dscp_table[VTSS_DSCP_TABLE_SIZE];
    vtss_dscp_entry_t    *entry;
      
    for (dscp = 0; dscp < VTSS_DSCP_TABLE_SIZE; dscp++) {
        if (req->dscp_list[dscp] == 0)
            continue;

        if (vtss_dscp_table_get(req->api_inst, req->table_id-1, dscp_table) != VTSS_RC_OK) {
            printf("Could not get DSCP table\n");
            return;
        }
        
        if (req->set) {
            entry = &dscp_table[0];
            entry[dscp].enable = req->enable;
            entry[dscp].qos.prio = req->class;
            entry[dscp].qos.red = req->red;
            if (vtss_dscp_table_set(req->api_inst, req->table_id-1, entry) != VTSS_RC_OK)
                printf("Could not compute\n");
        } else {
            if (first)
                cli_table_header("DSCP  Trusted   Class  Red"); 
            first = 0;
            entry = dscp_table;
            printf("%-4u  %s  %-5u  %u\n", 
                   dscp, cli_bool_txt(entry[dscp].enable), entry[dscp].qos.prio, entry[dscp].qos.red);
        }
    }
}

/* The DSCP table (1 or 2) used by each port         */
static void cli_cmd_qos_dscp_table(cli_req_t *req)
{
    int              first=1;
    vtss_port_no_t       port, port_no;
    vtss_qos_port_conf_t qos;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {        
            qos.dscp_table_no = req->table_id-1;
            
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {            
            if (first)
                cli_table_header("Port  Table ID");
            first = 0;
            printf("%-4u  %u\n", port, qos.dscp_table_no+1);
        }
    }            
}

/* Setup the global tcp/udp ports, 8 entries, range/specific */
static void cli_cmd_qos_global_tcpudp(cli_req_t *req)
{
    int              first=1, table_index;
    vtss_qos_conf_t qos;

    if (vtss_qos_conf_get(req->api_inst, &qos) != VTSS_RC_OK) {
        printf("Could not get QoS conf\n");
        return;
    }

    if (req->set) {        
        req->table_index = req->table_index - 1; /* 1-8 changes to 0-7  */
        qos.udp_tcp[req->table_index/2].pair[req->table_index%2].port = req->tcp_udp_port;
        if (req->range) {
            qos.udp_tcp[req->table_index/2].pair[0].qos.prio = req->class;
            qos.udp_tcp[req->table_index/2].pair[1].qos.prio = req->class;
            qos.udp_tcp[req->table_index/2].pair[0].qos.red = req->red;
            qos.udp_tcp[req->table_index/2].pair[1].qos.red = req->red;
        } else {
            qos.udp_tcp[req->table_index/2].pair[req->table_index%2].qos.prio = req->class;
            qos.udp_tcp[req->table_index/2].pair[req->table_index%2].qos.red = req->red;        
        }
        qos.udp_tcp[req->table_index/2].range = req->range;         

        if (vtss_qos_conf_set(req->api_inst, &qos) != VTSS_RC_OK) {
            printf("Could not set QoS conf\n");
            return;
        }        
    } else {
        if (first) {        
            printf("Index     Port     Range|Specific     Class         Red                 \n");            
            first = 0;
        }
        
        for (table_index=0; table_index<8; table_index++) {
            printf("%d %#12x %12s %12u %12u\n", table_index + 1,
                   qos.udp_tcp[table_index/2].pair[table_index%2].port,
                   qos.udp_tcp[table_index/2].range?"Range":"Specific",
                   qos.udp_tcp[table_index/2].pair[table_index%2].qos.prio,
                   qos.udp_tcp[table_index/2].pair[table_index%2].qos.red);
        }        
    }
}

/* Include / exclude the global tcp/udp ports per port  */
static void cli_cmd_qos_tcpudp_enable(cli_req_t *req)
{
    int              first=1, pair_no, i, ti;
    vtss_port_no_t       port, port_no;
    vtss_qos_port_conf_t qos;
    vtss_qos_conf_t glb_qos;
    char             buf[200];

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK || vtss_qos_conf_get(req->api_inst, &glb_qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {        
            ti = req->table_index - 1; /* 1-8 changes to 0-7  */
            if (glb_qos.udp_tcp[ti/2].range) {
                /* Enable both ports if they are in a range */
                qos.udp_tcp.global_enable[ti/2][0] = req->enable;
                qos.udp_tcp.global_enable[ti/2][1] = req->enable;
            } else {                
                qos.udp_tcp.global_enable[ti/2][ti%2] = req->enable;
            }
                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
            
        } else {
            if (first) {        
                memset(&buf, 0, sizeof(buf));
                printf("              Port enable/disable status for global TCP/UDP port settings\n");
                sprintf(buf,"     ");
                for (i=1; i<9; i++) {
                    sprintf(buf, "%s %8d",buf,i);
                }
                first = 0;
                printf("%s\n",buf);            
            }
            memset(&buf, 0, sizeof(buf));
            sprintf(buf, "%-8u",port);
            
            for (pair_no = 0; pair_no < VTSS_UDP_TCP_PAIR_COUNT; pair_no++) {        
                sprintf(buf, "%s %8s %8s",buf,qos.udp_tcp.global_enable[pair_no][0]?"Enabled":"Disabled",
                        qos.udp_tcp.global_enable[pair_no][1]?"Enabled":"Disabled");
            }                
            printf("%s\n",buf);            
        }
    }        
}

/* Include / exclude the global filters (8) per port   */
static void cli_cmd_qos_global_filter_enable(cli_req_t *req)
{
    int                   a;
    vtss_port_no_t       port, port_no;
    BOOL                  first=1;
    vtss_qos_port_conf_t qos;
    char                  buf[80], *p;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
    
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->table_index != 0) {
                qos.custom_filter.global_enable[req->table_index-1] = req->enable;
            } else {            
                for(a=0; a<VTSS_CUSTOM_FILTER_COUNT; a++) 
                    qos.custom_filter.global_enable[a] = req->enable;                
            }
            
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            if (first) {        
                first = 0;
                printf("                          Port membership status for global filter\n");
                p = buf;
                p +=sprintf(p, "Port  ");
                for(a=0; a<VTSS_CUSTOM_FILTER_COUNT; a++)
                    p += sprintf(p, "%-10d", a + 1);
                cli_table_header(buf);
            }
            
            printf("%-6u", port);
            for(a=0; a<VTSS_CUSTOM_FILTER_COUNT; a++) 
                printf("%s  ",cli_bool_txt(qos.custom_filter.global_enable[a]));
            printf("\n");
        }        
    }
}

/* 1 local Filter for port  */
static void cli_cmd_qos_filter(cli_req_t *req, BOOL trig)
{
    int                   i;
    vtss_port_no_t       port, port_no;
    vtss_qos_port_conf_t qos;
    char                  pos[3];
    BOOL                  first=1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
    
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }

        if (req->set) {
            if (trig) {                           
                if (req->offset%2) {
                    printf("The offset must be in even bytes\n");
                    return;
                }                
                qos.custom_filter.local.filter[req->trig_index].header = req->layer;
                qos.custom_filter.local.filter[req->trig_index].offset = req->offset;
                qos.custom_filter.local.filter[req->trig_index].val = req->pattern;
                qos.custom_filter.local.filter[req->trig_index].mask = req->mask;
            } else {
                qos.custom_filter.local.forward = req->forward;
                qos.custom_filter.local.qos.prio = req->class;
                qos.custom_filter.local.qos.red = req->red;
            }
            
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
            
        } else {
            if (trig) {                                     
                printf("Port no: %u\n",port);                
                printf("-----------\n");
                printf("Trig-id  Pos      Offset   Pattern  Mask   Drop/Fwd Class    Red\n");
                
                for(i=0; i<7; i++) {
                    if (qos.custom_filter.local.filter[i].header == VTSS_HEADER_L2) {
                        strcpy(pos,"L2");
                    } else if (qos.custom_filter.local.filter[i].header == VTSS_HEADER_L3) {
                        strcpy(pos,"L3");
                    } else if (qos.custom_filter.local.filter[i].header == VTSS_HEADER_L4) {
                        strcpy(pos,"L4");
                    } else {
                        strcpy(pos,"-");
                    }
                    printf("%-8d %-8s %-8u %#-8x %#-8x",i,
                           pos,
                           qos.custom_filter.local.filter[i].offset,
                           qos.custom_filter.local.filter[i].val,
                           qos.custom_filter.local.filter[i].mask);
                    
                    if (i==0) {
                        printf("%-8s", qos.custom_filter.local.forward?"Forward":"Drop");
                        if (qos.custom_filter.local.forward) {
                            printf(" %-8u %-8u\n", qos.custom_filter.local.qos.prio,
                                   qos.custom_filter.local.qos.red); 
                        } else {
                            printf("%-8s %-8s\n","-","-"); 
                        }
                    } else {
                        printf("\n");                        
                    }                        
                }                
            } else {
                if (first) {        
                    printf("Port   Drop/Fwd Class    Red\n");
                    first = 0;
                }
                
                printf("%-8u %-8s %-8u %-8u\n",
                       port,
                       qos.custom_filter.local.forward?"Forward":"Drop",
                       qos.custom_filter.local.qos.prio,
                       qos.custom_filter.local.qos.red);                                                    
            }
        }
    }
}

/* Setup the global filters (8) */
static void cli_cmd_qos_global_filter(cli_req_t *req, BOOL trig)
{
    int a,i,table_index;
    vtss_qos_conf_t qos_glb;
    char             pos[3];
    
    if (vtss_qos_conf_get(req->api_inst, &qos_glb) != VTSS_RC_OK) {
        printf("Could not get QoS conf\n");
        return;
    }
    
    if (req->set) {
        if (trig) {                           
            if (req->offset%2) {
                printf("The offset must be in even bytes\n");
                return;
            }
            
            qos_glb.custom_filter[req->table_index-1].filter[req->trig_index].header = req->layer;
            qos_glb.custom_filter[req->table_index-1].filter[req->trig_index].offset = req->offset;
            qos_glb.custom_filter[req->table_index-1].filter[req->trig_index].val = req->pattern;
            qos_glb.custom_filter[req->table_index-1].filter[req->trig_index].mask = req->mask;
        } else {
            qos_glb.custom_filter[req->table_index-1].forward = req->forward;
            qos_glb.custom_filter[req->table_index-1].qos.prio = req->class;
            qos_glb.custom_filter[req->table_index-1].qos.red = req->red;
        }

        if (vtss_qos_conf_set(req->api_inst, &qos_glb) != VTSS_RC_OK) {
            printf("Could not set QoS conf\n");
            return;
        }
        
    } else {

        if (trig) {
            for(a=0; a<VTSS_CUSTOM_FILTER_COUNT; a++) {                
                if (req->table_index != 0)
                    table_index = req->table_index-1;
                else
                    table_index = a;
                                       
                printf("Table id: %d\n",table_index+1);
                printf("-----------\n");
                printf("Trig-id  Pos      Offset   Pattern  Mask   Drop/Fwd Class    Red\n");
                    
                for(i=0; i<7; i++) {
                    if (qos_glb.custom_filter[a].filter[i].header == VTSS_HEADER_L2) {
                        strcpy(pos,"L2");
                    } else if (qos_glb.custom_filter[table_index].filter[i].header == VTSS_HEADER_L3) {
                        strcpy(pos,"L3");
                    } else if (qos_glb.custom_filter[table_index].filter[i].header == VTSS_HEADER_L4) {
                        strcpy(pos,"L4");
                    } else {
                        strcpy(pos,"-");
                    }
                    printf("%-8d %-8s %-8u %#-8x %#-8x",i,
                           pos,
                           qos_glb.custom_filter[table_index].filter[i].offset,
                           qos_glb.custom_filter[table_index].filter[i].val,
                           qos_glb.custom_filter[table_index].filter[i].mask);
                    
                    if (i==0) {
                        printf("%-8s",qos_glb.custom_filter[table_index].forward?"Forward":"Drop");
                        if (qos_glb.custom_filter[table_index].forward) {
                            printf(" %-8u %-8u\n", qos_glb.custom_filter[table_index].qos.prio,
                                                   qos_glb.custom_filter[table_index].qos.red); 
                        } else {
                            printf("%-8s %-8s\n","-","-"); 
                        }
                    } else {
                        printf("\n");                        
                    }                        
                }

                if (req->table_index != 0)
                    break;
            }
        } else {
            printf("Table-id   Drop/Fwd Class    Red\n");
            for(a=0; a<VTSS_CUSTOM_FILTER_COUNT; a++) {                
                if (req->table_index != 0)
                    table_index = req->table_index-1;
                else
                    table_index = a;                   
                   
                printf("%-11d %-8s %-8u %-8u\n",                               
                       table_index+1,
                       qos_glb.custom_filter[table_index].forward?"Fwd":"Drop",
                       qos_glb.custom_filter[table_index].qos.prio,
                       qos_glb.custom_filter[table_index].qos.red);                    

                if (req->table_index != 0)
                    break;
            }
        }
    }
}

/* The primary order where the endpoints are ranked and included/excluded */
static void cli_cmd_qos_priorder(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    BOOL                 first = 1;
    vtss_port_no_t       port, port_no;
    int                  i;


    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.order[req->order] = req->endpoint;                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            if (first) {
              first = 0;
              printf("Order_rank: ");
              for (i = 0; i < VTSS_QOS_ORDER_COUNT; i++) 
                printf("%-4d",i);

              printf("\n");                            
            }
            printf("%-12u",port);                            
            for (i = 0; i < VTSS_QOS_ORDER_COUNT; i++) 
                printf("%-4d",qos.order[i]);    

            printf("\n");                            
        }
    }
}

/* Setup VLAN tag releated functions */
static void cli_cmd_qos_tag(cli_req_t *req)
{
    vtss_port_no_t       port, port_no;
    BOOL first=1;
    vtss_qos_port_conf_t qos;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {

        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            if (req->cfi) {
                qos.ctag_cfi_stop = req->enable;
            } else if (req->ctag) {
                qos.ctag_stop = req->enable;
            } else if (req->vlan_aware) {
                qos.vlan_aware = req->enable;
            } else if (req->tag_inner) {
                qos.tag_inner = req->enable;
            } else {
                printf("No match\n");
            }

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {            
            if (first) {
                printf("Port       ctag        cfi      vlan_aware  tag_inner\n");
                first = 0;
            }
            printf("%-8u %-11s %-11s %-11s %-11s\n",
                   port, 
                   qos.ctag_stop?"Enabled":"Disabled", 
                   qos.ctag_cfi_stop?"Enabled":"Disabled", 
                   qos.vlan_aware?"Enabled":"Disabled", 
                   qos.tag_inner?"Enabled":"Disabled");
        }
    }
}

/* The alternate order where the endpoints are ranked and included/excluded */
static void cli_cmd_qos_l2_filter_altorder(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    BOOL                 first = 1;
    vtss_port_no_t       port, port_no;
    int                  i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
   
        if (req->set) {           
            qos.dmac_vid_etype.order[req->order] = req->endpoint;
                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            if (first) {
                printf("Endpoint:   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16\n");
                first = 0;
            }            
            printf("%-10u", port);                
            for (i = 0; i < VTSS_QOS_ORDER_COUNT; i++)
                printf("  %d",qos.dmac_vid_etype.order[i]);

            printf("\n");                
        }
    }
}

static void cli_cmd_qos_l2_filter_dmac(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                   i, table_id;
    char                  buf1[32];
    char                  buf2[32];

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }

  
        if (req->set) {           
            for (i=0; i<6; i++) {
                qos.dmac_vid_etype.filter[req->table_id-1].dmac.value[i] = req->dmac[i];
                qos.dmac_vid_etype.filter[req->table_id-1].dmac.mask[i] = req->dmac_mask[i];
            }
                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("%-10s %-8s\n","Table","Dmac/Mask");

            for (table_id=0; table_id < 2; table_id++) {                
                printf("%-8d%s\n%25s\n",table_id+1, 
                       misc_mac_txt(qos.dmac_vid_etype.filter[table_id].dmac.value, buf1),
                       misc_mac_txt(qos.dmac_vid_etype.filter[table_id].dmac.mask, buf2));
            }
        }
    }
}


static void cli_cmd_qos_l2_filter_etype(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  table_id;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
   
        if (req->set) {           
            qos.dmac_vid_etype.filter[req->table_id-1].etype.value = req->etype;
            qos.dmac_vid_etype.filter[req->table_id-1].etype.mask = req->mask;
                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");
            printf("%-8s %-8s\n","Table","Etype/Mask");

            for (table_id=0; table_id < 2; table_id++) {                
                printf("%-8d%#8x\n%#16x\n",table_id+1, 
                       qos.dmac_vid_etype.filter[table_id].etype.value,
                       qos.dmac_vid_etype.filter[table_id].etype.mask);
            }
        }
    }
}

static void cli_cmd_qos_l2_filter_vid(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  table_id;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
   
        if (req->set) {           
            qos.dmac_vid_etype.filter[req->table_id-1].vid.value = req->vid;
            qos.dmac_vid_etype.filter[req->table_id-1].vid.mask = req->mask;
                        
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("%-8s %-8s\n","Table","VID/Mask");

            for (table_id=0; table_id < 2; table_id++) {                
                printf("%-8d%8d\n%#16x\n",table_id+1, 
                       qos.dmac_vid_etype.filter[table_id].vid.value,
                       qos.dmac_vid_etype.filter[table_id].vid.mask);
            }
        }
    }
}
static void cli_cmd_qos_l2_filter_port(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  table_id;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
   
        if (req->set) {
            qos.dmac_vid_etype.filter[req->table_id-1].enable = req->enable;
            qos.dmac_vid_etype.order_enable = req->alt;                
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }            
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("%-10s %-10s %-10s\n","Table","Enabled","Class table");

            for (table_id=0; table_id < 2; table_id++) {
                       printf("%-10d %-10s %10s\n",table_id+1, 
                       qos.dmac_vid_etype.filter[table_id].enable?"Yes":"No",
                       qos.dmac_vid_etype.order_enable?"Alternate":"Primary");
            }
        }
    }
}

static void cli_cmd_qos_l2_filter_conf(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  table_id;
    char                 buf1[32];
    char                 buf2[32];


    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-5s %-8s %-10s %-36s %-10s %-10s\n",
               "Table","Enabled","Table","             Dmac/Mask","Etype/Mask","  Vid/Mask");
        
        for (table_id=0; table_id < 2; table_id++) {
            printf("%-5d %-8s %-10s %-17s/%-17s %#6x/%#-6x %4d/%#-6x\n",table_id+1, 
                   qos.dmac_vid_etype.filter[table_id].enable?"  Yes":"  No",
                   qos.dmac_vid_etype.order_enable?"Alternate":"Primary",
                   misc_mac_txt(qos.dmac_vid_etype.filter[table_id].dmac.value, buf1),
                   misc_mac_txt(qos.dmac_vid_etype.filter[table_id].dmac.mask, buf2),
                   qos.dmac_vid_etype.filter[table_id].etype.value,
                   qos.dmac_vid_etype.filter[table_id].etype.mask,
                   qos.dmac_vid_etype.filter[table_id].vid.value,
                   qos.dmac_vid_etype.filter[table_id].vid.mask);
        }
    }
}



/* Setup priority and red profiles for endpoints     */
static void cli_cmd_qos_class_misc(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;
    char                  buf[200];

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            if (req->def) {
                qos.default_qos.prio = req->class;
                qos.default_qos.red = req->red;
            } else if (req->l2_ctrl) {
                qos.l2_control_qos.prio = req->class;
                qos.l2_control_qos.red = req->red;
            } else if (req->l3_ctrl) {
                qos.l3_control_qos.prio = req->class;
                qos.l3_control_qos.red = req->red;
            } else if (req->cfi) {
                qos.cfi_qos.prio = req->class;
                qos.cfi_qos.red = req->red;
            } else if (req->ipv4) {
                qos.ipv4_arp_qos.prio = req->class;
                qos.ipv4_arp_qos.red = req->red;
            } else if (req->ipv6) {
                qos.ipv6_qos.prio = req->class;
                qos.ipv6_qos.red = req->red;
            } else if (req->llc) {
                qos.llc_qos.prio = req->class;
                qos.llc_qos.red = req->red;
            } else {
                printf("No endpoint selected\n");
                return;
            }
                                
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
            
        } else {
            memset(&buf, 0, sizeof(buf));

            if (first) {
                printf("                 Class | Red              \n");
                printf("Port    Def   L2    L3    CFI   IPv4  IPv6  LLC\n");
                first = 0;
            }
            printf("%-8u", port);
            printf("%u|%-4u",qos.default_qos.prio, qos.default_qos.red);
            printf("%u|%-4u",qos.l2_control_qos.prio, qos.l2_control_qos.red);
            printf("%u|%-4u",qos.l3_control_qos.prio, qos.l3_control_qos.red);
            printf("%u|%-4u",qos.cfi_qos.prio, qos.cfi_qos.red);
            printf("%u|%-4u",qos.ipv4_arp_qos.prio, qos.ipv4_arp_qos.red);
            printf("%u|%-4u",qos.ipv6_qos.prio, qos.ipv6_qos.red);
            printf("%u|%-4u",qos.llc_qos.prio, qos.llc_qos.red);
            printf("\n");                
        }
    }
}

/* Setup range/specific and classification   */
static void cli_cmd_qos_class_tcpudp(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;
    int                  i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.udp_tcp.local.pair[req->table_id-1].port = req->tcp_udp_port;
            qos.udp_tcp.local.range = req->range;         
            if (req->range) {
                qos.udp_tcp.local.pair[0].qos.prio = req->class;
                qos.udp_tcp.local.pair[0].qos.red = req->red;            
                qos.udp_tcp.local.pair[1].qos.prio = req->class;
                qos.udp_tcp.local.pair[1].qos.red = req->red;            
            } else {
                qos.udp_tcp.local.pair[req->table_id-1].qos.prio = req->class;
                qos.udp_tcp.local.pair[req->table_id-1].qos.red = req->red;            
            }

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-8s %-14s %-8s %-8s %-8s\n","Index","Range|Specific","IP-port","Class","Red");
                first = 0;
            }
            for (i = 0; i<2; i++) {
                printf("%-8d %-14s %#-8x %-8u %-8u\n",i+1,
                                            qos.udp_tcp.local.range?"Range":"Specific",
                                            qos.udp_tcp.local.pair[i].port,
                                            qos.udp_tcp.local.pair[i].qos.prio,
                                            qos.udp_tcp.local.pair[i].qos.red);
            }       
        }
    }
}

/* Setup priority and red profile IP protocol    */
static void cli_cmd_qos_class_ipproto(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.ip_proto.proto = req->ip_proto;
            qos.ip_proto.qos.prio = req->class;
            qos.ip_proto.qos.red = req->red;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
      
            if (first) {
                printf("%-8s %-10s %-8s %-8s\n","Port","IP-proto","Class","Red");
                first = 0;
            }
            printf("%-8u %-10d %-8u %-8u\n", port,
                   qos.ip_proto.proto,
                   qos.ip_proto.qos.prio,
                   qos.ip_proto.qos.red);       
        }
    }
}

/* Setup priority and red profile for Ethernet Type   */
static void cli_cmd_qos_class_etype(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.etype.etype = req->etype;
            qos.etype.qos.prio = req->class;
            qos.etype.qos.red = req->red;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
      
            if (first) {
                printf("%-8s %-10s %-8s %-8s\n","Port","Etype","Class","Red");
                first = 0;
            }
            printf("%-8u %#-10x %-8u %-8u\n",port,
                   qos.etype.etype,
                   qos.etype.qos.prio,
                   qos.etype.qos.red);       
        }
    }
}

/* Setup priority and red profile for VID   */
static void cli_cmd_qos_class_vid(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                 first = 1;
    int                  i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.vlan[req->table_id-1].vid = req->vid;
            qos.vlan[req->table_id-1].qos.prio = req->class;
            qos.vlan[req->table_id-1].qos.red = req->red;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-8s %-8s %-8s %-8s\n","Index","VID","Class","Red");
                first = 0;
            }
            for (i = 0; i<2; i++) {
                printf("%-8d %-8d %-8u %-8u\n", i + 1, qos.vlan[i].vid,
                                                       qos.vlan[i].qos.prio,
                                                       qos.vlan[i].qos.red);
            }
        }
    }
}

/* Setup priority and red profile for Tag Prio  */
static void cli_cmd_qos_class_tagprio(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  i;
    BOOL                  first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
    
        if (req->set) {
            qos.vlan_tag_qos[req->tag_prio].prio = req->class;
            qos.vlan_tag_qos[req->tag_prio].red = req->red;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-8s %-8s %-8s\n","Tag Prio","Class","Red");
                first = 0;
            }
            for (i = 0; i<8; i++) {
                printf("%-8d %-8u %-8u\n",i,
                       qos.vlan_tag_qos[i].prio,
                       qos.vlan_tag_qos[i].red);
            }
        }
    }
}

/* Setup priority and red profile for MPLS   */
static void cli_cmd_qos_class_mpls(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  i;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            qos.mpls_exp_qos[req->mpls_exp].prio = req->class;
            qos.mpls_exp_qos[req->mpls_exp].red = req->red;
            
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-10s %-8s %-8s\n","MPLS exp","Class","Red");
                first = 0;
            }
            for (i = 0; i<8; i++) {
                printf("%-10d %-8u %-8u\n",i,
                       qos.mpls_exp_qos[i].prio,
                       qos.mpls_exp_qos[i].red);
            }
        }
    }
}

/* Setup the port policer   */
static void cli_cmd_qos_port_police(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                   i;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->rate == 0) {
                qos.policer_port[req->table_id-1].broadcast = 0;
                qos.policer_port[req->table_id-1].multicast = 0;
                qos.policer_port[req->table_id-1].unicast = 0;
                qos.policer_port[req->table_id-1].rate = VTSS_BITRATE_DISABLED;
                qos.policer_port[req->table_id-1].level = 0;
            } else {
                if (req->policer.unicast)
                    qos.policer_port[req->table_id-1].unicast = 1;
                if (req->policer.multicast)
                    qos.policer_port[req->table_id-1].multicast = 1;
                if (req->policer.broadcast)
                    qos.policer_port[req->table_id-1].broadcast = 1;
                
                qos.policer_port[req->table_id-1].level = req->policer.level;
                qos.policer_port[req->table_id-1].rate = req->rate;
            }

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-10s %-10s %-10s %-10s %-10s %-10s\n","Policer-id","  UC","  BC","  MC","Rate/Kb","Level/KB");
                first = 0;
            }
            for (i = 0; i<2; i++) {
                if (qos.policer_port[i].rate == VTSS_BITRATE_DISABLED) {
                    printf("%-10d %-10s %-10s %-10s %-10s %-10s\n",i+1,"  -","  -","  -","-","-"); 
                } else {
                    printf("%-10d %-10s %-10s %-10s %-10u %-10u\n",i+1,
                           qos.policer_port[i].unicast?"Enabled":"Disabled",
                           qos.policer_port[i].broadcast?"Enabled":"Disabled",
                           qos.policer_port[i].multicast?"Enabled":"Disabled",
                           qos.policer_port[i].rate,
                           qos.policer_port[i].level/1024);
                }
            }
        }
    }
}


/* Setup the queue policers   */
static void cli_cmd_qos_queue_police(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                 first = 1;
    int                  i;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->rate == 0) {
                qos.policer_queue[req->queue_index].broadcast = 0;
                qos.policer_queue[req->queue_index].multicast = 0;
                qos.policer_queue[req->queue_index].unicast = 0;
                qos.policer_queue[req->queue_index].rate = VTSS_BITRATE_DISABLED;
                qos.policer_queue[req->queue_index].level = 0;
            } else {
                if (req->policer.unicast)
                    qos.policer_queue[req->queue_index].unicast = 1;
                if (req->policer.multicast)
                    qos.policer_queue[req->queue_index].multicast = 1;
                if (req->policer.broadcast)
                    qos.policer_queue[req->queue_index].broadcast = 1;
                
                qos.policer_queue[req->queue_index].level = req->policer.level;
                qos.policer_queue[req->queue_index].rate = req->rate;
            }
            
            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            if (first) {
                printf("%-10s %-10s %-10s %-10s %-10s %-10s\n","Queue","  UC","  BC","  MC","Rate/Kb","Level/KB");
                first = 0;
            }
            for (i = VTSS_QUEUE_START; i<VTSS_QUEUE_END; i++) {
                if (qos.policer_queue[i].rate == VTSS_BITRATE_DISABLED) {
                    printf("%-10d %-10s %-10s %-10s %-10s %-10s\n",i,"  -","  -","  -","-","-"); 
                } else {
                    printf("%-10d %-10s %-10s %-10s %-10u %-10u\n",i,
                           qos.policer_queue[i].unicast?"Enabled":"Disabled",
                           qos.policer_queue[i].broadcast?"Enabled":"Disabled",
                           qos.policer_queue[i].multicast?"Enabled":"Disabled",
                           qos.policer_queue[i].rate,
                           qos.policer_queue[i].level/1024);
                }
            }
        }
    }
}

/* Setup the port shapers   */
static void cli_cmd_qos_port_shaper(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->rate == 0) {
                qos.shaper_port.level = 0;
                qos.shaper_port.rate = VTSS_BITRATE_DISABLED;
            } else {
                qos.shaper_port.level = req->policer.level;
                qos.shaper_port.rate = req->rate;
            }

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            if (first) {
                printf("%-10s %-10s %-10s\n","Port","Rate/Kb","Level/KB");
                first = 0;
            }
            if (qos.shaper_port.rate == VTSS_BITRATE_DISABLED) {
                printf("%-10u %-10s %-10s\n",port,"-","-");
            } else {
                printf("%-10u %-10u %-10u\n",port,
                       qos.shaper_port.rate,
                       qos.shaper_port.level/1024);            
            }
        }
    }
}


/* Setup the RED threshold and weight  */
static void cli_cmd_qos_red_threshold(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  q;
    BOOL                 first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            qos.red[req->queue_index].min = req->red_min;
            qos.red[req->queue_index].max = req->red_max;
            qos.red[req->queue_index].weight = req->red_weight;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");

            if (first) {
                printf("%-10s %-10s %-10s %-10s\n","Queue","Min","Max","Weight");
                first = 0;
            }

            for (q = VTSS_QUEUE_START; q<VTSS_QUEUE_END; q++) {
                printf("%-10d %-10u %-10u %-10d\n",q,
                       qos.red[q].min, qos.red[q].max, qos.red[q].weight);
            }            
        }
    }
}

/* Setup the RED probability profiles  */
static void cli_cmd_qos_red_probability(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  q;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            qos.red[req->queue_index].max_prob_1 = req->red_prob1;
            qos.red[req->queue_index].max_prob_2 = req->red_prob2;
            qos.red[req->queue_index].max_prob_3 = req->red_prob3;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");

            if (first) {
                printf("%-10s %-10s %-10s %-10s\n","Queue","Prob_1 %","Prob_2 %","Prob_3 %");
                first = 0;
            }

            for (q = VTSS_QUEUE_START; q<VTSS_QUEUE_END; q++) {
                printf("%-10d %-10d %-10d %-10d\n",q,
                       qos.red[q].max_prob_1, qos.red[q].max_prob_2, qos.red[q].max_prob_3);
            }            
        }
    }
}

/* Enable / Disable the Red profile */
static void cli_cmd_qos_red_profile(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                  q;
    BOOL                 first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            qos.red[req->queue_index].enable = req->enable;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");

            if (first) {
                printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
                       "Queue","Enabled","Min","Max","Weight","Prob_1 %","Prob_2 %","Prob_3 %");
                first = 0;
            }

            for (q = VTSS_QUEUE_START; q<VTSS_QUEUE_END; q++) {
                printf("%-10d %-10s %-10u %-10u %-10d %-10d %-10d %-10d\n",q,
                       qos.red[q].enable?"Yes":"No", 
                       qos.red[q].min, 
                       qos.red[q].max, 
                       qos.red[q].weight, 
                       qos.red[q].max_prob_1, 
                       qos.red[q].max_prob_2, 
                       qos.red[q].max_prob_3);
            }
        }
    }
}

/* Setup the queue (1-6) scheduler (weight) */
static void cli_cmd_qos_queue_schedule(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    int                   q;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->queue_index > 5) {
                printf("Illegal queue no %u  (0-5) \n",req->queue_index);
                return;                    
            }
            qos.scheduler.queue_pct[req->queue_index] = req->value;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");

            if (first) {
                printf("%-10s %-10s\n","Queue","Pct");
                first = 0;
            }

            for (q = VTSS_QUEUE_START; q<VTSS_QUEUE_END-2; q++) {
                printf("%-10d %-10d\n",q,qos.scheduler.queue_pct[q]);

            }
        }
    }
}

/* Setup the port scheduler (weight) */
static void cli_cmd_qos_port_schedule(cli_req_t *req)
{
    vtss_qos_port_conf_t qos;
    vtss_port_no_t       port, port_no;
    BOOL                  first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;
        
        if (vtss_qos_port_conf_get(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
        
        if (req->set) {
            if (req->rate == 0) {
                qos.scheduler.rate = VTSS_BITRATE_DISABLED;
                printf("Disabled\n");
            }
            else
                qos.scheduler.rate = req->rate;

            if (vtss_qos_port_conf_set(req->api_inst, port_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            if (first) {
                printf("%-10s %-10s\n","Port","Min-Rate/Kb");
                first = 0;
            }
            if (qos.scheduler.rate == VTSS_BITRATE_DISABLED)
                printf("%-10u %-10s\n",port,"Disabled");                            
            else
                printf("%-10u %-10u\n",port, qos.scheduler.rate);       
        }
    }
}

/* Setup the logical port scheduler (weight) */
static void cli_cmd_qos_lport_schedule(cli_req_t *req)
{
    vtss_qos_lport_conf_t qos;
    int                    lport_no;
    char                   txbuf[20],rxbuf[20];
    BOOL                   first = 1;
    
    for (lport_no = 0; lport_no < VTSS_LPORTS; lport_no++) {
        if (req->lport_list[lport_no] == 0)
            continue;
        
        if (vtss_qos_lport_conf_get(req->api_inst, lport_no, &qos) != VTSS_RC_OK) {
            printf("Could not get QoS conf\n");
            return;
        }
       
        if (req->set) {
            if (req->tx) 
                qos.scheduler.tx_rate = req->rate;
            else if (req->rx) 
                qos.scheduler.rx_rate = req->rate;
            

            if (vtss_qos_lport_conf_set(req->api_inst, lport_no, &qos) != VTSS_RC_OK) {
                printf("Could not set QoS conf\n");
                return;
            }
        } else {
            if (first) {
                printf("%-10s %-10s %-10s\n","Lport","Rx Rate","Tx Rate");
                first = 0;
            }           
            if (qos.scheduler.rx_rate != VTSS_BITRATE_DISABLED)
                sprintf(rxbuf,"%u",qos.scheduler.rx_rate);
            else 
                sprintf(rxbuf,"%s","-");


            if (qos.scheduler.tx_rate != VTSS_BITRATE_DISABLED)
                sprintf(txbuf,"%u",qos.scheduler.tx_rate);
            else 
                sprintf(txbuf,"%s","-");


            printf("%-10d %-10s %-10s\n",lport_no, rxbuf, txbuf);
        }
    }
}
#endif /* VTSS_ARCH_B2 */
#endif /* VTSS_FEATURE_PORT_CONTROL */

#if defined(VTSS_FEATURE_LAYER2) || defined(VTSS_ARCH_DAYTONA)

static char *cli_mac_txt(const uchar *mac, char *buf)
{
    sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

#endif /* VTSS_FEATURE_LAYER2 || VTSS_ARCH_DAYTONA */

#if defined(VTSS_FEATURE_LAYER2)
static void cli_cmd_mac_add(cli_req_t *req)
{
    vtss_mac_table_entry_t entry;
    int                    i;
    vtss_port_no_t         port_no;

    memset(&entry, 0, sizeof(entry));
    entry.locked = 1;
    entry.vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++)
        entry.vid_mac.mac.addr[i] = req->mac[i];
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        entry.destination[port_no] = (req->port_list[iport2uport(port_no)] ? 1 : 0);
    vtss_mac_table_add(req->api_inst, &entry);
}

static void cli_cmd_mac_del(cli_req_t *req)
{
    vtss_vid_mac_t vid_mac;
    int            i;
    
    vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++)
        vid_mac.mac.addr[i] = req->mac[i];
    vtss_mac_table_del(req->api_inst, &vid_mac);
}

static char *cli_port_list_txt(BOOL port_list[VTSS_PORT_ARRAY_SIZE], char *buf)
{
    char           *p = buf;
    vtss_port_no_t port_no, port;
    int            first = 1, count = 0;
    BOOL           member;

    *p = '\0';
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        member = port_list[port_no];
        if ((member && 
             (count == 0 || port_no == (VTSS_PORT_NO_END - 1))) || (!member && count > 1)) {
            port = iport2uport(port_no);
            p += sprintf(p, "%s%u",
                         first ? "" : count > (member ? 1 : 2) ? "-" : ",",
                         member ? port : (port - 1));
            first = 0;
        }
        if (member)
            count++;
        else
            count=0;
    }
    if (p == buf)
        sprintf(p, "None");
    return buf;
}



static void cli_mac_print(vtss_mac_table_entry_t *entry, BOOL first)
{
    char buf[64];

    if (first)
        cli_table_header("Type    VID  MAC Address        Ports");
    printf("%s %-4d %s  ",
           entry->locked ? "Static " : "Dynamic",
           entry->vid_mac.vid, 
           cli_mac_txt(entry->vid_mac.mac.addr, buf));
    printf("%s%s\n",
           cli_port_list_txt(entry->destination, buf),
           entry->copy_to_cpu ? ",CPU" : "");
}

static void cli_cmd_mac_lookup(cli_req_t *req)
{
    vtss_mac_table_entry_t entry;
    int                    i;
    
    entry.vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++)
        entry.vid_mac.mac.addr[i] = req->mac[i];
    if (vtss_mac_table_get(req->api_inst, &entry.vid_mac, &entry) == VTSS_RC_OK)
        cli_mac_print(&entry, 1);
}

static void cli_cmd_mac_dump(cli_req_t *req)
{
    vtss_mac_table_entry_t entry;
    BOOL                   first = 1; 

    memset(&entry, 0, sizeof(entry));
    while (vtss_mac_table_get_next(req->api_inst, &entry.vid_mac, &entry) == VTSS_RC_OK) {
        cli_mac_print(&entry, first);
        first = 0;
    }
}

static void cli_cmd_mac_flush(cli_req_t *req)
{
    vtss_mac_table_flush(req->api_inst);
}

static void cli_cmd_mac_age_time(cli_req_t *req)
{
    vtss_mac_table_age_time_t age_time;
    
    if (req->set)
        vtss_mac_table_age_time_set(req->api_inst, req->value);
    else if (vtss_mac_table_age_time_get(req->api_inst, &age_time) == VTSS_RC_OK)
        printf("Age Time: %u\n", age_time);
}
#endif /* VTSS_FEATURE_LAYER2 */

#if defined(VTSS_FEATURE_MISC)
static void cli_cmd_debug_value(cli_req_t *req, u32 value)
{
    int i;
    
    if (req->binary)
        for (i = 31; i >= 0; i--)
            printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i == 0 ? "" : ".");
    else if (req->decimal)
        printf("%-10u", value);
    else 
        printf("0x%08x", value);
    printf("\n");
}

/* Debug register read/write */
static void cli_cmd_debug_reg_l28(cli_req_t *req, BOOL write)
{
    int   i, blk, sub, addr, addr_count = 0, first = 1;
    u32 reg, value;
    char  buf[80];
    BOOL  da = req->board->daytona;
    for (i = 0; i < 2; i++) {
        for (blk = 0; blk < CLI_TGT_MAX; blk++) {
            if (req->blk_list[blk] == 0)
                continue;

            for (sub = 0; sub < CLI_SUB_MAX; sub++) {
                if (req->sub_list[sub] == 0)
                    continue;
                
                for (addr = 0; addr < CLI_ADDR_MAX; addr++) {
                    if (req->addr_list[addr] == 0)
                        continue;
                    if (da)
                        reg = ((blk << 16) | (sub << 10) | (addr << 2));
                    else
                    reg = ((blk << 12) | (sub << 8) | addr);
                    
                    if (i == 0) {
                        /* Count number of addresses to read */
                        addr_count++;
                    } else if (write) {
                        /* Write */
                        vtss_reg_write(req->api_inst, 0, reg, req->value);
                    } else {
                        /* Read */
                        vtss_reg_read(req->api_inst, 0, reg, &value);
                        if (first) {
                            first = 0;
                            if (addr_count > 1 || req->binary) {
                                sprintf(buf, "%s%s", 
                                        addr_count > 1  ? "Blk   Sub   Addr  " : "",
                                        req->binary ? 
                                        "31*****24*23*****16*15******8*7*******0" : "Value");
                                cli_table_header(buf);
                            }
                        }
                        if (addr_count > 1)
                            printf("0x%02x  0x%02x  0x%02x  ", blk, sub, addr);
                        cli_cmd_debug_value(req, value);
                    }
                } /* addr loop */
            } /* sub loop */
        } /* blk loop */
    }
}

static void cli_cmd_debug_reg(cli_req_t *req, BOOL write)
{
    u32 i, tgt, addr, reg_addr, value, addr_count = 0, first = 1;
    char  buf[80];
    
    for (i = 0; i < 2; i++) {
        for (tgt = 0; tgt < CLI_TGT_MAX; tgt++) {
            if (req->blk_list[tgt] == 0)
                continue;

            for (addr = 0; addr < CLI_ADDR_MAX; addr++) {
                if (req->addr_list[addr] == 0)
                    continue;
                    
                reg_addr = (req->board->b2 ? ((tgt << 16) | addr) : ((tgt << 14) | addr));
#ifdef VTSS_ARCH_DAYTONA 
                reg_addr <<= 2;
#endif /* VTSS_ARCH_DAYTONA */
                
                if (i == 0) {
                    /* Count number of addresses to read */
                    addr_count++;
                } else if (write) {
                    /* Write */
                    vtss_reg_write(req->api_inst, 0, reg_addr, req->value);
                } else {
                    /* Read */
                    vtss_reg_read(req->api_inst, 0, reg_addr, &value);
                    if (first) {
                        first = 0;
                        if (addr_count > 1 || req->binary) {
                            sprintf(buf, "%s%s", 
                                    addr_count > 1  ? "Tgt   Addr    " : "",
                                    req->binary ? 
                                    "31*****24*23*****16*15******8*7*******0" : "Value");
                            cli_table_header(buf);
                        }
                    }
                    if (addr_count > 1)
                        printf("0x%02x  0x%04x  ", tgt, addr);
                    cli_cmd_debug_value(req, value);
                }
            } /* addr loop */
        } /* tgt loop */
    }
}
#endif /* VTSS_FEATURE_MISC */

#if defined(VTSS_FEATURE_INTERRUPTS)
static void cli_cmd_debug_interrupt(cli_req_t *req)
{
  static vtss_intr_t    intr_mask;
  vtss_intr_t    intr;
  vtss_port_no_t port_no;
  BOOL           first = 1;

  memset(&intr, 0, sizeof(intr));
  if (req->set) {
    /* Set interrupt mask */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
      // Do only change setting for ports in the port list
      if (req->port_list[iport2uport(port_no)] != 0 || vtss_appl_iftype_is_spi4(req->appl_inst, port_no)) {
        intr_mask.link_change[port_no] = req->enable; // Enable/disable link change
      }
    }            
    vtss_intr_mask_set(req->api_inst,&intr_mask);
  } else {
    /* Get interrupt status */
    if (vtss_intr_status_get(req->api_inst,&intr) == VTSS_RC_OK) {
      for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
          if (req->port_list[iport2uport(port_no)] == 0 || vtss_appl_iftype_is_spi4(req->appl_inst, port_no))
          continue;
        if (first)
          cli_table_header("Port  Status");
        first = 0;
        
        if (intr_mask.link_change[port_no]) {
          printf("%-4u  %shanged\n", iport2uport(port_no), intr.link_change[port_no] ? "C" : "Unc");
        } else {
          printf("%-4u  -\n", iport2uport(port_no));
        }
        
      } 
    } 
  }
}
#endif /* VTSS_FEATURE_INTERRUPTS */


#ifdef VTSS_ARCH_DAYTONA
static void cli_cmd_debug_event_config(cli_req_t *req)
{
    vtss_interrupt_block_t  blocks;
    u32 i;

    if (vtss_interrupt_enable_get(req->api_inst, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_get() failed\n");

    if (req->block == daytona_upi) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_UPI_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_UPI_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_UPI_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_UPI_INT; break;
                default: blocks.ch_0 |= VTSS_CH_UPI_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_upi_event_enable(NULL, req->port, req->enable, req->upi_event) != VTSS_RC_OK)  printf("vtss_upi_event_enable() failed\n");
    }
    if (req->block == daytona_xfi) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.misc |= VTSS_MISC_CXFI_0_INT; break;
                case 1: blocks.misc |= VTSS_MISC_CXFI_1_INT; break;
                case 2: blocks.misc |= VTSS_MISC_LXFI_0_INT; break;
                case 3: blocks.misc |= VTSS_MISC_LXFI_1_INT; break;
                default: blocks.misc |= VTSS_MISC_CXFI_0_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_xfi_event_enable(NULL, req->port, req->enable, req->xfi_event) != VTSS_RC_OK)  printf("vtss_xfi_event_enable() failed\n");
    }
    if (req->block == daytona_otn) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_OTN1_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_OTN1_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_OTN2_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_OTN2_INT; break;
                default: blocks.ch_0 |= VTSS_CH_OTN1_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_otn_och_event_enable(NULL, req->port, req->enable, req->otn_och_event) != VTSS_RC_OK)  printf("vtss_otn_och_event_enable() failed\n");
        if (vtss_otn_otu_event_enable(NULL, req->port, req->enable, req->otn_otu_event) != VTSS_RC_OK)  printf("vtss_otn_otu_event_enable() failed\n");
        if (vtss_otn_odu_event_enable(NULL, req->port, req->enable, req->otn_odu_event) != VTSS_RC_OK)  printf("vtss_otn_odu_event_enable() failed\n");
        for (i=0; i<VTSS_OTN_ODUT_LEVEL_NUMBER; ++i)
            if (vtss_otn_odut_event_enable(NULL, req->port, i, req->enable, req->otn_odut_event) != VTSS_RC_OK)  printf("vtss_otn_odut_event_enable() failed\n");
    }
    if (req->block == daytona_ewis) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_WIS1_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_WIS1_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_WIS2_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_WIS2_INT; break;
                default: blocks.ch_0 |= VTSS_CH_WIS1_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_ewis_event_enable(NULL, req->port, req->enable, req->ewis_event) != VTSS_RC_OK)  printf("vtss_ewis_event_enable() failed\n");
    }
    if (req->block == daytona_pcs) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_PCS1_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_PCS1_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_PCS2_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_PCS2_INT; break;
                default: blocks.ch_0 |= VTSS_CH_PCS1_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_pcs_10gbase_r_event_enable(NULL, req->port, req->enable, req->pcs_event) != VTSS_RC_OK)  printf("vtss_pcs_10gbase_r_event_enable() failed\n");
    }
    if (req->block == daytona_mac) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_MAC1_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_MAC1_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_MAC2_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_MAC2_INT; break;
                default: blocks.ch_0 |= VTSS_CH_MAC1_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_mac10g_event_enable(NULL, req->port, req->enable, req->mac_event) != VTSS_RC_OK)  printf("vtss_mac10g_event_enable() failed\n");
    }
    if (req->block == daytona_gfp) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.ch_0 |= VTSS_CH_GFP1_INT; break;
                case 1: blocks.ch_1 |= VTSS_CH_GFP1_INT; break;
                case 2: blocks.ch_0 |= VTSS_CH_GFP2_INT; break;
                case 3: blocks.ch_1 |= VTSS_CH_GFP2_INT; break;
                default: blocks.ch_0 |= VTSS_CH_GFP1_INT; break;
            }
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_gfp_event_enable(NULL, req->port, req->enable, req->gfp_event) != VTSS_RC_OK)  printf("vtss_gfp_event_enable() failed\n");
    }

    if (req->block == daytona_i1588) {
        if (req->enable) {
            switch (req->port)
            {
                case 0: blocks.misc |= VTSS_MISC_IP1588_0_INT; break;
                case 1: blocks.misc |= VTSS_MISC_IP1588_1_INT; break;
                case 2: blocks.misc |= VTSS_MISC_IP1588_0_INT; break;
                case 3: blocks.misc |= VTSS_MISC_IP1588_1_INT; break;
                default: blocks.misc |= VTSS_CH_UPI_INT; break;
            }
            printf("Inside FN :: %s misc - [%x] \n\r",__FUNCTION__, (unsigned int)blocks.misc);
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)        printf("vtss_interrupt_enable_set() failed\n");
        }
        /* Daytona 1588 Interrupts are enabled through Protocol */
    }
    if (req->block == daytona_efec) {
        if (req->enable) {
            switch (req->port)
            {    
                case 0: 
                case 2: blocks.ch_0 |= VTSS_CH_EFEC_INT; break; 
                case 1:
                case 3: blocks.ch_1 |= VTSS_CH_EFEC_INT; break; 
                default: blocks.ch_0 |= VTSS_CH_EFEC_INT; break;
            }             
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)
                printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_efec_event_enable(NULL, req->port, req->enable, req->efec_event) != VTSS_RC_OK)
            printf("vtss_efec event failed\n");
    }   
    if (req->block == daytona_efeci7) {
        if (req->enable) {
            switch (req->port)
            {    
                case 0: 
                case 2: blocks.misc |= VTSS_MISC_EFEC7_0_INT; break; 
                case 1:
                case 3: blocks.misc |= VTSS_MISC_EFEC7_1_INT; break; 
                default: blocks.misc |= VTSS_MISC_EFEC7_0_INT; break;
            }             
            if (vtss_interrupt_enable_set(NULL, &blocks) != VTSS_RC_OK)
                printf("vtss_interrupt_enable_set() failed\n");
        }
        if (vtss_efeci7_event_enable(NULL, req->port, req->enable, req->efeci7_event) != VTSS_RC_OK)
            printf("vtss_efec event failed\n");
    }   
 
}


static void cli_cmd_debug_event_screen(cli_req_t *req, BOOL clear)
{
    static BOOL event_screen = FALSE;
    static u8 command_win[] = {27, '[', '1', ';', '5', '5', 'r', '\0'};
    static u8 reset_term[] = {27, 'c', '\0'};
    static u8 clear_screen[] = {27, '[', '2', 'J', '\0'};
    static u8 clear_line[] = {27, '[', '2', 'K', '\0'};
    static u8 cur_down[] = {27, '[', '1', 'B', '\0'};
    static u8 cur_event_top[] = {27, '[', '5', '6', ';', '1', 'H', '\0'};
    static u8 cur_event_but[] = {27, '[', '7', '7', ';', '1', 'H', '\0'};
    static u8 cur_command[] = {27, '[', '1', ';', '1', 'H', '\0'};
    static u8 cur_save[] = {27, '7', '\0'};
    static u8 cur_restore[] = {27, '8', '\0'};
    u32 i;

    if (clear) {
        printf("%s", cur_save);        /* Save cursor position */
        printf("%s", cur_event_top);   /* cursor to top of event window */
        printf("***************************************************************************************************************************");
        for (i=0; i<20; ++i) {
            printf("%s", cur_down);    /* Cursor down */
            printf("%s", clear_line);  /* Clear line */
        }
        printf("%s", cur_event_but);   /* cursor to buttom of event window */
        printf("***************************************************************************************************************************");
        printf("%s", cur_restore);     /* Restore cursor position */
        return;
    }

    if (req->enable) {
        printf("%s", clear_screen);    /* Clear screen */
        printf("%s", command_win);     /* Define command window */
        printf("%s", cur_event_top);   /* cursor to top of event window */
        printf("***************************************************************************************************************************");
        printf("%s", cur_event_but);   /* cursor to buttom of event window */
        printf("***************************************************************************************************************************");
        printf("%s", cur_command);     /* cursor to command line */

        event_screen = TRUE;
        vtss_board_daytona_event_screen(TRUE);
    }
    else {
        printf("%s", reset_term);

        event_screen = FALSE;
        vtss_board_daytona_event_screen(FALSE);
    }
}

static void cli_cmd_interrupt_status(cli_req_t *req)
{
  vtss_port_no_t port_no, port;

  for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
     port = iport2uport(port_no);
     if ((req->port_list[port] == 0))
         continue;
     printf("INTERUPT STATUS FOR THE PORT %d::\n", port);
     vtss_board_daytona_interrupt_without_mask(port_no);
  }
}

#endif /* VTSS_ARCH_DAYTONA */


#ifndef VTSS_ARCH_DAYTONA
/* Debug PHY read/write */
static void cli_cmd_debug_phy(cli_req_t *req, BOOL write)
{
#ifdef VTSS_CHIP_CU_PHY
    vtss_port_no_t port, port_no;
    int            i, j, addr, addr_count = 0, first = 1;
    u32            reg;
    u16            value;

    char           buf[80];
    
    for (i = 0; i < 2; i++) {
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if (req->port_list[port] == 0)
                continue;

            for (addr = 0; addr < CLI_PHY_MAX; addr++) {
                if (req->addr_list[addr] == 0)
                    continue;
                
                reg = ((req->page << 5) | addr);
                if (i == 0) {
                    /* Count number of addresses to read */
                    addr_count++;
                } else if (write) {
                    /* Write */
                    vtss_phy_write(req->api_inst, port_no, reg, req->value);
                } else {
                    /* Read */
                    vtss_phy_read(req->api_inst, port_no, reg, &value);
                    if (first) {
                        first = 0;
                        if (addr_count > 1 || req->binary) {
                            sprintf(buf, "%s%s",
                                    addr_count > 1 ? "Port  Addr  " : "",
                                    req->binary ? "15******8*7*******0" : "Value");
                            cli_table_header(buf);
                        }
                    }

                    if (addr_count > 1)
                        printf("%-6u0x%02x  ", port, addr);
                    if (req->binary)
                        for (j = 15; j >= 0; j--)
                            printf("%d%s", 
                                   value & (1<<j) ? 1 : 0, (j % 4) || j==0 ? "" : ".");
                    else if (req->decimal)
                        printf("%-5d", value);
                    else 
                        printf("0x%04x", value);
                    printf("\n");

                }
            }
        }
    }
#endif
} 
#endif /* ifndef VTSS_ARCH_DAYTONA */


#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
/* MMD (MDIO Management Devices) read/write */
static void cli_cmd_debug_mmd(cli_req_t *req, BOOL write)
{
    vtss_port_no_t port_no, port;
    int            i, j, mmd, addr_count = 0, first = 1;
    ushort         value;
    
    for (i = 0; i < 2; i++) {
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if (req->port_list[port] == 0)
                continue;
            
            for (mmd = 0; mmd < CLI_PHY_MAX; mmd++) {
                if (req->mmd_list[mmd] == 0)
                    continue;
                
                if (i == 0) {
                    /* Count number of addresses to read */
                    addr_count++;
                } else if (write) {
                    /* Write */
                    vtss_port_mmd_write(req->api_inst, port_no, mmd, req->mmd_addr, req->value);
                } else {
                    /* Read */
                    vtss_port_mmd_read(req->api_inst, port_no, mmd, req->mmd_addr, &value);
                    if (first) {
                        first = 0;
                        if (addr_count > 1)
                            printf("Port  MMD  Addr    ");
                        if (req->binary)
                            printf("15      8 7       0\n");
                        else if (addr_count > 1)
                            printf("Value\n");
                    }

                    if (addr_count > 1)
                        printf("%-6u0x%02x  0x%04x", port, mmd, req->mmd_addr);
                    if (req->binary)
                        for (j = 15; j >= 0; j--)
                            printf("%d%s", 
                                    value & (1<<j) ? 1 : 0, (j % 4) || j==0 ? "" : ".");
                    else if (req->decimal)
                        printf("%6d", value);
                    else 
                        printf("0x%04x", value);
                    printf("\n");
                }
            }
        }
    }
} 
#endif /* VTSS_FEATURE_10G */

#if defined(VTSS_CHIP_10G_PHY)
static void cli_cmd_phy_10g_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_mode_t     mode;
    vtss_phy_10g_id_t       chip_id;
    BOOL               first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;

        if (vtss_phy_10g_mode_get(req->api_inst, port_no, &mode) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }

        if (vtss_phy_10g_id_get(req->api_inst, port_no, &chip_id) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }

        if (first) {
            printf("%-12s %-12s %-12s %-12s %-12s %-12s\n","Port","Mode","WAN/LAN","Wrefclk","Phy Id","Rev.");
            printf("%-12s %-12s %-12s %-12s %-12s %-12s\n","----","----","-------","-------","-------","---");
            first = 0;
        }
       
        printf("%-12u %-12s %-12s %-12s %-12x %-12x\n", port, 
               mode.interface == VTSS_PHY_XAUI_XFI ? "XAUI<->XFI" : "XGMII<->XFI",
               mode.oper_mode == VTSS_PHY_WAN_MODE ? "WAN" : "LAN", 
               mode.oper_mode == VTSS_PHY_WAN_MODE ? (mode.wrefclk == VTSS_WREFCLK_155_52 ? "155.02" : "622.08") : "-", chip_id.part_number, chip_id.revision);

    }
}

static void cli_cmd_phy_10g_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_status_t   status;
    vtss_phy_10g_mode_t     mode;
    BOOL                    first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;
       
        if (vtss_phy_10g_status_get(req->api_inst, port_no, &status) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_10g_status_get() operation");
            continue;
        }
        if (vtss_phy_10g_mode_get(req->api_inst, port_no, &mode) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }

        printf("Port: %u\n", port);
        printf("--------\n");

        if (first) {
            printf("%-12s %-12s %-16s %-12s %-12s\n","","Link","Link-down-event","Rx-Fault","Tx-Fault");
            first = 0;
        }           
        printf("%-12s %-12s %-16s %-12s %-12s\n","PMA:",status.pma.rx_link?"Yes":"No",
               status.pma.link_down?"Yes":"No",status.pma.rx_fault?"Yes":"No",status.pma.tx_fault?"Yes":"No");
        printf("%-12s %-12s %-16s %-12s %-12s\n","WIS:", mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.rx_link?"Yes":"No" : "-",
               mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.link_down?"Yes":"No":"-", mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.rx_fault?"Yes":"No":"-",
               mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.tx_fault?"Yes":"No":"-");
        printf("%-12s %-12s %-16s %-12s %-12s\n","PCS:",status.pcs.rx_link?"Yes":"No",
               status.pcs.link_down?"Yes":"No",status.pcs.rx_fault?"Yes":"No",status.pcs.tx_fault?"Yes":"No");
        printf("%-12s %-12s %-16s %-12s %-12s\n","XS:",status.xs.rx_link?"Yes":"No",
               status.xs.link_down?"Yes":"No",status.xs.rx_fault?"Yes":"No",status.xs.tx_fault?"Yes":"No");
        
    }
}

static void cli_cmd_phy_10g_reset(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
 
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;
        
        if (vtss_phy_10g_reset(req->api_inst, port_no) != VTSS_RC_OK) {
            printf("Could not perform a PHY reset port:%u\n", port_no);
        }
    }
}

static void cli_cmd_phy_10g_loopback(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_loopback_t lb;
    BOOL                    first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;
        
       
        if (req->set) {
            lb.enable = req->enable;
            lb.lb_type = req->lb.lb_type;
            if (vtss_phy_10g_loopback_set(req->api_inst, port_no, &lb) != VTSS_RC_OK) {
                printf("Could not perform vtss_phy_10g_loopback_set() for port %u\n", port_no);
                continue;
            }
        } else {
            if (vtss_phy_10g_loopback_get(req->api_inst, port_no, &lb) != VTSS_RC_OK) {
                printf("Could not perform vtss_phy_10g_loopback_get() for port %u\n", port_no);
                continue;
            }

            if (first) {
                printf("%-12s %-12s\n","Port","Loopback");
                printf("%-12s %-12s\n","----","--------");
                first = 0;
            }           

            printf("%-12u %-12s\n", port, lb.lb_type==VTSS_LB_SYSTEM_XS_SHALLOW?"b":
                   lb.lb_type==VTSS_LB_SYSTEM_XS_DEEP?"c":
                   lb.lb_type==VTSS_LB_SYSTEM_PCS_SHALLOW?"e":
                   lb.lb_type==VTSS_LB_SYSTEM_PCS_DEEP?"e":
                   lb.lb_type==VTSS_LB_SYSTEM_PMA?"j":
                   lb.lb_type==VTSS_LB_NETWORK_XS_SHALLOW?"d":
                   lb.lb_type==VTSS_LB_NETWORK_XS_DEEP?"a":
                   lb.lb_type==VTSS_LB_NETWORK_PCS?"f":
                   lb.lb_type==VTSS_LB_NETWORK_WIS?"h":
                   lb.lb_type==VTSS_LB_NETWORK_PMA?"k":"None");           
        }
    }
}

static void cli_cmd_phy_10g_stats(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_cnt_t cnt;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;
       
        if (vtss_phy_10g_cnt_get(req->api_inst, port_no, &cnt) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_10g_cnt_get() operation");
            continue;
        }

        printf("Port: %u\n", port);
        printf("--------\n");
        printf("PCS:\n");
        printf("%-20s %-12s\n", "  Block_lacthed:",cnt.pcs.block_lock_latched?"Yes":"No");
        printf("%-20s %-12s\n", "  High_ber_latched:",cnt.pcs.high_ber_latched?"Yes":"No");
        printf("%-20s %-12d\n", "  Ber_cnt:",cnt.pcs.ber_cnt);
        printf("%-20s %-12d\n", "  Err_blk_cnt:",cnt.pcs.err_blk_cnt);        
    }
}

static void cli_cmd_phy_10g_power(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_power_t power;
    BOOL            first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) || !vtss_phy_10G_is_valid(NULL, port_no)) 
            continue;
      
        if (req->set) {
            if (req->enable) {
                power = VTSS_PHY_10G_POWER_ENABLE;
            } else {
                power = VTSS_PHY_10G_POWER_DISABLE;
            }
            if (vtss_phy_10g_power_set(req->api_inst, port_no, &power) != VTSS_RC_OK) {
                printf("Could not perform vtss_phy_10g_power_set() for port %u\n", port_no);
                continue;
            }
        } else {
            if (vtss_phy_10g_power_get(req->api_inst, port_no, &power) != VTSS_RC_OK) {
                printf("Could not perform vtss_phy_10g_power_get() for port %u\n", port_no);
                continue;
            }

            if (first) {
                printf("%-12s %-12s\n","Port","Power");
                printf("%-12s %-12s\n","----","-----");
                first = 0;
            }                       
            printf("%-12u %-12s\n", port, power == VTSS_PHY_10G_POWER_ENABLE ? "Enabled":"Disabled");          
        }
    }
}

#endif /* VTSS_CHIP_10G_PHY */

#if defined(VTSS_ARCH_B2)
static const char *cli_rate_txt(vtss_bitrate_t rate, char *buf)
{
    if (rate == VTSS_BITRATE_DISABLED)
        return cli_bool_txt(0);
    sprintf(buf, "%u kbps", rate);
    return buf;
}

/* System information */
static void cli_cmd_system_init(cli_req_t *req)
{
    vtss_init_conf_t conf;
    vtss_host_conf_t host;
    vtss_qs_conf_t   *qs;
    vtss_port_no_t port_no;
    vtss_spi4_conf_t *spi4;
    vtss_xaui_conf_t *xaui;
    char              buf[32];

    if (vtss_appl_init_conf_get(req->appl_inst, &conf) != VTSS_RC_OK)
        return;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) 
        if (vtss_appl_port_is_host(req->appl_inst, port_no))
            break;

    if (vtss_appl_host_conf_get(req->appl_inst, port_no, &host) != VTSS_RC_OK)
        return;
    

    cli_table_header("General");
    printf("Host Mode           : %u\n", conf.host_mode);
    printf("S-tag Ethernet Type : 0x%04x\n", conf.stag_etype); 
    qs = &conf.qs;
    printf("Rx Queue Share      : "); 
    if (qs->rx_share)
        printf("%u %%\n", qs->rx_share);
    else
        printf("Automatic\n");
    printf("Queue System MTU    : %u\n", qs->mtu);
    printf("Queue 6 Reserve     : %s\n", cli_bool_txt(qs->qs_6 == VTSS_QUEUE_STRICT_RESERVE?1:0));
    printf("Queue 7 Reserve     : %s\n", cli_bool_txt(qs->qs_7 == VTSS_QUEUE_STRICT_RESERVE?1:0));
    printf("Queue WRR 0-5 status: q0:%d q1:%d q2:%d q3:%d q4:%d q5:%d\n",
           qs->qw[0]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0, qs->qw[1]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,
           qs->qw[2]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0, qs->qw[3]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,
           qs->qw[4]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0, qs->qw[5]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0);
    printf("Strict Q BW factor  : %d\n", qs->qss_qsp_bwf_pct);
    printf("Rx Queue Mode       : %s\n", 
           qs->rx_mode == VTSS_RX_MODE_SHARED_LOSSLESS ? "Shared Lossless" :
           qs->rx_mode == VTSS_RX_MODE_PORT_FIFO ? "Port FIFO" :
           qs->rx_mode == VTSS_RX_MODE_SHARED_DROP ? "Shared Drop" : "?");
    printf("Tx Queue Mode       : %s\n", 
           qs->tx_mode == VTSS_TX_MODE_SHARED_LOSSLESS ? "Shared Lossless" :
           qs->tx_mode == VTSS_TX_MODE_PORT_FIFO ? "Port FIFO" : "?");
    printf("Scheduler Max Rate  : %u kbps\n\n", conf.sch_max_rate); 

    if (conf.host_mode > 3) {
                
        cli_table_header("SPI4");
        spi4 = &host.spi4;
        printf("Flow Control        : %s\n", cli_bool_txt(spi4->fc.enable));
        printf("Three Level         : %s\n", cli_bool_txt(spi4->fc.three_level_enable));
        printf("Shaper              : %s\n", cli_rate_txt(spi4->qos.shaper.rate, buf));
        printf("IB FCS Mode         : %s\n", 
               spi4->ib.fcs == VTSS_SPI4_FCS_DISABLED ? cli_bool_txt(0) :
               spi4->ib.fcs == VTSS_SPI4_FCS_CHECK ? "Check" :
               spi4->ib.fcs == VTSS_SPI4_FCS_ADD ? "Add" : "?");
        printf("IB Clock Range      : %s MHz\n",
               spi4->ib.clock == VTSS_SPI4_CLOCK_250_TO_290 ? "250-290" :
               spi4->ib.clock == VTSS_SPI4_CLOCK_290_TO_360 ? "290-360" :
               spi4->ib.clock == VTSS_SPI4_CLOCK_360_TO_450 ? "360-450" :
               spi4->ib.clock == VTSS_SPI4_CLOCK_450_TO_500 ? "450-500" : "?");
        printf("IB Data Swap        : %s\n", cli_bool_txt(spi4->ib.data_swap));
        printf("IB Data Invert      : %s\n", cli_bool_txt(spi4->ib.data_invert));
        printf("IB Clock Shift      : %s\n", cli_bool_txt(spi4->ib.clock_shift));
        printf("OB Frame Interleave : %s\n", cli_bool_txt(spi4->ob.frame_interleave));
        printf("OB HIH              : %s\n", cli_bool_txt(spi4->ob.hih_enable));
        printf("OB FCS Strip        : %s\n", cli_bool_txt(spi4->ob.fcs_strip));
        printf("OB HIH Length Update: %s\n", cli_bool_txt(spi4->ob.hih_length_update));
        printf("OB Clock            : %s MHz\n", 
               spi4->ob.clock == VTSS_SPI4_CLOCK_250_0 ? "250.0" :
               spi4->ob.clock == VTSS_SPI4_CLOCK_312_5 ? "312.5" :
               spi4->ob.clock == VTSS_SPI4_CLOCK_375_0 ? "375.0" :
               spi4->ob.clock == VTSS_SPI4_CLOCK_437_5 ? "437.5" :
               spi4->ob.clock == VTSS_SPI4_CLOCK_500_0 ? "500.0" : "?");
        printf("OB Clock Phase      : %s Degrees\n",
               spi4->ob.clock_phase == VTSS_SPI4_CLOCK_PHASE_0 ? "0" :
               spi4->ob.clock_phase == VTSS_SPI4_CLOCK_PHASE_90 ? "90" :
               spi4->ob.clock_phase == VTSS_SPI4_CLOCK_PHASE_180 ? "180" :
               spi4->ob.clock_phase == VTSS_SPI4_CLOCK_PHASE_270 ? "270" : "?");
        printf("OB Data Swap        : %s\n", cli_bool_txt(spi4->ob.data_swap));
        printf("OB Data Invert      : %s\n", cli_bool_txt(spi4->ob.data_invert));
        printf("OB Clock Shift      : %s\n", cli_bool_txt(spi4->ob.clock_shift));
        printf("OB Burst Size       : %s Bytes\n",
               spi4->ob.burst_size == VTSS_SPI4_BURST_64 ? "64" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_96 ? "96" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_128 ? "128" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_160 ? "160" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_192 ? "192" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_224 ? "224" :
               spi4->ob.burst_size == VTSS_SPI4_BURST_256 ? "256" : "?");
        printf("OB MaxBurst1        : %u x 16 Bytes\n", spi4->ob.max_burst_1);
        printf("OB MaxBurst2        : %u x 16 Bytes\n", spi4->ob.max_burst_2);
        printf("OB Link Up Limit    : %u\n", spi4->ob.link_up_limit);
        printf("OB Link Down Limit  : %u\n", spi4->ob.link_down_limit);
        printf("OB Training Mode    : %s\n",
               spi4->ob.training_mode == VTSS_SPI4_TRAINING_DISABLED ? cli_bool_txt(0) :
               spi4->ob.training_mode == VTSS_SPI4_TRAINING_AUTO ? "Auto" :
               spi4->ob.training_mode == VTSS_SPI4_TRAINING_FORCED ? "Forced" :
               spi4->ob.training_mode == VTSS_SPI4_TRAINING_NORMAL ? "Normal" : "?");
        printf("OB ALPHA Value      : %u\n", spi4->ob.alpha);
        printf("OB DATA_MAX_T Value : %u Cycles\n\n", spi4->ob.data_max_t);
    } else {    
        cli_table_header("XAUI");
        xaui = &host.xaui;
        printf("Speed               : %s Gbps\n", "10");
        printf("Channel Flow Contr  : %s\n", cli_bool_txt(xaui->fc.channel_enable));
        printf("In Bound Flow Contr : %s\n", cli_bool_txt(xaui->fc.ib.enable));
        printf("Three Level         : %s\n", cli_bool_txt(xaui->fc.three_level_enable));
        printf("Obey Pause          : %s\n", cli_bool_txt(xaui->fc.obey_pause));
        printf("Status Channel      : %s\n", 
               xaui->status_select == VTSS_XAUI_STATUS_BOTH ? "Both" :
               xaui->status_select == VTSS_XAUI_STATUS_XAUI_0 ? "XAUI_0" : "XAUI_1");
        printf("Mapping Type        : %s\n", xaui->dmac_map_enable ? "DMAC" : "VID");
        printf("Shaper              : %s\n", cli_rate_txt(xaui->qos.shaper.rate, buf));
        printf("HIH Format          : %s\n", 
               xaui->hih.format == VTSS_HIH_POST_SFD ? "After SFD" :
               xaui->hih.format == VTSS_HIH_PRE_SFD ? "Before SFD" : "?");
        printf("HIH Checksum        : %s\n", cli_bool_txt(xaui->hih.cksum_enable));
        printf("IB Clock Shift      : %s\n", cli_bool_txt(xaui->ib.clock_shift));
        printf("OB Clock Shift      : %s\n", cli_bool_txt(xaui->ob.clock_shift));
    }
}

/* B2 specific port configuration */
static void cli_cmd_b2_port_conf(cli_req_t *req, BOOL ct, BOOL pfci) 
{
    vtss_port_no_t          iport;
    vtss_appl_port_conf_t   conf;
    char                    buf[80], *p;
    BOOL                    first = 1;
    
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
      if (req->port_list[iport2uport(iport)] == 0 || 
          vtss_appl_port_conf_get(req->appl_inst, iport, &conf) != VTSS_RC_OK ||
          vtss_appl_port_is_host(req->appl_inst, iport))
          continue;       
        
        if (req->set) {
          T_D("host port conf ct = %d, pfci = %d",ct,pfci);
          if (ct) {
            if (req->rx) {
              conf.ct.rx_ct = req->value;
            } else {
              conf.ct.tx_ct = req->value;
            }
          } 
          if (pfci)
            conf.pfci_enable = req->enable;
          if (vtss_appl_port_conf_set(req->appl_inst, iport, &conf) != VTSS_RC_OK) 
            printf("Could not configure port\n");
        } else {
            if (first) {        
                p = &buf[0];
                p += sprintf(p, "Port  ");
                if (ct) {
                    p += sprintf(p, "Rx Cut-through  ");         
                    p += sprintf(p, "Tx Cut-through");         
                }
                if (pfci)
                    p += sprintf(p, "PFCI");         
                cli_table_header(buf);
                first = 0;
            }
            
            printf("%-6u", iport2uport(iport));
            
            if (ct) {
                if (conf.ct.rx_ct > 0)
                    printf("%-16u", conf.ct.rx_ct);
                else
                    printf("%-16s", "Disabled");

                if (conf.ct.tx_ct > 0)
                    printf("%-16u", conf.ct.tx_ct);
                else
                    printf("%-16s", "Disabled");                
            }
            if (pfci)
                printf("%s", cli_bool_txt(conf.pfci_enable));
            printf("\n");
        }
    }
}

/* Port filter */
static void cli_cmd_port_filter(cli_req_t *req)
{
    vtss_port_no_t          port_no;
    vtss_port_filter_t      filter;
    BOOL                    first = 1;
    char                    tags[5];
    u32                     port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0 || vtss_appl_port_is_host(req->appl_inst, port_no))
            continue;

        if (vtss_port_filter_get(req->api_inst, port_no, &filter) != VTSS_RC_OK) {
            printf("Could not get filter\n");
            return;
        }
        
        if (req->set) {
            if (req->filter.mac_ctrl_enable)
                filter.mac_ctrl_enable = req->allow;
            else if (req->filter.mac_zero_enable)
                filter.mac_zero_enable = req->allow;
            else if (req->filter.dmac_bc_enable)
                filter.dmac_bc_enable = req->allow;
            else if (req->filter.smac_mc_enable)
                filter.smac_mc_enable = req->allow;
            else if (req->filter.untag_enable)
                filter.untag_enable = req->allow;
            else if (req->filter.prio_tag_enable)
                filter.prio_tag_enable = req->allow;
            else if (req->filter.ctag_enable)
                filter.ctag_enable = req->allow;
            else if (req->filter.stag_enable)
                filter.stag_enable = req->allow;
            else if (req->max_tags) 
                filter.max_tags = req->filter.max_tags;
         
            if (vtss_port_filter_set(req->api_inst, port_no, &filter) != VTSS_RC_OK) {
                printf("Could not set filter\n");
            }
        } else {
            if (first) {
                printf("Port mac_ctrl mac_zero dmac_bc smac_mc untag  prio_tag  c_tag  s_tag   tags\n");
                first = 0;
            }
            if (filter.max_tags == VTSS_TAG_ONE) {
                strcpy(tags,"1");
            } else if (filter.max_tags == VTSS_TAG_TWO) {
                strcpy(tags,"2");
            } else {
                strcpy(tags,"Any");
            }
            printf("%-8u", port);
            printf("%-8s",filter.mac_ctrl_enable?"Allow":"Deny");
            printf("%-8s",filter.mac_zero_enable?"Allow":"Deny");
            printf("%-8s",filter.dmac_bc_enable?"Allow":"Deny");
            printf("%-8s",filter.smac_mc_enable?"Allow":"Deny");
            printf("%-8s",filter.untag_enable?"Allow":"Deny");
            printf("%-8s",filter.prio_tag_enable?"Allow":"Deny");
            printf("%-8s",filter.ctag_enable?"Allow":"Deny");
            printf("%-8s",filter.stag_enable?"Allow":"Deny");
            printf("%-8s",tags);
            printf("\n");

        }
    }
}

static void cli_cmd_port_forward(cli_req_t *req)
{
    vtss_port_no_t      iport;
    vtss_port_forward_t fwd;
    int                 first = 1;
    
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
      if (req->port_list[iport2uport(iport)] == 0 || vtss_appl_port_is_host(req->appl_inst, iport))
        continue;

      if (req->set) {
        fwd = (req->enable ? VTSS_PORT_FORWARD_ENABLED :
               req->disable ? VTSS_PORT_FORWARD_DISABLED :
               req->ingress ? VTSS_PORT_FORWARD_INGRESS :
               VTSS_PORT_FORWARD_EGRESS);
        if (vtss_port_forward_state_set(req->api_inst,iport, fwd) != VTSS_RC_OK) {
          T_E("Forwarding state was not configured correctly");
        };
        T_D("Set iport = %u, fwd = %d", iport,fwd);
      } else {
        vtss_port_forward_state_get(req->api_inst,iport, &fwd);
        T_D("Get iport = %u, fwd = %d", iport,fwd);
        if (first) {
          cli_table_header("Port  Forwarding");
          first = 0;
        }
        printf("%-4u  %s\n", iport2uport(iport), 
               fwd == VTSS_PORT_FORWARD_ENABLED ? "Enabled" :
               fwd == VTSS_PORT_FORWARD_DISABLED ? "Disabled" :
               fwd == VTSS_PORT_FORWARD_INGRESS ? "Ingress" : "Egress");
      }
    }
}

// Port status interface clock frequecy configuration
static void cli_cmd_psi_clock(cli_req_t *req)
{
  u32 psi_clock;
    
  if (req->set)
    vtss_port_status_interface_set(req->api_inst, req->value);
  else if (vtss_port_status_interface_get(req->api_inst, &psi_clock) == VTSS_RC_OK)
    printf("PSI clock: %u\n", psi_clock);
}

static void cli_b2_tgt_name(u32 tgt, char *buf)
{
    if (tgt >= 0x20 && tgt <= 0x37)
        sprintf(buf, "%s_%u", "DEV1G", tgt - 0x20);
    else
        strcpy(buf, 
               tgt == 0x00 ? "DEVCPU_ORG" :
               tgt == 0x01 ? "DEVCPU_GCB" :
               tgt == 0x02 ? "ASM" :
               tgt == 0x03 ? "DSM" :
               tgt == 0x04 ? "ANA_CL" :
               tgt == 0x05 ? "ANA_AC" :
               tgt == 0x06 ? "SCH" :
               tgt == 0x07 ? "QSS" :
               tgt == 0x0c ? "DEVSPI" :
               tgt == 0x38 ? "DEV10G_0" :
               tgt == 0x39 ? "SPAUI_STAT_0" :
               tgt == 0x3a ? "DEV10G_1" :
               tgt == 0x3b ? "SPAUI_STAT_1" :
               tgt == 0x3f ? "FAST_REGS" : "?");
}

static void cli_cmd_debug_targets(void)
{
    u32  tgt;
    char buf[32];
    
    cli_table_header("ID    Name");
    for (tgt = 0; tgt < 64; tgt++) {
        cli_b2_tgt_name(tgt, buf);
        if (strlen(buf) < 2)
            continue;
        printf("0x%02x  %s\n", tgt, buf);
    }
}

static u32 cli_reg_base(u32 tgt, u32 port)
{
    u32 reg;
    
    if (tgt == 0x20)
        reg = (tgt + port);
    else if (tgt == 0x38 || tgt == 0x39)
        reg = (tgt + (port - 24)*2);
    else
        reg = tgt;
    return (reg << 16);
}

/* Show B2 register */
static void cli_b2_reg_show(u32 tgt, u32 addr, u32 addr_count, u32 addr_offset, const char *name)
{
    u32  base, reg, value, i;
    char buf[32];
    
    cli_b2_tgt_name(tgt, buf);
    base = cli_reg_base(tgt, 0);
    for (i = 0; i < addr_count; i++) {
        reg = (addr + i*addr_offset);
        if (vtss_reg_read(cli_board->inst, 0, base + reg, &value) != VTSS_RC_OK)
            continue;
        printf("0x%02x-0x%04x: 0x%08x  %s", tgt, reg, value, buf);
        if (name != NULL) {
            printf("-%s", name);
            if (addr_count > 1)
                printf("[%u]", i);
        }
        printf("\n");
    }
}

/* Show QSS registers */
static void cli_reg_qss_show(u32 addr, u32 addr_count, const char *name)
{
    cli_b2_reg_show(0x07, addr, addr_count, 1, name);
}

static void cli_cmd_debug_qss(cli_req_t *req)
{
    cli_reg_qss_show(0x100, 96, "APORT_ENA_CLASS_MAP");
    cli_reg_qss_show(0x300, 1, "RX_TOTAL_SIZE");
    cli_reg_qss_show(0x301, 1, "TX_TOTAL_SIZE");
    cli_reg_qss_show(0x400, 2, "RX_GBL_ENA");
    cli_reg_qss_show(0x402, 2, "RX_GBL_SHMAX");
    cli_reg_qss_show(0x404, 2, "RX_GBL_DROP_HLTH");
    cli_reg_qss_show(0x406, 2, "RX_GBL_FC_HLTH");
    cli_reg_qss_show(0x500, 2, "TX_GBL_ENA");
    cli_reg_qss_show(0x502, 2, "TX_GBL_SHMAX");
    cli_reg_qss_show(0x504, 2, "TX_GBL_DROP_HLTH");
    cli_reg_qss_show(0x506, 2, "TX_GBL_FC_HLTH");
    cli_reg_qss_show(0x600, 96, "PORT_ENA");
    cli_reg_qss_show(0x660, 96, "PORT_RSVD_GFC_LTH");
    cli_reg_qss_show(0x6c0, 96, "PORT_SHMAX");
    cli_reg_qss_show(0x720, 96, "PORT_DROP_HLTH");
    cli_reg_qss_show(0x780, 96, "PORT_FC_HLTH");
    cli_reg_qss_show(0x7e0, 96, "PORT_PRE_ALLOC");
    cli_reg_qss_show(0xef0, 240, "Q_RSVD");
    cli_reg_qss_show(0xfe0, 240, "Q_MAX");
    cli_reg_qss_show(0x10d0, 240, "Q_DROP_HLTH");
}

static void cli_cmd_debug_qss_bufcnt(cli_req_t *req)
{

    int port_no, addr, q, base;
    u32 value;

    base = cli_reg_base(0x07, 0);
  
    printf("Global watermarks:\n");
    printf("-----------------\n");
    /* Shared Rx/Tx QSS */
    addr = 0x402;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Rx global max (RX_GBL_SHMAX)                    0x%x (%u bytes)\n", value, value * 128);
    addr = 0x404;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Rx global drop (RX_GBL_DROP_HLTH)               low:0x%x  high:0x%x\n", value & 0x1fff, value >> 16);
    addr = 0x406;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Rx global flow ctrl thresh. (RX_GBL_FC_HLTH)    low:0x%x  high:0x%x\n", value & 0x1fff, value >> 16);    
    addr = 0x502;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Tx global max (TX_GBL_SHMAX)                    0x%x (%u bytes)\n", value, value * 128);
    addr = 0x504;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Tx global drop (TX_GBL_DROP_HLTH)               low:0x%x  high:0x%x\n", value & 0x1fff, value >> 16);
    addr = 0x506;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Tx global flow ctrl thresh. (TX_GBL_FC_HLTH)    low:0x%x  high:0x%x\n\n", value & 0x1fff, value >> 16);    

    printf("Port watermarks:\n");
    printf("-----------------\n");
    /* Port QSS */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->port_list[port_no] == 0)
            continue;

        addr = 0x6c0;
        addr = addr+(port_no-1);
        if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
            printf("Port max threshold (PORT_SHMAX)              0x%x (0x%u bytes)\n", value, value * 128);    
        
        addr = 0x720;
        addr = addr+(port_no-1);
        if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
            printf("Port drop threshold (PORT_DROP_HLTH)         low:0x%x  high:0x%x\n", value & 0x1fff, value >> 16);

        addr = 0x780;
        addr = addr+(port_no-1);
        if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
            printf("Port flow control threshold (PORT_FC_HLTH)   low:0x%x  high:0x%x\n", value & 0x1fff, value >> 16);

    }
    printf("\n");
   
    printf("Buffer fillings:\n");
    printf("-----------------\n");
    /* Buffer dynamic fillings   */

    addr = 0x303;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Global Rx buffer (RX_SHCNT):   0x%x (%u bytes)\n", value, value * 128);
    addr = 0x305;
    if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
        printf("Global Tx buffer (TX_SHCNT):   0x%x (%u bytes)\n",value,value * 128);        
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->port_list[port_no] == 0)
            continue;

        addr = (0x8a0 + (port_no-1));
        if (vtss_reg_read(req->api_inst, 0, base+addr, &value) == VTSS_RC_OK)
            printf("Tx port %d buffer:         0x%x (%u bytes)\n", port_no, value, value * 128);
        
        printf("Rx port %d queues:\n",port_no);
        addr = (0x11c0 + (port_no-1));
        for (q = 0; q < 8; q++) {
            if (vtss_reg_read(req->api_inst, 0, base+addr+q, &value) == VTSS_RC_OK) {
                printf("  Queue %d:  0x%x (%u bytes)\n", q, value & 0x1fff, ((value & 0x1fff) * 128));
            }
        }       
    }
}


static void cli_cmd_debug_red_cnt(cli_req_t *req)
{
    int i,indx,port_no;

    req->blk_list[7] = 1; /* QSS target */

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->port_list[port_no] == 0)
            continue;

        indx = 0x1670 + (VTSS_QUEUES * (port_no-1));            
        for (i = 0; i < CLI_ADDR_MAX; i++) {
            if (i >= indx && i <= indx+8) 
                req->addr_list[i] = 1;
        }        
        printf ("Port %d queues red drop cnt 1-8:\n",port_no);
        cli_cmd_debug_reg(req, 0);
    }

}

#endif /* VTSS_ARCH_B2 */

#if defined(BOARD_B2_EVAL)
static void cli_cmd_debug_fpga(cli_req_t *req, BOOL write)
{
    int i, j, addr, addr_count = 0, first = 1;
    u32 value;
    
    for (i = 0; i < (write ? 1 : 2); i++) {
        for (addr = 0; addr < 256; addr++) {
            if (req->addr_list[addr] == 0)
                continue;
            
            if (write) /* Write */
                vtss_rabbit_rd_wr_fpga(addr, &req->value, 0);
            else if (i == 0) /* Count number of addresses to read */
                addr_count++;
            else {
                /* Read */
                vtss_rabbit_rd_wr_fpga(addr, &value, 1);
                if (first && req->binary) {
                    if (addr_count > 1)
                        printf("      ");
                    printf("31     24 23     16 15      8 7       0\n");
                    first = 0;
                }
                if (addr_count > 1)
                    printf("0x%02x: ", addr);
                if (req->binary)
                    for (j = 31; j >= 0; j--)
                        printf("%d%s", 
                               value & (1<<j) ? 1 : 0, (j % 4) || j==0 ? "" : ".");
                else if (req->decimal)
                    printf("%10u", value);
                else 
                    printf("0x%08x", value);
                printf("\n");
            }
        }
    }
}

/* FPGA status */
static void cli_cmd_debug_fpga_status(cli_req_t *req)
{
    u32 val;
    int j;
 
    /* Get interrupt status             */
    vtss_rabbit_rd_wr_fpga(0x14, &val, 1);
    printf("External Interrupt pin (INTR_N) status:%s\n",(val & (1<<2))?"Low":"High");

    /* Read GPIO In status  */
    vtss_rabbit_rd_wr_fpga(0x50, &val, 1);
    printf("External GPIO pins status:\n");
    printf("15      8 7       0\n");
    for (j = 15; j >= 0; j--) 
        printf("%d%s", val & (1<<j) ? 1 : 0, (j % 4) || j==0 ? "" : ".");    
    printf("\n");
    
} 
#endif /* BOARD_B2_EVAL */


#if defined(VTSS_GPIOS)
static void cli_cmd_debug_gpio(cli_req_t *req, cli_cmd_id_t cmd)
{
    vtss_gpio_no_t gpio;
    BOOL           first = 1, value;
    
    for (gpio = VTSS_GPIO_NO_START; gpio < VTSS_GPIO_NO_END; gpio++) {
        if (req->gpio_list[gpio] == 0)
            continue;
        if (cmd == CLI_CMD_ID_DEBUG_GPIO_OUTPUT)

            vtss_gpio_direction_set(req->api_inst, 0, gpio, req->set ? req->enable : 1);
        else if (cmd == CLI_CMD_ID_DEBUG_GPIO_WRITE)
            vtss_gpio_write(req->api_inst, 0, gpio, req->value);
        else if (vtss_gpio_read(req->api_inst, 0, gpio, &value) == VTSS_RC_OK) {
            if (first)
                cli_table_header("GPIO  Value");
            first = 0;
            printf("%-4u  %u\n", gpio, value);
        }
    }
}

#if defined(VTSS_ARCH_B2)
/* Setup recovered clock  */
static void cli_cmd_clk_gpio(cli_req_t *req, vtss_port_no_t port_no)
{
    vtss_recovered_clock_t clk;
    
    if (port_no == VTSS_PORT_NO_NONE) {
        if (vtss_gpio_clk_set(req->api_inst, req->gpio_no, VTSS_PORT_NO_NONE, VTSS_RECOVERED_CLOCK_25) != VTSS_RC_OK) {
            printf("Could not disable gpio:%u\n",req->gpio_no);
        }
        return;
    }
   
    if (req->clk_freq == 1)
        clk = VTSS_RECOVERED_CLOCK_25;
    else if (req->clk_freq == 2)
        clk = VTSS_RECOVERED_CLOCK_31_25;
    else if (req->clk_freq == 3)
        clk = VTSS_RECOVERED_CLOCK_39_06;
    else if (req->clk_freq == 4)
        clk = VTSS_RECOVERED_CLOCK_62_5;
    else if (req->clk_freq == 5)
        clk = VTSS_RECOVERED_CLOCK_78_125;
    else if (req->clk_freq == 6)
        clk = VTSS_RECOVERED_CLOCK_125;
    else if (req->clk_freq == 7)
        clk = VTSS_RECOVERED_CLOCK_156_25;
    else {
        printf("Illegal clock value\n");
        return;
    }
        
    if (vtss_gpio_clk_set(req->api_inst, req->gpio_no, port_no, clk) != VTSS_RC_OK) {
        printf("Could not set GPIO clk\n");
        return;
    }
    
}
#endif /* VTSS_ARCH_B2 */
#endif /* VTSS_GPIOS */

#if defined(VTSS_ARCH_B2)
static void cli_sticky_check(cli_req_t *req, u32 offset, const char *name)
{
    if (req->value & (1<<offset))
        printf("Error at target 0x%02x, address 0x%04x: %s_sticky set\n", 
               (req->addr >> 16) & 0xff, req->addr & 0xffff, name);
}

static void cli_sticky_check_tgt(cli_req_t *req, u32 offset, const char *name)
{
    if (req->value & (1<<offset))
        printf("Error at target 0x%02x, address 0x%04x: %s_sticky set, target: %u (0x%02x)\n", 
               (req->addr >> 16) & 0xff, req->addr & 0xffff, name, 
               req->value & 0xff, req->value & 0xff);
}

/* Read and optionally clear sticky register */
static int cli_reg_read_clear(cli_req_t *req, u32 addr)
{
    u32 value;
    
    if (vtss_reg_read(req->api_inst, 0, addr, &value) != VTSS_RC_OK)
        return 0;
    if (req->clear)
        vtss_reg_write(req->api_inst, 0, addr, value);
    req->addr = addr;
    req->value = value;
    return 1;
}

/* Read, check and optionally clear sticky bits */
static void cli_cmd_debug_reg_check(cli_req_t *req)
{
    u32  port, base;
    int  i;
    char name[80];
    
    for (port = 0; port <= CLI_PORT_MAX; port++) {
        if (!req->chip_port_list[port])
            continue;

        printf("Checking port: %u %s\n", port, port == 26 ? "(SPI4)" : "");
        if (port < 24) {
            /* 1G port */
            base = cli_reg_base(0x20, port);
            
            /* DEV1G::DEV_STICKY */
            if (cli_reg_read_clear(req, base + 0x01)) {
                cli_sticky_check(req, 10, "rx_oflw");
                cli_sticky_check(req, 1, "tx_oflw");
                cli_sticky_check(req, 0, "tx_uflw");
            }

            /* DEV1G::PCS1G_STICKY */
            if (cli_reg_read_clear(req, base + 0x24))
                cli_sticky_check(req, 0, "out_of_sync");
            
#if 0
            /* DEV1G::FX100_RX_STAT */
            if (cli_reg_read_clear(req, base + 0x07)) {
                cli_sticky_check(req, 9, "esd_err");
                cli_sticky_check(req, 8, "ssd_err");
                cli_sticky_check(req, 7, "rcv_err");
                cli_sticky_check(req, 6, "remote_xmit_err");
                cli_sticky_check(req, 2, "lock_err");
            }

            /* DEV1G::FX100_EYE_LOGIC_STATUS */
            if (cli_reg_read_clear(req, base + 0x10)) {
                cli_sticky_check(req, 20, "transitions_crossed");
                cli_sticky_check(req, 19, "transitions_too_close");
                cli_sticky_check(req, 18, "lower_transition_too_high");
                cli_sticky_check(req, 10, "lower_bounds_exceeded");
                cli_sticky_check(req, 8, "upper_transition_too_low");
                cli_sticky_check(req, 1, "upper_bounds_exceeded");
            }
#endif
        } else if (port < 26) {
            /* 10G port */
            base = cli_reg_base(0x38, port);

#if 0
            /* DEV10G::MAC_TX_MONITOR_STICKY */
            if (cli_reg_read_clear(req, base + 0x18)) {
                cli_sticky_check(req, 4, "local_err_state");
                cli_sticky_check(req, 3, "remote_err_state");
            }
#endif
            /* DEV10G::MAC_STICKY */
            if (cli_reg_read_clear(req, base + 0x19)) {
                cli_sticky_check(req, 1, "tx_uflw");
                cli_sticky_check(req, 0, "tx_abort");
            }

            /* DEV10G::DEV_STICKY */
            if (cli_reg_read_clear(req, base + 0x6b)) {
                cli_sticky_check(req, 10, "rx_resync_fifo_oflw");
                cli_sticky_check(req, 9, "rx_taxi_fifo_oflw");
                cli_sticky_check(req, 1, "tx_fifo_oflw");
                cli_sticky_check(req, 0, "tx_fifo_uflw");
            }
            
            /* XAUI_PHY_STAT::XAUI_OBS_STICKY */
            base = cli_reg_base(0x39, port);
            if (cli_reg_read_clear(req, base + 0xb5))
                cli_sticky_check(req, 1, "obs_dip2_err");
            
        } else if (port == 26) {
            /* SPI4 */
            base = cli_reg_base(0x0c, 0);

            /* DEVSPI::SPI4_IB_STICKY */
            if (cli_reg_read_clear(req, base + 0x08)) {
                cli_sticky_check(req, 25, "ib_invalid_port_num");
                cli_sticky_check(req, 23, "ib_usize_pkt");
                cli_sticky_check(req, 21, "ib_dip4_err");
                cli_sticky_check(req, 20, "ib_ram_error");
                cli_sticky_check(req, 19, "ib_buf_ovfl");
                cli_sticky_check(req, 9, "ib_eopa_word");
                cli_sticky_check(req, 5, "ib_unkn_data_word");
                cli_sticky_check(req, 3, "ib_unkn_ctrl_word");
                cli_sticky_check(req, 2, "ib_ext_ctrl_word");
            }

            /* DEVSPI::SPI4_OB_STICKY */
            if (cli_reg_read_clear(req, base + 0x0f)) {
                cli_sticky_check(req, 20, "ob_ram_error");
                cli_sticky_check(req, 19, "ob_buf_ovfl");
                cli_sticky_check(req, 9, "ob_eopa_word");
                cli_sticky_check(req, 5, "ob_unkn_data_word");
                cli_sticky_check(req, 3, "ob_unkn_ctrl_word");
                cli_sticky_check(req, 2, "ob_ext_ctrl_word");
            }

            /* DEVSPI::SPI4_OBS_STICKY */
            if (cli_reg_read_clear(req, base + 0x98)) {
                cli_sticky_check(req, 1, "obs_dip2_err");
                cli_sticky_check(req, 0, "obs_idle_mode");
            }

            /* DEVSPI::SPI4_IB_TC_STICKY */
            if (cli_reg_read_clear(req, base + 0xa7)) {
                for (i = 0; i < 17; i++) {
                    sprintf(name, "ib_tc_lane%u_err", i);
                    cli_sticky_check(req, i, name);
                }
            }
        }
    }
    
    printf("Checking ASM\n");
    base = cli_reg_base(0x02, 0);

    /* ASM::SPI_ERR_STICKY */
    if (cli_reg_read_clear(req, base + 0x169f)) {
        cli_sticky_check(req, 11, "fc_oflw");
        cli_sticky_check(req, 8, "cc_oflw");
        cli_sticky_check(req, 0, "main_sm_oflw");
    }
    
    /* ASM::NON_SPI_ERR_STICKY */
    if (cli_reg_read_clear(req, base + 0x16a0)) {
        cli_sticky_check(req, 16, "hih");
        for (i = 0; i < 4; i++) {
            sprintf(name, "fifo_oflw_%u", i);
            cli_sticky_check(req, i, name);
        }
    }

    /* ASM::CSC_STICKY */
    if (cli_reg_read_clear(req, base + 0x16a7))
        cli_sticky_check(req, 2, "csc_soft");

    printf("Checking DEVCPU_ORG\n");
    base = cli_reg_base(0x00, 0);

    /* DEVCPU_ORG::ERR_ACCESS_DROP_1 */
    if (cli_reg_read_clear(req, base + 0x00))
        cli_sticky_check_tgt(req, 8, "utm");
    
    /* DEVCPU_ORG::ERR_ACCESS_DROP_2 */
    if (cli_reg_read_clear(req, base + 0x01))
        cli_sticky_check_tgt(req, 9, "no_action");
    
    /* DEVCPU_ORG::ERR_TGT_MSB */
    if (cli_reg_read_clear(req, base + 0x03))
        cli_sticky_check_tgt(req, 8, "bsy");
    
    /* DEVCPU_ORG::ERR_TGT_LSB */
    if (cli_reg_read_clear(req, base + 0x04))
        cli_sticky_check_tgt(req, 9, "wd");
    
    /* DEVCPU_ORG::CFG_STATUS */
    if (cli_reg_read_clear(req, base + 0x0d))
        cli_sticky_check(req, 1, "rd_err");

    printf("Checking DSM\n");
    base = cli_reg_base(0x03, 0);

    /* DSM::FCS_STATUS */
    if (cli_reg_read_clear(req, base + 0x0168))
        cli_sticky_check(req, 0, "fcs_err");
    
    /* DSM::FCS_STATUS */
    if (cli_reg_read_clear(req, base + 0x0168))
        cli_sticky_check(req, 0, "fcs_err");
    
    /* DSM::IFH_STATUS */
    if (cli_reg_read_clear(req, base + 0x0169))
        cli_sticky_check(req, 0, "ifh_bip8_err");
    
    /* DSM::CELL_BUS_STICKY */
    if (cli_reg_read_clear(req, base + 0x01b7)) {
        cli_sticky_check(req, 3, "cell_bus_illegal_port_num");
        cli_sticky_check(req, 2, "cons_cell_for_same_taxi");
        cli_sticky_check(req, 1, "cell_bus_missing_sof");
        cli_sticky_check(req, 0, "cell_bus_missing_eof");
    }

    /* DSM::BUF_OFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01ba)) {
        for (i = 0; i < 26; i++) {
            sprintf(name, "buf_oflw_%u", i);
            cli_sticky_check(req, i, name);
        }
    }
    
    /* DSM::BUF_UFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01bb)) {
        for (i = 0; i < 26; i++) {
            sprintf(name, "buf_uflw_%u", i);
            cli_sticky_check(req, i, name);
        }
    }
    
    /* DSM::SPI4_LO_CH_BUF_OFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01bc)) {
        for (i = 0; i < 24; i++) {
            sprintf(name, "spi4_ch_buf_oflw_%u", i + 48);
            cli_sticky_check(req, i, name);
        }
    }
    
    /* DSM::SPI4_HI_CH_BUF_OFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01bd)) {
        for (i = 0; i < 24; i++) {
            sprintf(name, "spi4_ch_buf_oflw_%u", i + 72);
            cli_sticky_check(req, i, name);
        }
    }
    
    /* DSM::SPI4_LO_CTXT_BUF_OFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01c0)) {
        for (i = 0; i < 24; i++) {
            sprintf(name, "spi4_ctxt_buf_oflw_%u", i + 48);
            cli_sticky_check(req, i, name);
        }
    }
    
    /* DSM::SPI4_HI_CTXT_BUF_OFLW_STICKY */
    if (cli_reg_read_clear(req, base + 0x01c1)) {
        for (i = 0; i < 24; i++) {
            sprintf(name, "spi4_ctxt_buf_oflw_%u", i + 72);
            cli_sticky_check(req, i, name);
        }
    }

    /* DSM::RAM_PARITY_ERR_STICKY */
    if (cli_reg_read_clear(req, base + 0x01c2)) {
        for (i = 0; i < 7; i++) {
            sprintf(name, "ram_parity_err_%u", i);
            cli_sticky_check(req, i, name);
        }
    }

    printf("Checking ANA_CL\n");
    base = cli_reg_base(0x04, 0);

    /* ANA_CL::SOFT_STICKY */
    if (cli_reg_read_clear(req, base + 0x133c)) {
        cli_sticky_check(req, 1, "port_cfg_soft");
        cli_sticky_check(req, 0, "endpt_cfg_soft");
    }

    /* ANA_CL::TAG_AND_LBL_STICKY */
    if (cli_reg_read_clear(req, base + 0x1340)) {
        cli_sticky_check(req, 10, "filter_max_tag");
        cli_sticky_check(req, 9, "filter_single_service");
        cli_sticky_check(req, 8, "filter_prio_tagged");
        cli_sticky_check(req, 7, "filter_tagged");
        cli_sticky_check(req, 6, "filter_untagged");
    }

    /* ANA_CL::DETECTION_STICKY */
    if (cli_reg_read_clear(req, base + 0x1341)) {
        cli_sticky_check(req, 6, "filter_global_custom");
        cli_sticky_check(req, 5, "mac_ctrl");
        cli_sticky_check(req, 4, "filter_l2_bc");
        cli_sticky_check(req, 3, "filter_etype_or_mac");
        cli_sticky_check(req, 2, "bad_macs");
        cli_sticky_check(req, 1, "smac_mc");
        cli_sticky_check(req, 0, "filter_local_custom");
    }

    printf("Checking ANA_AC\n");
    base = cli_reg_base(0x05, 0);
    
    /* ANA_AC::SOFT_STICKY */
    if (cli_reg_read_clear(req, base + 0x0501)) {
        cli_sticky_check(req, 8, "lb_cfg_soft");
        cli_sticky_check(req, 7, "prio_stat_info_soft");
        cli_sticky_check(req, 6, "port_stat_info_soft");
        cli_sticky_check(req, 5, "prio_stat_cnt_soft");
        cli_sticky_check(req, 4, "port_stat_cnt_soft");
        cli_sticky_check(req, 3, "port_lb_cnt_soft");
        cli_sticky_check(req, 2, "port_lb_cfg_soft");
        cli_sticky_check(req, 1, "prio_lb_cnt_soft");
        cli_sticky_check(req, 0, "prio_lb_cfg_soft");
    }

    printf("Checking QSS\n");
    base = cli_reg_base(0x07, 0);

    /* QSS::QSS_STICKY */
    if (cli_reg_read_clear(req, base + 0x02)) {
        cli_sticky_check(req, 11, "ariv_disabled_brm");
        cli_sticky_check(req, 10, "rx_discard_class_1");
        cli_sticky_check(req, 9, "rx_discard_class_0");
        cli_sticky_check(req, 8, "tx_discard_class_1");
        cli_sticky_check(req, 7, "tx_discard_class_0");
        cli_sticky_check(req, 6, "flist_uflw");
        cli_sticky_check(req, 5, "flist_empty");
        cli_sticky_check(req, 4, "sch_req_err");
        cli_sticky_check(req, 3, "frm_trunc");
        cli_sticky_check(req, 2, "ariv_max_frm_len");
        cli_sticky_check(req, 1, "ariv_cell_seq_err");
        cli_sticky_check(req, 0, "aport_disabled");
    }    

    /* QSS::FRM_UFLW_PORTS_0_31_STICKY */
    if (cli_reg_read_clear(req, base + 0x03)) {
        for (i = 0; i < 32; i++) {
            sprintf(name, "frm_uflw_port_%u", i);
            cli_sticky_check(req, i, name);
        }
    }    

    /* QSS::FRM_UFLW_PORTS_32_63_STICKY */
    if (cli_reg_read_clear(req, base + 0x04)) {
        for (i = 0; i < 32; i++) {
            sprintf(name, "frm_uflw_port_%u", i + 32);
            cli_sticky_check(req, i, name);
        }
    }    

    /* QSS::FRM_UFLW_PORTS_64_95_STICKY */
    if (cli_reg_read_clear(req, base + 0x05)) {
        for (i = 0; i < 32; i++) {
            sprintf(name, "frm_uflw_port_%u", i + 64);
            cli_sticky_check(req, i, name);
        }
    }    

    /* QSS::RAM_PTY_ERR_STICKY */
    if (cli_reg_read_clear(req, base + 0x05)) {
        for (i = 0; i < 16; i++) {
            sprintf(name, "ram_pty_err_%u", i);
            cli_sticky_check(req, i, name);
        }
    }    
} 

static void cli_cmd_host_stats(cli_req_t *req, const char *col1, const char *col2, u32 reg1, u32 reg2)
{
    u64 c1, c2;
    u32     val;

    if (req->clear) {
        /* Clear counters */
        if (col1 != NULL)
            vtss_reg_write(req->api_inst, 0, reg1, 0);
        if (col2 != NULL)
            vtss_reg_write(req->api_inst, 0, reg2, 0);
    } else {
        /* Show counters */
        c1 = ((col1 != NULL && vtss_reg_read(req->api_inst, 0, reg1, &val)) == VTSS_RC_OK ? val : 0);
        c2 = ((col2 != NULL && vtss_reg_read(req->api_inst, 0, reg2, &val)) == VTSS_RC_OK ? val : 0);
        cli_cmd_stats(col1, col2, c1, c2);
    }
}

static void cli_cmd_spi4_stats(cli_req_t *req, const char *col1, const char *col2, u32 reg1, u32 reg2)
{
    u32 base;

    base = cli_reg_base(0x0c, 0);
    cli_cmd_host_stats(req, col1, col2, reg1 + base, reg2 + base);
}

static void cli_cmd_debug_spi4(cli_req_t *req)
{
    cli_cmd_spi4_stats(req, "Packets", "", 0x04, 0x0c);
    cli_cmd_spi4_stats(req, "Bytes", "", 0x06, 0x0e);
    cli_cmd_spi4_stats(req, "EOPAs", "", 0x05, 0x0d);
    cli_cmd_spi4_stats(req, "DIP-4 Errors", "DIP-2 Errors", 0x03, 0x97);
}
#endif /* VTSS_ARCH_B2 */


/* Debug suspend/resume */
static void cli_cmd_debug_suspend(BOOL suspend)
{
    if (suspend != suspended) {
        suspended = suspend;
        printf("Application %s\n", suspend ? "suspended" : "resumed");
    }
}

static void cli_cmd_debug_trace(cli_req_t *req)
{
    vtss_trace_layer_t layer;
    vtss_trace_group_t group;
    vtss_trace_conf_t  conf;
    vtss_trace_level_t level;
    int                first = 1;
    
    for (layer = 0; layer < VTSS_TRACE_LAYER_COUNT; layer++) {
        if (req->trace_layer != VTSS_TRACE_LAYER_COUNT && layer != req->trace_layer)
            continue;
        
        for (group = 0; group <= VTSS_TRACE_GROUP_COUNT; group++) {
            if (req->group != CLI_TRACE_GROUP_ALL && group != req->group)
                continue;
            
            if (group > VTSS_TRACE_GROUP_L2 && group != VTSS_TRACE_GROUP_COUNT)
                continue;
            
            if (group == VTSS_APPL_TRACE_GROUP)
                conf = vtss_appl_trace_conf;
            else
                vtss_trace_conf_get(group, &conf);
            
            if (req->set) {
                conf.level[layer] = req->level;
                if (group == VTSS_APPL_TRACE_GROUP)
                    vtss_appl_trace_conf = conf;
                else
                    vtss_trace_conf_set(group, &conf);
            } else {
                if (first) {
                    cli_table_header("Layer  Group    Level");
                    first = 0;
                } 
                level = conf.level[layer];
                printf("%-7s%-9s%s\n",
                       layer == VTSS_TRACE_LAYER_AIL ? "ail" : "cil", 
                       group == VTSS_TRACE_GROUP_COUNT ? "main" :
                       group == VTSS_TRACE_GROUP_DEFAULT ? "default" :
                       group == VTSS_TRACE_GROUP_PORT ? "port" :
                       group == VTSS_TRACE_GROUP_PHY ? "phy" :
                       group == VTSS_TRACE_GROUP_PACKET ? "packet" : 
                       group == VTSS_TRACE_GROUP_QOS ? "qos" : 
                       group == VTSS_TRACE_GROUP_L2 ? "l2" : "?",
                       level == VTSS_TRACE_LEVEL_NONE ? "off" :
                       level == VTSS_TRACE_LEVEL_ERROR ? "error" :
                       level == VTSS_TRACE_LEVEL_INFO ? "info" :
                       level == VTSS_TRACE_LEVEL_DEBUG ? "debug" :
                       level == VTSS_TRACE_LEVEL_NOISE ? "noise" : "?");
            }
        }
    }
}

#if defined(VTSS_FEATURE_WIS)
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
static void cli_cmd_wis_reset(cli_req_t *req)
{
    vtss_port_no_t port_no, port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;

        if (req->set) {
            if (vtss_ewis_reset(NULL, port_no) != VTSS_RC_OK) {
                printf("WIS block reset failed[vtss_ewis_reset()] for port %u\n",port);
                continue;
            }
        }
    }
}

static void cli_cmd_wis_test_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_test_status_t test_status;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_test_counter_get(req->api_inst, port_no, &test_status) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_test_counter_get(): %u operation\n",port);
            continue;
        }

        printf("WIS Test Status Port: %u\n",port);
        printf("------------------------\n");
        printf("%-30s %-12d\n", "  PRBS31 test pattern error counter:",test_status.tstpat_cnt);
        printf("%-30s %-12s\n", "  PRBS31 test Analyzer status      :",(test_status.ana_sync)? "Sync" : "Not Synced");
    }
}

#endif
static void cli_cmd_wis_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_mode_t  wis_mode;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_mode_get(req->api_inst, port_no, &wis_mode) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_mode_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
            if (req->enable) {
                wis_mode = VTSS_WIS_OPERMODE_WIS_MODE;
            } else {
                wis_mode = VTSS_WIS_OPERMODE_DISABLE;
            }
            if (vtss_ewis_mode_set(NULL, port_no, &wis_mode) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_mode_set() for port %u\n",port);
                continue;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("WIS Mode :%-12s  \n",wis_mode ? "Enabled" : "Disabled");
        }
    }
}

static void cli_cmd_wis_satti_get(cli_req_t *req)
{
    vtss_port_no_t   port_no, port;
    vtss_ewis_tti_t       wis_acti;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_section_acti_get(req->api_inst, port_no, &wis_acti) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_section_acti_get() operation for port %u\n",port);
            continue;
        }
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("Section received TTI :\n");
        printf("%-20s: %-4u\n","Mode",wis_acti.mode);
        if (wis_acti.valid != FALSE) {
            printf("%-20s: %-64s\n","TTI",wis_acti.tti);
        } else {
            printf("Invalid TxTI configuration :: Transmiter and Receiver Section TxTI modes are not matching\n");
        }
    }
}
static void cli_cmd_wis_patti_get(cli_req_t *req)
{
    vtss_port_no_t      port_no, port;
    vtss_ewis_tti_t          wis_acti;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_path_acti_get(req->api_inst, port_no, &wis_acti) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_section_acti_get() operation for port %u\n",port);
            continue;
        }
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("Path received TTI :\n");
        printf("%-20s: %-4u\n","Mode",wis_acti.mode);
        if (wis_acti.valid != FALSE) {
            printf("%-20s: %-64s\n","TTI",wis_acti.tti);
        } else {
            printf("Invalid TxTI configuration :: Transmiter and Receiver Path TxTI modes are not matching\n");
        }
    }
}
static void cli_cmd_wis_section_tti(cli_req_t *req)
{
    static const char * tti_txt [] = {"MODE_1 ","MODE_16","MODE_64"};

    vtss_port_no_t port_no, port;
    vtss_ewis_tti_t tti;

    memset(&tti, 0, sizeof(vtss_ewis_tti_t));
    if (req->set) {
        tti.mode = req->wis_tti_mode;
        if (req->wis_tti_set) {
            if (tti.mode == TTI_MODE_16) {
                memcpy(tti.tti, req->wis_tti,15);
                tti.tti[15] = 0x89; /* MSB set to 1 */
            } else if (tti.mode == TTI_MODE_64) {
                memcpy(&tti.tti[2],req->wis_tti,62);
                tti.tti[0] = 13; /* CR */
                tti.tti[1] = 10; /* LF */
            } else if (tti.mode == TTI_MODE_1) {
                tti.tti[0] = req->wis_tti[0];
            }
        }
        printf("ttimode: %d, tti: %s\n", tti.mode, tti.tti);
    } else {
        printf("Wis Section Transmitted TTI\n");
        cli_table_header("Port  tti_mode  tti             ");
    }
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (req->set) {
            CLI_TST_RC(vtss_ewis_section_txti_set(req->api_inst,
                                   port_no,
                                   &tti));
        } else {
            CLI_TST_RC(vtss_ewis_section_txti_get(req->api_inst,
                                   port_no,
                                   &tti));
            printf("%-4u  %8s  %s\n", port, tti.mode <TTI_MODE_MAX ? tti_txt[tti.mode] : "Inval", tti.tti);
        }
    }
}

static void cli_cmd_wis_path_tti(cli_req_t *req)
{
    static const char * tti_txt [] = {"MODE_1 ","MODE_16","MODE_64"};
    
    vtss_port_no_t port_no, port;
    vtss_ewis_tti_t tti;
    
    memset(&tti, 0, sizeof(vtss_ewis_tti_t));
    if (req->set) {

        tti.mode = req->wis_tti_mode;
        if (req->wis_tti_set) {
            if (tti.mode == TTI_MODE_16) {
                memcpy(tti.tti, req->wis_tti,15);
                tti.tti[15] = 0x89; /* MSB set to 1 */
            } else if (tti.mode == TTI_MODE_64) {
                memcpy(&tti.tti[2],req->wis_tti,62);
                tti.tti[0] = 13; /* CR */
                tti.tti[1] = 10; /* LF */
            } else if (tti.mode == TTI_MODE_1) {
                tti.tti[0] = req->wis_tti[0];
            }
        }
        printf("ttimode: %d, tti: %s\n", tti.mode, tti.tti);
    } else {
        printf("Wis Path Transmitted TTI\n");
        cli_table_header("Port  tti_mode  tti             ");
    }
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->set) {
            CLI_TST_RC(vtss_ewis_path_txti_set(req->api_inst,
                                   port_no,
                                   &tti));
        } else {
            CLI_TST_RC(vtss_ewis_path_txti_get(req->api_inst,
                                   port_no,
                                   &tti));
            printf("%-4u  %8s  %s\n", port, tti.mode < TTI_MODE_MAX ? tti_txt[tti.mode] : "Inval", tti.tti);
        }
    }
}

static void cli_cmd_wis_path_txsl(cli_req_t *req)
{
    vtss_port_no_t    port_no, port;
    vtss_ewis_tx_oh_t tx_oh;

    if (!req->set) {
        printf("WIS Path TX Signal Label:\n");
        cli_table_header("Port   Signal Label");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK != vtss_ewis_tx_oh_get(req->api_inst, port_no, &tx_oh)) printf("vtss_ewis_tx_oh_get failed\n");

        if (req->set) {
            tx_oh.tx_c2 = req->sl;
            if (VTSS_RC_OK != vtss_ewis_tx_oh_set(req->api_inst, port_no, &tx_oh)) printf("vtss_ewis_tx_oh_set failed\n");
        }
        else {
            printf("%4u%10u\n", port, tx_oh.tx_c2);
        }
    }
}

static void cli_cmd_wis_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_status_t status;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (vtss_ewis_status_get(req->api_inst, port_no, &status) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_status_get(): %u operation\n",port);
            continue;
        }
        
        printf("WIS Status Port: %u\n",port);
        printf("-------------------\n");
        printf("%-30s %-12s\n", "  Fault condition       :",status.fault?"Fault":"Normal");
        printf("%-30s %-12s\n", "  Link status condition :",status.link_stat?"Up":"Down");
    }
}

static void cli_cmd_wis_cons_act(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_cons_act_t cons_act;

    if (!req->set) {
        printf("WIS cons act:\n");
        cli_table_header("Port    ais_on_los    ais_on_lof   rdil_on_los   rdil_on_lof   rdil_on_ais");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK != vtss_ewis_cons_act_get(req->api_inst, port_no, &cons_act)) printf("vtss_ewis_cons_act_get failed\n");

        if (req->set) {
            if (req->ais_on_los)    cons_act.aisl.ais_on_los = (req->enable);
            if (req->ais_on_lof)    cons_act.aisl.ais_on_lof = (req->enable);
            if (req->rdil_on_los)   cons_act.rdil.rdil_on_los = (req->enable);
            if (req->rdil_on_lof)   cons_act.rdil.rdil_on_lof = (req->enable);
            if (req->rdil_on_ais)   cons_act.rdil.rdil_on_ais_l = (req->enable);
            if (VTSS_RC_OK != vtss_ewis_cons_act_set(req->api_inst, port_no, &cons_act)) printf("vtss_ewis_cons_act_set failed\n");
        }
        else {
            printf("%4u%14s%14s%14s%14s%14s\n", port,
                   cli_bool_disp(cons_act.aisl.ais_on_los),
                   cli_bool_disp(cons_act.aisl.ais_on_lof),
                   cli_bool_disp(cons_act.rdil.rdil_on_los),
                   cli_bool_disp(cons_act.rdil.rdil_on_lof),
                   cli_bool_disp(cons_act.rdil.rdil_on_ais_l));
        }
    }
}

#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
static void cli_cmd_wis_test_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_test_conf_t  test_mode;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_test_mode_get(req->api_inst, port_no, &test_mode) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_test_mode_get() operation");
            continue;
        }
        if (req->set) {
            if (req->wis_loopback_set) {
                test_mode.loopback         = req->wis_loopback;
            }
            test_mode.test_pattern_ana = req->wis_ana_mode;
            test_mode.test_pattern_gen = req->wis_gen_mode;
            if (vtss_ewis_test_mode_set(NULL, port_no, &test_mode) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_test_mode_set() for port %u\n", port);
                continue;
            }
        } else {
            printf("Port: %u\n", port);
            printf("--------\n");
            printf("Loopback :%-12s  Test Pattern generator  : %u   Test pattern analyzer  :%u\n",
                    test_mode.loopback?"Yes":"No", 
                    test_mode.test_pattern_gen, test_mode.test_pattern_ana);
        }
    }

}
#endif

static void cli_cmd_wis_counters(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_counter_t      counter;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_counter_get(req->api_inst, port_no, &counter) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_ewis_counter_get(): %u operation\n",port);
            continue;
        }

        printf("WIS Error Counters:\n\n");
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-30s %-12u\n", "  Section BIP error count               :",counter.pn_ebc_s);
        printf("%-30s %-12u\n", "  Near end line block (BIP) error count:",counter.pn_ebc_l);
        printf("%-30s %-12u\n", "  Far end line block (BIP) error count :",counter.pf_ebc_l);
        printf("%-30s %-12u\n", "  Path block error count                :",counter.pn_ebc_p);
        printf("%-30s %-12u\n", "  Far end path block error count        :",counter.pf_ebc_p);
    }
}

static void cli_cmd_wis_defects(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_ewis_defects_t      defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_ewis_defects_get(req->api_inst, port_no, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_defects_get(): %u operation\n",port);
            continue;
        }
        printf("WIS Defects:\n\n");
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Loss of signal                      :",defects.dlos_s?"Yes":"No");
        printf("%-30s %-12s\n", "  Loss of frame                       :",defects.dlof_s?"Yes":"No");
        printf("%-30s %-12s\n", "  Line alarm indication signal        :",defects.dais_l?"Yes":"No");
        printf("%-30s %-12s\n", "  Line remote defect indication       :",defects.drdi_l?"Yes":"No");
        printf("%-30s %-12s\n", "  Path alarm indication signal        :",defects.dais_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Loss of pointer                     :",defects.dlop_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Path Unequipped                     :",defects.duneq_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Path Remote Defect Indication       :",defects.drdi_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Path loss of code-group delineation :",defects.dlcd_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Path label Mismatch                 :",defects.dplm_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Far-end AIS-P or LOP-P              :",defects.dfais_p?"Yes":"No");
        printf("%-30s %-12s\n", "  Far-end PLM-P or LCD-P defect       :",defects.dfplm_p?"Yes":"No");
    }
}

#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
static void cli_cmd_wis_force_conf(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_wis_force_mode_t   force_conf;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        if (req->set) {
            
            force_conf.line_rx_force.force_ais =  req->wis_line_rx&1;
            force_conf.line_rx_force.force_rdi = (req->wis_line_rx>>1)&1;
            force_conf.line_tx_force.force_ais =  req->wis_line_tx&1;
            force_conf.line_tx_force.force_rdi = (req->wis_line_tx>>1)&1;
            force_conf.path_force.force_uneq   =  req->wis_path_force&1;;
            force_conf.path_force.force_rdi    = (req->wis_path_force>>1)&1;;
            
            if (vtss_ewis_force_conf_set(NULL, port_no, &force_conf) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_force_conf_set() for port %u\n", port);
                continue;
            }
        } else {
            if (vtss_ewis_force_conf_get(req->api_inst, port_no, &force_conf) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_force_conf_get() for port %u\n", port);
                continue;
            }
            printf("Port: %u\n", port);
            printf("--------\n");
            printf("Line rx force ais :%-6s \n",force_conf.line_rx_force.force_ais?"Yes":"No");
            printf("Line rx force rdi :%-6s \n",force_conf.line_rx_force.force_rdi?"Yes":"No");
            printf("Line tx force ais :%-6s \n",force_conf.line_tx_force.force_ais?"Yes":"No");
            printf("Line tx force rdi :%-6s \n",force_conf.line_tx_force.force_rdi?"Yes":"No");
            printf("path force uneq   :%-6s \n",force_conf.path_force.force_uneq?"Yes":"No");
            printf("path force rdi   :%-6s \n",force_conf.path_force.force_rdi?"Yes":"No");
        }
    }
}
#endif
static void cli_cmd_wis_oh_conf(cli_req_t * req)
{
    vtss_port_no_t     port_no, port;
    vtss_ewis_tx_oh_t  tx_oh;
    u8                 *oh_value,i=0;
    /* Debug WIS tx_oh_conf <port_list> [[<oh_name>] [<oh_value>]] */
    oh_value = req->wis_oh_val;
    memset(&tx_oh, 0,sizeof(vtss_ewis_tx_oh_t));
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (vtss_ewis_tx_oh_get(req->api_inst, port_no, &tx_oh) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_tx_oh_get(): %u operation\n",port);
            continue;
        }
        if (req->set) {
            switch(req->wis_oh_id){
                case 1:
                    memcpy((u8 *)(tx_oh.tx_dcc_s),oh_value, 3); i=i+3;
                    tx_oh.tx_e1       = *(oh_value+i++);
                    tx_oh.tx_f1       = *(oh_value+i++);
                    tx_oh.tx_z0       = *(oh_value+i++);
                    memcpy((u8 *)(tx_oh.tx_dcc_l),(oh_value+i), 9); i=i+9;
                    tx_oh.tx_e2= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_k1_k2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_s1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z1_z2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_c2= *(oh_value+i++);
                    tx_oh.tx_f2= *(oh_value+i++);
                    tx_oh.tx_n1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z3_z4),(oh_value+i), 2);
                    break;
                case 2:
                    memcpy((u8 *)(tx_oh.tx_dcc_s),oh_value, 3); i=i+3;
                    tx_oh.tx_e1       = *(oh_value+i++);
                    tx_oh.tx_f1       = *(oh_value+i++);
                    tx_oh.tx_z0       = *(oh_value+i++);
                    break;
                case 3:
                    memcpy((u8 *) (tx_oh.tx_dcc_l),oh_value, 9); i=i+9;
                    tx_oh.tx_e2= *(oh_value+i++);
                    memcpy((u8 *) (&tx_oh.tx_k1_k2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_s1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z1_z2),(oh_value+i), 2);
                    break;
                case 4:
                    tx_oh.tx_c2= *(oh_value+i++);
                    tx_oh.tx_f2= *(oh_value+i++);
                    tx_oh.tx_n1= *(oh_value+i++);
                    memcpy((u8 *) (&tx_oh.tx_z3_z4),(oh_value+i), 2);
                    break;
                case 5:
                    memcpy((u8 *) (tx_oh.tx_dcc_s),oh_value, 3);
                    break;
                case 6:
                    tx_oh.tx_e1       = oh_value[0];
                    break;
                case 7:
                    tx_oh.tx_f1       = oh_value[0];
                    break;
                case 8:
                    tx_oh.tx_z0       = oh_value[0];
                    break;
                case 9:
                    memcpy((u8 *) (tx_oh.tx_dcc_l),oh_value, 9);
                    break;
                case 10:
                    tx_oh.tx_e2= oh_value[0];
                    break;
                case 11:
                    memcpy((u8 *) (&tx_oh.tx_k1_k2),oh_value, 2);
                    break;
                case 12:
                    tx_oh.tx_s1= oh_value[0];
                    break;
                case 13:
                    memcpy((u8 *) (&tx_oh.tx_z1_z2),oh_value, 2);
                    break;
                case 14:
                    tx_oh.tx_c2= oh_value[0];
                    break;
                case 15:
                    tx_oh.tx_f2= oh_value[0];
                    break;
                case 16:
                    tx_oh.tx_n1= oh_value[0];
                    break;
                case 17:
                    memcpy((u8 *) (&tx_oh.tx_z3_z4),oh_value, 2);
                    break;
                default:
                    
                    break;
            }
            if (vtss_ewis_tx_oh_set(NULL, port_no, &tx_oh) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_tx_oh_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_ewis_tx_oh_get(req->api_inst, port_no, &tx_oh) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_tx_oh_get(): %u operation\n",port);
                continue;
            }
            printf("Tx Overhead for Port: %u\n",port);
            printf("------------------------\n");
            printf("D1-D3     :  0x%x%x%x \n",tx_oh.tx_dcc_s[0],tx_oh.tx_dcc_s[1],tx_oh.tx_dcc_s[2]);
            printf("SEC-ORD   :  0x%x\n",tx_oh.tx_e1);
            printf("SUC       :  0x%x \n",tx_oh.tx_f1);
            printf("RES-SG    :  0x%x \n",tx_oh.tx_z0);
            printf("D4-D12    :  0x%x%x%x%x%x%x%x%x%x \n",tx_oh.tx_dcc_l[0],tx_oh.tx_dcc_l[1],tx_oh.tx_dcc_l[2],
                    tx_oh.tx_dcc_l[3],tx_oh.tx_dcc_l[4],tx_oh.tx_dcc_l[5],
                    tx_oh.tx_dcc_l[6],tx_oh.tx_dcc_l[7],tx_oh.tx_dcc_l[8]);
            printf("LINE-ORD  :  0x%x \n",tx_oh.tx_e2);
            printf("APS-RDIL  :  0x%x \n",tx_oh.tx_e2);
            printf("SYNC      :  0x%x \n",tx_oh.tx_k1_k2);
            printf("RES-LG    :  0x%x \n",tx_oh.tx_z1_z2);
            printf("C2PL      :  0x%x \n",tx_oh.tx_c2);
            printf("PUC       :  0x%x \n",tx_oh.tx_f2);
            printf("PTCM      :  0x%x \n",tx_oh.tx_n1);
            printf("RES-PG    :  0x%x \n\n",tx_oh.tx_z3_z4);
        }
    }
}

#if defined(VTSS_DAYTONA_WIS_CLI_ALL)

static void cli_cmd_wis_prbs_err_inj(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_prbs31_err_inj_t  err_inj;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        err_inj = req->wis_err_inj;
        if (vtss_ewis_prbs31_err_inj_set(NULL, port_no, &err_inj) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_prbs31_err_inj_set(): %u operation\n", port);
            continue;
        }
    }
}

static void cli_cmd_wis_tx_pref_thre(cli_req_t *req)
{
    vtss_port_no_t                  port_no, port;
    vtss_wis_counter_threshold_t    threshold;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (req->set) {
            threshold.n_ebc_thr_s = req->wis_n_ebc_thr_s; 
            threshold.n_ebc_thr_l = req->wis_n_ebc_thr_l; 
            threshold.f_ebc_thr_l = req->wis_f_ebc_thr_l; 
            threshold.n_ebc_thr_p = req->wis_n_ebc_thr_p; 
            threshold.f_ebc_thr_p = req->wis_f_ebc_thr_p; 
            if (vtss_ewis_counter_threshold_set(NULL, port_no, &threshold) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_counter_threshold_set() for port %u\n", port);
                continue;
            }
        } else {
            if (vtss_ewis_counter_threshold_get(req->api_inst, port_no, &threshold) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_counter_threshold_get(): %u operation\n", port);
                continue;
            }
            printf("Threshold conf for Port: %u\n", port);
            printf("---------------------------\n");
            printf("Section error count (B1) threshold        :%-12u\n",threshold.n_ebc_thr_s);
            printf("Near end line error count (B2) threshold  :%-12u\n",threshold.n_ebc_thr_l );
            printf("Far end line error count threshold        :%-12u\n",threshold.f_ebc_thr_l );
            printf("Path block error count (B3) threshold     :%-12u\n",threshold.n_ebc_thr_p );
            printf("Far end path error count threshold        :%-12u\n\n",threshold.f_ebc_thr_p );
        }
    }
}


#endif
static void cli_cmd_wis_pref_counters(cli_req_t *req)
{
        vtss_port_no_t      port_no, port;
        vtss_ewis_perf_t     perf_pre;
    
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if ((req->port_list[port] == 0) ) 
                continue;
           
            if (vtss_ewis_perf_get(req->api_inst, port_no, &perf_pre) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_perf_get(): %u operation\n",port);
                continue;
            }
            printf("WIS performance primitives:\n\n");
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("Section BIP error count               : %-12u\n",perf_pre.pn_ebc_s);
            printf("Near end line block (BIP) error count : %-12u\n",perf_pre.pn_ebc_l);
            printf("Far end line block (BIP) error count  : %-12u\n",perf_pre.pf_ebc_l);
            printf("Path block error count                : %-12u\n",perf_pre.pn_ebc_p);
            printf("Far end path block error count        : %-12u\n",perf_pre.pf_ebc_p);
        }
}

static void cli_cmd_wis_forced_insertion(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_ewis_force_mode_t  force_conf;
  
    printf("WIS Block forced insertion of AIS/RDI:\n");
    cli_table_header("Port  RX_AIS  RX_RDI  TX_AIS  TX_RDI");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&force_conf, 0, sizeof(vtss_ewis_force_mode_t));
        CLI_TST_RC(vtss_ewis_force_conf_get(req->api_inst, port_no, &force_conf));
        if (req->set) {
            if (req->wis_rx_force) {
                if (req->wis_forced_ais_l) {
                    force_conf.line_rx_force.force_ais = req->enable;
                } else if (req->wis_forced_rdi_l){
                    force_conf.line_rx_force.force_rdi = req->enable;
                }
            } else if (req->wis_tx_force){
                if (req->wis_forced_ais_l) {
                    force_conf.line_tx_force.force_ais = req->enable;
                } else if (req->wis_forced_rdi_l){
                    force_conf.line_tx_force.force_rdi = req->enable;
                }
            }
            CLI_TST_RC(vtss_ewis_force_conf_set(req->api_inst, port_no, &force_conf));
        }
        printf("%-4u  %-6s  %-6s  %-6s  %-6s\n", port, cli_bool_disp(force_conf.line_rx_force.force_ais), cli_bool_disp(force_conf.line_rx_force.force_rdi),
                                                        cli_bool_disp(force_conf.line_tx_force.force_ais), cli_bool_disp(force_conf.line_tx_force.force_rdi));
   }
}

static void cli_cmd_wis_perf_mode(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_ewis_perf_mode_t   perf_mode;

    printf("WIS Block Perf Counter Mode Configuration:\n");
    cli_table_header("Port  B1_Mode  B2_Mode  B3_Mode  REIL_Mode  REIP_Mode");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&perf_mode, 0, sizeof(vtss_ewis_perf_mode_t));
        CLI_TST_RC(vtss_ewis_perf_mode_get(req->api_inst, port_no, &perf_mode));
        if (req->set) {
            if (req->wis_pn_ebc_mode_s && req->wis_perf_mode) {
                perf_mode.pn_ebc_mode_s = req->wis_perf_mode_bit;
            } else if (req->wis_pn_ebc_mode_l && req->wis_perf_mode) {
                perf_mode.pn_ebc_mode_l = req->wis_perf_mode_bit;
            } else if (req->wis_pf_ebc_mode_l && req->wis_perf_mode) {
                perf_mode.pf_ebc_mode_l = req->wis_perf_mode_bit;
            } else if (req->wis_pn_ebc_mode_p && req->wis_perf_mode) {
                perf_mode.pn_ebc_mode_p = req->wis_perf_mode_bit;
            } else if (req->wis_pf_ebc_mode_p && req->wis_perf_mode) {
                perf_mode.pf_ebc_mode_p = req->wis_perf_mode_bit;
            }
            CLI_TST_RC(vtss_ewis_perf_mode_set(req->api_inst, port_no, &perf_mode));
        }
        printf("%-4u  %-7s  %-7s  %-7s  %-9s  %-9s\n", port, (perf_mode.pn_ebc_mode_s ? "Block" : "Bit"), (perf_mode.pn_ebc_mode_l ? "Block" : "Bit"), (perf_mode.pn_ebc_mode_p ?                                                       "Block" : "Bit"), (perf_mode.pf_ebc_mode_l ? "Block" : "Bit"), (perf_mode.pf_ebc_mode_p ? "Block" : "Bit"));
    }
}
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
static void cli_cmd_wis_event_force(cli_req_t *req)
{
    vtss_port_no_t       port_no, port;
    vtss_ewis_event_t    ev_force;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        ev_force = req->wis_force_events;
        if (vtss_ewis_event_force(NULL, port_no, 1, ev_force) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_events_force(): %u operation\n", port);
            continue;
        }
    }
}
#endif
static void cli_cmd_wis_passthru(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_tx_oh_passthru_t tx_oh_passthru;
    u32  passthru; 
    passthru = req->wis_passthru; 
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) 
            continue;
        if (vtss_ewis_tx_oh_passthru_get(req->api_inst, port_no, &tx_oh_passthru) != VTSS_RC_OK) {
            printf("Could not perform vtss_ewis_tx_oh_passthru_get() operation");
            continue;
        }
        if (req->set) {
            
            tx_oh_passthru.tx_j0 = (passthru&1); /**< j0 Section TTI  passthrough */
            tx_oh_passthru.tx_z0 = (passthru >> 1&1); /**< z0 Section growth passthrough */
            tx_oh_passthru.tx_b1 = (passthru >> 2&1); /**< b1 BIP passthrough */
            tx_oh_passthru.tx_e1 = (passthru>>3 & 1);; /**< e1 order wire passthrough */
            tx_oh_passthru.tx_f1 = (passthru >> 4 & 1); /**< f1 Section user channel */
            tx_oh_passthru.tx_dcc_s = (passthru >> 5 & 1);; /**< Section Data communication channel passthrough D1-D3 */
            tx_oh_passthru.tx_soh = (passthru >> 6 & 1);; /**< Section Reserved National and unused bytes passthrough */
            /**< line overhead: */
            tx_oh_passthru.tx_b2 = (passthru >> 7 & 1);; /**< b2 BIP passthrough */
            tx_oh_passthru.tx_k1 = (passthru >> 8 & 1); /**< k1 passthrough */
            tx_oh_passthru.tx_k2 = (passthru >> 9 & 1); /**< k2 passthrough */
            tx_oh_passthru.tx_reil = (passthru >>10 & 1); /**< reil passthrough */
            tx_oh_passthru.tx_dcc_l = (passthru >> 11 & 1); /**< Section Data communication channel passthrough D4-D12*/
            tx_oh_passthru.tx_s1 = (passthru >> 12 & 1);; /**< Synchronization messaging passthrough */
            tx_oh_passthru.tx_e2 = (passthru >> 13 & 1); /**< order wire passthrough */
            tx_oh_passthru.tx_z1_z2 = (passthru >> 14 & 1); /**< Reserved for path growth passthrough */
            tx_oh_passthru.tx_loh = (passthru >> 15 & 1); /**< Line Reserved National and unused bytes passthrough */
            if (vtss_ewis_tx_oh_passthru_set(NULL, port_no, &tx_oh_passthru) != VTSS_RC_OK) {
                printf("Could not perform vtss_ewis_tx_oh_passthru_set() for port %u\n",port);
                continue;
            }
        } else {
            printf("Port: %u\n",port);
            printf("--------\n");
            printf("tx_j0    :%-12s\n",tx_oh_passthru.tx_j0?"TRUE":"FALSE");/**< j0 Section TTI  passthrough */
            printf("tx_z0    :%-12s\n",tx_oh_passthru.tx_z0?"TRUE":"FALSE");/**< z0 Section growth passthrough */
            printf("tx_b1    :%-12s\n",tx_oh_passthru.tx_b1?"TRUE":"FALSE");/**< b1 BIP passthrough */
            printf("tx_e1    :%-12s\n",tx_oh_passthru.tx_e1?"TRUE":"FALSE");/**< e1 order wire passthrough */
            printf("tx_f1    :%-12s\n",tx_oh_passthru.tx_f1?"TRUE":"FALSE");/**< f1 Section user channel */
            printf("tx_dcc_s :%-12s\n",tx_oh_passthru.tx_dcc_s?"TRUE":"FALSE");/**< Section Data communication channel passthrough D1-D3 */
            printf("tx_soh   :%-12s\n",tx_oh_passthru.tx_soh?"TRUE":"FALSE");/**< Section Reserved National and unused bytes passthrough */
            /**< line overhead: */
            printf("tx_b2    :%-12s\n",tx_oh_passthru.tx_b2?"TRUE":"FALSE");/**< b2 BIP passthrough */
            printf("tx_k1    :%-12s\n",tx_oh_passthru.tx_k1?"TRUE":"FALSE"); /**< k1 passthrough */
            printf("tx_k2    :%-12s\n",tx_oh_passthru.tx_k2?"TRUE":"FALSE");/**< k2 passthrough */
            printf("tx_reil  :%-12s\n",tx_oh_passthru.tx_reil?"TRUE":"FALSE");/**< reil passthrough */
            printf("tx_dcc_l :%-12s\n",tx_oh_passthru.tx_dcc_l?"TRUE":"FALSE");/**< Section Data communication channel passthrough D4-D12*/
            printf("tx_s1    :%-12s\n",tx_oh_passthru.tx_s1?"TRUE":"FALSE");/**< Synchronization messaging passthrough */
            printf("tx_e2    :%-12s\n",tx_oh_passthru.tx_e2?"TRUE":"FALSE");/**< order wire passthrough */
            printf("tx_z1_z2 :%-12s\n",tx_oh_passthru.tx_z1_z2?"TRUE":"FALSE");/**< Reserved for path growth passthrough */
            printf("tx_loh   :%-12s\n",tx_oh_passthru.tx_loh?"TRUE":"FALSE");/**< Line Reserved National and unused bytes passthrough */
        }
    }
}

#endif /* VTSS_FEATURE_WIS */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
static void cli_cmd_1588_block_init(cli_req_t *req)
{
    vtss_port_no_t port_no;
    vtss_phy_ts_init_conf_t phy_init = {VTSS_PHY_TS_CLOCK_FREQ_250M, VTSS_PHY_TS_CLOCK_SRC_LINE_TX, VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP, VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT, VTSS_PHY_TS_FIFO_MODE_SPI, VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_10BYTE, 0};
    static BOOL ts_init_info[4] = {FALSE, FALSE, FALSE, FALSE};
    vtss_rc rc;
    
    port_no = uport2iport(req->port_no);
    if(req->clk_freq_spec)
        phy_init.clk_freq = req->clkfreq;
    if(req->clk_src_spec)
        phy_init.clk_src = req->clk_src;
    if(req->rx_ts_pos_spec)
        phy_init.rx_ts_pos = req->rx_ts_pos;
    if(req->tx_fifo_mode_spec)
        phy_init.tx_fifo_mode = req->tx_fifo_mode;
    if(req->modify_frm)
        phy_init.chk_ing_modified = req->modify_frm;  
    if (ts_init_info[port_no] == FALSE) {
        rc = vtss_phy_ts_init(API_INST_DEFAULT, port_no, &phy_init);
        if (rc == VTSS_RC_OK) {
            ts_init_info[port_no] = TRUE;
            printf("PHY TS Init Success\n");
        } else {
            printf("PHY TS Init Failed!\n");
            return;
        }
        rc = vtss_phy_ts_mode_set(API_INST_DEFAULT, port_no, TRUE);
        if (rc != VTSS_RC_OK) {
            printf("PHY TS Block Enable Failed\n");
        }
    }
}
static void cli_cmd_1588_engine_init(cli_req_t *req)
{
    vtss_port_no_t port_no, base_port;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    port_no = uport2iport(req->port_no);
    if ((rc = vtss_phy_ts_base_port_get(port_no, &base_port)) != VTSS_RC_OK) {
        printf("Engine initialized failed!!!\n");
        return;
    }

    if (req->set == FALSE) {
        if(engine_init_info[base_port][dir][eng_id].init == FALSE) {
            printf("Engine not initialized!\n");
        } else {
            printf("encap_type = %d flow_start = %d flow_end = %d flow_match = %s\n",
                    engine_init_info[base_port][dir][eng_id].encap_type,
                    engine_init_info[base_port][dir][eng_id].flow_st_index,
                    engine_init_info[base_port][dir][eng_id].flow_end_index,
                    ((engine_init_info[base_port][dir][eng_id].flow_match_mode == VTSS_PHY_TS_ENG_FLOW_MATCH_STRICT) ? "strict" : "non-strict"));
        }
    } else {
        printf("port_no = %d, ingress = %d, eng_id = %d, encap = %d, st_ind = %d, end_ind = %d, match_mode = %d\n",
                (int)port_no, req->ingress, eng_id, req->encap_type,
                req->flow_st_index, req->flow_end_index, req->flow_match_mode);
        if (req->ingress) {
            rc = vtss_phy_ts_ingress_engine_init(API_INST_DEFAULT, port_no, eng_id,
                             req->encap_type,
                             req->flow_st_index,
                             req->flow_end_index,
                             req->flow_match_mode);
        } else {
            rc = vtss_phy_ts_egress_engine_init(API_INST_DEFAULT, port_no, eng_id,
                             req->encap_type,
                             req->flow_st_index,
                             req->flow_end_index,
                             req->flow_match_mode);
        }
        if (rc == VTSS_RC_OK) {
            engine_init_info[base_port][dir][eng_id].init = TRUE;
            engine_init_info[base_port][dir][eng_id].encap_type = req->encap_type;
            engine_init_info[base_port][dir][eng_id].flow_st_index = req->flow_st_index;
            engine_init_info[base_port][dir][eng_id].flow_end_index = req->flow_end_index;
            engine_init_info[base_port][dir][eng_id].flow_match_mode = req->flow_match_mode;
            engine_init_info[port_no][dir][eng_id] = engine_init_info[base_port][dir][eng_id];
            printf("Engine init success\n");
        } else {
            printf("Engine init Failed!\n");
        }
    }
}

static void cli_cmd_1588_engine_clear(cli_req_t *req)
{
    vtss_port_no_t port_no, base_port;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;

    port_no = uport2iport(req->port_no);

    if ((rc = vtss_phy_ts_base_port_get(port_no, &base_port)) != VTSS_RC_OK) {
        printf("Engine initialized failed!!!\n");
        return;
    }
    if(engine_init_info[base_port][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
    } else {
        if (req->ingress) {
            rc = vtss_phy_ts_ingress_engine_clear(API_INST_DEFAULT, port_no, eng_id);
        } else {
            rc = vtss_phy_ts_egress_engine_clear(API_INST_DEFAULT, port_no, eng_id);
        }
        if (rc == VTSS_RC_OK) {
            engine_init_info[base_port][dir][eng_id].init = FALSE;
            engine_init_info[base_port][dir][eng_id].encap_type = 0;
            engine_init_info[base_port][dir][eng_id].flow_st_index = 0;
            engine_init_info[base_port][dir][eng_id].flow_end_index = 0;
            engine_init_info[base_port][dir][eng_id].flow_match_mode = 0;
            engine_init_info[port_no][dir][eng_id] = engine_init_info[base_port][dir][eng_id];
            printf("Engine clear success\n");
        } else {
            printf("Engine init Failed!\n");
        }
    }
}
static void cli_cmd_1588_engine_mode(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_phy_ts_engine_flow_conf_t *flow_conf = NULL, *conf_ptr;
    vtss_rc rc;

    port_no = uport2iport(req->port_no);
    if(engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }
    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            flow_conf->eng_mode = req->enable;
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("Engine Mode:%d\n", flow_conf->eng_mode);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}
static void cli_cmd_1588_engine_channel_map(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;
    vtss_rc rc;
    int i;

    port_no = uport2iport(req->port_no);
    if(engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }
    if (req->flow_id_spec &&
        (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index)) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (req->flow_id_spec) {
                flow_conf->channel_map[req->flow_id] = req->channel_map;
            } else {
                for (i = engine_init_info[port_no][dir][eng_id].flow_st_index;
                     i < engine_init_info[port_no][dir][eng_id].flow_end_index; i++) {
                    flow_conf->channel_map[i] = req->channel_map;
                }
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("Channel_map: ");
            for (i = engine_init_info[port_no][dir][eng_id].flow_st_index;
                 i <= engine_init_info[port_no][dir][eng_id].flow_end_index; i++) {
                printf("%x ", flow_conf->channel_map[i]);
            }
            printf("\n");
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_eth1_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth1_conf = &flow_conf->flow_conf.ptp.eth1_opt;
    } else {
        eth1_conf = &flow_conf->flow_conf.oam.eth1_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (req->pbb_spec) eth1_conf->comm_opt.pbb_en = req->pbb_en;
            eth1_conf->comm_opt.etype = (req->etype ? req->etype : eth1_conf->comm_opt.etype);
            eth1_conf->comm_opt.tpid = (req->tpid ? req->tpid : eth1_conf->comm_opt.tpid);
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("ETH1 conf: pbb = %d, etype = 0x%x, tpid = 0x%x\n",
                    eth1_conf->comm_opt.pbb_en, eth1_conf->comm_opt.etype,
                    eth1_conf->comm_opt.tpid);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_eth1_flow_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    if (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth1_conf = &flow_conf->flow_conf.ptp.eth1_opt;
    } else {
        eth1_conf = &flow_conf->flow_conf.oam.eth1_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth1_conf->flow_opt[req->flow_id].flow_en = req->enable;
            if (req->mac_match_mode_spec) {
                eth1_conf->flow_opt[req->flow_id].addr_match_mode = req->mac_match_mode;
            }
            if (req->ptp_mac_spec) {
                memcpy(eth1_conf->flow_opt[req->flow_id].mac_addr, req->ptp_mac, 6);
            }
            if (req->addr_match_select_spec) {
                if (req->addr_match_select == 0) {
                    eth1_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_ADDR;
                } else if (req->addr_match_select == 1) {
                    eth1_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
                } else {
                    eth1_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_OR_DEST;
                }
            }
            if (req->vlan_chk_spec) {
                eth1_conf->flow_opt[req->flow_id].vlan_check = req->vlan_chk;
            }
            if (req->num_tag_spec) {
                eth1_conf->flow_opt[req->flow_id].num_tag = req->num_tag;
            }
            if (req->tag_rng_mode_spec) {
                eth1_conf->flow_opt[req->flow_id].tag_range_mode = req->tag_rng_mode;
            }
            if (req->tag1_type > 0 && req->tag1_type < 5) {
                eth1_conf->flow_opt[req->flow_id].outer_tag_type = req->tag1_type;
            }
            if (req->tag2_type > 0 && req->tag2_type < 5) {
                eth1_conf->flow_opt[req->flow_id].inner_tag_type = req->tag2_type;
            }
            if (req->tag_rng_mode_spec && req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                if (req->tag1_lower) {
                    eth1_conf->flow_opt[req->flow_id].outer_tag.range.lower = req->tag1_lower;
                }
                if (req->tag1_upper) {
                    eth1_conf->flow_opt[req->flow_id].outer_tag.range.upper = req->tag1_upper;
                }
            } else {
                eth1_conf->flow_opt[req->flow_id].outer_tag.value.val = req->tag1_lower;
                eth1_conf->flow_opt[req->flow_id].outer_tag.value.mask = req->tag1_upper;
            }
            if (req->tag_rng_mode_spec && req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                if (req->tag2_lower) {
                    eth1_conf->flow_opt[req->flow_id].inner_tag.range.lower = req->tag2_lower;
                }
                if (req->tag2_upper) {
                    eth1_conf->flow_opt[req->flow_id].inner_tag.range.upper = req->tag2_upper;
                }
            } else {
                eth1_conf->flow_opt[req->flow_id].inner_tag.value.val = req->tag2_lower;
                eth1_conf->flow_opt[req->flow_id].inner_tag.value.mask = req->tag2_upper;
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("ETH1 flow conf: enable = %d, match_mode = %d, mac = 0x%x-%x-%x-%x-%x-%x\n",
                    eth1_conf->flow_opt[req->flow_id].flow_en,
                    eth1_conf->flow_opt[req->flow_id].addr_match_mode,
                    eth1_conf->flow_opt[req->flow_id].mac_addr[0],
                    eth1_conf->flow_opt[req->flow_id].mac_addr[1],
                    eth1_conf->flow_opt[req->flow_id].mac_addr[2],
                    eth1_conf->flow_opt[req->flow_id].mac_addr[3],
                    eth1_conf->flow_opt[req->flow_id].mac_addr[4],
                    eth1_conf->flow_opt[req->flow_id].mac_addr[5]);
            printf("match_select = %d, vlan_chk = %d, num_tag = %d, range_mode = %d\n",
                     eth1_conf->flow_opt[req->flow_id].addr_match_select,
                     eth1_conf->flow_opt[req->flow_id].vlan_check,
                     eth1_conf->flow_opt[req->flow_id].num_tag,
                     eth1_conf->flow_opt[req->flow_id].tag_range_mode);
            printf("tag1_type = %d, tag2_type = %d, tag1_lower = %d, upper = %d, tag2_lower = %d, upper = %d\n",
                    eth1_conf->flow_opt[req->flow_id].outer_tag_type,
                    eth1_conf->flow_opt[req->flow_id].inner_tag_type,
                    eth1_conf->flow_opt[req->flow_id].outer_tag.range.lower,
                    eth1_conf->flow_opt[req->flow_id].outer_tag.range.upper,
                    eth1_conf->flow_opt[req->flow_id].inner_tag.range.lower,
                    eth1_conf->flow_opt[req->flow_id].inner_tag.range.upper);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}
static void cli_cmd_1588_engine_eth2_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth2_conf = &flow_conf->flow_conf.ptp.eth2_opt;
    } else {
        eth2_conf = &flow_conf->flow_conf.oam.eth2_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth2_conf->comm_opt.etype = (req->etype ? req->etype : eth2_conf->comm_opt.etype);
            eth2_conf->comm_opt.tpid = (req->tpid ? req->tpid : eth2_conf->comm_opt.tpid);
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("ETH2 conf: etype = 0x%x, tpid = 0x%x\n",
                    eth2_conf->comm_opt.etype,
                    eth2_conf->comm_opt.tpid);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_eth2_flow_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_eth_conf_t *eth2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    if (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (req->eng_id < VTSS_PHY_TS_OAM_ENGINE_ID_2A) {
        eth2_conf = &flow_conf->flow_conf.ptp.eth2_opt;
    } else {
        eth2_conf = &flow_conf->flow_conf.oam.eth2_opt;
    }
    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            eth2_conf->flow_opt[req->flow_id].flow_en = req->enable;
            if (req->mac_match_mode_spec) {
                eth2_conf->flow_opt[req->flow_id].addr_match_mode = req->mac_match_mode;
            }
            if (req->ptp_mac_spec) {
                memcpy(eth2_conf->flow_opt[req->flow_id].mac_addr, req->ptp_mac, 6);
            }
            if (req->addr_match_select_spec) {
                if (req->addr_match_select == 0) {
                    eth2_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_ADDR;
                } else if (req->addr_match_select == 1) {
                    eth2_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_DEST_ADDR;
                } else {
                    eth2_conf->flow_opt[req->flow_id].addr_match_select = VTSS_PHY_TS_ETH_MATCH_SRC_OR_DEST;
                }
            }
            if (req->vlan_chk_spec) {
                eth2_conf->flow_opt[req->flow_id].vlan_check = req->vlan_chk;
            }
            if (req->num_tag_spec) {
                eth2_conf->flow_opt[req->flow_id].num_tag = req->num_tag;
            }
            if (req->tag_rng_mode_spec) {
                eth2_conf->flow_opt[req->flow_id].tag_range_mode = req->tag_rng_mode;
            }
            if (req->tag1_type > 0 && req->tag1_type < 5) {
                eth2_conf->flow_opt[req->flow_id].outer_tag_type = req->tag1_type;
            }
            if (req->tag2_type > 0 && req->tag2_type < 5) {
                eth2_conf->flow_opt[req->flow_id].inner_tag_type = req->tag2_type;
            }

            if (req->tag_rng_mode_spec && req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_OUTER) {
                if (req->tag1_lower) {
                    eth2_conf->flow_opt[req->flow_id].outer_tag.range.lower = req->tag1_lower;
                }
                if (req->tag1_upper) {
                    eth2_conf->flow_opt[req->flow_id].outer_tag.range.upper = req->tag1_upper;
                }
            } else {
                eth2_conf->flow_opt[req->flow_id].outer_tag.value.val = req->tag1_lower;
                eth2_conf->flow_opt[req->flow_id].outer_tag.value.mask = req->tag1_upper;
            }
            if (req->tag_rng_mode_spec && req->tag_rng_mode == VTSS_PHY_TS_TAG_RANGE_INNER) {
                if (req->tag2_lower) {
                    eth2_conf->flow_opt[req->flow_id].inner_tag.range.lower = req->tag2_lower;
                }
                if (req->tag2_upper) {
                    eth2_conf->flow_opt[req->flow_id].inner_tag.range.upper = req->tag2_upper;
                }
            } else {
                eth2_conf->flow_opt[req->flow_id].inner_tag.value.val = req->tag2_lower;
                eth2_conf->flow_opt[req->flow_id].inner_tag.value.mask = req->tag2_upper;
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("ETH2 flow conf: enable = %d, match_mode = %d, mac = 0x%x-%x-%x-%x-%x-%x\n",
                    eth2_conf->flow_opt[req->flow_id].flow_en,
                    eth2_conf->flow_opt[req->flow_id].addr_match_mode,
                    eth2_conf->flow_opt[req->flow_id].mac_addr[0],
                    eth2_conf->flow_opt[req->flow_id].mac_addr[1],
                    eth2_conf->flow_opt[req->flow_id].mac_addr[2],
                    eth2_conf->flow_opt[req->flow_id].mac_addr[3],
                    eth2_conf->flow_opt[req->flow_id].mac_addr[4],
                    eth2_conf->flow_opt[req->flow_id].mac_addr[5]);
            printf("match_select = %d, vlan_chk = %d, num_tag = %d, range_mode = %d\n",
                     eth2_conf->flow_opt[req->flow_id].addr_match_select,
                     eth2_conf->flow_opt[req->flow_id].vlan_check,
                     eth2_conf->flow_opt[req->flow_id].num_tag,
                     eth2_conf->flow_opt[req->flow_id].tag_range_mode);
            printf("tag1_type = %d, tag2_type = %d, tag1_lower = %d, upper = %d, tag2_lower = %d, upper = %d\n",
                    eth2_conf->flow_opt[req->flow_id].outer_tag_type,
                    eth2_conf->flow_opt[req->flow_id].inner_tag_type,
                    eth2_conf->flow_opt[req->flow_id].outer_tag.range.lower,
                    eth2_conf->flow_opt[req->flow_id].outer_tag.range.upper,
                    eth2_conf->flow_opt[req->flow_id].inner_tag.range.lower,
                    eth2_conf->flow_opt[req->flow_id].inner_tag.range.upper);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_ip1_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;
    u32 i;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    ip1_conf = &flow_conf->flow_conf.ptp.ip1_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (ip1_conf->comm_opt.ip_mode != req->ip_mode) {
            for (i = 0; i < 8; i++) {
                if (ip1_conf->flow_opt[i].flow_en) {
                    free(flow_conf);
                    printf("IP mode error! disable all the flows before changing the mode\n");
                    return;
                }
            }
        }
        if (req->set == TRUE) {
            if (req->ip_mode_spec) {
                ip1_conf->comm_opt.ip_mode = req->ip_mode;
            }
            if (req->sport_spec) {
                ip1_conf->comm_opt.sport_val = req->sport_val;
                ip1_conf->comm_opt.sport_mask = req->sport_mask;
            }
            if (req->dport_spec) {
                ip1_conf->comm_opt.dport_val = req->dport_val;
                ip1_conf->comm_opt.dport_mask = req->dport_mask;
            }
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("IP1 conf: mode = %d, sport_val = %d, mask = 0x%x, dport_val = %d, mask = 0x%x\n",
                    ip1_conf->comm_opt.ip_mode,
                    ip1_conf->comm_opt.sport_val, ip1_conf->comm_opt.sport_mask,
                    ip1_conf->comm_opt.dport_val, ip1_conf->comm_opt.dport_mask);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_ip1_flow_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip1_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        printf("Engine not initialized!\n");
        return;
    }

    if (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    ip1_conf = &flow_conf->flow_conf.ptp.ip1_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if ((ip1_conf->comm_opt.ip_mode != req->ip_mode)&& req->enable) {
                free(flow_conf);
                printf("IP mode mismatch!\n");
                return;
            }

            ip1_conf->flow_opt[req->flow_id].flow_en = req->enable;
            if (req->addr_match_select_spec) {
                if (req->addr_match_select == 0) {
                    ip1_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                } else if (req->addr_match_select == 1) {
                    ip1_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                } else {
                    ip1_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC_OR_DEST;
                }
            }
            if (ip1_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                if (req->ipv4_addr_spec) {
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv4.addr = req->ipv4_addr;
                }
                if (req->ipv4_mask_spec) {
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv4.mask = req->ipv4_mask;
                }
            } else {         
                if (req->ipv4_addr_spec) {
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]= req->ipv6_addr[0];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1]= req->ipv6_addr[1];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2]= req->ipv6_addr[2];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3]= req->ipv6_addr[3];
                }
                if (req->ipv4_mask_spec) {
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]= req->ipv6_mask[0];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1]= req->ipv6_mask[1];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2]= req->ipv6_mask[2];
                    ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3]= req->ipv6_mask[3];
                }
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            if (ip1_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {  
                printf("IP1 flow_conf: enable = %d, match_mode = %d, addr = 0x%x, mask = 0x%x\n",
                    ip1_conf->flow_opt[req->flow_id].flow_en,
                    ip1_conf->flow_opt[req->flow_id].match_mode,
                    (unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv4.addr,
                    (unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv4.mask);
            } else {
                printf("IP1 flow_conf: enable = %d, match_mode = %d, addr = %x:%x:%x:%x:%x:%x:%x:%x, mask = %x:%x:%x:%x:%x:%x:%x:%x\n",
                    ip1_conf->flow_opt[req->flow_id].flow_en,
                    ip1_conf->flow_opt[req->flow_id].match_mode,
                    (unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]&0xFFFF,
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3]>>16) & 0xFFFF),
                    (unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]&0xFFFF,
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2]>>16) & 0xFFFF),
                    ((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3] & 0xFFFF),
                    (((unsigned int)ip1_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3]>>16) & 0xFFFF));
            }
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_ip2_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;
    u32 i;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    ip2_conf = &flow_conf->flow_conf.ptp.ip2_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (ip2_conf->comm_opt.ip_mode != req->ip_mode) {
                for (i = 0; i < 8; i++) {
                    if (ip2_conf->flow_opt[i].flow_en) {
                        free(flow_conf);
                        printf("IP mode error! disable all the flows before changing the mode\n");
                        return;
                    }
                }
            }

            if (req->ip_mode_spec) {
                ip2_conf->comm_opt.ip_mode = req->ip_mode;
            }
            if (req->sport_spec) {
                ip2_conf->comm_opt.sport_val = req->sport_val;
                ip2_conf->comm_opt.sport_mask = req->sport_mask;
            }
            if (req->dport_spec) {
                ip2_conf->comm_opt.dport_val = req->dport_val;
                ip2_conf->comm_opt.dport_mask = req->dport_mask;
            }
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("IP2 conf: mode = %d, sport_val = %d, mask = 0x%x, dport_val = %d, mask = 0x%x\n",
                    ip2_conf->comm_opt.ip_mode,
                    ip2_conf->comm_opt.sport_val, ip2_conf->comm_opt.sport_mask,
                    ip2_conf->comm_opt.dport_val, ip2_conf->comm_opt.dport_mask);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_ip2_flow_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ip_conf_t *ip2_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        printf("Engine not initialized!\n");
        return;
    }

    if (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    ip2_conf = &flow_conf->flow_conf.ptp.ip2_opt;

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if ((ip2_conf->comm_opt.ip_mode != req->ip_mode)&& req->enable) {
                free(flow_conf);
                printf("IP mode mismatch!\n");
                return;
            }

            ip2_conf->flow_opt[req->flow_id].flow_en = req->enable;
            if (req->addr_match_select_spec) {
                if (req->addr_match_select == 0) {
                    ip2_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC;
                } else if (req->addr_match_select == 1) {
                    ip2_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_DEST;
                } else {
                    ip2_conf->flow_opt[req->flow_id].match_mode = VTSS_PHY_TS_IP_MATCH_SRC_OR_DEST;
                }
            }
        if (ip2_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {
                if (req->ipv4_addr_spec) {
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv4.addr = req->ipv4_addr;
                }
                if (req->ipv4_mask_spec) {
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv4.mask = req->ipv4_mask;
                }
            } else {

                if (req->ipv4_addr_spec) {
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]= req->ipv6_addr[0];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1]= req->ipv6_addr[1];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2]= req->ipv6_addr[2];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3]= req->ipv6_addr[3];
                }
                if (req->ipv4_mask_spec) {
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]= req->ipv6_mask[0];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1]= req->ipv6_mask[1];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2]= req->ipv6_mask[2];
                    ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3]= req->ipv6_mask[3];
                }
            }
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            if (ip2_conf->comm_opt.ip_mode == VTSS_PHY_TS_IP_VER_4) {  
                printf("IP2 flow_conf: enable = %d, match_mode = %d, addr = 0x%x, mask = 0x%x\n",
                    ip2_conf->flow_opt[req->flow_id].flow_en,
                    ip2_conf->flow_opt[req->flow_id].match_mode,
                    (unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv4.addr,
                    (unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv4.mask);
            } else {
                 printf("IP2 flow_conf: enable = %d, match_mode = %d, addr = %x:%x:%x:%x:%x:%x:%x:%x, mask = %x:%x:%x:%x:%x:%x:%x:%x\n",
                        ip2_conf->flow_opt[req->flow_id].flow_en,
                        ip2_conf->flow_opt[req->flow_id].match_mode,
                        (unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]&0xFFFF,
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[0]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[1]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[2]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.addr[3]>>16) & 0xFFFF),
                        (unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]&0xFFFF,
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[0]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[1]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[2]>>16) & 0xFFFF),
                        ((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3] & 0xFFFF),
                        (((unsigned int)ip2_conf->flow_opt[req->flow_id].ip_addr.ipv6.mask[3]>>16) & 0xFFFF));
            }
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_mpls_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_mpls_conf_t *mpls_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM ){ 
        mpls_conf = &flow_conf->flow_conf.oam.mpls_opt;
    } else if((engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP) || 
        (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP)) {
        mpls_conf = &flow_conf->flow_conf.ptp.mpls_opt;
    } else { 
        printf("Invalid Operation: Engine encapsulation type !\n");
        free(flow_conf);
        return;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (mpls_conf->comm_opt.cw_en == req->cw_en){
                free(flow_conf);
                printf("Success\n");
                return;
            }
            mpls_conf->comm_opt.cw_en = req->cw_en;
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("MPLS Control Word: enable = %d\n", mpls_conf->comm_opt.cw_en);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_mpls_flow_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_mpls_conf_t *mpls_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;

    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    if (req->flow_id < engine_init_info[port_no][dir][eng_id].flow_st_index ||
        req->flow_id > engine_init_info[port_no][dir][eng_id].flow_end_index) {
        printf("Invalid flow ID!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM || 
        engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        mpls_conf = &flow_conf->flow_conf.oam.mpls_opt;
    } else {
        mpls_conf = &flow_conf->flow_conf.ptp.mpls_opt;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            mpls_conf->flow_opt[req->flow_id].flow_en = req->enable;
            if (req->stk_depth_spec) {
                mpls_conf->flow_opt[req->flow_id].stack_depth = req->stk_depth;
            }
            if (req->stk_ref_point_spec) {
                mpls_conf->flow_opt[req->flow_id].stack_ref_point = req->stk_ref_point;
            }
            if (req->stk_lvl_0) {
                if (mpls_conf->flow_opt[req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.top.lower = req->stk_lvl_0_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.top.upper = req->stk_lvl_0_upper;
                } else {
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.end.lower = req->stk_lvl_0_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.end.upper = req->stk_lvl_0_upper;
                }
            }
            if (req->stk_lvl_1) {
                if (mpls_conf->flow_opt[req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.frst_lvl_after_top.lower = req->stk_lvl_1_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.frst_lvl_after_top.upper = req->stk_lvl_1_upper;
                } else {
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.frst_lvl_before_end.lower = req->stk_lvl_1_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.frst_lvl_before_end.upper = req->stk_lvl_1_upper;
                }
            }
            if (req->stk_lvl_2) {
                if (mpls_conf->flow_opt[req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.snd_lvl_after_top.lower = req->stk_lvl_2_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.snd_lvl_after_top.upper = req->stk_lvl_2_upper;
                } else {
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.snd_lvl_before_end.lower = req->stk_lvl_2_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.snd_lvl_before_end.upper = req->stk_lvl_2_upper;
                }
            }
            if (req->stk_lvl_3) {
                if (mpls_conf->flow_opt[req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.thrd_lvl_after_top.lower = req->stk_lvl_3_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.top_down.thrd_lvl_after_top.upper = req->stk_lvl_3_upper;
                } else {
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.lower = req->stk_lvl_3_lower;
                    mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.upper = req->stk_lvl_3_upper;
                }
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("MPLS flow_conf: enable = %d, stack_depth = %d, ref_point = %d\n",
                     mpls_conf->flow_opt[req->flow_id].flow_en,
                     mpls_conf->flow_opt[req->flow_id].stack_depth,
                     mpls_conf->flow_opt[req->flow_id].stack_ref_point);
            if (mpls_conf->flow_opt[req->flow_id].stack_ref_point == VTSS_PHY_TS_MPLS_STACK_REF_POINT_TOP) {
                printf("level_0 = %u - %u, level_1 = %u - %u, level_2 = %u - %u, level_3 = %u - %u\n",
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.top.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.top.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.frst_lvl_after_top.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.frst_lvl_after_top.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.snd_lvl_after_top.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.snd_lvl_after_top.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.thrd_lvl_after_top.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.top_down.thrd_lvl_after_top.upper);
            } else {
                printf("level_0 = %u - %u, level_1 = %u - %u, level_2 = %u - %u, level_3 = %u - %u\n",
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.end.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.end.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.frst_lvl_before_end.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.frst_lvl_before_end.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.snd_lvl_before_end.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.snd_lvl_before_end.upper,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.lower,
                     mpls_conf->flow_opt[req->flow_id].stack_level.bottom_up.thrd_lvl_before_end.upper);
            }
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_ach_comm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_ach_conf_t *ach_conf;
    vtss_phy_ts_engine_flow_conf_t *flow_conf, *conf_ptr;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE ||
        req->eng_id > VTSS_PHY_TS_PTP_ENGINE_ID_1) {
        printf("Engine not initialized!\n");
        return;
    }

    flow_conf = malloc(sizeof(vtss_phy_ts_engine_flow_conf_t));
    if (flow_conf == NULL) {
        printf("Failed!\n");
        return;
    }

    if (engine_init_info[port_no][dir][eng_id].encap_type == VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM) {
        ach_conf = &flow_conf->flow_conf.oam.ach_opt;
    } else {
        ach_conf = &flow_conf->flow_conf.ptp.ach_opt;
    }

    conf_ptr = flow_conf; /* using alt ptr to engine_conf_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_conf_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (req->set == TRUE) {
            if (req->ach_ver_spec) {
                ach_conf->comm_opt.version.value = req->ach_ver;
                ach_conf->comm_opt.version.mask = 0xF;
            }
            if (req->channel_type_spec) {
                ach_conf->comm_opt.channel_type.value = req->channel_type;
                ach_conf->comm_opt.channel_type.mask = 0xFFFF;
            }
            if (req->proto_id_spec) {
                ach_conf->comm_opt.proto_id.value = req->proto_id;
                ach_conf->comm_opt.proto_id.mask = 0xFFFF;
            }

            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_conf_set(API_INST_DEFAULT, port_no, eng_id, flow_conf);
            }
        } else {
            printf("ACH conf: ver = %d, chan_type = %d, proto_id = %d\n",
                    ach_conf->comm_opt.version.value,
                    ach_conf->comm_opt.channel_type.value,
                    ach_conf->comm_opt.proto_id.value);
        }
    }

    free(flow_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_action_add(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    action_conf = malloc(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp == TRUE && req->ptp_spec == TRUE) {
            if (req->action_id < 3) {
                ptp_action = &action_conf->action.ptp_conf[req->action_id];
                ptp_action->enable = TRUE;
                ptp_action->channel_map = req->channel_map;
                ptp_action->clk_mode = req->clk_mode;
                ptp_action->delaym_type = req->delaym;
                ptp_action->ptp_conf.range_en = TRUE;
                ptp_action->ptp_conf.domain.range.lower = req->domain_meg_lower;
                ptp_action->ptp_conf.domain.range.upper = req->domain_meg_upper;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else if (action_conf->action_ptp == FALSE && req->y1731_oam_spec == TRUE) {
            if (req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[req->action_id];
                oam_action->enable = TRUE;
                oam_action->channel_map = req->channel_map;
                oam_action->version     = req->version;
                oam_action->y1731_en    = TRUE;
                oam_action->oam_conf.y1731_oam_conf.delaym_type = req->delaym;
                oam_action->oam_conf.y1731_oam_conf.range_en = TRUE;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower = req->domain_meg_lower;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper = req->domain_meg_upper;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else if (action_conf->action_ptp == FALSE && req->ietf_oam_spec == TRUE) {
            if (req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[req->action_id];
                oam_action->enable = TRUE;
                oam_action->channel_map = req->channel_map;
                oam_action->version     = req->version;
                oam_action->y1731_en    = FALSE;
                oam_action->oam_conf.ietf_oam_conf.delaym_type = req->delaym;
                oam_action->oam_conf.ietf_oam_conf.ts_format = req->ietf_tf;
                oam_action->oam_conf.ietf_oam_conf.ds = req->ietf_ds;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else {
            rc = VTSS_RC_ERROR;
        }

        if (rc == VTSS_RC_OK) {
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            }
        }
    }

    free(action_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_action_delete(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    action_conf = malloc(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp) {
            if (req->action_id < 3) {
                ptp_action = &action_conf->action.ptp_conf[req->action_id];
                ptp_action->enable = FALSE;
                ptp_action->channel_map = 0;
                ptp_action->clk_mode = 0;
                ptp_action->delaym_type = 0;
                ptp_action->ptp_conf.range_en = FALSE;
                ptp_action->ptp_conf.domain.range.lower = 0;
                ptp_action->ptp_conf.domain.range.upper = 0;
            } else {
                rc = VTSS_RC_ERROR;
            }
        } else {
            if (req->action_id < 6) {
                oam_action = &action_conf->action.oam_conf[req->action_id];
                oam_action->enable = FALSE;
                oam_action->channel_map = 0;
                oam_action->oam_conf.y1731_oam_conf.delaym_type = 0;
                oam_action->oam_conf.y1731_oam_conf.range_en = FALSE;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower = 0;
                oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper = 0;
            } else {
                rc = VTSS_RC_ERROR;
            }
        }

        if (rc == VTSS_RC_OK) {
            if (req->ingress) {
                rc = vtss_phy_ts_ingress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            } else {
                rc = vtss_phy_ts_egress_engine_action_set(API_INST_DEFAULT, port_no, eng_id, action_conf);
            }
        }
    }

    free(action_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

static void cli_cmd_1588_engine_action_show(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL dir = (req->ingress ? 0 : 1);
    u8 eng_id = req->eng_id;
    vtss_rc rc;
    vtss_phy_ts_engine_action_t *action_conf, *conf_ptr;
    vtss_phy_ts_ptp_engine_action_t *ptp_action;
    vtss_phy_ts_oam_engine_action_t *oam_action;
    int i;


    port_no = uport2iport(req->port_no);
    if (engine_init_info[port_no][dir][eng_id].init == FALSE) {
        printf("Engine not initialized!\n");
        return;
    }

    action_conf = malloc(sizeof(vtss_phy_ts_engine_action_t));
    if (action_conf == NULL) {
        printf("Failed!\n");
        return;
    }
    conf_ptr = action_conf;  /* using alt ptr to engine_action_get, otherwise Lint complains on custody problem i.e. error 429 */
    if (req->ingress) {
        rc = vtss_phy_ts_ingress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    } else {
        rc = vtss_phy_ts_egress_engine_action_get(req->api_inst, port_no, eng_id, conf_ptr);
    }

    if (rc == VTSS_RC_OK) {
        if (action_conf->action_ptp) {
            printf("***PTP action***\n");
            for (i = 0; i < 2; i++) {
                ptp_action = &action_conf->action.ptp_conf[i];
                printf("id = %d, enable = %d, channel_map = %d, clk_mode = %d, delaym = %d\n",
                         i, ptp_action->enable, ptp_action->channel_map,
                         ptp_action->clk_mode, ptp_action->delaym_type);
                printf("domain range_en = %d, domain range lower = %d, upper = %d\n\n",
                        ptp_action->ptp_conf.range_en,
                        ptp_action->ptp_conf.domain.range.lower,
                        ptp_action->ptp_conf.domain.range.upper);
            }
        } else {
            printf("***OAM action***\n");
            for (i = 0; i < 6; i++) {
                oam_action = &action_conf->action.oam_conf[i];
                printf("id = %d, enable = %d, channel_map = %d, delaym = %d\n",
                        i, oam_action->enable,
                        oam_action->channel_map,
                        oam_action->oam_conf.y1731_oam_conf.delaym_type);
                printf("MEG range_en = %d, MEG range lower = %d, upper = %d\n\n",
                        oam_action->oam_conf.y1731_oam_conf.range_en,
                        oam_action->oam_conf.y1731_oam_conf.meg_level.range.lower,
                        oam_action->oam_conf.y1731_oam_conf.meg_level.range.upper);
            }
        }
    }

    free(action_conf);
    if (rc == VTSS_RC_OK) {
        printf("Success\n");
     } else {
        printf("Failed\n");
     }
}

#define MISC_CLI_PHY_TS_SIG_MSG_TYPE_LEN       1
#define MISC_CLI_PHY_TS_SIG_DOMAIN_NUM_LEN     1
#define MISC_CLI_PHY_TS_SIG_SOURCE_PORT_ID_LEN 10
#define MISC_CLI_PHY_TS_SIG_SEQUENCE_ID_LEN    2
#define MISC_CLI_PHY_TS_SIG_DEST_IP_LEN        4
#define MISC_CLI_PHY_TS_SIG_SRC_IP_LEN         4
#define MISC_CLI_PHY_TS_SIG_DST_MAC_LEN        6

static void cli_cmd_1588_signature_conf(cli_req_t *req)
{
    u16 len = 0;
    u32 sig_mask = 0;
    vtss_port_no_t port_no;

    port_no = uport2iport(req->port_no);
    sig_mask = req->sig_mask;
    if (req->set) {

        if ( req->sig_mask & VTSS_PHY_TS_FIFO_SIG_MSG_TYPE) {
            len += MISC_CLI_PHY_TS_SIG_MSG_TYPE_LEN;
            printf("Msg Type |");
        }
        if ( req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DOMAIN_NUM) {
            len += MISC_CLI_PHY_TS_SIG_DOMAIN_NUM_LEN;
            printf("Domain Num |");
        }
        if (req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SOURCE_PORT_ID) {
            len += MISC_CLI_PHY_TS_SIG_SOURCE_PORT_ID_LEN;
            printf("Source Port ID |");
        }
        if (req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SEQ_ID) {
            len += MISC_CLI_PHY_TS_SIG_SEQUENCE_ID_LEN ;
            printf("Sequence ID |");
        }
        if ( req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_IP) {
            len += MISC_CLI_PHY_TS_SIG_DEST_IP_LEN;
            printf("Dest IP |");
        }
        if (req->sig_mask & VTSS_PHY_TS_FIFO_SIG_SRC_IP) {
            len += MISC_CLI_PHY_TS_SIG_SRC_IP_LEN;
            printf("Src IP |");
        }
        if (req->sig_mask & VTSS_PHY_TS_FIFO_SIG_DEST_MAC) {
            len += MISC_CLI_PHY_TS_SIG_DST_MAC_LEN;
            printf("Dest MAC |");
        }
        printf("\nLength : %d \n",len);
        if (vtss_phy_ts_fifo_sig_set(API_INST_DEFAULT, port_no, req->sig_mask) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_ts_fifo_sig_set() operation \n");
            return;
        }
    } else {
        if (vtss_phy_ts_fifo_sig_get(req->api_inst, port_no, (vtss_phy_ts_fifo_sig_mask_t *)&sig_mask) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_ts_fifo_sig_get() operation \n");
            return;
        }
        printf("Port : %d Signature : %x \n", port_no, sig_mask);
    }
}

static void cli_cmd_1588_ts_stats_show(cli_req_t *req)
{
    u16 i =0, j=0;
    vtss_port_no_t port_no,cfg_port;
    vtss_port_no_t ts_port_no[6] = {0};

    vtss_phy_ts_stats_t ts_stats[6];
    u16   num_ports=6, num_sec=5;
    memset(ts_stats, 0, sizeof(vtss_phy_ts_stats_t)*6);
    port_no = uport2iport(req->port_no);
    if (req->time_sec){
        num_sec = req->time_sec;
    }
    
    for (j = 0; j<6; j++) {
        ts_port_no[j] = 0;
    }
    for (j=0;j<num_sec; j++) {
        for (port_no = 21, i=0; port_no <= 26; port_no++) {
            cfg_port = uport2iport(port_no);
            if ( vtss_phy_ts_stats_get(req->api_inst, cfg_port, &ts_stats[i]) == VTSS_RC_OK) {
                 ts_port_no[i++] = port_no;
            }
        }
        num_ports = i;
        for (i=0;i<num_ports; i++) {
             printf("Port : %u\n", ts_port_no[i]);
             printf("Frames with preambles too short to shrink  Ingress        : %u\n",ts_stats[i].ingr_pream_shrink_err);
             printf("Frames with preambles too short to shrink  Egress         : %u\n",ts_stats[i].egr_pream_shrink_err );
             printf("Timestamp block received frame with FCS error in ingress  : %u\n",ts_stats[i].ingr_fcs_err );
             printf("Timestamp block received frame with FCS error in egress   : %u\n",ts_stats[i].egr_fcs_err );
             printf("No of frames modified by timestamp block in ingress       : %u\n",ts_stats[i].ingr_frm_mod_cnt );
             printf("No of frames modified by timestamp block in egress        : %u\n",ts_stats[i].egr_frm_mod_cnt );
             printf("The number of timestamps transmitted to the SPI interface : %u\n",ts_stats[i].ts_fifo_tx_cnt );
             printf("Count of dropped Timestamps not enqueued to the Tx TSFIFO : %u\n",ts_stats[i].ts_fifo_drop_cnt );
        }
        VTSS_MSLEEP(1000);
    }
}
static void cli_cmd_1588_latency(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    BOOL                       error = FALSE;
    vtss_timeinterval_t        latency_val;
    u32                        temp;
   
    port_no = uport2iport(req->port_no);
    if (req->set) {
        if (req->ingress) {
            if(vtss_phy_ts_ingress_latency_set(API_INST_DEFAULT, port_no, &(req->latency_val)) != VTSS_RC_OK) {
                error = TRUE;
            }        
        } else {
            if (vtss_phy_ts_egress_latency_set(API_INST_DEFAULT, port_no, &(req->latency_val)) != VTSS_RC_OK) {
                error = TRUE;
            }
        }
        if (!error) {
            printf("Set the Latency Successfully\n");
        } else {
            printf("Failed...! Set Operation is not processed\n");
        }
    } else {
        if(req->ingress) {
            if(vtss_phy_ts_ingress_latency_get(req->api_inst, port_no, &latency_val) != VTSS_RC_OK) {
                error = TRUE;
                printf("Failed...! Get Operation is not processed\n");
            }
        } else {
            if(vtss_phy_ts_egress_latency_get(req->api_inst, port_no, &latency_val) != VTSS_RC_OK) {
                error = TRUE;
                printf("Failed...! Get Operation is not processed\n");
            }
        }
        if(!error) {
            latency_val = latency_val >> 16;
            temp = (u32) latency_val;
            printf("Port  Ingress/Egress  Latency\n----  --------------  -------\n");
            printf("%-4d  %-14s  %u\n", req->port_no, req->ingress ? "Ingress" : "Egress", temp);
        }
    }
}

static void cli_cmd_1588_delay(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    vtss_timeinterval_t        delay_val;
    u32                        temp;

    port_no = uport2iport(req->port_no);
    if(req->set) {
         if(vtss_phy_ts_path_delay_set(API_INST_DEFAULT, port_no, &(req->delay_val)) != VTSS_RC_OK)  {
             printf("Failed to set the port delay for the port %d\n", req->port_no);
         } else {
             printf("Successfully set the port delay..\n");
         }
     } else {
         if(vtss_phy_ts_path_delay_get(req->api_inst, port_no, &delay_val) != VTSS_RC_OK)  {
             printf("Failed to get the port delay for the port %d\n", req->port_no);
         } else {
             temp = (u32)(delay_val >> 16);
             printf("Port  PathDelay\n----  ---------\n");
             printf("%-4d  %-u\n", req->port_no, temp);
         }
     }
}
static void cli_cmd_1588_delay_asym(cli_req_t *req)
{
    vtss_port_no_t             port_no;
    i32                        temp;
    vtss_timeinterval_t        asym_val;

    port_no = uport2iport(req->port_no);
    if(req->set) {
        if(vtss_phy_ts_delay_asymmetry_set(API_INST_DEFAULT, port_no, &(req->asym_val)) != VTSS_RC_OK) {
            printf("Failed to set the Delay Asymmetry for the port %d\n", req->port_no);
        } else {
            printf("Successfully set the delay asymmetry...\n");
        }
    } else {
        if(vtss_phy_ts_delay_asymmetry_get(req->api_inst, port_no, &asym_val) != VTSS_RC_OK) {
            printf("Failed to get the asymmetry delay for the port %d\n", req->port_no);
        } else {
            asym_val = asym_val >> 16;
            temp = (i32) asym_val;
            printf("Port  DelayAsym\n----  ---------\n");
            printf("%-4d  %d\n", req->port_no, temp);
        }
    }
}
static void cli_cmd_1588_mmd_read ( cli_req_t * req )
{
    vtss_port_no_t     port_no, port;
    BOOL               first = 1;
    u32                value = 0;

    req->set = 0;
    req->phy_mmd_access = TRUE;
    printf(" Port 2 : Channel-0 :: Port 3: Channel-1 \n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        
#if 0
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) {
            continue;
        }
#endif /* 0 */
        if (req->port_list[port] == 0) {
            continue;
        }
        T_D("port_no: %u Blk_id: %d Offset: %x\n\r",
                          port_no,(int)req->blk_id_1588, req->csr_reg_offset_1588);

        if (vtss_phy_1588_csr_reg_read(NULL, port_no, req->blk_id_1588,
                        req->csr_reg_offset_1588, &value) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_1588_csr_reg_read() operation \n");
            continue;
        }


        if (first) {
            printf("%-12s %-12s %-12s %-12s\n","Port","Blk-Id","CSR-Offset","Value");
            printf("%-12s %-12s %-12s %-12s\n","----","------","----------","-----");
            first = 0;
        }

        printf("%-12u %-12u %-12x %-12x\n", port, req->blk_id_1588, req->csr_reg_offset_1588, value);
    }
}

static void cli_cmd_1588_mmd_write ( cli_req_t * req )
{
    vtss_port_no_t port_no, port;
    
    req->set = 1;
    req->phy_mmd_access = TRUE;

    printf(" Port 2 : Channel-0 :: Port 3 : Channel-1 \n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
#if 0
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no))
            continue;
#endif /* 0 */
        if (req->port_list[port] == 0){
            continue;
        } 
        T_D("port_no: %u Blk_id: %d Offset: %x, Value: %x\n\r",
                          port_no,(int)req->blk_id_1588, req->csr_reg_offset_1588, req->value);
        if (vtss_phy_1588_csr_reg_write(NULL, port_no, req->blk_id_1588,
                        req->csr_reg_offset_1588, (u32 *)&req->value) != VTSS_RC_OK) {
            printf("Could not perform vtss_phy_1588_csr_reg_write() operation \n");
            continue;
        }
    }
}

#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_FEATURE_OTN)
static void cli_cmd_otn_otu_tti(cli_req_t * req)
{
    vtss_otn_otu_tti_t tti;
    u16 first = 1;

    memset(&tti, 0, sizeof(vtss_otn_otu_tti_t));
    if(req->set) {
        if(req->enable) {
            if (req->otn_tti_set) {
                memcpy(&tti.tx_tti[0],req->otn_tti,64);
            }
            tti.tx_enable = req->enable;
            tti.tim_sapi = req->otn_sapi_tim;
            tti.tim_dapi = req->otn_dapi_tim;
            tti.tim_os = req->otn_os_tim;
            if(req->otn_extti_set) {
                memcpy(&tti.exp_tti[0],req->otn_extti,64);
            }
        } 

    } else {
        printf("OTN OTU SM Transmitted TTI\n");
    }

    if (req->set) {
        CLI_TST_RC(vtss_otn_otu_tti_set(req->api_inst,
                    req->port,
                    &tti));
    } else {
        CLI_TST_RC(vtss_otn_otu_tti_get(req->api_inst,
                    req->port,
                    &tti));
        if(first)
            cli_table_header("Port  TTI_Enable/Disable");
        printf("%-6u  %s\n", iport2uport(req->port), cli_bool_disp(tti.tx_enable));
        if(first) 
            cli_table_header("TXTTI");
        printf("%s\n",tti.tx_tti);
        if(first)
            cli_table_header("SAPI_TIM  DAPI_TIM  OS_TIM");
        printf("%-9s %-10s %s\n",cli_bool_disp(tti.tim_sapi),cli_bool_disp(tti.tim_dapi), cli_bool_disp(tti.tim_os));
        if(first)
            cli_table_header("EXPECTED TTI");
        printf("%s\n",tti.exp_tti);
        first = 0;
    }
}

static void cli_cmd_otn_odu_tti(cli_req_t * req)
{
    vtss_otn_odu_tti_t tti;
    u16 first = 1;

    memset(&tti, 0, sizeof(vtss_otn_odu_tti_t));
    if(req->set) {
        if(req->enable) {
            if (req->otn_tti_set) {
                memcpy(&tti.tx_tti[0],req->otn_tti,64);
            }
            tti.tx_enable = req->enable;
            tti.tim_sapi = req->otn_sapi_tim;
            tti.tim_dapi = req->otn_dapi_tim;
            tti.tim_os = req->otn_os_tim;
            if(req->otn_extti_set) {
                memcpy(&tti.exp_tti[0],req->otn_extti,64);
            }
        }
    } else {
        printf("OTN ODU Transmitted TTI\n");
    }

    if (req->set) {
        CLI_TST_RC(vtss_otn_odu_tti_set(req->api_inst,
                    req->port,
                    &tti));
    } else {
        CLI_TST_RC(vtss_otn_odu_tti_get(req->api_inst,
                    req->port,
                    &tti));
        if(first)
            cli_table_header("Port  TTI_Enable/Disable");
        printf("%-6u  %s\n", iport2uport(req->port), cli_bool_disp(tti.tx_enable));
        if(first)
            cli_table_header("TXTTI");
        printf("%s\n",tti.tx_tti);
        if(first)
            cli_table_header("SAPI_TIM  DAPI_TIM  OS_TIM");
        printf("%-9s %-10s %s\n",cli_bool_disp(tti.tim_sapi),cli_bool_disp(tti.tim_dapi), cli_bool_disp(tti.tim_os));
        if(first)
            cli_table_header("EXPECTED TTI");
        printf("%s\n",tti.exp_tti);
        first = 0;
    }
}
static void cli_cmd_otn_otu_acc_res(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_acc_res_t  res;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&res, 0, sizeof(res)); 
        if (vtss_otn_otu_acc_res_get (req->api_inst, port_no, &res) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_otu_acc_res_get(): %u\n",port);
            continue;
        }
            if(first) {
                cli_table_header("Port  Inconsistent  Value");
                first = 0;
            }
            printf("%-5u %-13s %u\n", port, res.inconsistent?"Yes":"No", res.value);
    }
}
static void cli_cmd_otn_otu_acc_smres(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_acc_smres_t  smres;
    u8 first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&smres, 0, sizeof(smres)); 
        if (vtss_otn_otu_acc_smres_get(req->api_inst, port_no, &smres) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_otu_acc_smres_get(): %u\n",port);
            continue;
        }
            if(first) {
                cli_table_header("Port  Inconsistent  Value");
                first = 0;
            }
            printf("%-5u %-13s %u\n", port, smres.inconsistent?"Yes":"No", smres.value);
    }
}
static void cli_cmd_otn_otu_acc_gcc0(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_acc_gcc0_t  gcc0;
    u8 first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&gcc0, 0, sizeof(gcc0)); 
        if (vtss_otn_otu_acc_gcc0_get(req->api_inst, port_no, &gcc0) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_otu_acc_gcc0_get(): %u\n",port);
            continue;
        }
            if(first) {
                cli_table_header("Port  Inconsistent  Value");
                first = 0;
            }
            printf("%-5u %-13s %u\n", port, gcc0.inconsistent?"Yes":"No", gcc0.value);
     }
}
static void cli_cmd_otn_otu_tx_res(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_tx_res_t  cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0)) {
            continue;
        }
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            cfg.value = req->otn_value;
            if (vtss_otn_otu_tx_res_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_res_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_otu_tx_res_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_res_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Value");
                first = 0;
            }
            printf("%-5u %-7s %u\n", port, cfg.enable?"Yes":"No", cfg.value);
        }

    }
}
static void cli_cmd_otn_otu_tx_smres(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_tx_smres_t  cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            cfg.value = req->otn_value;
            if (vtss_otn_otu_tx_smres_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_smres_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_otu_tx_smres_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_smres_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Value");
                first = 0;
            }
            printf("%-5u %-7s %u\n", port, cfg.enable?"Yes":"No", cfg.value);
        }
    }
}
static void cli_cmd_otn_otu_tx_gcc0(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_tx_gcc0_t  cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            cfg.value = req->otn_value;
            if (vtss_otn_otu_tx_gcc0_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_gcc0_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_otu_tx_gcc0_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_tx_gcc0_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Value");
                first = 0;
            }
            printf("%-5u %-7s %u\n", port, cfg.enable?"Yes":"No", cfg.value);
        }
    }
}
static void cli_cmd_otn_otu_oh_insertion(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_oh_ins_t   cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        memset(&cfg, 0, sizeof(cfg));
        if (req->set) {
            cfg.mode = req->oh_ins_mode;
            if (vtss_otn_otu_oh_insertion_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_oh_insertion_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_otu_oh_insertion_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_oh_insertion_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Mode");
                first = 0;
            }
            printf("%-5u %u\n", port, cfg.mode);
        }
    }
}
static void cli_cmd_otn_odu_oh_insertion(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_oh_ins_t   cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.mode = req->oh_ins_mode;
            if (vtss_otn_odu_oh_insertion_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_oh_insertion_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_oh_insertion_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_oh_insertion_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Mode");
                first = 0;
            }
            printf("%-5u %u\n", port, cfg.mode);
        }
    }
}
static void cli_cmd_otn_odu_tx_res(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_tx_res_t  cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            cfg.value = req->otn_value;
            if (vtss_otn_odu_tx_res_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_res_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_tx_res_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_res_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Value");
                first = 0;
            }
            printf("%-5u %-7s %u\n", port, cfg.enable?"Yes":"No", cfg.value);
        }
    }
}
static void cli_cmd_otn_odu_acc_res(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_acc_res_t  res;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&res, 0, sizeof(res)); 
        if (vtss_otn_odu_acc_res_get(req->api_inst, port_no, &res) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odu_acc_res_get(): %u\n",port);
            continue;
        }
            if(first) {
                cli_table_header("Port  Inconsistent  Value");
                first = 0;
            }
            printf("%-5u %-13s %u\n", port, res.inconsistent?"Yes":"No", res.value);
    }
}
static void cli_cmd_otn_odu_tx_exp(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_tx_exp_t  cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            cfg.value = req->otn_value;
            if (vtss_otn_odu_tx_exp_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_exp_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_tx_exp_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_exp_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Value");
                first = 0;
            }
            printf("%-5u %-7s %u\n", port, cfg.enable?"Yes":"No", cfg.value);
        }
    }
}
static void cli_cmd_otn_odu_acc_exp(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_acc_exp_t  res;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&res, 0, sizeof(res)); 
        if (vtss_otn_odu_acc_exp_get(req->api_inst, port_no, &res) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odu_acc_exp_get(): %u\n",port);
            continue;
        }
            if(first) {
                cli_table_header("Port  Inconsistent  Value");
                first = 0;
            }
            printf("%-5u %-13s %u\n", port, res.inconsistent?"Yes":"No", res.value);
    }
}
static void cli_cmd_otn_odu_tx_ftfl(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_tx_ftfl_t  cfg;
    u8 first = 1;
    u32 i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.enable = req->enable;
            memset(&cfg.value, 0, sizeof(cfg.value));
            cfg.value[req->ftfl_index-1] = req->ftfl_value;
            if (vtss_otn_odu_tx_ftfl_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_ftfl_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_tx_ftfl_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_tx_ftfl_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Enable  Index  Value");
                first = 0;
            }
            for (i=0; i<sizeof(cfg.value); ++i)     if (cfg.value[i])   break;
            if (i < sizeof(cfg.value))
                printf("%-4u  %-6s  %-5u  %-5u\n", port, cfg.enable?"Yes":"No", i+1, cfg.value[i]);
            else
                printf("%-4u  %-6s  %-5s  %s\n", port, cfg.enable?"Yes":"No", "None", "None");
        }
    }
}
static void cli_cmd_otn_odu_acc_ftfl(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_acc_ftfl_t  cfg;
    u8 first = 1;
    u32 i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (vtss_otn_odu_acc_ftfl_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odu_acc_ftfl_get(): %u\n",port);
            continue;
        }
        if(first) {
            cli_table_header("Port  Inconsistent  Index  Value");
            first = 0;
        }
        for (i=0; i<sizeof(cfg.value); ++i)     if (cfg.value[i])   break;
        if (i < sizeof(cfg.value))
            printf("%-4u  %-12s  %-5u  %-5u\n", port, cfg.inconsistent?"Yes":"No", i+1, cfg.value[i]);
        else
            printf("%-4u  %-12s  %-5s  %s\n", port, cfg.inconsistent?"Yes":"No", "None", "None");
    }
}
static void cli_cmd_otn_opu_oh_insertion(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_opu_oh_ins_t   cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.mode = req->opu_oh_ins_mode;
            if (vtss_otn_odu_opu_oh_insertion_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_opu_oh_insertion_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_opu_oh_insertion_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_opu_oh_insertion_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Mode");
                first = 0;
            }
            printf("%-5u %u\n", port, cfg.mode);
        }
    }
}
static void cli_cmd_otn_opu_test_insertion(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_opu_test_ins_t   cfg;
    u8 first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&cfg, 0, sizeof(cfg)); 
        if (req->set) {
            cfg.payload_value = req->otn_value;
            cfg.ins_payload = req->test_ins_mode;
            cfg.ins_payload_type = req->enable;
            if (vtss_otn_odu_opu_test_insertion_set(NULL, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_opu_test_insertion_set(): %u\n",port);
                continue;
            }
            /* Set not applicable */
        } else {
            if (vtss_otn_odu_opu_test_insertion_get(req->api_inst, port_no, &cfg) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_opu_test_insertion_get(): %u\n",port);
                continue;
            }
            if(first) {
                cli_table_header("Port  Insert Test Payload  Test Payload Type  Payload Value");
                first = 0;
            }
            printf("%-5u %-20u %-17u %u\n", port, cfg.ins_payload_type, cfg.ins_payload, cfg.payload_value);
        }
    }
}
static void cli_cmd_otn_och_fec(cli_req_t *req)
{
    vtss_port_no_t         port_no, port;
    vtss_otn_och_fec_t     fec;
    vtss_rc                rc;

    printf("OTN OCH FEC Configuration:\n");
    cli_table_header("Port  FEC type");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&fec, 0, sizeof(fec));
        CLI_TST_RC(vtss_otn_och_fec_get(req->api_inst, port_no, &fec));
        if (req->set) {
            fec.type = req->fec_type;
            rc = vtss_otn_och_fec_set(req->api_inst, port_no, &fec);
            if (rc != VTSS_RC_OK)
                continue;
        }
        switch (fec.type) {
            case VTSS_OTN_OCH_FEC_RS: printf("%-4u  %-8s\n", port, "RS  "); break;
            case VTSS_OTN_OCH_FEC_I4: printf("%-4u  %-8s\n", port, "I4  "); break;
            case VTSS_OTN_OCH_FEC_I7: printf("%-4u  %-8s\n", port, "I7  "); break;
            default:                  printf("%-4u  %-8s\n", port, "None"); break;
        }
    }
}

static void cli_cmd_otn_och_ber_threshold(cli_req_t *req)
{
    vtss_port_no_t     port_no, port;
    vtss_otn_och_fec_t fec;

    printf("OTN OCH FEC Configuration:\n");
    cli_table_header("Port  Ber Threshold");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&fec, 0, sizeof(fec));
        if(vtss_otn_och_ber_threshold_get(req->api_inst, port_no, &fec) != VTSS_RC_OK) {
            printf("ber threshold failed\n");
            return;
        }
        if (req->set) {
            fec.type = req->fec_type;
            fec.ber_threshold = req->ber_threshold;
            if(vtss_otn_och_ber_threshold_set(req->api_inst, port_no, &fec) != VTSS_RC_OK) {
                printf("ber threshold set failed\n");
                return;
            }
        }
        if (fec.type == VTSS_OTN_OCH_FEC_RS) {
            if (fec.ber_threshold == 1) {
                printf("%-4u  %-8s\n", port, "10^-3");
            }
            else 
                printf("%-4u  %-8s\n", port, "10^-4");
        }
        if (fec.type == VTSS_OTN_OCH_FEC_I7) 
            printf("%-4u  0x%x\n", port, fec.ber_threshold);
    }
}
     
static void cli_cmd_otn_och_loopback(cli_req_t *req)
{
    vtss_port_no_t          port_no,port;
    vtss_otn_och_loop_t     och_loop;

    printf("OTN OCH Loop Configuration:\n");
    cli_table_header("Port  Line_side_equip  Line_Side_Line  Sys_side_Equip  Sys_side_line");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&och_loop, 0, sizeof(och_loop));
        CLI_TST_RC(vtss_otn_och_loopback_get(req->api_inst, port_no, &och_loop));
        if (req->set) {
            if (req->xxx_set_lb) {
                och_loop.line_side_equipment_loopback = req->xxx_line_side_equip_lb;
                och_loop.line_side_line_loopback = req->xxx_line_side_line_lb;
                och_loop.system_side_line_loopback = req->xxx_sys_side_line_lb;
                och_loop.system_side_equipment_loopback = req->xxx_sys_side_equip_lb;
                if (och_loop.line_side_equipment_loopback && och_loop.system_side_equipment_loopback) {
                    printf("Error..! Both TX facility and TX terminal loopbacks cannot be configured parallelly\n");
                    break;
                }
                if (och_loop.line_side_line_loopback && och_loop.system_side_line_loopback) {
                    printf("Error..! Both RX facility and RX terminal loopbackss cannot be configured parallelly\n");
                    break;
                }
            }
            CLI_TST_RC(vtss_otn_och_loopback_set(req->api_inst, port_no, &och_loop));
        }
        printf("%-4u  %-15s  %-14s  %-14s  %-13s\n", port, cli_bool_disp(och_loop.line_side_equipment_loopback), cli_bool_disp(och_loop.line_side_line_loopback),
                                                      cli_bool_disp(och_loop.system_side_equipment_loopback), cli_bool_disp(och_loop.system_side_line_loopback));
    }
}

static void cli_cmd_otn_och_defects(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_och_defects_t  defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_otn_och_defects_get(req->api_inst, port_no, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_och_defects_get(): %u\n",port);
            continue;
        }

        printf("OTN OCH Defects:\n\n");
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Loss Of Signal              :",defects.dlos?"Yes":"No");
        printf("%-30s %-12s\n", "  Loss Of Frame               :",defects.dlof?"Yes":"No");
        printf("%-30s %-12s\n", "  Loss of Multiframe          :",defects.dlom?"Yes":"No");
        printf("%-30s %-12s\n", "  Alarm Indication Signal     :",defects.dais?"Yes":"No");
        printf("%-30s %-12s\n", "  Out of frame Signal         :",defects.doof?"Yes":"No");
        printf("%-30s %-12s\n", "  Out of Multi-frame Signal   :",defects.doom?"Yes":"No");
    }
}

static void cli_cmd_otn_och_perform(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_och_perf_t perf;

    memset(&perf, 0, sizeof(perf));
    printf("OTN OCH performance:\n");
    cli_table_header("Port  corrected sym  corrected zero  corrected one  uncorrected");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_otn_och_perf_get(req->api_inst, port_no, &perf)) {
            printf("%-4u  %-13u  %-14u  %-13u  %-12u\n", port, perf.pfec_corrected_symbols, perf.pfec_corrected_zero, perf.pfec_corrected_one, perf.pfec_uncorrectable_codewords);
        }
    }
}

static void cli_cmd_otn_otu_defects(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_otu_defects_t  defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_otn_otu_defects_get(req->api_inst, port_no, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_otu_defects_get(): %u\n",port);
            continue;
        }

        printf("OTN OTU Defects:\n\n");
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Trail Trace Identifier Mismatch     :",defects.dtim?"Yes":"No");
        printf("%-30s %-12s\n", "  Backward Defect Indication          :",defects.dbdi?"Yes":"No");
        printf("%-30s %-12s\n", "  Incoming Alignment Error            :",defects.diae?"Yes":"No");
        printf("%-30s %-12s\n", "  Backward Incoming Alignment Error   :",defects.dbiae?"Yes":"No");
    }
}

static void cli_cmd_otn_odu_defects(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_defects_t  defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_otn_odu_defects_get(req->api_inst, port_no, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odu_defects_get(): %u\n",port);
            continue;
        }

        printf("OTN ODU Defects:\n\n");
        printf("Port: %u\n",port);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Trail Trace Identifier Mismatch     :",defects.dtim?"Yes":"No");
        printf("%-30s %-12s\n", "  Backward Defect Indication          :",defects.dbdi?"Yes":"No");
        printf("%-30s %-12s\n", "  Open Connection Indication          :",defects.doci?"Yes":"No");
        printf("%-30s %-12s\n", "  Locked defect                       :",defects.dlck?"Yes":"No");
        printf("%-30s %-12s\n", "  Payload Mismatch                    :",defects.dplm?"Yes":"No");
        printf("%-30s %-12s\n", "  Alarm Indication Signal             :",defects.dais?"Yes":"No");
    }
}

static void cli_cmd_otn_otu_acctti(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_otu_acc_tti_t acc_tti;
    u16 i;


    memset(&acc_tti, 0, sizeof(acc_tti));
    
    if(req->set){
    } else {
    /* get */
        printf("OTN OTU Accepted TTI\n");
    }
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (req->set) {
            /* Set not applicable */
        } else {
            CLI_TST_RC(vtss_otn_otu_acc_tti_get(req->api_inst, port_no, &acc_tti));
            printf("Port Inconsistent\n");
            printf("---- ------------\n");
            printf("%-4u %-12s\n", port, cli_bool_disp(acc_tti.inconsistent));
            printf("Accepted TTI\n");
            printf("------------\n");
            for(i = 0; i < 64; i++) {
                printf("%c",acc_tti.tti[i]);
            }
            printf("\n");
        }
    }
}

static void cli_cmd_otn_otu_cons_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_otu_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port beibiae bdi ais bdi_los bdi_lof bdi_lom bdi_ais bdi_tim ais_lom ais_tim");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            cons_act.beibiae =  req->beibiae;
            cons_act.bdi     =  req->bdi;
            cons_act.ais     =  req->ais;
            cons_act.bdi_los =  req->bdi_los;
            cons_act.bdi_lof =  req->bdi_lof;
            cons_act.bdi_lom =  req->bdi_lom;
            cons_act.bdi_ais =  req->bdi_ais;
            cons_act.bdi_tim =  req->bdi_tim;
            cons_act.ais_lom =  req->ais_lom;
            cons_act.ais_tim =  req->ais_tim;

            if (vtss_otn_otu_consequent_actions_set(req->api_inst, port_no, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_otu_consequent_actions_get(req->api_inst, port_no, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u %-7s %-3s %-3s %-7s %-7s %-7s %-7s %-7s %-7s %-7s\n", port,
                   cli_bool_disp(cons_act.beibiae),
                   cli_bool_disp(cons_act.bdi),
                   cli_bool_disp(cons_act.ais),
                   cli_bool_disp(cons_act.bdi_los),
                   cli_bool_disp(cons_act.bdi_lof),
                   cli_bool_disp(cons_act.bdi_lom),
                   cli_bool_disp(cons_act.bdi_ais),
                   cli_bool_disp(cons_act.bdi_tim),
                   cli_bool_disp(cons_act.ais_lom),
                   cli_bool_disp(cons_act.ais_tim));
        }
    }
}

static void cli_cmd_otn_otu_cons_iae_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_otu_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port  iae");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_otu_consequent_actions_get(req->api_inst, port_no, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_act.iae =  req->iae;
            if (vtss_otn_otu_consequent_actions_set(req->api_inst, port_no, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_otu_consequent_actions_get(req->api_inst, port_no, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_otu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-3s\n", port, cli_bool_disp(cons_act.iae));
        }
    }
}

static void cli_cmd_otn_otu_perform(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_otu_perf_t perf;

    memset(&perf, 0, sizeof(perf));
    printf("OTN OTU performance:\n");
    cli_table_header("Port  Near End Errored Block Count  Far End Errored Block Count");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_otn_otu_perf_get(req->api_inst,
                                     port_no,
                                     &perf)) {
            printf("%-4u  %-28u  %-27u\n", port, perf.pnebc, perf.pfebc);
        }
    }
}

static void cli_cmd_otn_odu_acctti(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_acc_tti_t odu_acc_tti;
    u16 i;

    memset(&odu_acc_tti, 0, sizeof(odu_acc_tti));
    
    printf("OTN ODU Accepted TTI\n");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_otn_odu_acc_tti_get(req->api_inst, port_no, &odu_acc_tti));
        printf("Port Inconsistent\n");
        printf("---- ------------\n");
        printf("%-4u %-12s\n", port, cli_bool_disp(odu_acc_tti.inconsistent));
        printf("Accepted TTI\n");
        printf("------------\n");
        for(i = 0; i < 64; i++) {
            printf("%c",odu_acc_tti.tti[i]);
        }
        printf("\n");
    }
}

static void cli_cmd_otn_odu_ais(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_ais_t ais;

    memset(&ais, 0, sizeof(vtss_otn_odu_ais_t));
    if (!req->set) {
        cli_table_header("Port  si_ais  so_ais");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            ais.si_enable =  req->si_ais;
            ais.so_enable =  req->so_ais;
            if (vtss_otn_odu_ais_set(req->api_inst, port_no, &ais) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_ais_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odu_ais_get(req->api_inst, port_no, &ais) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_ais_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-6s  %-6s\n", port, cli_bool_disp(ais.si_enable), cli_bool_disp(ais.so_enable));
        }
    }
}

static void cli_cmd_otn_odu_mode(cli_req_t *req)
{
    vtss_port_no_t         port_no, port;
    vtss_otn_odu_mode_t    mode;

    printf("OTN ODU mode Configuration:\n");
    cli_table_header("Port  Mode type");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&mode, 0, sizeof(mode));
        CLI_TST_RC(vtss_otn_odu_mode_get(req->api_inst, port_no, &mode));
        if (req->set) {
            mode.mode = req->odu_mode;
            CLI_TST_RC(vtss_otn_odu_mode_set(req->api_inst, port_no, &mode));
        }
        switch (mode.mode) {
            case VTSS_OTN_ODU_MODE_TERMINATION: printf("%-4u  %-11s\n", port, "Termination"); break;
            case VTSS_OTN_ODU_MODE_MONITOR:     printf("%-4u  %-11s\n", port, "Monitor"); break;
            default:                            printf("%-4u  %-11s\n", port, "Unknown"); break;
        }
    }
}

static void cli_cmd_otn_odu_cons_bei_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_cons_act_t cons_odu_act;

    memset(&cons_odu_act, 0, sizeof(cons_odu_act));
    if (!req->set) {
        cli_table_header("Port  bei");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_odu_act.bei =  req->bei;
            if (vtss_otn_odu_consequent_actions_set(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-3s\n", port, cli_bool_disp(cons_odu_act.bei));
        }
    }
}

static void cli_cmd_otn_odu_cons_bdi_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_cons_act_t cons_odu_act;

    memset(&cons_odu_act, 0, sizeof(cons_odu_act));
    if (!req->set) {
        cli_table_header("Port bdi bdilos bdilof bdilom bdiotuais bdiotutim bdiais bdioci bdilck bditim");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_odu_act.bdi         =  req->odu_bdi;
            cons_odu_act.bdi_los     =  req->odu_bdi_los;
            cons_odu_act.bdi_lof     =  req->odu_bdi_lof;
            cons_odu_act.bdi_lom     =  req->odu_bdi_lom;
            cons_odu_act.bdi_otu_ais =  req->odu_bdi_otu_ais;
            cons_odu_act.bdi_otu_tim =  req->odu_bdi_otu_tim;
            cons_odu_act.bdi_ais     =  req->odu_bdi_ais;
            cons_odu_act.bdi_oci     =  req->odu_bdi_oci;
            cons_odu_act.bdi_lck     =  req->odu_bdi_lck;
            cons_odu_act.bdi_tim     =  req->odu_bdi_tim;

            if (vtss_otn_odu_consequent_actions_set(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u %-3s %-6s %-6s %-6s %-9s %-9s %-6s %-6s %-6s %-6s\n", port,
                   cli_bool_disp(cons_odu_act.bdi),
                   cli_bool_disp(cons_odu_act.bdi_los),
                   cli_bool_disp(cons_odu_act.bdi_lof),
                   cli_bool_disp(cons_odu_act.bdi_lom),
                   cli_bool_disp(cons_odu_act.bdi_otu_ais),
                   cli_bool_disp(cons_odu_act.bdi_otu_tim),
                   cli_bool_disp(cons_odu_act.bdi_ais),
                   cli_bool_disp(cons_odu_act.bdi_oci),
                   cli_bool_disp(cons_odu_act.bdi_lck),
                   cli_bool_disp(cons_odu_act.bdi_tim));
        }
    }
}

static void cli_cmd_otn_odu_cons_ais_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_cons_act_t cons_odu_act;

    memset(&cons_odu_act, 0, sizeof(cons_odu_act));
    if (!req->set) {
        cli_table_header("Port  ais  ais_timocilck  ais_plm");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_odu_act.ais           =  req->odu_ais;
            cons_odu_act.ais_timocilck =  req->odu_ais_timocilck;
            cons_odu_act.ais_plm       =  req->odu_ais_plm;

            if (vtss_otn_odu_consequent_actions_set(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odu_consequent_actions_get(req->api_inst, port_no, &cons_odu_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-3s  %-13s  %-7s\n", port,
                   cli_bool_disp(cons_odu_act.ais),
                   cli_bool_disp(cons_odu_act.ais_timocilck),
                   cli_bool_disp(cons_odu_act.ais_plm));
        }
    }
}

static void cli_cmd_otn_odu_payload_type(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_pt_t odu_pl_type;

    memset(&odu_pl_type, 0, sizeof(odu_pl_type));
    if (!req->set) {
        cli_table_header("Port  plm  tx  pay_load");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            odu_pl_type.plm         =  req->odu_pl_plm;
            odu_pl_type.tx_enable   =  req->odu_pl_tx;
            odu_pl_type.pt          =  req->odu_pl_type;

            if (vtss_otn_odu_pt_set(req->api_inst, port_no, &odu_pl_type) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_pt_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odu_pt_get(req->api_inst, port_no, &odu_pl_type) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odu_pt_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-3s  %-2s  %u\n", port,
                   cli_bool_disp(odu_pl_type.plm),
                   cli_bool_disp(odu_pl_type.tx_enable),
                   odu_pl_type.pt);
        }
    }
}

static void cli_cmd_otn_odu_acc_pltype(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_acc_pt_t odu_acc_pt;

    memset(&odu_acc_pt, 0, sizeof(odu_acc_pt));
    cli_table_header("Port  inconsistent  pay_load");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;

        if (vtss_otn_odu_acc_pt_get(req->api_inst, port_no, &odu_acc_pt) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odu_pt_get() for port %u\n",port);
                continue;
        }
        printf("%-4u  %-12s  %u\n", port,
                   cli_bool_disp(odu_acc_pt.inconsistent), odu_acc_pt.pt);
    }
}

static void cli_cmd_otn_odu_aps(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odu_tx_aps_t   aps;

    printf("OTN ODU TX APS Configuration:\n");
    cli_table_header("Port  enabled  tx_aps");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&aps, 0, sizeof(aps));
        CLI_TST_RC(vtss_otn_odu_tx_aps_get(req->api_inst, port_no, &aps));
        if (req->set) {
            aps.enable = req->enable;
            memcpy(aps.value, req->odut_aps, sizeof(aps.value));;
            CLI_TST_RC(vtss_otn_odu_tx_aps_set(req->api_inst, port_no, &aps));
        }
        printf("%-4u  %-7s  %X %X %X %X\n", port, cli_bool_disp(aps.enable), aps.value[0], aps.value[1], aps.value[2], aps.value[3]);
    }
}

static void cli_cmd_otn_odu_acc_aps(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_acc_aps_t aps;

    memset(&aps, 0, sizeof(aps));
    
    printf("OTN ODU Accepted APS values\n");
    cli_table_header("Port  Inconsistent  acc_aps");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_otn_odu_acc_aps_get(req->api_inst, port_no, &aps));
        printf("%-4u  %-12s  %X %X %X %X\n", port, cli_bool_disp(aps.inconsistent), aps.value[0], aps.value[1], aps.value[2], aps.value[3]);
    }
}

static void cli_cmd_otn_odu_perform(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odu_perf_t odu_perf;

    memset(&odu_perf, 0, sizeof(odu_perf));
    printf("OTN ODU performance:\n");
    cli_table_header("Port  pnebc       pfebc       njus        pjus        prbs");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_otn_odu_perf_get(req->api_inst,
                                     port_no,
                                     &odu_perf)) {
            printf("%-4u  %-10u  %-10u  %-10u  %-10u  %-10u\n", 
                    port, odu_perf.pnebc, odu_perf.pfebc, odu_perf.njus, odu_perf.pjus, odu_perf.prbs);
        }
    }
}

static void cli_cmd_otn_odut_mode(cli_req_t *req)
{
    vtss_port_no_t         port_no, port;
    vtss_otn_odut_mode_t    mode;

    printf("OTN ODUT mode Configuration:\n");
    cli_table_header("Port  Level  Mode type");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&mode, 0, sizeof(mode));
        CLI_TST_RC(vtss_otn_odut_mode_get(req->api_inst, port_no, req->odut_level, &mode));
        if (req->set) {
            mode.mode = req->odut_mode;
            CLI_TST_RC(vtss_otn_odut_mode_set(req->api_inst, port_no, req->odut_level, &mode));
        }
        switch (mode.mode) {
            case VTSS_OTN_ODUT_MODE_TERMINATION: printf("%-4u  %-5u  %-11s\n", port, req->odut_level+1, "Termination"); break;
            case VTSS_OTN_ODUT_MODE_MONITOR:     printf("%-4u  %-5u  %-11s\n", port, req->odut_level+1, "Monitor"); break;
            default:                             printf("%-4u  %-5u  %-11s\n", port, req->odut_level+1, "Unknown"); break;
        }
    }
}

static void cli_cmd_otn_odut_tti(cli_req_t * req)
{
    vtss_otn_odut_tti_t tti;
    u16 first = 1;

    memset(&tti, 0, sizeof(vtss_otn_odut_tti_t));
    if(req->set) {
        if(req->enable) {
            if (req->otn_tti_set) {
                memcpy(&tti.tx_tti[0],req->otn_tti,64);
            }
            tti.tx_enable = req->enable;
            tti.tim_sapi = req->otn_sapi_tim;
            tti.tim_dapi = req->otn_dapi_tim;
            tti.tim_os = req->otn_os_tim;
            if(req->otn_extti_set) {
                memcpy(&tti.exp_tti[0],req->otn_extti,64);
            }
        }
    } else {
        printf("OTN ODUT Transmitted TTI\n");
    }

    if (req->set) {
        CLI_TST_RC(vtss_otn_odut_tti_set(req->api_inst, req->port, req->odut_level, &tti));
    } else {
        CLI_TST_RC(vtss_otn_odut_tti_get(req->api_inst, req->port, req->odut_level, &tti));
        if(first)
            cli_table_header("Port  Level  TTI_Enable/Disable");
        printf("%-4u  %-5u  %-18s\n", iport2uport(req->port), req->odut_level+1, cli_bool_disp(tti.tx_enable));
        if(first)
            cli_table_header("TXTTI");
        printf("%s\n",tti.tx_tti);
        if(first)
            cli_table_header("SAPI_TIM  DAPI_TIM  OS_TIM");
        printf("%-8s  %-8s  %-6s\n", cli_bool_disp(tti.tim_sapi), cli_bool_disp(tti.tim_dapi), cli_bool_disp(tti.tim_os));
        if(first)
            cli_table_header("EXPECTED TTI");
        printf("%s\n",tti.exp_tti);
        first = 0;
    }
}

static void cli_cmd_otn_odut_acctti(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_acc_tti_t acc_tti;
    u16 i;

    memset(&acc_tti, 0, sizeof(acc_tti));
    
    printf("OTN ODUT Accepted TTI\n");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_otn_odut_acc_tti_get(req->api_inst, port_no, req->odut_level, &acc_tti));
        printf("Port  Level  Inconsistent\n");
        printf("----  -----  ------------\n");
        printf("%-4u  %-5u  %-12s\n", port, req->odut_level+1, cli_bool_disp(acc_tti.inconsistent));
        printf("Accepted TTI\n");
        printf("------------\n");
        for(i = 0; i < 64; i++) {
            printf("%c",acc_tti.tti[i]);
        }
        printf("\n");
    }
}

static void cli_cmd_otn_odut_cons_bei_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port  Level  beibiae");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_act.beibiae =  req->beibiae;
            if (vtss_otn_odut_consequent_actions_set(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-5u  %-3s\n", port, req->odut_level+1, cli_bool_disp(cons_act.beibiae));
        }
    }
}

static void cli_cmd_otn_odut_cons_bdi_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port  Level  bdi  bdilos  bdilof  bdilom  bdiotuais  bdiotutim  bdiais  bdioci  bdilck  bdiltc  bditim");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_act.bdi         =  req->bdi;
            cons_act.bdi_los     =  req->odu_bdi_los;
            cons_act.bdi_lof     =  req->odu_bdi_lof;
            cons_act.bdi_lom     =  req->odu_bdi_lom;
            cons_act.bdi_otu_ais =  req->odu_bdi_otu_ais;
            cons_act.bdi_otu_tim =  req->odu_bdi_otu_tim;
            cons_act.bdi_ais     =  req->odu_bdi_ais;
            cons_act.bdi_oci     =  req->odu_bdi_oci;
            cons_act.bdi_lck     =  req->odu_bdi_lck;
            cons_act.bdi_ltc     =  req->odu_bdi_ltc;
            cons_act.bdi_tim     =  req->odu_bdi_tim;

            if (vtss_otn_odut_consequent_actions_set(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-5u  %-3s  %-6s  %-6s  %-6s  %-9s  %-9s  %-6s  %-6s  %-6s  %-6s  %-6s\n", port, req->odut_level+1,
                   cli_bool_disp(cons_act.bdi),
                   cli_bool_disp(cons_act.bdi_los),
                   cli_bool_disp(cons_act.bdi_lof),
                   cli_bool_disp(cons_act.bdi_lom),
                   cli_bool_disp(cons_act.bdi_otu_ais),
                   cli_bool_disp(cons_act.bdi_otu_tim),
                   cli_bool_disp(cons_act.bdi_ais),
                   cli_bool_disp(cons_act.bdi_oci),
                   cli_bool_disp(cons_act.bdi_lck),
                   cli_bool_disp(cons_act.bdi_ltc),
                   cli_bool_disp(cons_act.bdi_tim));
        }
    }
}

static void cli_cmd_otn_odut_cons_ais_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port  Level  ais  ais_timocilck");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_act.ais           =  req->ais;
            cons_act.ais_timocilck =  req->odu_ais_timocilck;

            if (vtss_otn_odut_consequent_actions_set(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-5u  %-3s  %-13s\n", port, req->odut_level+1, cli_bool_disp(cons_act.ais), cli_bool_disp(cons_act.ais_timocilck));
        }
    }
}

static void cli_cmd_otn_odut_cons_iae_actions(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_cons_act_t cons_act;

    memset(&cons_act, 0, sizeof(cons_act));
    if (!req->set) {
        cli_table_header("Port  Level  iae");
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) )
            continue;
        if (req->set) {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            cons_act.iae =  req->iae;
            if (vtss_otn_odut_consequent_actions_set(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_otn_odut_consequent_actions_get(req->api_inst, port_no, req->odut_level, &cons_act) != VTSS_RC_OK) {
                printf("Could not perform vtss_otn_odut_consequent_actions_get() for port %u\n",port);
                continue;
            }
            printf("%-4u  %-5u  %-3s\n", port, req->odut_level+1, cli_bool_disp(cons_act.iae));
        }
    }
}

static void cli_cmd_otn_odut_bdi(cli_req_t *req)
{
    vtss_port_no_t        port_no, port;
    vtss_otn_odut_bdi_t   bdi;

    printf("OTN ODUT BDI Configuration:\n");
    cli_table_header("Port  Level  Mode");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&bdi, 0, sizeof(bdi));
        CLI_TST_RC(vtss_otn_odut_bdi_get(req->api_inst, port_no, req->odut_level, &bdi));
        if (req->set) {
            bdi.mode = req->odut_bdi;
            CLI_TST_RC(vtss_otn_odut_bdi_set(req->api_inst, port_no, req->odut_level, &bdi));
        }
        switch (bdi.mode) {
            case VTSS_OTN_ODUT_BDI_UNFORCED:      printf("%-4u  %-5u  %-8s\n", port, req->odut_level+1, "Unforced"); break;
            case VTSS_OTN_ODUT_BDI_FORCED_HIGH:   printf("%-4u  %-5u  %-8s\n", port, req->odut_level+1, "High BDI"); break;
            case VTSS_OTN_ODUT_BDI_FORCED_LOW:    printf("%-4u  %-5u  %-8s\n", port, req->odut_level+1, "Low BDI"); break;
            default:                              printf("%-4u  %-5u  %-8s\n", port, req->odut_level+1, "Unknown"); break;
        }
    }
}

static void cli_cmd_otn_odut_stat(cli_req_t *req)
{
    vtss_port_no_t            port_no, port;
    vtss_otn_odut_tx_stat_t   stat;

    printf("OTN ODUT TX STAT Configuration:\n");
    cli_table_header("Port  Level  enabled  tx_stat");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&stat, 0, sizeof(stat));
        CLI_TST_RC(vtss_otn_odut_tx_stat_get(req->api_inst, port_no, req->odut_level, &stat));
        if (req->set) {
            stat.enable = req->enable;
            stat.value = req->odut_stat;
            CLI_TST_RC(vtss_otn_odut_tx_stat_set(req->api_inst, port_no, req->odut_level, &stat));
        }
        printf("%-4u  %-5u  %-7s  %-7u\n", port, req->odut_level + 1, cli_bool_disp(stat.enable), stat.value);
    }
}

static void cli_cmd_otn_odut_acc_stat(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_acc_stat_t stat;

    memset(&stat, 0, sizeof(stat));
    
    printf("OTN ODUT Accepted STAT value\n");
    cli_table_header("Port  Level  Inconsistent  acc_stat");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_otn_odut_acc_stat_get(req->api_inst, port_no, req->odut_level, &stat));
        printf("%-4u  %-5u  %-12s  %-8u\n", port, req->odut_level + 1, cli_bool_disp(stat.inconsistent), stat.value);
    }
}

static void cli_cmd_otn_odut_aps(cli_req_t *req)
{
    vtss_port_no_t            port_no, port;
    vtss_otn_odut_tx_aps_t   aps;

    printf("OTN ODUT TX APS Configuration:\n");
    cli_table_header("Port  Level  enabled  tx_aps");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&aps, 0, sizeof(aps));
        CLI_TST_RC(vtss_otn_odut_tx_aps_get(req->api_inst, port_no, req->odut_level, &aps));
        if (req->set) {
            aps.enable = req->enable;
            memcpy(aps.value, req->odut_aps, sizeof(aps.value));;
            CLI_TST_RC(vtss_otn_odut_tx_aps_set(req->api_inst, port_no, req->odut_level, &aps));
        }
        printf("%-4u  %-5u  %-7s  %X %X %X %X\n", port, req->odut_level + 1, cli_bool_disp(aps.enable), aps.value[0], aps.value[1], aps.value[2], aps.value[3]);
    }
}

static void cli_cmd_otn_odut_acc_aps(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_acc_aps_t aps;

    memset(&aps, 0, sizeof(aps));
    
    printf("OTN ODUT Accepted APS values\n");
    cli_table_header("Port  Level  Inconsistent  acc_aps");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_otn_odut_acc_aps_get(req->api_inst, port_no, req->odut_level, &aps));
        printf("%-4u  %-5u  %-12s  %X %X %X %X\n", port, req->odut_level + 1, cli_bool_disp(aps.inconsistent), aps.value[0], aps.value[1], aps.value[2], aps.value[3]);
    }
}

static void cli_cmd_otn_odut_defects(cli_req_t *req)
{
    vtss_port_no_t          port_no, port;
    vtss_otn_odut_defects_t  defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_otn_odut_defects_get(req->api_inst, port_no, req->odut_level, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_otn_odut_defects_get(): %u\n",port);
            continue;
        }

        printf("OTN ODUT Defects:\n\n");
        printf("Port: %u  Level: %u\n", port, req->odut_level + 1);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Trail Trace Identifier Mismatch     :",defects.dtim?"Yes":"No");
        printf("%-30s %-12s\n", "  Backward Defect Indication          :",defects.dbdi?"Yes":"No");
        printf("%-30s %-12s\n", "  Open Connection Indication          :",defects.doci?"Yes":"No");
        printf("%-30s %-12s\n", "  Locked defect                       :",defects.dlck?"Yes":"No");
        printf("%-30s %-12s\n", "  Loss of Tandem Connection           :",defects.dltc?"Yes":"No");
        printf("%-30s %-12s\n", "  Incoming Alignment Error            :",defects.diae?"Yes":"No");
        printf("%-30s %-12s\n", "  Backward Incoming Alignment Error   :",defects.dbiae?"Yes":"No");
        printf("%-30s %-12s\n", "  Alarm Indication Signal             :",defects.dais?"Yes":"No");
    }
}

static void cli_cmd_otn_odut_perform(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_otn_odut_perf_t perf;

    memset(&perf, 0, sizeof(perf));
    printf("OTN ODUT performance:\n");
    cli_table_header("Port  Level  pnebc count  pfebc count");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_otn_odut_perf_get(req->api_inst, port_no, req->odut_level, &perf)) {
            printf("%-4u  %-5u  %-11u  %-11u\n", port, req->odut_level+1, perf.pnebc, perf.pfebc);
        }
    }
}
#endif /* VTSS_FEATURE_OTN */

#if defined(VTSS_FEATURE_GFP)
static void cli_cmd_gfp_single_err_corr(cli_req_t *req)
{
    vtss_port_no_t         port_no, port;
    vtss_gfp_err_corr_t    conf;

    printf("GFP Single Error Correction Configuration:\n");
    cli_table_header("Port  core  payload  trans");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_single_err_corr_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.core_enable = req->core_enable;
            conf.pay_enable = req->pay_enable;
            conf.trans_enable = req->trans_enable;
            CLI_TST_RC(vtss_gfp_single_err_corr_set(req->api_inst, port_no, &conf));
        }
        printf("%-4u  %-4s  %-7s  %-5s\n", port,
               cli_bool_disp(conf.core_enable),
               cli_bool_disp(conf.pay_enable),
               cli_bool_disp(conf.trans_enable));
    }
}

static void cli_cmd_gfp_cmf_auto_sf_gen(cli_req_t *req)
{
    vtss_port_no_t            port_no, port;
    vtss_gfp_cmf_auto_sf_t    conf;

    printf("GFP Automatic CMF SF generation Configuration:\n");
    cli_table_header("Port  los_sf         lsync_sf     ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_cmf_auto_sf_gen_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.los_sf = req->los_sf;
            conf.lsync_sf = req->lsync_sf;
            CLI_TST_RC(vtss_gfp_cmf_auto_sf_gen_set(req->api_inst, port_no, &conf));
        }
        printf("%-4u", port);
        switch (conf.los_sf) {
            case VTSS_GFP_CMF_SF_NONE:          printf("  %-13s", "SF none"); break;
            case VTSS_GFP_CMF_SF_HW:            printf("  %-13s", "SF hw based"); break;
            case VTSS_GFP_CMF_SF_FORCED_ON:     printf("  %-13s", "SF forced on"); break;
            case VTSS_GFP_CMF_SF_FORCED_OFF:    printf("  %-13s", "SF forced off"); break;
            default:                            printf("  %-13s", "Unknown"); break;
        }
        switch (conf.lsync_sf) {
            case VTSS_GFP_CMF_SF_NONE:          printf("  %-13s\n", "SF none"); break;
            case VTSS_GFP_CMF_SF_HW:            printf("  %-13s\n", "SF hw based"); break;
            case VTSS_GFP_CMF_SF_FORCED_ON:     printf("  %-13s\n", "SF forced on"); break;
            case VTSS_GFP_CMF_SF_FORCED_OFF:    printf("  %-13s\n", "SF forced off"); break;
            default:                            printf("  %-13s\n", "Unknown"); break;
        }
    }
}

static void cli_cmd_gfp_cmf_forced_gen(cli_req_t *req)
{
    vtss_port_no_t           port_no, port;
    vtss_gfp_cmf_forced_t    conf;
    vtss_rc                  rc = VTSS_RC_OK;
    
    printf("GFP Forced CMF generation Configuration:\n");
    cli_table_header("Port  enabled  interval  upi  pti  frame_len");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_cmf_forced_gen_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.enable = req->enable;
            conf.interval = req->forced_interval;
            conf.frame_len = req->cmf_fr_len;
            conf.upi = req->upi;
            conf.pti = req->pti;
            rc = vtss_gfp_cmf_forced_gen_set(req->api_inst, port_no, &conf);
        }
        if (rc != VTSS_RC_OK)
            continue;
        printf("%-4u  %-7s  %-8u  %-3X  %-3X %-4u\n", port, cli_bool_disp(conf.enable), conf.interval, conf.upi, conf.pti, conf.frame_len);
    }
}

static void cli_cmd_gfp_cmf_acc_upi(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    u32 upi, pti;

    upi = 0;
    
    printf("GFP CMF Accepted UPI values\n");
    cli_table_header("Port  acc_upi  acc_pti");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_gfp_cmf_acc_upi_pti_get(req->api_inst, port_no, &upi, &pti));
        printf("%-4u  %-7X  %7X\n", port, upi, pti);
    }
}

static void cli_cmd_gfp_cdf_fcs_insert(cli_req_t *req)
{
    vtss_port_no_t              port_no, port;
    vtss_gfp_cdf_fcs_insert_t   conf;

    printf("GFP CDF FCS insert Configuration:\n");
    cli_table_header("Port  Enabled");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_cdf_fcs_insert_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.enable = req->enable;
            CLI_TST_RC(vtss_gfp_cdf_fcs_insert_set(req->api_inst, port_no, &conf));
        }
        printf("%-4u  %-7s\n", port, cli_bool_disp(conf.enable));
    }
}

static void cli_cmd_gfp_cdf_upi(cli_req_t *req)
{
    vtss_port_no_t       port_no, port;
    vtss_gfp_cdf_upi_t   conf;

    printf("GFP CDF transmitted/expected UPI Configuration:\n");
    cli_table_header("Port  upi  ordered_upi");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_cdf_upi_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.upi = req->upi;
            conf.ordered_upi = req->ordered_upi;
            CLI_TST_RC(vtss_gfp_cdf_upi_set(req->api_inst, port_no, &conf));
        }
        printf("%-4u  %-3X  %-11X\n", port, conf.upi, conf.ordered_upi);
    }
}

static void cli_cmd_gfp_cdf_acc_upi(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    u32 upi;

    upi = 0;
    
    printf("GFP CDF Accepted but not matching UPI values\n");
    cli_table_header("Port  acc_upi");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        CLI_TST_RC(vtss_gfp_cdf_acc_upi_get(req->api_inst, port_no, &upi));
        printf("%-4u  %-7X\n", port, upi);
    }
}

static void cli_cmd_gfp_8b10b_control_code(cli_req_t *req)
{
    vtss_port_no_t                  port_no, port;
    vtss_gfp_8b10b_control_code_t   conf;

    printf("GFP CDF UPI Configuration:\n");
    cli_table_header("Port  force_los  force_lsync  err_code");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        memset(&conf, 0, sizeof(conf));
        CLI_TST_RC(vtss_gfp_8b10b_control_code_get(req->api_inst, port_no, &conf));
        if (req->set) {
            conf.force_los = req->force_los;
            conf.force_lsync = req->force_lsync;
            conf.err_code = req->err_code;
            CLI_TST_RC(vtss_gfp_8b10b_control_code_set(req->api_inst, port_no, &conf));
        }
        printf("%-4u  %-9s  %-11s", port, cli_bool_disp(conf.force_los), cli_bool_disp(conf.force_lsync));
        switch (conf.err_code) {
            case VTSS_GFP_10B_ERR_ERROR_CODE:   printf("  %-8s\n", "10B_ERR"); break;
            case VTSS_GFP_K30_7_ERROR_CODE:     printf("  %-8s\n", "K30.7"); break;
            default:                            printf("  %-8s\n", "Unknown"); break;
        }
    }
}

static void cli_cmd_gfp_defects(cli_req_t *req)
{
    vtss_port_no_t      port_no, port;
    vtss_gfp_defects_t  defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0) ) 
            continue;
        memset(&defects, 0, sizeof(defects)); 
        if (vtss_gfp_defects_get(req->api_inst, port_no, &defects) != VTSS_RC_OK) {
            printf("Could not perform vtss_gfp_defects_get(): %u\n",port);
            continue;
        }

        printf("GFP Defects:\n\n");
        printf("Port: %u\n", port);
        printf("--------\n");
        printf("%-30s %-12s\n", "  Server Signal Fail             :",defects.dssf?"Yes":"No");
        printf("%-30s %-12s\n", "  CMF Inconsistent               :",defects.dcmf_inconsistent?"Yes":"No");
        printf("%-30s %-12s\n", "  8B10B Client Loss Of Signal    :",defects.dlos?"Yes":"No");
        printf("%-30s %-12s\n", "  8B10B Client Loss of sync      :",defects.dlsync?"Yes":"No");
    }
}

static void cli_cmd_gfp_perf(cli_req_t *req)
{
    vtss_port_no_t  port_no, port;
    vtss_gfp_perf_t perf;

    memset(&perf, 0, sizeof(perf));
    printf("GFP performance:\n");
    cli_table_header("Port  super_corr  super_uncorr  core_corr  pyld_corr  pyld_uncorr  tx_ctrl   tx_cmf    tx_cdf    rx_ctrl   rx_cmf    rx_cdf    rx_invalid");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_gfp_perf_get(req->api_inst, port_no, &perf)) {
            printf("%-4u  %-10u  %-12u  %-9u  %-9u  %-11u  %-8u  %-8u  %-8u  %-8u  %-8u  %-8u  %-10u\n",
                   port, perf.psuper_block_corrected, perf.psuper_block_uncorrected, perf.pcore_header_corrected, perf.ppyld_header_corrected, perf.ppyld_header_uncorrected, perf.ptx_control_frames,
                   perf.ptx_client_management_frames, perf.ptx_client_data_frames, perf.prx_control_frames, perf.prx_client_management_frames, perf.prx_client_data_frames, perf.prx_client_invalid_frames);
        }
    }
}
#endif /* VTSS_FEATURE_GFP */

#if defined(VTSS_FEATURE_UPI)
static void cli_cmd_upi_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_upi_cfg_t cfg;
    vtss_rc rc;
    printf("UPI configuration:\n");
    cli_table_header("Port  equip_loopback  line_loopback ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        rc = vtss_upi_conf_get(req->api_inst,
                                       port_no,
                                       &cfg);
        if (req->set && rc == VTSS_RC_OK) {
            if (req->xxx_set_lb) {
                cfg.tx_to_rx_loopback = req->xxx_equip_lb;
                cfg.rx_to_tx_loopback = req->xxx_line_lb;
            }
            rc = vtss_upi_conf_set(req->api_inst,
                                                  port_no,
                                                  &cfg);
        }
        if (rc == VTSS_RC_OK) {
            printf("%-4u  %-14s  %-13s\n", port,
                   cli_bool_disp(cfg.tx_to_rx_loopback), 
                   cli_bool_disp(cfg.rx_to_tx_loopback) );
        }
    }
}

static void cli_cmd_upi_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_upi_status_t status;
    printf("UPI status:\n");
    cli_table_header("Port  mode   proto  ssf    los    oflow  uflow  plock  rst_o  eyequ  rx_sigdet[0..3]");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_upi_status_get(req->api_inst,
                                     port_no,
                                     &status)) {
            
            printf("%-4u  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-3s %-3s %-3s %-3s\n", port,
                   status.upi_2lane ? "2lane" : "4lane",
                   cli_upi_proto_disp(status.proto),
                   cli_bool_disp(status.ssf),
                   cli_bool_disp(status.rx_los), 
                   cli_bool_disp(status.tx_fifo_oflow), 
                   cli_bool_disp(status.tx_fifo_uflow), 
                   cli_bool_disp(status.hss_status.pll_lock), 
                   cli_bool_disp(status.hss_status.hss_rst_out), 
                   cli_bool_disp(status.hss_status.hss_eyequality), 
                   cli_bool_disp(status.hss_status.rx_sigdet[0]), 
                   cli_bool_disp(status.hss_status.rx_sigdet[1]), 
                   cli_bool_disp(status.hss_status.rx_sigdet[2]), 
                   cli_bool_disp(status.hss_status.rx_sigdet[3]) );
        }
    }
}

static void cli_cmd_upi_test_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_upi_test_cfg_t cfg;
    vtss_rc rc;
    printf("UPI test configuration:\n");
    cli_table_header("Port  prbs_gen  prbs_chk ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        rc = vtss_upi_test_conf_get(req->api_inst,
                               port_no,
                               &cfg);
        if (req->set && rc == VTSS_RC_OK) {
            cfg.test_cfg_gen = req->upi_gen_mode;
            cfg.test_cfg_chk = req->upi_chk_mode;
            cfg.wrap_mode = req->upi_wrp_mode;
            rc = vtss_upi_test_conf_set(req->api_inst,
                                   port_no,
                                   &cfg);
        }
        if (rc == VTSS_RC_OK) {
            printf("%-4u  %-8s  %-8s  %-7s\n", port, cli_upi_test_mode_disp(cfg.test_cfg_gen),
                   cli_upi_test_mode_disp(cfg.test_cfg_chk),
                   cli_upi_wrap_mode_disp(cfg.wrap_mode));
        }
    }
}

static void cli_cmd_upi_test_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_upi_test_status_t status[4];
    u32 lane;
    printf("UPI test status:\n");
    cli_table_header("Port  lane  lbsync  lberror  tx_prbssync  tx_prbs_err  rx_prbs_err_count  tx_prbs_err_count");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_upi_test_status_get(req->api_inst,
                                              port_no, status)) {
            for (lane = 0; lane < 4; lane++) {
            printf("%-4u  %-4u  %-6s  %-7s  %-11s  %-11s  %-17u  %-17u\n", port,lane,
                   cli_bool_disp(status[lane].lbsync),
                   cli_bool_disp(status[lane].lberror), 
                   cli_bool_disp(status[lane].tx_prbssync), 
                   cli_bool_disp(status[lane].tx_prbs_err), 
                   status[lane].rx_prbs_err_count,
                   status[lane].tx_prbs_err_count);
            }
        }
    }
}

static void cli_cmd_upi_txeq_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_upi_txeq_cfg_t cfg;
    vtss_init_conf_t init_conf;    
    vtss_rc rc = VTSS_RC_OK;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->set) {
            cfg.mode = req->upi_txeq_mode;
            cfg.channel_loss = req->upi_txeq_chnloss;
            rc = vtss_upi_txeq_mode_set(req->api_inst, port_no, &cfg);
            if (rc != VTSS_RC_OK) {
                printf("Equalizer mode Set failed\n");
                return;
            }
        }

        memset(&cfg, 0, sizeof(cfg));
        rc = vtss_upi_txeq_mode_get(req->api_inst, port_no, &cfg);
        if (rc != VTSS_RC_OK) {
            printf("Equalizer mode Get failed for port %d\n", port);
            continue;
        }

        CLI_TST_RC(vtss_init_conf_get(req->api_inst, &init_conf));

        printf("UPI Transmit Equalization configuration for port %d\n", port);
        printf("Mode             :");

        if (init_conf.two_lane_upi == 0)
            printf("Xaui ");
        else
            printf("Rxaui ");

        if (cfg.mode == UPI_TXEQ_OVERRIDE) 
            printf("Override ");
        else
            printf("Normal ");

        printf("\n");

        printf("Channel Loss     :");
        if (cfg.channel_loss == upi_chloss_lt10inch) {
            printf("Less than 10 inch");
        } else if (cfg.channel_loss == upi_chloss_gt10inch) {
            printf("Greater than 10 inch");
        } else if (cfg.channel_loss == upi_chloss_backplane) {
            printf("Backplane");
        }
        printf("\n\n");

    }
}

static void cli_cmd_upi_txeq_coef_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    u32 coefs[upi_txeq_coef_max];
    BOOL polarity_arr[upi_txeq_coef_max];
    int i;
    vtss_rc rc = VTSS_RC_OK;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->set) {
            rc = vtss_upi_txeq_coef_adjust(req->api_inst,port_no,
                        req->upi_coef_id, req->upi_coef_step, req->upi_incr,
                        req->upi_polarity);
            if (rc != VTSS_RC_OK){
                printf("UPI Equalizer Coefficient Update failed\n");
                return;
            }
        }

        memset(coefs, 0, upi_txeq_coef_max);
        rc = vtss_upi_txeq_coef_get(req->api_inst, port_no, coefs, polarity_arr);

        printf("\n");
        if (rc == VTSS_RC_OK) {
            printf("Transmit UPI Equalization Coefficients Port No: %u \n", port);
            for(i = 0; i< upi_txeq_coef_max; i++) {
                printf("C%u = %c", i, !coefs[i] ? ' ' : (polarity_arr[i] ? '+' : '-'));
                printf("%u\n", coefs[i]);
            }
        }
        else
            printf("Equalizer Coefficient Get failed for Port No: %u\n", port);    
    }
}

static void cli_cmd_upi_txeq_power(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    u32 power = 0;
    vtss_rc rc = VTSS_RC_OK;
	
    printf("Port no    Equalizer amplitude\n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->set) {
            rc = vtss_upi_txamp_set(req->api_inst, port_no, req->upi_power);
            if (rc != VTSS_RC_OK) {
                printf("Equalizer Power Set failed\n");
                return;
            }
        }	

        rc = vtss_upi_txamp_get(req->api_inst, port_no, &power);
        if (rc != VTSS_RC_OK) {
            printf("Equalizer Power Get failed\n");
            return;
        }
        printf("Port %d      %d \n", port, power);
    }
}

static void cli_cmd_upi_txeq_slew(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    u32 slew = 0;
    vtss_rc rc;

    printf("Port no    Slew-rate\n");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->set) {
            rc = vtss_upi_txslew_set(req->api_inst, port_no, req->upi_slew);
            if (rc != VTSS_RC_OK) {
                printf("Equalizer Slew rate Set failed\n");
                return;
            }
        }

        slew = 0;
        rc = vtss_upi_txslew_get(req->api_inst, port_no, &slew);
        if (rc != VTSS_RC_OK) {
            printf("Equalizer Slew rate Get failed\n");
            return;
        }

        printf("Port %d     ", port);
        if (slew == 1)
            printf("60 ps\n");
        else
            printf("30 ps\n");
    }
}

static void cli_cmd_upi_rxeq_mode(cli_req_t *req)
{
    vtss_port_no_t      port_no,port;
    vtss_upi_rxeq_cfg_t cfg;
    const char          *str[3] = {"< 10 inch", "> 10 inch", "Backplane"};

    printf("UPI RX_EQ Mode Configuration:\n");
    cli_table_header("Port   Channel_Loss_Type");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        if (req->set) {
            cfg.channel_loss = req->upi_rxeq_mode;
            CLI_TST_RC(vtss_upi_rxeq_mode_set(NULL, port_no, &cfg));
        }
        CLI_TST_RC(vtss_upi_rxeq_mode_get(req->api_inst, port_no, &cfg));
        printf("%-4u   %s\n", port, str[cfg.channel_loss]);
    }
}

#endif /* VTSS_FEATURE_UPI */

#if defined(VTSS_FEATURE_XAUI)
static void cli_cmd_xaui_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xaui_cfg_t cfg;
    printf("XAUI configuration:\n");
    cli_table_header("Port  equip_loopback  line_loopback ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
    
        CLI_TST_RC(vtss_xaui_config_get(req->api_inst,
                                     port_no,
                                     &cfg));
        if (req->set) {
            if (req->xxx_set_lb) {
                cfg.equipment_loopback = req->xxx_equip_lb;
                cfg.line_loopback = req->xxx_line_lb;
            }
            CLI_TST_RC(vtss_xaui_config_set(req->api_inst,
                                         port_no,
                                         &cfg));
        }
        printf("%-4u  %-14s  %-13s\n", port,
               cli_bool_disp(cfg.equipment_loopback), 
               cli_bool_disp(cfg.line_loopback) );
    }
}

static void cli_cmd_xaui_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xaui_status_t status;
    printf("XAUI status:\n");
    cli_table_header("Port  sync_status  lanes_aligned  link_ok");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_xaui_status_get(req->api_inst,
                                              port_no,
        &status)) {
            
            printf("%-4u  %-2d%-2d%-2d%-5d  %-13s  %-7s\n", port, (status.sync_status>>3) & 1, 
                   (status.sync_status>>2) & 1, (status.sync_status>>1) & 1,status.sync_status & 1,
                   cli_bool_disp(status.lanes_aligned),
                   cli_bool_disp(status.link_ok));
            
        }
    }
}

static void cli_cmd_xaui_statistics(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xaui_pm_cnt_t stats;
    printf("XAUI statistics:\n");
    cli_table_header("Port  rx_align   xgmii_err  rx_fifo_o  rx_fifo_u  rx_dispar  c_gr_err ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_xaui_counters_get(req->api_inst,
                                               port_no, &stats)) {
            
            printf("%-4u  %-11u%-11u%-11u%-11u%-11u%-11u\n", port,
                   stats.rx_align_err_cnt,
                   stats.xgmii_err_cnt,
                   stats.rx_fifo_overflow_cnt,
                   stats.rx_fifo_underflow_cnt,
                   stats.rx_disparity_err_cnt,
                   stats.code_group_err_cnt);
        }
    }
    
    cli_table_header("Port  good_bytes_received   good_bytes_transmitted ");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
    
        if (VTSS_RC_OK == vtss_xaui_counters_get(req->api_inst,
        port_no, &stats)) {
            
            printf("%-4u  %-22llu%-22llu\n", port,
                   stats.good_bytes_rx,
                   stats.good_bytes_tx);
        }
        if (req->clear) {
            (void) vtss_xaui_counters_clear(req->api_inst, port_no);
        }
            
    }
}

#endif /* VTSS_FEATURE_XAUI */

#if defined(VTSS_FEATURE_XFI)
static void cli_cmd_xfi_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_cfg_t cfg;
    printf("XFI configuration:\n");
    cli_table_header("Port  xfi_lane  equip_loopback  line_loopback ");
    

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        CLI_TST_RC(vtss_xfi_conf_get(req->api_inst,
                                     port_no,
                                     &cfg));
        if (req->set) {
            if (req->upi_set_lane) {
                cfg.sel_lane_b = req->xfi_sel_lane_b;
            }
            if (req->xxx_set_lb) {
                cfg.equipment_loopback = req->xxx_equip_lb;
                cfg.line_loopback = req->xxx_line_lb;
            }
            CLI_TST_RC(vtss_xfi_conf_set(req->api_inst,
                                         port_no,
                                         &cfg));
        }
        printf("%-4u  %-8s  %-14s  %-13s\n", port,
               cfg.sel_lane_b ? "B" : "A",
               cli_bool_disp(cfg.equipment_loopback), 
               cli_bool_disp(cfg.line_loopback) );
    }
}

static void cli_cmd_xfi_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_status_t status;
    printf("XFI status:\n");
    cli_table_header("Port  ssf    plock  rst_o  eyequ  a_sig  a_los  b_sig  b_los  a_rx_intstatus  b_rx_intstatus");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        CLI_TST_RC(vtss_xfi_status_get(req->api_inst,
                                       port_no,
                                       &status));
        printf("%-4u  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-5s  %-14X  %-14X\n", port,
               cli_bool_disp(status.ssf),
               cli_bool_disp(status.hss_status.pll_lock), 
               cli_bool_disp(status.hss_status.hss_rst_out), 
               cli_bool_disp(status.hss_status.hss_eyequality), 
               cli_bool_disp(status.hss_status.rx_a_sigdet), 
               cli_bool_disp(status.hss_status.rx_a_los), 
               cli_bool_disp(status.hss_status.rx_b_sigdet), 
               cli_bool_disp(status.hss_status.rx_b_los),
               status.internal_status_a,
               status.internal_status_b);
    }
}

static void cli_cmd_xfi_test_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_test_cfg_t cfg;
    vtss_rc rc;
    printf("XFI test configuration:\n");
    cli_table_header("Port  prbs_gen  prbs_chk ");

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        rc = vtss_xfi_test_conf_get(req->api_inst,
                                    port_no,
                                    &cfg);
        if (req->set && rc == VTSS_RC_OK) {
            cfg.test_cfg_gen = req->upi_gen_mode;
            cfg.test_cfg_chk = req->upi_chk_mode;
            rc = vtss_xfi_test_conf_set(req->api_inst,
                                        port_no,
                                        &cfg);
        }
        if (rc == VTSS_RC_OK) {
            printf("%-4u  %-8s  %-8s\n", port, cli_xfi_test_mode_disp(cfg.test_cfg_gen),
                   cli_xfi_test_mode_disp(cfg.test_cfg_chk) );
        }
    }
}

static void cli_cmd_xfi_test_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_test_status_t status;
    printf("XFI test status:\n");
    printf("            |-- pr lane --|  |-------- selected lane ---------|\n");
    cli_table_header("Port  lane  lbsync  lberror  prbssync  prbs_err  prbs_err_count");
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (VTSS_RC_OK == vtss_xfi_test_status_get(req->api_inst, port_no, &status)) {
            printf("%-4u  %-4s  %-6s  %-7s  %-8s  %-8s  %-14u\n", port,"A",
                       cli_bool_disp(status.lbsync[0]),
                       cli_bool_disp(status.lberror[0]), 
                       cli_bool_disp(status.rx_prbssync), 
                       cli_bool_disp(status.rx_prbs_err), 
                       status.rx_prbs_err_count);
            printf("%-4u  %-4s  %-6s  %-7s\n", port,"B",
                   cli_bool_disp(status.lbsync[1]),
                   cli_bool_disp(status.lberror[1]));
        }
    }
}

static void cli_cmd_xfi_rec_clk_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_rec_clock_output_t rec_clock;

    printf("XFI Recovery clock output status:\n");
    cli_table_header("Port  Lane  Clock_Ouput_mode");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
       port = iport2uport(port_no);
       if (req->port_list[port] == 0)
           continue;
        memset(&rec_clock, 0, sizeof(vtss_xfi_rec_clock_output_t));
        CLI_TST_RC(vtss_xfi_rec_clock_output_get(req->api_inst, port_no, &rec_clock));
        if (req->set) {
            if (req->xfi_lane_A) {
                rec_clock.lane_a = req->enable;
            } else if (req->xfi_lane_B) {
                rec_clock.lane_b = req->enable;
            }
            CLI_TST_RC(vtss_xfi_rec_clock_output_set(req->api_inst, port_no, &rec_clock));
        }
        printf("%-4u  %-4s  %s\n", port, "A", (rec_clock.lane_a ? "Enabled" : "Disabled"));
        printf("%-4d  %-4s  %s\n", port, "B", (rec_clock.lane_b ? "Enabled" : "Disabled"));
    }
}

static void cli_cmd_xfi_txeq_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_txeq_cfg_t cfg;
   
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if(req->set) {
            cfg.mode = req->xfi_txeq_mode;
            cfg.channel_loss = req->xfi_txeq_chnloss;
            CLI_TST_RC(vtss_xfi_txeq_mode_set(req->api_inst, port_no, &cfg));
        }
     
        memset(&cfg, 0, sizeof(cfg));
        CLI_TST_RC(vtss_xfi_txeq_mode_get(req->api_inst, port_no, &cfg));

        printf("\n");
        printf("Transmit Equalization Configuration for Port No: %u \n", port);
        printf("Mode          :");
        if (cfg.mode == XFI_TXEQ_TRADITIONAL_OVERRIDE) {
            printf("Traditional Override");
        } else if (cfg.mode == XFI_TXEQ_TRADITIONAL) {
            printf("Traditional");
        } else if (cfg.mode == XFI_TXEQ_8023AP_OVERRIDE) {
            printf("802.3ap with Override");
        } else if (cfg.mode == XFI_TXEQ_8023AP) {
            printf("802.3ap ");
        }
        printf("\n");

        printf("Channel Loss  :");
        if (cfg.channel_loss == xfi_chloss_lt10inch) {
            printf("Less than 10 inch");
        } else if (cfg.channel_loss == xfi_chloss_gt10inch) {
            printf("Greater than 10 inch");
        } else if (cfg.channel_loss == xfi_chloss_sfp_plus) {
            printf("SFP Plus");
        } else if (cfg.channel_loss == xfi_chloss_copper_cable) {
            printf("Copper Cable");
        }
        printf("\n");

    }
}

static void cli_cmd_xfi_txmisc_parm_conf(cli_req_t *req)
{
    vtss_port_no_t port_no,port;
    u32            slew,power;

    printf("XFI Misc Parmaeters Configuration:\n");
    cli_table_header("Port    Slew      Power");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        if (req->set) {
            if (req->xfi_txeq_misc_parm == 1) {
                if ((req->xfi_misc_parm_value == 0) || (req->xfi_misc_parm_value == 3) || (req->xfi_misc_parm_value == 5) || (req->xfi_misc_parm_value == 7))
                CLI_TST_RC(vtss_xfi_txslew_set(NULL, port_no, req->xfi_misc_parm_value));
            } else if (req->xfi_txeq_misc_parm == 2) {
                if ((req->xfi_misc_parm_value > 32) && (req->xfi_misc_parm_value < 127))
                CLI_TST_RC(vtss_xfi_txamp_set(NULL, port_no, req->xfi_misc_parm_value));
            }
        }
        CLI_TST_RC(vtss_xfi_txslew_get(req->api_inst, port_no, &slew));
        CLI_TST_RC(vtss_xfi_txamp_get(req->api_inst, port_no, &power));
        printf("%-8u%-10u%u\n", port, slew, power);
    }
}

static void cli_cmd_xfi_rxeq_mode(cli_req_t *req)
{
    vtss_port_no_t      port_no,port;
    vtss_xfi_rxeq_cfg_t cfg;
    const char          *str[4] = {"< 10 inch", "> 10 inch", "sfp_plus", "copper_cable"};

    printf("XFI RX_EQ Mode Configuration:\n");
    cli_table_header("Port   Channel_Loss_Type");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;
        CLI_TST_RC(vtss_xfi_rxeq_mode_get(req->api_inst, port_no, &cfg));
        if (req->set) {
            cfg.channel_loss = req->xfi_rxeq_mode - 1;
            CLI_TST_RC(vtss_xfi_rxeq_mode_set(NULL, port_no, &cfg));
        }
        printf("%-4u  %s\n", port, str[cfg.channel_loss]);
    }
}

static void cli_cmd_xfi_txeq_coef_update(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_ffe_coef_t coef;
    vtss_xfi_ffe_coef_t coef_arr[txeq_ffe_max_coef];
    u32 step_size;
    BOOL incr;
    BOOL polarity;
    BOOL polarity_arr[txeq_ffe_max_coef];
    u32 i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if(req->set) {
            coef = req->xfi_txeq_coef;
            step_size = req->xfi_txeq_step;
            incr = (BOOL)req->xfi_txeq_incr;
            polarity = (BOOL)req->xfi_txeq_polarity;
            CLI_TST_RC(vtss_xfi_txeq_coef_adjust(req->api_inst, port_no, coef, step_size, incr, polarity));
        }

        memset(coef_arr, sizeof(coef_arr),0);
        memset(polarity_arr, sizeof(polarity_arr),0);
        CLI_TST_RC(vtss_xfi_txeq_coef_get(req->api_inst, port_no, coef_arr, polarity_arr));
        printf("\n");
        printf("Transmit Equalization Coefficients Port No: %u \n", port);
        for(i =0; i< txeq_ffe_max_coef; i++) {
            printf("C%u = %c", i, (!coef_arr[i] ? ' ':(polarity_arr[i] ? '+' : '-')));
            printf("%u\n", coef_arr[i]);
        }
    }
}

static void cli_cmd_xfi_802a_txeq_cof_upd(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_xfi_lane_t lane;
    vtss_xfi_ffe_coef_t coef;
    vtss_txeq_8023ap_updreq_t updreq;
    vtss_txeq_8023ap_coef_stat_t cof_802ap;
    vtss_txeq_8023ap_status_t status[txeq_ffe_max_coef];
    vtss_rc rc;
    int i;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        lane   = req->xfi_802a_txeq_lane;
        coef   = req->xfi_802a_txeq_coef;
        updreq = req->xfi_802a_txeq_upd;
        if(req->set) {
            rc = vtss_xfi_txeq_8023ap_coef_update(req->api_inst, port_no, lane, coef, updreq);
            if (rc != VTSS_RC_OK) {
                printf("802.3 ap co-efficient update failed\n");
                return;
            }
        }

        memset(&cof_802ap, 0, sizeof(vtss_txeq_8023ap_status_t));
        rc = vtss_xfi_txeq_8023ap_coef_stat_get(req->api_inst, port_no, lane, &cof_802ap);

        if (rc != VTSS_RC_OK) {
            printf("802.3 ap co-efficient get failed\n");
            return;
        }

        status[0] = cof_802ap.state_c0;
        status[1] = cof_802ap.state_c1;
        status[2] = cof_802ap.state_c2;

        printf("\n");
        printf("Transmit Equalization 802.3 ap Coefficients status for Port No: %u  lane: %d\n", port, lane);
        for(i = 0; i < txeq_ffe_max_coef; i++) {
            switch (status[i]) {
                case txeq_8023ap_stat_noupdt:
                printf("Co-efficient %d not updated\n", i);
                break;

                case txeq_8023ap_stat_updt:
                printf("Co-efficient %d updated\n", i);
                break;

                case txeq_8023ap_stat_min:
                printf("Co-efficient %d at minumum value\n", i);
                break;

                case txeq_8023ap_stat_max:
                printf("Co-efficient %d at maximum value\n", i);
                break;

                default:
                printf("Co-efficient status unknown 0x%x\n", status[i]);
            }
        }
    }
}        

static void cli_cmd_xfi_txeq_802a_fsm(cli_req_t *req)
{
    vtss_port_no_t      port_no,port;
    u32                 val;

    printf("XFI 802.3AP Eq FSM state for lane %u:\n", req->xfi_802a_txeq_lane);
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->port_list[port] == 0))
            continue;

        if (req->set) {
            CLI_TST_RC(vtss_xfi_txeq_8023ap_fsm_ctl_set(NULL, port_no, req->xfi_802a_txeq_lane, req->xfi_802a_txeq_fsm));
        }
        printf("Port no  FSM State\n");
        CLI_TST_RC(vtss_xfi_txeq_8023ap_fsm_stat_get(req->api_inst, port_no, req->xfi_802a_txeq_lane, &val));
        printf("%-4u  %4u\n", port, val);
    }
}
#endif /* VTSS_FEATURE_XFI */

#ifdef VTSS_FEATURE_MAC10G
static void cli_cmd_mac10g_config  (cli_req_t *req, BOOL lb, BOOL tx_flow_ctrl, BOOL rx_flow_ctrl_drop, BOOL rx_flow_ctrl_action, BOOL max_frame, BOOL status)
{
    vtss_port_no_t port_no, port;
    vtss_mac10g_cfg_t cfg;
    vtss_port_counters_t counters;
    i32 i;
    char buf[64];
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (req->clear) {
            vtss_mac10g_counters_clear(req->api_inst, port_no);
            continue;
        }

        memset(&cfg, 0, sizeof(cfg));
        CLI_TST_RC(vtss_mac10g_config_get(req->api_inst, port_no, &cfg));

        if (req->set) {
            if (lb) {
                cfg.rx_tx_loopback = req->mac10g_rx_tx_loopback;
                cfg.tx_rx_loopback = req->mac10g_tx_rx_loopback;
            } else if (tx_flow_ctrl) {
                cfg.flow_control.generate = req->enable;
                for (i = 0; i < 6; i++)
                     cfg.flow_control.smac.addr[i] = req->mac[i];
            } else if (rx_flow_ctrl_drop) {
                cfg.flow_control.rx_drop = req->enable;
            } else if (rx_flow_ctrl_action) {
                cfg.flow_control.obey = req->enable;
            } else if (max_frame) {
                cfg.max_frame_length = req->mac10g_max_frame_len;
                cfg.max_tags = req->mac10g_max_tags;
            } else {
                printf("Invalid Command \n\r");
            }
            CLI_TST_RC(vtss_mac10g_config_set(req->api_inst, port_no, &cfg));
        } else {
            if (lb) {
                if (first) {
                    cli_table_header("Port  Side    Loopback");
                }
                printf("%-4u %-6s    %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.tx_rx_loopback ? "Tx_Rx_Loopback" : (cfg.rx_tx_loopback ? "Rx_Tx_Loopback" : "No_Loopback"));
            } else if (tx_flow_ctrl) {
                if (first) {
                    cli_table_header("Port  Side    Tx_Flow_ctrl   Mac_Addr");
                }
                printf("%-4u %-6s    %-12s %s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.flow_control.generate ? "Enable" : "Disable",cli_mac_txt(cfg.flow_control.smac.addr, buf));
            } else if (rx_flow_ctrl_drop || rx_flow_ctrl_action) {
                if (first) {
                    cli_table_header("Port  Side    Rx_Flow_Ctrl_Drop Rx_Flow_Ctrl_Action");
                }
                printf("%-4u %-6s    %-17s %-17s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.flow_control.rx_drop ? "Enable" : "Disable",cfg.flow_control.obey ? "Enable" : "Disable");
            } else if (max_frame) {
                if (first) {
                    cli_table_header("Port  Side    Max_Frame_Len  Max_Tags");
                }
                printf("%-4u %-6s    %-13u %-8u\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.max_frame_length, cfg.max_tags);
            } else if (status) {
                vtss_mac10g_status_t status;

                if (first) {
                    cli_table_header("Port  Side    Link_Status    Pause_State     Local_Fault    Remote_Fault");
                }
                memset(&status, 0, sizeof(vtss_mac10g_status_t));
                vtss_mac10g_status_get(req->api_inst, port_no, &status);
                printf("%-4u %-6s   %-12s   %-12s    %-15s%-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line",
                                                         status.link_state ? "Up" : "Down",
                                                         status.pause ? "Paused" : "Not Paused",
                                                         status.local_fault ? "Detected" : "Not Detected",
                                                         status.remote_fault ? "Detected" : "Not Detected");
            } else {
                cli_table_header("Port  Side  ");
                printf("%-4u %-6s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line");
                memset(&counters,0,sizeof(counters));
                CLI_TST_RC(vtss_mac10g_counters_get(req->api_inst, port_no, &counters));
                printf("RMON :: \n");
                cli_table_header("Counter                         Value ");
                printf("rx_etherStatsDropEvents           :%-10llu \n",counters.rmon.rx_etherStatsDropEvents           );     
                printf("rx_etherStatsOctets               :%-10llu \n",counters.rmon.rx_etherStatsOctets               ); 
                printf("rx_etherStatsPkts                 :%-10llu \n",counters.rmon.rx_etherStatsPkts                 ); 
                printf("rx_etherStatsBroadcastPkts        :%-10llu \n",counters.rmon.rx_etherStatsBroadcastPkts        ); 
                printf("rx_etherStatsMulticastPkts        :%-10llu \n",counters.rmon.rx_etherStatsMulticastPkts        ); 
                printf("rx_etherStatsCRCAlignErrors       :%-10llu \n",counters.rmon.rx_etherStatsCRCAlignErrors       ); 
                printf("rx_etherStatsUndersizePkts        :%-10llu \n",counters.rmon.rx_etherStatsUndersizePkts        ); 
                printf("rx_etherStatsOversizePkts         :%-10llu \n",counters.rmon.rx_etherStatsOversizePkts         ); 
                printf("rx_etherStatsFragments            :%-10llu \n",counters.rmon.rx_etherStatsFragments            ); 
                printf("rx_etherStatsJabbers              :%-10llu \n",counters.rmon.rx_etherStatsJabbers              ); 
                printf("rx_etherStatsPkts64Octets         :%-10llu \n",counters.rmon.rx_etherStatsPkts64Octets         ); 
                printf("rx_etherStatsPkts65to127Octets    :%-10llu \n",counters.rmon.rx_etherStatsPkts65to127Octets    ); 
                printf("rx_etherStatsPkts128to255Octets   :%-10llu \n",counters.rmon.rx_etherStatsPkts128to255Octets   ); 
                printf("rx_etherStatsPkts256to511Octets   :%-10llu \n",counters.rmon.rx_etherStatsPkts256to511Octets   ); 
                printf("rx_etherStatsPkts512to1023Octets  :%-10llu \n",counters.rmon.rx_etherStatsPkts512to1023Octets  ); 
                printf("rx_etherStatsPkts1024to1518Octets :%-10llu \n",counters.rmon.rx_etherStatsPkts1024to1518Octets );
                printf("rx_etherStatsPkts1519toMaxOctets  :%-10llu \n",counters.rmon.rx_etherStatsPkts1519toMaxOctets  ); 
                printf("tx_etherStatsDropEvents           :%-10llu \n",counters.rmon.tx_etherStatsDropEvents           ); 
                printf("tx_etherStatsOctets               :%-10llu \n",counters.rmon.tx_etherStatsOctets               ); 
                printf("tx_etherStatsPkts                 :%-10llu \n",counters.rmon.tx_etherStatsPkts                 ); 
                printf("tx_etherStatsBroadcastPkts        :%-10llu \n",counters.rmon.tx_etherStatsBroadcastPkts        ); 
                printf("tx_etherStatsMulticastPkts        :%-10llu \n",counters.rmon.tx_etherStatsMulticastPkts        ); 
                printf("tx_etherStatsCollisions           :%-10llu \n",counters.rmon.tx_etherStatsCollisions           ); 
                printf("tx_etherStatsPkts64Octets         :%-10llu \n",counters.rmon.tx_etherStatsPkts64Octets         ); 
                printf("tx_etherStatsPkts65to127Octets    :%-10llu \n",counters.rmon.tx_etherStatsPkts65to127Octets    ); 
                printf("tx_etherStatsPkts128to255Octets   :%-10llu \n",counters.rmon.tx_etherStatsPkts128to255Octets   ); 
                printf("tx_etherStatsPkts256to511Octets   :%-10llu \n",counters.rmon.tx_etherStatsPkts256to511Octets   ); 
                printf("tx_etherStatsPkts512to1023Octets  :%-10llu \n",counters.rmon.tx_etherStatsPkts512to1023Octets  ); 
                printf("tx_etherStatsPkts1024to1518Octets :%-10llu \n",counters.rmon.tx_etherStatsPkts1024to1518Octets );
                printf("tx_etherStatsPkts1519toMaxOctets  :%-10llu \n",counters.rmon.tx_etherStatsPkts1519toMaxOctets  ); 
                printf("Ethernet Like :: \n");
                printf("dot3InPauseFrames  :%-10llu \n",counters.ethernet_like.dot3InPauseFrames);
                printf("dot3OutPauseFrames :%-10llu \n",counters.ethernet_like.dot3OutPauseFrames); 
            }
        }
        first = FALSE;
    }
}

static void cli_cmd_mac10g_lb_config (cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_mac10g_tx_flow_ctrl_config (cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_mac10g_rx_flow_ctrl_drop_config (cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_mac10g_rx_flow_ctrl_action_config (cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_mac10g_frame_len_config (cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 0, 0, 0, 1, 0);
}

#if defined VTSS_ARCH_DAYTONA
static void cli_cmd_mac10g_arch_specific_config (cli_req_t *req, BOOL preamble_check, BOOL local_fault, BOOL remote_fault)
{
    vtss_port_no_t port_no, port;
    vtss_mac10g_cfg_t cfg;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        memset(&cfg, 0, sizeof(cfg));
        CLI_TST_RC(vtss_mac10g_config_get(req->api_inst, port_no, &cfg));

        if (req->set) {
            if (preamble_check) {
                cfg.preamble_check = req->enable;
            } else if (local_fault) {
                cfg.local_fault_force = req->enable;
            } else if (remote_fault) {
                cfg.remote_fault_force = req->enable;
            }
            CLI_TST_RC(vtss_mac10g_config_set(req->api_inst, port_no, &cfg));
        } else {
            if (preamble_check) {
                if (first) {
                    cli_table_header("Port  Side    Preamble Check");
                }
                printf("%-4u %-6s    %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.preamble_check ? "Strict Check" : "Disable");
            } else if (local_fault) {
                if (first) {
                    cli_table_header("Port  Side    Local Fault Force");
                }
                printf("%-4u %-6s    %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.local_fault_force ? "Enable" : "Disable");
            } else if (remote_fault) {
                if (first) {
                    cli_table_header("Port  Side    Remote Fault Force");
                }
                printf("%-4u %-6s    %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.remote_fault_force ? "Enable" : "Disable");
            }
        }
        first = FALSE;
    }
}

static void cli_cmd_mac10g_preamble_check_config (cli_req_t *req)
{
    cli_cmd_mac10g_arch_specific_config(req, 1, 0, 0);
}

static void cli_cmd_mac10g_force_local_fault_config (cli_req_t *req)
{
    cli_cmd_mac10g_arch_specific_config(req, 0, 1, 0);
}

static void cli_cmd_mac10g_force_remote_fault_config (cli_req_t *req)
{
    cli_cmd_mac10g_arch_specific_config(req, 0, 0, 1);
}

#endif /* VTSS_ARCH_DAYTONA */

static void cli_cmd_mac_status(cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 0, 0, 0, 0, 1);
}
static void cli_cmd_mac_stats(cli_req_t *req)
{
    cli_cmd_mac10g_config(req, 0, 0, 0, 0, 0, 0);
}

#endif /* VTSS_FEATURE_MAC10G */

#ifdef VTSS_FEATURE_PCS_10GBASE_R
static void cli_cmd_pcs_10gbase_r_lb_config (cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_pcs_10gbase_r_cfg_t cfg;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        memset(&cfg, 0, sizeof(cfg));
        CLI_TST_RC(vtss_pcs_10gbase_r_config_get(req->api_inst, port_no, &cfg));

        if (req->set) {
            cfg.loopback = req->enable;
            CLI_TST_RC(vtss_pcs_10gbase_r_config_set(req->api_inst, port_no, &cfg));
        } else {
            if (first) {
                cli_table_header("Port  Side    Loopback");
            }
            printf("%-4u %-6s    %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.loopback ? "Enable" : "Disable");
        }
        CLI_TST_RC(vtss_pcs_10gbase_r_config_set(req->api_inst, port_no, &cfg));
        first = FALSE;
    }
}

static void cli_cmd_pcs_10gbase_r_status (cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_pcs_10gbase_r_status_t status;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        memset(&status, 0, sizeof(status));
        CLI_TST_RC(vtss_pcs_10gbase_r_status_get(req->api_inst, port_no, &status));
        if (first) {
            cli_table_header("Port  Side    PCS_status    Block_Lock    Hi_Ber    Test_Pattern_Match");
        }
        printf("%-4u %-6s    %-9s    %-9s     %-9s    %-9s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", status.PCS_status ? "Up" : "Down", status.block_lock ? "True" : "False", status.hi_ber ? "True" : "False", status.testpat_match ? "True" : "False");
        first = FALSE;
    }
}

static void cli_cmd_pcs_10gbase_r_stats (cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_pcs_10gbase_r_cnt_t counters;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (req->clear) {
            vtss_pcs_10gbase_r_counters_clear(req->api_inst, port_no);
            continue;
        }
        memset(&counters, 0, sizeof(counters));
        CLI_TST_RC(vtss_pcs_10gbase_r_counters_get(req->api_inst, port_no, &counters));
        if (first) {
            cli_table_header("Port  Side    BER_count    Rx_errored_block_count   Tx_errored_block_count    test_pattern_error_count");
        }
        printf("%-4u %-6s            %-9llu            %-9llu             %-9llu    %-9llu\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", counters.ber_count, counters.rx_errored_block_count, counters.tx_errored_block_count, counters.test_pattern_error_count);
        first = FALSE;
    }
}
#endif /* VTSS_FEATURE_PCS_10GBASE_R */


#if defined (VTSS_FEATURE_RAB)
static void cli_cmd_rab_config(cli_req_t *req, BOOL rx_thresh_conf, BOOL tx_thresh_conf, BOOL display_stats, BOOL cut_thru_rx, BOOL cut_thru_tx) {
    vtss_port_no_t port_no, port;
    vtss_rab_cfg_t cfg;
    vtss_rab_counters_t cntrs;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        if (display_stats) {
            memset(&cntrs, 0, sizeof(cntrs));
            CLI_TST_RC(vtss_rab_counters_get(req->api_inst, port_no, &cntrs));
            if (first) {
                cli_table_header("Port  Side    Rx_Pkt_Drop    Tx_Pkt_Drop");
            }
            printf("%-4u %-6s        %-8u        %-8u \n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cntrs.pkt_drop_cnt_rx, cntrs.pkt_drop_cnt_tx );
            first = FALSE;
            continue;
        }

        memset(&cfg, 0, sizeof(cfg));
        CLI_TST_RC(vtss_rab_config_get(req->api_inst, port_no, &cfg));

        if (req->set) {
            if (rx_thresh_conf) {
                cfg.fifo_threshold_high_rx  = req->fifo_thresh_high;
                cfg.fifo_threshold_low_rx   = req->fifo_thresh_low;
                cfg.fifo_threshold_adapt_rx = req->fifo_thresh_adapt;
            } else if (tx_thresh_conf) {
                cfg.fifo_threshold_high_tx  = req->fifo_thresh_high;
                cfg.fifo_threshold_low_tx   = req->fifo_thresh_low;
                cfg.fifo_threshold_adapt_tx = req->fifo_thresh_adapt;
            } else if (cut_thru_rx) {
                cfg.cut_thru_rx = req->enable;
            } else if (cut_thru_tx) {
                cfg.cut_thru_tx = req->enable;
            } else {
                printf("Invalid Command \n\r");
            }
            CLI_TST_RC(vtss_rab_config_set(req->api_inst, port_no, &cfg));
        } else {
            if (rx_thresh_conf) {
                if (first) {
                    cli_table_header("Port  Side    Thresh_High_Rx    Thresh_Low_Rx    Thresh_Adapt_Rx");
                }
                printf("%-4u %-6s              %-15u    %-8u      %-8u \n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.fifo_threshold_high_rx, cfg.fifo_threshold_low_rx, cfg.fifo_threshold_adapt_rx);
            } else if (tx_thresh_conf) {
                if (first) {
                    cli_table_header("Port  Side    Thresh_High_Tx    Thresh_Low_Tx    Thresh_Adapt_Tx");
                }
                printf("%-4u %-6s              %-15u    %-8u      %-8u \n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", cfg.fifo_threshold_high_tx, cfg.fifo_threshold_low_tx, cfg.fifo_threshold_adapt_tx);
            }  else if (cut_thru_rx) {
                if (first) {
                    cli_table_header("Port  Side    Cut_Thru_RX");
                }
                printf("%-4u %-6s              %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", (cfg.cut_thru_rx != FALSE)? "Enable" : "Disable");
            } else if (cut_thru_tx) {
                if (first) {
                    cli_table_header("Port  Side    Cut_Thru_Tx");
                }
                printf("%-4u %-6s              %-15s\n", port, ((port == 1) || (port == 2)) ? "Client" : "Line", (cfg.cut_thru_tx != FALSE)? "Enable" : "Disable");
            }
        }
        first = FALSE;
    }
}

static void  cli_cmd_rab_rx_thresh_config(cli_req_t *req) {
    cli_cmd_rab_config(req, 1, 0, 0, 0, 0);
}

static void  cli_cmd_rab_tx_thresh_config(cli_req_t *req) {
    cli_cmd_rab_config(req, 0, 1, 0, 0, 0);
}

static void  cli_cmd_rab_stats(cli_req_t *req) {
    cli_cmd_rab_config(req, 0, 0, 1, 0, 0);
}

static void  cli_cmd_rab_rx_cut_thru(cli_req_t *req) {
    cli_cmd_rab_config(req, 0, 0, 0, 1, 0);
}

static void  cli_cmd_rab_tx_cut_thru(cli_req_t *req) {
    cli_cmd_rab_config(req, 0, 0, 0, 0, 1);
}
#endif /* VTSS_FEATURE_RAB */

#if defined(DAYTONA_EVAL_INIT)
static void cli_cmd_debug_lbfpga(cli_req_t *req, BOOL write)
{
    u32 mask = 0xffffffff;
    u32 value;
    int i;

    if (write) {
        if(req->set)
            mask = req->fpga_mask;
        lbfpga_write(req->lbfpga_reg, req->value, mask);
    }
    else {
        lbfpga_read(req->lbfpga_reg, &value);
        if (req->binary) {
            printf("31     24 23     16 15      8 7       0\n");
            for (i = 31; i >= 0 ; i--) {
                printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i==0 ? "" : ".");
            }
        } else if (req->decimal)
            printf("%11u", value);
        else 
            printf("0x%08x", value);
        printf("\n");

    }
}
 
static void cli_cmd_debug_pcf8575(cli_req_t *req, BOOL write)
{
    u16 mask = 0xffff;
    u16 value;
    int i;

    if (write) {
        if(req->set)
            mask = req->val_mask;
        pcf8575_write(req->pcf8575_tgtid, req->value, mask);
    }
    else {
        pcf8575_read(req->pcf8575_tgtid, &value);
        if (req->binary) {
            printf("15      8 7       0\n");
            for (i = 15; i >= 0 ; i--) {
                printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i==0 ? "" : ".");
            }
        } else if (req->decimal)
            printf("%6u", value);
        else 
            printf("0x%04x", value);
        printf("\n");

    }
}

static void cli_cmd_debug_vsc3172(cli_req_t *req, BOOL write)
{
    u16 mask = 0xffff;
    u16 value;
    int i;

    if (write) {
        if(req->set)
            mask = req->val_mask;
        vsc3172_write((u16)req->vsc3172_reg, req->value, mask);
    }
    else {
        vsc3172_read((u16)req->vsc3172_reg, &value);
        if (req->binary) {
            printf("15      8 7       0\n");
            for (i = 15; i >= 0 ; i--) {
                printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i==0 ? "" : ".");
            }
        } else if (req->decimal)
            printf("%6u", value);
        else 
            printf("0x%04x", value);
        printf("\n");
    }
}

static void cli_cmd_debug_si5338(cli_req_t *req, BOOL write)
{
    u16 mask = 0xffff;
    u16 value;
    int i;

    if (write) {
        if(req->set)
            mask = req->val_mask;
        si5338_write(req->si5338_tgtid, (u16)req->si5338_reg, (u16)req->value, mask);
    }
    else {
        si5338_read(req->si5338_tgtid, (u16)req->si5338_reg, &value);
        if (req->binary) {
            printf("15      8 7       0\n");
            for (i = 15; i >= 0 ; i--) {
                printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i==0 ? "" : ".");
            }
        } else if (req->decimal)
            printf("%6u", value);
        else 
            printf("0x%04x", value);
        printf("\n");
    }
}

static void cli_cmd_debug_si5326(cli_req_t *req, BOOL write)
{
    u16 mask = 0xffff;
    u16 value;
    int i;

    if (write) {
        if(req->set)
            mask = req->val_mask;
        si5326_write(req->si5326_tgtid, (u16)req->si5326_reg, (u16)req->value, mask);
    }
    else {
        si5326_read(req->si5326_tgtid, (u16)req->si5326_reg, &value);
        if (req->binary) {
            printf("15      8 7       0\n");
            for (i = 15; i >= 0 ; i--) {
                printf("%d%s", value & (1<<i) ? 1 : 0, (i % 4) || i==0 ? "" : ".");
            }
        } else if (req->decimal)
            printf("%6u", value);
        else 
            printf("0x%04x", value);
        printf("\n");
    }
}
static void cli_cmd_debug_clk_sync(cli_req_t *req)
{
  int client, line;
  u32 value, channel, target, reg_addr;
  
  channel = req->channel_num;

  client = SI5326_CLK_GEN_BASE+(channel*2) + (req->appl_inst * 4);
  line = SI5326_CLK_GEN_BASE+(channel*2)+1 + (req->appl_inst * 4);

  /* SI5326 settings */
  si5326_write(client, 1, 0, 0xC);
  si5326_write(client, 1, 1, 0x3);
  si5326_write(client, 136, (1 << 6), ((1 << 6)));
  sleep(1);
  si5326_write(line, 1, 0, 0xC);
  si5326_write(line, 1, 1, 0x3);
  si5326_write(line, 136, (1 << 6), ((1 << 6)));
  sleep(1);
  /* Device settings */
  target = 0x8 + (channel*4);
  reg_addr = (((target << 14) | 0x2804) << 2);
  vtss_reg_read(req->api_inst, 0, reg_addr, &value);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value | 0x2));
  sleep(1);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value & ~0x2));

  reg_addr = (((++target << 14) | 0x404) << 2);
  vtss_reg_read(req->api_inst, 0, reg_addr, &value);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value | 0x2));
  sleep(1);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value & ~0x2));

  reg_addr = (((++target << 14) | 0x404) << 2);
  vtss_reg_read(req->api_inst, 0, reg_addr, &value);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value | 0x2));
  sleep(1);
  vtss_reg_write(req->api_inst, 0, reg_addr, (value & ~0x2));

  /* SI5326 settings */
  si5326_write(client, 1, (1 << 2), 0xC);
  si5326_write(client, 1, 0, 0x3);
  si5326_write(line, 1, (1 << 2), 0xC);
  si5326_write(line, 1, 0, 0x3);
}

#endif /* DAYTONA_EVAL_INIT */

#if defined(VTSS_FEATURE_OHA)
static void cli_cmd_oha_conf(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_oha_cfg_t cfg;
    BOOL first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;
        if (req->set) {
            if (req->oha_rosi_tosi == 2) {
                cfg.tosi_rosi_otu2_1 = VTSS_OHA_TOSI_ROSI_OTU2_1;
            } else if (req->oha_rosi_tosi == 1){
                cfg.tosi_rosi_otu2_1 = VTSS_OHA_TOSI_ROSI_EWIS2;
            } else if (req->oha_rosi_tosi == 0){
                cfg.tosi_rosi_otu2_1 = VTSS_OHA_TOSI_ROSI_NONE;
            }
          
            if (req->oha_otn == 2) {
                cfg.otu2_1_otu2_2 = VTSS_OHA_OTU2_OTU2_2;
            } else if (req->oha_otn == 1){
                cfg.otu2_1_otu2_2 = VTSS_OHA_OTU2_OTU2_1;
            } else {
                cfg.otu2_1_otu2_2 = VTSS_OHA_OTU2_NONE;
            }
            CLI_TST_RC(vtss_oha_config_set(req->api_inst, port_no, &cfg));
        } else {
            CLI_TST_RC(vtss_oha_config_get(req->api_inst, port_no, &cfg));
            if (first) {
                printf("%-10s %-10s \n", "ROSI/TOSI", "OTN1_OTN2");
                first = 0;
            }
            printf("%-10s %-10s \n", (cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_EWIS2) ? "EWIS2" : \
                                     ((cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_OTU2_1) ? "OTU2_1" : "NONE"),
                                     (cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_1) ? "OTU2_1" : \
                                     ((cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_2) ? "OTU2_2" : "NONE"));
        }
    }
}
#endif

#if defined(VTSS_FEATURE_I2C)

/*******************************************************************************************************
 * The following commands are to demonstrate the VTSS API as I2C driver for DAYTONA device.            *
 * It's not a generic I2C driver.                                                                      *
 *                                                                                                     *
 * The code written in command handlers is tested with PCF8575 and JDSU SFP+ devices.                  *
 *                                                                                                     *
 * The following description explains the commands added for I2C read/write sake                       *
 *                                                                                                     *
 * Initialize the controller                                                                           *
 * -------------------------                                                                           *
 * I2C controller needs to be initilized prior to any read or write. I2C controller's pre-scale value  *
 * needs to be configured according to the I2C device to be accessed. The command goes like this       *
 *                                                                                                     * 
 *  i2c init <controller-no>                                                                           *
 *                                                                                                     *
 * Write operation:                                                                                    *
 * ----------------                                                                                    *
 *  i2c tx <controller-no> <dev_addr> <reg_addr> <data>                                                *
 *                                                                                                     *
 * Read operation                                                                                      *
 * ------------------                                                                                  *
 *  i2c rx <controller-no> <dev_addr> <reg_addr>                                                       *
 *                                                                                                     *
 * Note: To accommodate the PCF8575, this command does 2-byte operations. Otherwise this command       *
 *       does byte operations only.                                                                    *
 *                                                                                                     *
 *******************************************************************************************************/
 
static void cli_cmd_i2c_init(cli_req_t *req)
{
    vtss_rc               rc;
    vtss_i2c_controller_t controller_cfg;
    vtss_i2c_cfg_t        i2c_cfg;


    controller_cfg.controller_no = req->i2c_controller_no - 1;

    /* Read the configuration from HW */
    rc = vtss_i2c_conf_get(0, &controller_cfg, &i2c_cfg);
    if (rc != VTSS_RC_OK) {
        return;
    }
    /* Apply back the new configuration */
    i2c_cfg.enable = TRUE;

    /* pre_scale needs to be calculated based on (sys_clk/(5 * SCL frequency)) - 1 equarion                  *
     * In our environment sys_clk is 250MHz. To make sure it works at lower rates, we are taking SCL as 1KHz.*
     * We tested with this value using JDSU SFP+ modules and PCF8575 device                                  *
     * This value varies based on I2C device and should be calculated accordingly                            */
      
    i2c_cfg.pre_scale = 0xbf67; 
          
    rc = vtss_i2c_conf_set(0, &controller_cfg, &i2c_cfg);
    if (rc != VTSS_RC_OK) {
        return;
    }

    return;
}

static u8 i2c_cmd(u32 i2c_controller_no, u8 cmd)
{
    vtss_i2c_controller_t controller_cfg;
    u8                    error_stage = 0;
    u16                   max_loop_cnt = 500;
    vtss_rc               rc = VTSS_RC_OK;
    u16                   tmp_cnt = 0;
    u16                   status;


    controller_cfg.controller_no = i2c_controller_no;

    do {

        /* Step 1: Initiate the command                              */
        rc = vtss_i2c_cmd(0, &controller_cfg, cmd);
        if (rc != VTSS_RC_OK) {
            error_stage = 1;
            break;
        }
        tmp_cnt = 0;

        /* Step 2: Wait for the TIP status to be cleared               */
        while(tmp_cnt < max_loop_cnt) {
            rc = vtss_i2c_status_get(0, &controller_cfg, &status);
            if (rc != VTSS_RC_OK) {
                error_stage = 2;
                break;
            }
            if (!(status & VTSS_I2C_STATUS_TIP)) {
                break;
            } 
            tmp_cnt++;
            continue;
        }
        if (tmp_cnt == max_loop_cnt || rc != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            error_stage = 3;
            break;
        }
        usleep(5);
        /* Step 3: Wait for the TX Acknowledgement                    */
        if (cmd & VTSS_I2C_CMD_STA) {
            tmp_cnt = 0;
            while(tmp_cnt < max_loop_cnt) {
                if ( (status & VTSS_I2C_STATUS_TXACK_STICKY) ||
                     (status & VTSS_I2C_STATUS_TC_ACK) ) {
                    break;
                } 
                rc = vtss_i2c_status_get(0, &controller_cfg, &status);
                if (rc != VTSS_RC_OK) {
                    error_stage = 4;
                    break;
                }
                tmp_cnt++;
                continue;
            }
            if (tmp_cnt == max_loop_cnt || rc != VTSS_RC_OK) {
                rc = VTSS_RC_ERROR;
                error_stage = 5;
                break;
            }
        } else if (cmd & VTSS_I2C_CMD_WR) {
        /* Step 4: Wait for the TX Acknowledgement                    */
            tmp_cnt = 0;
            while(tmp_cnt < max_loop_cnt) {
                if (status & VTSS_I2C_STATUS_TC_ACK) {
                    break;
                } 
                rc = vtss_i2c_status_get(0, &controller_cfg, &status);
                if (rc != VTSS_RC_OK) {
                    error_stage = 6;
                    break;
                }
                tmp_cnt++;
                continue;
            }
            if (tmp_cnt == max_loop_cnt || rc != VTSS_RC_OK) {
                rc = VTSS_RC_ERROR;
                error_stage = 7;
                break;
            }
       } 
       /* VTSS_I2C_CMD_RD is handled along with TIP */ 
    } while(0); /* end of do-while */

    if (error_stage) {
        printf ("Error occured while executing the command:%u\n", cmd);
    }

    return error_stage;
}

static void cli_cmd_i2c_rx(cli_req_t *req)
{
    vtss_rc               rc;
    u8                    data = 0;
    vtss_i2c_controller_t controller_cfg;
    u8                    error_stage = 0;
    u16                   tmp_data = 0;

    do {

        controller_cfg.controller_no = req->i2c_controller_no-1;

        /* Step 1: Write target address with write mode               */
        data = req->i2c_dev_addr << 1;
        rc = vtss_i2c_tx(0, &controller_cfg, &data);

        if (rc != VTSS_RC_OK) {
            error_stage = 1;
            break;
        }
        /* Step 2: Initiate the write command with start bit          */
        error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STA | VTSS_I2C_CMD_WR);

        if (error_stage) {
            break;
        }

        /* Step 3: Write register address                              */
        data = req->i2c_reg_addr;
        rc = vtss_i2c_tx(0, &controller_cfg, &data);

        if (rc != VTSS_RC_OK) {
            error_stage = 2;
            break;
        }
        
        /* Step 4: Initiate the write command                         */
        error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_WR);

        if (error_stage) {
            break;
        }

        /* Step 5: Write target address with read mode                */
        data = (req->i2c_dev_addr << 1) | 1;
        rc = vtss_i2c_tx(0, &controller_cfg, &data);

        if (rc != VTSS_RC_OK) {
            error_stage = 3;
            break;
        }

        /* Step 6: Initiate the repeated start with write command     */
        error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STA | VTSS_I2C_CMD_WR);

        if (error_stage) {
            break;
        }

        /* Step 7: Initiate the read operation                        */
        /* This operation reads the specified register at Step 3      */
        if (req->i2c_dev_addr == 0x20) {
            error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_RD);
        } else {
            error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_RD | VTSS_I2C_CMD_RD_ACK);
        }

        if (error_stage) {
            break;
        }

        /* Step 8: Read the data from the I2C controller */
        rc = vtss_i2c_rx(NULL, &controller_cfg, &data);
        if (rc != VTSS_RC_OK) {
            rc = VTSS_RC_ERROR;
            error_stage = 4;
            break;
        }
        tmp_data = data;
        /* We can continue further reading by issusing read commands       */
        /* Below code shows an example for PCF 8575 devices                */
        if (req->i2c_dev_addr == 0x20) {
            /* Step 7: Initiate the read operation                         */
            error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_RD | VTSS_I2C_CMD_RD_ACK);

            if (error_stage) {
                break;
            }
            /* Step 8: Read the data from the I2C controller */
            rc = vtss_i2c_rx(NULL, &controller_cfg, &data);
            if (rc != VTSS_RC_OK) {
                rc = VTSS_RC_ERROR;
                error_stage = 5;
                break;
            }
            tmp_data |= (data << 8);
        }
    } while(0); /* end-of-do-while */

    if (rc != VTSS_RC_OK || error_stage) {
        printf ("Read operation failed..\n");
    } else {
        printf ("Read data: 0x%x\n", tmp_data);
    }

    do {
        /* Step:10 Stop the operation */
        rc = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STO);
        if (rc != VTSS_RC_OK) {
            break;
        }
    } while(0); /* end-of-do-while */
    return;
}

static void cli_cmd_i2c_tx(cli_req_t *req)
{
    vtss_rc               rc;
    u8                    data = 0;
    vtss_i2c_controller_t controller_cfg;
    u8                    error_stage = 0;

    do {

        controller_cfg.controller_no = req->i2c_controller_no-1;

        /* Step 1: Write target address with write mode               */
        data = req->i2c_dev_addr << 1;
        rc = vtss_i2c_tx(0, &controller_cfg, &data);

        if (rc != VTSS_RC_OK) {
            error_stage = 1;
            break;
        }
        /* Step 2: Initiate the write command with start bit          */
        error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STA | VTSS_I2C_CMD_WR);

        if (error_stage) {
            break;
        }
        if (req->i2c_dev_addr != 0x20) {

            /* Step 3: Write register address                               */
            data = req->i2c_reg_addr;
            rc = vtss_i2c_tx(0, &controller_cfg, &data);

            if (rc != VTSS_RC_OK) {
                error_stage = 2;
                break;
            }

            /* Step 4: Initiate the write command                         */
            error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_WR);

            if (error_stage) {
                break;
            }
        }

        /* Step 5: Write data                                           */
        data = (req->i2c_data & 0xff);
        rc = vtss_i2c_tx(0, &controller_cfg, &data);

        if (rc != VTSS_RC_OK) {
            error_stage = 3;
            break;
        }
        /* Step 6: Initiate the write command                         */
        error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_WR);

        if (error_stage) {
            error_stage = 4;
            break;
        }
        if (req->i2c_dev_addr == 0x20) {
            /* Step 7: Write data                                           */
            data = (req->i2c_data & 0xff00) >> 8;
            rc = vtss_i2c_tx(0, &controller_cfg, &data);

            if (rc != VTSS_RC_OK) {
                error_stage = 5;
                break;
            }

            /* Step 8: Initiate the write command                         */
            error_stage = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_WR);

            if (error_stage) {
                error_stage = 6;
                break;
            }
        }
    } while(0); /* end-of-do-while */

    if (rc != VTSS_RC_OK || error_stage) {
        printf ("Write operation failed..\n");
    } else {
        printf ("Write operation success..\n");
    }

    do {
        /* Step:10 Stop the operation */
        rc = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STO);
        if (rc != VTSS_RC_OK) {
            break;
        }
#if 0
        rc = i2c_cmd(controller_cfg.controller_no, VTSS_I2C_CMD_STO);
        if (rc != VTSS_RC_OK) {
            break;
        }
#endif
    } while(0); /* end-of-do-while */

    return;

}

#endif /* VTSS_FEATURE_I2C */


#if defined VTSS_ARCH_DAYTONA  /* KPV */
static void cli_cmd_pmtick_config(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_pmtick_control_t pmtick_control;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->port_list[port] == 0)
            continue;

        pmtick_control.period = req->pmtick_interval;
        pmtick_control.mode   = req->pmtick_mode;
        vtss_pmtick_set(req->api_inst, port_no, &pmtick_control);

    }
}
#endif /* VTSS_CHIP_DAYTONA */
#ifdef VTSS_ARCH_DAYTONA
static void cli_cmd_channel_mode_config(cli_req_t *req)
{
#ifdef DAYTONA_EVAL_INIT
    if(req->ev_init) {
        printf("\nInitializing %u channel\n", req->channel_num);
        if (d_init_channel_clock_sw(req->channel_num, req->channel_mode, 0, 0) != VTSS_RC_OK) {
            T_E("d_init_channel_clock_sw returned Failure for channel 0");
        }
        if ((req->channel_num % 2) == 0) {
          if (d_init_board_conn_sw(req->channel_num, IF_UPI0, IF_XAUI0, PROTO_10GELAN, 0) != VTSS_RC_OK) {
            T_E("d_init_board_conn_sw returned Failure for channel 0");
          }
        }
        else
        {
          if (d_init_board_conn_sw(req->channel_num, IF_UPI1, IF_XAUI1, PROTO_10GELAN, 0) != VTSS_RC_OK) {
            T_E("d_init_board_conn_sw returned Failure for channel 1");
          }
        }
    }
#endif
     vtss_channel_mode_set(req->api_inst, req->channel_num , req->channel_mode, req->channel_xauimode);
}
#endif


#if defined(VTSS_FEATURE_PACKET)
static void cli_cmd_port_transmit(cli_req_t *req)
{
    vtss_rc rc;
    vtss_port_no_t                     port, port_no;
    u8 frame[64];

    /* NULL BC */
    memset(frame, 0, sizeof(frame));
    memset(frame, 0xff, 6);
    frame[6+0] = 0x11;
    frame[6+1] = 0x22;
    frame[6+2] = 0x33;
    frame[6+3] = 0x44;
    frame[6+4] = 0x55;
    frame[6+5] = 0x66;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if(req->port_list[port]) {
            printf("Transmit %d\n", port);
            if ((rc = vtss_packet_tx_frame_port(NULL, port_no, frame, sizeof(frame))) != VTSS_RC_OK) {
                T_E("tx failure: rc %d", rc);
            }
        }
    }
}

static vtss_rc board_or_api_tx(struct vtss_board_t        *board, 
                               const vtss_packet_tx_ifh_t *const ifh,
                               const u8                   *const frame,
                               const u32                   length)
{
    if (board->board_transmit) {
        return board->board_transmit(board, ifh, frame, length);
    }
    return vtss_packet_tx_frame(NULL, ifh, frame, length);
}


static void cli_cmd_transmit_frame(cli_req_t *req, BOOL switched)
{
    static u8 sendbuf[200] = {
        0x01, 0x80, 0xc2, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc1, 0x00, 0x71, 0xff, 0xaa, 0xbb, 0xcc, 0xdd,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    };
    vtss_packet_tx_ifh_t ifh;
    vtss_packet_tx_info_t tx_info;

    vtss_packet_tx_info_init(NULL, &tx_info);
    if (switched) {
        tx_info.tag.vid = req->vid;
        tx_info.switch_frm = TRUE;
        printf("Tx VID %d (switched)\n", tx_info.tag.vid);
    } else {
        vtss_port_no_t port_no;
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)  {
            if(req->port_list[iport2uport(port_no)]) {
                tx_info.dst_port_mask |= (1 << port_no);
            }
        }
        printf("Tx port mask 0x%" PRIx64 "\n", tx_info.dst_port_mask);
    }
    if (vtss_packet_tx_hdr_compile(NULL, &tx_info, &ifh) != VTSS_RC_OK ||
        board_or_api_tx(req->board, &ifh, sendbuf, sizeof(sendbuf)) != VTSS_RC_OK) { 
        printf("vtss_packet_tx_hdr_compile/vtss_packet_tx_frame(): Error\n");
    }
}
#endif

#if defined(NPI_INTERFACE)
static void cli_cmd_transmit_npi(cli_req_t *req)
{
#define BUF_SIZ 256
    char sendbuf[BUF_SIZ];
    struct sockaddr_ll socket_address;
    int tx_len, port_no;
    struct ifreq if_idx;
    extern int npi_sock;
    vtss_packet_tx_info_t tx_info;
    u32 length;
    static char encap[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x88, 0x80, 0x00, 0x05
    };

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, NPI_INTERFACE, IFNAMSIZ-1);
    if (ioctl(npi_sock, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");

    memset(sendbuf, 0, BUF_SIZ);

    /* Encap header */
    memcpy(sendbuf, encap, sizeof(encap));
    tx_len = sizeof(encap);
 
    /* IFH */
    vtss_packet_tx_info_init(NULL, &tx_info);
    tx_info.dst_port_mask = 0;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)  {
        if(req->port_list[iport2uport(port_no)])
            tx_info.dst_port_mask |= (1 << port_no);
    }
    length = BUF_SIZ - tx_len; /* Plenty of room */
    if(vtss_packet_tx_hdr_encode(NULL, &tx_info, (u8*) sendbuf+tx_len, &length) == VTSS_RC_OK) {
        tx_len += length;
    } else {
        T_E("vtss_packet_tx_hdr_encode");
        return;
    }
                
    /* Construct the Ethernet header */
    memset(sendbuf+tx_len, 0xff, 6);
    tx_len += 6;
    memset(sendbuf+tx_len, 0x02, 6);
    tx_len += 6;
    sendbuf[tx_len++] = 0xca;
    sendbuf[tx_len++] = 0xfe;

    /* Packet data */
    sendbuf[tx_len++] = 0xde;
    sendbuf[tx_len++] = 0xad;
    sendbuf[tx_len++] = 0xbe;
    sendbuf[tx_len++] = 0xef;
 
    tx_len += 60;           /* Bogus payload */

    /* Index of the output network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length */
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    memcpy(socket_address.sll_addr, encap, 6); /* Don't-care address */
 
    /* Send packet */
    if (sendto(npi_sock, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
        perror("sendto");
}
#endif  /* NPI_INTERFACE */

static void cli_cmd_debug_api(cli_req_t *req)
{
    vtss_debug_info_t  info;
    vtss_debug_group_t group;
    vtss_port_no_t     iport;
    
    if (req->api_group == VTSS_DEBUG_GROUP_COUNT) {
        printf("Legal groups are:\n\n");
        for (group = VTSS_DEBUG_GROUP_ALL; group < VTSS_DEBUG_GROUP_COUNT; group++) {
            if (cli_api_group_table[group]) {
                printf("%s\n", cli_api_group_table[group]);
            } else {
                printf("%d - unknown\n", group);
            }
        }
    } else if (vtss_debug_info_get(&info) == VTSS_RC_OK) {
        info.layer = req->api_layer;
        info.group = req->api_group;
        info.full = req->full;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++)
            info.port_list[iport] = req->port_list[iport2uport(iport)];
        vtss_debug_info_print(NULL, (vtss_debug_printf_t)printf, &info);
    }
}

/* Parse command */
static void cli_command(cli_cmd_id_t id, cli_req_t *req)
{
    switch (id) {
    case CLI_CMD_ID_HELP:
        cli_cmd_help();
        break;
    case CLI_CMD_ID_SYSTEM_CONF:
        printf("Version: %s\n", vtss_version_string);
        printf("Date   : %s\n", vtss_compile_time);
        break;
#if (VTSS_APPL_INST_CNT > 1)
    case CLI_CMD_ID_SYSTEM_INST:
        if (req->set)
            cli_inst_update(req->appl_inst);
        else
            printf("System Instance: %d\n", req->appl_inst);
        break;
#endif /* VTSS_APPL_INST_CNT > 1 */
#if defined(VTSS_ARCH_B2)
    case CLI_CMD_ID_SYSTEM_INIT:
        cli_cmd_system_init(req);
        break;
#endif /* VTSS_ARCH_B2 */
    case CLI_CMD_ID_SYSTEM_RESET:
#if defined(VTSS_OPSYS_ECOS)
        HAL_PLATFORM_RESET();
#endif /* VTSS_OPSYS_ECOS */
        break;
#if defined(VTSS_FEATURE_WARM_START)
    case CLI_CMD_ID_SYSTEM_RESTART:
        cli_cmd_system_restart(req);
        break;
#endif /* VTSS_FEATURE_WARM_START */
#ifndef VTSS_ARCH_DAYTONA
    case CLI_CMD_ID_PORT_CONF:
        cli_cmd_port_conf(req, 1, 1, 1, 1);
        break;
    case CLI_CMD_ID_PORT_STATE:
        cli_cmd_port_conf(req, 1, 0, 0, 0);
        break;
    case CLI_CMD_ID_PORT_MODE:
        cli_cmd_port_conf(req, 0, 1, 0, 0);
        break;
    case CLI_CMD_ID_PORT_FLOW_CONTROL:
        cli_cmd_port_conf(req, 0, 0, 1, 0);
        break;
#endif /* ifndef VTSS_ARCH_DAYTONA */
#if defined(VTSS_FEATURE_PACKET)
    case CLI_CMD_ID_PORT_TRANSMIT:
        cli_cmd_port_transmit(req);
        break;
#endif
#if defined(VTSS_FEATURE_PORT_CONTROL)
#ifndef VTSS_ARCH_DAYTONA
    case CLI_CMD_ID_PORT_MAX_FRAME:
        cli_cmd_port_conf(req, 0, 0, 0, 1);
        break;
    case CLI_CMD_ID_PORT_STATS:
        cli_cmd_port_stats(req);
        break;
#endif /* ifndef VTSS_ARCH_DAYTONA */
    case CLI_CMD_ID_PORT_MAP:
        cli_cmd_port_map(req);
        break;
#endif /* VTSS_FEATURE_PORT_CONTROL */
#if defined(VTSS_FEATURE_LAYER2)
    case CLI_CMD_ID_MAC_ADD:
        cli_cmd_mac_add(req);
        break;
    case CLI_CMD_ID_MAC_DEL:
        cli_cmd_mac_del(req);
        break;
    case CLI_CMD_ID_MAC_LOOKUP:
        cli_cmd_mac_lookup(req);
        break;
    case CLI_CMD_ID_MAC_DUMP:
        cli_cmd_mac_dump(req);
        break;
    case CLI_CMD_ID_MAC_FLUSH:
        cli_cmd_mac_flush(req);
        break;
    case CLI_CMD_ID_MAC_AGE_TIME:
        cli_cmd_mac_age_time(req);
        break;
#endif /* VTSS_FEATURE_LAYER2 */
#if defined(VTSS_ARCH_B2)
    case CLI_CMD_ID_PORT_FILTER:
        cli_cmd_port_filter(req);
        break;
    case CLI_CMD_ID_PORT_PSI:
        cli_cmd_psi_clock(req);
      break;
    case CLI_CMD_ID_PORT_FORWARD:
        cli_cmd_port_forward(req);
        break;
    case CLI_CMD_ID_PORT_CT:
        cli_cmd_b2_port_conf(req, 1, 0);
        break;
    case CLI_CMD_ID_PORT_PFCI:
        cli_cmd_b2_port_conf(req, 0, 1);
      break;
    case CLI_CMD_ID_QOS_GLOBAL_DSCP:
        cli_cmd_qos_global_dscp(req);
        break;
    case CLI_CMD_ID_QOS_DSCP_TABLE:
        cli_cmd_qos_dscp_table(req);
        break;
    case CLI_CMD_ID_QOS_GLOBAL_TCPUDP:
        cli_cmd_qos_global_tcpudp(req);
        break;
    case CLI_CMD_ID_QOS_GLOBAL_TCPUDP_ENABLE:
        cli_cmd_qos_tcpudp_enable(req);
        break;
    case CLI_CMD_ID_QOS_GLOBAL_FILTTRIG:
        cli_cmd_qos_global_filter(req,1);
        break;
    case CLI_CMD_ID_QOS_GLOBAL_FILTACTION:
        cli_cmd_qos_global_filter(req,0);
        break;
    case CLI_CMD_ID_QOS_FILTTRIG:
        cli_cmd_qos_filter(req,1);
        break;
    case CLI_CMD_ID_QOS_FILTACTION:
        cli_cmd_qos_filter(req,0);
        break;
    case CLI_CMD_ID_QOS_GLOBAL_FILTENABLE:
        cli_cmd_qos_global_filter_enable(req);
        break;
    case CLI_CMD_ID_QOS_PRIORDER:
        cli_cmd_qos_priorder(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_MISC:
        cli_cmd_qos_class_misc(req);
        break;

    case CLI_CMD_ID_QOS_CLASS_TCPUDP:
        cli_cmd_qos_class_tcpudp(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_IP_PROTO:
        cli_cmd_qos_class_ipproto(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_ETYPE:
        cli_cmd_qos_class_etype(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_VID:
        cli_cmd_qos_class_vid(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_TAG_PRIO:
        cli_cmd_qos_class_tagprio(req);
        break;
    case CLI_CMD_ID_QOS_CLASS_MPLS:
        cli_cmd_qos_class_mpls(req);
        break;
    case CLI_CMD_ID_QOS_PORT_POLICE:
        cli_cmd_qos_port_police(req);
        break;
    case CLI_CMD_ID_QOS_PORT_SHAPER:
        cli_cmd_qos_port_shaper(req);
        break;
    case CLI_CMD_ID_QOS_LPORT_SHAPER:
        cli_cmd_qos_lport_shaper(req);
        break;
    case CLI_CMD_ID_QOS_QUEUE_POLICE:
        cli_cmd_qos_queue_police(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_ALTORDER:
        cli_cmd_qos_l2_filter_altorder(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_DMAC:
        cli_cmd_qos_l2_filter_dmac(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_VID:
        cli_cmd_qos_l2_filter_vid(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_ETYPE:
        cli_cmd_qos_l2_filter_etype(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_PORT:
        cli_cmd_qos_l2_filter_port(req);
        break;
    case CLI_CMD_ID_QOS_L2_FILTER_CONF:
        cli_cmd_qos_l2_filter_conf(req);
        break;
    case CLI_CMD_ID_QOS_RED_THRESHOLD:
        cli_cmd_qos_red_threshold(req);
        break;
    case CLI_CMD_ID_QOS_RED_PROBABILITY:
        cli_cmd_qos_red_probability(req);
        break;
    case CLI_CMD_ID_QOS_RED_PROFILE:
        cli_cmd_qos_red_profile(req);
        break;
    case CLI_CMD_ID_QOS_TAG:
        cli_cmd_qos_tag(req);
        break;
    case CLI_CMD_ID_QOS_QUEUE_SCHEDULER:
        cli_cmd_qos_queue_schedule(req);
        break;
    case CLI_CMD_ID_QOS_PORT_SCHEDULER:
        cli_cmd_qos_port_schedule(req);
        break;
    case CLI_CMD_ID_QOS_LPORT_SCHEDULER:
        cli_cmd_qos_lport_schedule(req);
        break;
    case CLI_CMD_ID_MAP_PORT:
        vtss_vid2port_set(req->api_inst, req->vid, req->disable ? VTSS_PORT_NO_NONE : uport2iport(req->port));
        break;
    case CLI_CMD_ID_MAP_LPORT:
        vtss_vid2lport_set(req->api_inst, req->vid, req->disable ? VTSS_LPORT_NONE : req->lport);
        break;      
#if defined(VTSS_GPIOS)
    case CLI_CMD_ID_CLK_GPIO:
        cli_cmd_clk_gpio(req, req->disable ? VTSS_PORT_NO_NONE : req->port);
        break;
#endif /* VTSS_GPIOS */
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_CHIP_10G_PHY)
    case CLI_CMD_ID_PHY_10G_MODE:
        cli_cmd_phy_10g_mode(req);
        break;
    case CLI_CMD_ID_PHY_10G_STATUS:
        cli_cmd_phy_10g_status(req);
        break;
    case CLI_CMD_ID_PHY_10G_RESET:
        cli_cmd_phy_10g_reset(req);
        break;
    case CLI_CMD_ID_PHY_10G_LOOPBACK:
        cli_cmd_phy_10g_loopback(req);
        break;
    case CLI_CMD_ID_PHY_10G_STATS:
        cli_cmd_phy_10g_stats(req);
        break;
    case CLI_CMD_ID_PHY_10G_POWER:
        cli_cmd_phy_10g_power(req);
        break;
#endif /* VTSS_CHIP_10G_PHY */
    case CLI_CMD_ID_DEBUG_API:
        cli_cmd_debug_api(req);
        break;
#if defined(VTSS_FEATURE_MISC)
    case CLI_CMD_ID_DEBUG_REG_READ:
    case CLI_CMD_ID_DEBUG_REG_READ_DAYTONA:
        cli_cmd_debug_reg(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_REG_READ_L28:
        cli_cmd_debug_reg_l28(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_REG_WRITE:
    case CLI_CMD_ID_DEBUG_REG_WRITE_DAYTONA:
        cli_cmd_debug_reg(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_REG_WRITE_L28:
        cli_cmd_debug_reg_l28(req, 1);
        break;
#endif /* VTSS_FEATURE_MISC */
#if defined(VTSS_ARCH_B2)
    case CLI_CMD_ID_DEBUG_REG_CHECK:
        cli_cmd_debug_reg_check(req);
        break;
    case CLI_CMD_ID_DEBUG_TARGETS:
        cli_cmd_debug_targets();
#endif /* VTSS_ARCH_B2 */
#ifndef VTSS_ARCH_DAYTONA
    case CLI_CMD_ID_DEBUG_PHY_READ:
        cli_cmd_debug_phy(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_PHY_WRITE:
        cli_cmd_debug_phy(req, 1);
        break;
#endif /* ifndef VTSS_ARCH_DAYTONA */
#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
    case CLI_CMD_ID_DEBUG_MMD_READ:
        cli_cmd_debug_mmd(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_MMD_WRITE:
        cli_cmd_debug_mmd(req, 1);
        break;
#endif /* VTSS_FEATURE_10G */
#if defined(VTSS_FEATURE_MISC)
    case CLI_CMD_ID_DEBUG_CHIP_ID:
    {
        vtss_chip_id_t id;
        if (vtss_chip_id_get(req->api_inst,&id) == VTSS_RC_OK) {
            printf("Part Number: 0x%04x\n", id.part_number);
            printf("Revision   : %u\n", id.revision);
#ifdef VTSS_ARCH_DAYTONA
            printf("Variant    : %u\n", id.variant);
#endif /* VTSS_ARCH_DAYTONA */
        }
        break;
    }
#endif /* VTSS_FEATURE_MISC */
    case CLI_CMD_ID_DEBUG_SUSPEND:
        cli_cmd_debug_suspend(1);
        break;
    case CLI_CMD_ID_DEBUG_RESUME:
        cli_cmd_debug_suspend(0);
        break;
    case CLI_CMD_ID_DEBUG_TRACE:
        cli_cmd_debug_trace(req);
        break;
#if defined(BOARD_B2_EVAL) 
    case CLI_CMD_ID_DEBUG_FPGA_READ:
        cli_cmd_debug_fpga(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_FPGA_WRITE:
        cli_cmd_debug_fpga(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_FPGA_STATUS:
        cli_cmd_debug_fpga_status(req);
        break;
#endif /* BOARD_B2_EVAL */
#if defined(VTSS_ARCH_B2)
    case CLI_CMD_ID_DEBUG_SPI4:
        cli_cmd_debug_spi4(req);
        break;
    case CLI_CMD_ID_DEBUG_QSS:
        cli_cmd_debug_qss(req);
        break;
    case CLI_CMD_ID_DEBUG_BUF:
        cli_cmd_debug_qss_bufcnt(req);
        break;
    case CLI_CMD_ID_DEBUG_RED_CNT:
        cli_cmd_debug_red_cnt(req);
        break;
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_GPIOS)
    case CLI_CMD_ID_DEBUG_GPIO_OUTPUT:
    case CLI_CMD_ID_DEBUG_GPIO_READ:
    case CLI_CMD_ID_DEBUG_GPIO_WRITE:
        cli_cmd_debug_gpio(req, id);
        break;
#endif /* VTSS_GPIOS */
#if defined(VTSS_FEATURE_WIS)
    case CLI_CMD_ID_WIS_MODE:
        cli_cmd_wis_mode(req);
        break;
    case CLI_CMD_ID_WIS_SECTION_ATTI_GET:
        cli_cmd_wis_satti_get(req);
        break;
    case CLI_CMD_ID_WIS_PATH_ATTI_GET:
        cli_cmd_wis_patti_get(req);
        break;
    case CLI_CMD_ID_WIS_SECTION_TXTTI:
        cli_cmd_wis_section_tti(req);
        break;
    case CLI_CMD_ID_WIS_PATH_TXTTI:
        cli_cmd_wis_path_tti(req);
        break;
    case CLI_CMD_ID_WIS_PATH_TXSL:
        cli_cmd_wis_path_txsl(req);
        break;
    case CLI_CMD_ID_WIS_STATUS:
        cli_cmd_wis_status(req);
        break;
    case CLI_CMD_ID_WIS_CONS_ACT:
        cli_cmd_wis_cons_act(req);
        break;
    case CLI_CMD_ID_WIS_COUNTERS:
        cli_cmd_wis_counters(req);
        break;
    case CLI_CMD_ID_WIS_DEFECTS:
        cli_cmd_wis_defects(req);
        break;
    case CLI_CMD_ID_WIS_OH_CONF:
        cli_cmd_wis_oh_conf(req);
        break;
    case CLI_CMD_ID_WIS_PREF_COUNTERS:
        cli_cmd_wis_pref_counters(req);
        break;
    case CLI_CMD_ID_WIS_PASSTHRU:
        cli_cmd_wis_passthru(req);
        break;
    case CLI_CMD_ID_WIS_FORCED_INSERTION:
        cli_cmd_wis_forced_insertion(req);
        break;
    case CLI_CMD_ID_WIS_PERF_MODE:
        cli_cmd_wis_perf_mode(req);
        break;
#if defined(VTSS_DAYTONA_WIS_CLI_ALL)
    case CLI_CMD_ID_WIS_RESET:
        cli_cmd_wis_reset(req);
        break;
    case CLI_CMD_ID_WIS_TEST_STATUS:
        cli_cmd_wis_test_status(req);
        break;
    case CLI_CMD_ID_WIS_PRBS_ERR_INJ:
        cli_cmd_wis_prbs_err_inj(req);
        break;
    case CLI_CMD_ID_WIS_TEST_MODE:
        cli_cmd_wis_test_mode(req);
        break;
    case CLI_CMD_ID_WIS_FORCE_CONF:
        cli_cmd_wis_force_conf(req);
        break;
    case CLI_CMD_ID_WIS_EVENT_FORCE:
        cli_cmd_wis_event_force(req);
        break;
    case CLI_CMD_ID_WIS_TX_PREF_THRE:
        cli_cmd_wis_tx_pref_thre(req);
        break;
#endif
#endif /* VTSS_FEATURE_WIS */
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
    case CLI_CMD_ID_1588_BLOCK_INIT:
        cli_cmd_1588_block_init(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_INIT:
        cli_cmd_1588_engine_init(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_CLEAR:
        cli_cmd_1588_engine_clear(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_MODE:
        cli_cmd_1588_engine_mode(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_CHANNEL_MAP:
        cli_cmd_1588_engine_channel_map(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ETH1_COMM_CONF:
        cli_cmd_1588_engine_eth1_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ETH1_FLOW_CONF:
        cli_cmd_1588_engine_eth1_flow_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ETH2_COMM_CONF:
        cli_cmd_1588_engine_eth2_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ETH2_FLOW_CONF:
        cli_cmd_1588_engine_eth2_flow_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_IP1_COMM_CONF:
        cli_cmd_1588_engine_ip1_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_IP1_FLOW_CONF:
        cli_cmd_1588_engine_ip1_flow_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_IP2_COMM_CONF:
        cli_cmd_1588_engine_ip2_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_IP2_FLOW_CONF:
        cli_cmd_1588_engine_ip2_flow_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_MPLS_COMM_CONF:
        cli_cmd_1588_engine_mpls_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_MPLS_FLOW_CONF:
        cli_cmd_1588_engine_mpls_flow_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ACH_COMM_CONF:
        cli_cmd_1588_engine_ach_comm_conf(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ACTION_ADD:
        cli_cmd_1588_engine_action_add(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ACTION_DELETE:
        cli_cmd_1588_engine_action_delete(req);
        break;
    case CLI_CMD_ID_1588_ENGINE_ACTION_SHOW:
        cli_cmd_1588_engine_action_show(req);
        break;
    case CLI_CMD_ID_1588_SIGNATURE_CONF:
        cli_cmd_1588_signature_conf(req);
        break;
    case CLI_CMD_ID_1588_STATS_SHOW:
        cli_cmd_1588_ts_stats_show(req);
        break;
    case CLI_CMD_ID_1588_LATENCY:
        cli_cmd_1588_latency(req);
        break;
    case CLI_CMD_ID_1588_DELAY:
        cli_cmd_1588_delay(req);
        break;
    case CLI_CMD_ID_1588_DELAY_ASYM:
        cli_cmd_1588_delay_asym(req);
        break;
    case CLI_CMD_ID_1588_MMD_READ:
        cli_cmd_1588_mmd_read(req);
        break;
    case CLI_CMD_ID_1588_MMD_WRITE:
        cli_cmd_1588_mmd_write(req);
        break;
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_FEATURE_OTN)
    case CLI_CMD_ID_OTN_OTU_ACC_RES:
        cli_cmd_otn_otu_acc_res(req);
        break;
    case CLI_CMD_ID_OTN_OTU_ACC_SMRES:
        cli_cmd_otn_otu_acc_smres(req);
        break;
    case CLI_CMD_ID_OTN_OTU_ACC_GCC0:
        cli_cmd_otn_otu_acc_gcc0(req);
        break;
    case CLI_CMD_ID_OTN_OTU_TX_RES:
        cli_cmd_otn_otu_tx_res(req);
        break;
    case CLI_CMD_ID_OTN_OTU_TX_SMRES:
        cli_cmd_otn_otu_tx_smres(req);
        break;
    case CLI_CMD_ID_OTN_OTU_TX_GCC0:
        cli_cmd_otn_otu_tx_gcc0(req);
        break;
    case CLI_CMD_ID_OTN_ODU_OH_INSERTION:
        cli_cmd_otn_odu_oh_insertion(req);
        break;
    case CLI_CMD_ID_OTN_OTU_OH_INSERTION:
        cli_cmd_otn_otu_oh_insertion(req);
        break;
    case CLI_CMD_ID_OTN_ODU_TX_RES:
        cli_cmd_otn_odu_tx_res(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACC_RES:
        cli_cmd_otn_odu_acc_res(req);
        break;
    case CLI_CMD_ID_OTN_ODU_TX_EXP:
        cli_cmd_otn_odu_tx_exp(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACC_EXP:
        cli_cmd_otn_odu_acc_exp(req);
        break;
    case CLI_CMD_ID_OTN_ODU_TX_FTFL:
        cli_cmd_otn_odu_tx_ftfl(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACC_FTFL:
        cli_cmd_otn_odu_acc_ftfl(req);
        break;
    case CLI_CMD_ID_OTN_OPU_OH_INSERTION:
        cli_cmd_otn_opu_oh_insertion(req);
        break;
    case CLI_CMD_ID_OTN_OPU_TEST_INSERTION:
        cli_cmd_otn_opu_test_insertion(req);
        break;
    case CLI_CMD_ID_OTN_OCH_LOOPBACK:
        cli_cmd_otn_och_loopback(req);
        break;
    case CLI_CMD_ID_OTN_OCH_FEC:
        cli_cmd_otn_och_fec(req);
        break;
    case CLI_CMD_ID_OTN_OCH_BER_THRESHOLD:
        cli_cmd_otn_och_ber_threshold(req);
        break;
    case CLI_CMD_ID_OTN_OCH_DEFECTS:
        cli_cmd_otn_och_defects(req);
        break;
    case CLI_CMD_ID_OTN_OCH_PERF:
        cli_cmd_otn_och_perform(req);
        break;
    case CLI_CMD_ID_OTN_OTU_TTI:
        cli_cmd_otn_otu_tti(req);
        break;
    case CLI_CMD_ID_OTN_OTU_ACCTTI:
        cli_cmd_otn_otu_acctti(req);
        break;
    case CLI_CMD_ID_OTN_OTU_CONSACTS:
        cli_cmd_otn_otu_cons_actions(req);
        break;
    case CLI_CMD_ID_OTN_OTU_CONSACTS_IAE:
        cli_cmd_otn_otu_cons_iae_actions(req);
        break;
    case CLI_CMD_ID_OTN_OTU_DEFECTS:
        cli_cmd_otn_otu_defects(req);
        break;
    case CLI_CMD_ID_OTN_OTU_PERF:
        cli_cmd_otn_otu_perform(req);
        break;
    case CLI_CMD_ID_OTN_ODU_MODE:
        cli_cmd_otn_odu_mode(req);
        break;
    case CLI_CMD_ID_OTN_ODU_TTI:
        cli_cmd_otn_odu_tti(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACCTTI:
        cli_cmd_otn_odu_acctti(req);
        break;
    case CLI_CMD_ID_OTN_ODU_AIS:
        cli_cmd_otn_odu_ais(req);
        break;
    case CLI_CMD_ID_OTN_ODU_CONSACTS_BEI:
        cli_cmd_otn_odu_cons_bei_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODU_CONSACTS_BDI:
        cli_cmd_otn_odu_cons_bdi_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODU_CONSACTS_AIS:
        cli_cmd_otn_odu_cons_ais_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODU_PAYLOAD_TYPE:
        cli_cmd_otn_odu_payload_type(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACC_PAYLOAD_TYPE:
        cli_cmd_otn_odu_acc_pltype(req);
        break;
    case CLI_CMD_ID_OTN_ODU_APS:
        cli_cmd_otn_odu_aps(req);
        break;
    case CLI_CMD_ID_OTN_ODU_ACCAPS:
        cli_cmd_otn_odu_acc_aps(req);
        break;
    case CLI_CMD_ID_OTN_ODU_DEFECTS:
        cli_cmd_otn_odu_defects(req);
        break;
    case CLI_CMD_ID_OTN_ODU_PERF:
        cli_cmd_otn_odu_perform(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_MODE:
        cli_cmd_otn_odut_mode(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_TTI:
        cli_cmd_otn_odut_tti(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_ACCTTI:
        cli_cmd_otn_odut_acctti(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_CONSACTS_BEI:
        cli_cmd_otn_odut_cons_bei_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_CONSACTS_BDI:
        cli_cmd_otn_odut_cons_bdi_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_CONSACTS_AIS:
        cli_cmd_otn_odut_cons_ais_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_CONSACTS_IAE:
        cli_cmd_otn_odut_cons_iae_actions(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_BDI:
        cli_cmd_otn_odut_bdi(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_STAT:
        cli_cmd_otn_odut_stat(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_ACCSTAT:
        cli_cmd_otn_odut_acc_stat(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_APS:
        cli_cmd_otn_odut_aps(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_ACCAPS:
        cli_cmd_otn_odut_acc_aps(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_DEFECTS:
        cli_cmd_otn_odut_defects(req);
        break;
    case CLI_CMD_ID_OTN_ODUT_PERF:
        cli_cmd_otn_odut_perform(req);
        break;
#endif /* VTSS_FEATURE_OTN */
#if defined(VTSS_FEATURE_GFP)
    case CLI_CMD_ID_GFP_SINGLE_ERR_CORR:
        cli_cmd_gfp_single_err_corr(req);
        break;
    case CLI_CMD_ID_GFP_CMF_AUTO_SF_GEN:
        cli_cmd_gfp_cmf_auto_sf_gen(req);
        break;
    case CLI_CMD_ID_GFP_CMF_FORCED_GEN:
        cli_cmd_gfp_cmf_forced_gen(req);
        break;
    case CLI_CMD_ID_GFP_CMF_ACC_UPI:
        cli_cmd_gfp_cmf_acc_upi(req);
        break;
    case CLI_CMD_ID_GFP_CDF_FCS_INSERT:
        cli_cmd_gfp_cdf_fcs_insert(req);
        break;
    case CLI_CMD_ID_GFP_CDF_UPI:
        cli_cmd_gfp_cdf_upi(req);
        break;
    case CLI_CMD_ID_GFP_CDF_ACC_UPI:
        cli_cmd_gfp_cdf_acc_upi(req);
        break;
    case CLI_CMD_ID_GFP_8B10B_CONTROL_CODE:
        cli_cmd_gfp_8b10b_control_code(req);
        break;
    case CLI_CMD_ID_GFP_DEFECTS:
        cli_cmd_gfp_defects(req);
        break;
    case CLI_CMD_ID_GFP_PERF:
        cli_cmd_gfp_perf(req);
        break;
#endif /* VTSS_FEATURE_GFP */
#if defined(VTSS_FEATURE_UPI)
    case CLI_CMD_ID_UPI_CONFIG:
        cli_cmd_upi_config(req);
        break;
    case CLI_CMD_ID_UPI_STATUS:
        cli_cmd_upi_status(req);
        break;
    case CLI_CMD_ID_UPI_TEST_CONFIG:
        cli_cmd_upi_test_config(req);
        break;
    case CLI_CMD_ID_UPI_TEST_STATUS:
        cli_cmd_upi_test_status(req);
        break;
    case CLI_CMD_ID_UPI_TXEQ_MODE:
        cli_cmd_upi_txeq_config(req);
        break;
    case CLI_CMD_ID_UPI_TXEQ_COEF:
        cli_cmd_upi_txeq_coef_config(req);
        break;
    case CLI_CMD_ID_UPI_TXEQ_AMP:
        cli_cmd_upi_txeq_power(req);
        break;
    case CLI_CMD_ID_UPI_TXEQ_SLEW:
        cli_cmd_upi_txeq_slew(req);
        break;
    case CLI_CMD_ID_UPI_RXEQ_MODE: 
        cli_cmd_upi_rxeq_mode(req);
        break;
#endif /* VTSS_FEATURE_UPI */
#if defined(VTSS_FEATURE_XAUI)
    case CLI_CMD_ID_XAUI_CONFIG:
        cli_cmd_xaui_config(req);
        break;
    case CLI_CMD_ID_XAUI_STATUS:
        cli_cmd_xaui_status(req);
        break;
        
    case CLI_CMD_ID_XAUI_STATS:
        cli_cmd_xaui_statistics(req);
        break;
        
#endif /* VTSS_FEATURE_XAUI */
#if defined(VTSS_FEATURE_XFI)
    case CLI_CMD_ID_XFI_CONFIG:
        cli_cmd_xfi_config(req);
        break;
    case CLI_CMD_ID_XFI_STATUS:
        cli_cmd_xfi_status(req);
        break;
    case CLI_CMD_ID_XFI_TEST_CONFIG:
        cli_cmd_xfi_test_config(req);
        break;
    case CLI_CMD_ID_XFI_TEST_STATUS:
        cli_cmd_xfi_test_status(req);
        break;
    case CLI_CMD_ID_XFI_REC_CLK_STATUS:
        cli_cmd_xfi_rec_clk_status(req);
        break;
    case CLI_CMD_ID_XFI_TXEQ_MODE:
        cli_cmd_xfi_txeq_config(req);
        break;
    case CLI_CMD_ID_XFI_TXMISC_PARM:
        cli_cmd_xfi_txmisc_parm_conf(req);
        break;
    case CLI_CMD_ID_XFI_RXEQ_MODE:
        cli_cmd_xfi_rxeq_mode(req);
        break;
    case CLI_CMD_ID_XFI_TXEQ_COEFUPD:
        cli_cmd_xfi_txeq_coef_update(req);
        break;
    case CLI_CMD_ID_XFI_802a_TXEQ_COEUPD:
        cli_cmd_xfi_802a_txeq_cof_upd(req);
        break;
    case CLI_CMD_ID_XFI_802a_TXEQ_FSM:
        cli_cmd_xfi_txeq_802a_fsm(req);
        break;
#endif /* VTSS_FEATURE_XFI */
#if defined(VTSS_FEATURE_MAC10G)
    case CLI_CMD_ID_MAC10G_LB_CONFIG:
        cli_cmd_mac10g_lb_config(req);
        break;
    case CLI_CMD_ID_MAC10G_TX_FLOW_CTRL_CONFIG:
        cli_cmd_mac10g_tx_flow_ctrl_config(req);
        break;
    case CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_DROP_CONFIG:
        cli_cmd_mac10g_rx_flow_ctrl_drop_config(req);
        break;
    case CLI_CMD_ID_MAC10G_RX_FLOW_CTRL_ACTION_CONFIG:
        cli_cmd_mac10g_rx_flow_ctrl_action_config(req);
        break;
    case CLI_CMD_ID_MAC10G_FRAME_LEN_CONFIG:
        cli_cmd_mac10g_frame_len_config(req);
        break;
#ifdef VTSS_ARCH_DAYTONA
    case CLI_CMD_ID_MAC10G_PREAMBLE_CHECK_CONFIG:
        cli_cmd_mac10g_preamble_check_config(req);
        break;
    case CLI_CMD_ID_MAC10G_LF_CONFIG:
        cli_cmd_mac10g_force_local_fault_config(req);
        break;
    case CLI_CMD_ID_MAC10G_RF_CONFIG:
        cli_cmd_mac10g_force_remote_fault_config(req);
        break;
#endif /* VTSS_ARCH_DAYTONA */
    case CLI_CMD_ID_MAC10G_STATUS:
        cli_cmd_mac_status(req);
        break;
    case CLI_CMD_ID_MAC10G_RMON_STATS:
        cli_cmd_mac_stats(req);
        break;
#endif /* VTSS_FEATURE_MAC10G */
#if defined (VTSS_FEATURE_PCS_10GBASE_R) 
    case CLI_CMD_ID_PCS_10GBASE_R_LB_CONFIG:
        cli_cmd_pcs_10gbase_r_lb_config(req);
        break;
    case CLI_CMD_ID_PCS_10GBASE_R_STATUS:
        cli_cmd_pcs_10gbase_r_status(req);
        break;
    case CLI_CMD_ID_PCS_10GBASE_R_STATS:
        cli_cmd_pcs_10gbase_r_stats(req);
        break;
#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#if defined (VTSS_FEATURE_RAB)
    case CLI_CMD_ID_RAB_RX_THRESH_CONFIG:
        cli_cmd_rab_rx_thresh_config(req);
        break;
    case CLI_CMD_ID_RAB_TX_THRESH_CONFIG:
        cli_cmd_rab_tx_thresh_config(req);
        break;
    case CLI_CMD_ID_RAB_STATS:
        cli_cmd_rab_stats(req);
        break;
    case CLI_CMD_ID_RAB_CUT_THRU_RX:
        cli_cmd_rab_rx_cut_thru(req);
        break;
    case CLI_CMD_ID_RAB_CUT_THRU_TX:
        cli_cmd_rab_tx_cut_thru(req);
        break;
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_I2C)
     case CLI_CMD_ID_I2C_RX:
          cli_cmd_i2c_rx(req);
          break;
     case CLI_CMD_ID_I2C_TX:
          cli_cmd_i2c_tx(req);
          break;
     case CLI_CMD_ID_I2C_INIT:
          cli_cmd_i2c_init(req);
          break;
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_FEATURE_INTERRUPTS)
    case CLI_CMD_ID_DEBUG_INTERRUPT:
        cli_cmd_debug_interrupt(req);
        break;
#endif /* VTSS_FEATURE_INTERRUPTS */
#ifdef VTSS_ARCH_DAYTONA
     case CLI_CMD_ID_DEBUG_EVENT_CONFIG:
          cli_cmd_debug_event_config(req);
          break;
     case CLI_CMD_ID_DEBUG_EVENT_SCREEN:
          cli_cmd_debug_event_screen(req, FALSE);
          break;
     case CLI_CMD_ID_DEBUG_EVENT_CLEAR:
          cli_cmd_debug_event_screen(req, TRUE);
          break;
     case CLI_CMD_ID_DEBUG_INTERRUPT_WITHOUT_MASK:
          cli_cmd_interrupt_status(req);
          break;
     case CLI_CMD_ID_PMTICK_CONFIG:
          cli_cmd_pmtick_config(req);
          break;
     case CLI_CMD_ID_CHANNEL_MODE_CONFIG:
          cli_cmd_channel_mode_config(req);
          break;

#endif /* VTSS_ARCH_DAYTONA */
#if defined (DAYTONA_EVAL_INIT)
    case CLI_CMD_ID_DEBUG_LBFPGA_READ:
        cli_cmd_debug_lbfpga(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_LBFPGA_WRITE:
        cli_cmd_debug_lbfpga(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_PCF8575_READ:
        cli_cmd_debug_pcf8575(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_PCF8575_WRITE:
        cli_cmd_debug_pcf8575(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_VSC3172_READ:
        cli_cmd_debug_vsc3172(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_VSC3172_WRITE:
        cli_cmd_debug_vsc3172(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_SI5338_READ:
        cli_cmd_debug_si5338(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_SI5338_WRITE:
        cli_cmd_debug_si5338(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_SI5326_READ:
        cli_cmd_debug_si5326(req, 0);
        break;
    case CLI_CMD_ID_DEBUG_SI5326_WRITE:
        cli_cmd_debug_si5326(req, 1);
        break;
    case CLI_CMD_ID_DEBUG_CLK_RESYNC:
        cli_cmd_debug_clk_sync(req);
        break;
#endif /* DAYTONA_EVAL_INIT */
#if defined(VTSS_FEATURE_OHA)
    case CLI_CMD_ID_OHA_CONF:
        cli_cmd_oha_conf(req);
        break;
#endif
#if defined(NPI_INTERFACE)
    case CLI_CMD_ID_NPI_PORT:
    {
        vtss_npi_conf_t conf;
        if (vtss_npi_conf_get(NULL, &conf) == VTSS_RC_OK) {
            if (req->set) {
                vtss_packet_rx_conf_t rx_conf;
                // Get Rx packet configuration */
                if (vtss_packet_rx_conf_get(0, &rx_conf) == VTSS_RC_OK) {
                    int i;
                    /* NPI redirect setup (all queues) */
                    for (i = VTSS_PACKET_RX_QUEUE_START; i < VTSS_PACKET_RX_QUEUE_END; i++) {
                        rx_conf.queue[i].npi.enable = !req->disable;
                    }
                    if (vtss_packet_rx_conf_set(0, &rx_conf) != VTSS_RC_OK) {
                        T_E("vtss_packet_rx_conf_set failed");
                    }
                } else {
                    T_E("vtss_packet_rx_conf_get failed");
                }
                if (req->disable) {
                    printf("Disabled NPI.\n");
                    conf.enable = FALSE;
                } else {
                    conf.enable = TRUE;
                    conf.port_no = uport2iport(req->port);
                    printf("Enable, port %d\n", req->port);
                }
                if (vtss_npi_conf_set(NULL, &conf) != VTSS_RC_OK) {
                    T_E("Error: vtss_npi_conf_set fails");
                }
            } else {
                printf("NPI mode: %sabled\n", conf.enable ? "en" : "dis");
                if (conf.enable) {
                    printf("NPI port: %d\n", iport2uport(conf.port_no));
                }
            }
        } else {
            T_E("Error: vtss_npi_conf_get fails");
        }
    }
    break;
    case CLI_CMD_ID_TRANSMIT_NPI:
        cli_cmd_transmit_npi(req);
        break;
#endif
#if defined(VTSS_FEATURE_PACKET)
    case CLI_CMD_ID_TRANSMIT_PORT:
        cli_cmd_transmit_frame(req, FALSE);
        break;
    case CLI_CMD_ID_TRANSMIT_VLAN:
        cli_cmd_transmit_frame(req, TRUE);
        break;
#endif
    default:
        printf("command id: %d not implemented\n", id);
        break;
    }
}

static char *cli_lower_word(char *in, char *out)
{
    int i, len;

    len = strlen(in);
    for (i = 0; i <= len; i++)
        out[i] = tolower(in[i]);
    return out;
}

/* Build array of command/syntax words */
static void cli_build_words(char *str, int *count, char **words, BOOL lower)
{
    int  i, j, len;
    char *p;
    
    len = strlen(str);
    j = 0;
    *count = 0;
    for (i = 0; i < len; i++) {
        p = &str[i];
        if (isspace(*p)) {
            j = 0;
            *p = '\0';
        } else {
            if (j == 0) {
                words[*count] = p;
                (*count)++;
            }
            if (lower)
                *p = tolower(*p);
            j++;
        }
    }
}

/* Parse command */
static void cli_parse_command(void)
{
    char       *cmd, *stx, *cmd2;
    char       cmd_buf[MAX_CMD_LEN], stx_buf[MAX_CMD_LEN], *cmd_words[64], *stx_words[64];
    char       cmd1_buf[MAX_WORD_LEN], cmd2_buf[MAX_WORD_LEN];
    int        i, i_cmd, i_stx, i_parm = 0, cmd_count, stx_count, max, len, j, error, idx;
    int        match_cmds[sizeof(cli_cmd_table)/sizeof(cli_cmd_t)], match_count = 0;
    cli_cmd_t  *cli_cmd;
    BOOL       match, help = 0;
    cli_req_t  *req;
    cli_parm_t *parm;

    /* Read command and skip leading spaces */
    cmd = cmd_get();

    /* Remove CR */
    i = strlen(cmd);
    if (i)
        cmd[i-1] = '\0';

    /* Build array of command words */
    strcpy(cmd_buf, cmd);
    cli_build_words(cmd_buf, &cmd_count, cmd_words, 0);

    /* Remove trailing 'help' or '?' command */
    if (cmd_count > 1) {
        cmd = cli_lower_word(cmd_words[cmd_count-1], cmd1_buf);
        if (strcmp(cmd, "?") == 0 || strcmp(cmd, "help") == 0) {
            cmd_count--;
            help = 1;
        }
    }
    
    /* Compare entered command with each entry in CLI command table */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        cli_cmd = &cli_cmd_table[i];
        
        if (strlen(cli_cmd->syntax) > MAX_CMD_LEN) {
            /* Command too long */
            continue;
        }

#if (VTSS_APPL_INST_CNT > 1)
        /* Only use register read/write command from the selected instance */
        if (cli_board->b2) {
            if (cli_cmd->id == CLI_CMD_ID_DEBUG_REG_READ_L28 ||
                cli_cmd->id == CLI_CMD_ID_DEBUG_REG_WRITE_L28)
                continue;
        } else {
            if (cli_cmd->id == CLI_CMD_ID_DEBUG_REG_READ ||
                cli_cmd->id == CLI_CMD_ID_DEBUG_REG_WRITE)
                continue;
        }
#endif /* VTSS_APPL_INST_CNT > 1 */

        /* Build array of syntax words */
        strcpy(stx_buf, cli_cmd->syntax);
        cli_build_words(stx_buf, &stx_count, stx_words, 1);

        match = 1; 
        for (i_cmd = 0, i_stx = 0; i_stx < stx_count; i_cmd++, i_stx++) {
            stx = stx_words[i_stx];
            if (*stx == '<' || *stx == '[') {
                /* Parameters start here */
                i_parm = i_stx;
                break;
            }

            if (i_cmd >= cmd_count)
                continue;
            
            cmd = cli_lower_word(cmd_words[i_cmd], cmd1_buf);
            if (strstr(stx, cmd) != stx) {
                /* Command word mismatch */
                match = 0;
                break;
            }
        }
        
        if (match) {
            match_cmds[match_count++] = i;
        }
    }
    
    if (match_count == 0) {
        /* No matching commands */
        printf("Invalid command\n");
    } else if (match_count == 1) {
        /* One matching command */
        cli_cmd = &cli_cmd_table[match_cmds[0]];
        
        /* Rebuild array of syntax words */
        strcpy(stx_buf, cli_cmd->syntax);
        cli_build_words(stx_buf, &stx_count, stx_words, 1);

        if (help) {
            uchar done[sizeof(cli_parm_table)/sizeof(*parm)];
            
            memset(done, 0, sizeof(done));
            cli_header_nl("Description", 0, 0);
            printf("%s.\n\n", cli_cmd->descr);
            cli_header_nl("Syntax", 0, 0);
            printf("%s\n\n", cli_cmd->syntax);
            for (max = 0, i = 0; i_parm && i < 2; i++) {
                for (i_stx = i_parm; i_stx < stx_count; i_stx++) {
                    if ((parm = cli_parm_lookup(stx_words[i_stx], cli_cmd->id, &idx)) == NULL)
                        continue;
                    len = strlen(parm->txt);
                    if (i == 0) {
                        if (!(parm->flags & CLI_PARM_FLAG_NO_TXT)) {
                            if (len > max)
                                max = len;
                        }
                    } else if (!done[idx]) {
                        done[idx] = 1;
                        if (i_stx == i_parm)
                            cli_header_nl("Parameters", 0, 0);
                        if (!(parm->flags & CLI_PARM_FLAG_NO_TXT)) {
                            printf("%s", parm->txt);
                            for (j = len; j < max; j++)
                                printf(" ");
                            printf(": ");
                        }
                        printf("%s\n", parm->help);
                    }
                }
            }
        } else {
            enum {
                CLI_STATE_IDLE,
                CLI_STATE_PARSING,
                CLI_STATE_DONE,
                CLI_STATE_ERROR
            } state;
            BOOL end = 0, separator, skip_parm;
            
            /* Create default parameters */
            req = &cli_req;
            cli_req_default_set(req);
            
            /* Parse arguments */
            state = CLI_STATE_IDLE;
            for (i_cmd = i_parm, i_stx = i_parm; i_parm && i_stx < stx_count; i_stx++) {
                stx = stx_words[i_stx];

                separator = (strcmp(stx, "|") == 0);
                skip_parm = 0;
                switch (state) {
                case CLI_STATE_IDLE:
                    if (stx[0] == '(' || stx[1] == '(') {
                        i = i_cmd;
                        state = CLI_STATE_PARSING;
                    }
                    break;
                case CLI_STATE_PARSING:
                    break;
                case CLI_STATE_ERROR:
                    if (end && separator) {
                        /* Parse next group */
                        i_cmd = i;
                        state = CLI_STATE_PARSING;
                    } else if (strstr(stx, ")]") != NULL) {
                        i_cmd = i;
                        state = CLI_STATE_IDLE;
                    }
                    skip_parm = 1;
                    break;
                case CLI_STATE_DONE:
                    if (end && !separator)
                        state = CLI_STATE_IDLE;
                    else
                        skip_parm = 1;
                    break;
                default:
                    printf("Illegal state: %d\n", state);
                    return;
                }
                end = (strstr(stx, ")") != NULL);
                
#if 0
                printf("stx: %s, cmd: %s, state: %s->%s\n",
                        stx, i_cmd < cmd_count ? cmd_words[i_cmd] : NULL,
                        state == CLI_STATE_IDLE ? "IDLE" :
                        state == CLI_STATE_PARSING ? "PARSING" :
                        state == CLI_STATE_ERROR ? "ERROR" : "DONE");
#endif                
                /* Skip if separator or not in parsing state */
                if (separator || skip_parm)
                    continue;

                /* Lookup parameter */
                if ((parm = cli_parm_lookup(stx, cli_cmd->id, &idx)) == NULL) {
                    printf("Unknown parameter: %s\n", stx);
                    return;
                } 
                
                if (i_cmd >= cmd_count) {
                    /* No more command words */
                    cmd = NULL;
                    error = 1;
                } else {
                    /* Parse command parameter */
                    do {
                        cmd = cli_lower_word(cmd_words[i_cmd], cmd1_buf);
                        cmd2 = ((i_cmd + 1) < cmd_count ? 
                                cli_lower_word(cmd_words[i_cmd + 1], cmd2_buf) : NULL);
                        error = cli_parse_parm(parm->type, cli_cmd, cmd, cmd2, 
                                               stx, cmd_words[i_cmd], req); 
#if 0
                        printf("stx: %s, cmd: %s, error: %d\n", stx, cmd, error);
#endif
                        if (error)
                            break;
                        if (parm->flags & CLI_PARM_FLAG_SET)
                            req->set = 1;
                        i_cmd += req->parm_parsed;
                    } while (i_cmd < cmd_count && (parm->flags & CLI_PARM_FLAG_DUAL));
                }
                
                /* No error or error in optional parameter */
                if (!error ||
                    (stx[0] == '[' && (stx[1] != '(' || stx[2] == '['))) {
                    if (state == CLI_STATE_PARSING && end)
                        state = CLI_STATE_DONE;
                    continue;
                }

                /* Error in mandatory parameter of group */
                if (state == CLI_STATE_PARSING) {
                    state = CLI_STATE_ERROR;
                    continue;
                }

                /* Error in mandatory parameter */
                if (cmd == NULL)
                    printf("Missing %s parameter\n\n", parm->txt);
                else
                    printf("Invalid %s parameter: %s\n\n", parm->txt, cmd);
                printf("Syntax:\n%s\n", cli_cmd->syntax);
                return;
            } /* for loop */
            if (i_parm) { 
                if (i_cmd < cmd_count) {
                    printf("Invalid parameter: %s\n\n", cmd_words[i_cmd]);
                    printf("Syntax:\n%s\n", cli_cmd->syntax);
                    return;
                }
                if (state == CLI_STATE_ERROR) {
                    printf("Invalid parameter\n\n");
                    printf("Syntax:\n%s\n", cli_cmd->syntax);
                    return;
                }
            } /* Parameter handling */

            /* Handle CLI command */
            cli_command(cli_cmd->id, req);
        }
    } else {
        printf("Available Commands:\n\n");
        for (i = 0; i < match_count ; i++)
            printf("%s\n", cli_cmd_table[match_cmds[i]].syntax);
    }
} /* cli_parse_command */

void vtss_appl_cli_task(void)
{
    do {
        if (cmd_ready()) {
            cli_parse_command();
            prompt();
        }
    } while (suspended);
}
