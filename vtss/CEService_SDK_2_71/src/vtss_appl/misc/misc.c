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
 
 $Id$
 $Revision$

*/

#include "main.h"
#include "misc_api.h"
#include "critd_api.h"
#include "msg_api.h"
#include "vtss_api_if_api.h"
#include "misc.h"
#include "version.h"
#include "port_api.h"

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif

#include <network.h>
#include <netinet/in.h>

#include <net/if_var.h>
#include <netinet6/in6_var.h>
#include <netinet6/in6_ifattach.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/nd6.h>
#include <netinet/icmp6.h>
#include <netinet6/scope6_var.h>

#ifdef VTSS_SW_OPTION_VCLI
#include "misc_cli.h"
#endif
#ifdef VTSS_SW_OPTION_PHY
#include "phy_api.h"
#endif

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
#include "icfg_api.h"
#endif

#include "mgmt_api.h" // For mgmt_txt2ipv4

static misc_global_t misc;

static const vtss_software_type_t software_type = VTSS_SW_ID;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "misc",
    .descr     = "Miscellaneous Control"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = { 
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Stack functions                                                         */
/****************************************************************************/

/* Allocate message buffer */
static misc_msg_t *misc_msg_alloc(misc_msg_buf_t *buf, misc_msg_id_t msg_id)
{
    buf->sem = &misc.sem;
    buf->msg = &misc.msg;
    VTSS_OS_SEM_WAIT(buf->sem);
    buf->msg->msg_id = msg_id;
    return buf->msg;
}

static void misc_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    VTSS_OS_SEM_POST(contxt);
}

static void misc_msg_tx(misc_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(buf->sem, misc_msg_tx_done, MSG_TX_OPT_DONT_FREE, 
               VTSS_MODULE_ID_MISC, isid, buf->msg, len + 
               MSG_TX_DATA_HDR_LEN(misc_msg_t, data));
}

static BOOL misc_msg_rx(void *contxt, const void * const rx_msg, const size_t len, 
                        const vtss_module_id_t modid, const ulong isid)
{
    misc_msg_t     *msg = (misc_msg_t *)rx_msg;
    misc_msg_buf_t buf;
    
    switch (msg->msg_id) {
    case MISC_MSG_ID_REG_READ_REQ:
    {
        vtss_chip_no_t chip_no = msg->data.reg.chip_no;
        ulong          addr = msg->data.reg.addr;
        
        T_D("REG_READ_REQ, isid: %d, chip_no: %u, addr: 0x%08x", isid, chip_no, addr);
        msg = misc_msg_alloc(&buf, MISC_MSG_ID_REG_READ_REP);
        msg->data.reg.addr = addr;
        vtss_reg_read(NULL, chip_no, addr, &msg->data.reg.value);
        misc_msg_tx(&buf, isid, sizeof(msg->data.reg));
        break;
    }
    case MISC_MSG_ID_REG_WRITE_REQ:
    {
        vtss_chip_no_t chip_no = msg->data.reg.chip_no;
        ulong          addr = msg->data.reg.addr;
        ulong          value = msg->data.reg.value;

        T_D("REG_WRITE_REQ, isid: %d, chip_no: %u, addr: 0x%08x, value: 0x%08x", 
            isid, chip_no, addr, value);
        vtss_reg_write(NULL, chip_no, addr, value);
        break;
    }
    case MISC_MSG_ID_REG_READ_REP:
    {
        vtss_chip_no_t chip_no = msg->data.reg.chip_no;
        ulong          addr = msg->data.reg.addr;
        ulong          value = msg->data.reg.value;

        T_D("REG_READ_REP, isid: %d, chip_no: %u, addr: 0x%08x, value: 0x%08x", 
            isid, chip_no, addr, value);
        misc.value = value;
        cyg_flag_setbits(&misc.flags, MISC_FLAG_READ_DONE);
        break;
    }
    case MISC_MSG_ID_PHY_READ_REQ:
    {
        vtss_port_no_t port_no = msg->data.phy.port_no;
        uint           addr = msg->data.phy.addr;
        
        T_D("PHY_READ_REQ, isid: %d, port_no: %u, addr: 0x%08x", isid, port_no, addr);
        msg = misc_msg_alloc(&buf, MISC_MSG_ID_PHY_READ_REP);
        msg->data.phy.port_no = port_no;
        msg->data.phy.addr = addr;

        if (msg->data.phy.mmd_access) {
            if (port_phy(port_no)) {
                vtss_phy_mmd_read(PHY_INST, port_no, msg->data.phy.devad, addr, &msg->data.phy.value) ;
#if defined(VTSS_FEATURE_10G)
            } else {
                vtss_port_mmd_read(NULL, port_no, msg->data.phy.devad, addr, &msg->data.phy.value) ;
#endif /* VTSS_FEATURE_10G */
            }
        } else {
            vtss_phy_read(PHY_INST, port_no, addr, &msg->data.phy.value);
        }
        misc_msg_tx(&buf, isid, sizeof(msg->data.phy));
        break;
    }
    case MISC_MSG_ID_PHY_WRITE_REQ:
    {
        vtss_port_no_t port_no = msg->data.phy.port_no;
        uint           addr = msg->data.phy.addr;
        ushort         value = msg->data.phy.value;

        T_D("PHY_WRITE_REQ, isid: %d, port_no: %u, addr: 0x%08x, value: %04x", 
            isid, port_no, addr, value);
        vtss_phy_write(PHY_INST, port_no, addr, value);
        break;
    }
    case MISC_MSG_ID_PHY_READ_REP:
    {
        vtss_port_no_t port_no = msg->data.phy.port_no;
        uint           addr = msg->data.phy.addr;
        ushort         value = msg->data.phy.value;

        T_D("PHY_READ_REP, isid: %d, port_no: %u, addr: 0x%08x, value: %04x", 
            isid, port_no, addr, value);
        misc.value = msg->data.phy.value;
        cyg_flag_setbits(&misc.flags, MISC_FLAG_READ_DONE);
        break;
    }
    case MISC_MSG_ID_SUSPEND_RESUME:
    {
        vtss_init_data_t data;
        BOOL             resume = msg->data.suspend_resume.resume;
        
        T_D("SUSPEND_RESUME, isid: %d, resume: %d", isid, resume);
        data.cmd = INIT_CMD_SUSPEND_RESUME;
        data.resume = resume;
        init_modules(&data);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg->msg_id);
        break;
    }
    return TRUE;
}

/* Determine if ISID must be accessed locally */
static BOOL misc_stack_local(vtss_isid_t isid)
{
    return (isid == VTSS_ISID_LOCAL || !msg_switch_is_master() || msg_switch_is_local(isid));
}

/* Determine if ISID is ready and read operation idle */
static BOOL misc_stack_ready(vtss_isid_t isid, BOOL write)
{
    return (msg_switch_exists(isid) &&
            (write || cyg_flag_poll(&misc.flags, MISC_FLAG_READ_IDLE, 
                                    CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR)));
}

/* Wait for read response */
static void misc_stack_read(ulong *value)
{
    cyg_tick_count_t time;

    /* Wait for DONE event */
    time = cyg_current_time() + VTSS_OS_MSEC2TICK(5 * 1000);
    cyg_flag_timed_wait(&misc.flags, MISC_FLAG_READ_DONE, 
                        CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, time);
    *value = misc.value;
    cyg_flag_setbits(&misc.flags, MISC_FLAG_READ_IDLE);
}

/* Read/write register from ISID */
static vtss_rc misc_stack_reg(vtss_isid_t isid, vtss_chip_no_t chip_no, ulong addr, ulong *value, BOOL write)
{
    vtss_rc          rc = VTSS_OK;
    misc_msg_buf_t   buf;
    misc_msg_t       *msg;
    
    if (misc_stack_local(isid)) {
        /* Local access */
        if (write) {
            rc = vtss_reg_write(NULL, chip_no, addr, *value);
        } else {
            rc = vtss_reg_read(NULL, chip_no, addr, value);
        }
    } else if (misc_stack_ready(isid, write)) {
        /* Stack access */
        msg = misc_msg_alloc(&buf, write ? MISC_MSG_ID_REG_WRITE_REQ : 
                             MISC_MSG_ID_REG_READ_REQ);
        msg->data.reg.chip_no = chip_no;
        msg->data.reg.addr = addr;
        msg->data.reg.value = *value;
        misc_msg_tx(&buf, isid, sizeof(msg->data.reg));
        if (write) {
            T_D("REG_WRITE_REQ, isid: %d, chip_no: %u, addr: 0x%08x, value: 0x%08x", 
                isid, chip_no, addr, *value);
        } else {
            T_D("REG_READ_REQ, isid: %d, chip_no: %u, addr: 0x%08x", isid, chip_no, addr);
            misc_stack_read(value);
        }
    }
    return rc;
}

/* Read/write PHY register from ISID */
static vtss_rc misc_stack_phy(vtss_isid_t isid, vtss_port_no_t port_no, 
                              uint reg, uint page, ushort *value, BOOL write, BOOL mmd_access, ushort devad)
{
    vtss_rc          rc = VTSS_OK;
    misc_msg_buf_t   buf;
    misc_msg_t       *msg;
    uint             addr;
    ulong            val;

    addr = ((page<<5) | reg);
    if (misc_stack_local(isid)) {
        /* Local access */
        if (write) {
            if (mmd_access) {
                if (port_phy(port_no)) {
                    rc = vtss_phy_mmd_write(PHY_INST, port_no, devad, addr, *value);
#if defined(VTSS_FEATURE_10G)
                } else {
                    rc = vtss_port_mmd_write(NULL, port_no, devad, reg, *value);
#endif /* VTSS_FEATURE_10G */
                }
            } else {
                rc = vtss_phy_write(PHY_INST, port_no, addr, *value);
            }
        } else {
            if (mmd_access) {
                if (port_phy(port_no)) {
                    rc = vtss_phy_mmd_read(PHY_INST, port_no, devad, addr, value);
#if defined(VTSS_FEATURE_10G)
                } else {                    
                    rc = vtss_port_mmd_read(NULL, port_no, devad, reg, value);
#endif /* VTSS_FEATURE_10G */
                }
            } else {
                rc = vtss_phy_read(PHY_INST, port_no, addr, value);
            }
        }
    } else if (misc_stack_ready(isid, write)) {
        /* Stack access */
        msg = misc_msg_alloc(&buf, write ? MISC_MSG_ID_PHY_WRITE_REQ : 
                             MISC_MSG_ID_PHY_READ_REQ);
        msg->data.phy.port_no = port_no;
        msg->data.phy.addr = port_phy(port_no) ? addr : reg;
        msg->data.phy.value = *value;
        msg->data.phy.mmd_access = mmd_access;
        msg->data.phy.devad = devad;
        misc_msg_tx(&buf, isid, sizeof(msg->data.phy));
        if (write) {
            T_D("PHY_WRITE_REQ, isid: %d, port_no: %u, addr: 0x%08x, value: 0x%04x", 
                isid, port_no, addr, *value);
        } else {
            T_D("PHY_READ_REQ, isid: %d, port_no: %u, addr: 0x%08x", isid, port_no, addr);
            misc_stack_read(&val);
            *value = val;
        }
    }
    return rc;
}

static vtss_rc misc_stack_register(void)
{
    msg_rx_filter_t filter;    

    memset(&filter, 0, sizeof(filter));
    filter.cb = misc_msg_rx;
    filter.modid = VTSS_MODULE_ID_MISC;
    return msg_rx_filter_register(&filter);
}

/****************************************************************************/
/*  Debug register and PHY access                                           */
/****************************************************************************/

/* Get chip number */
vtss_rc misc_chip_no_get(vtss_chip_no_t *chip_no)
{
    *chip_no = misc.chip_no;
    return VTSS_RC_OK;
}

/* Set chip number */
vtss_rc misc_chip_no_set(vtss_chip_no_t chip_no)
{
    if (chip_no == VTSS_CHIP_NO_ALL || chip_no < vtss_api_if_chip_count()) 
        misc.chip_no = chip_no;
    return VTSS_RC_OK;
}

#if defined(VTSS_ARCH_LUTON28)
/* Convert (blk, sub, addr) to 32-bit address */
ulong misc_l28_reg_addr(uint blk, uint sub, uint addr)
{
    return ((blk<<12) | (sub<<8) | addr);
}
#endif /* VTSS_ARCH_LUTON28 */

/* Read switch chip register */
vtss_rc misc_debug_reg_read(vtss_isid_t isid, vtss_chip_no_t chip_no, ulong addr, ulong *value)
{
    *value = 0xffffffff;
    return misc_stack_reg(isid, chip_no, addr, value, 0);
}

/* Write switch chip register */
vtss_rc misc_debug_reg_write(vtss_isid_t isid, vtss_chip_no_t chip_no, ulong addr, ulong value)
{
    return misc_stack_reg(isid, chip_no, addr, &value, 1);
}

/* Read PHY register */
vtss_rc misc_debug_phy_read(vtss_isid_t isid, 
                            vtss_port_no_t port_no, uint reg, uint page, ushort *value, BOOL mmd_access, ushort devad)
{
    *value = 0xffff;
    return misc_stack_phy(isid, port_no, reg, page, value, 0, mmd_access, devad);
}

/* Write PHY register */
vtss_rc misc_debug_phy_write(vtss_isid_t isid, 
                             vtss_port_no_t port_no, uint reg, uint page, ushort value, BOOL mmd_access, ushort devad)
{
    return misc_stack_phy(isid, port_no, reg, page, &value, 1, mmd_access, devad);
}

/* Suspend/resume */
vtss_rc misc_suspend_resume(vtss_isid_t isid, BOOL resume)
{
    misc_msg_buf_t   buf;
    misc_msg_t       *msg;
    vtss_init_data_t data;
    
    if (misc_stack_local(isid)) {
        /* Local switch */
        data.cmd = INIT_CMD_SUSPEND_RESUME;
        data.resume = resume;
        init_modules(&data);
    } else {
        msg = misc_msg_alloc(&buf, MISC_MSG_ID_SUSPEND_RESUME);
        msg->data.suspend_resume.resume = resume;
        misc_msg_tx(&buf, isid, sizeof(msg->data.suspend_resume));
        if (!resume) /* Wait for suspend */
            VTSS_OS_MSLEEP(1000);
    }
    return VTSS_OK;
}

vtss_inst_t misc_phy_inst_get(void)
{
    return misc.phy_inst;
}
vtss_rc misc_phy_inst_set(vtss_inst_t inst)
{
    misc.phy_inst = inst;
    return VTSS_RC_OK;
}

/****************************************************************************/
/*  String conversions                                                      */
/****************************************************************************/

/* strip leading path from file */
const char *misc_filename(const char *fn)
{
    if(!fn)
        return NULL;
    int i, start;
    for (start = 0, i = strlen(fn); i > 0; i--) {
        if (fn[i-1] == '/') {
            start = i;
            break;
        }
    }
    return fn+start;
}

/* MAC address text string */
char *misc_mac_txt(const uchar *mac, char *buf)
{
    sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return buf;
}

/* MAC to string */
const char *misc_mac2str(const uchar *mac)
{
    static char buf[6*3];
    return misc_mac_txt(mac, buf);
}

/* Create an instantiated MAC address based on base MAC and instance number */
void misc_instantiate_mac(uchar *mac, const uchar *base, int instance)
{
    int i;
    ulong x;
    for (i = 0; i < 6; i++)
        mac[i] = base[i];
    x = (((mac[3]<<16) | (mac[4]<<8) | mac[5]) + instance);
    mac[3] = ((x>>16) & 0xff);
    mac[4] = ((x>>8) & 0xff);
    mac[5] = (x & 0xff);
}

/* IPv4 address text string */
char *misc_ipv4_txt(vtss_ipv4_t ipv4, char *buf)
{
    sprintf(buf, "%d.%d.%d.%d", (ipv4 >> 24) & 0xff, (ipv4 >> 16) & 0xff, (ipv4 >> 8) & 0xff, ipv4 & 0xff);
    return buf;
}

/* IPv6 address text string */
char *misc_ipv6_txt(const vtss_ipv6_t *ipv6, char *buf)
{
    struct in6_aliasreq ifra6;
    struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&ifra6.ifra_addr;
    int    i;

    memset(buf, 0, 40 * sizeof(char));
    for(i = 0; i < 16; i++) 
        addr6->sin6_addr.__u6_addr.__u6_addr8[i] = ipv6->addr[i];
    inet_ntop(AF_INET6, (void *)&ifra6.ifra_addr.sin6_addr, buf, 40 * sizeof(char));
    return buf;
}

/* IPv4 or IPv6 to string */
char *misc_ip_txt(vtss_ip_addr_t *ip_addr, char *buf)
{
    switch (ip_addr->type) {
    case VTSS_IP_TYPE_IPV4:
        return misc_ipv4_txt(ip_addr->addr.ipv4, buf);

    case VTSS_IP_TYPE_IPV6:
        return misc_ipv6_txt(&ip_addr->addr.ipv6, buf);

    default:
        break;
    }

    return "<none>";
}

/* IPv4/v6 network text string. Give about 64 bytes. */
char *misc_ipaddr_txt(char *buf, size_t bufsize, vtss_ip_addr_t *addr, vtss_prefix_size_t prefix_size)
{
    char temp[64];
    switch (addr->type) {
    case VTSS_IP_TYPE_IPV4:
        misc_ipv4_txt(addr->addr.ipv4, temp);
        snprintf(buf, bufsize, "%s/%d", temp, prefix_size);
        break;
    case VTSS_IP_TYPE_IPV6:
        misc_ipv6_txt(&addr->addr.ipv6, temp);
        snprintf(buf, bufsize, "%s/%d", temp, prefix_size);
        break;
    default:
        strncpy(buf, "<unknown>", bufsize);
        break;
    }
    return buf;
}

/* IP address text string - network order */
const char *misc_ntoa(ulong ip)
{
    struct in_addr ina;
    ina.s_addr = ip;
    return inet_ntoa(ina);
}

/* IP address text string - host order */
const char *misc_htoa(ulong ip)
{
    return misc_ntoa(htonl(ip));
}

/* Time to Interval (string) */
const char *misc_time2interval(time_t time)
{
    static char buf[sizeof "999d 99:99:99"];
    int days, hrs, mins;

#define SECS_DAY  (60 * 60 * 24)
#define SECS_HOUR (60 * 60 * 1)
#define SECS_MIN  (60)

    if(time > 0) {
        days = time / SECS_DAY;
        time %= SECS_DAY;
        hrs = time / SECS_HOUR;
        time %= SECS_HOUR;
        mins = time / SECS_MIN;
        time %= SECS_MIN;
        
        snprintf(buf, sizeof(buf), 
                 "%3dd %02d:%02d:%02d",
                 days, hrs, mins, time);
        
        return buf;
    }
    return "Never";
}

/* Time to string
   Whereas RFC3339 makes allowances for multiple syntaxes,
   here lists an example that we used now.

   2033-08-24T05:14:15-07:00

   This represents 24 August 2003 at 05:14:15.  The timestamp indicates that its
   local time is -7 hours from UTC.  This timestamp might be created in
   the US Pacific time zone during daylight savings time.
*/
const char *misc_time2str_r(time_t time, char *buf)
{
    if(time >= 0) {
        struct tm *timeinfo_p;
        int       tz_off;  
#ifdef VTSS_SW_OPTION_SYSUTIL
        tz_off = system_get_tz_off();
        time += (tz_off * 60); /* Adjust for TZ minutes => seconds*/
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
        time += (time_dst_get_offset() * 60); /* Correct for DST */
#endif
        timeinfo_p = localtime(&time);
        sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                 timeinfo_p->tm_year+1900, 
                 timeinfo_p->tm_mon+1,
                 timeinfo_p->tm_mday,
                 timeinfo_p->tm_hour,
                 timeinfo_p->tm_min,
                 timeinfo_p->tm_sec,
                 tz_off >= 0 ? '+' : '-',
                 tz_off >= 0 ? tz_off / 60 : (~tz_off + 1) / 60,
                 tz_off >= 0 ? tz_off % 60 : (~tz_off + 1) % 60);
#else
        timeinfo_p = localtime(&time);
        snprintf(buf, sizeof(buf)-1, "%04d-%02d-%02dT%02d:%02d:%02d%s", 
                 timeinfo_p->tm_year+1900, 
                 timeinfo_p->tm_mon+1,
                 timeinfo_p->tm_mday,
                 timeinfo_p->tm_hour,
                 timeinfo_p->tm_min,
                 timeinfo_p->tm_sec,
                 "+0000");
#endif
    } else {
        strcpy(buf, "-");
    }

    return buf;
}

const char *misc_time2str(time_t time)
{
    static char buf[MISC_RFC3339_TIME_STR_LEN];
    return misc_time2str_r(time, buf);
}

/* engine ID to string */
const char *misc_engineid2str(const uchar *engineid, ulong engineid_len)
{
    static char buf[64 + 1];
    int i, j;

    if (!engineid || engineid_len > 64)
        return "fail";

    memset(buf, 0x0, sizeof(buf));
    for (i = 0, j = 0; i <engineid_len; i++) {
        if (!engineid[i]) {
            sprintf(buf + j, "00");
        }  else if (engineid[i] <= 0xf) {
            sprintf(buf + j, "0%x", engineid[i]);
        } else {
            sprintf(buf + j, "%x", engineid[i]);
        }
        j = strlen(buf);
    }

    return buf;
}

/* OID to string.
  'oid_mask' is a list with 8 bits of hex octets.
  'oid_mask_len' is the total mask len.
  For example:
  oid = {.1.3.6.1.2.1.2.2.1.1.1},
  oid_len = 11
  oid_mask = {0xFF, 0xA0}
  oid_mask_len = 11;
  ---> The output is .1.3.6.1.2.1.2.2.1.*.1.

  Note1: The value of 'oid_len' and 'oid_mask_len' should be not great than 128.
  Note2: The default output will exclude the character '*' when oid_mask = NULL
 */
const char *misc_oid2str(const ulong *oid, ulong oid_len, const uchar *oid_mask, ulong oid_mask_len)
{
    static char buf[128 * 2 + 1];
    int i, j;
    int mask = 0x80, maskpos = 0;

    if (!oid || oid_len > 128 || oid_mask_len > 128)
        return "fail";

    memset(buf, 0x0, sizeof(buf));
    for (i = 0, j = 0; i < oid_len; i++) {
        if (oid_mask == NULL || (oid_mask[maskpos] & mask) != 0) {
            sprintf(buf + j, ".%d", oid[i]);
        } else {
            sprintf(buf + j, ".*");
        }
        j = strlen(buf);

        if (mask == 1) {
            mask = 0x80;
            maskpos++;
        } else {
            mask >>= 1;
        }
    }

    return buf;
}

/* OID mask to string.
  'oid_mask' is a list with 8 bits of hex octets.
  'oid_mask_len' is the total mask len.
  For example:
  oid_mask = {0xF0, 0xFF}
  oid_mask_len = 9;
  ---> The output is .F0.80.

  Note: The value of 'oid_mask_len' should be not great than 128.
 */
const char *misc_oidmask2str(const uchar *oid_mask, ulong oid_mask_len)
{
    static char buf[128 * 2 + 1];
    int i, j, k, cnt;

    if (!oid_mask || oid_mask_len > 128)
        return "fail";

    memset(buf, 0x0, sizeof(buf));
    cnt = oid_mask_len / 8 + 1;
    for (i = 0, j = 0; i < cnt; i++) {
        if (i == (cnt - 1) && (oid_mask_len % 8)) {
            int mask = 0;
            for (k = 0; k < oid_mask_len % 8; k++) {
                mask |= (1 << (7 - k));
            }
            sprintf(buf + j, ".%x", oid_mask[i] & mask);
        } else {
            sprintf(buf + j, ".%x", oid_mask[i]);
        }
        j = strlen(buf);
    }

    return buf;
}

/* OUI address text string */
char *misc_oui_addr_txt(const uchar *oui, char *buf)
{
    sprintf(buf, "%02x-%02x-%02x", oui[0], oui[1], oui[2]);
    return buf;
}

/* Zero terminated strncpy */
void
misc_strncpyz(char *dst, const char *src, size_t maxlen)
{
    if (maxlen > 0) {
        (void) strncpy(dst, src, maxlen);
        dst[maxlen - 1] = '\0'; /* Explicit null terminated/truncated */
    }
}

/****************************************************************************/
/*  String check                                                            */
/****************************************************************************/

// Checks if a string only contains numbers 
//
// In : str - String to check;
//
// Return : VTSS_OK if string only contained numbers else VTSS_INVALID_PARAMETER
//
vtss_rc misc_str_chk_numbers_only(const char *str) {
    // Loop through all characters in str, and check that cmd only contains numbers
    u16 i;
    char char_txt;
    for (i = 0; i < strlen(str); i++) {
        char_txt = *(str+i);
        if (isdigit(char_txt) == 0) {
            return VTSS_INVALID_PARAMETER; // The character was not a number. return error indication. 
        }
    }
    return VTSS_OK;
}
   

// Checks if a string is a valid IPv4 of the format xxx.yyy.zzz.qqq
//
// In : str - String to check;
//
// Return : VTSS_OK if string only a valid IPv4 format else VTSS_INVALID_PARAMETER
//
#if defined(VTSS_SW_OPTION_IP2)
vtss_rc misc_str_is_ipv4(const char *str) {
    vtss_ipv4_t temp1;
    return mgmt_txt2ipv4(str, &temp1, NULL, 0);
}

vtss_rc misc_str_is_ipv6(const char *str) {
    vtss_ipv6_t temp1;
    return mgmt_txt2ipv6(str, &temp1);
}

// Checks if a string is a valid hostname (if DNS is part of the build) or IPv4 or IPv6
// A valid hostname is defined in RFC 1123 and 952
// A trailing '.' is not allowed
//
// The code is inspired from cyg_valid_hostname(), but with (hopefully) fewer bugs
//
// In : str - String to check;
//
// Return : VTSS_OK if string only a valid host name else VTSS_INVALID_PARAMETER
//
vtss_rc misc_str_is_hostname(const char *hostname) {
    if (!hostname) {
        return VTSS_INVALID_PARAMETER;
    }
    if (*hostname == '\0') {
        return VTSS_INVALID_PARAMETER;
    }
    if (misc_str_is_ipv4(hostname) == VTSS_OK) {
        return VTSS_OK; // It is a valid IPv4 address
    }

// Only allow host names if we have DNS
#ifdef VTSS_SW_OPTION_DNS
    {
        const char * label;
        const char * label_end = NULL;
        int alpha_cnt = 0; // We require at least one alpha char so we count them here
        const char *pre_label;   
                
        label = hostname;
        while (*label) { // Loop through each label
            if (isalpha(*label)) {
                pre_label = label - 1; 
                if ((*pre_label) != '0') {
                    alpha_cnt++;
                    //printf("xxx %c xxx", *pre_label);
                }    
            }
            if (!isalnum(*label)) { 
                T_D("First label char ('%c') invalid in %s", *label, hostname);
                return VTSS_INVALID_PARAMETER; // First char must be alphanumeric
            }
            label_end = strchr(label, (int)'.') - 1;
            if (label_end == (char *)-1) {
                label_end = strchr(label, (int)'\0') - 1;
            }
            // label_end now points to the last char in the label (the one before '.' or null)
            while (label != label_end) { // Loop through this label up to, but not including the last char
                if (isalpha(*label)) {
                    pre_label = label - 1; 
                    if ((*pre_label) != '0') {
                        alpha_cnt++;
                        //printf("xxx %c xxx", *pre_label);
                    }    
                }
                if (!isalnum(*label) && (*label != '-')) {
                    T_D("Middle label char ('%c') invalid in %s", *label, hostname);
                    return VTSS_INVALID_PARAMETER; // Char must be alphanumeric or '-'
                }
                label++;
            }
            // label now points to the last char in the label (the one before '.' or null)
            if (isalpha(*label)) {
                pre_label = label - 1; 
                if ((*pre_label) != '0') {
                    alpha_cnt++;
                    //printf("xxx %c xxx", *pre_label);
                } 
            }
            if (!isalnum(*label)) {
                T_D("Last label char ('%c') invalid in %s", *label, hostname);
                return VTSS_INVALID_PARAMETER; // Last char must be alphanumeric
            }
            label++; // Move label onto the '.' or the null

            if (*label == '.') {
                label++; // Move label past the '.' to the next label or null
            }
        }
        label_end++; // Move label_end onto the last '.' or the null
        if (*label_end == '.') {
            T_D("Last char ('%c') invalid in %s", *label_end, hostname);
            return VTSS_INVALID_PARAMETER; // Last char must not be '.'
        }
        if (alpha_cnt == 0) {
            T_D("No alpha chars in %s", hostname);
            return VTSS_INVALID_PARAMETER; // No alpha chars found in hostname
        }
        else {
            return VTSS_OK; // It is a valid hostname
        }
    }
#endif /* VTSS_SW_OPTION_DNS */
    return VTSS_INVALID_PARAMETER;
}
#endif /* defined(VTSS_SW_OPTION_IP2) */

/****************************************************************************/
/*  Chiptype functions                                                      */
/****************************************************************************/

vtss_chip_family_t misc_chip2family(cyg_uint16 chiptype)
{
    switch(chiptype) {

    case VTSS_TARGET_E_STAX_34:
        return VTSS_CHIP_FAMILY_ESTAX_34;

    case VTSS_TARGET_SPARX_III_10:
    case VTSS_TARGET_SPARX_III_18:
    case VTSS_TARGET_SPARX_III_24:
    case VTSS_TARGET_SPARX_III_26:
    case VTSS_TARGET_CARACAL_1:
    case VTSS_TARGET_CARACAL_2:
        return VTSS_CHIP_FAMILY_SPARX_III;

    case VTSS_TARGET_JAGUAR_1:
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_CE_MAX_24:
    case VTSS_TARGET_CE_MAX_12:
    case VTSS_TARGET_E_STAX_III_48:
    case VTSS_TARGET_E_STAX_III_68:
    case (VTSS_TARGET_E_STAX_III_24_DUAL & 0xffff):
        return VTSS_CHIP_FAMILY_JAGUAR1;

    case VTSS_TARGET_SERVAL:
    case VTSS_TARGET_SERVAL_LITE:
    case VTSS_TARGET_SPARX_III_11:
        return VTSS_CHIP_FAMILY_SERVAL;

    default:
        return VTSS_CHIP_FAMILY_UNKNOWN;
    }
}

const cyg_uint16         misc_chiptype(void)
{
    return (cyg_uint16) vtss_api_chipid();
}

const vtss_chip_family_t misc_chipfamily(void)
{
    return misc_chip2family(misc_chiptype());
}

const vtss_software_type_t misc_softwaretype(void)
{
    return (vtss_software_type_t) (software_type | (VTSS_SWITCH_STACKABLE << 7));
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc misc_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
#ifdef VTSS_SW_OPTION_VCLI
        misc_cli_req_init(TRACE_GRP_CRIT);
#endif
        misc.chip_no = VTSS_CHIP_NO_ALL;
        misc.phy_inst = PHY_INST;

        /* Initialize ISID and message buffer */
        VTSS_OS_SEM_CREATE(&misc.sem, 1);
        cyg_flag_init(&misc.flags);
        cyg_flag_setbits(&misc.flags, MISC_FLAG_READ_IDLE);
        
        T_D("INIT");
        break;
    case INIT_CMD_START:
        T_D("START");
        misc_stack_register();
        break;
    default:
        break;
    }
    return VTSS_OK;
}



/****************************************************************************/
/*  I2C functions                                                */
/****************************************************************************/
 
#if defined(VTSS_ARCH_LUTON28)
#include <cyg/io/i2c_vcoreii.h>
#else
#include <cyg/io/i2c_vcoreiii.h>
#endif

// Public function for doing i2c reads.
// IN - i2c_addr - The address of the i2c device
//      data     - Pointer to where to put the read data
//      size     - The number of bytes to read
//      max_wait_time - The maximum time to wait for the i2c controller to perform the read (in ms)
vtss_rc vtss_i2c_rd(const vtss_inst_t              inst,
                    const u8 i2c_addr,
                    u8 *data, 
                    const u8 size,
                    const u8 max_wait_time,
                    const i8 i2c_clk_sel)
{
    CYG_I2C_VCORE_DEVICE(i2c_device, i2c_addr);
    return vtss_i2c_dev_rd(&i2c_device, data, size, max_wait_time, i2c_clk_sel);
}

vtss_rc vtss_i2c_dev_rd(const cyg_i2c_device* dev,
                        u8 *data, 
                        const u8 size,
                        const u8 max_wait_time,
                        const i8 i2c_clk_sel)
{
    vtss_rc rc;
    cyg_i2c_transaction_begin(dev);
    ((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_wait = max_wait_time;
    rc = (size == cyg_i2c_transaction_rx(dev,
                                         true, data, size, TRUE, true)) ? VTSS_RC_OK : VTSS_RC_INCOMPLETE;
    cyg_i2c_transaction_end(dev);
    return rc;
}

// Public function for doing i2c write.
// IN - i2c_addr - The address of the i2c device
//      data     - Pointer to the data to be written
//      size     - The number of bytes to read
//      max_wait_time - The maximum time to wait for the i2c controller to perform the read (in ms)
vtss_rc vtss_i2c_wr(const vtss_inst_t              inst,
                    const u8 i2c_addr, 
                    const u8 *data, 
                    const u8 size,
                    const u8 max_wait_time,
                    const i8 i2c_clk_sel) {

    CYG_I2C_VCORE_DEVICE(i2c_device, i2c_addr);
    return vtss_i2c_dev_wr(&i2c_device, data, size, max_wait_time, i2c_clk_sel);
}

vtss_rc vtss_i2c_dev_wr(const cyg_i2c_device* dev,
                        const u8 *data, 
                        const u8 size,
                        const u8 max_wait_time,
                        const i8 i2c_clk_sel) {

    vtss_rc rc = VTSS_RC_OK;
    int size_written;
    cyg_i2c_transaction_begin(dev);
    ((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_wait = max_wait_time;
    ((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_clk_sel = i2c_clk_sel;

    if ((size_written = cyg_i2c_transaction_tx(dev,
                                               true, data, size, true)) != size) {
        if (((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_flags & I2C_FLAG_REBOOT) {
            rc = VTSS_RC_MISC_I2C_REBOOT_IN_PROGRESS; // Signal that i2c isn't valid due to system is rebooting
        } else {
            T_D("Write size:%d, wr_size:%d", size_written, size);
            rc = VTSS_RC_ERROR;
        }
    }
    
    cyg_i2c_transaction_end(dev);
    return rc;
}

// Public function for doing i2c read/write in one sequence.
// IN - i2c_addr - The address of the i2c device
//      wr_data  - Pointer to the data to be written
//      wr_size  - The number of bytes to wrtie
//      rd_data  - Pointer to the data to read
//      rd_size  - The number of bytes to read
//      max_wait_time - The maximum time to wait for the i2c controller to perform the read (in ms)
vtss_rc vtss_i2c_wr_rd(const vtss_inst_t              inst,
                       const u8                       i2c_addr,
                       u8                             *wr_data,
                       const u8                       wr_size,
                       u8                             *rd_data,
                       const u8                       rd_size,
                       const u8                       max_wait_time,
                       const i8                       i2c_clk_sel)
{
    CYG_I2C_VCORE_DEVICE(i2c_device, i2c_addr);
    return vtss_i2c_dev_wr_rd(&i2c_device, wr_data, wr_size, rd_data, rd_size, max_wait_time, i2c_clk_sel);
}

vtss_rc vtss_i2c_dev_wr_rd(const cyg_i2c_device*          dev,
                           u8                             *wr_data,
                           const u8                       wr_size,
                           u8                             *rd_data,
                           const u8                       rd_size,
                           u8                             max_wait_time,
                           i8                             i2c_clk_sel)
{
    vtss_rc rc = VTSS_RC_OK;
    int size;
    cyg_i2c_transaction_begin(dev);
    ((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_wait = max_wait_time;
    ((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_clk_sel = i2c_clk_sel;
    if ((size = cyg_i2c_transaction_tx(dev,
                                       true, wr_data, wr_size, false)) != wr_size) {
        T_D("Write size:%d, wr_size:%d", size, wr_size);
        rc = VTSS_RC_ERROR;
        goto done;
    }
    
    if ((size = cyg_i2c_transaction_rx(dev,
                                       false, rd_data, rd_size, TRUE, true)) != rd_size) {
        T_D("Read size:%d, rd_size:%d", size, rd_size);

        rc = VTSS_RC_ERROR;
    };
done:
    if (((cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra)->i2c_flags & I2C_FLAG_REBOOT) {
        rc = VTSS_RC_MISC_I2C_REBOOT_IN_PROGRESS; // Signal that i2c isn't valid due to system is rebooting
    }

    cyg_i2c_transaction_end(dev);
    return rc;
}


/****************************************************************************/
/*  Silent Upgrade functions                                                */
/****************************************************************************/

BOOL misc_conf_read_use(void)
{
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
    return vtss_icfg_silent_upgrade_active();
#else
    return TRUE;
#endif
}

//-----------------------------------------------------------------------------
// URL utilities
//-----------------------------------------------------------------------------

#define MAX_PORT (0xffff)

void misc_url_parts_init(misc_url_parts_t *parts)
{
    parts->protocol[0] = 0;
    parts->host[0]     = 0;
    parts->port        = 0;
    parts->path[0]     = 0;
}

BOOL misc_url_decompose(const char *url, misc_url_parts_t *parts)
{
    const char *p;
    u32        n;

    misc_url_parts_init(parts);

    // Protocol

    p = url;
    n = 0;
    while (*p  &&  *p != ':'  &&  n < sizeof(parts->protocol) - 1) {
        parts->protocol[n++] = *p++;
    }
    if ((*p != ':')  ||  (n == 0)) {
        return FALSE;
    }
    parts->protocol[n] = 0;
    p++;

    // For flash: protocol, only decompose protocol and path

    if (strcmp(parts->protocol, "flash") == 0) {
        n = 0;
        // Skip initial redundant '/' (e.g. '///foo' => '/foo')
        while (*p == '/'  &&  *(p + 1) == '/') {
            p++;
        }
        while (*p  &&  n < sizeof(parts->path) - 1) {
            parts->path[n++] = *p++;
        }
        parts->path[n] = 0;
        return (!*p)  &&  (n > 0)  &&  (n < NAME_MAX);
    }

    // Not flash:, decompose all

    // Double-slash

    if ((*p != '/')  ||  (*(p + 1) != '/')) {
        return FALSE;
    }
    p += 2;

    // Host

    n = 0;
    while (*p  &&  *p != '/'  &&  *p != ':'  &&  n < sizeof(parts->host) - 1) {
        parts->host[n++] = *p++;
    }
    parts->host[n] = 0;
    if ((n == 0)  ||  (n == sizeof(parts->host) - 1)) {
        return FALSE;
    }

    // Optional port

    if (*p == ':') {
        int port = 0;
        ++p;
        if (!(*p >= '0'  &&  *p <= '9')) {
            return FALSE;
        }
        while (*p >= '0'  &&  *p <= '9') {
            port = 10 * port + (*p - '0');
            if (port > MAX_PORT) {
                return FALSE;
            }
            ++p;
        }
        parts->port = (u16)port;
    }

    // Single slash

    if (*p != '/') {
        return FALSE;
    }

    // Path

    n = 0;
    while (*p  &&  n < sizeof(parts->path) - 1) {
        parts->path[n++] = *p++;
    }
    parts->path[n] = 0;

    return (!*p)  &&  (n > 0);
}



// Shortest possible URL (but meaningless in our world) is length 7: a://b/c
BOOL misc_url_compose(char *url, int max_len, const misc_url_parts_t *parts)
{
    int n;

    if (!url || max_len < 8 || !parts || !(parts->protocol[0]) || !(parts->path[0])) {
        return FALSE;
    }

    if (strcmp(parts->protocol, "flash") == 0) {
        n = snprintf(url, max_len, "%s:%s", parts->protocol, parts->path);
    } else {
        if (!(parts->host[0])) {
            return FALSE;
        }
        if (parts->port) {
            n = snprintf(url, max_len, "%s://%s:%u/%s",
                         parts->protocol, parts->host, parts->port, parts->path);
        } else {
            n = snprintf(url, max_len, "%s://%s/%s",
                         parts->protocol, parts->host, parts->path);
        }
    }

    return (n > 0)  &&  (n < max_len - 1);   // Less-than because n doesn't count trailing '\0'; -1 because of eCos silliness regarding snprintf return value
}

// See misc_api.h
char *str_tolower(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++) {
        str[i] = tolower(str[i]);
    }
    return str;
} /* str_tolower */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
