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

#define __VTSS_CONF_C__

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "control_api.h" /* For control_flash_XXX() */
#include "flash_mgmt_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "conf.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "conf_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

#include <cyg/crc/crc.h>
#include <cyg/compress/zlib.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_CONF

#define FLASH_CONF_BOARD_SIZE 1024       /* 1k board configuration */
#define FLASH_CONF_LOCAL_SIZE (128*1024) /* Local section size */

/* Board configuration signatures */
#define FLASH_CONF_BOARD_SIG      "#@(#)VtssConfig\n"
#define FLASH_CONF_MAC_SIG        "MAC="
#define FLASH_CONF_BOARD_ID_SIG   "BOARDID="
#define FLASH_CONF_BOARD_TYPE_SIG "BOARDTYPE="

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Variables containing address and size of where to retrieve/store configuration from/in the flash */
static flash_mgmt_section_info_t conf_flash_info_local;
static flash_mgmt_section_info_t conf_flash_info_stack;

static cyg_flag_t control_flags; /* CONF thread control */

/* Global structure */
static conf_global_t conf;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "conf",
    .descr     = "Configuration"
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
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Common functions                                                        */
/****************************************************************************/

#if VTSS_TRACE_ENABLED
#define CONF_CRIT_ENTER()  critd_enter(&conf.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CONF_CRIT_EXIT()   critd_exit( &conf.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else 
#define CONF_CRIT_ENTER()  critd_enter(&conf.crit)
#define CONF_CRIT_EXIT()   critd_exit( &conf.crit)
#endif

/* Update flash section info */
static void conf_flash_get_section_info(void)
{
  if(!flash_mgmt_lookup("conf", &conf_flash_info_local)) {
    T_E("Unable to locate 'conf' section in flash");
  }

  if(!flash_mgmt_lookup("stackconf", &conf_flash_info_stack)) {
    T_E("Unable to locate 'stackconf' section in flash");
  }
}

/* Get base address and size of Flash section */
static cyg_flashaddr_t conf_flash_section_base(conf_sec_t sec, ulong *size)
{
    /* Global section is located at the beginning */
    cyg_flashaddr_t base = conf_flash_info_stack.base_fladdr;
    *size = (conf_flash_info_stack.size_bytes - FLASH_CONF_LOCAL_SIZE);

    /* Local section is located at the end */
    if (sec == CONF_SEC_LOCAL) {
        base += *size;
        *size = FLASH_CONF_LOCAL_SIZE;
    }

    return base;
}

/* Write data to flash, optionally erasing */
static int conf_flash_write(cyg_flashaddr_t dest, const void *data, size_t data_len, int erase, int erase_len)
{
    cyg_flashaddr_t base, addr;
    int   rc;

    /* Check that the destination area is valid */
    addr = dest;
    base = conf_flash_info_local.base_fladdr;
    if (addr >= base && addr < (base + conf_flash_info_local.size_bytes)) {
        /* Inside board section */
    } else {
        base = conf_flash_info_stack.base_fladdr;
        if (addr < base || addr > (base + conf_flash_info_stack.size_bytes)) {
            /* Outsize stack section */
            T_E("illegal dest: 0x%08x", dest);
            return -1;
        }
    }
    
    /* Erase flash */
    if (erase && ((rc = control_flash_erase(dest, erase_len)) != FLASH_ERR_OK)) {
        return -1;
    }

    /* Program flash */
    if ((rc = control_flash_program(dest, data, data_len)) != FLASH_ERR_OK) {
        return -1;
    }
    return 0;
}

/****************************************************************************/
/*  Board configuration                                                     */
/****************************************************************************/

int conf_mgmt_board_get(conf_board_t *board)
{
    T_D("enter");

    CONF_CRIT_ENTER();
    *board = conf.board;
    CONF_CRIT_EXIT();
    
    T_D("exit");
    return 0;
}

/* Internal conf_mgmt_board_set function */
static int conf_mgmt_board_set_(conf_board_t *board) 
{
    char buf[128];
    
    /* Set binary config */
    if (&conf.board != board)
        conf.board = *board;

    /* Stringify config */
    sprintf(buf, "%s%s%02x:%02x:%02x:%02x:%02x:%02x\n%s%u\n%s%d\n",
            FLASH_CONF_BOARD_SIG,
            FLASH_CONF_MAC_SIG,
            board->mac_address[0],
            board->mac_address[1],
            board->mac_address[2],
            board->mac_address[3],
            board->mac_address[4],
            board->mac_address[5],
            FLASH_CONF_BOARD_ID_SIG,
            board->board_id,
            FLASH_CONF_BOARD_TYPE_SIG,
            board->board_type);            

    /* Save to FLASH */
    int ret = conf_flash_write(conf_flash_info_local.base_fladdr, buf, strlen(buf) + 1, 
                               1, conf_flash_info_local.size_bytes);
    return ret;
}

/* Set board configuration */
int conf_mgmt_board_set(conf_board_t *board)
{
    int rc;
    
    T_D("enter");

    CONF_CRIT_ENTER();
    rc = conf_mgmt_board_set_(board);
    CONF_CRIT_EXIT();

    T_D("exit");
    return rc;
}

/* Get MAC address (index 0 is the base MAC address). */
int conf_mgmt_mac_addr_get(uchar *mac, uint index)
{

    T_N("enter");
    if (index > VTSS_PORTS) {
        T_E("index %d out of range", index);
        return -1;
    }
    
    CONF_CRIT_ENTER();
    misc_instantiate_mac(mac, conf.board.mac_address, index);
    CONF_CRIT_EXIT();
    
    T_N("exit");
    return 0;
}

static void conf_board_start(void)
{
    char         *base, *p;
    char         *sig = FLASH_CONF_BOARD_SIG;
    char         *mac_sig = FLASH_CONF_MAC_SIG;
    char         *board_sig = FLASH_CONF_BOARD_ID_SIG;
    char         *boardtype_sig = FLASH_CONF_BOARD_TYPE_SIG;
    conf_board_t *board = &conf.board;
    BOOL         mac_found = FALSE, board_found = FALSE, boardtype_found = FALSE;
    
    T_D("enter");
    if ((base = VTSS_MALLOC(FLASH_CONF_BOARD_SIZE)) &&
        (CYG_FLASH_ERR_OK == cyg_flash_read(conf_flash_info_local.base_fladdr, base, FLASH_CONF_BOARD_SIZE, NULL )) &&
        strncmp(sig, base, strlen(sig)) == 0) {
        base[FLASH_CONF_BOARD_SIZE-1] = 0; /* Zero terminate */
        if((p = strstr(base, mac_sig))) {
            uint i, mac[6];
            p += strlen(mac_sig);
            if (sscanf(p, "%2x:%2x:%2x:%2x:%2x:%2x",
                       &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
                for (i = 0; i < 6; i++)
                    board->mac_address[i] = (mac[i] & 0xff);
                T_D("Board MAC address %02x:%02x:%02x:%02x:%02x:%02x", 
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                mac_found = TRUE;
            }
        }
        if((p = strstr(base, board_sig))) {
            ulong board_id;
            p += strlen(board_sig);
            if (sscanf(p, "%d", &board_id) == 1) {
                board->board_id = board_id;
                T_D("Board ID: %d", board_id);
                board_found = TRUE;
            }
        }
        if((p = strstr(base, boardtype_sig))) {
            p += strlen(boardtype_sig);
            if (sscanf(p, "%d", &board->board_type) == 1) {
                T_D("Board Type: %d", board->board_type);
                boardtype_found = TRUE;
            }
        }
    } else {
        T_D("Invalid configuration detected (Signature Check Failed)");
    }
    if(base) {
        VTSS_FREE(base);
        base = NULL;
    }

    if (!mac_found) {
        int i;
        T_W("MAC address not found, using default");
        for (i = 0; i < 6; i++)
            board->mac_address[i] = (i == 1 ? 0x01 : i == 2 ? 0xc1 : 0x00);
    }

    if (!board_found) {
        T_W("Board ID not found, using default");
        board->board_id = 0;
    }

    if(!boardtype_found) {
        T_D("Board Type not found, probing enabled");
        board->board_type = 0;
    }

    if (!mac_found && !board_found)
        conf.board_changed = 1;

    T_D("exit");
}

/****************************************************************************/
/*  Application configuration                                                */
/****************************************************************************/

/* Check if section is valid */
static BOOL conf_sec_valid(conf_sec_t sec)
{
    /* Check that the section is valid */
    if (sec >= CONF_SEC_CNT) {
        T_E("illegal sec: %d", sec);
        return 0;
    }
    return 1;
}

vtss_rc conf_mgmt_sec_blk_get(conf_sec_t sec, conf_blk_id_t id, 
                              conf_mgmt_blk_t *mgmt_blk, BOOL next)
{
    conf_blk_t *blk;

    if (!conf_sec_valid(sec))
        return -1;

    CONF_CRIT_ENTER();
    for (blk = conf.section[sec].blk_list; blk != NULL; blk = blk->next) {
        if ((blk->hdr.id == id && !next) || (blk->hdr.id > id && next))
            break;
    }
    if (blk != NULL) {
        mgmt_blk->id = blk->hdr.id;
        mgmt_blk->size = blk->hdr.size;
        mgmt_blk->data = blk->data;
        mgmt_blk->crc = blk->crc;
        mgmt_blk->change_count = blk->change_count;
        sprintf(mgmt_blk->name, "%s", 
                blk->hdr.id < (sizeof(conf_blk_name)/sizeof(char *)) &&
                conf_blk_name[blk->hdr.id] != NULL ? conf_blk_name[blk->hdr.id] : "?");
    }
    CONF_CRIT_EXIT();
    
    return (blk == NULL ? -1 : VTSS_OK);
}

/* Get module configuration */
vtss_rc conf_mgmt_conf_get(conf_mgmt_conf_t *data)
{
    CONF_CRIT_ENTER();
    *data = conf.conf;
    CONF_CRIT_EXIT();

    return VTSS_OK;
}

static void conf_sec_changed(conf_section_t *section)
{
    section->changed = 1;
    cyg_flag_setbits(&control_flags, CONF_THREAD_FLAG_COMMIT);
}

void conf_blk_crc_update(conf_section_t *section, conf_blk_t *blk) 
{
    ulong crc = cyg_crc32(blk->data, blk->hdr.size);

    if (crc != blk->crc) {
        /* Block changed, mark section changed and lock reset */
        T_D("section: %s, id: %d changed, size: %u", section->name, blk->hdr.id,  blk->hdr.size);
        blk->crc = crc;
        blk->change_count++;
        conf_sec_changed(section);
    }
}

/* Set module configuration */
vtss_rc conf_mgmt_conf_set(conf_mgmt_conf_t *data)
{
    conf_sec_t     sec;
    conf_section_t *section;
    conf_blk_t     *blk;

    CONF_CRIT_ENTER();
    if (data->change_detect && !conf.conf.change_detect) {
        /* Enable change detection */
        for (sec = CONF_SEC_LOCAL; sec < CONF_SEC_CNT; sec++) {
            section = &conf.section[sec];
            for (blk = section->blk_list; blk != NULL; blk = blk->next) {
                conf_blk_crc_update(section, blk);
            }
        }
    }
    conf.conf = *data;
    CONF_CRIT_EXIT();

    return VTSS_OK;
}

static conf_blk_t *conf_blk_lookup(conf_sec_t sec, conf_blk_id_t id)
{
    conf_blk_t *blk;
    
    for (blk = conf.section[sec].blk_list; blk != NULL; blk = blk->next) {
        if (blk->hdr.id == id)
            break;
    }
    return blk;
}

/* Internal conf_create function */
static conf_blk_t *conf_create_(conf_sec_t sec, conf_blk_id_t id, ulong size)
{
    conf_blk_t     *blk, *prev, *new;
    conf_section_t *section;
    ulong          max_size;

    section = &conf.section[sec];
    T_D("enter, section: %s, id: %d, size: %d", section->name, id, size);

    max_size = (sec == CONF_SEC_LOCAL ? CONF_BLK_MAX_LOCAL : CONF_BLK_MAX_GLOBAL);
    if (size >= max_size) {
        T_E("illegal block size: %d, max: %d, id: %d", size, max_size, id);
        return NULL;
    }

    /* Look for block or insertion point */
    for (blk = section->blk_list, prev = NULL; blk != NULL; prev = blk, blk = blk->next) {
        if (blk->hdr.id == id) {
            /* Found block ID */
            if (blk->hdr.size == size) {
                T_D("exit, unchanged size");
                return blk;
            }
            T_D("changed size, old size: %d", blk->hdr.size);
            break;
        }
        if (blk->hdr.id > id) {
            /* Found insertion point */
            break;
        }
    }
    
    /* Delete block */
    if (size == 0) {
        if (blk != NULL && blk->hdr.id == id) {
            if (prev == NULL)
                section->blk_list = blk->next;
            else
                prev->next = blk->next;
            VTSS_FREE(blk);

            /* Mark section changed and lock reset */
            conf_sec_changed(section);
        }
        T_D("exit, block deleted");
        return NULL;
    }
    
    /* Create/resize the block */
    if ((new = (conf_blk_t *)VTSS_MALLOC(sizeof(conf_blk_t) + size)) == NULL) {
        T_E("exit, malloc failed");
        return NULL;
    }

    new->hdr.id = id;
    new->hdr.size = size;
    new->data = (new + 1);
    new->change_count = 0;
    new->crc = 0;

    memset(new->data, 0, size);
    if (blk != NULL && blk->hdr.id == id) {
        /* Resizing existing block */
        new->next = blk->next;
        new->change_count = blk->change_count;
        memcpy(new->data, blk->data, blk->hdr.size > size ? size : blk->hdr.size);
        VTSS_FREE(blk);
    } else {
        new->next = blk;
    }
    if (prev == NULL)
        section->blk_list = new;
    else
        prev->next = new;
                
    T_D("exit, new/resized block");
    return new;
}

/* Create/resize configuration block, returns NULL on error */
void *conf_sec_create(conf_sec_t sec, conf_blk_id_t id, ulong size)
{
    conf_blk_t *blk;

    if (!conf_sec_valid(sec))
        return NULL;

    CONF_CRIT_ENTER();
    blk = conf_create_(sec, id, size);
    CONF_CRIT_EXIT();
    
    return (blk == NULL ? NULL : blk->data);
}

/* Open configuration block for read/write, returns NULL on error */
void *conf_sec_open(conf_sec_t sec, conf_blk_id_t id, ulong *size)
{
    conf_blk_t *blk;
    
    if (!conf_sec_valid(sec))
        return NULL;

    CONF_CRIT_ENTER();
    T_D("enter, section: %s, id: %d", conf.section[sec].name, id);
    blk = conf_blk_lookup(sec, id);
    CONF_CRIT_EXIT();
    if (blk != NULL && size != NULL)
        *size = blk->hdr.size;
    T_D("exit, block %sfound", blk == NULL ? "not " : "");
    return (blk == NULL ? NULL : blk->data);
}

/* Close configuration block */
void conf_sec_close(conf_sec_t sec, conf_blk_id_t id)
{
    conf_blk_t     *blk;
    conf_section_t *section;

    if (!conf_sec_valid(sec))
        return;

    CONF_CRIT_ENTER();
    section = &conf.section[sec];
    T_D("enter, section: %s, id: %d", section->name, id);

    /* Search for block in RAM */
    if (conf.conf.change_detect && (blk = conf_blk_lookup(sec, id)) != NULL) {
        /* Block exists, check if CRC has changed */
        conf_blk_crc_update(section, blk);
    }
    
    CONF_CRIT_EXIT();
    T_D("exit");
}

/* Get configuration section information */
void conf_sec_get(conf_sec_t sec, conf_sec_info_t *info)
{
    conf_section_t *section;

    if (!conf_sec_valid(sec))
        return;

    CONF_CRIT_ENTER();
    section = &conf.section[sec];
    T_D("enter, section: %s", section->name);
    info->save_count = section->save_count;
    CONF_CRIT_EXIT();
    
    T_D("exit");
}

/* Renew configuration section */
void conf_sec_renew(conf_sec_t sec)
{
    conf_section_t *section;
    
    if (!conf_sec_valid(sec))
        return;

    CONF_CRIT_ENTER();
    section = &conf.section[sec];
    T_D("enter, section: %s", section->name);
    /* Change section save count and force save */
    conf_sec_changed(section);      
    section->timer_started = 0;
    section->save_count = 0;
    CONF_CRIT_EXIT();
    
    T_D("exit");
}

/* Create/resize local section configuration block, returns NULL on error */
void *conf_create(conf_blk_id_t id, ulong size)
{
    return conf_sec_create(CONF_SEC_LOCAL, id, size);
}

/* Open local section configuration block for read/write, returns NULL on error */
void *conf_open(conf_blk_id_t id, ulong *size)
{
    return conf_sec_open(CONF_SEC_LOCAL, id, size);
}

/* Close local section configuration block */
void conf_close(conf_blk_id_t id)
{
    conf_sec_close(CONF_SEC_LOCAL, id);
}

/* Force immediate configuration flushing to flash (iff pending) */
void conf_flush(void)
{
    cyg_flag_setbits(&control_flags, CONF_THREAD_FLAG_FLUSH);
}

static ulong conf_hdr_version(conf_sec_t sec)
{
    return (conf_sec_valid(sec) ? 
            (sec == CONF_SEC_LOCAL ? CONF_HDR_VERSION_LOCAL : CONF_HDR_VERSION_GLOBAL) : 0);
}

/* Read section from Flash or message buffer */
static void conf_sec_read(conf_sec_t sec, uchar *ram_base, size_t size) 
{
    conf_section_t *section;
    conf_hdr_t     hdr;
    conf_blk_hdr_t blk_hdr;
    conf_blk_t     *blk;
    BOOL           flash;
    u32            version;
    unsigned long  data_size, max_size;
    uchar          *p, *data;
    int            rc = Z_OK;
    cyg_flashaddr_t flash_base = 0;

    if (!conf_sec_valid(sec))
        return;
    section = &conf.section[sec];
    T_D("enter, section: %s, ram_base: %p, size: %d", section->name, ram_base, size);

    /* Get Flash base address and size */
    if((flash = (ram_base == NULL))) {
        flash_base = conf_flash_section_base(sec, &size);
        T_D("flash, using base: 0x%08x, size: %d", flash_base, size);
        /* Read header */
        if(control_flash_read(flash_base, &hdr, sizeof(hdr)) != CYG_FLASH_ERR_OK)
            return;
    } else {
        /* Copy header */
        memcpy(&hdr, ram_base, sizeof(hdr));
    }
    
    /* Check cookie */
    if (hdr.cookie != CONF_HDR_COOKIE_UNCOMP && hdr.cookie != CONF_HDR_COOKIE_COMP) {
        T_W("illegal cookie[%d] at 0x%08x: 0x%08x", sec, (flash ? flash_base : (ulong) ram_base) , hdr.cookie);
        if (flash)
            conf_sec_changed(section); /* Force flash update */
        return;
    }

    /* Check version */
    version = conf_hdr_version(sec);
    if (hdr.version != version) {
        T_W("illegal version[%d] at 0x%08x: 0x%08x, expected: 0x%08x", 
            sec, (flash ? flash_base : (ulong) ram_base), hdr.version, version);
        return;
    }
    
    /* Check length */
    max_size = CONF_SIZE_MAX;
    if (hdr.size > max_size) {
        T_W("illegal size: %u, max: %lu", hdr.size, max_size);
        return;
    }
    
    /* Allocate data buffer */
    if ((data = VTSS_MALLOC(max_size)) == NULL) {
        T_E("malloc failed, size: %lu", max_size);
        return;
    }
    section->save_count = hdr.save_count;

    if (flash) {
        if((ram_base = VTSS_MALLOC(hdr.size)) == NULL) {
            T_E("malloc failed, size: %u", hdr.size);
            VTSS_FREE(data);
            return;
        }
        if(control_flash_read(flash_base, ram_base, hdr.size) != CYG_FLASH_ERR_OK) {
            VTSS_FREE(ram_base);
            VTSS_FREE(data);
            return;
        }
    }

    /* Read data */
    section->crc = cyg_crc32(ram_base + sizeof(hdr), hdr.size - sizeof(hdr));
    p = (ram_base + sizeof(hdr));
    data_size = (hdr.size - sizeof(hdr));
    if (hdr.cookie == CONF_HDR_COOKIE_UNCOMP) {
        /* Uncompressed data */
        T_I("uncompressed data, %lu bytes", data_size);
        memcpy(data, p, data_size);
    } else {
        /* Compressed data */
        T_I("compressed data, %lu bytes", data_size);
        if (!flash)
            CONF_CRIT_EXIT(); /* Leave critical region while uncompressing */
        rc = uncompress(data, &max_size, p, data_size);
        if (!flash)
            CONF_CRIT_ENTER();
        data_size = max_size;
    } 

    /* If uncompress failed, we return here after unlocking flash */
    if (rc != Z_OK) {
        T_E("uncompress failed, rc: %d", rc);
        VTSS_FREE(data);
        if(flash)
            VTSS_FREE(ram_base);
        return;
    }
    
    /* Copy block data */
    p = data;
    while (p < (data + data_size)) {
        memcpy(&blk_hdr, p, sizeof(blk_hdr));
        if ((blk = conf_create_(sec, blk_hdr.id, blk_hdr.size)) != NULL) {
            memcpy(blk->data, p + sizeof(blk_hdr), blk_hdr.size);
            blk->crc = cyg_crc32(blk->data, blk->hdr.size);
        }
        p += (sizeof(blk_hdr) + blk_hdr.size);
    }
    VTSS_FREE(data);
    if(flash)
        VTSS_FREE(ram_base);

    T_D("exit");
}

static conf_msg_conf_set_req_t *conf_msg_build(conf_sec_t sec, ulong *msg_size)
{
    conf_section_t          *section;
    conf_hdr_t              hdr;
    conf_blk_t              *blk;
    uLong                   data_size, size;
    uchar                   *p, *tmp;
    conf_msg_conf_set_req_t *msg;
    int                     rc;
    
    /* Calculate data size */
    section = &conf.section[sec];
    data_size = 0;
    for (blk = section->blk_list; blk != NULL; blk = blk->next)
        data_size += (sizeof(blk->hdr) + blk->hdr.size);

    if(data_size > CONF_SIZE_MAX) { /* Warn about upcoming read problems */
        T_E("Configuration total: %lu bytes, read linit %d", data_size, CONF_SIZE_MAX);
    }

    /* Allocate message buffer */
    size = (sizeof(*msg) + sizeof(hdr) + data_size);
    /* Allow for buffer growth (can happen for small buffer sizes) */
#define COMPRESS_OVERHEAD 1024
    if ((msg = VTSS_MALLOC(size + COMPRESS_OVERHEAD)) == NULL) {
        T_E("malloc failed, size: %lu", size);
        return NULL;
    }

    if (data_size == 0) {
        /* No data blocks, save uncompressed */
        hdr.cookie = CONF_HDR_COOKIE_UNCOMP;
        size = 0;
    } else {
        /* Save compressed */
        hdr.cookie = CONF_HDR_COOKIE_COMP;
        
        /* Allocate temporary buffer for uncompressed data */
        if ((tmp = VTSS_MALLOC(data_size)) == NULL) {
            T_E("malloc failed, size: %lu", data_size);
            VTSS_FREE(msg);
            return NULL;
        }

        /* Copy blocks */
        p = tmp;
        for (blk = section->blk_list; blk != NULL; blk = blk->next) {
            size = (sizeof(blk->hdr) + blk->hdr.size);
            memcpy(p, &blk->hdr, size);
            p += size;
        }
        
        /* Compress data */
        size = data_size + COMPRESS_OVERHEAD; /* Output buffer lenghth */
        CONF_CRIT_EXIT(); /* Leave critical region while compressing */
        rc = compress(&msg->data[sizeof(hdr)], &size, tmp, data_size);
        CONF_CRIT_ENTER();
        VTSS_FREE(tmp);
        if (rc != Z_OK) {
            T_E("compress failed, rc: %d", rc);
            VTSS_FREE(msg);
            return NULL;
        }
    }

    /* Fill out header */
    msg->msg_id = CONF_MSG_CONF_SET_REQ;
    hdr.size = (size + sizeof(hdr));
    hdr.save_count = section->save_count;
    hdr.version = conf_hdr_version(sec);
    memcpy(&msg->data[0], &hdr, sizeof(hdr));

    *msg_size = hdr.size;
    return msg;
}

static void conf_flash_save(conf_sec_t sec)
{
    conf_msg_conf_set_req_t *msg;
    uchar                   *p;
    cyg_flashaddr_t         base;
    ulong                   size, msg_size = 0, hdr_len = sizeof(conf_hdr_t);

    T_I("enter, sec: %d", sec);

    /* Allocate and build message */
    if ((msg = conf_msg_build(sec, &msg_size)) != NULL) {
        /* Update section CRC */
        base = conf_flash_section_base(sec, &size);
        size = (msg_size - hdr_len);
        p = &msg->data[0];
        conf.section[sec].crc = cyg_crc32(p + hdr_len, size);
        
        if (conf.conf.flash_save) {
            /* Leave critical region and write to Flash (header is written last) */
            CONF_CRIT_EXIT();
            if (conf_flash_write(base + hdr_len, p + hdr_len, size, 1, size) == 0)
                conf_flash_write(base, p, hdr_len, 0, 0);
            CONF_CRIT_ENTER();
        } else {
            /* Flash save is disabled (but section CRC has been updated) */
            T_I("no flash save");
        }
        VTSS_FREE(msg);
    }

    T_I("exit, size: %u", msg_size);
}

/****************************************************************************/
/*  Stack messages                                                          */
/****************************************************************************/

/* Receive message */
static BOOL conf_msg_rx(void *contxt, const void * const rx_msg, const size_t len, 
                        const vtss_module_id_t modid, ulong id)
{
    conf_msg_conf_set_req_t *msg;
    conf_hdr_t              hdr;
    conf_sec_t              sec;
    conf_section_t          *section;
    ulong                   save_count;
    
    T_D("id: %d, len: %zu", id, len);

    T_D_HEX(rx_msg, 64);

    /* Check that we are a slave switch */
    if (msg_switch_is_master()) {
        T_W("master");
        return TRUE;
    }

    /* Check message ID */
    msg = (conf_msg_conf_set_req_t *)rx_msg;
    if (msg->msg_id != CONF_MSG_CONF_SET_REQ) {
        T_W("illegal msg_id: %d", msg->msg_id);
        return TRUE;
    }

    /* Check message length */
    memcpy(&hdr, &msg->data[0], sizeof(hdr));
    if ((sizeof(*msg) + hdr.size) != len) {
        T_W("length mismatch, len: %zu, hdr.size: %d", len, hdr.size);
        return TRUE;
    }
    
    /* Compare with flash CRC to avoid unnecessary write operations */
    sec = CONF_SEC_GLOBAL;
    section = &conf.section[sec];
    if (section->crc == cyg_crc32(&msg->data[sizeof(hdr)], hdr.size - sizeof(hdr))) {
        T_D("section not changed");
        return TRUE;
    }
    
    /* Read blocks into global section and force update */
    CONF_CRIT_ENTER();
    save_count = section->save_count;
    conf_sec_read(sec, &msg->data[0], hdr.size);
    section->save_count = save_count;
    conf_sec_changed(section);
    CONF_CRIT_EXIT();
    
    return TRUE;
}

/* Register for messages */
static vtss_rc conf_stack_register(void)
{
    msg_rx_filter_t filter;    

    memset(&filter, 0, sizeof(filter));
    filter.cb = conf_msg_rx;
    filter.modid = VTSS_MODULE_ID_CONF;
    return msg_rx_filter_register(&filter);
}

/* Release message buffer */
static void conf_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    T_D("enter");

    CONF_CRIT_ENTER();
    if (conf.msg_tx_count == 0) {
        T_E("buffer already free");
    } else {
        conf.msg_tx_count--;
        if (conf.msg_tx_count == 0) {
            VTSS_FREE(msg);
        }
    }
    CONF_CRIT_EXIT();

    T_D("exit");
}

/* Copy global section to slave switches */
static BOOL conf_stack_copy(void)
{
    vtss_isid_t             isid;
    ulong                   msg_size;
    conf_msg_conf_set_req_t *msg;
    
    T_I("enter");

    if (!conf.conf.stack_copy) {
        T_I("exit, no stack copy");
        return 1;
    }

    /* Only copy if master */
    if (!msg_switch_is_master()) {
        T_D("not master");
        return 1;
    }

    /* Only copy if at least one slave exists */
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (conf.isid_copy[isid - VTSS_ISID_START] &&
            msg_switch_exists(isid) && !msg_switch_is_local(isid))
            break;
    }
    if (isid == VTSS_ISID_END) {
        T_D("no slave switches");
        return 1;
    }

    /* Only copy if no messages are currently being sent */
    if (conf.msg_tx_count) {
        T_D("buffer not ready");
        return 0;
    }

    /* Allocate and build message */
    if ((msg = conf_msg_build(CONF_SEC_GLOBAL, &msg_size)) == NULL)
        return 1;
    msg_size += sizeof(*msg);

    /* Send copy to all managed slave switches */
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (conf.isid_copy[isid - VTSS_ISID_START] &&
            msg_switch_exists(isid) && !msg_switch_is_local(isid)) {
            T_I("Tx to isid %d, size: %u", isid, msg_size);
            conf.msg_tx_count++;
            msg_tx_adv(NULL, conf_msg_tx_done, MSG_TX_OPT_DONT_FREE, 
                       VTSS_MODULE_ID_CONF, isid, msg, msg_size);
        }
    }

    /* Free buffer if no slaves were found */
    if (!conf.msg_tx_count) {
        T_D("no slaves found");
        VTSS_FREE(msg);
    } 

    T_I("exit");
    
    return 1;
}

/* Start application configuration */
static void conf_appl_start(void) 
{
    conf_sec_t     sec;
    conf_section_t *section;

    T_D("enter");

    /* Read all sections from Flash */
    for (sec = CONF_SEC_LOCAL; sec < CONF_SEC_CNT; sec++) {
        section = &conf.section[sec];
        section->blk_list = NULL;
        section->name = (sec == CONF_SEC_LOCAL ? "Local" : "Global");
        conf_sec_read(sec, NULL, 0);
    }

    T_D("exit");
}

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

static void conf_thread(cyg_addrword_t data)
{
    conf_sec_t              sec;
    conf_section_t          *section;
    int                     change_count;
    BOOL                    global_changed = 0;
    BOOL                    reset_locked = 0;
    vtss_isid_t             isid;
    ulong                   timer;
    
    T_D("enter, data: %d", data);

    /* Initialize stack message interface */
    conf.msg_tx_count = 0;
    conf_stack_register();

    for (;;) {
        BOOL do_flush;
        cyg_tick_count_t wakeup = cyg_current_time() + (100/ECOS_MSECS_PER_HWTICK);
        cyg_flag_value_t flags;
        do_flush = 0;
        while ((flags = cyg_flag_timed_wait(&control_flags, 0xffff, 
                                            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, wakeup))) {
            if (flags & CONF_THREAD_FLAG_COMMIT) {
                if (!reset_locked) {
                    T_I("locking reset");
                    control_system_flash_lock();
                    reset_locked = 1;
                }
            }
            if (flags & CONF_THREAD_FLAG_FLUSH) {
                T_I("Forced flush");
                do_flush = 1;
                break;          /* Fast forward */
            }
        }

        change_count = 0;
        CONF_CRIT_ENTER();

        /* Check if board configuration must be saved */
        if (conf.board_changed) {
            conf.board_changed = 0;
            conf_mgmt_board_set_(&conf.board);
        }

        for (sec = CONF_SEC_LOCAL; sec < CONF_SEC_CNT; sec++) {
            section = &conf.section[sec];
            if (!section->changed && !section->copy) /* Skip unchanged sections */
                continue;
            
            change_count++;
            if (section->timer_started) {
                if (do_flush || VTSS_MTIMER_TIMEOUT(&section->mtimer)) {
                    section->timer_started = 0;
                    section->copy = 0;
                    if (sec == CONF_SEC_GLOBAL)
                        global_changed = 1;
                    if (section->changed) {
                        T_I("section %s %s, saving to flash", 
                            section->name, do_flush ? "flush" : "timeout");
                        section->changed = 0;
                        section->save_count++;
                        if (!reset_locked) {
                            /* If reset not already locked, do it now before updating Flash */
                            T_I("locking reset");
                            control_system_flash_lock();
                            reset_locked = 1;
                        }
                        conf_flash_save(sec);

                        /* If global section changed, copy to all switches */
                        if (sec == CONF_SEC_GLOBAL)
                            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++)
                                conf.isid_copy[isid - VTSS_ISID_START] = 1;
                    }
                }
            } else {
                /* To protect the Flash, the timer grows with the number of save operations */
                timer = (1<<(section->save_count/10000));
                VTSS_MTIMER_START(&section->mtimer, 1000*timer); 
                section->timer_started = 1;
                T_I("section %s changed, starting timer of %u seconds", section->name, timer);
            }
        } /* Section loop */
        
        /* Copy global section, if changed */
        if (global_changed && conf_stack_copy()) {
            global_changed = 0;
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++)
                conf.isid_copy[isid - VTSS_ISID_START] = 0;
        }
        
        if (change_count == 0 && reset_locked && conf.msg_tx_count == 0) {
            T_I("unlocking reset");
            control_system_flash_unlock();
            reset_locked = 0;
        }
        CONF_CRIT_EXIT();
    } /* Forever loop */
}

vtss_rc conf_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);
    
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Update our own static variables with info about where the configuration is stored. */
        conf_flash_get_section_info();
        
        /* Initialize configuration */
        conf.conf.stack_copy = 1;
        conf.conf.flash_save = 1;
        conf.conf.change_detect = 1;

        critd_init(&conf.crit, "conf.crit", VTSS_MODULE_ID_CONF, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        /* The conf critd's maximum lock time should be higher than what it takes
         * to update the firmware image to flash, and on some old flashes, this may
         * take up to a hundred seconds, so we increase this particular semaphore's
         * lock time to 5 minutes.
         */
        if (conf.crit.max_lock_time < 300)
            /* Only change it if we wouldn't set the new lock time to something smaller than the default. */
            conf.crit.max_lock_time = 300;

        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++)
            conf.isid_copy[isid - VTSS_ISID_START] = 0;
        
        /* Read in config data in order to serve configuration early */
        conf_board_start();
        conf_appl_start();

        CONF_CRIT_EXIT();       /* NB: config data *is* ready, so unlock now */
    
        cyg_flag_init( &control_flags );
        cyg_thread_create(THREAD_BELOW_NORMAL_PRIO,
                          conf_thread, 
                          0, 
                          "Configuration", 
                          conf.thread_stack, 
                          sizeof(conf.thread_stack),
                          &conf.thread_handle,
                          &conf.thread_block);
        cyg_thread_resume(conf.thread_handle);

#ifdef VTSS_SW_OPTION_VCLI
        conf_cli_init();
#endif
        break;
    case INIT_CMD_START:
        break;
    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF, isid: %d, flags: 0x%08x", isid, data->flags);
        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP");
        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD, isid: %d", isid);
        if (VTSS_ISID_LEGAL(isid) && !msg_switch_is_local(isid)) {
            /* Start configuration copy */
            CONF_CRIT_ENTER();
            conf.isid_copy[isid - VTSS_ISID_START] = 1;
            conf.section[CONF_SEC_GLOBAL].copy = 1;
            CONF_CRIT_EXIT();
        }
        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
