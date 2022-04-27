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

#include "main.h"

#if defined(CYGPKG_FS_RAM) && defined(VTSS_SW_OPTION_ICFG)

#include "os_file_api.h"
#include "critd_api.h"

#include "vtss_trace_api.h"
#include "conf_api.h"
#include "misc_api.h"

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SYSTEM /* Pick one... */
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SYSTEM /* Pick one... */

#define OS_FILE_FILES_MAX             4

typedef struct {
    unsigned long version;      /* Block version */
#define OSFILE_CONF_VERSION 2

    /* Metadata for r/w files */
    struct {
        char   name[NAME_MAX];
        size_t size;
        time_t mtime;
    } meta[OS_FILE_FILES_MAX];
    u8 data[];
} file_blk_flash_t;

static cyg_mutex_t osf_mutex;   /* Global module/API protection */

#define FILE_LOCK()          (void) cyg_mutex_lock(&osf_mutex)
#define FILE_UNLOCK()        cyg_mutex_unlock(&osf_mutex)

static void os_file_readdata(file_blk_flash_t *flash, int nfiles)
{
    int i;
    u8 *ptr = &flash->data[0];
    for (i = 0; i < nfiles; i++) {
        int fd = open(flash->meta[i].name, O_RDONLY, 0);
        ssize_t act_read;
        if (fd >= 0) {
            if ((act_read = read(fd, ptr, flash->meta[i].size)) >= 0) { /* In one scoop */
                ptr += act_read;
                if (flash->meta[i].size < (size_t) act_read) {
                    T_W("%s: File shrunk from %zd to %ld", flash->meta[i].name, flash->meta[i].size, act_read);
                    flash->meta[i].size = act_read;
                }
            }
            (void) close(fd);
        } else {
            T_W("%s: %s - skipping file", flash->meta[i].name, strerror(errno));
            memset(&flash->meta[i], 0, sizeof(flash->meta[i]));
        }
    }
}

static void os_file_writefile(const char *name, const u8 *buf, size_t fsize, size_t mtime)
{
    int fd;

    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP));
    if (fd < 0 ) {
        T_E("%s: %s", name, strerror(errno));
        return;
    }

    T_I("%s: %zd bytes", name, fsize);

    if (fsize) {
        ssize_t wrote = write(fd, buf, fsize);
        if (wrote < 0) {
            T_E("%s: %s (%zd bytes) ", name, strerror(errno), fsize);
        } else {
            if (wrote != fsize) {
                T_E("%s: Requested %zd, wrote %lu", name, fsize, wrote);
            }
        }
    }

    /* Hack In absence of utime() */
    if (ioctl(fd, SIOCSHIWAT, mtime) < 0) {
        T_E("%s: %s", name, strerror(errno));
    }

    if (close(fd) < 0) {
        T_E("%s: %s", name, strerror(errno));
    }
}

/*
 * Externally visible API
 */

void os_file_fs2flash(void)
{
    char *dirname = "/";
    int err, files;
    DIR *dirp;
    struct dirent *entry;
    file_blk_flash_t *flash = VTSS_MALLOC(sizeof(*flash)), *nflash;
    size_t size = sizeof(*flash);

    if (!flash) {
        T_E("Unable to persist data struct, size %zd", size);
        return;
    }

    T_D("Reading directory %s", dirname);

    dirp = opendir(dirname);
    if (dirp == NULL) {
        T_E("opendir(%s): %s", dirname, strerror(errno));
        VTSS_FREE(flash);
        return;
    }

    memset(flash, 0, sizeof(*flash));
    flash->version = OSFILE_CONF_VERSION;

    for (files = 0; files < OS_FILE_FILES_MAX && (entry = readdir(dirp)) != NULL; ) {
        char fullname[PATH_MAX];
        struct stat sbuf;
        strcpy(fullname, dirname);
        strcat(fullname, entry->d_name);
        err = stat(fullname, &sbuf);
        if (err < 0) {
            if (errno == ENOSYS) {
                T_E("%s: <no status available>", fullname);
            } else {
                T_E("%s: %s", fullname, strerror(errno));
            }
        } else {
            // Write regular file, unless it's 'default-config'
            if (S_ISREG(sbuf.st_mode) && strcmp(entry->d_name, "default-config")) {
                T_D("File %s: %lu bytes", fullname, sbuf.st_size);
                misc_strncpyz(flash->meta[files].name, fullname, sizeof(flash->meta[files].name));
                flash->meta[files].size = sbuf.st_size;
                flash->meta[files].mtime = sbuf.st_mtime;
                size += sbuf.st_size;
                files++;
            }
        }
    }

    err = closedir(dirp);
    if (err < 0) {
        T_W("closedir: %s", strerror(errno));
    }

    FILE_LOCK();
    if ((nflash = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_OS_FILE_CONF, size))) {
        /* Save metadata, version */
        memcpy(nflash, flash, sizeof(*flash));

        /* Read file contents to data section */
        os_file_readdata(nflash, files);

        /* Sync data to flash (compressed); flush initiates immediate sync. */
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_OS_FILE_CONF);
        conf_flush();
    } else {
        T_E("Unable to store persistent filedata, %zd bytes", size);
    }
    FILE_UNLOCK();

    VTSS_FREE(flash);
}

void os_file_flash2fs(void)
{
    file_blk_flash_t *blk;
    ulong             size;
    FILE_LOCK();
    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_OS_FILE_CONF, &size)) &&
        blk->version == OSFILE_CONF_VERSION) {
        uint i;
        u8 *ptr = &blk->data[0];
        T_D("Config block %u bytes", size);
        for (i = 0; i < ARRSZ(blk->meta); i++)  {
            if (blk->meta[i].name[0]) {
                os_file_writefile(blk->meta[i].name, ptr, blk->meta[i].size, blk->meta[i].mtime);
                ptr += blk->meta[i].size;
            } else {
                T_D("%d: Null entry", i);
            }
        }
    }
    FILE_UNLOCK();
}

vtss_rc os_file_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        cyg_mutex_init(&osf_mutex);
        break;

    case INIT_CMD_MASTER_UP:
        os_file_flash2fs();
        break;

    case INIT_CMD_CONF_DEF:
        break;

    default:
        break;
    }

    return VTSS_RC_OK;
}
#endif  /* CYGPKG_FS_RAM && VTSS_SW_OPTION_ICFG */
