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
#include "vtss_options.h"
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "vtss_api.h"
#include "vtss_appl.h"
#include "vtss_appl_cli.h"
#include "vtss_version.h"


#if defined(NPI_INTERFACE)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#endif

#if defined(VTSS_SW_OPTION_REMOTE_TS_PHY)
#include <sys/types.h>
#undef __USE_EXTERN_INLINES /* Avoid warning */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>             /* close() */
#include <fcntl.h>
#include <vtss_phy_ts_api.h>
#include "vtss_remote_ts_phy_api.h"

#define MAX_MSG                         5000
#define VTSS_DAYTONA_GENIE_DISCOVERY    "Request ID from switch"
#define VTSS_DAYTONA_GENIE_INBM_CONNECT "connect"
#define VTSS_DAYTONA_GENIE_INBM_DISCONN "disconnect"
#define VTSS_DAYTONA_GENIE_INBM_AUTH    "authenticated"

#define VTSS_DAYTONA_HOST_NAME_MAX      50

#define VTSS_DAYTONA_LOCATE_SERVER_PORT     49152
#define VTSS_DAYTONA_INBM_SERVER_PORT       49153
#define VTSS_DAYTONA_INBM_SERVER_EVT_PORT   49154

typedef struct {
    int  socket_descriptor;
    int  port_no;
}socket_data;

typedef struct {
    socket_data   locate_socket;
    socket_data   inbm_socket;
    socket_data   inbm_event_socket;
    unsigned long inbm_ip;     /* IP address of the Jaguar from which the requests are expected */
    int           connected;
} remote_cxn_t;

typedef struct locate_sys_conf_s {
    struct ifreq       ifr_ip;
    struct ifreq       ifr_mac;
    char               hostname[VTSS_DAYTONA_HOST_NAME_MAX];
} locate_sys_conf_t;


static remote_cxn_t      remote_server;
#endif /*VTSS_SW_OPTION_REMOTE_TS_PHY*/

/* Board specifics */
static vtss_board_t board_table[VTSS_APPL_INST_CNT];

/* ================================================================= *
 *  Trace
 * ================================================================= */

vtss_trace_conf_t vtss_appl_trace_conf = {
    .level = {VTSS_TRACE_LEVEL_ERROR, VTSS_TRACE_LEVEL_ERROR}
};

static void printf_trace_head(const vtss_trace_layer_t layer,
                              const vtss_trace_group_t group,
                              const vtss_trace_level_t level,
                              const char *file,
                              const int line,
                              const char *function,
                              const char *lcont)
{
    time_t  t;
    int     h, m, s;    

    t = time(NULL);
    h = (t / 3600 % 24);
    m = (t / 60 % 60);
    s = (t % 60);
    printf("%u:%02u:%02u %s/%s %s%s", 
           h, m, s,
           layer == VTSS_APPL_TRACE_LAYER ? "APPL": layer == VTSS_TRACE_LAYER_AIL ? "AIL" : "CIL",
           level == VTSS_TRACE_LEVEL_ERROR ? "Error" :
           level == VTSS_TRACE_LEVEL_INFO ? "Info " :
           level == VTSS_TRACE_LEVEL_DEBUG ? "Debug" :
           level == VTSS_TRACE_LEVEL_NOISE ? "Noise" : "?????",
           function, lcont);
}

/* Trace callout function */
void vtss_callout_trace_printf(const vtss_trace_layer_t layer,
                               const vtss_trace_group_t group,
                               const vtss_trace_level_t level,
                               const char *file,
                               const int line,
                               const char *function,
                               const char *format,
                               ...)
{
    va_list va;
    printf_trace_head(layer, group, level, file, line, function, ": ");

    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    printf("\n");
}

/* Trace hex-dump callout function */
void vtss_callout_trace_hex_dump(const vtss_trace_layer_t layer,
                                 const vtss_trace_group_t group,
                                 const vtss_trace_level_t level,
                                 const char               *file,
                                 const int                line,
                                 const char               *function,
                                 const unsigned char      *byte_p,
                                 const int                byte_cnt)
{
    int i;

    printf_trace_head(layer, group, level, file, line, function, "\n");

    for (i= 0; i < byte_cnt; i += 16) {
        int j = 0;
        printf("%04x:", i);
        while (j+i < byte_cnt && j < 16) {
            printf(" %02x", byte_p[i+j]); 
            j++;
        }
        putchar('\n');
    }
}

// Pre-declare these to avoid warnings when built on some platforms.
void *vtss_callout_malloc(size_t size, vtss_mem_flags_t flags);
void  vtss_callout_free(void *ptr, vtss_mem_flags_t flags);

/* Alloc/Free callout functions */
void *vtss_callout_malloc(size_t size, vtss_mem_flags_t flags)
{
    return malloc(size);
}

void vtss_callout_free(void *ptr, vtss_mem_flags_t flags)
{
    free(ptr);
}

/* ================================================================= *
 *  API lock/unlock functions
 * ================================================================= */
BOOL api_locked = FALSE;

void vtss_callout_lock(const vtss_api_lock_t *const lock)
{
  T_N("Locking function %s", lock->function);
  // For testing we don't get a deadlock. The API must be unlocked before "locking"
  if (api_locked) {
    T_E("API lock problem");
  }
  api_locked = TRUE;
}

void vtss_callout_unlock(const vtss_api_lock_t *const lock)
{
  T_N("Unlocking function %s", lock->function);
  // For testing we don't get a deadlock. vtss_callout_lock must have been called before vtss_callout_unlock is called.
  if (!api_locked) {
    T_E("API unlock problem");
  }
  api_locked = FALSE;
}

/* ================================================================= *
 *  Main
 * ================================================================= */

/* Convert from internal to user port number */
vtss_uport_no_t iport2uport(vtss_port_no_t iport) 
{
    return (iport + 1 - VTSS_PORT_NO_START);
}

/* Convert from user to internal port number */
vtss_port_no_t uport2iport(vtss_uport_no_t uport) 
{
    return (uport == 0 ? VTSS_PORT_NO_NONE : (uport - 1 + VTSS_PORT_NO_START));
}

/* Determine if port is connected to 1G PHY */
static BOOL board_port_phy(vtss_board_t *board, vtss_port_no_t port_no)
{
    return (board->port_interface(port_no) == VTSS_PORT_INTERFACE_SGMII ? 1 : 0);
}

static BOOL board_port_10g(vtss_board_t *board, vtss_port_no_t port_no)
{
    return (board->port_interface(port_no) == VTSS_PORT_INTERFACE_XAUI ? 1 : 0);
}

#if defined(VTSS_ARCH_B2)
static BOOL board_port_spi4(vtss_board_t *board, vtss_port_no_t port_no)
{
    return (board->port_interface(port_no) == VTSS_PORT_INTERFACE_SPI4 ? 1 : 0);
}
#endif    

/* Setup port based on configuration and auto negotiation result */
static void port_setup(vtss_board_t *board, vtss_port_no_t port_no, BOOL aneg)
{
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_conf_t         phy;
    vtss_appl_port_conf_t   *pc = &board->port_conf[port_no];
#endif
#if defined(VTSS_FEATURE_PORT_CONTROL)
    vtss_appl_port_status_t *ps;
    vtss_port_conf_t        conf;
    
    if (board->feature.port_control)
        vtss_port_conf_get(board->inst, port_no, &conf);
    conf.if_type = board->port_interface(port_no);
    conf.power_down = (pc->enable ? 0 : 1);
    conf.flow_control.smac.addr[5] = port_no;
    conf.max_frame_length = pc->max_length;
#endif /* VTSS_FEATURE_PORT_CONTROL */
    
    if (aneg) {
#if defined(VTSS_FEATURE_PORT_CONTROL)
        /* Setup port based on auto negotiation status */
        ps = &board->port_status[port_no];
        conf.speed = ps->speed;
        conf.fdx = ps->fdx;
        conf.flow_control.obey = ps->aneg.obey_pause;
        conf.flow_control.generate = ps->aneg.generate_pause;
#endif /* VTSS_FEATURE_PORT_CONTROL */
    } else {
        /* Setup port based on configuration */
        if (board_port_phy(board, port_no)) {
            /* Setup PHY */
#ifdef VTSS_CHIP_CU_PHY
            phy.mode = (pc->enable ? 
                        (pc->autoneg || pc->speed == VTSS_SPEED_1G ? 
                         VTSS_PHY_MODE_ANEG : VTSS_PHY_MODE_FORCED) : 
                        VTSS_PHY_MODE_POWER_DOWN);
            phy.aneg.speed_10m_hdx = 1;
            phy.aneg.speed_10m_fdx = 1;
            phy.aneg.speed_100m_hdx = 1;
            phy.aneg.speed_100m_fdx = 1;
            phy.aneg.speed_1g_fdx = 1;
            phy.aneg.symmetric_pause = pc->flow_control;
            phy.aneg.asymmetric_pause = pc->flow_control;
            phy.forced.speed = pc->speed;
            phy.forced.fdx = pc->fdx;
            phy.mdi = VTSS_PHY_MDIX_AUTO; // always enable auto detection of crossed/non-crossed cables
            vtss_phy_conf_set(board->inst, port_no, &phy);
#endif
#if defined(VTSS_FEATURE_CLAUSE_37)
        } else if (conf.if_type == VTSS_PORT_INTERFACE_SERDES) {
            /* IEEE 802.3 clause 37 auto-negotiation */
            vtss_port_clause_37_control_t control;
            vtss_port_clause_37_adv_t     *adv;

            control.enable = pc->autoneg;
            adv = &control.advertisement;
            adv->fdx = 1;
            adv->hdx = 0;
            adv->symmetric_pause = pc->flow_control;
            adv->asymmetric_pause = pc->flow_control;
            adv->remote_fault = (pc->enable ? VTSS_PORT_CLAUSE_37_RF_LINK_OK : 
                                 VTSS_PORT_CLAUSE_37_RF_OFFLINE);
            adv->acknowledge = 0;
            adv->next_page = 0;
            vtss_port_clause_37_control_set(board->inst, port_no, &control);
#endif /* VTSS_FEATURE_CLAUSE_37 */
        }
#if defined(VTSS_FEATURE_PORT_CONTROL)
        /* Use configured values */
        conf.speed = (pc->autoneg ? VTSS_SPEED_1G : pc->speed);
        conf.fdx = pc->fdx;
        conf.flow_control.obey = pc->flow_control;
        conf.flow_control.generate = pc->flow_control;
#endif /* VTSS_FEATURE_PORT_CONTROL */
    }
#if defined(VTSS_FEATURE_PORT_CONTROL)
#if defined(VTSS_ARCH_B2)
    conf.flow_control.pfci_enable = pc->pfci_enable;
    conf.ct.rx_ct = pc->ct.rx_ct;
    conf.ct.tx_ct = pc->ct.tx_ct;
#endif /* VTSS_ARCH_B2 */
    if (board->feature.port_control)
        vtss_port_conf_set(board->inst, port_no, &conf);
#endif /* VTSS_FEATURE_PORT_CONTROL */
}

#if defined(NPI_INTERFACE)
int npi_sock;
static void npi_init(vtss_board_t *board)
{
    struct timeval tv;
    const char *iface = NPI_INTERFACE;
    struct ifreq ifreq; 
    
    /* Open RAW socket to send on VTSS ethertype 0x8880 */
    if ((npi_sock = socket(PF_PACKET, SOCK_RAW, htons(0x8880))) == -1) {
        perror("socket");
    }

    /* This may be unnecessary in some configurations */
    memset(&ifreq, 0, sizeof(ifreq)); 
    strncpy(ifreq.ifr_ifrn.ifrn_name, iface, strlen(iface)); 
    if (setsockopt(npi_sock, SOL_SOCKET, SO_BINDTODEVICE, &ifreq, sizeof(ifreq)) < 0) 
        perror("SO_BINDTODEVICE");

    tv.tv_sec = 0;
    tv.tv_usec = 10;
    setsockopt(npi_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
}

static void npi_poll(vtss_board_t *board)
{
    if(npi_sock >= 0) {
        static char buf[2048];
        int len;
        if((len = recv(npi_sock, buf, sizeof(buf), 0)) > 0) {
            vtss_packet_rx_meta_t pmeta;
            vtss_packet_rx_info_t info;
            memset(&pmeta, 0, sizeof(pmeta));
            pmeta.length = len - 32;
            if (vtss_packet_rx_hdr_decode(NULL, &pmeta, (u8*) &buf[16], &info) == VTSS_RC_OK) {
                T_I("NPI: Received %d bytes on port %d cos %d", info.length, iport2uport(info.port_no), info.cos);
            } else {
                T_E("vtss_packet_rx_hdr_decode error");
            }
        }
    }
}
#endif  /* NPI_INTERFACE */

#if defined(VTSS_FEATURE_PACKET) && !defined(VTSS_FEATURE_IRQ_CONTROL)
static void packet_poll(vtss_board_t *board)
{
    vtss_packet_rx_queue_t  queue;
    vtss_packet_rx_header_t header;
    u8                      frame[2000];
    u32                     i;
    char                    buf[100], *p;
    
    for (queue = VTSS_PACKET_RX_QUEUE_START; queue < VTSS_PACKET_RX_QUEUE_END; queue++) {
        if (vtss_packet_rx_frame_get(board->inst, queue, &header, frame, 2000) != VTSS_RC_OK)
            continue;

        T_I("received frame on port_no: %u, queue_mask: 0x%08x, length: %u",
            header.port_no, header.queue_mask, header.length);

        /* Dump frame */
        for (i = 0, p = &buf[0]; i < header.length; i++) {
            if ((i % 16) == 0) {
                p = &buf[0];
                p += sprintf(p, "%04x: ", i);
            }
            p += sprintf(p,"%02x%c", frame[i], ((i+9)%16)==0 ? '-' : ' ');
            if (((i+1) % 16) == 0 || (i+1) == header.length) {
                T_D("%s", buf);
            }
        }
    }
}
#endif /* VTSS_FEATURE_PACKET */

int str2layer(const char *layer)
{
    if (strcmp(layer, "app") == 0) {
        return VTSS_TRACE_LAYER_COUNT;
    } else if (strcmp(layer, "ail") == 0) {
        return VTSS_TRACE_LAYER_AIL;
    } else if (strcmp(layer, "cil") == 0) {
        return VTSS_TRACE_LAYER_CIL;
    }
    printf("'%s': layer unrecognized - use (app|ail|cil))\n", layer);
    return -1;
}

static struct {
    int group;
    const char *name;
} groups[] = {
    { VTSS_TRACE_GROUP_DEFAULT,      "default" },
    { VTSS_TRACE_GROUP_PORT,         "port" },
    { VTSS_TRACE_GROUP_PHY,          "phy" },
    { VTSS_TRACE_GROUP_PACKET,       "packet" },
    { VTSS_TRACE_GROUP_QOS,          "qos" },
    { VTSS_TRACE_GROUP_L2,           "l2" },
    { VTSS_TRACE_GROUP_L3,           "l3" },
    { VTSS_TRACE_GROUP_SECURITY,     "security" },
    { VTSS_TRACE_GROUP_EVC,          "evc" },
    { VTSS_TRACE_GROUP_FDMA_NORMAL,  "fdma_normal" },
    { VTSS_TRACE_GROUP_FDMA_IRQ,     "fdma_irq" },
#ifdef VTSS_ARCH_DAYTONA           
    { VTSS_TRACE_GROUP_UPI,          "upi" },
    { VTSS_TRACE_GROUP_XFI,          "xfi" },
    { VTSS_TRACE_GROUP_PCS_10GBASE_R,"10base-r" },
    { VTSS_TRACE_GROUP_MAC10G,       "mac10g" },
    { VTSS_TRACE_GROUP_RAB,          "rab" },
#endif /* VTSS_ARCH_DAYTONA */
    { VTSS_TRACE_GROUP_REG_CHECK,    "reg_check" },
    { VTSS_TRACE_GROUP_MPLS,         "mpls" },
    { VTSS_TRACE_GROUP_MACSEC,       "macsec" },
    { VTSS_TRACE_GROUP_VCAP,         "vcap" },
    { VTSS_TRACE_GROUP_OAM,          "oam" },
    { VTSS_TRACE_GROUP_TS,           "ts" },
    { VTSS_TRACE_GROUP_COUNT,        NULL },
};

int str2group(const char *group)
{
    int i;
    for(i = 0; groups[i].name != NULL; i++) {
        if(strcmp(groups[i].name, group) == 0) {
            return groups[i].group;
        }
    }
    if (strcmp(group, "show") != 0) {
        printf("'%s': group unrecognized - use 'show' to list\n", group);
    }
    return -1;
}

int str2level(const char *level)
{
    if (strcmp(level, "none") == 0) {
        return VTSS_TRACE_LEVEL_NONE;
    } else if (strcmp(level, "error") == 0) {
        return VTSS_TRACE_LEVEL_ERROR;
    } else if (strcmp(level, "info") == 0) {
        return VTSS_TRACE_LEVEL_INFO;
    } else if (strcmp(level, "debug") == 0) {
        return VTSS_TRACE_LEVEL_DEBUG;
    } else if (strcmp(level, "noise") == 0) {
        return VTSS_TRACE_LEVEL_NOISE;
    }
    printf("'%s': level unrecognized - use (none|error|info|debug|noise)\n", level);
    return -1;
}

static int parse_options(int argc, char **argv, int *count)
{
    int opt;

    while ((opt = getopt(argc, argv, "hvd:t:")) != -1) {
        switch (opt) {
        case '?':
        case 'h': /* Help */
            printf("vtss_appl [-v] -d <level> [-t[<layer>][:<group>][:<level>]] [-- <board-specific args>]\n");
            printf("-v: Show version\n");
            printf("-d: Enable application trace\n");
            printf("-t: Show or set trace level for a specific layer and group. (Multiple occurrences allowed)\n");
            return 1;
        case 'v': /* Version */
            printf("Version: %s\n", vtss_version_string);
            printf("Date   : %s\n", __DATE__ ", " __TIME__);
            return 1;
        case 'd':
        {
            int level = str2level(optarg);
            if (level < 0) {
                return 1;
            } else {
                printf("Setting trace level for: layer '%s', group '%s', level '%s'\n", "app", "all", optarg);
                vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_AIL]  = level;
                vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_CIL]  = level;
                vtss_appl_trace_conf.level[VTSS_TRACE_LAYER_COUNT]  = level;
            }
        }
        break;
        case 't':
        {
            int group, layer, level;
            const char *s_layer, *s_group, *s_level;
            char *saveptr;
            
            s_layer = strtok_r(optarg, ":", &saveptr);
            s_group = strtok_r(NULL, ":", &saveptr);
            s_level = strtok_r(NULL, ":", &saveptr);

            if(s_layer == NULL) {
                s_layer = "ail"; /* Default */
            }
            if(s_group == NULL) {
                s_group = "default"; /* Default */
            }
            if(s_level == NULL) {
                s_level = "debug"; /* Default */
            }

            layer = str2layer(s_layer);
            if (layer < 0) {
                return 1;
            }
            group = str2group(s_group);
            if (group < 0) {
                if (strcmp(s_group, "show") == 0) {
                    int i;
                    printf("Trace groups: ");
                    for(i = 0; groups[i].name != NULL; i++) {
                        printf("'%s'| ", groups[i].name);
                    }
                    printf("'show'\n");
                }
                return 1;
            }
            level = str2level(s_level);
            if(level < 0) {
                return 1;
            }

            printf("Setting trace level for: layer '%s', group '%s', level '%s'\n", s_layer, s_group, s_level);

            if (group == VTSS_TRACE_GROUP_COUNT) {
                vtss_appl_trace_conf.level[layer]  = level;
            } else {
                vtss_trace_conf_t  conf;
                if(vtss_trace_conf_get(group, &conf) == VTSS_RC_OK) {
                    conf.level[layer]  = level;
                    vtss_trace_conf_set(group, &conf);
                } else {
                    T_E("vtss_trace_conf_get(%d) fails", group);
                }
            }
            break;
        }
        default:
            break;              /* Unknow/board option */
        } 
    }
    *count = optind;
    return 0;
}

#if defined(VTSS_SW_OPTION_REMOTE_TS_PHY)
/* Set the socket as non-blocking */
static vtss_rc setnonblocking(int sock)
{
        int opts;

        opts = fcntl(sock, F_GETFL);
        if (opts < 0) {
            perror("fcntl(F_GETFL)");
            return VTSS_RC_ERROR;
        }
        opts = (opts | O_NONBLOCK);
        if (fcntl(sock, F_SETFL, opts) < 0) {
            perror("fcntl(F_SETFL)");
            return VTSS_RC_ERROR;
        }
        return VTSS_RC_OK;
}

static vtss_rc open_socket(void* sock_data)
{
    int                 return_value;
    struct sockaddr_in  server_address;
    socket_data         *server = NULL;

    server = (socket_data*)sock_data;
    /* bind local server port */
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl (INADDR_ANY);
    server_address.sin_port = htons(server->port_no);
    return_value =  bind (server->socket_descriptor, (struct sockaddr *) &server_address, sizeof (server_address));
    if (return_value < 0)
    {
        printf ("Server: cannot bind port number %d \n", server->port_no);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_sys_setup(locate_sys_conf_t *sys_conf)
{
    int fd, rc;

    /* Create the socket and Bind the socket */
    rc = gethostname(sys_conf->hostname, sizeof (sys_conf->hostname));
    if (rc < 0) {
        perror("error in getting the hostname");
        return VTSS_RC_ERROR;
    }
    /* Get the IP address of the Ethernet Interface ETH-0 */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("error in creating the socket");
        return VTSS_RC_ERROR;
    }
    memcpy(sys_conf->ifr_ip.ifr_name, "eth0", sizeof("eth0"));

    rc = ioctl(fd, SIOCGIFADDR, &sys_conf->ifr_ip);
    if (rc < 0) {
        perror("error in getting the IP address");
        return VTSS_RC_ERROR;
    }

    memcpy(sys_conf->ifr_mac.ifr_name, "eth0", sizeof("eth0"));
    /* Get the MAC address of the Ethernet Interface ETH-0 */
    rc = ioctl(fd, SIOCGIFHWADDR, &sys_conf->ifr_mac);
    if (rc < 0) {
        perror("error in getting the IP address");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc locate_request_process(remote_cxn_t* remote_server, locate_sys_conf_t *sys_conf)
{
    unsigned int        client_address_length;
    int                 tx_len, rx_len;
    struct sockaddr_in  tx_address, rx_address;
    char                message[MAX_MSG] = {0};
    unsigned char       *buff_ptr = NULL;

    client_address_length = sizeof (rx_address);
    printf("Inside FN :: %s \n\r",__FUNCTION__);
    rx_len = recvfrom(remote_server->locate_socket.socket_descriptor, message, MAX_MSG, 0,
                                (struct sockaddr *) &rx_address, &client_address_length);

    if (rx_len < 0)
    {
        printf ("Locate Socket: cannot receive data \n");
        return VTSS_RC_ERROR;
    } else {
        message[rx_len]='\0';
        if (memcmp((message + 1), VTSS_DAYTONA_GENIE_DISCOVERY, sizeof(VTSS_DAYTONA_GENIE_DISCOVERY)) == 0) {
            /* Reset the Buff */
            memset(message, 0, sizeof(message));
            /* Fill in the MAC address */
            buff_ptr = message + 1;
            tx_len = 0;
            memcpy(buff_ptr, sys_conf->ifr_mac.ifr_hwaddr.sa_data, 6);
            tx_len += (6 + 2);
            buff_ptr += (6 + 2);

            tx_address.sin_addr.s_addr = ((struct sockaddr_in *)&sys_conf->ifr_ip.ifr_ifru)->sin_addr.s_addr;
            memcpy(buff_ptr, &tx_address.sin_addr.s_addr, 4);
            tx_len = 21;
            buff_ptr = message + 21;
            memcpy(buff_ptr, sys_conf->hostname, strlen(sys_conf->hostname));
            tx_len += strlen(sys_conf->hostname);
            buff_ptr += strlen(sys_conf->hostname);
            message[tx_len] = '\0';
            rx_len = sendto(remote_server->locate_socket.socket_descriptor, message, tx_len, 0, (struct sockaddr *)&rx_address, sizeof(rx_address));
            if (rx_len < 0) {
                perror("error in sending the frame");
                return VTSS_RC_ERROR;
            }
        }
    }
    return VTSS_RC_OK;
}


/* Update the internal timestamp table, from HW */
static void remote_ts_phy_fifo_read(  const vtss_inst_t              inst,
                                      const vtss_port_no_t           port_no,
                                      const vtss_phy_timestamp_t     *const fifo_ts,
                                      const vtss_phy_ts_fifo_sig_t   *const sig,
                                      void                           *cntxt,
                                      const vtss_phy_ts_fifo_status_t status)
{
    struct sockaddr_in si = {0};
    unsigned char buff[5000];
    remote_ts_msg_header_t *header = NULL;
    vtss_phy_ts_fifo_sig_t sig_loc;
    static int event_id;
    int error;
    /* Send Message from Daytona to the connected Jaguar Board */
    memset(&sig_loc, 0, sizeof(sig_loc));
    memcpy(&sig_loc, sig, sizeof(vtss_phy_ts_fifo_sig_t));
//    printf("Inside FN :: %s need to send a message to the Jaguar 50% Job Done Line : %d \n\r",__FUNCTION__, __LINE__);
    printf("port_no %u, tx signature: mask %x, type %d, domain %d, seq %d\n\r",
         port_no,
         sig->sig_mask, sig->msg_type, sig->domain_num, sig->sequence_id);
    printf("tx time:  %d, %u:%u, hex ns %x\n\r",
         fifo_ts->seconds.high, fifo_ts->seconds.low, fifo_ts->nanoseconds, fifo_ts->nanoseconds);


//    static remote_cxn_t      remote_server;
//    T_D("Got socket fd %d", *fd);
    memset((char *)&si, 0, sizeof(si));
    memset(buff, 0, sizeof(buff));
    si.sin_family = AF_INET;
    si.sin_port = htons(remote_server.inbm_event_socket.port_no);
    si.sin_addr.s_addr = htonl(remote_server.inbm_ip);

    header = (remote_ts_msg_header_t *) buff;

    // Create the header...
    header->message_id   = event_id++;
    header->command      = REMOTE_TS_PHY_COMMAND_EVENT;
    header->message_type = REMOTE_TS_PHY_MESSAGE_TYPE_EVENT;
    header->length       = htons(sizeof (remote_ts_msg_header_t) + sizeof(vtss_phy_timestamp_t) + sizeof(vtss_phy_ts_fifo_sig_t) + sizeof(vtss_phy_ts_fifo_status_t));
    header->instance     = 0;
    header->port_no      = htons(port_no);
    header->status       = 0;

    memcpy(&buff[sizeof(remote_ts_msg_header_t)], fifo_ts, sizeof(vtss_phy_timestamp_t));
    memcpy(&buff[sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_timestamp_t)], sig, sizeof(vtss_phy_ts_fifo_sig_t));
    memcpy(&buff[sizeof(remote_ts_msg_header_t) + sizeof(vtss_phy_timestamp_t) + sizeof(vtss_phy_ts_fifo_sig_t)], &status, sizeof(vtss_phy_ts_fifo_status_t));

    error = sendto(remote_server.inbm_event_socket.socket_descriptor, buff, (sizeof (remote_ts_msg_header_t) + sizeof(vtss_phy_timestamp_t) + sizeof(vtss_phy_ts_fifo_sig_t) + sizeof(vtss_phy_ts_fifo_status_t)), 0, (struct sockaddr *)&si, sizeof(si));
    if (error < 0) {
        printf("Error in sending the message to the server");
    }
    printf("Send the message to the server \n\r");
}
static vtss_rc inbm_connection_process(remote_cxn_t *remote_server)
{
    int                rc, rx_len = sizeof(struct sockaddr);
    vtss_rc   api_rc = VTSS_RC_ERROR;
    struct sockaddr_in rx_addr = {0};
    unsigned char      buff[MAX_MSG];
    vtss_phy_ts_init_conf_t  phy_conf;

    //printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);
    memset(buff, 0, sizeof(buff));
    rc = recvfrom(remote_server->inbm_socket.socket_descriptor, buff, sizeof(buff), 0, (struct sockaddr *)&rx_addr, &rx_len);
    if (rc < 0) {
        perror("error in receiving the frame");
        return VTSS_RC_ERROR;
    }

//    printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);

    if (remote_server->connected == TRUE) {
//        printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);
        if (memcmp(buff, VTSS_DAYTONA_GENIE_INBM_DISCONN, sizeof(VTSS_DAYTONA_GENIE_INBM_DISCONN)) == 0) {
            //printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);
            /* Reset the Buff */
            memset(buff, 0, sizeof(buff));
            memcpy(buff, VTSS_DAYTONA_GENIE_INBM_AUTH, sizeof(VTSS_DAYTONA_GENIE_INBM_AUTH));
            rc = sendto(remote_server->inbm_socket.socket_descriptor, buff, sizeof(VTSS_DAYTONA_GENIE_INBM_AUTH), 0, (struct sockaddr *)&rx_addr, sizeof(rx_addr));
            if (rc < 0) {
                perror("error in sending the frame");
                return VTSS_RC_ERROR;
            }
            remote_server->connected = FALSE;
            memset(&remote_server->inbm_ip, 0, sizeof(remote_server->inbm_ip));
        } else if (memcmp(buff, VTSS_DAYTONA_GENIE_INBM_CONNECT, sizeof(VTSS_DAYTONA_GENIE_INBM_CONNECT)) != 0) {
//          printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);
          if (remote_server->inbm_ip == rx_addr.sin_addr.s_addr) {
              remote_ts_msg_header_t *msg_hdr = NULL;

              //printf("Inside FN : %s Line : %d \n\r",__FUNCTION__, __LINE__);
              /* Received the Messages -*/
              msg_hdr = (remote_ts_msg_header_t *) buff;
              msg_hdr->length       = ntohs(msg_hdr->length);
              msg_hdr->port_no      = ntohs(msg_hdr->port_no);

              /* Parse the message and call the API
               * Get the return value from the API
               */
              //printf("    version      - [%x] \n\r",msg_hdr->version);
              //printf("    message_id   - [%x] \n\r",msg_hdr->message_id);
              //printf("    command      - [%x] \n\r",msg_hdr->command);
              //printf("    message_type - [%x] \n\r",msg_hdr->message_type);
              //printf("    length       - [%x] \n\r",msg_hdr->length);
              //printf("    instance     - [%x] \n\r",msg_hdr->instance);
              //printf("    status       - [%x] \n\r",msg_hdr->status);
              //printf("    port_no      - [%x] \n\r",msg_hdr->port_no);

              //printf("\n\r");
              //for (i = 0; i< msg_hdr->length; i++ ) {
              //   printf(" %x ",buff[i]);
              //}
              //printf("\n\r");
              rc = vtss_daytona_remote_phy_ts_rx_parse(msg_hdr, (buff + (sizeof(remote_ts_msg_header_t))));
              if (!((msg_hdr->command == REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_SET) ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_INGRESS_LATENCY_SET) ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_EGRESS_LATENCY_SET ) ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_DELAY_ASYMMETRY_SET) ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_PATH_DELAY_SET)      ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_PTP_TIME_SET)        ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_PTP_TIME_SET)        ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_CLOCK_ADJUST_1NS)    ||
                    (msg_hdr->command == REMOTE_TS_PHY_COMMAND_MODE_SET))) {
                  /* Send a Reply Message to the Originator */
                  msg_hdr->message_type    = REMOTE_TS_PHY_MESSAGE_TYPE_REPLY;
                  //printf("Before sending the Message of length - [%d] skd - [%d] \n\r",msg_hdr->length, remote_server->inbm_socket.socket_descriptor);
                  //printf("\n\r");
                  //for (i = 0; i< msg_hdr->length; i++ ) {
                  //   printf(" %x ",buff[i]);
                  //}

                  rc = sendto(remote_server->inbm_socket.socket_descriptor, buff, msg_hdr->length, 0, (struct sockaddr *)&rx_addr, sizeof(rx_addr));
                  if (rc < 0) {
                      printf("Error in sending the frame \n\r");
                      perror("error in sending the frame");
                      return VTSS_RC_ERROR;
                  }
              }
          } else {
              printf("Got message from different IP \n\r");
          }
        } else {
            /* Check if the request is received from the same IP
             * Also check if the received is a 1588 API packet
             * If yes parse the packet; else return error as it is already connected
             */
//            printf("Already connected \n");
            return VTSS_RC_ERROR;
        }
    } else {
        //printf("Inside FN :: %s B4 connecting \n\r",__FUNCTION__);
        if (memcmp(buff, VTSS_DAYTONA_GENIE_INBM_CONNECT, sizeof(VTSS_DAYTONA_GENIE_INBM_CONNECT)) == 0) {
            /* Reset the Buff */
            memset(buff, 0, sizeof(buff));
            memcpy(buff, VTSS_DAYTONA_GENIE_INBM_AUTH, sizeof(VTSS_DAYTONA_GENIE_INBM_AUTH));
            rc = sendto(remote_server->inbm_socket.socket_descriptor, buff, sizeof(VTSS_DAYTONA_GENIE_INBM_AUTH), 0, (struct sockaddr *)&rx_addr, sizeof(rx_addr));
            if (rc < 0) {
                perror("error in sending the frame");
                return VTSS_RC_ERROR;
            }
            /* Connection has been established */
            printf("Connection is established and doing the initialization \n\r");
            memset(&phy_conf, 0 , sizeof(phy_conf));
            phy_conf.clk_freq = VTSS_PHY_TS_CLOCK_FREQ_15625M ;
            phy_conf.clk_src = VTSS_PHY_TS_CLOCK_SRC_EXTERNAL;
            phy_conf.rx_ts_pos = VTSS_PHY_TS_RX_TIMESTAMP_POS_IN_PTP;
            phy_conf.rx_ts_len = VTSS_PHY_TS_RX_TIMESTAMP_LEN_30BIT;
            phy_conf.tx_fifo_mode = VTSS_PHY_TS_FIFO_MODE_NORMAL;
            phy_conf.tx_ts_len = VTSS_PHY_TS_FIFO_TIMESTAMP_LEN_4BYTE;
            printf("Inside FN :: %s Line : %d \n\r",__FUNCTION__, __LINE__);
            api_rc = vtss_phy_ts_init(NULL, 2, &phy_conf);
            if (api_rc < 0) {
                printf("Error in initialising the PHY \n\r");
            }
            api_rc = vtss_phy_ts_init(NULL, 3, &phy_conf);
            if (api_rc < 0) {
                printf("Error in initialising the PHY \n\r");
            }
            /* Install the callback function to the PHY_TS_API */
           api_rc = vtss_phy_ts_fifo_read_install(NULL, remote_ts_phy_fifo_read, NULL);
           if (api_rc < 0) {
               printf("Error in installing fifo \n\r");
           }

            api_rc = vtss_phy_ts_event_enable_set(NULL, 2, TRUE, 0x3F);
            if (api_rc < 0) {
                printf("Error in enabling the event \n\r");
            }


            api_rc = vtss_phy_ts_event_enable_set(NULL, 3, TRUE, 0x3F);
            if (api_rc < 0) {
                printf("Error in enabling the event \n\r");
            }


            remote_server->connected = TRUE;
            memcpy(&remote_server->inbm_ip, &rx_addr.sin_addr.s_addr, sizeof(remote_server->inbm_ip));
        }
    }
    return VTSS_RC_OK;
}
static void handle_data(remote_cxn_t *remote_server, locate_sys_conf_t *sys_conf, fd_set *rfds)
{
    if(FD_ISSET(remote_server->locate_socket.socket_descriptor, rfds))  {
        locate_request_process(remote_server, sys_conf);
    }
    if(FD_ISSET(remote_server->inbm_socket.socket_descriptor, rfds)) {
        inbm_connection_process(remote_server);
    }
}

static vtss_rc daytona_jr_comm_init(remote_cxn_t* remote_server, locate_sys_conf_t *sys_conf)
{
    /* Get the info about the machine */
    daytona_sys_setup(sys_conf);
    /* socket creation */
    remote_server->locate_socket.socket_descriptor = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (remote_server->locate_socket.socket_descriptor < 0)
    {
        printf ("Cannot open locate socket\n");
        return VTSS_RC_ERROR;
    }
    /* Set sockets as Non-Blocking */
    if (setnonblocking(remote_server->locate_socket.socket_descriptor) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    remote_server->locate_socket.port_no = VTSS_DAYTONA_LOCATE_SERVER_PORT;
    if (open_socket((void*)&remote_server->locate_socket) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    remote_server->inbm_socket.socket_descriptor = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (remote_server->inbm_socket.socket_descriptor < 0)
    {
        printf ("Cannot open INBM socket\n");
        return VTSS_RC_ERROR;
    }
    if (setnonblocking(remote_server->inbm_socket.socket_descriptor) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    remote_server->inbm_socket.port_no = VTSS_DAYTONA_INBM_SERVER_PORT;
    if (open_socket((void*)&remote_server->inbm_socket) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }

    remote_server->inbm_event_socket.socket_descriptor = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (remote_server->inbm_event_socket.socket_descriptor < 0)
    {
        printf ("Cannot open INBM Event socket\n");
        return VTSS_RC_ERROR;
    }
    if (setnonblocking(remote_server->inbm_event_socket.socket_descriptor) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    remote_server->inbm_event_socket.port_no = VTSS_DAYTONA_INBM_SERVER_EVT_PORT;
    if (open_socket((void*)&remote_server->inbm_event_socket) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}
#endif /*VTSS_SW_OPTION_REMOTE_TS_PHY*/

#if defined(VTSS_FEATURE_PACKET)
static void setup_rx_reg(void)
{
    vtss_packet_rx_conf_t conf;
    int i;

    // Get Rx packet configuration */
    if (vtss_packet_rx_conf_get(0, &conf) != VTSS_RC_OK) {
        T_E("vtss_packet_rx_conf_get failed");
        return;
    }

    // Setup CPU rx registration
    memset(&conf.reg, FALSE, sizeof(conf.reg));
    conf.reg.bpdu_cpu_only = TRUE;
    conf.map.bpdu_queue = VTSS_PACKET_RX_QUEUE_CNT-1;

    /* Queue setup */
    for (i = VTSS_PACKET_RX_QUEUE_START; i < VTSS_PACKET_RX_QUEUE_END; i++) {
        conf.queue[i].size = 8 * 1024;
    }

    // Set Rx packet configuration */
    if (vtss_packet_rx_conf_set(0, &conf) != VTSS_RC_OK) {
        T_E("vtss_packet_rx_conf_set failed");
    }

#if defined(VTSS_FEATURE_ACL_V1) || defined(VTSS_FEATURE_ACL_V2)
    {
        vtss_ace_t ace;
        vtss_rc rc;
        vtss_ace_init(NULL, VTSS_ACE_TYPE_ANY, &ace);
        ace.id = 1;
        ace.action.cpu = TRUE;
        ace.action.cpu_queue = VTSS_PACKET_RX_QUEUE_CNT-2;
#if defined(VTSS_FEATURE_ACL_V1)
        ace.port_no = VTSS_PORT_NO_ANY;
#endif /* VTSS_FEATURE_ACL_V1 */
#if defined(VTSS_FEATURE_ACL_V2)
        memset(ace.port_list, TRUE, sizeof(ace.port_list));
#endif /* VTSS_FEATURE_ACL_V2 */
        if((rc = vtss_ace_add(NULL, VTSS_ACE_ID_LAST, &ace)) != VTSS_RC_OK) {
            printf("Error adding ACE %u: %d\n", ace.id, rc);
        } else {
            printf("Added ACE %u for CPU capture\n", ace.id);
        }
    }
#endif
}
#endif /* VTSS_FEATURE_PACKET */

int main(int argc, char **argv)
{
    vtss_board_t            *board;
    vtss_appl_inst_t        inst;
    int                     count = 0;
    vtss_inst_create_t      create;
    vtss_init_conf_t        init_conf;
    vtss_port_no_t          port_no;
    vtss_appl_port_conf_t   *pc;
#if defined(VTSS_SW_OPTION_REMOTE_TS_PHY)
    static locate_sys_conf_t sys_conf;
    fd_set                   rdfs_daytona;
    struct timeval           tv;
    BOOL                     rem_flag = FALSE;
    int                      retval;
#endif/*VTSS_SW_OPTION_REMOTE_TS_PHY*/
#ifndef VTSS_ARCH_DAYTONA
    vtss_appl_port_status_t *ps;
#endif /* VTSS_ARCH_DAYTONA */
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_reset_conf_t   phy_reset;
#endif
#ifndef VTSS_ARCH_DAYTONA
    vtss_port_status_t      status;
#endif /* VTSS_ARCH_DAYTONA */
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t     oper_mode;
#endif /* VTSS_CHIP_10G_PHY */
#ifndef VTSS_ARCH_DAYTONA
    BOOL                    link_old;
#endif /* VTSS_ARCH_DAYTONA */
    BOOL                    port_poll[VTSS_APPL_INST_CNT][VTSS_PORT_ARRAY_SIZE];
    BOOL                    init_done = 0;
#if defined(VTSS_ARCH_B2)
    vtss_host_conf_t        host_conf;          
#endif /* VTSS_ARCH_B2 */
    
    if (parse_options(argc, argv, &count))
        return 1;

    T_D("enter");

    /* Initialize board table */
    board = &board_table[0];
#if defined(BOARD_ESTAX_34_REF)
    vtss_board_estax34_init(board);
    board++;
#endif /* BOARD_ESTAX_34_REF */
#if defined(BOARD_B2_EVAL)
    vtss_board_b2_init(board);
#endif
#if defined(BOARD_DAYTONA_EVAL) || defined(BOARD_DAYTONA) || defined(BOARD_TALLADEGA)
    vtss_board_daytona_init(board);
    board++;
#endif
#if defined(BOARD_DAYTONA_EVAL) || defined(BOARD_TALLADEGA)
    vtss_board_daytona_init(board);
    board++;
#endif /* BOARD_ESTAX_34_REF */
#if defined(BOARD_LUTON26_EVAL)
    vtss_board_l26_eval_init(board);
#endif /* BOARD_LUTON26_EVAL */
#if defined(BOARD_SERVAL_EVAL)
    vtss_board_serval_eval_init(board);
#endif /* BOARD_SERVAL_EVAL */
#if defined(BOARD_SERVAL_REF)
    vtss_board_serval_ref_init(board);
#endif /* BOARD_SERVAL_REF */
#if defined(BOARD_JAGUAR1_EVAL)
    vtss_board_jr1_eval_init(board);
#endif /* BOARD_JAGUAR1_EVAL */
#if defined(BOARD_JAGUAR2_REF)
    vtss_board_jr2_ref_init(board);
#endif /* BOARD_JAGUAR2_ref */

#if defined(VTSS_CHIP_10G_PHY)
    /* Configure the 10G phy operating mode */    
    oper_mode.oper_mode = VTSS_PHY_LAN_MODE;  /* LAN Mode */
    oper_mode.high_input_gain = 0; /* Disable high input gain */
    oper_mode.interface = VTSS_PHY_XAUI_XFI; /* XAUI <-> XFI */
#endif /* VTSS_CHIP_10G_PHY */

    /* Initialize boards */
    for (inst = 0; inst < VTSS_APPL_INST_CNT; inst++) {

        T_D("---------- init inst 0 ---------");

        board = &board_table[inst];

        vtss_inst_get(board->target, &create);
        vtss_inst_create(&create, &board->inst);
    
        vtss_init_conf_get(board->inst, &init_conf);
        board->init.init_conf = &init_conf;

#if defined(VTSS_ARCH_B2)
        if (board->b2) {
            vtss_host_conf_get(board->inst, VTSS_PORT_NO_NONE, &host_conf); /* Get Host defaults (using VTSS_PORT_NO_NONE) */
            board->init.xaui = &host_conf.xaui;            
            board->init.spi4 = &host_conf.spi4;
        } 
#endif
        if (board->board_init(argc - count, (const char **)(argv + count), board))
            return 1;

        if (vtss_init_conf_set(board->inst, &init_conf) == VTSS_RC_ERROR) {
            T_E("Could not initialize");
            return 1;
        };
      
#if defined(VTSS_FEATURE_PORT_CONTROL)
        if (board->feature.port_control)
            vtss_port_map_set(board->inst, board->port_map);
#endif /* VTSS_FEATURE_PORT_CONTROL */

        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            /* Default port status */
            board->port_status[port_no].link = 0;

            /* Default port configuration */
            pc = &board->port_conf[port_no];
            memset(pc, 0, sizeof(*pc));
            switch (board->port_interface(port_no)) {
            case VTSS_PORT_INTERFACE_100FX:
                pc->speed = VTSS_SPEED_100M;
                break;
            case VTSS_PORT_INTERFACE_XAUI:
                pc->speed = VTSS_SPEED_10G;
                break;
            case VTSS_PORT_INTERFACE_VAUI:
                pc->speed = VTSS_SPEED_5G;
                break;
            case VTSS_PORT_INTERFACE_SGMII:
            case VTSS_PORT_INTERFACE_QSGMII:
                pc->speed = VTSS_SPEED_1G;
                pc->autoneg = 1;
                pc->flow_control = 1;
                break;
            case VTSS_PORT_INTERFACE_SERDES:
                pc->speed = VTSS_SPEED_1G;
                break;
            default:
                T_E("unknown if_type on port %u", port_no);
            }
            pc->enable = 1;
            pc->fdx = 1;
            port_poll[inst][port_no] = (board->port_poll == NULL || 
                                        board->port_poll(port_no) ? 1 : 0);
#if defined(VTSS_FEATURE_PORT_CONTROL)
            pc->max_length = VTSS_MAX_FRAME_LENGTH_STANDARD;
            if (board->port_map != NULL && board->port_map[port_no].chip_port < 0) {
                port_poll[inst][port_no] = 0;
            } 
#endif /* VTSS_FEATURE_PORT_CONTROL */
        }
        
        if (board->board_init_post)
            board->board_init_post(board);

#if defined(VTSS_FEATURE_PACKET)
        /* Packet RX */
        setup_rx_reg();
#endif

#ifdef VTSS_CHIP_CU_PHY
        board->pre_reset(); // Mainly PHY pre-reset 
#endif
        
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            if (port_poll[inst][port_no] == 0)
                continue;

#if defined(VTSS_CHIP_10G_PHY)     
            if (board_port_10g(board, port_no)) {
                if (vtss_phy_10g_mode_set(board->inst, port_no, &oper_mode) != VTSS_RC_OK) {
                    T_E("Could not set the 10g Phy operating mode (port %u)", port_no);
                }                    
            }
#endif /* VTSS_CHIP_10G_PHY */
        
#if defined(VTSS_ARCH_B2)
            if (board->b2) {
                if (board_port_spi4(board, port_no)) { 
                    port_poll[inst][port_no] = 0;  /* Exclude poll SPI4 port */
                    if (vtss_host_conf_set(board->inst, port_no, &host_conf) != VTSS_RC_OK)
                        T_E("Could not configure SPI4 host (port %u)", port_no);
                    continue;
                } else if (board_port_10g(board, port_no)) {
                    if (init_conf.host_mode != 6 &&
                        vtss_host_conf_set(board->inst, port_no, &host_conf) != VTSS_RC_OK) {
                        T_E("Could not configure XAUI port %u", port_no);                
                    }
                    if (init_conf.host_mode < 4) /* XAUI host mode */
                        continue; 
                }
            }
#endif /* VTSS_ARCH_B2 */        
#ifdef VTSS_CHIP_CU_PHY
            if (board_port_phy(board, port_no)) {
                phy_reset.mac_if = board->port_interface(port_no);
                phy_reset.media_if = VTSS_PHY_MEDIA_IF_CU;
                vtss_phy_reset(board->inst, port_no, &phy_reset);
            }
#endif
            port_setup(board, port_no, 0);
        }

#ifdef VTSS_CHIP_CU_PHY
        board->post_reset(); // Mainly PHY post reset 
#endif
        if (board->board_init_done)
            board->board_init_done(board);

    } /* Instance loop */

    vtss_appl_cli_init();

#if defined(VTSS_SW_OPTION_REMOTE_TS_PHY)
    if (daytona_jr_comm_init(&remote_server, &sys_conf) == VTSS_RC_OK) {
        rem_flag = TRUE;
    }
#endif /*VTSS_SW_OPTION_REMOTE_TS_PHY*/

#if defined(NPI_INTERFACE)
    npi_init(board);
#endif /* NPI_INTERFACE */            
    
    for (;;) {
        inst++;
        if (inst >= VTSS_APPL_INST_CNT)
            inst = 0;
        board = &board_table[inst];
#if defined(VTSS_SW_OPTION_REMOTE_TS_PHY)
        if (rem_flag) {
            FD_ZERO(&rdfs_daytona);
            FD_SET(remote_server.locate_socket.socket_descriptor, &rdfs_daytona);
            FD_SET(remote_server.inbm_socket.socket_descriptor, &rdfs_daytona);
            tv.tv_sec = 0;
            tv.tv_usec = 0;
            retval = select(((remote_server.locate_socket.socket_descriptor > remote_server.inbm_socket.socket_descriptor) ? remote_server.locate_socket.socket_descriptor : remote_server.inbm_socket.socket_descriptor) + 1, &rdfs_daytona, NULL, NULL, &tv);
            if (retval < 0) {
                perror("select()");
            } else if (retval) {
                handle_data(&remote_server, &sys_conf, &rdfs_daytona);
            }
        }
#endif /*VTSS_SW_OPTION_REMOTE_TS_PHY*/

#ifdef VTSS_ARCH_DAYTONA
        if (board->daytona) {
            vtss_board_daytona_interrupt(); 
        }
#endif
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
#if defined(VTSS_FEATURE_PACKET) && !defined(VTSS_FEATURE_IRQ_CONTROL)
            if (board->feature.packet)
                packet_poll(board);
#endif /* VTSS_FEATURE_PACKET */            
            if (!port_poll[inst][port_no])
                continue;

#ifdef BOARD_B2_EVAL
            vtss_appl_cli_task(); /* Increase the CLI polling when using the B2 eval board */
#if defined(VTSS_CHIP_10G_PHY) && !defined(VTSS_ARCH_B2)
            continue; /* Port polling is dropped in 10G PHY only configuration */
#endif /* VTSS_CHIP_10G_PHY */            
#endif /* BOARD_B2_EVAL */

#ifndef VTSS_ARCH_DAYTONA
            if (board->feature.port_control) {
#if defined(VTSS_FEATURE_PORT_CONTROL)
                if (vtss_port_status_get(board->inst, port_no, &status) != VTSS_RC_OK)
                    continue;
#endif /* VTSS_FEATURE_PORT_CONTROL */
            } else {
#ifdef VTSS_CHIP_CU_PHY
                if (vtss_phy_status_get(board->inst, port_no, &status) != VTSS_RC_OK)
                    continue;                                
#endif
            }

            ps = &board->port_status[port_no];
            link_old = ps->link;
            ps->link = status.link;
            ps->speed = status.speed;
            ps->fdx = status.fdx;
            ps->aneg.obey_pause = status.aneg.obey_pause;
            ps->aneg.generate_pause = status.aneg.generate_pause;
            /* Detect link down and disable port */
            if ((!status.link || status.link_down) && link_old) {
                T_D("link down event on port_no: %u", port_no);
                link_old = 0;
#if defined(VTSS_FEATURE_LAYER2)
                if (board->feature.layer2) {
                    vtss_port_state_set(board->inst, port_no, 0);
                    vtss_mac_table_port_flush(board->inst, port_no);
                }
#endif /* VTSS_FEATURE_LAYER2 */
            }
         
            /* Detect link up and setup port */
            if (status.link && !link_old) { 
                T_D("link up event on port_no: %u", port_no);
#if defined(VTSS_FEATURE_LAYER2)
                if (board->feature.layer2) 
                    vtss_port_state_set(board->inst, port_no, 1);
#endif /* VTSS_FEATURE_LAYER2 */

                if (board->port_conf[port_no].autoneg)
                    port_setup(board, port_no, 1);
            }
#endif /* VTSS_ARCH_DAYTONA */
         vtss_appl_cli_task();
        } /* Port loop */
        if (!init_done) {
            init_done = 1;
#if defined(VTSS_FEATURE_WARM_START)
        /* Configuration done */
            vtss_restart_conf_end(board->inst);
#endif /* VTSS_FEATURE_WARM_START */
        }
#if defined(NPI_INTERFACE)
        npi_poll(board);
#endif /* NPI_INTERFACE */            
        if (board->board_irq_poll)
            board->board_irq_poll(board);
    } /* for (;;) loop */
    
    return 0;
}

/* ================================================================= *
 *  API functions
 * ================================================================= */

vtss_board_t *vtss_appl_board_get(vtss_appl_inst_t inst)
{
    return &board_table[inst];
}

static BOOL port_invalid(vtss_board_t *board, const vtss_port_no_t port_no)
{
    return (port_no >= board->port_count || 
            (board->port_poll != NULL && board->port_poll(port_no) == 0));
}

BOOL vtss_appl_port_is_host(const vtss_appl_inst_t inst,
                            const vtss_port_no_t port_no)
{ 
#if defined(VTSS_ARCH_B2)
    vtss_board_t     *board = &board_table[inst];
    vtss_init_conf_t init_conf;

    return (board->b2 &&
            vtss_init_conf_get(board->inst, &init_conf) == VTSS_RC_OK &&
            ((board_port_10g(board, port_no) && init_conf.host_mode < 4) ||
             board_port_spi4(board, port_no)));
#else
    return 0;
#endif /* VTSS_ARCH_B2 */
}

// Function check if the interface type is SPI4
BOOL vtss_appl_iftype_is_spi4(const vtss_appl_inst_t inst,
                              const vtss_port_no_t port_no)
{ 
    return (board_table[inst].port_interface(port_no) == VTSS_PORT_INTERFACE_SPI4);
}

// Function check if the interface type is XAUI
BOOL vtss_appl_iftype_is_xaui(const vtss_appl_inst_t inst,
                              const vtss_port_no_t port_no)
{ 
    return (board_table[inst].port_interface(port_no) == VTSS_PORT_INTERFACE_XAUI);
}

vtss_rc vtss_appl_port_status_get(const vtss_appl_inst_t inst,
                                  const vtss_port_no_t    port_no,
                                  vtss_appl_port_status_t *const status)
{
    vtss_board_t *board = &board_table[inst];
    
    if (port_invalid(board, port_no))
        return VTSS_RC_ERROR;

    *status = board->port_status[port_no];
    return VTSS_RC_OK;
}

/* Get port configuration */
vtss_rc vtss_appl_port_conf_get(const vtss_appl_inst_t inst,
                                const vtss_port_no_t          port_no,
                                vtss_appl_port_conf_t * const conf)
{
    vtss_board_t *board = &board_table[inst];
    
    if (port_invalid(board, port_no))
        return VTSS_RC_ERROR;

    *conf = board->port_conf[port_no];
    return VTSS_RC_OK;
}

/* Set port configuration */
vtss_rc vtss_appl_port_conf_set(const vtss_appl_inst_t inst,
                                const vtss_port_no_t                port_no,
                                const vtss_appl_port_conf_t * const conf)
{
    BOOL                  error = 0;
    vtss_port_interface_t if_type;
    vtss_board_t          *board = &board_table[inst];
    
    if (port_invalid(board, port_no))
        return VTSS_RC_ERROR;

    /* Host port config not supported */
    if (vtss_appl_port_is_host(inst, port_no)) {
        T_E("Not possible to configure host ports");
        error = 1;
    }
        
    /* PHY ports must run auto negotiation or 1G/100M/10M */
    if (board_port_phy(board, port_no) && conf->autoneg == 0 && 
        (conf->speed < VTSS_SPEED_10M || conf->speed > VTSS_SPEED_1G))
        error = 1;
    
    /* SerDes ports must run auto negotiation or 1G fdx */
    if_type = board->port_interface(port_no);
    if (if_type == VTSS_PORT_INTERFACE_SERDES &&
        conf->autoneg == 0 && (conf->speed != VTSS_SPEED_1G || conf->fdx == 0))
        error = 1;
    
    /* 100FX ports only support 100Mb fdx */
    if (if_type == VTSS_PORT_INTERFACE_100FX &&
        (conf->autoneg || conf->speed != VTSS_SPEED_100M || conf->fdx == 0))
        error = 1;

    /* 10G ports must run 10G/12G fdx */
    if (board_port_10g(board, port_no) && 
        (conf->autoneg || conf->fdx == 0 || 
         (conf->speed != VTSS_SPEED_10G && conf->speed != VTSS_SPEED_12G)))
         error = 1;

    if (error) {
        T_D("illegal port or mode, port_no: %u", port_no);
        return VTSS_RC_ERROR;
    }
    
    board->port_conf[port_no] = *conf;
    port_setup(board, port_no, 0);
    return VTSS_RC_OK;
}

vtss_rc vtss_appl_init_conf_get(const vtss_appl_inst_t inst,
                                vtss_init_conf_t * const conf)
{ 
    if (vtss_init_conf_get(board_table[inst].inst, conf) == VTSS_RC_ERROR) {
        T_E("Could not get init conf");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_ARCH_B2)
vtss_rc vtss_appl_host_conf_get(const vtss_appl_inst_t inst, 
                                const vtss_port_no_t port_no, 
                                vtss_host_conf_t * const conf)
{ 
    if (vtss_host_conf_get(board_table[inst].inst, port_no, conf) == VTSS_RC_ERROR) {
        T_E("Could not get host conf");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_B2 */

vtss_rc vtss_appl_port_if_get(const vtss_appl_inst_t inst,
                              const vtss_port_no_t  port_no,
                              vtss_port_interface_t *const if_type)
{
    vtss_board_t *board = &board_table[inst];
    
    if (port_invalid(board, port_no)) {
        T_D("illegal port : %u", port_no);
        return VTSS_RC_ERROR;
    }
    *if_type = board->port_interface(port_no);
    return VTSS_RC_OK;
}
