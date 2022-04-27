/*

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include <netinet/in.h>         /* for the ntohs/htons */
#include "vtss_lacp.h"
#include "vtss_lacp_private.h"	/* Dirty trick to show internal state */
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/ioctl.h>
#include <linux/socket.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

int no_lacp_trace = 1;

typedef struct virtmac {
    vtss_common_linkstate_t link_state;
    vtss_common_linkspeed_t link_speed;
    vtss_common_duplex_t    link_duplex;
    vtss_common_fwdstate_t  link_fwd;
    vtss_common_macaddr_t   link_macaddr;
    int sock;
} virtmac_t;

static virtmac_t vlink[VTSS_LACP_MAX_PORTS];
static int _common_lineno = 0;
static const char *_common_funcname = __FILE__;
static int _common_trlevel = 0;

const vtss_common_macaddr_t vtss_lacp_slowmac = { VTSS_LACP_MULTICAST_MACADDR };
const vtss_common_macaddr_t vtss_common_zeromac = { { 0, 0, 0, 0, 0, 0 } };

const char *vtss_common_str_macaddr(const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *mac)
{
    static char VTSS_COMMON_DATA_ATTRIB buf[24];

    sprintf(buf, "%02X-%02X-%02X-%02X-%02X-%02X",
            mac->macaddr[0], mac->macaddr[1], mac->macaddr[2], 
            mac->macaddr[3], mac->macaddr[4], mac->macaddr[5]);
    return buf;
}

const char *vtss_common_str_linkstate(vtss_common_linkstate_t state)
{
    switch (state) {
    case VTSS_COMMON_LINKSTATE_DOWN :
        return "down";
    case VTSS_COMMON_LINKSTATE_UP :
        return "up";
    default :
        return "Undef";
    }
}

const char *vtss_common_str_linkduplex(vtss_common_duplex_t duplex)
{
    switch (duplex) {
    case VTSS_COMMON_LINKDUPLEX_HALF :
        return "half";
    case VTSS_COMMON_LINKDUPLEX_FULL :
        return "full";
    default :
        return "undef";
    }
}

const char *vtss_common_str_linkspeed(vtss_common_linkspeed_t speed)
{
    static char buf[64];

    snprintf(buf, sizeof(buf), "%lu", speed);
    return buf;
}

const char *vtss_lacp_str_portstate(vtss_lacp_portstate_t pst)
{
    static char buf[128];

    buf[0] = '\0';
    if (pst & VTSS_LACP_PORTSTATE_LACP_ACTIVITY)
        strcat(buf, "activity ");
    if (pst & VTSS_LACP_PORTSTATE_LACP_TIMEOUT)
        strcat(buf, "timeout ");
    if (pst & VTSS_LACP_PORTSTATE_AGGREGATION)
        strcat(buf, "aggregation ");
    if (pst & VTSS_LACP_PORTSTATE_SYNCHRONIZATION)
        strcat(buf, "synchronization ");
    if (pst & VTSS_LACP_PORTSTATE_COLLECTING)
        strcat(buf, "collecting ");
    if (pst & VTSS_LACP_PORTSTATE_DISTRIBUTING)
        strcat(buf, "distributing ");
    if (pst & VTSS_LACP_PORTSTATE_DEFAULTED)
        strcat(buf, "defaulted ");
    if (pst & VTSS_LACP_PORTSTATE_EXPIRED)
        strcat(buf, "expired ");
    return buf;
}

static const char *vtss_lacp_str_sm(vtss_lacp_sm_t sm)
{
    static char buf[128];

    buf[0] = '\0';
    if (sm & VTSS_LACP_PORT_BEGIN)
        strcat(buf, "begin ");
    if (sm & VTSS_LACP_PORT_LACP_ENABLED)
        strcat(buf, "lacp_enabled ");
    if (sm & VTSS_LACP_PORT_ACTOR_CHURN)
        strcat(buf, "actor_churn ");
    if (sm & VTSS_LACP_PORT_PARTNER_CHURN)
        strcat(buf, "partner_churn ");
    if (sm & VTSS_LACP_PORT_READY)
        strcat(buf, "ready ");
    if (sm & VTSS_LACP_PORT_READY_N)
        strcat(buf, "ready_n ");
    if (sm & VTSS_LACP_PORT_MATCHED)
        strcat(buf, "matched ");
    if (sm & VTSS_LACP_PORT_STANDBY)
        strcat(buf, "standby ");
    if (sm & VTSS_LACP_PORT_SELECTED)
        strcat(buf, "selected ");
    if (sm & VTSS_LACP_PORT_MOVED)
        strcat(buf, "moved ");
    return buf;
}

static const char *vtss_lacp_str_rx_state(vtss_lacp_rx_state_t st)
{
    static const char *buf[] = {
        "undef", "initialize", "port_disabled", "lacp_disabled",
        "expired", "defaulted", "current"
    };

    return buf[st];
}

static const char *vtss_lacp_str_mux_state(vtss_lacp_mux_state_t st)
{
    static const char *buf[] = {
        "undef", "detached", "waiting", "attached", "colldist"
    };

    return buf[st];
}

static const char *vtss_lacp_str_periodic_state(vtss_lacp_periodic_state_t st)
{
    static const char *buf[] = {
        "undef", "none", "fast", "slow", "tx"
    };

    return buf[st];
}

void vtss_common_savetrace(int lvl, const char *funcname, int lineno)
{
    _common_trlevel = lvl;
    _common_funcname = funcname;
    _common_lineno = lineno;
}

void vtss_common_trace(const char *fmt, ...)
{
    char buf[256];
    va_list arg;

    va_start(arg, fmt);
    if (no_lacp_trace)
        return;
    vsnprintf(buf, sizeof(buf), fmt, arg);
    printf("%s: line %d: %c -> %s\n",
           _common_funcname, _common_lineno, "EWDN"[_common_trlevel], buf);
}

static void show_ports(void)
{
    vtss_lacp_port_vars_t *pp = &LACP->ports[0];
    int i;

    for (i = 0; i < VTSS_LACP_MAX_PORTS; i++, pp++) {
        printf("Port %2d: MAC %s Key 0x%x (0x%x) Aggregator %d\n",
               pp->actor_port_number, vtss_common_str_macaddr(&pp->port_macaddr),
               pp->actor_oper_port_key, pp->port_config.port_key,
               pp->aggregator->aggregator_identifier);
        printf("         port ntt %d state 0x%x (0x%x) = %s\n", pp->ntt,
               pp->actor_oper_port_state, pp->port_config.xmit_mode,
               vtss_lacp_str_portstate(pp->actor_oper_port_state));
        printf("         sm vars 0x%x = %s\n", pp->sm_vars,
               vtss_lacp_str_sm(pp->sm_vars));
        printf("         mux tcnt %u state 0x%x = %s\n",
               pp->sm_mux_timer_counter, pp->sm_mux_state,
               vtss_lacp_str_mux_state(pp->sm_mux_state));
        printf("         rx tcnt %u state 0x%x = %s\n",
               pp->sm_rx_timer_counter, pp->sm_rx_state,
               vtss_lacp_str_rx_state(pp->sm_rx_state));
        printf("         periodic tcnt %u state 0x%x = %s\n",
               pp->sm_periodic_timer_counter, pp->sm_periodic_state,
               vtss_lacp_str_periodic_state(pp->sm_periodic_state));
        printf("         tx tcnt %u state 0x%x\n",
               pp->sm_tx_timer_counter, pp->sm_tx_state);
/*        printf("         enabled = %d phys_enabled = %d\n",
               pp->port_enabled, pp->port_config.enable_lacp);
*/
    }
}

static void show_aggr(void)
{
    vtss_lacp_aggregator_vars_t *ap = &LACP->aggregators[0];
    vtss_lacp_port_vars_t *pp;
    int i;

    for (i = 0; i < VTSS_LACP_MAX_AGGR; i++, ap++) {
        printf("Aggr %2d: Individual %d numports %u key 0x%x (0x%x)\n",
               ap->aggregator_identifier,
               ap->is_individual, ap->num_of_ports,
               ap->actor_oper_aggregator_key, ap->actor_admin_aggregator_key);
        printf("         partner MAC %s key 0x%x prio 0x%x\n",
               vtss_common_str_macaddr(&ap->partner_system),
               ap->partner_oper_aggregator_key, ap->partner_system_priority);
        printf("         ports:");
        for (pp = ap->lag_ports; pp; pp = pp->next_lag_port)
            printf(" %d", pp->actor_port_number);
        printf("\n");
    }
}

static void show_sys(void)
{
    printf("System MAC %s prio 0x%x datasize = %u port = %u aggr = %u\n",
           vtss_common_str_macaddr(&LACP->system_config.system_id),
           LACP->system_config.system_prio,
           sizeof(vtss_lacp_vars),
           sizeof(vtss_lacp_port_vars_t), sizeof(vtss_lacp_aggregator_vars_t));
}

static void show_info(const char *title, vtss_lacp_info_t *ip)
{
    printf("%s information:\n", title);
    printf("Priority 0x%x MAC address %s key 0x%x\n",
           UNAL_NET2HOSTS(ip->system_priority), vtss_common_str_macaddr((vtss_common_macaddr_t *)ip->system_macaddr),
           UNAL_NET2HOSTS(ip->key));
    printf("Port prio 0x%x portno %u state 0x%x %s\n",
           UNAL_NET2HOSTS(ip->port_priority), UNAL_NET2HOSTS(ip->port), ip->state,
           vtss_lacp_str_portstate(ip->state));
}


static void show_vlink(void)
{
    int i;

    for (i = 0; i < VTSS_LACP_MAX_PORTS; i++)
        printf("link %d: state \"%s\" - speed %s - duplex %s\n",
               i + 1, vtss_common_str_linkstate(vlink[i].link_state),
               vtss_common_str_linkspeed(vlink[i].link_speed),
               vtss_common_str_linkduplex(vlink[i].link_duplex));
}

static void vtss_common_dump_frame(const vtss_common_octet_t VTSS_COMMON_PTR_ATTRIB *frame, vtss_common_framelen_t len)
{
#define MBUF    ((const vtss_lacp_frame_header_t *)frame)
    vtss_common_framelen_t i;

    vtss_printf("Frame len %u dst %s",
                (unsigned)len, vtss_common_str_macaddr((vtss_common_macaddr_t *)MBUF->dst_mac));
    vtss_printf(" src %s type 0x%x",
                vtss_common_str_macaddr((vtss_common_macaddr_t *)MBUF->src_mac), (unsigned)MBUF->eth_type);
    for (i = 0; i < len; i++) {
        if ((i & 0xF) == 0) {
            vtss_printf("\n%5d: ", i);
        }
        vtss_printf("%02X ", frame[i]);
    }
    vtss_printf("\n");
#undef MBUF
}

static void rpoll(vtss_common_port_t pno)
{
    fd_set readfds;
    struct timeval tv;
    int n, s, i;
    char buffer[1600];

    for (i = 0; i < 20; i++) {
        FD_ZERO(&readfds);
        s = vlink[pno - 1].sock;
        FD_SET(s, &readfds);
        timerclear(&tv);
        if ((n = select(s + 1, &readfds, NULL, NULL, &tv)) < 0) {
            vtss_printf("select(nic_fd %d) failed: %d errno %d", s, n, errno);
            return;
        }
        if (n == 0) {
            vtss_printf("Nothing to receive\n");
            return;
        }
        n = recv(s, buffer, sizeof(buffer), 0);
        if (n < 0 ) {
            vtss_printf("recv(nic_fd %d) failed: %d errno %d", s, n, errno);
            return;
        }
        if (UNAL_NET2HOSTS(&buffer[12]) == VTSS_LACP_ETHTYPE)
            vtss_printf("Delivering frame len %u to LACP\n", n);
        else {
            vtss_printf("Ignored frame len %u\n", n);
            vtss_common_dump_frame(buffer, n);
        }
        vtss_lacp_receive(pno, buffer, n);
    }
}

/* Break line up into argv */
static void parseline(char *line, char *argv[])
{
    int i = 0;

    do {
        while (*line && isspace(*line))
            line++;
        argv[i++] = line;
        while (*line && !isspace(*line))
            line++;
        if (*line)
            *line++ = '\0';
    } while (*line);
    while (i < 16)
        argv[i++] = NULL;
}
    
static int exec_line(char *cmd)
{
    char *argv[16];
    unsigned int pno, val;
        
    parseline(cmd, argv);
    printf("Command : %s %s %s %s\n",
           argv[0], argv[1], argv[2], argv[3]);
    if (strcasecmp(argv[0], "trace") == 0) {
        if (argv[1] == NULL)
            no_lacp_trace = !no_lacp_trace;
        else if (strcasecmp(argv[1], "on") == 0)
            no_lacp_trace = 0;
        else if (strcasecmp(argv[1], "off") == 0)
            no_lacp_trace = 1;
        else
            fprintf(stderr, "Wrong trace arg: \"%s\" - must be \"on\" or \"off\"\n", argv[1]);
        printf("Trace is now \"%s\"\n", no_lacp_trace ? "OFF" : "ON");
        return 1;
    }
    if (strcasecmp(argv[0], "source") == 0) {
        if (argv[1]) {
            FILE *fp = fopen(argv[1], "r");
            char buf[128];
            
            if (fp == NULL)
                fprintf(stderr, "Cannot open source file \"%s\": %s\n",
                        argv[1], strerror(errno));
            else {
                while (fgets(buf, sizeof(buf), fp) != NULL)
                    exec_line(buf);
                fclose(fp);
            }
        }
        else
            fprintf(stderr, "Missing source filename for command \"source\"\n");
        return 1;
    }
    if (strcasecmp(argv[0], "tick") == 0) {
        if (argv[1] == NULL || sscanf(argv[1], "%u", &val) != 1)
            val = 1;
        if (val > 1)
            printf("Running %u ticks\n", val);
        while (val--)
            vtss_lacp_tick();
        return 1;
    }
#if 0
    if (strcasecmp(argv[0], "frame") == 0) {
        vtss_lacp_lacpdu_t lacpdu;
        static const vtss_common_macaddr_t vtss_lacp_slowmac = { VTSS_LACP_MACADDR };

        if (argv[1] && sscanf(argv[1], "%u", &pno) == 1) {
            if (pno < 1 || pno > VTSS_LACP_MAX_PORTS) {
                fprintf(stderr, "Illegal port number %d: Must be between 1 and %d\n",
                        pno, VTSS_LACP_MAX_PORTS);
                return 1;
            }
        }
        else {
            fprintf(stderr, "Missing port number for \"frame\" command\n");
            return 1;
        }
        if (vlink[pno - 1].link_state != VTSS_COMMON_LINKSTATE_UP) {
            fprintf(stderr, "Cannot receive a frame from a port that is not UP\n");
            return 1;
        }
        memset(&lacpdu, 0, sizeof(lacpdu));
        lacpdu.frame_header.macheader.dst_mac = vtss_lacp_slowmac;
        lacpdu.frame_header.macheader.eth_type = VTSS_LACP_ETHTYPE;
        lacpdu.frame_header.subheader.subtype = VTSS_LACP_SUBTYPE_LACP;
        lacpdu.frame_header.subheader.version = VTSS_LACP_VERSION_NO;
        lacpdu.tvl_type_actor = VTSS_LACP_TVLTYPE_ACTOR_INFO;
        lacpdu.tvl_length_actor = VTSS_LACP_TVLLEN_ACTOR_INFO;
        lacpdu.tvl_type_partner = VTSS_LACP_TVLTYPE_PARTNER_INFO;
        lacpdu.tvl_length_partner = VTSS_LACP_TVLLEN_PARTNER_INFO;
        lacpdu.collector_max_delay = VTSS_LACP_COLLECTOR_MAX_DELAY;
        lacpdu.actor_info.system_priority = htons(0x1234);
        lacpdu.actor_info.system_macaddr = vtss_lacp_slowmac;
        lacpdu.actor_info.key = htons(0xDEAD);
        lacpdu.actor_info.port = htons(3);
        lacpdu.actor_info.state = 0x47;
        vtss_lacp_receive(pno, &lacpdu.frame_header, sizeof(lacpdu));
        return 1;
    }
#endif
    if (strcasecmp(argv[0], "rpoll") == 0) {
        if (argv[1] && sscanf(argv[1], "%u", &pno) == 1) {
            if (pno < 1 || pno > VTSS_LACP_MAX_PORTS) {
                fprintf(stderr, "Illegal port number %d: Must be between 1 and %d\n",
                        pno, VTSS_LACP_MAX_PORTS);
                return 1;
            }
        }
        else {
            fprintf(stderr, "Missing port number for \"rpoll\" command\n");
            return 1;
        }
        if (vlink[pno - 1].link_state != VTSS_COMMON_LINKSTATE_UP) {
            fprintf(stderr, "Cannot receive a frame from a port that is not UP\n");
            return 1;
        }
        rpoll(pno);
        return 1;
    }
    if (strcasecmp(argv[0], "show") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "Missing subcommand to \"show\"\n");
            return 1;
        }
        if (strcasecmp(argv[1], "links") == 0) {
            show_vlink();
            return 1;
        }
        if (strcasecmp(argv[1], "ports") == 0) {
            show_ports();
            return 1;
        }
        if (strcasecmp(argv[1], "aggr") == 0) {
            show_aggr();
            return 1;
        }
        if (strcasecmp(argv[1], "sys") == 0) {
            show_sys();
            return 1;
        }
        if (strcasecmp(argv[1], "all") == 0) {
            show_vlink();
            show_ports();
            show_aggr();
            show_aggr();
            return 1;
        }
        fprintf(stderr, "Unknown show sub command \"%s\".\n", argv[1]);
        return 1;
    }
    if (strcasecmp(argv[0], "link") == 0) {
        if (argv[1] && sscanf(argv[1], "%u", &pno) == 1) {
            if (pno < 1 || pno > VTSS_LACP_MAX_PORTS) {
                fprintf(stderr, "Illegal link number %d: Must be between 1 and %d\n",
                        pno, VTSS_LACP_MAX_PORTS);
                return 1;
            }
        }
        else {
            fprintf(stderr, "Missing link number for \"link\" command\n");
            return 1;
        }
        if (argv[2] == NULL || argv[3] == NULL) {
            fprintf(stderr, "Missing subcommand for \"link\" command\n");
            return 1;
        }
        if (strcasecmp(argv[2], "speed") == 0) {
            if (sscanf(argv[3], "%u", &val) == 1) {
                vlink[pno - 1].link_speed = val;
            }
            else
                fprintf(stderr, "Illegal speed value \"%s\": Decimal number\n", argv[3]);
            return 1;
        }
            
        if (strcasecmp(argv[2], "duplex") == 0) {
            if (strcasecmp(argv[3], "full") == 0)
                vlink[pno - 1].link_duplex = VTSS_COMMON_LINKDUPLEX_FULL;
            else if (strcasecmp(argv[3], "half") == 0)
                vlink[pno - 1].link_duplex = VTSS_COMMON_LINKDUPLEX_HALF;
            else
                fprintf(stderr, "Illegal duplex value \"%s\": Must be \"half\" or \"full\"\n", argv[3]);
            return 1;
        }
        if (strcasecmp(argv[2], "state") == 0) {
            if (strcasecmp(argv[3], "up") == 0)
                vlink[pno - 1].link_state = VTSS_COMMON_LINKSTATE_UP;
            else if (strcasecmp(argv[3], "down") == 0)
                vlink[pno - 1].link_state = VTSS_COMMON_LINKSTATE_DOWN;
            else
                fprintf(stderr, "Illegal state value \"%s\": Must be \"up\" or \"down\"\n", argv[3]);
            vtss_lacp_linkstate_changed(pno, vlink[pno - 1].link_state);
            return 1;
        }
        if (strcasecmp(argv[2], "activity") == 0) {
            vtss_lacp_port_config_t pconf;

            vtss_lacp_get_portconfig(pno, &pconf);
            if (strcasecmp(argv[3], "passive") == 0) {
                pconf.active_or_passive = VTSS_LACP_ACTMODE_PASSIVE;
                vtss_lacp_set_portconfig(pno, &pconf);
            }
            else if (strcasecmp(argv[3], "active") == 0) {
                pconf. active_or_passive = VTSS_LACP_ACTMODE_ACTIVE;
                vtss_lacp_set_portconfig(pno, &pconf);
            }
            else
                fprintf(stderr, "Illegal activity value \"%s\": Must be \"passive\" or \"active\"\n",
                        argv[3]);
            return 1;
        }
        if (strcasecmp(argv[2], "lacp") == 0) {
            vtss_lacp_port_config_t pconf;

            vtss_lacp_get_portconfig(pno, &pconf);
            if (strcasecmp(argv[3], "enable") == 0) {
                pconf.enable_lacp = VTSS_COMMON_BOOL_TRUE;
                vtss_lacp_set_portconfig(pno, &pconf);
            }
            else if (strcasecmp(argv[3], "disable") == 0) {
                pconf.enable_lacp = VTSS_COMMON_BOOL_FALSE;
                vtss_lacp_set_portconfig(pno, &pconf);
            }
            else
                fprintf(stderr, "Illegal lacp value \"%s\": Must be \"enable\" or \"disable\"\n", argv[3]);
            return 1;
        }
        fprintf(stderr, "Unknown link command: %s\n", argv[2]);
        return 1;
    }
    if (strcasecmp(argv[0], "quit") == 0)
        return 0;
    fprintf(stderr, "Unknown command \"%s\". Known commands are:\n \"tick [count]\" - Simulate timertick\n \"frame\" - Simulate reception of frame\n \"source filename\" - Read command from filename\n \"link pno state|speed|duplex|prio|key|activity|lacp val\" - Set a specific link state or speed or duplex\n \"rpoll pno\" - Poll for frames received.\n \"quit\" - terminate program.\n", cmd);
    return 1;
}

static int linux_eth(const char *ethname, unsigned char *macaddr)
{
    int s, r, eno;
    struct ifreq ifr;

    r = -1;
    s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s < 0) {
        perror("socket(PF_PACKET)");
        return r;
    }
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
    if (ioctl(s, SIOCGIFHWADDR, &ifr) >= 0) {
        memcpy(macaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
        printf("Reading MAC %s from %s\n",
               vtss_common_str_macaddr((vtss_common_macaddr_t *)macaddr), ethname);
        strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
        if (ioctl(s, SIOCGIFINDEX, &ifr) >= 0) {
            r = ifr.ifr_ifindex;
            printf("Reading index %d from %s\n",
                        r, ethname);
            strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
            if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) {
                perror("SIOCGIFFLAGS");
                r = -1;
            }
            else {
                strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
                ifr.ifr_flags |= IFF_PROMISC;
                if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) {
                    perror("SIOCSIFFLAGS");
                    r = -1;
                }
                else
                    vtss_printf("Setting promisc mode for %s\n", ethname);
            }
        }
        else
            vtss_printf("SIOCGIFINDEX failed: errno %d \"%s\"\n", errno, strerror(errno));
    }
    else
        vtss_printf("SIOCGIFHWADDR failed: errno %d \"%s\"\n", errno, strerror(errno));
    eno = errno;
    close(s);
    errno = eno;
    return r;
}

int main(int ac, char *av[])
{
    vtss_common_port_t pix;
    char *cmd = NULL;
    struct sockaddr_ll saddr;
    const char *ifnm;
    int s;

    s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s < 0) {
        perror("socket");
        return 1;
    }
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_hatype = htons(ARPHRD_ETHER);
    saddr.sll_pkttype = PACKET_OTHERHOST;
    saddr.sll_halen = ETH_ALEN;
    ifnm = ac > 1 ? av[1] : "eth1";
    saddr.sll_ifindex = linux_eth(ifnm, saddr.sll_addr);
    if (bind(s, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        vtss_printf("bind(PF_PACKET, ifindex %d/%s): %s\n",
                    saddr.sll_ifindex, ifnm, strerror(errno));
        return 2;
    }
    for (pix = 0; pix < VTSS_LACP_MAX_PORTS; pix++) {
        vlink[pix].link_state = VTSS_COMMON_LINKSTATE_DOWN;
        vlink[pix].link_speed = 0;
        vlink[pix].link_duplex = VTSS_COMMON_LINKDUPLEX_FULL;
        vlink[pix].link_fwd = VTSS_COMMON_FWDSTATE_ENABLED;
        vlink[pix].sock = s;
        VTSS_COMMON_MACADDR_ASSIGN(vlink[pix].link_macaddr.macaddr, saddr.sll_addr);
    }
    vtss_lacp_init();
    do {
        if (cmd)
            free(cmd);
        cmd = readline("Enter command : ");
        if (cmd == NULL)        /* EOF */
            break;
        if (*cmd == '\0')       /* Ignore empty lines */
            continue;
        add_history(cmd);
    } while (exec_line(cmd));
    fprintf(stderr, "\nTerminated normally.\n");
    return 0;
}

/* -------------------------------------------------------------------------------- */

/**
 * vtss_os_get_linkstate - Return link state for a given physical port
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_common_linkstate_t vtss_os_get_linkstate(vtss_common_port_t portno)
{
    return vlink[portno - 1].link_state;
}

/**
 * vtss_os_get_linkduplex - Return link duplex mode for a given physical port
 * If link state is "down" the duplexmode is returned as VTSS_LACP_LINKDUPLEX_HALF.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_common_duplex_t vtss_os_get_linkduplex(vtss_common_port_t portno)
{
    if (vlink[portno - 1].link_state == VTSS_COMMON_LINKSTATE_UP)
        return vlink[portno - 1].link_duplex;
    return VTSS_COMMON_LINKDUPLEX_HALF;
}

/**
 * vtss_os_set_fwdstate - Set the forwarding state of a specific port.
 */
void vtss_os_set_fwdstate(vtss_common_port_t portno, vtss_common_fwdstate_t new_state)
{
    vlink[portno - 1].link_fwd = new_state;
}

/**
 * vtss_os_get_fwdstate - Get the forwarding state of a specific port.
 */
vtss_common_fwdstate_t vtss_os_get_fwdstate(vtss_common_port_t portno)
{
    return vlink[portno - 1].link_fwd;
}

/**
 * vtss_os_make_key - Return the operational key given physical port and the admin key.
 * This value should be the same for ports that can physically aggregate together.
 * An obvious candidate is the link speed.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @new_key: The admin value of the new key. By convention VTSS_LACP_AUTOKEY means that
 *           the switch is free to choose the key.
 *
 */
vtss_lacp_key_t vtss_os_make_key(vtss_common_port_t portno, vtss_lacp_key_t new_key)
{
    if (new_key == VTSS_LACP_AUTOKEY)
        /* We don't really care what the value is when the link is down */
        return (vtss_lacp_key_t)vlink[portno - 1].link_speed;
    return new_key;
}

/**
 * vtss_os_get_systemmac - Return MAC address associated with the system.
 * @system_macaddr: Return the value.
 *
 */
void vtss_os_get_systemmac(vtss_common_macaddr_t *system_macaddr)
{
    *system_macaddr = vlink[0].link_macaddr;
}

/**
 * vtss_os_get_portmac - Return MAC address associated a specific physical port.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @port_macaddr: Return the value.
 *
 */
void vtss_os_get_portmac(vtss_common_port_t portno, vtss_common_macaddr_t *port_macaddr)
{
    *port_macaddr = vlink[portno - 1].link_macaddr;
}

/**
 * vtss_os_alloc_xmit - Return a pointer to a buffer that can be used for
 * transmitting a frame of length <len> on port <portno>.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @len: Length of frame to transmitted.
 * @pbuf: OS specific reference to the buffer.
 * RETURN: Pointer to the for available data byte.
 *
 */
void VTSS_COMMON_BUFMEM_ATTRIB *vtss_os_alloc_xmit(vtss_common_port_t portno, vtss_common_framelen_t len, vtss_common_bufref_t *pbufref)
{
    void *p;

    p = malloc(len);
    *pbufref = p;
    return p;
}

/**
 * vtss_os_send - Transmit a MAC frame on a specific physical port and
 * free the frame buffer.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @frame: A full MAC frame.
 * @len: The length of the MAC frame.
 * @bufref: The value returned from the vtss_os_alloc_xmit() call for
 * this buffer.
 *
 * Return: VTSS_LACP_CC_OK if frame was successfully queued for transmission.
 *         VTSS_LACP_CC_GENERR in case of error.
 */
int vtss_os_xmit(vtss_common_port_t portno, void VTSS_COMMON_BUFMEM_ATTRIB *frame, vtss_common_framelen_t len, vtss_common_bufref_t bufref)
{
    int n;
    vtss_lacp_lacpdu_t *lacpdu = (vtss_lacp_lacpdu_t *)frame;
    vtss_lacp_frame_header_t *fhead = (vtss_lacp_frame_header_t *)lacpdu->frame_header;
    
    printf("Xmit frame on port %d (state \"%s\" - speed %s - duplex %s):\n",
           portno, vtss_common_str_linkstate(vlink[portno - 1].link_state),
           vtss_common_str_linkspeed(vlink[portno - 1].link_speed),
           vtss_common_str_linkduplex(vlink[portno - 1].link_duplex));
    printf("Frame dst = %s ", vtss_common_str_macaddr((vtss_common_macaddr_t *)fhead->dst_mac));
    printf("src = %s ethtype = 0x%04X\n",
           vtss_common_str_macaddr((vtss_common_macaddr_t *)fhead->src_mac),
           UNAL_NET2HOSTS(fhead->eth_type));
    printf("subtype = %d version = %d\n", fhead->subtype, fhead->version);
    show_info("Actor", (vtss_lacp_info_t *)lacpdu->actor_info);
    show_info("Partner", (vtss_lacp_info_t *)lacpdu->partner_info);
    n = send(vlink[portno - 1].sock, frame, len, 0);
    free(bufref);
    return VTSS_COMMON_CC_OK;
}

/**
 * vtss_os_set_hwaggr - Add a specified port to an existing set of ports for aggregation.
 * @aid: The aggregation group id for the new port (1 - VTSS_LACP_MAX_AGGR)
 * @new_port: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
void vtss_os_set_hwaggr(vtss_lacp_agid_t aid,
                        vtss_common_port_t new_port)
{
    printf("Setting new port %u in aggregate %u\n", new_port, aid);
}

/**
 * vtss_os_clear_hwaggr - Remove a specified port from an existing set of ports for aggregation.
 * @old_port: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
void vtss_os_clear_hwaggr(vtss_lacp_agid_t aid,
                          vtss_common_port_t old_port)
{
    printf("Clearing old port %u from aggregate\n", old_port);
}
