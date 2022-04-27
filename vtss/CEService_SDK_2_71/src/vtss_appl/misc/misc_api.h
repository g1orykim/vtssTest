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

#ifndef _VTSS_APPL_MISC_API_H_
#define _VTSS_APPL_MISC_API_H_

#include <time.h>
#include <cyg/io/i2c.h>
#include <sys/types.h>
#include "vtss_types.h" /* For vtss_rc     */
#include "main.h"       /* for vtss_isid_t */
#ifdef VTSS_SW_OPTION_IP2
#include "ip2_api.h"
#endif /* VTSS_SW_OPTION_IP2 */

/* ========================================================================== *
 * Silent upgrade.
 *
 * The silent upgrade feature allows an ICFG-enabled build to upgrade
 * the configuration from 'conf' to ICFG + ICLI format ("running-config").
 *
 * Normal conf-using modules must follow these rules:
 *
 *   * Only read conf blocks during the silent upgrade phase, i.e. during boot.
 *   * No creation/writing of conf blocks at any time.
 *
 * Initially, silent upgrade is only supported on standalone builds, not
 * stacking. In order to minimize the impact of per-module changes, and to
 * keep code paths essentially unchanged for stack builds, two items are
 * created:
 *
 *   * A function that indicates whether conf blocks should be read at all.
 *   * A macro symbol that indicates whether the module's conf write code is
 *     to be included or not.
 *
 * ========================================================================= */

#if defined(VTSS_SW_OPTION_ICFG)
#define VTSS_SW_OPTION_SILENT_UPGRADE     /* Don't include conf write code */
#endif

/* Return TRUE if normal modules should use conf for *reading* their
 * configuration. This value depends on two things:
 *
 *   1. Does this build use ICFG for configuration?
 *          No => return TRUE.
 *   2. If yes, are we doing a silent upgrade from conf to ICFG?
 *          Yes => return TRUE.
 *          No  => return FALSE.
 */
BOOL misc_conf_read_use(void);

/* Get chip number */
vtss_rc misc_chip_no_get(vtss_chip_no_t *chip_no);

/* Set chip number */
vtss_rc misc_chip_no_set(vtss_chip_no_t chip_no);

/* Convert (blk, sub, addr) to 32-bit address */
ulong misc_l28_reg_addr(uint blk, uint sub, uint addr);

/* Read switch chip register */
vtss_rc misc_debug_reg_read(vtss_isid_t isid, vtss_chip_no_t chip_no, ulong addr, ulong *value);

/* Write switch chip register */
vtss_rc misc_debug_reg_write(vtss_isid_t isid, vtss_chip_no_t chip_no, ulong addr, ulong value);

/* Read PHY register */
vtss_rc misc_debug_phy_read(vtss_isid_t isid,
                            vtss_port_no_t port_no, uint reg, uint page, ushort *value, BOOL mmd_access, ushort devad);

/* Write PHY register */
vtss_rc misc_debug_phy_write(vtss_isid_t isid,
                             vtss_port_no_t port_no, uint reg, uint page, ushort value, BOOL mmd_access, ushort devad);

/* Suspend/resume */
vtss_rc misc_suspend_resume(vtss_isid_t isid, BOOL resume);

/* strip leading path from file */
const char *misc_filename(const char *fn);

vtss_rc mgmt_txt2list_bf(char *buf, BOOL *list, ulong min, ulong max, BOOL def, BOOL bf);

/* MAC address text string */
char *misc_mac_txt(const uchar *mac, char *buf);

/* MAC to string */
const char *misc_mac2str(const uchar *mac);

/* Create an instantiated MAC address based on base MAC and instance number */
void misc_instantiate_mac(uchar *mac, const uchar *base, int instance);

/* IPv4 address text string */
char *misc_ipv4_txt(vtss_ipv4_t ip, char *buf);

/* IPv6 address text string. Buf must be at least 50 chars long */
char *misc_ipv6_txt(const vtss_ipv6_t *ipv6, char *buf);

/* IPv4/v6 address text string. Buf must be at least 50 chars long */
char *misc_ip_txt(vtss_ip_addr_t *ip, char *buf);

/* IPv4/v6 address/mask text string. Give about 128 bytes. */
char *misc_ipaddr_txt(char *buf, size_t bufsize, vtss_ip_addr_t *addr, vtss_prefix_size_t prefix_size);

/* IP address text string - network order */
const char *misc_ntoa(ulong ip);

/* IP address text string - host order */
const char *misc_htoa(ulong ip);

/* Software version text string */
const char *misc_software_version_txt(void);

/* Product name text string */
const char *misc_product_name(void);

/* Software codebase revision string */
const char *misc_software_code_revision_txt(void);

/* Software date text string */
const char *misc_software_date_txt(void);

/* Time to string */
const char *misc_time2str(time_t time);

#define MISC_RFC3339_TIME_STR_LEN 26
/* Like misc_time2str() but doesn't use a thread unsafe internal buffer
 * to build the string up.
 * #input_str must be at least MISC_RFC3339_TIME_STR_LEN (including
 * terminating '\0' character) bytes long.
 * Returns input_str on success, NULL on error.
 */
const char *misc_time2str_r(time_t time, char *input_str);

/* Seconds to interval (string) */
const char *misc_time2interval(time_t time);

/* engine ID to string */
const char *misc_engineid2str(const uchar *engineid, ulong engineid_len);

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
const char *misc_oid2str(const ulong *oid, ulong oid_len, const uchar *oid_mask, ulong oid_mask_len);

/* OID mask to string.
  'oid_mask' is a list with 8 bits of hex octets.
  'oid_mask_len' is the total mask len.
  For example:
  oid_mask = {0xF0, 0xFF}
  oid_mask_len = 9;
  ---> The output is .F0.80.

  Note: The value of 'oid_mask_len' should be not great than 128.
 */
const char *misc_oidmask2str(const uchar *oid_mask, ulong oid_mask_len);

/* OUI address text string */
char *misc_oui_addr_txt(const uchar *oui, char *buf);

/*  Checks if a string only contains numbers */
vtss_rc misc_str_chk_numbers_only(const char *str);

/* Zero terminated strncpy */
void
misc_strncpyz(char *dst, const char *src, size_t maxlen);

vtss_rc misc_str_is_ipv4(const char *str);
vtss_rc misc_str_is_ipv6(const char *str);
vtss_rc misc_str_is_hostname(const char *hostname);

vtss_inst_t misc_phy_inst_get(void);
vtss_rc misc_phy_inst_set(vtss_inst_t instance);

/* Chiptype functions */
typedef enum {
    VTSS_CHIP_FAMILY_UNKNOWN,      /* ??? */
    VTSS_CHIP_FAMILY_ESTAX_34,     /* Luton28 */
    VTSS_CHIP_FAMILY_SPARX_III,    /* Luton26 */
    VTSS_CHIP_FAMILY_JAGUAR1,      /* Jaguar1 */
    VTSS_CHIP_FAMILY_SERVAL,       /* Serval */
} vtss_chip_family_t;

vtss_chip_family_t misc_chip2family(cyg_uint16 chiptype);
const cyg_uint16         misc_chiptype(void);
const vtss_chip_family_t misc_chipfamily(void);

/* Software Identification functions */
typedef enum {
    VTSS_SOFTWARE_TYPE_UNKNOWN,    /* ??? */
    VTSS_SOFTWARE_TYPE_DEFAULT,    /* Default */
    VTSS_SOFTWARE_TYPE_WEBSTAX,    /* WebStaX/WebSparX */
    VTSS_SOFTWARE_TYPE_SMBSTAX,    /* SMBStaX */
    VTSS_SOFTWARE_TYPE_CESERVICES, /* CEServices */
    VTSS_SOFTWARE_TYPE_CEMAX,      /* CEMax */
    /* ADD NEW TYPES IMMEDIATELY ABOVE THIS LINE - DONT REMOVE LINES EVER! */
} vtss_software_type_t;

const vtss_software_type_t misc_softwaretype(void);

/* Initialize module */
vtss_rc misc_init(vtss_init_data_t *data);

/* ================================================================= *
 *  Conversion between internal and user port numbers.
 *  These functions are intended to be used only in places that
 *  interact with the user, i.e. Web, CLI, and SNMP.
 * ================================================================= */
typedef vtss_port_no_t vtss_uport_no_t; /* User port is of same type as normal, internal port. */

/****************************************************************************/
// Convert from internal to user port number.
/****************************************************************************/
static inline vtss_uport_no_t iport2uport(vtss_port_no_t iport) {
  return iport + 1 - VTSS_PORT_NO_START;
}

/****************************************************************************/
// Convert from user to internal port number.
/****************************************************************************/
static inline vtss_port_no_t uport2iport(vtss_uport_no_t uport) {
  return (uport == 0) ? VTSS_PORT_NO_NONE : (uport - 1 + VTSS_PORT_NO_START);
}

/* ================================================================= *
 *  Conversion between internal and user priority.
 *  These functions are intended to be used only in places that
 *  interact with the user, i.e. Web, CLI, and SNMP.
 * ================================================================= */

/****************************************************************************/
// Convert from internal to user priority.
/****************************************************************************/
static inline vtss_prio_t iprio2uprio(vtss_prio_t iprio) {
#ifdef VTSS_ARCH_LUTON28
  return iprio + 1 - VTSS_PRIO_START;
#else
  return iprio - VTSS_PRIO_START;
#endif
}

/****************************************************************************/
// Convert from user to internal priority.
/****************************************************************************/
static inline vtss_prio_t uprio2iprio(vtss_prio_t uprio) {
#ifdef VTSS_ARCH_LUTON28
  return (uprio == 0) ? uprio : (uprio - 1 + VTSS_PRIO_START);
#else
  return uprio + VTSS_PRIO_START;
#endif
}

/* ================================================================= *
 *  Conversion between internal and user policy.
 *  These functions are intended to be used only in places that
 *  interact with the user, i.e. Web, CLI, and SNMP.
 * ================================================================= */

/****************************************************************************/
// Convert from internal to user policer.
/****************************************************************************/
static inline vtss_acl_policer_no_t ipolicer2upolicer(vtss_acl_policer_no_t ipolicer) {
  return (ipolicer + 1 - VTSS_ACL_POLICER_NO_START);
}

/****************************************************************************/
// Convert from user to internal policer.
/****************************************************************************/
static inline vtss_acl_policer_no_t upolicer2ipolicer(vtss_acl_policer_no_t upolicer) {
  return (upolicer - 1 + VTSS_ACL_POLICER_NO_START);
}

#if defined(VTSS_ARCH_CARACAL)
/* ================================================================= *
 *  Conversion between internal and user policer.
 *  These functions are intended to be used only in places that
 *  interact with the user, i.e. Web, CLI, and SNMP.
 * ================================================================= */

/****************************************************************************/
// Convert from internal to user EVC policer.
/****************************************************************************/
static inline vtss_evc_policer_id_t ievcpolicer2uevcpolicer(vtss_evc_policer_id_t ievcpolicer) {
  return (ievcpolicer + 1);
}

/****************************************************************************/
// Convert from user to internal EVC policer.
/****************************************************************************/
static inline vtss_evc_policer_id_t uevcpolicer2ievcpolicer(vtss_evc_policer_id_t uevcpolicer) {
  return (uevcpolicer - 1);
}
#endif /* !VTSS_ARCH_CARACAL */

// -1 is used to signal the i2c multiplexer not used.
#define NO_I2C_MULTIPLEXER -1

/**
 * \brief Read from i2c controller
 *
 * \param inst        [IN]    Target instance reference.
 * \param i2c_addr    [IN]    The address of the i2c device.
 * \param data        [OUT]   Pointer to where to put the read data.
 * \param size        [IN]    The number of bytes to read.
 * \param max_wait_time [IN]  The maximum time to wait for the i2c controller to perform the read (in ms).
 *
 * \return Return code.
 */
vtss_rc vtss_i2c_rd(const vtss_inst_t              inst,
                    const u8 i2c_addr, 
                    u8 *data, 
                    const u8 size,
                    const u8 max_wait_time,
                    const i8 i2c_clk_sel);

/* Same, cyg_i2c_device style */

vtss_rc vtss_i2c_dev_rd(const cyg_i2c_device* dev,
                        u8 *data, 
                        const u8 size,
                        const u8 max_wait_time,
                        const i8 i2c_clk_sel);

/**
 * \brief Write to i2c controller
 *
 * \param inst        [IN]    Target instance reference
 * \param i2c_addr    [IN]    The address of the i2c device.
 * \param data        [IN]    Data to be written.
 * \param size        [IN]    The number of bytes to write.
 * \param max_wait_time [IN]  The maximum time to wait for the i2c controller to perform the wrtie (in ms).
 *
 * \return Return code.
 */
vtss_rc vtss_i2c_wr(const vtss_inst_t              inst,
                    const u8 i2c_addr, 
                    const u8 *data, 
                    const u8 size,
                    const u8 max_wait_time,
                    const i8 i2c_clk_sel);

/* Same, cyg_i2c_device style */

vtss_rc vtss_i2c_dev_wr(const cyg_i2c_device* dev,
                        const u8 *data, 
                        const u8 size,
                        const u8 max_wait_time,
                        const i8 i2c_clk_sel);

/**
 * \brief Do a write and read in one step
 *
 * \param inst           [IN]  Target instance reference
 * \param i2c_addr       [IN]  The address of the i2c device.
 * \param wr_data        [IN]  Data to be written.
 * \param wr_size        [IN]  The number of bytes to write.
 * \param rd_data        [IN]  Pointer to where to put the read data.
 * \param rd_size        [IN]  The number of bytes to read.
 * \param max_wait_time  [IN]  The maximum time to wait for the i2c controller to perform the wrtie (in ms).
 *
 * \return Return code.
 */

vtss_rc vtss_i2c_wr_rd(const vtss_inst_t              inst,
                       const u8                       i2c_addr,
                       u8                             *wr_data,
                       const u8                       wr_size,
                       u8                             *rd_data,
                       const u8                       rd_size,
                       const u8                       max_wait_time,
                       const i8                       i2c_clk_sel);


/* Same, cyg_i2c_device style */

vtss_rc vtss_i2c_dev_wr_rd(const cyg_i2c_device*          dev,
                           u8                             *wr_data,
                           const u8                       wr_size,
                           u8                             *rd_data,
                           const u8                       rd_size,
                           const u8                       max_wait_time,
                           const i8                       i2c_clk_sel);

/** \brief Structure for the elements that make up a URL of the form:
 *
 *      protocol://host[:port]/path
 *
 * or
 *
 *      flash:path
 *
 * e.g.
 *
 *      tftp://1.2.3.4:5678/path/to/myfile.dat
 *
 * or
 *
 *      flash:startup-config
 *      flash:/startup-config
 */
typedef struct {
    char protocol[32];
    char host[64];
    u16  port;         /* Port number, or zero if none given */
    char path[256];
} misc_url_parts_t;

/** \brief Initialize #misc_url_parts_t to defaults (empty strings, port zero)
 *
 * \param parts [IN/OUT] The initialized struct
 */
void misc_url_parts_init(misc_url_parts_t *parts);

/** \brief Decompose URL string by scanning #url.
 *
 * \param url   [IN]  Pointer to string
 * \param parts [OUT] The decomposed URL
 *
 * \return TRUE = decomposition is valid; FALSE = error in URL
 */
BOOL misc_url_decompose(const char *url, misc_url_parts_t *parts);

/** \brief Compose URL into pre-allocated buffer of given max length.
 *
 * \param url      [IN/OUT] Destination buffer
 * \param max_len  [IN]     Max length of buffer, including terminating \0
 * \param parts    [IN]     URL parts
 *
 * \return TRUE = composition is valid; FALSE = result too long or #parts invalid
 */
BOOL misc_url_compose(char *url, int max_len, const misc_url_parts_t *parts);


/** \brief Convert string to lowercase characters only
 *
 * \param url      [IN/OUT] string to convert
 */
char *str_tolower(char *str);

/****************************************************************************/
// Error Return Codes (vtss_rc)
/****************************************************************************/
enum {
    MISC_ERROR_INV_PARAM = MODULE_ERROR_START(VTSS_MODULE_ID_MISC), // NULL parameter passed to one of the mirror_XXX functions, where a non-NULL was expected.
    VTSS_RC_MISC_I2C_REBOOT_IN_PROGRESS                         // Signaling that the result of the I2C access is invalid due to the system is rebooting. 
};
#endif /* _VTSS_APPL_MISC_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
