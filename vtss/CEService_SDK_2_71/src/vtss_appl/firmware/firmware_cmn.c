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
#include "firmware.h"
#include "firmware_api.h"
#include "misc_api.h"
#include "control_api.h"
#include "led_api.h"
#include "flash_mgmt_api.h"
#include "version.h"

#include <tftp_support.h>
#include <arpa/inet.h>
#include <cyg/crc/crc.h>
#include <cyg/compress/zlib.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_FIRMWARE

/*
 * Private data
 */

#define FIRMWARE_DEBUG_PAUSE_TIME_MS 30000
static int debug_pause_position;
static BOOL debug_check_same = TRUE;

enum {
    _CPU_TYPE_ARM  = 1,
    _CPU_TYPE_MIPS = 2,
};

#if defined(__ARMEL__)
#define MY_CPU_ARC  _CPU_TYPE_ARM
#elif defined (_MIPS_ARCH_MIPS32)
#define MY_CPU_ARC  _CPU_TYPE_MIPS
#else
#error Unable to determine CPU architecture
#endif

#if defined(VTSS_OPT_VCORE_III)
#define FLASH_BUS_LOCK()   vcoreiii_spi_bus_lock()
#define FLASH_BUS_UNLOCK() vcoreiii_spi_bus_unlock()
#else
#define FLASH_BUS_LOCK()        /* Go away */
#define FLASH_BUS_UNLOCK()      /* Go away */
#endif

/****************************************************************************/
// do_fis_lookup()
// Simply wraps flash_mgmt_fis_lookup() and calls T_D() on success.
/****************************************************************************/
static int do_fis_lookup(const char *name, struct fis_table_entry *pEntry)
{
  int result;
  if((result = flash_mgmt_fis_lookup(name, pEntry)) >= 0) {
    T_D("%d: Flash 0x%08x, size 0x%08lx length %ld, crc 0x%08lx", result, pEntry->flash_base, pEntry->size, pEntry->data_length, pEntry->file_cksum);
  }
  return result;
}

/* Swap managed and managed.bk FIS entries */
int firmware_swap_entries(void)
{
    struct fis_table_entry fis_act, fis_bak;
    int idx_act = 0, idx_bak = 0;
    const char *pri_name = "managed", *alt_name = "managed.bk";
    uchar running[32];
    int rc = 0;

    fis_act.name[0] = fis_bak.name[0] = '\0';
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, &fis_act);
    strncpy(running, fis_act.name, sizeof(running));

    /* See if we can start the swap */
    if (control_system_flash_islock()) {
        return FIRMWARE_ERROR_BUSY;
    }

    if(((idx_act = flash_mgmt_fis_lookup(running, &fis_act)) >= 0) && 
       strcmp(fis_act.name, pri_name) == 0 &&
       ((idx_bak = flash_mgmt_fis_lookup(alt_name, &fis_bak)) >= 0) &&
       strcmp(fis_bak.name, alt_name) == 0) {
        vtss_restart_t restart_type = VTSS_RESTART_WARM;
        T_D("Swapping entries: Active = %d (%s), backup = %d", idx_act, running, idx_bak);
        strcpy(fis_act.name, alt_name);
        strcpy(fis_bak.name, pri_name);
        CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_MODIFY_ENTRY, idx_act, &fis_act);
        CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_MODIFY_ENTRY, idx_bak,  &fis_bak);
        FLASH_BUS_LOCK(); /* Guard against simultaneous SPI bus access */
        CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_START_UPDATE, 0, NULL);
        CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_FINISH_UPDATE, 0, NULL);
        FLASH_BUS_UNLOCK(); /* Guard against simultaneous SPI bus access */
        /* Now restart */
        if(vtss_switch_standalone()) {
            (void) control_system_reset(TRUE, VTSS_USID_ALL, restart_type);
        } else {
            (void) control_system_reset(FALSE, VTSS_USID_ALL, restart_type);
        }
    } else {
        T_W("Unable to swap images - pri = %d = %s, bak = %d = %s", 
            idx_act, fis_act.name, idx_bak, fis_bak.name);
        rc = -1;
    }

    return rc;
}

static const char *bin_findstr(const uchar *haystack, const uchar *haystack_end,
                               const char *needle, size_t nlen)
{
    //diag_printf("%s: start\n", __FUNCTION__);
    while(haystack &&
          haystack < haystack_end &&
          (haystack = memchr(haystack, needle[0], haystack_end - haystack))) {
        if(strncmp(haystack, needle, nlen) == 0)
            return haystack;
        haystack++;             /* Advance past initial hit */
    }
    return NULL;
}

void scan_props_buffer(const uchar *mem, size_t sz_buf,
                       uchar *vers, size_t sz_vers,
                       uchar *date, size_t sz_date)
{
    const uchar *ptr, *mem_end;

    T_I("Consume %zu", sz_buf);

    ptr = mem;
    mem_end = mem + sz_buf;
    while((ptr = bin_findstr(ptr, mem_end, MAGIC_ID, sizeof(MAGIC_ID)-1))) {
        const uchar *id_end;
        T_I("Cookie found: %s", ptr);
        if((id_end = memchr(ptr, '\0', mem_end - ptr))) {
            int id_len = id_end - ptr;
            if (id_len > MAGIC_ID_LEN) {
                if(strncmp(ptr, MAGIC_ID_VERS, MAGIC_ID_LEN) == 0) {
                    T_I("Version found: %s", ptr);
                    strncpy(vers, ptr + MAGIC_ID_LEN, sz_vers);
                } else if(strncmp(ptr, MAGIC_ID_DATE, MAGIC_ID_LEN) == 0) {
                    T_I("Date found: %s", ptr);
                    strncpy(date, ptr + MAGIC_ID_LEN, sz_date);
                }
            }
            /* Advance past string */
            ptr += id_len;
        } else {
            /* Cookie, but unterminated within buffer size */
            break;
        }
    }
}

#define Z_BUFSIZ (1024*512)     /* 0.5 Megs I/O */
#define Z_WINDOW 256            /* Sliding window overlap */
void firmware_scan_props(const struct fis_table_entry *fis, 
                         uchar *vers, size_t sz_vers,
                         uchar *date, size_t sz_date)
{
    z_stream gz;
    u8 *inbuf, *outbuf, *out_decompress;;
    int err;
    cyg_flashaddr_t flash_off, flash_err;
    ulong fl_read, blk_rd;

    vers[0] = date[0] = '\0';
    memset(&gz, 0, sizeof(gz));

    err = inflateInit(&gz);
    if(err) {
        T_W("inflateInit: return %d", err);
        return;
    }

    inbuf = VTSS_MALLOC(Z_BUFSIZ);
    outbuf = VTSS_MALLOC(Z_BUFSIZ + Z_WINDOW);
    out_decompress = outbuf + Z_WINDOW;
    if(inbuf == NULL ||
       outbuf == NULL) {
        T_W("%s: Memory allocation error", __FUNCTION__);
        goto out;
    }

    flash_off = fis->flash_base;
    fl_read = 0;

    memset(outbuf, ' ', Z_WINDOW); /* Initialize window to don't care data */

    while(fl_read < fis->data_length) {
        /* Are we done scanning ? */
        if(vers[0] != '\0' && date[0] != '\0') {
            T_D("firmware scan complete");
            break;
        }

        /* Read next block */
        if((blk_rd = (fis->size - fl_read)) > Z_BUFSIZ)
            blk_rd = Z_BUFSIZ;
        if(CYG_FLASH_ERR_OK == (err = cyg_flash_read(flash_off, inbuf, blk_rd, &flash_err))) {
            T_I("Flash read: %u bytes at 0x%x", blk_rd, flash_off);
            fl_read += blk_rd;
            flash_off += blk_rd;
        } else {
            T_W("Flash read: ret %d at 0x%x", err, flash_err);
            break;
        }

        /* Decompress block completely */
        gz.next_in = inbuf;
        gz.avail_in = blk_rd;
        gz.next_out = out_decompress;
        gz.avail_out = Z_BUFSIZ;
        
        do {
            err = inflate(&gz, Z_SYNC_FLUSH);
            T_I("inflate = %d, gz.avail_out = %u, gz.avail_in = %u", err, gz.avail_out, gz.avail_in);
            if(err == Z_STREAM_END || err == Z_OK) {
                size_t out_len = Z_WINDOW + Z_BUFSIZ - gz.avail_out;
                /* Note: Scan from sliding window start */
                scan_props_buffer(outbuf, out_len, vers, sz_vers, date, sz_date);
                if(err != Z_STREAM_END) { /* Still more? */
                    /* Rewind */
                    gz.next_out = out_decompress;
                    gz.avail_out = Z_BUFSIZ;
                    /* Copy tail of output buffer to sliding window start */
                    memcpy(outbuf, outbuf + (out_len - Z_WINDOW), Z_WINDOW);
                }
            }
        } while(err == Z_OK && gz.avail_in > 0);

        if(err != Z_OK) {
            T_I("firmware scan stopping at %u with error %d", fl_read, err);
            break;
        }
    }

out:
    if(inbuf)
        VTSS_FREE(inbuf);
    if(outbuf)
        VTSS_FREE(outbuf);

    inflateEnd(&gz);
}

/*
 * Initial checks before being able to actually burn the new image.
 * If @pReq is NULL, this is only used in checking, so that
 * we can send an error message to the user in case of error.
 * Otherwise, it's part of the real program-process.
 */
static vtss_rc firmware_flash_system_check_doit(firmware_flash_args_t *pReq,
                                                const char *name,
                                                struct fis_table_entry *entry_to_update,
                                                struct fis_table_entry *entry_of_currently_running,
                                                int *idx_to_update,
                                                int *idx_of_currently_running,
                                                char *name_of_bk_image,
                                                unsigned long flashlen,
                                                unsigned long image_version)
{
    char name_of_currently_running[16];
    struct fis_table_entry redundant_fis_fis_entry;
    cyg_flashaddr_t flash_start;
    size_t flash_size;

    CYG_ASSERT(strlen(name) < 13, "name length"); // Gotta have room for ".bk\0"

    strcpy(name_of_bk_image, name);
    strcat(name_of_bk_image, ".bk");

#ifdef VTSS_SW_OPTION_CLI
    if(pReq) {
      cli_io_printf(pReq->io, "Starting flash update - do not power off device!\n");
    }
#endif //  VTSS_SW_OPTION_CLI

    if(pReq) {
      /* Reset/Firmware/flash interlock */
      control_system_flash_lock();
    }

    if(image_version < 1) {
      // The image_version is the one held in the header of the .dat file.
      // If this version is less than 1, then we cannot update the image
      // if the RedBoot we're running on is version 1.07 or later, because
      // that image will potentially cause the FIS Directory or redundant FIS entries
      // to be overwritten by the conf module because image_version < 1
      // use a fixed flash layout where the MAC address is written to address
      // 0x80FC0000 (also the syslog uses a fixed flash layout in applications
      // with image_version < 1).
      // We check the RedBoot version by looking at the location of the
      // two FIS sections.
      // Get the flash size.
      control_flash_get_info(&flash_start, &flash_size);

      if(do_fis_lookup("Redundant FIS", &redundant_fis_fis_entry) >= 0) {
        // We're indeed running on a RedBoot v. 1.06 or later. If we ran on
        // 1.05 or earlier, then the "Redundant FIS" wouldn't exist, and
        // we wouldn't have a problem with loading an image with version < 1.
        // Now check if this section is within two max-supported-size sectors
        // from the end of the flash. If so, we're running RedBoot v. 1.07.
        // A max-size sector is 128 KBytes.
        if(redundant_fis_fis_entry.flash_base >= (flash_start + flash_size - (2 * 128 * 1024))) {
          return FIRMWARE_ERROR_INCORRECT_IMAGE_VERSION;
        }
      }
    }

    // Attempt to look up the name of the currently running image.
    // This will fail on RedBoot version 1.05 an earlier, but will
    // give a valid name on RedBoot version 1.06 and later.
    // RedBoot v. 1.06 supports dual flash images (and redundant FIS).
    entry_of_currently_running->name[0] = '\0';
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_LOADED_ENTRY, 0, entry_of_currently_running);
    // The above call only fills in the entry_of_currently_running->name. Get it out.
    strcpy(name_of_currently_running, entry_of_currently_running->name);

    if(name_of_currently_running[0] == '\0' || strcmp(name_of_currently_running, name_of_bk_image) == 0) {
      // We can get here for two reasons:
      // If name_of_currently_running[0] == '\0', then the dual-flash-image feature
      // is not supported, either because we're running on RedBoot v. 1.05 or
      // earlier or because the currently running image is loaded through a
      // JTAG probe, which causes RedBoot not to (necessarily) having been executed.
      //
      // If the name of the currently running image is @name.bk, we wish to update
      // the normal image - not the backup image, because that is currently running.
      //
      // In either case update the normal image.
      *idx_to_update = do_fis_lookup(name, entry_to_update);

    } else if(strcmp(name_of_currently_running, name) == 0) {
      // The normal image is loaded. Overwrite the backup image.
      *idx_to_update = do_fis_lookup(name_of_bk_image, entry_to_update);
      if(*idx_to_update < 0) {
        // The backup image was not found. Assume that the customer doesn't
        // want dual-flash-image support, and ask RedBoot for the normal
        // image instead.
        *idx_to_update = do_fis_lookup(name, entry_to_update);
      }
    } else {
      // An unknown image is currently loaded. Bail out.
#ifdef VTSS_SW_OPTION_CLI
      if(pReq) {
        cli_io_printf(pReq->io, "An unknown image (\"%s\") is currently loaded. Unable to update\n", name_of_currently_running);
      }
#endif // VTSS_SW_OPTION_CLI
      return FIRMWARE_ERROR_CURRENT_UNKNOWN;
    }

    if(name_of_currently_running[0] == '\0') {
      // Couldn't update the name_of_currently_running before now, beacuse
      // of the final "else" above.
      // We get here when running on an early RedBoot
      strcpy(name_of_currently_running, name);
    }

    if((*idx_of_currently_running = do_fis_lookup(name_of_currently_running, entry_of_currently_running)) < 0) {
      // Couldn't lookup the currently running. Bail out
#ifdef VTSS_SW_OPTION_CLI
      if(pReq) {
        cli_io_printf(pReq->io, "Currently running image (\"%s\") was not found in flash. Unable to update\n", name_of_currently_running);
      }
#endif // VTSS_SW_OPTION_CLI
      return FIRMWARE_ERROR_CURRENT_NOT_FOUND;
    }

    if(*idx_to_update < 0) {
#ifdef VTSS_SW_OPTION_CLI
      if(pReq) {
        cli_io_printf(pReq->io, "Image \"%s\" to update was not found in flash. Unable to update\n", name);
      }
#endif // VTSS_SW_OPTION_CLI
      return FIRMWARE_ERROR_UPDATE_NOT_FOUND;
    }

    if (flashlen > entry_to_update->size) {
#ifdef VTSS_SW_OPTION_CLI
      if(pReq) {
        cli_io_printf(pReq->io, "Image too big\n");
      }
#endif // VTSS_SW_OPTION_CLI
      T_W("Image too big (%ld), cannot extend flash entry len = %ld!", flashlen, entry_to_update->size);
      return FIRMWARE_ERROR_SIZE;
    }
    return FIRMWARE_ERROR_IN_PROGRESS;
}

/*
 * Actually update the flash now
 */
void firmware_program_doit(firmware_flash_args_t *pReq, const char *name) /* Name of FIS entry to update */
{
    int                    err, idx_to_update, idx_of_currently_running;
    char                   name_of_bk_image[16];
    struct fis_table_entry entry_to_update, entry_of_currently_running;
    unsigned char          *image;
    unsigned long          flashlen, crc;

    image = (char *) (pReq->buffer + 8);
    flashlen = ((unsigned long *)pReq->buffer)[1]; /* Size in bytes of real image excluding header and trailer */

    pReq->rc = firmware_flash_system_check_doit(pReq, name, &entry_to_update, &entry_of_currently_running, &idx_to_update, &idx_of_currently_running, name_of_bk_image, flashlen, pReq->image_version);
    if(pReq->rc != FIRMWARE_ERROR_IN_PROGRESS) {
        goto exit;
    }

    crc = cyg_crc32(image, flashlen); /* RedBoot uses different CRC algorithm */
    if (debug_check_same && flashlen == entry_of_currently_running.data_length && crc == entry_of_currently_running.file_cksum) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(pReq->io, "Downloaded file is already present in flash\n");
#endif // VTSS_SW_OPTION_CLI
        T_D("FLASH already updated (CRC 0x%08lx).", crc);
        pReq->rc = FIRMWARE_ERROR_SAME;
        goto exit;
    }

    T_D("Image needs update. Len (%ld, %ld). CRC (0x%08lx, 0x08%lx)!", flashlen, entry_of_currently_running.data_length, crc, entry_of_currently_running.file_cksum);

    /* LED state: Firmware update */
    led_front_led_state(LED_FRONT_LED_FLASHING_BOARD);

    firmware_status_set("Erasing, please wait...");
#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(pReq->io, "Erasing image...\n");
#endif // VTSS_SW_OPTION_CLI
    if ((err = control_flash_erase(entry_to_update.flash_base, flashlen)) != FLASH_ERR_OK) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(pReq->io, "Flash erase error: %d %s\n", err, flash_errmsg(err));
#endif // VTSS_SW_OPTION_CLI
        pReq->rc = FIRMWARE_ERROR_FLASH_ERASE;
        goto exit;
    }

    if (debug_pause_position == 1) {
        T_I("Sleeping for %d seconds after erase", FIRMWARE_DEBUG_PAUSE_TIME_MS/1000);
        VTSS_OS_MSLEEP(FIRMWARE_DEBUG_PAUSE_TIME_MS);
    }

    T_D("Erased flash...");
    firmware_status_set("Programming, please wait...");
#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(pReq->io, "Programming image...\n");
#endif // VTSS_SW_OPTION_CLI
    if ((err = control_flash_program(entry_to_update.flash_base, image, flashlen)) != FLASH_ERR_OK) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(pReq->io, "Flash program error: %d %s\n", err, flash_errmsg(err));
#endif // VTSS_SW_OPTION_CLI
        pReq->rc = FIRMWARE_ERROR_FLASH_PROGRAM;
        goto exit;
    }

    if (debug_pause_position == 2) {
        T_I("Sleeping for %d seconds after program", FIRMWARE_DEBUG_PAUSE_TIME_MS/1000);
        VTSS_OS_MSLEEP(FIRMWARE_DEBUG_PAUSE_TIME_MS);
    }

    /* Update Redboot Flash Directory with new entry, and possibly swap the old and new names. */
    entry_to_update.data_length = flashlen;
    entry_to_update.file_cksum = crc;
    if(idx_to_update != idx_of_currently_running) {
      // Dual-flash-image support.
      // Rename idx_to_update @name and idx_of_currently_running to @name.bk
      strcpy(entry_to_update.name, name);
      strcpy(entry_of_currently_running.name, name_of_bk_image);
      CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_MODIFY_ENTRY, idx_of_currently_running, &entry_of_currently_running);
    }
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_MODIFY_ENTRY, idx_to_update, &entry_to_update);
    FLASH_BUS_LOCK(); /* Guard against simultaneous SPI bus access */
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_START_UPDATE, 0, NULL);
    if (debug_pause_position == 3) {
        T_I("Sleeping for %d seconds between FIS start and finish update", FIRMWARE_DEBUG_PAUSE_TIME_MS/1000);
        VTSS_OS_MSLEEP(FIRMWARE_DEBUG_PAUSE_TIME_MS);
    }
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_FINISH_UPDATE, 0, NULL);
    FLASH_BUS_UNLOCK(); /* Guard against simultaneous SPI bus access */

    /* Flash WAS updated */
    pReq->rc = VTSS_OK;

 exit:
    /* LED state back to normal */
    led_front_led_state(LED_FRONT_LED_NORMAL);

    T_D("Flash done, rc = %d", pReq->rc);
    firmware_status_set("Restarting, please wait...");
#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(pReq->io, "Flash update %s.\n",
                  pReq->rc == VTSS_OK ? "succeeded" : "failed");
    if(pReq->rc != VTSS_OK) {
        cli_io_printf(pReq->io, "Error: %s\n", error_txt(pReq->rc));
    }
#endif // VTSS_SW_OPTION_CLI
    /* Cleanup */
    if(!pReq->sync) {
        if(pReq->rc == VTSS_OK) {
#ifdef VTSS_SW_OPTION_CLI
            cli_io_printf(pReq->io, "Rebooting system...");
#endif // VTSS_SW_OPTION_CLI
            /* We need to release the flash lock before calling control_system_reset_sync()
               because that function attempts to take the reset semaphore, which may
               already have been taken by e.g. the conf module, so that is can write
               (and make sure the write finalizes) configuration changes without the
               risk of a re-boot right after. By releasing the flash lock, the conf
               module may proceed.
            */
            control_system_flash_unlock();
            control_system_reset_sync(pReq->restart);
            /* NOTREACHED */
        } else {
            /* Failure, free the sw image buffer and req */
            VTSS_FREE((void*)pReq->buffer);
            VTSS_FREE(pReq);
        }
    } else {
        /* Somebody's waiting for this (rc *is* set) */
        cyg_semaphore_post(&pReq->sem);
    }

    /* Unlock flash */
    control_system_flash_unlock();
}

/*
 * Exposed API functions
 */


/*
 * Check firmware chipset compatibility
 */
BOOL firmware_compatible(cyg_uint8 product, cyg_uint16 chiptype)
{
    cyg_uint16 hw_chiptype = misc_chiptype();
    cyg_uint8 sw_product = misc_softwaretype();
    // If the Chip ID doesn't match, the new firmware image will not execute properly on this chip.
    if(chiptype != hw_chiptype) {
        T_E("Chip-ID mismatch (Image ID was %04x, have %04x)", chiptype, hw_chiptype);
        return FALSE;
    }
    // If the Product type is set, and does not match the current product, deny update
    if(product && product != sw_product) {
        T_E("Product mismatch (Product ID was %02x, have %02x)", product, sw_product);
        return FALSE;
    }
    return TRUE;
}




/*
 * Determine validity of image buffer
 */
vtss_rc firmware_check(const unsigned char *buffer,
                       size_t length,
                       unsigned long type,
                       unsigned long flags,
                       unsigned long *image_version)
{
    const unsigned char *image = buffer + 8;
    size_t crc_len = length - 8, image_len = *(unsigned long*)(buffer+4);
    unsigned long crc = cyg_posix_crc32((void *)image, crc_len);
    if (((unsigned long *)buffer)[0] != crc) {
        T_W("CRC mismatch (0x%08lx/0x%08lx) in downloaded image!",
            ((unsigned long *)buffer)[0], crc);
        return FIRMWARE_ERROR_INVALID;
    }
    /* Is there a gap - and is it "big" enough? */
    if(crc_len > image_len &&
       (crc_len-image_len) >= sizeof(unsigned long) + sizeof(firmware_trailer_t)) {
        unsigned long tlen, magic;
        const unsigned char *trailer = &image[image_len]; /* Possibly unaligned! */
        memcpy(&tlen, trailer + 0, sizeof(tlen)); /* Trailer length */
        memcpy(&magic, trailer + 4, sizeof(magic)); /* Real trailer starts here */
        /* Check magic */
        if(magic != FIRMWARE_TRAILER_MAGIC) {
            T_W("Bad trailer magic: %lx", magic);
            return FIRMWARE_ERROR_INVALID;
        }
        /* Consistency - do we fit the gap? length sane?*/
        if(tlen < (crc_len-image_len-sizeof(unsigned long))) {
            T_W("Trailer length inconsistency,tlen %ld, gap %zd (%zd - %zd)",
                tlen, crc_len-image_len, crc_len, image_len);
            return FIRMWARE_ERROR_INVALID;
        }
        firmware_trailer_t *t;
        if((t=VTSS_MALLOC(tlen))) {
            memcpy(t, trailer + 4, tlen); /* Copy from start of real trailer */
            /* Signed hash */
            unsigned long org_crc = t->crc, calc_crc, version;
            t->crc = FIRMWARE_TRAILER_AUTHKEY;
            calc_crc = cyg_posix_crc32((void*)t, tlen);
            vtss_rc result = VTSS_OK;
            if(org_crc != calc_crc) {
                T_W("Trailer: crc mismatch %lx/%lx len %lu, magic %lx",
                    org_crc, calc_crc, tlen, t->magic);
                result = FIRMWARE_ERROR_INVALID;
            }
            /* Signature OK, check version */
            if(result == VTSS_OK && t->version != FIRMWARE_TRAILER_VER_1) {
                T_W("Trailer: invalid version (%ld)", t->version);
                result = FIRMWARE_ERROR_INVALID;
            }
            /* Version OK, check V1 type */
            if(result == VTSS_OK && t->v1.type != type) {
                T_W("Trailer: invalid type (was %ld, want %ld)", t->v1.type, type);
                result = FIRMWARE_ERROR_INVALID;
            }
            /* Version, type OK, check flags */
            if(result == VTSS_OK && (t->v1.flags & flags) != flags) {
                T_W("Trailer: invalid flags (was 0x%lx, want 0x%lx)", t->v1.flags, flags);
                result = FIRMWARE_ERROR_INVALID;
            }
            /* Flags OK, check CPU/Chipset */
            if(result == VTSS_OK) {
                if(t->v1.cpu_product_chiptype != 0) {
                    cyg_uint8 cputype = (cyg_uint8) t->v1.cpu_product_chiptype;
                    cyg_uint8 product = (cyg_uint8) (t->v1.cpu_product_chiptype >> 8);
                    cyg_uint16 chiptype = (cyg_uint16) (t->v1.cpu_product_chiptype >> 16);
                    T_D("Trailer: CPU/PRODUCT/CHIP %08lx", t->v1.cpu_product_chiptype);
                    if(cputype != MY_CPU_ARC) {
                        T_W("Trailer: CPU type mismatch (was %d, want %d)", cputype, MY_CPU_ARC);
                        result = FIRMWARE_ERROR_INVALID;
                    } else
                        if(!firmware_compatible(product, chiptype))
                            result = FIRMWARE_ERROR_INVALID;
                } else if(MY_CPU_ARC != _CPU_TYPE_ARM) {
                    T_W("Trailer: CPU type must be set (and was zero)");
                    result = FIRMWARE_ERROR_INVALID;
                }
            }
            version = t->v1.image_version;
            VTSS_FREE(t);
            if(result != VTSS_OK) /* OK this far? */
                return result;
            *image_version = version;
            /* Now, check if already in flash */
            return
                firmware_flash_system_check(image, crc_len - 4 - tlen, "managed", version);
        }
    } else {
        T_D("Image_len %zd, crc_len %zd", image_len, crc_len);
        T_W("Type trailer missing from image, use newer software version!");
    }
    return FIRMWARE_ERROR_INVALID;
}

/*
 * Get the maximum size of the image.
 */
static unsigned long get_max_code_size(const char *image_name)
{
  struct fis_table_entry entry;
  if(flash_mgmt_fis_lookup(image_name, &entry) >= 0) {
    return entry.size;
  } else {
    return 0;
  }
}

/*
 * TFTP firmware image
 */
char *firmware_tftp(cli_iolayer_t *io,
                    const unsigned char *server,
                    const unsigned char *file,
                    const unsigned char *section,
                    size_t *length)
{
    struct sockaddr_in host;
    int res, err;
    char *buffer;
    unsigned long bufferlen = get_max_code_size(section);
    if (bufferlen == 0) {
#ifdef VTSS_SW_OPTION_CLI
      cli_io_printf(io, "Couldn't obtain size of flash image.\n");
#endif // VTSS_SW_OPTION_CLI
      return NULL;
    }

    memset((char *)&host, 0, sizeof(host));
    host.sin_len = sizeof(host);
    host.sin_family = AF_INET;
    if (!inet_aton(server, &host.sin_addr)) {
#ifdef VTSS_SW_OPTION_CLI
       cli_io_printf(io, "Invalid server address: '%s'\n", server);
#endif // VTSS_SW_OPTION_CLI
       return NULL;
    }
    host.sin_port = 0;

    if (!(buffer = VTSS_MALLOC(bufferlen))) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Unable to allocate download buffer, aborted download\n");
#endif // VTSS_SW_OPTION_CLI
        return NULL;
    }

    T_I("Get %s from %s", file, inet_ntoa(host.sin_addr));
    res = tftp_get(file, &host, buffer, bufferlen, TFTP_OCTET, &err);
    T_D("FTP: res = %d, err = %d", res, err);
    if (res <= 0) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Download of %s from %s failed: ", file, inet_ntoa(host.sin_addr));
#endif // VTSS_SW_OPTION_CLI
        char err_msg_str[100];
        firmware_tftp_err2str(err,&err_msg_str[0]);
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "%s\n",err_msg_str);
#endif //  VTSS_SW_OPTION_CLI
        VTSS_FREE(buffer);
        return NULL;
    }

#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(io, "Downloaded \"%s\", %d bytes\n", file, res);
#endif // VTSS_SW_OPTION_CLI
    *length = res;
    return buffer;
}

#ifdef VTSS_SW_OPTION_IPV6
char *firmware_tftp_ipv6(cli_iolayer_t *io,
                    const unsigned char *server,
                    const unsigned char *file,
                    size_t *length)
{
    struct sockaddr_in6 host;
    int res, err;
    char *buffer;
    unsigned long bufferlen = get_max_code_size("managed");
    if(bufferlen == 0) {
#ifdef VTSS_SW_OPTION_CLI
      cli_io_printf(io, "Couldn't obtain size of flash image.\n");
#endif // VTSS_SW_OPTION_CLI
      return NULL;
    }

    if(!(buffer = VTSS_MALLOC(bufferlen))) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Unable to allocate download buffer, aborted download\n");
#endif // VTSS_SW_OPTION_CLI
        return NULL;
    }

    memset((char *)&host, 0, sizeof(host));
    host.sin6_len = sizeof(host);
    host.sin6_family = AF_INET6;
    inet_pton(AF_INET6, server, (void *)&host.sin6_addr);
    host.sin6_port = 0;
//    T_I("Get %s from %s", file, inet_ntoa(host.sin_addr));
    res = tftp_get_ipv6(file, &host, buffer, bufferlen, TFTP_OCTET, &err);
    T_D("FTP: res = %d, err = %d", res, err);
    if (res <= 0) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Download of %s from %s failed: ", file, inet_ntop(AF_INET6, (void *)&host.sin6_addr, buffer, bufferlen));
#endif // VTSS_SW_OPTION_CLI
        char err_msg_str[100];
        firmware_tftp_err2str(err,&err_msg_str[0]);
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "%s\n",err_msg_str);
#endif //  VTSS_SW_OPTION_CLI
        VTSS_FREE(buffer);
        return NULL;
    }

#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(io, "Downloaded \"%s\", %d bytes\n", file, res);
#endif // VTSS_SW_OPTION_CLI
    *length = res;
    return buffer;
}
#endif /* VTSS_SW_OPTION_IPV6 */

/*
 * Turn image buffer into mbox-able request (if valid)
 */
vtss_rc
firmware_update_mkreq(cli_iolayer_t *io,
                      const unsigned char *buffer,
                      size_t length,
                      firmware_flash_args_t **ppReq,
                      cyg_bool sync,
                      unsigned long type,
                      unsigned long flags,
                      vtss_restart_t restart)
{
    unsigned long image_version;
    vtss_rc result;

    result = firmware_check(buffer, length, type, flags, &image_version);
    if(result != VTSS_OK)
        return result;

    firmware_flash_args_t *pReq = VTSS_MALLOC(sizeof(firmware_flash_args_t));
    if(!pReq) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Memory allocation error, no flash update\n");
#endif // VTSS_SW_OPTION_CLI
        return FIRMWARE_ERROR_MALLOC;
    }

    /* Fill in request */
    pReq->sync          = sync;
    pReq->io            = io;
    pReq->buffer        = buffer;
    pReq->length        = length;
    pReq->image_version = image_version;
    pReq->rc            = FIRMWARE_ERROR_IN_PROGRESS;
    pReq->restart       = restart;
    if(pReq->sync) {            /* Synchronized request? */
        cyg_semaphore_init(&pReq->sem, 0);
    }

    /* Return buffer to caller */
    *ppReq = pReq;

    /* All OK */
    return VTSS_OK;
}

int firmware_debug_pause_get(void)
{
  return debug_pause_position;
}

void firmware_debug_pause_set(int value)
{
  debug_pause_position = value;
}

BOOL firmware_debug_check_same_get(void)
{
  return debug_check_same;
}

void firmware_debug_check_same_set(BOOL value)
{
  debug_check_same = value;
}

vtss_rc firmware_flash_system_check(const unsigned char *buffer,
                                    unsigned long image_len, 
                                    const char *name, 
                                    unsigned long image_version)
{
  struct fis_table_entry entry_to_update, entry_of_currently_running;
  int                    idx1, idx2;
  char                   name_of_bk_image[16];
  vtss_rc                result;
  result = firmware_flash_system_check_doit(NULL, name, &entry_to_update, &entry_of_currently_running, 
                                            &idx1, &idx2, name_of_bk_image, image_len, image_version);
  if(result == FIRMWARE_ERROR_IN_PROGRESS) {
      cyg_uint32 crc = cyg_crc32(buffer, image_len); /* RedBoot uses different CRC algorithm */
      if (debug_check_same && 
          image_len == entry_of_currently_running.data_length && 
          crc == entry_of_currently_running.file_cksum) {
          T_D("FLASH already updated (CRC 0x%08x).", crc);
          result = FIRMWARE_ERROR_SAME;
      } else
          result = VTSS_OK;
  }
  return result;
}

vtss_rc firmware_bootloader_update(cli_iolayer_t *io, const simage_t *simg, const char *name)
{
    vtss_rc result = VTSS_OK;
    struct fis_table_entry fis;
    int ent;

    if((ent = flash_mgmt_fis_lookup(name, &fis)) > 0 &&
       simg->img_len <= fis.size) {
        u32 arch = 0, chip = 0, imgtype = 0;
        if(!simage_get_tlv_dword(simg, SIMAGE_TLV_ARCH, &arch) || arch != MY_CPU_ARC) {
            T_E("Invalid or missing architecture: %d\n", arch);
            result = FIRMWARE_ERROR_INVALID;
        } else if(!simage_get_tlv_dword(simg, SIMAGE_TLV_CHIP, &chip) ||
                  misc_chip2family(chip) != misc_chipfamily()) {
            T_D("CHIPFAMILY mismatch (was %d, want %d, CHIP was %04x, have %04x)",
                misc_chip2family(chip), misc_chipfamily(), chip, misc_chiptype());
            cli_io_printf(io, "Firmware image architecture does not match system: CHIP was %04x, have %04x\n",
                          misc_chip2family(chip), misc_chipfamily());
            result = FIRMWARE_ERROR_INVALID;
        } else if(!simage_get_tlv_dword(simg, SIMAGE_TLV_IMGTYPE, &imgtype) ||
                  imgtype != SIMAGE_IMGTYPE_BOOT_LOADER) {
            T_E("Image type mismatch (was %d, want %d)", imgtype, SIMAGE_IMGTYPE_BOOT_LOADER);
            result = FIRMWARE_ERROR_INVALID;
        } else {
            u8 *old = VTSS_MALLOC(simg->img_len);
            if(old) {
                if(cyg_flash_read(fis.flash_base, old, simg->img_len, NULL) == CYG_FLASH_ERR_OK && 
                   memcmp(old, simg->img_ptr, simg->img_len) != 0) {
                    int err;
                    cli_io_printf(io, "Updating, *DO NOT RESET BOARD*\n");
                    /* LED state: Firmware update */
                    led_front_led_state(LED_FRONT_LED_FLASHING_BOARD);
                    cli_io_printf(io, "Erasing...\n");
                    if ((err = control_flash_erase(fis.flash_base, simg->img_len)) == FLASH_ERR_OK) {
                        cli_io_printf(io, "Erasing...OK\n");
                        cli_io_printf(io, "Programming...\n");
                        if ((err = control_flash_program(fis.flash_base, 
                                                         simg->img_ptr, simg->img_len)) != FLASH_ERR_OK) {
                            T_E("Flash program error: %d %s", err, flash_errmsg(err));
                            cli_io_printf(io, "Flash program error: %d %s\n", err, flash_errmsg(err));
                            result = FIRMWARE_ERROR_FLASH_PROGRAM;
                        } else {
                            cli_io_printf(io, "Programming...OK\n");
                        }
                    } else {
                        T_E("Flash erase error: %d %s", err, flash_errmsg(err));
                        result = FIRMWARE_ERROR_FLASH_ERASE;
                    }
                    /* LED state back to normal */
                    led_front_led_state(LED_FRONT_LED_NORMAL);
                } else {
                    T_I("Flash already contain the bootloader image");
                    result = FIRMWARE_ERROR_SAME;
                }
                VTSS_FREE(old);
            } else {
                T_E("Malloc error, %zu bytes", simg->img_len);
                result = FIRMWARE_ERROR_MALLOC;
            }
        }
    } else {
        if(ent > 0) {
            T_E("FIS entry '%s' too small: has %lu bytes, image is %zd bytes", 
                name, fis.size, simg->img_len);
            result = FIRMWARE_ERROR_SIZE;
        } else {
            T_E("FIS entry not found: '%s'", name);
            result = FIRMWARE_ERROR_UPDATE_NOT_FOUND;
        }
    }
    return result;
}

/*
 * firmware_error_txt()
 */
char *firmware_error_txt(vtss_rc rc)
{
  switch(rc) {
    case FIRMWARE_ERROR_IN_PROGRESS:             return "Firmware update in progress";
    case FIRMWARE_ERROR_IP:                      return "IP Setup error";
    case FIRMWARE_ERROR_TFTP:                    return "TFTP error";
    case FIRMWARE_ERROR_BUSY:                    return "Already updating";
    case FIRMWARE_ERROR_MALLOC:                  return "Memory allocation error";
    case FIRMWARE_ERROR_INVALID:                 return "Invalid image";
    case FIRMWARE_ERROR_FLASH_PROGRAM:           return "Flash write error";
    case FIRMWARE_ERROR_SAME:                    return "Flash is already updated with this image";
    case FIRMWARE_ERROR_CURRENT_UNKNOWN:         return "The currently loaded image is unknown";
    case FIRMWARE_ERROR_CURRENT_NOT_FOUND:       return "The image that we're currently running was not found in flash";
    case FIRMWARE_ERROR_UPDATE_NOT_FOUND:        return "The required flash entry was not found";
    case FIRMWARE_ERROR_CRC:                     return "The entry has invalid CRC";
    case FIRMWARE_ERROR_SIZE:                    return "The size of the firmware image is too big to fit into the flash";
    case FIRMWARE_ERROR_FLASH_ERASE:             return "An error occurred while attempting to erase the flash";
    case FIRMWARE_ERROR_INCORRECT_IMAGE_VERSION: return "The image version is incorrect"; /// This typically happens if running on RedBoot v. 1.07 and attempting to upload an application with .dat version number set to a value smaller than 2.
    default:                                     return "Unknown firmware error code";
  }
}

// Function that converts a TFTP error code into a string.
void firmware_tftp_err2str(int err_num, char *err_str)
{
    switch (err_num) {
    case 0 :
        strcpy(err_str,"Bad file permissions.");
        break;

    case TFTP_ENOTFOUND:
        strcpy(err_str,"File not found.");
        break;
    case TFTP_EACCESS:
        strcpy(err_str,"Access violation.");
        break;

    case TFTP_ENOSPACE:
        strcpy(err_str,"Disk full or allocation exceeded.");
        break;

    case TFTP_EBADOP:
        strcpy(err_str,"Illegal TFTP operation.");
        break;

    case TFTP_EBADID:
        strcpy(err_str,"Unknown transfer ID.");
        break;

    case TFTP_EEXISTS:
        strcpy(err_str,"File already exists.");
        break;

    case TFTP_ENOUSER:
        strcpy(err_str,"No such user.");
        break;

    /* Non-net errors */
    case TFTP_TIMEOUT:
        strcpy(err_str,"Operation timed out.");
        break;

    case TFTP_NETERR:
        strcpy(err_str,"Network error");
        break;

    case TFTP_INVALID:
        strcpy(err_str,"Invalid parameter");
        break;

    case TFTP_PROTOCOL:
        strcpy(err_str,"Protocol violation");
        break;

    case TFTP_TOOLARGE:
        strcpy(err_str,"File too large");
        break;

    default:
        sprintf(err_str,"%s%d","Unknown error. Error code = ",err_num);
    }
    T_D("TEST %s , %d", &err_str[0], err_num);
}





























































/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
