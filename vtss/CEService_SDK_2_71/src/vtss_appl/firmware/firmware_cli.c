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

#include "cli.h"
#include "main.h"
#include "firmware_cli.h"
#include "firmware_api.h"
#include "flash_mgmt_api.h"
#include "cli_trace_def.h"
#include "network.h"
#include "control_api.h" /* For vtss_restart_t */
#ifdef VTSS_SW_OPTION_LIBFETCH
#include "fetch.h"
#endif

#include <arpa/inet.h>
#include <cyg/crc/crc.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_FIRMWARE

typedef struct {
    cli_spec_t     ipv4_router_spec;
    vtss_ipv4_t    ipv4_router;
    vtss_ipv4_t    ipv4_server;
    BOOL           cool_restart_set;
} firmware_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void firmware_cli_init(void)
{
    /* register the size required for Firmware req. structure */
    cli_req_size_register(sizeof(firmware_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* CLI software update */
static void cli_cmd_load_firmware(cli_req_t *req)
{
    char server[sizeof "000.000.000.000"], *file = req->parm;
    vtss_ipv4_t ipv4_server;
    struct sockaddr_in host;
    struct hostent *hp;
    firmware_cli_req_t *firmware_req = req->module_req;

    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    if(!inet_aton(req->host_name, &host.sin_addr)) {
        hp = gethostbyname(req->host_name);
        if(hp == NULL) {
            #ifdef VTSS_SW_OPTION_CLI
            CPRINTF("*** Invalid IP address: %s\n", req->host_name);
            #endif // VTSS_SW_OPTION_CLI
            return;
        }
        else {
            memmove(&host.sin_addr, hp->h_addr, hp->h_length);
        }
    }
    ipv4_server = htonl(host.sin_addr.s_addr);
    misc_ipv4_txt(ipv4_server, server);

    {     /* Managed - just do the TFTP & flash */
        char *buffer;
        size_t length;
        cli_iolayer_t *pIO = cli_get_io_handle();

        if((buffer = firmware_tftp(pIO, server, file, "managed", &length))) {
            vtss_rc result;
            /* This only returns if an error occurred */
            if((result = firmware_update(pIO, buffer, length, firmware_req->cool_restart_set ? VTSS_RESTART_COOL : VTSS_RESTART_WARM)) != VTSS_OK) {
              cli_printf("Error: %s\n", error_txt(result));
            }
        }
    }
}

#ifdef VTSS_SW_OPTION_IPV6
/* CLI software update */
static void cli_cmd_load_firmware_ipv6(cli_req_t *req)
{
    char server[50], *file = req->parm;
    firmware_cli_req_t *firmware_req = req->module_req;

    misc_ipv6_txt(&req->ipv6_addr, server);

    if(vtss_switch_mgd()) {     /* Managed - just do the TFTP & flash */
        char *buffer;
        size_t length;
        cli_iolayer_t *pIO = cli_get_io_handle();

        if(req->ipv6_addr.addr[0] == 0xfe && req->ipv6_addr.addr[1] == 0x80) {
            CPRINTF("Using IPv6 link-local address is not allowed here.\n");
            return;
        }
        else if(req->ipv6_addr.addr[0] == 0xff) {
            CPRINTF("Using IPv6 multicast address is not allowed here.\n");
            return;
        }

        if((buffer = firmware_tftp_ipv6(pIO, server, file, &length))) {
            vtss_rc result;
            /* This only returns if an error occurred */
            if((result = firmware_update(pIO, buffer, length, firmware_req->cool_restart_set ? VTSS_RESTART_COOL : VTSS_RESTART_WARM)) != VTSS_OK) {
                cli_printf("Error: %s\n", error_txt(result));
            }
        }
    }
}
#endif /* VTSS_SW_OPTION_IPV6 */

static void display_props(char *type, const struct fis_table_entry *fis)
{
    uchar vers[256], date[256];
    firmware_scan_props(fis, vers, sizeof(vers), date, sizeof(date));

    cli_table_header(type);
    CPRINTF("%-10s: %s\n", "Image", fis->name);
    CPRINTF("%-10s: %s\n", "Version", vers);
    CPRINTF("%-10s: %s\n\n", "Date", date);
}

static void cli_cmd_firmware_info(cli_req_t *req)
{
    struct fis_table_entry fis;
    const char *pri_name = "managed", *alt_name = "managed.bk";
    char active_image[sizeof(fis.name)];

    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis);
    strcpy(active_image, fis.name);

    if(flash_mgmt_fis_lookup(active_image, &fis) >= 0) {
        display_props("Active Image", &fis);
    } else {
        CPRINTF("Error: unable to lookup active image \"%s\"\n", active_image);
    }

    if(strcmp(fis.name, pri_name) == 0 &&
       flash_mgmt_fis_lookup(alt_name, &fis) >= 0) {
        display_props("Alternate Image", &fis);
    }
}

#if VTSS_SWITCH_STANDALONE
static void cli_cmd_firmware_swap(cli_req_t *req)
{
    struct fis_table_entry fis;
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis);
    if(strcmp(fis.name, "managed") == 0) {
        if(firmware_swap_entries() == 0)
            CPRINTF("Alternate image activated, now rebooting.\n");
        else
            CPRINTF("Alternate image activation failed.\n");
    } else {
        CPRINTF("Active image is %s, alternate image activation is disabled\n", fis.name);
    }
}
#endif /* VTSS_SWITCH_STANDALONE */

static void  cli_cmd_debug_firmware_pause(cli_req_t *req)
{
    if(req->set) {
        if(req->int_values[0] < 0 || req->int_values[0] > 3) {
           CPRINTF("Error: Valid values are [0; 3]\n");
        } else {
           firmware_debug_pause_set(req->int_values[0]);
        }
    } else {
        CPRINTF("%d\n", firmware_debug_pause_get());
    }
}

static void  cli_cmd_debug_firmware_update(struct fis_table_entry *fis, int fis_idx, const char *buffer, size_t length)
{
    int err;
    u32 cksum = cyg_crc32(buffer, length);

    if (length == fis->data_length && cksum == fis->file_cksum) {
        CPRINTF("Flash entry %s already contain same image, not updating.\n", fis->name);
    } else {
        CPRINTF("Erasing image...\n");
        if ((err = control_flash_erase(fis->flash_base, length)) == FLASH_ERR_OK) {
            CPRINTF("Programming image...\n");
            if ((err = control_flash_program(fis->flash_base, buffer, length)) == FLASH_ERR_OK) {
                CPRINTF("Programmed %d bytes (cksum 0x%08x) to %s - OK\n", length, cksum, fis->name);
                /* Update Redboot Flash Directory with new entry. */
                fis->data_length = length;
                fis->file_cksum = cksum;
                CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_MODIFY_ENTRY, fis_idx, fis);
                CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_START_UPDATE, 0, NULL);
                CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_FINISH_UPDATE, 0, NULL);
            } else {
                CPRINTF("Flash program error: %d %s\n", err, flash_errmsg(err));
            }
        } else {
            CPRINTF("Flash erase error: %d %s\n", err, flash_errmsg(err));
        }
    }
}

static void  cli_cmd_debug_firmware_fis(cli_req_t *req)
{
    char server[sizeof "000.000.000.000"],*file = req->parm;
    const char *section = req->host_name;
    struct fis_table_entry fis;
    int ent;

    misc_ipv4_txt(req->ipv4_addr, server);

    if((ent = flash_mgmt_fis_lookup(req->host_name, &fis)) > 0) {
        char *buffer;
        size_t length;

        if((buffer = firmware_tftp(cli_get_io_handle(), server, file, section, &length))) {
            cli_cmd_debug_firmware_update(&fis, ent, buffer, length);
            VTSS_FREE(buffer);
        }
    } else {
        CPRINTF("Did not find section %s\n", req->host_name);
    }
}

static void  cli_cmd_debug_firmware_bootloader(cli_req_t *req)
{
    char server[sizeof "000.000.000.000"], *file = req->parm;
    const char *name = "RedBoot";
    char *buffer;
    size_t length;
    cli_iolayer_t *pIO = cli_get_io_handle();

    misc_ipv4_txt(req->ipv4_addr, server);

    if((buffer = firmware_tftp(pIO, server, file, name, &length))) {
        simage_t image;
        if(simage_parse(&image, buffer, length) == VTSS_OK) {
            vtss_rc rc = firmware_bootloader_update(pIO, &image, name);
            if(rc != VTSS_OK)
                CPRINTF("Update failed: %s\n", error_txt(rc));
        } else
            CPRINTF("Bad Image, invalid format\n");
        VTSS_FREE(buffer);
    }
}

static void cli_cmd_debug_firmware_check_same(cli_req_t *req)
{
    if(req->set) {
        firmware_debug_check_same_set(req->enable);
    } else {
        CPRINTF("%s\n", cli_bool_txt(firmware_debug_check_same_get()));
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_firmware_file_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
        cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_raw(cmd_org, req);

    return error;
}

static int32_t flash_fis_name_parse (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_text(cmd_org, req->host_name, sizeof(req->host_name)-1);
}

#ifdef VTSS_SW_OPTION_IPV6
static int32_t cli_firmware_ipv6_addr_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
        cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_ipv6(cmd, &req->ipv6_addr, &req->ipv6_addr_spec);

    return error;
}
#endif

static int32_t cli_firmware_integer_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
        cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_integer(cmd, req, stx);

    return error;
}




















/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t firmware_cli_parm_table[] = {
    {
        "<file_name>",
        "Firmware file name",
        CLI_PARM_FLAG_NONE,
        cli_firmware_file_name_parse,
        NULL
    },
    {
        "<url>",
        "Network Link (Syntax:\n"
        "    http://[user[:pwd]@]host[:port]/path)\n"
        "    ftp://[user[:pwd]@]host[:port]/path)",
        CLI_PARM_FLAG_NONE,
        cli_firmware_file_name_parse,
        NULL
    },
    {
        "<flash_name>",
        "Flash entry name",
        CLI_PARM_FLAG_NONE,
        flash_fis_name_parse,
        NULL
    },
#ifdef VTSS_SW_OPTION_IPV6
    {
        "<ipv6_server>",
        "TFTP server IPv6 address",
        CLI_PARM_FLAG_NONE,
        cli_firmware_ipv6_addr_parse,
        cli_cmd_load_firmware_ipv6
    },
#endif /* VTSS_SW_OPTION_IPV6 */
    {
        "<integer>",
        "\n0: No pauses (default)\n"
        "1: Insert 30 sec. pause after erase\n"
        "2: Insert 30 sec. pause after program\n"
        "3: Insert 30 sec. pause betw. FIS start and FIS finish update\n",
        CLI_PARM_FLAG_SET,
        cli_firmware_integer_parse,
        cli_cmd_debug_firmware_pause
    },
    {
        "enable|disable",
        "enable : If image the same as in flash, don't write (default)\n"
        "disable: If image the same as in flash, write it anyway\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_firmware_check_same
    },









#if defined(VTSS_SW_OPTION_WARM_START) && defined(VTSS_SW_OPTION_IPV6)
    {
        "cool",
        "Perform a cool restart after firmware upload (default: warm)",
        CLI_PARM_FLAG_SET,
        cli_firmware_restart_parse,
        cli_cmd_load_firmware_ipv6
    },
#endif /* VTSS_SW_OPTION_WARM_START */
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_LOAD_FIRMWARE,
#ifdef VTSS_SW_OPTION_IPV6
    PRIO_LOAD_FIRMWARE_IPV6,
#endif /* VTSS_SW_OPTION_IPV6 */
    PRIO_FIRMWARE_INFO,
    PRIO_FIRMWARE_SWAP,
    PRIO_DEBUG_FIRMWARE_NAME = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_FIRMWARE_PAUSE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_FIRMWARE_CHECK_SAME = CLI_CMD_SORT_KEY_DEFAULT,
};




#define COOL_KW                 /* None */

#define FIRMWARE_LOAD_IPV4_CMD "Firmware Load <ip_addr_string> <file_name>" COOL_KW
#define FIRMWARE_LOAD_IPV6_CMD "Firmware IPv6 Load <ipv6_server> <file_name>" COOL_KW

cli_cmd_tab_entry (
    NULL,
    FIRMWARE_LOAD_IPV4_CMD,
    "Load new firmware from TFTP server",
    PRIO_LOAD_FIRMWARE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_load_firmware,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#ifdef VTSS_SW_OPTION_IPV6
cli_cmd_tab_entry (
    NULL,
    FIRMWARE_LOAD_IPV6_CMD,
    "Load new firmware from IPv6 TFTP server",
    PRIO_LOAD_FIRMWARE_IPV6,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_load_firmware_ipv6, 
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_IPV6 */
cli_cmd_tab_entry (
    NULL,
    "Firmware Information",
    "Display information about active and alternate firmware images",
    PRIO_FIRMWARE_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MISC,
    cli_cmd_firmware_info,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#if VTSS_SWITCH_STANDALONE
cli_cmd_tab_entry (
    NULL,
    "Firmware Swap",
    "Activate the alternate firmware image.",
    PRIO_FIRMWARE_SWAP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_firmware_swap,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SWITCH_STANDALONE */
cli_cmd_tab_entry (
    "Debug Firmware Pause",
    "Debug Firmware Pause [<integer>]",
    "Show or insert a 30 sec. pause at various positions while updating",
    PRIO_DEBUG_FIRMWARE_PAUSE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_firmware_pause,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug Firmware CheckSame",
    "Debug Firmware CheckSame [enable|disable]",
    "Set or show check for the same image when updating",
    PRIO_DEBUG_FIRMWARE_CHECK_SAME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_firmware_check_same,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Firmware Load",
    "Debug Firmware Load <flash_name> <ip_addr> <file_name>",
    "Update existing FIS entry via tftp",
    PRIO_DEBUG_FIRMWARE_CHECK_SAME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_firmware_fis,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Firmware Bootloader",
    "Debug Firmware Bootloader <ip_addr> <file_name>",
    "Update bootloader via tftp",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_firmware_bootloader,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_LIBFETCH

static u8 *firware_netload(size_t max_size, size_t *actual_size, const char *target)
{
    struct url *url;
    u8 *buffer = NULL;
    size_t length = 0;
    if ((url = fetchParseURL(target)) == NULL) {
        CPRINTF("Malformed URL: %s\n", target);
    } else {
        fetchIO *fp;
        struct url_stat us;
        // Send HTTP GET request and get reply
        CPRINTF("Fetching...\n");
        fetchConnTimeout = 3;   /* 3 secs for network connect */
        if ((fp = fetchXGet(url, &us, "v"))) {
            ssize_t recv_len;
            size_t buflen;
            if (us.size < 0) {
                buflen = max_size; /* Assume the worst */
            } else {
                buflen = us.size; /* We have a file length in advance */
            }
            if (buflen > max_size) {
                CPRINTF("File too large (%zd bytes), NOT updating flash!\n", buflen);
            } else if ((buffer = VTSS_MALLOC(buflen)) == NULL) {
                CPRINTF("Unable to malloc buffers of %zd bytes\n", buflen);
            } else {
                do {
                    recv_len = fetchIO_read(fp, buffer + length, buflen - length);
                    if (recv_len <= 0) {
                        break;
                    }
                    length += recv_len;
                } while (recv_len > 0);
            }
            fetchIO_close(fp);
            *actual_size = length;
        } else {
            CPRINTF("Getting \"%s\" failed: %s\n", target, fetchLastErrString);
        }
        fetchFreeURL(url);
    }
    return buffer;
}

static void cli_cmd_load_firmware_http(cli_req_t *req)
{
    firmware_cli_req_t *firmware_req = req->module_req;
    const char *target = req->parm;
    struct fis_table_entry fis;
    int ent;

    if((ent = flash_mgmt_fis_lookup("managed", &fis)) < 0) {
        CPRINTF("Did not find firmware flash entry\n");
    } else {
        char *buffer;
        size_t length = 0;
        if ((buffer = firware_netload(fis.size, &length, target)) != NULL && length) {
            cli_iolayer_t *pIO = cli_get_io_handle();
            vtss_rc result;
            CPRINTF("Retreived %zd bytes, flashing ...\n", length);
            if((result = firmware_update(pIO, buffer, length, firmware_req->cool_restart_set ? VTSS_RESTART_COOL : VTSS_RESTART_WARM)) != VTSS_OK) {
                CPRINTF("Error: %s\n", error_txt(result));
            }
            /* If we return, there was an error, and the buffer is freed */
            buffer = NULL;
        }
        if (buffer) {
            VTSS_FREE(buffer);
        }
    }
}
cli_cmd_tab_entry (
    NULL,
    "Firmware NetLoad <url>" COOL_KW,
    "Load new firmware via http",
    PRIO_LOAD_FIRMWARE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MISC,
    cli_cmd_load_firmware_http,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

static void cli_cmd_debug_firmware_http_fis(cli_req_t *req)
{
    const char *target = req->parm;
    struct fis_table_entry fis;
    int ent;

    if((ent = flash_mgmt_fis_lookup(req->host_name, &fis)) < 0) {
        CPRINTF("Did not find section %s\n", req->host_name);
    } else {
        char *buffer;
        size_t length = 0;
        if ((buffer = firware_netload(fis.size, &length, target)) != NULL && length) {
            CPRINTF("Retreived %zd bytes from %s, flashing ...\n", length, target);
            cli_cmd_debug_firmware_update(&fis, ent, buffer, length);
        }
        if (buffer) {
            VTSS_FREE(buffer);
        }
    }
}
cli_cmd_tab_entry (
    "Debug Firmware NetLoad",
    "Debug Firmware NetLoad <flash_name> <url>",
    "Update existing FIS entry via http",
    PRIO_DEBUG_FIRMWARE_CHECK_SAME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_firmware_http_fis,
    NULL,
    firmware_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif  /* VTSS_SW_OPTION_LIBFETCH */

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
