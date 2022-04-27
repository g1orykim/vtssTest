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
#include "vtss_appl.h"

#if defined(BOARD_B2_EVAL)
#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_CIL
#include "../base/b2/vtss_b2_reg.h"
#include "../base/ail/vtss_state.h"
#include <string.h>
#include <sys/types.h>
#undef __USE_EXTERN_INLINES /* Avoid warning */
#include <sys/socket.h>
#include <arpa/inet.h>

static int fd; /* Socket descriptor */

/* Various options */
static struct {
    BOOL if_100fx; /* 100FX used for SFP ports */
    BOOL if_mix;   /* SERDES/100FX used for SFP ports */
} b2_options;

#if defined(VTSS_FEATURE_PORT_CONTROL)
/* Board port map */
static vtss_port_map_t board_port_map[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_PORT_CONTROL */
/* Execute Rabbit command */
static vtss_rc vtss_rabbit_cmd(const char *cmd, u32 *value)
{
    char buf[100];
    int  n;
    /* Send command terminated by newline */
    strcpy(buf, cmd);
    n = strlen(buf);
    buf[n] = '\n';
    if (send(fd, buf, n + 1, 0) < 0) {
        T_E("send cmd failed: %s", cmd);
        return VTSS_RC_ERROR;
    }

    T_N("send: %s", cmd);
    /* Receive command reply */
    if ((n = recv(fd, buf, sizeof(buf), 0)) < 0) {
        T_E("recv cmd failed: %s", cmd);
        return VTSS_RC_ERROR;
    }
    buf[n - 1] = '\0';
    T_N("recv: %s", buf);
    if (value != NULL && n == sizeof("[0x12345678]") && strstr(buf, "[0x") == buf) {
        *value = strtoul(buf + 1, NULL, 0);
        T_N("value: 0x%08x", *value);
    }
    return VTSS_RC_OK;
}    

/* Execute Rabbit direct read/write command */
static vtss_rc vtss_rabbit_rd_wr(u32 addr, u32 *value, BOOL read)
{
    char cmd[64], data[9];

   
    if (read)
        data[0] = '\0';
    else
        sprintf(data, "%08x", *value);
    sprintf(cmd, "%s%08x%s%s", 
            read ? "WPRP 0x2018 0x" : "WP 0x2000 0x", 
            0x84000000 + addr, 
            data, 
            read ? "03" : "02");
    return vtss_rabbit_cmd(cmd, value);
}

static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    return vtss_rabbit_rd_wr(addr, value, 1);
}

static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{
    u32 val = value;
    return vtss_rabbit_rd_wr(addr, &val, 0);
}

static vtss_port_interface_t port_interface(vtss_port_no_t port_no)
{
#if defined(VTSS_ARCH_B2)
    vtss_port_map_t *port_map = &board_port_map[port_no];

    return (port_map->chip_port == 26 ? VTSS_PORT_INTERFACE_SPI4 : 
            port_map->chip_port > 23 ? VTSS_PORT_INTERFACE_XAUI : 
            port_map->miim_controller != VTSS_MIIM_CONTROLLER_NONE ? 
            VTSS_PORT_INTERFACE_SGMII :
            (b2_options.if_100fx || (b2_options.if_mix && (iport2uport(port_no) % 2) == 0)) ? 
            VTSS_PORT_INTERFACE_100FX : VTSS_PORT_INTERFACE_SERDES);
#else /* VTSS_CHIP_10G_PHY */
    return ((port_no > 1) ? VTSS_PORT_INTERFACE_NO_CONNECTION : VTSS_PORT_INTERFACE_XAUI);
#endif /* VTSS_CHIP_10G_PHY */
}

#if defined(VTSS_CHIP_10G_PHY) && !defined(VTSS_ARCH_B2) 

/* Only 10G PHY API is in use.                */
/* The MMD functions are defined below        */
/* The access is through B2's MDIO controller  */

#define PHY_CMD_ADDRESS  0
#define PHY_CMD_WRITE    1 
#define PHY_CMD_READ     3 

#define VTSS_BITMASK(x)              ((1U << (x)) - 1)
#define VTSS_EXTRACT_BITFIELD(x,o,w) (((x) >> (o)) & VTSS_BITMASK(w))
#define B2F(tgt, addr, fld, value) \
VTSS_EXTRACT_BITFIELD(value, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld)

static vtss_rc b2_pi_rd_wr(u32 tgt, u32 addr, u32 *value, BOOL read)
{
    u32              address, val, mask, i;
    vtss_reg_read_t  read_func;
    vtss_reg_write_t write_func;
   
    read_func = reg_read;
    write_func = reg_write;

    address = (tgt << 18);
    if (tgt == VTSS_TGT_DEVCPU_ORG || tgt == VTSS_TGT_FAST_REGS) {
        /* Direct operation */
        address += (addr << 1);
        return (read ? read_func(0, address, value) : write_func(0, address, *value));
    }

    /* Indirect operation */
    address += (addr << 2);
    if (read) {
        VTSS_RC(read_func(0, address, &val));
    } else {
        val = *value;
        VTSS_RC(write_func(0, address,     val >> 16));    /* MSB */
        VTSS_RC(write_func(0, address + 2, val & 0xffff)); /* LSB */
    }

    /* Wait for operation to complete */
    mask = ((1 << VTSS_OFF_FAST_REGS_CFG_STATUS_2_WR_IN_PROGRESS) |
            (1 << VTSS_OFF_FAST_REGS_CFG_STATUS_2_RD_IN_PROGRESS));
    for (i = 0; i < 25; i++) {
        VTSS_RC(b2_pi_rd_wr(VTSS_TGT_FAST_REGS, VTSS_ADDR_FAST_REGS_CFG_STATUS_2, &val, 1));
        if ((val & mask) == 0) {
            /* Read/write operation completed */
            if (read) {
                VTSS_RC(b2_pi_rd_wr(VTSS_TGT_FAST_REGS, 
                                    VTSS_ADDR_FAST_REGS_SLOWDATA_MSB, &val, 1));
                VTSS_RC(b2_pi_rd_wr(VTSS_TGT_FAST_REGS, 
                                    VTSS_ADDR_FAST_REGS_SLOWDATA_LSB, value, 1));
                *value += (val << 16);
            }
            return VTSS_RC_OK;
        }
        VTSS_NSLEEP(100);
    }    
    return VTSS_RC_ERROR;
}

static vtss_rc b2_mmd_cmd(u32 cmd, u8 miim_addr, u8 addr, u16 *data)
{
    u32 n, value;
    vtss_rc rc;

    value = (0 << VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_ST_CFG_FIELD) | (0x40 << VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_PRESCALE);
    /* write */
    rc = b2_pi_rd_wr(VTSS_TGT_DEVCPU_GCB, VTSS_ADDX_DEVCPU_GCB_MIIM_CFG(1),&value, 0);

    value = ((unsigned)1 << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_VLD) |
          (miim_addr << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_PHYAD) |
          (addr << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_REGAD) |
          (*data << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_WRDATA) |
          (cmd << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_OPR_FIELD);
    /* write */
    rc = b2_pi_rd_wr(VTSS_TGT_DEVCPU_GCB, VTSS_ADDX_DEVCPU_GCB_MIIM_CMD(1),&value, 0);
   
    /* Wait for access to complete */
   for (n = 0; ; n++) {
       /* read */
       rc = b2_pi_rd_wr(VTSS_TGT_DEVCPU_GCB, VTSS_ADDX_DEVCPU_GCB_MIIM_STATUS(1), &value,1);
       if (B2F(DEVCPU_GCB, MIIM_STATUS, MIIM_STAT_PENDING_RD, value) == 0 &&
           B2F(DEVCPU_GCB, MIIM_STATUS, MIIM_STAT_PENDING_WR, value) == 0)
           break;
       if (n == 1000) {
         goto mmd_error;
       }
   }
    /* Read data */
   if (cmd == PHY_CMD_READ) {
       /* read */
       rc = b2_pi_rd_wr(VTSS_TGT_DEVCPU_GCB, VTSS_ADDX_DEVCPU_GCB_MIIM_DATA(1), &value, 1);
       if (B2F(DEVCPU_GCB, MIIM_DATA, MIIM_DATA_SUCCESS, value)) {
         goto mmd_error;
       }
       *data = B2F(DEVCPU_GCB, MIIM_DATA, MIIM_DATA_RDDATA, value);
   }

    return rc;

 mmd_error:
    T_E("miim failed, cmd: %s, miim_addr: %u, addr: %u", 
        cmd == PHY_CMD_READ ? "PHY_CMD_READ" : cmd == PHY_CMD_WRITE ? "PHY_CMD_WRITE" : 
        cmd == PHY_CMD_ADDRESS ? "PHY_CMD_ADDRESS" : "PHY_CMD_READ_INC", 
        miim_addr, addr);
    return VTSS_RC_ERROR;

}

static vtss_rc mmd_read(const vtss_inst_t     inst,
                        const vtss_port_no_t  port_no,
                        const u8              mmd,
                        u16                   addr,
                        u16                   *const value)
{
    u8 mdio_addr = port_no+24; /* Port map */

    VTSS_RC(b2_mmd_cmd(PHY_CMD_ADDRESS, mdio_addr, mmd, &addr));
    return b2_mmd_cmd(PHY_CMD_READ, mdio_addr, mmd, value);
}

static vtss_rc mmd_write(const vtss_inst_t     inst,
                         const vtss_port_no_t  port_no,
                         const u8              mmd,
                         u16                   addr,
                         u16                   data)
{
    u8 mdio_addr = port_no+24; /* Port map */
    
    VTSS_RC(b2_mmd_cmd(PHY_CMD_ADDRESS, mdio_addr, mmd, &addr));
    return b2_mmd_cmd(PHY_CMD_WRITE, mdio_addr, mmd, &data);
}
#endif /* VTSS_CHIP_10G_PHY */


#if defined(VTSS_ARCH_B2)
static int b2_parse_options(int argc, const char **argv, vtss_init_t *config)
{
    struct sockaddr_in saddr;
    vtss_port_no_t     port_no;
    BOOL               sfp = 0, error, val_error, done=FALSE;
    u32                value = 0, i, port_max = VTSS_PORTS, q_en=0;
    char               *s, *end;
    const char         *opt, *p;
    vtss_spi4_conf_t   *spi4 = config->spi4;
    vtss_xaui_conf_t   *xaui = config->xaui;
    vtss_init_conf_t   *conf = config->init_conf;
    vtss_port_map_t    *map;
    int                a, cnt=0;
  
    if ((argc > 1 && *argv[1] == '?') || 
        argc < 2 || inet_aton(argv[1], &saddr.sin_addr) == 0) {
        printf("Usage: %s <rabbit_ip> [<parm>[:<value>]]\n\n", argv[0]);
        printf("Example (host mode 5): %s 1.2.3.4 h:5\n\n", argv[0]);
        printf("Mandatory parameter:\n");
        printf("--------------------\n");
        printf("rabbit_ip: Rabbit board IP address\n\n");
        printf("Optional parameters:\n");
        printf("--------------------\n");
        printf("Each optional parameter may consist of two elements:\n");
        printf("<parm> : Keyword identifying the parameter. This may be abbreviated.\n");
        printf("<value>: Value required for some <parm> values.\n\n");
        printf("host_mode    : Host mode, value: 0,1,3,4,5,6,8,9 (default: 4)\n");
        printf("               If host_mode > 4, port 1-2 are 10G ports\n");
        printf("               If host_mode > 5, port 3-24 are unused\n");
        printf("serdes       : SFP module with SERDES for port 17-20 (port 21-24 unused)\n");
        printf("100fx        : SFP module with 100FX for port 17-20 (port 21-24 unused)\n");
        printf("fiber_mix    : SFP, port 17,19: SERDES, port 18,20: 100FX (port 21-24 unused)\n");
        printf("port_max     : Maximum number of ports, value: 1-24 (default: %u)\n", VTSS_PORTS);
        printf("stag_etype   : S-Tag type, value: Ethernet Type (default: 0x88a8)\n");
        printf("rx_share     : Rx queue system share, value in percent (default: auto)\n");
        printf("rx_mode      : Rx mode, value: port_fifo/shared_drop (default: shared_lossless)\n");
        printf("tx_mode      : Tx mode port_fifo enable (default: shared_lossless)\n");
        printf("mtu          : Queue system MTU, value: 1518-10240 (default: 1518)\n");
        printf("q_profile    : QoS profile 1-7. 7 predefined Q profiles for easier setup. (Default disabled)\n");  
        printf("q6_resv      : Q6 reservation enable (default: disabled)\n");
        printf("q7_resv      : Q7 reservation enable (default: disabled)\n");
        printf("qss_enable_qw: 6-bit vector to enable Q5-Q0 weighted scheduling. (default:0)\n");
        printf("qss_qsp_bwf  : Strict Prio Q BW factor ratio, between 0-100 (default:100)\n");
        printf("sch_max_rate : Scheduler maximum rate, value in kbps\n");
        printf("spi4_flow    : SPI4 flow control disabled (default: enabled)\n");
        printf("spi4_3lvl    : SPI4 three level flow ctrl. enabled (default: disabled)\n");
        printf("spi4_rate    : SPI4 shaper rate, value in kbps (default: disabled)\n");
        printf("spi4_ib_fcs  : SPI4 FCS action, value: disabled/add (default: check)\n");
        printf("spi4_ib_clock: SPI4 inbound maximum clock, value: 290/360/450 (default: 500)\n");
        printf("spi4_ib_swap : SPI4 inbound swap enable (default: disabled)\n");
        printf("spi4_ib_inv  : SPI4 inbound invert enable (default: disabled)\n");
        printf("spi4_ib_shift: SPI4 inbound clock shift disable (default: enabled)\n");
        printf("spi4_ob_frame: SPI4 frame interleave mode (default: burst interleave)\n");
        printf("spi4_ob_hih  : SPI4 HIH enable (default: disabled)\n");
        printf("spi4_ob_fcs  : SPI4 FCS strip enable (default: disabled)\n");
        printf("spi4_ob_len  : SPI4 HIH length update enable (default: disabled)\n");
        printf("spi4_ob_clock: SPI4 outbound clock, value: 250/312/375/437 (default: 500)\n");
        printf("spi4_ob_phase: SPI4 outbound clock phase, value: 90/180/270 (default: 0)\n");
        printf("spi4_ob_swap : SPI4 outbound swap enable (default: disabled)\n");
        printf("spi4_ob_inv  : SPI4 outbound invert enable (default: disabled)\n");
        printf("spi4_ob_shift: SPI4 outbound clock shift enable (default: disabled)\n");
        printf("spi4_ob_burst: SPI4 burst size, value: 64/96/160/192/224/256 (default: 128)\n");
        printf("spi4_ob_max_1: SPI4 MaxBurst1, value: 0-255 (default: 64)\n");
        printf("spi4_ob_max_2: SPI4 MaxBurst2, value: 0-255 (default: 32)\n");
        printf("spi4_ob_up   : SPI4 link up limit, value: 1-16 (default: 2)\n");
        printf("spi4_ob_down : SPI4 link down limit, value: 1-16 (default: 2)\n");
        printf("spi4_ob_train: SPI4 training mode, value: disabled/forced/normal (default: auto)\n");
        printf("spi4_ob_alpha: SPI4 ALPHA, value: 1-255 (default: 1)\n");
        printf("spi4_ob_data : SPI4 DATA_MAX_T, value: 1-131000 (default: 10240)\n");
        printf("xaui_flow    : XAUI channel flow control enable (default: disabled)\n");
        printf("xaui_ibfc    : XAUI inbound flow control enable (default: disabled)\n");
        printf("xaui_3lvl    : XAUI three level flow ctrl. enabled (default: disabled)\n");
        printf("xaui_rate    : XAUI shaper rate, value in kbps (default: disabled)\n");
        printf("xaui_pause   : XAUI pause frame obey enable (default: disabled)\n");
        printf("xaui_map_dmac: XAUI DMAC logical map enable (default: disabled)\n");
        printf("xaui_hih_pre : XAUI HIH before SFD enable (default: disabled)\n");
        printf("xaui_hih_chk : XAUI HIH checksum enable (default: disabled)\n");
        printf("xaui_status  : XAUI status channel, value: 0/1 (default: both)\n");
        printf("xaui_ib_shift: XAUI inbound clock shift enable (default: disabled)\n");
        printf("xaui_ob_shift: XAUI outbound clock shift enable (default: disabled)\n");
        return 0;
    }

    /* Connect to board */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(26);
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        T_E("socket create failed");
        return 0;
    }
    if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        T_E("connect to rabbit failed");
        return 0;
    }
    
    memset(&b2_options, 0, sizeof(b2_options));

    /*  The inbound clock is shifted because the ports are in loopback  */
    spi4->ib.clock_shift = 1;
    
    /* Parse options */

    for (i = 2 ; i < argc; i++) {
        p = argv[i];
        opt = p;
        s = strstr(p, ":");
        val_error = 1;
        if (s != NULL) {
            s++;
            value = strtoul(s, &end, 0);
            val_error = (end == s);
        }
        error = 0;

        if (p[0] == 'h' && s != NULL) { /* host_mode */
            if (val_error || value > 11 || value == 2 || value == 7) {
                printf("Illegal host_mode: %s\n", s);
                return 0;
            }
            conf->host_mode = value;
        } else if (strstr(p, "se") == p) /* serdes */
            sfp = 1;
        else if (p[0] == '1') { /* 100fx */
            sfp = 1;
            b2_options.if_100fx = 1;
        } else if (p[0] == 'f') { /* fiber_mix */
            sfp = 1;
            b2_options.if_mix = 1;
        } else if (p[0] == 'p' && val_error == 0 && value <= VTSS_PORTS) /* port_max */
            port_max = value;
        else if (strstr(p, "rx_") == p) {
            p += 3;
            if (p[0] == 's' && val_error == 0 && value <= 100) /* rx_share */
                conf->qs.rx_share = value;
            else if (p[0] == 'm' && s != NULL) { /* rx_mode */
                p += 5;
                if (strstr(p, "port_fifo") == p) {
                    conf->qs.rx_mode = VTSS_RX_MODE_PORT_FIFO;
                } else if (strstr(p, "shared_drop") == p) {
                    conf->qs.rx_mode = VTSS_RX_MODE_SHARED_DROP;
                } else {
                    conf->qs.rx_mode = VTSS_RX_MODE_SHARED_LOSSLESS;
                }
            }
            else
                error = 1;
        } else if (strstr(p, "tx_") == p) { /* tx_mode */
            p += 3;
            if (p[0] == 'm' && s != NULL) 
                conf->qs.tx_mode = (s[0] == 'p' ? VTSS_TX_MODE_PORT_FIFO :
                                     VTSS_TX_MODE_SHARED_LOSSLESS);
            else
                error = 1;
        } else if (strstr(p, "q_profile") == p) {
            if (conf->qs.rx_mode == VTSS_RX_MODE_SHARED_LOSSLESS) {
                printf("Error. Rx mode VTSS_RX_MODE_SHARED_LOSSLESS does not support QoS\n");
                return 0;
            }
                
            if (value == 1) {
                q_en = 0x94; /* q2,q4,q7 */
            } else if (value == 2) {
                q_en = 0xb4; /* q2,q4,q5,q7 */
            } else if (value == 3) {
                q_en = 0xd4; /* q2,q4,q6,q7 */
            } else if (value == 4) {
                q_en = 0xf4; /* q2,q4,q5,q6,q7 */
            } else if (value == 5) {
                q_en = 0xfc; /* q2,q3,q4,q5,q6,q7 */
            } else if (value == 6) {
                q_en = 0xfd; /* q0,q2,q4,q5,q6,q7 */
            } else if (value == 7) {
                q_en = 0xff; /* q0,q1,q2,q4,q5,q6,q7 */
            } else {
                printf("QoS profile %u not supported\n",value);
            }

            for(a=0;a<6;a++)
                conf->qs.qw[a] = (q_en & (1<<a)) ? VTSS_QUEUE_WEIGHT_MAX_ENABLE : VTSS_QUEUE_WEIGHT_DISABLE;
            conf->qs.qs_6 = (q_en & (1<<6)) ? VTSS_QUEUE_STRICT_RESERVE : VTSS_QUEUE_STRICT_DISABLE;
            conf->qs.qs_7 = (q_en & (1<<7)) ? VTSS_QUEUE_STRICT_RESERVE : VTSS_QUEUE_STRICT_DISABLE;            

            printf("Queue profile %u: Strict queues (buffer is reserved):",value);
            if (conf->qs.qs_7 == VTSS_QUEUE_STRICT_RESERVE)
                printf("q7 ");
            if (conf->qs.qs_6 == VTSS_QUEUE_STRICT_RESERVE)
                printf("q6 ");
            printf("   Weighted queues:");
            for(a=5; a>=0; a--)
                if (conf->qs.qw[a] == VTSS_QUEUE_WEIGHT_MAX_ENABLE)
                    printf("q%d ",a);
            printf("are enabled (Q_MAX is configured).\n");

        } else if (strstr(p, "qss_en") == p) {
            for(a=0;a<6;a++)
                conf->qs.qw[a] = value & (1<<a) ? VTSS_QUEUE_WEIGHT_MAX_ENABLE : VTSS_QUEUE_WEIGHT_DISABLE;
        } else if (strstr(p, "qss_qsp") == p) {
            conf->qs.qss_qsp_bwf_pct = value;
        } else if (strstr(p, "q")) {
            if (p[1] == '6')      /* q6_resv */
                conf->qs.qs_6 = VTSS_QUEUE_STRICT_RESERVE;
            else if (p[1] == '7') /* q7_resv */
                conf->qs.qs_7 = VTSS_QUEUE_STRICT_RESERVE;
            else
                error = 1;
        } else if (p[0] == 'm' && val_error == 0 && 
                   value >= VTSS_MAX_FRAME_LENGTH_STANDARD && 
                   value <= VTSS_MAX_FRAME_LENGTH_MAX) {
            conf->qs.mtu = value; /* mtu */
        } else if (strstr(p, "st") == p && val_error == 0 && value < 0xffff) /* stag_etype */
            
            conf->stag_etype = value;
        else if (strstr(p, "sc") == p && val_error == 0) /* sch_max_rate */
            conf->sch_max_rate = value;
        else if (strstr(p, "spi4_") == p) {
            p += 5;
            if (p[0] == 'f') /* spi4_flow */
                spi4->fc.enable = 0;
            else if (p[0] == '3') /* spi4_3lvl */
                spi4->fc.three_level_enable = 1;
            else if (p[0] == 'r' && val_error == 0) /* spi4_rate */
                spi4->qos.shaper.rate = value;
            else if (strstr(p, "ib_") == p) {
                p += 3;
                if (p[0] == 'f' && s != NULL && (s[0] == 'd' || s[0] == 'a')) /* spi4_ib_fcs */
                    spi4->ib.fcs = (s[0] == 'd' ? VTSS_SPI4_FCS_DISABLED : VTSS_SPI4_FCS_ADD);
                else if (p[0] == 'c' && val_error == 0) {/* spi4_ib_clock */
                  // Check for correct ib clock value
                  if (value == 290 || value == 360 || value == 450 || value == 500 ) {
                    spi4->ib.clock = (value == 290 ? VTSS_SPI4_CLOCK_250_TO_290 :
                                      value == 360 ? VTSS_SPI4_CLOCK_290_TO_360 :
                                      value == 450 ? VTSS_SPI4_CLOCK_360_TO_450 :
                                      VTSS_SPI4_CLOCK_450_TO_500);
                  } else {
                    T_E("Invalid spi4_ib_clock value - defaulting to 500");
                    spi4->ib.clock = VTSS_SPI4_CLOCK_450_TO_500;
                  } 
                }
                else if (p[0] == 's' && p[1] == 'w') /* spi4_ib_swap */
                    spi4->ib.data_swap = 1;
                else if (p[0] == 's' && p[1] == 'h') /* spi4_ib_shift */
                    spi4->ib.clock_shift = 0;
                else if (p[0] == 'i') /* spi4_ib_inv */
                    spi4->ib.data_invert = 1;
                else
                    error = 1;
            } else if (strstr(p, "ob_") == p) {
                p += 3;
                if (p[0] == 'f' && p[1] == 'r') /* spi4_ob_frame */
                    spi4->ob.frame_interleave = 1;
                else if (p[0] == 'h') /* spi4_ob_hih */
                    spi4->ob.hih_enable = 1;
                else if (p[0] == 'f' && p[1] == 'c') /* spi4_ob_fcs */
                    spi4->ob.fcs_strip = 1;
                else if (p[0] == 'l') /* spi4_ob_len */
                    spi4->ob.hih_length_update = 1;
                else if (p[0] == 'c' && val_error == 0) /* spi4_ob_clock */
                    spi4->ob.clock = (value == 250 ? VTSS_SPI4_CLOCK_250_0 :
                                      value == 312 ? VTSS_SPI4_CLOCK_312_5 :
                                      value == 375  ? VTSS_SPI4_CLOCK_375_0 :
                                      value == 437 ? VTSS_SPI4_CLOCK_437_5 :
                                      VTSS_SPI4_CLOCK_500_0);
                else if (p[0] == 'p' && val_error == 0) /* spi4_ob_phase */
                    spi4->ob.clock_phase = (value == 90 ? VTSS_SPI4_CLOCK_PHASE_90 :
                                            value == 180 ? VTSS_SPI4_CLOCK_PHASE_180 :
                                            value == 270 ? VTSS_SPI4_CLOCK_PHASE_270 :
                                            VTSS_SPI4_CLOCK_PHASE_0);
                else if (p[0] == 's' && p[1] == 'w') /* spi4_ob_swap */
                    spi4->ob.data_swap = 1;
                else if (p[0] == 's' && p[1] == 'h') /* spi4_ob_shift */
                    spi4->ob.clock_shift = 1;
                else if (p[0] == 'i') /* spi4_ob_inv */
                    spi4->ob.data_invert = 1;
                else if (p[0] == 'b' && val_error == 0) /* spi4_ob_burst */
                    spi4->ob.burst_size = (value == 64 ? VTSS_SPI4_BURST_64 :
                                           value == 96 ? VTSS_SPI4_BURST_96 :
                                           value == 160 ? VTSS_SPI4_BURST_160 :
                                           value == 192 ? VTSS_SPI4_BURST_192 :
                                           value == 224 ? VTSS_SPI4_BURST_224 :
                                           value == 256 ? VTSS_SPI4_BURST_256 :
                                           VTSS_SPI4_BURST_128);
                else if (strstr(p, "max_") == p && val_error == 0 && value < 256) {
                    if (p[4] == '1') /* spi4_ob_max_1 */
                        spi4->ob.max_burst_1 = value;
                    else if (p[4] == '2') /* spi4_ob_max_2 */
                        spi4->ob.max_burst_2 = value;
                    else
                        error = 1;
                } else if (p[0] == 'u' && val_error == 0 && value != 0 && value < 17) /* spi4_ob_up */
                    spi4->ob.link_up_limit = value;
                else if (p[0] == 'd' && val_error == 0 && value != 0 && value < 17) /* spi4_ob_down */
                    spi4->ob.link_down_limit = value;
                else if (p[0] == 't' && s != NULL) /* spi4_ob_train */
                    spi4->ob.training_mode = (s[0] == 'd' ? VTSS_SPI4_TRAINING_DISABLED :
                                              s[0] == 'f' ? VTSS_SPI4_TRAINING_FORCED :
                                              s[0] == 'n' ? VTSS_SPI4_TRAINING_NORMAL :
                                              VTSS_SPI4_TRAINING_AUTO);
                else if (p[0] == 'a' && val_error == 0 && value < 256) /* spi4_ob_alpha */
                    spi4->ob.alpha = value;
                else if (p[0] == 'd' && val_error == 0 && value <= 131000) /* spi4_ob_data */
                    spi4->ob.data_max_t = value;
                else
                    error = 1;
            } else
                error = 1;
        } else if (strstr(p, "xaui_") == p) {
            p += 5;
            if (p[0] == 'f') /* xaui_flow */
                xaui->fc.channel_enable = 1;
            else if (p[0] == '3') /* xaui_3lvl */
                xaui->fc.three_level_enable = 1;
            else if (p[0] == 'r' && val_error == 0) /* xaui_rate */
                xaui->qos.shaper.rate = value;
            else if (p[0] == 'p') /* xaui_pause */
                xaui->fc.obey_pause = 1;
            else if (strstr(p, "ibfc") == p) {
                xaui->fc.ib.enable = 1;                
                xaui->fc.ib.etype = 0x8880;      /* Vitesses ETYPE for demo purposes */
                xaui->fc.ib.dmac.addr[0] = 0x0;  /* Some random address for demo purposes */
                xaui->fc.ib.dmac.addr[1] = 0x0;
                xaui->fc.ib.dmac.addr[2] = 0x0;
                xaui->fc.ib.dmac.addr[3] = 0x0;
                xaui->fc.ib.dmac.addr[4] = 0x1;
                xaui->fc.ib.dmac.addr[5] = 0x1;
                xaui->fc.ib.pause_value = 0xffff;
            } else if (p[0] == 'm') /* xaui_map_dmac */
                xaui->dmac_map_enable = 1;
            else if (strstr(p, "hih_") == p) {
                p += 4;
                if (p[0] == 'p') /* xaui_hih_pre */
                    xaui->hih.format = VTSS_HIH_PRE_SFD;
                else if (p[0] == 'c') /* xaui_hih_chk */
                    xaui->hih.cksum_enable = 1;
                else
                    error = 1;
            } else if (p[0] == 's' && val_error == 0 && value < 2) /* xaui_status */ 
                xaui->status_select = (value == 0 ? VTSS_XAUI_STATUS_XAUI_0 :
                                       VTSS_XAUI_STATUS_XAUI_1);
            else if (p[0] == 'i') /* xaui_ib_shift */
                xaui->ib.clock_shift = 1;
            else if (p[0] == 'o') /* xaui_ob_shift */
                xaui->ob.clock_shift = 1;
            else
                error = 1;
        } else
            error = 1;
        if (error) {
            printf("Illegal parameter or missing/illegal value: %s\n", opt);
            return 0;
        }
    }
   
    /* Apply options */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &board_port_map[port_no];
        if (port_no < 2 && conf->host_mode > 4 && conf->host_mode < 10) {
            /* Port 1 and 2 are XAUI line ports */
            map->chip_port = (24 + port_no - VTSS_PORT_NO_START);
            map->miim_controller = VTSS_MIIM_CONTROLLER_1;
            map->miim_addr = map->chip_port;
        }

        if (conf->host_mode == VTSS_HOST_MODE_5) {
          // Xaui ports are mapped to 0 and 1. 1G ports are mapped to 2-23.
          // All other ports shall be mapped as unused.
          if (port_no > 23) {
            map->chip_port = CHIP_PORT_UNUSED; // Map as unused.
          }
        }

        if (conf->host_mode > 9) {
            /* Host mode 10/11, one XAUI line port used */
            map->chip_port = (port_no == 0 ? (conf->host_mode == 10 ? 24 : 25) : CHIP_PORT_UNUSED);
            map->miim_controller = (port_no == 0 ? 
                                    VTSS_MIIM_CONTROLLER_1 : VTSS_MIIM_CONTROLLER_NONE);
            map->miim_addr = map->chip_port;
        }
        if (port_no > 1 && conf->host_mode > 5) {
            /* Port 3-24 are unused */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no > 23 && conf->host_mode == 4) {
            /* XAUI ports 24-25 are unused */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no == 25 && 
            (conf->host_mode == VTSS_HOST_MODE_0 || 
             conf->host_mode == VTSS_HOST_MODE_1)) {
          /* Only one XAUI port in host mode 0 and 1 */
          map->chip_port = CHIP_PORT_UNUSED;
          map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no > 15 && sfp) {                
            /* Port 17-20 are SFP and port 21-24 are unused */
            if (port_no > 19)
                map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }

        if (port_no >= port_max) 
            map->chip_port = CHIP_PORT_UNUSED;

#if defined(VTSS_CHIP_SCHAUMBURG_II)
        if (port_no > 11) 
            map->chip_port = CHIP_PORT_UNUSED;
#endif
        
        /* Host ports are the last ports in the portmap */
        if ((map->chip_port == CHIP_PORT_UNUSED || port_no >= 24) && !done) {
            /*  XAUI_0, XAUI_1 or both are hosts */
            if (conf->host_mode < 4) {
                if (conf->host_mode == VTSS_HOST_MODE_0) {
                    map->chip_port = 24;
                    done = TRUE;
                } else if (conf->host_mode == VTSS_HOST_MODE_1) {
                    map->chip_port = 25;
                    done = TRUE;
                } else if (conf->host_mode == 3) {                    
                    map->chip_port = 24+cnt;
                    cnt > 0 ? done = TRUE : cnt++;
                }
                map->miim_controller = VTSS_MIIM_CONTROLLER_1;
                map->miim_addr = map->chip_port;          
            } else {
                /* SPI4 host */
                map->chip_port = 26;
                map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
                done = TRUE;
            }
            map->lport_no = VTSS_LPORT_MAP_NONE;          
        }
     }
    
    printf("Initializing chip, host mode: %u\n", conf->host_mode);
    return 1;
}
#else /* VTSS_CHIP_10G_PHY */
static int b2_parse_options(int argc, const char **argv, vtss_init_t *config)
{
    struct sockaddr_in saddr;
    BOOL               error, val_error;
    u32                value = 0, i;
    char               *s, *end;
    const char         *opt, *p;
  
    if ((argc > 1 && *argv[1] == '?') || 
        argc < 2 || inet_aton(argv[1], &saddr.sin_addr) == 0) {
        printf("Usage: %s <rabbit_ip> [<parm>[:<value>]]\n\n", argv[0]);
        printf("Example (host mode 5): %s 1.2.3.4 h:5\n\n", argv[0]);
        printf("Mandatory parameter:\n");
        printf("--------------------\n");
        printf("rabbit_ip: Rabbit board IP address\n\n");
        printf("Optional parameters:\n");
        printf("--------------------\n");
        printf("Each optional parameter may consist of two elements:\n");
        printf("<parm> : Keyword identifying the parameter. This may be abbreviated.\n");
        printf("<value>: Value required for some <parm> values.\n\n");
        return 0;
    }

    /* Connect to board */
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(26);
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        T_E("socket create failed");
        return 0;
    }
    if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        T_E("connect to rabbit failed");
        return 0;
    }
    
    memset(&b2_options, 0, sizeof(b2_options));

    /* Parse options */

    for (i = 2 ; i < argc; i++) {
        p = argv[i];
        opt = p;
        s = strstr(p, ":");
        val_error = 1;
        if (s != NULL) {
            s++;
            value = strtoul(s, &end, 0);
            val_error = (end == s);
        }
        error = 0;
    }
    return 1;
}
#endif /* VTSS_CHIP_10G_PHY */
/* ================================================================= *
 *  Board init.
 * ================================================================= */

static void b2_board_init(void)
{
    vtss_rabbit_cmd("reset_val_board 1", NULL);
    vtss_rabbit_cmd("reset_val_board 0", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000006403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x800000640000000502", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000004002", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000006002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000003", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000010000004c02", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000004002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000003", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x88000001000000c802", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000004002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000003", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000010000008f02", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000004002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000003", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x880000030000005002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000303", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8800000003", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000004403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000044000001f702", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000004403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000044000001e702", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000004403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000044000001c702", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000002403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000024000008aa02", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000002403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000024000000aa02", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000002403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000024000001aa02", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000002403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x80000024000005aa02", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x800000200000000002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000003803", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x800000380000001f02", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000005403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x800000540000000002", NULL);
    vtss_rabbit_cmd("WPRP 0x2018 0x8000005403", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x800000540000000202", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x84fc00060000000002", NULL);
    vtss_rabbit_cmd("WP 0x2000 0x84fc00060000000002", NULL);
}


#if defined(VTSS_ARCH_B2)
static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_init_t      *board_conf = &board->init;
    vtss_init_conf_t *init_conf = board->init.init_conf;
    vtss_port_no_t   port_no;
    vtss_port_map_t  *map;

    /* Setup default port map */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &board_port_map[port_no];
        map->chip_port = port_no;
        map->lport_no = (port_no < 24 ? VTSS_LPORT_MAP_DEFAULT : VTSS_LPORT_MAP_NONE);
        map->miim_controller = VTSS_MIIM_CONTROLLER_0;
        map->miim_addr = port_no;
    }

    /* Parse options (may change port map) */
    if (b2_parse_options(argc, argv, board_conf) == 0)
        return 1;

    b2_board_init();

    init_conf->reg_read = reg_read;
    init_conf->reg_write = reg_write;

    board->port_map = board_port_map;
    board->port_interface = port_interface;

    /* Update port count */
    board->port_count = VTSS_PORTS;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (board_port_map[port_no].chip_port == CHIP_PORT_UNUSED) {
            board->port_count = port_no;
            break;
        }
    }

    return 0;
}
#else /* VTSS_CHIP_10G_PHY */
static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_init_t      *board_conf = &board->init;
    vtss_init_conf_t *init_conf = board->init.init_conf;

    if (b2_parse_options(argc, argv, board_conf) == 0)
        return 1;

    init_conf->mmd_read = mmd_read;
    init_conf->mmd_write = mmd_write;

    b2_board_init();
    board->port_interface = port_interface;

    /* Update port count */
    board->port_count = VTSS_PORTS;
    return 0;
}
#endif /* VTSS_CHIP_10G_PHY */

void vtss_board_b2_init(vtss_board_t *board)
{
    
#if defined(VTSS_ARCH_B2)    
    board->descr = "Barrington-II";
    board->target = VTSS_TARGET_BARRINGTON_II;
    board->feature.port_control = 1;
    board->b2 = 1;
#else /* VTSS_CHIP_10G_PHY only */
    board->descr = "10G Phy family";
    board->target = VTSS_TARGET_10G_PHY;
    board->b2 = 0;
#endif /* VTSS_CHIP_10G_PHY only */
    board->board_init = board_init;
}

/* Execute Rabbit direct read/write command */
vtss_rc vtss_rabbit_rd_wr_fpga(u32 addr, u32 *value, BOOL read)
{
    char  cmd[64], data[9];
    
    if (read)
        data[0] = '\0';
    else
        sprintf(data, "%08x", *value);
    sprintf(cmd, "%s%08x%s%s", 
            read ? "WPRP 0x2018 0x" : "WP 0x2000 0x", 
            0x80000000 + addr, 
            data, 
            read ? "03" : "02");

    return vtss_rabbit_cmd(cmd, value);
}


#endif /* BOARD_B2_EVAL */
