// ####ECOSHOSTGPLCOPYRIGHTBEGIN####                                        
// -------------------------------------------                              
// This file is part of the eCos host tools.                                
// Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.            
//
// This program is free software; you can redistribute it and/or modify     
// it under the terms of the GNU General Public License as published by     
// the Free Software Foundation; either version 2 or (at your option) any   
// later version.                                                           
//
// This program is distributed in the hope that it will be useful, but      
// WITHOUT ANY WARRANTY; without even the implied warranty of               
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        
// General Public License for more details.                                 
//
// You should have received a copy of the GNU General Public License        
// along with this program; if not, write to the                            
// Free Software Foundation, Inc., 51 Franklin Street,                      
// Fifth Floor, Boston, MA  02110-1301, USA.                                
// -------------------------------------------                              
// ####ECOSHOSTGPLCOPYRIGHTEND####                                          
//=================================================================
//
//        webstax_binmerge.c
//
//        Flash image creation
//
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Rene Nielsen <rbn@vitesse.com>
// Contributors:
// Date:        2009-04-30
// Purpose:     Flash image creation tool
// Description: Used to produce flash images for production line
//
//####DESCRIPTIONEND####
#include <stdio.h>
#include <stdlib.h> /* For exit() */
#include <string.h> /* For strcmp() a.o. */
#include <ctype.h>  /* For isalpha(), isprint(), etc. */

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef unsigned long  bool;
typedef u32            CYG_ADDRESS;

#define false 0
#define true  1

#define BINMERGE_VERSION        "2.1"

#ifndef ALLOWED_MAXIMUM_SECTOR_SIZE 
#define ALLOWED_MAXIMUM_SECTOR_SIZE 128UL
#endif

#if (ALLOWED_MAXIMUM_SECTOR_SIZE != 128UL) && (ALLOWED_MAXIMUM_SECTOR_SIZE != 256UL)
#error invalid ALLOWED_MAXIMUM_SECTOR_SIZE value!
#endif

// Sector size should be set to the maximum sector size we can image a flash contain. It does not
// need to be the actual flash's sector size, as long as the one chosen is a multiple of the
// actual sector size
#define SECTOR_SIZE_BYTES       (ALLOWED_MAXIMUM_SECTOR_SIZE * 1024UL)

// Default sizes.
#define DEFAULT_SIZE_FLASH      16777216UL  /* 16  MBytes */
#define DEFAULT_SIZE_SECTOR     SECTOR_SIZE_BYTES
#define DEFAULT_SIZE_REDBOOT      262144UL  /* 256 KBytes */
#define DEFAULT_SIZE_CONF       SECTOR_SIZE_BYTES /* 128 or 256 KBytes, according to ALLOWED_MAXIMUM_SECTOR_SIZE */
#define DEFAULT_SIZE_STACKCONF   1048576UL  /* 1.0 MBytes */
#define DEFAULT_SIZE_SYSLOG       262144UL  /* 256 KBytes */
#define DEFAULT_SIZE_APPL        6291456UL  /* 6.0 MBytes */

// FIS names
#define FIS_NAME_REDBOOT         "RedBoot"
#define FIS_NAME_CONF            "conf"
#define FIS_NAME_STACKCONF       "stackconf"
#define FIS_NAME_SYSLOG          "syslog"
#define FIS_NAME_APPL1           "managed"
#define FIS_NAME_APPL2           "managed.bk"
#define FIS_NAME_REDBOOT_CONFIG  "RedBoot config"
#define FIS_NAME_FIS_DIRECTORY   "FIS directory"
#define FIS_NAME_REDUNDANT_FIS   "Redundant FIS"

// Other constants
#define PAD_CHAR                 0xff
#define VCOREII_FLASH_START               0x80000000
#define VCOREII_APPL_MEMORY_BASE          0x00100000
#define VCOREII_APPL_MEMORY_ENTRY_POINT  (VCOREII_APPL_MEMORY_BASE + 0xbc)

#define VCOREIII_FLASH_START              0x40000000    /* DDR RAM offset of first byte of flash */
#define VCOREIII_APPL_MEMORY_BASE         0x80040000    /* This is where the application is loaded to from flash to RAM and is the same for both managed and managed.bk */
#define VCOREIII_APPL_MEMORY_ENTRY_POINT  (VCOREIII_APPL_MEMORY_BASE + 0xbc) /* This is where the application execution entry point is located */

// This is taken from $(Top)/sw_mgd_switch/conf.c
// Board configuration signatures
static const char *FLASH_CONF_BOARD_SIG = "#@(#)VtssConfig\n";

// Holds the user-applied (with -a option) <attribute, value> pairs
// to be programmed in the FIS 'conf' section.
static char  conf_section[DEFAULT_SIZE_CONF];
static char *conf_section_ptr;

static char *prog_name;
static bool verbose = false;
static bool vcoreii = false;

/******************************************************************************/
// The CRC32 code is borrowed from
//   $(Top)/eCos/packages/services/crc/current/src/crc32.c
/******************************************************************************/

// ======================================================================
//  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
//  code or tables extracted from it, as desired without restriction.
//
//  First, the polynomial itself and its table of feedback terms.  The
//  polynomial is
//  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
//
// ======================================================================
static const u32 crc32_tab[] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
   };

/******************************************************************************/
// This is the standard Gary S. Brown's 32 bit CRC algorithm, but
// accumulate the CRC into the result of a previous CRC.
/******************************************************************************/
static u32 cyg_crc32_accumulate(u32 crc32val, u8 *s, int len)
{
  int i;

  for (i = 0;  i < len;  i++) {
    crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);
  }
  return crc32val;
}

/******************************************************************************/
// This is the standard Gary S. Brown's 32 bit CRC algorithm
/******************************************************************************/
static u32 cyg_crc32(u8 *s, int len)
{
  return (cyg_crc32_accumulate(0, s, len));
}

/******************************************************************************/
/******************************************************************************/
static u32 fsize(FILE *file)
{
  u32 curpos = ftell(file);
  u32 result;
  fseek(file, 0, SEEK_END);
  result = ftell(file);
  fseek(file, curpos, SEEK_SET);
  return result;
}

/******************************************************************************/
// This is provided in order to create only parts of the flash for debugging
// purposes.
/******************************************************************************/
static size_t my_fwrite(const void *ptr, size_t size, size_t nitems, FILE *stream)
{
  static long curpos = 0;
  #define FWRITE_START_FROM -1 /* Change this... */
  #define FWRITE_END_AT     -1 /* ...or this to control where to start and end the actual writing (-1 means, don't use it) */

  long bytes_left = size * nitems;

  if(FWRITE_START_FROM == -1 && FWRITE_END_AT == -1) {
    // Not debugging.
    return fwrite(ptr, size, nitems, stream);
  }

  // Debugging. For simplicity, write one byte at a time.
  while(bytes_left != 0) {
    if((FWRITE_START_FROM == -1 || curpos >= FWRITE_START_FROM) && (FWRITE_END_AT == -1 || curpos < FWRITE_END_AT)) {
      if(fwrite(ptr++, 1, 1, stream) != 1)
        return 0;
    }
    curpos++;
    bytes_left--;
  }
  return nitems; // Pretend that all have been written.
}

/******************************************************************************/
/******************************************************************************/
static bool pad(FILE *output, u32 pad_to)
{
  u32 curpos = ftell(output);
  u8 ch = PAD_CHAR;

  if(curpos > pad_to) {
    printf("Can't pad to 0x%08lx, since the file is already %lu bytes long!\n", pad_to, curpos);
    return false;
  }

  if(verbose)
    printf("0x%08lx - 0x%08lx: Padding\n", ftell(output), pad_to);

  while(curpos++ < pad_to) {
    if(my_fwrite(&ch, 1, 1, output) != 1) {
      printf("Error: Couldn't pad output file\n");
      return false;
    }
  }
  return true;
}

/******************************************************************************/
/******************************************************************************/
static bool copy_file(FILE *output, FILE *input, u32 pad_to, u32 *crc, char *what)
{
  u8 buf[256];
  size_t cnt;

  fseek(input, 0, SEEK_SET);
  *crc = 0;

  if(verbose) {
    printf("0x%08lx - 0x%08lx: Copying %s\n", ftell(output), ftell(output) + fsize(input), what);
  }

  while((cnt = fread(buf, 1, sizeof(buf), input)) > 0) {
    // Compute CRC while we're at it
    *crc = cyg_crc32_accumulate(*crc, buf, cnt);
    if(my_fwrite(buf, 1, cnt, output) != cnt) {
      printf("Error: Couldn't write to output file\n");
      return false;
    }
  }

  // Now pad to the required length
  return pad(output, pad_to);
}

/******************************************************************************/
// This is taken from
// $(Top)/eCos/packages/redboot/current/include/flash_config.h and
// $(Top)/eCos/packages/redboot/current/include/fconfig.h
// Internal structure used to hold RedBoot config data.
/******************************************************************************/
#define MAX_SCRIPT_LENGTH  512 /* CYGNUM_REDBOOT_FLASH_SCRIPT_SIZE */
#define MAX_STRING_LENGTH  128 /* CYGNUM_REDBOOT_FLASH_STRING_SIZE */
#define MAX_CONFIG_DATA   4096 /* CYGNUM_REDBOOT_FLASH_CONFIG_SIZE */

// Holds the (user-supplied (with -t option)) boot script
// to be programmed in the 'RedBoot config' section.
static char  boot_script[MAX_SCRIPT_LENGTH];
static char *boot_script_ptr;

struct _config {
  unsigned long len;
  unsigned long key1;
  unsigned char config_data[MAX_CONFIG_DATA-(4*4)];
  unsigned long key2;
  unsigned long cksum;
};

#define CONFIG_KEY1 0x0BADFACE
#define CONFIG_KEY2 0xDEADDEAD

#define CONFIG_EMPTY   0
#define CONFIG_BOOL    1
#define CONFIG_INT     2
#define CONFIG_STRING  3
#define CONFIG_SCRIPT  4

#define CONFIG_OBJECT_TYPE(dp)          (dp)[0]
#define CONFIG_OBJECT_KEYLEN(dp)        (dp)[1]
#define CONFIG_OBJECT_ENABLE_SENSE(dp)  (dp)[2]
#define CONFIG_OBJECT_ENABLE_KEYLEN(dp) (dp)[3]
#define CONFIG_OBJECT_KEY(dp)           ((dp)+4)
#define CONFIG_OBJECT_ENABLE_KEY(dp)    ((dp)+4+CONFIG_OBJECT_KEYLEN(dp))
#define CONFIG_OBJECT_VALUE(dp)         ((dp)+4+CONFIG_OBJECT_KEYLEN(dp)+CONFIG_OBJECT_ENABLE_KEYLEN(dp))

struct config_option {
    char *key;
    char *title;
    char *enable;
    bool  enable_sense;
    int   type;
    unsigned long dflt;
};

#define ALWAYS_ENABLED (char *)0

static struct config_option redboot_config_boot_script = {
  .key          = "boot_script",
  .title        = "Run script at boot",
  .enable       = ALWAYS_ENABLED, /* (char *)0 */
  .enable_sense = true,
  .type         = CONFIG_BOOL,
  .dflt         = true /* false */
};

static struct config_option redboot_config_boot_script_data = {
  .key          = "boot_script_data",
  .title        = "Boot script",
  .enable       = "boot_script",
  .enable_sense = true,
  .type         = CONFIG_SCRIPT,
  .dflt         = 0 /* Filled in later */
};

static struct config_option redboot_config_boot_script_timeout = {
  .key          = "boot_script_timeout",
  .title        = "Boot script timeout (1000 ms resolution)",
  .enable       = "boot_script",
  .enable_sense = true,
  .type         = CONFIG_INT,
  .dflt         = 3 /* seconds */
};

/******************************************************************************/
// This function ensures that data pointed to by p is written as little endian
// to the same location independent of current processor endianness.
/******************************************************************************/
static void le32(u8 *p, u32 val)
{
  int i;
  u8 ch;

  for(i = 0; i < 4; i++) {
    ch = (u8)(val >> (8 * i));
    *p++ = ch;
  }
}

/******************************************************************************/
// This function ensures that data pointed to by p is written as little endian
// to the same location independent of current processor endianness.
/******************************************************************************/
static void le16(u8 *p, u16 val)
{
  int i;
  u8 ch;

  for(i = 0; i < 2; i++) {
    ch = (u8)(val >> (8 * i));
    *p++ = ch;
  }
}

/******************************************************************************/
// Most of this function is taken from config_length() in
// $(Top)/eCos/packages/redboot/current/src/fconfig.c
/******************************************************************************/
static int config_length(int type)
{
  switch(type) {
    case CONFIG_BOOL:
      return sizeof(bool);
    case CONFIG_INT:
      return sizeof(unsigned long);
    case CONFIG_STRING:
      return MAX_STRING_LENGTH;
    case CONFIG_SCRIPT:
      return MAX_SCRIPT_LENGTH;
    default:
      return 0;
  }
}

/******************************************************************************/
// Most of this function is taken from conf_endian_fixup() in
// $(Top)/eCos/packages/redboot/current/src/fconfig.c
// Change endianness of config data
/******************************************************************************/
static void conf_endian_fixup(struct _config *p)
{
  unsigned char *dp = p->config_data;
  void *val_ptr;
  int len;
  u16 uint16;
  u32 uint32;

  le32((u8 *)&p->len, p->len);
  le32((u8 *)&p->key1, p->key1);
  le32((u8 *)&p->key2, p->key2);
  le32((u8 *)&p->cksum, p->cksum);

  while(dp < &p->config_data[sizeof(p->config_data)]) {
    len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) + config_length(CONFIG_OBJECT_TYPE(dp));
    val_ptr = (void *)CONFIG_OBJECT_VALUE(dp);

    switch (CONFIG_OBJECT_TYPE(dp)) {
      // Note: the data may be unaligned in the configuration data
      case CONFIG_BOOL:
        if (sizeof(bool) == 2) {
          memcpy(&uint16, val_ptr, 2);
          le16((u8 *)val_ptr, uint16);
        } else if (sizeof(bool) == 4) {
          memcpy(&uint32, val_ptr, 4);
          le32((u8 *)val_ptr, uint32);
        }
        break;
      case CONFIG_INT:
        if (sizeof(unsigned long) == 2) {
          memcpy(&uint16, val_ptr, 2);
          le16((u8 *)val_ptr, uint16);
        } else if (sizeof(unsigned long) == 4) {
          memcpy(&uint32, val_ptr, 4);
          le32((u8 *)val_ptr, uint32);
        }
        break;
    }
    dp += len;
  }
}

/******************************************************************************/
// Most of this function is taken from flash_config_insert_value() in
// $(Top)/eCos/packages/redboot/current/src/fconfig.c
// Copy data into the config area
/******************************************************************************/
static void flash_config_insert_value(unsigned char *dp, struct config_option *opt)
{
  switch(opt->type) {
    // Note: the data may be unaligned in the configuration data
    case CONFIG_BOOL:
      memcpy(dp, (void *)&opt->dflt, sizeof(bool));
      break;
    case CONFIG_INT:
      memcpy(dp, (void *)&opt->dflt, sizeof(unsigned long));
      break;
    case CONFIG_STRING:
      memcpy(dp, (void *)opt->dflt, config_length(CONFIG_STRING));
      break;
    case CONFIG_SCRIPT:
      // RBN: Added this line because we really need to add a script already at initialization.
      // This is in contradiction to RedBoot, in which the script is empty after an fconfig -i.
      memcpy(dp, (void *)opt->dflt, strlen((char *)opt->dflt));
      break;
  }
}

/******************************************************************************/
// Most of this function is taken from flash_add_config() in
// $(Top)/eCos/packages/redboot/current/src/fconfig.c
/******************************************************************************/
static bool redboot_cfg_option(struct _config *config, struct config_option *opt)
{
  unsigned char *dp;
  char *kp;
  int len, elen, size;

  // Add the data item
  dp = &config->config_data[0];
  size = 0;
  while(size < sizeof(config->config_data)) {
    if(CONFIG_OBJECT_TYPE(dp) == CONFIG_EMPTY) {
      kp = opt->key;
      len = strlen(kp) + 1;
      size += len + 2 + 2 + config_length(opt->type);
      if (opt->enable) {
        elen = strlen(opt->enable) + 1;
        size += elen;
      } else {
        elen = 0;
      }
      if (size > sizeof(config->config_data)) {
        break;
      }
      CONFIG_OBJECT_TYPE(dp) = opt->type;
      CONFIG_OBJECT_KEYLEN(dp) = len;
      CONFIG_OBJECT_ENABLE_SENSE(dp) = opt->enable_sense;
      CONFIG_OBJECT_ENABLE_KEYLEN(dp) = elen;
      dp = CONFIG_OBJECT_KEY(dp);
      while (*kp) *dp++ += *kp++;
      *dp++ = '\0';
      if (elen) {
        kp = opt->enable;
        while (*kp) *dp++ += *kp++;
        *dp++ = '\0';
      }
      flash_config_insert_value(dp, opt);
      return true;
    } else {
      len = 4 + CONFIG_OBJECT_KEYLEN(dp) + CONFIG_OBJECT_ENABLE_KEYLEN(dp) + config_length(CONFIG_OBJECT_TYPE(dp));
      dp += len;
      size += len;
    }
  }
  printf("Error: Redboot Option Add: No space to add '%s'\n", opt->key);
  return false;
}

/******************************************************************************/
/******************************************************************************/
static bool write_redboot_cfg(FILE *output, u32 pad_to)
{
  struct _config config;

  memset(&config, 0, sizeof(config));
  config.len   = MAX_CONFIG_DATA;
  config.key1  = CONFIG_KEY1;
  config.key2  = CONFIG_KEY2;

  if(verbose) {
    printf("0x%08lx - 0x%08lx: Writing '%s'\n", ftell(output), ftell(output) + sizeof(config), FIS_NAME_REDBOOT_CONFIG);
  }

  // Add boot_script option
  if(!redboot_cfg_option(&config, &redboot_config_boot_script))
    return false;

  // The script is already created and located in the global boot_script variable.
  redboot_config_boot_script_data.dflt = (u32)&boot_script[0];

  if(!redboot_cfg_option(&config, &redboot_config_boot_script_data))
    return false;

  if(!redboot_cfg_option(&config, &redboot_config_boot_script_timeout))
    return false;

  config.cksum = cyg_crc32((u8 *)&config, sizeof(config) - sizeof(config.cksum));

  conf_endian_fixup(&config);

  if(my_fwrite(&config, 1, sizeof(config), output) != sizeof(config)) {
    printf("Error: Couldn't write to output file\n");
    return false;
  }

  return pad(output, pad_to);
}

/******************************************************************************/
// These defines and structures are taken from
// $(Top)/eCos/packages/redboot/current/include/fis.h
/******************************************************************************/
#define CYG_REDBOOT_RFIS_VALID_MAGIC_LENGTH 10
#define CYG_REDBOOT_RFIS_VALID_MAGIC ".FisValid"    /* Exactly 10 bytes */
#define CYG_REDBOOT_RFIS_VALID       (0xa5)         /* This FIS table is valid. The only "good" value */
#define CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_SIZE 256 /* From redboot.cdl */
#define CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_COUNT  8 /* From redboot.cdl */

#define FIS_IMAGE_DESC_SIZE_UNPADDED (16 + 4 * sizeof(unsigned long) + 3 * sizeof(CYG_ADDRESS))

struct fis_valid_info
{
 char magic_name[CYG_REDBOOT_RFIS_VALID_MAGIC_LENGTH];
 unsigned char valid_flag[2];   // one of the flags defined above
 unsigned long version_count;   // with each successfull update the version count will increase by 1
};

struct fis_image_desc {
  union {
    char name[16];             // Null terminated name
    struct fis_valid_info valid_info;
  } u;
  CYG_ADDRESS   flash_base;    // Address within FLASH of image
  CYG_ADDRESS   mem_base;      // Address in memory where it executes
  unsigned long size;          // Length of image
  CYG_ADDRESS   entry_point;   // Execution entry point
  unsigned long data_length;   // Length of actual data
  unsigned char _pad[CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_SIZE - FIS_IMAGE_DESC_SIZE_UNPADDED];
  unsigned long desc_cksum;    // Checksum over image descriptor
  unsigned long file_cksum;    // Checksum over image data
};

static u32  fisdir_size;
static u8   workspace_end[ALLOWED_MAXIMUM_SECTOR_SIZE * 1024];
static void *flash_start;
static void *cfg_base;
static int  cfg_size; // Length of config data - rounded to Flash block size

/******************************************************************************/
// Most of this function is taken from fis_endian_fixup() in
// $(Top)/eCos/packages/redboot/current/src/flash.c
// fis_endian_fixup() is used to swap endianess if required.
/******************************************************************************/
static void fis_endian_fixup(struct fis_image_desc *p)
{
  int cnt = fisdir_size / sizeof(struct fis_image_desc);
  while (cnt-- > 0) {
    le32((u8 *)&p->flash_base, p->flash_base);
    le32((u8 *)&p->mem_base, p->mem_base);
    le32((u8 *)&p->size, p->size);
    le32((u8 *)&p->entry_point, p->entry_point);
    le32((u8 *)&p->data_length, p->data_length);
    le32((u8 *)&p->desc_cksum, p->desc_cksum);
    le32((u8 *)&p->file_cksum, p->file_cksum);
    p++;
  }
}

#define ASSERT_IMG_VALID(img) do {if(((u8 *)(img) + sizeof(struct fis_image_desc)) > workspace_end + sizeof(workspace_end)) {printf("Internal Error: Too many directory entries\n"); exit(-1);}} while(0);

/******************************************************************************/
// Most of this function is taken from fis_init() in
// $(Top)/eCos/packages/redboot/current/src/flash.c
/******************************************************************************/
static u32 fis_init(FILE *output,
                    u32 redboot_offset,        u32 size_redboot_section,        u32 size_redboot_file, u32 redboot_crc,
                    u32 conf_offset,           u32 size_conf_section,
                    u32 stackconf_offset,      u32 size_stackconf_section,
                    u32 syslog_offset,         u32 size_syslog_section,
                    u32 appl1_offset,          u32 size_appl1_section,          u32 size_appl1_file,   u32 appl1_crc,
                    u32 appl2_offset,          u32 size_appl2_section,          u32 size_appl2_file,   u32 appl2_crc,
                    u32 redboot_config_offset,
                    u32 fis_directory_offset,
                    u32 redundant_fis_offset)
{
  struct fis_image_desc *img;

  img = (struct fis_image_desc *)workspace_end;
  memset(img, 0xFF, fisdir_size);  // Start with erased data

  // Create the valid flag entry
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(struct fis_image_desc));
  strcpy(img->u.valid_info.magic_name, CYG_REDBOOT_RFIS_VALID_MAGIC);
  img->u.valid_info.valid_flag[0]=CYG_REDBOOT_RFIS_VALID;
  img->u.valid_info.valid_flag[1]=CYG_REDBOOT_RFIS_VALID;
  img->u.valid_info.version_count=0;
  img++;

  // Create RedBoot section
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_REDBOOT);
  img->flash_base  = (CYG_ADDRESS)flash_start + redboot_offset;
  img->mem_base    = 0;
  img->size        = size_redboot_section;
  img->data_length = size_redboot_file;
  img->file_cksum  = redboot_crc;
  img++;

  // Create conf section
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_CONF);
  img->flash_base  = (CYG_ADDRESS)flash_start + conf_offset;
  img->mem_base    = 0;
  img->size        = size_conf_section;
  img->data_length = size_conf_section;
  img++;

  // Create stackconf section
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_STACKCONF);
  img->flash_base  = (CYG_ADDRESS)flash_start + stackconf_offset;
  img->mem_base    = 0;
  img->size        = size_stackconf_section;
  img->data_length = size_stackconf_section;
  img++;

  // Optionally create syslog section
  if(size_syslog_section) {
    ASSERT_IMG_VALID(img);
    memset(img, 0, sizeof(*img));
    strcpy(img->u.name, FIS_NAME_SYSLOG);
    img->flash_base  = (CYG_ADDRESS)flash_start + syslog_offset;
    img->mem_base    = 0;
    img->size        = size_syslog_section;
    img->data_length = size_syslog_section;
    img++;
  }

  // Create the appl1 entry
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_APPL1);
  img->flash_base  = (CYG_ADDRESS)flash_start + appl1_offset;
  img->mem_base    = (vcoreii) ? (CYG_ADDRESS)VCOREII_APPL_MEMORY_BASE : (CYG_ADDRESS)VCOREIII_APPL_MEMORY_BASE;
  img->size        = size_appl1_section;
  img->entry_point = (vcoreii) ? (CYG_ADDRESS)VCOREII_APPL_MEMORY_ENTRY_POINT : (CYG_ADDRESS)VCOREIII_APPL_MEMORY_ENTRY_POINT;
  img->data_length = size_appl1_file;
  img->file_cksum  = appl1_crc;
  img++;

  // Optionally create the appl2 entry
  if(size_appl2_section) {
    ASSERT_IMG_VALID(img);
    memset(img, 0, sizeof(*img));
    strcpy(img->u.name, FIS_NAME_APPL2);
    img->flash_base  = (CYG_ADDRESS)flash_start + appl2_offset;
    img->mem_base    = (vcoreii) ? (CYG_ADDRESS)VCOREII_APPL_MEMORY_BASE : (CYG_ADDRESS)VCOREIII_APPL_MEMORY_BASE;
    img->size        = size_appl2_section;
    img->entry_point = (vcoreii) ? (CYG_ADDRESS)VCOREII_APPL_MEMORY_ENTRY_POINT : (CYG_ADDRESS)VCOREIII_APPL_MEMORY_ENTRY_POINT;
    img->data_length = size_appl2_file;
    img->file_cksum  = appl2_crc;
    img++;
  }

  // And a descriptor for the configuration data
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_REDBOOT_CONFIG);
  img->flash_base = (CYG_ADDRESS)flash_start + redboot_config_offset;
  img->mem_base   = img->flash_base;
  img->size       = cfg_size;
  img++;

  // And a descriptor for the descriptor table itself
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_FIS_DIRECTORY);
  img->flash_base = (CYG_ADDRESS)flash_start + fis_directory_offset;
  img->mem_base   = (CYG_ADDRESS)flash_start + fis_directory_offset;
  img->size       = fisdir_size;
  img++;

  // Create the entry for the redundant fis table
  ASSERT_IMG_VALID(img);
  memset(img, 0, sizeof(*img));
  strcpy(img->u.name, FIS_NAME_REDUNDANT_FIS);
  img->flash_base = (CYG_ADDRESS)flash_start + redundant_fis_offset;
  img->mem_base   = (CYG_ADDRESS)flash_start + redundant_fis_offset;
  img->size       = fisdir_size;
  img++;

  fis_endian_fixup((struct fis_image_desc *)workspace_end);
  return (u8 *)img - workspace_end; // Number of valid bytes in directory
}

/******************************************************************************/
/******************************************************************************/
static bool write_fis_dir(FILE *output_file,         bool primary,
                          u32 redboot_offset,        u32 size_redboot_section,        u32 size_redboot_file, u32 redboot_crc,
                          u32 conf_offset,           u32 size_conf_section,
                          u32 stackconf_offset,      u32 size_stackconf_section,
                          u32 syslog_offset,         u32 size_syslog_section,
                          u32 appl1_offset,          u32 size_appl1_section,          u32 size_appl1_file,   u32 appl1_crc,
                          u32 appl2_offset,          u32 size_appl2_section,          u32 size_appl2_file,   u32 appl2_crc,
                          u32 redboot_config_offset,
                          u32 fis_directory_offset,
                          u32 redundant_fis_offset,
                          u32 size_largest_sector,
                          u32 pad_to)
{
  u32 len;

  // Various initializations required by fis_init().
  fisdir_size = CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_COUNT * CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_SIZE; // Size of FIS directory
  fisdir_size = ((fisdir_size + size_largest_sector - 1) / size_largest_sector) * size_largest_sector;
  if(fisdir_size > sizeof(workspace_end)) {
    printf("Internal error: workspace_end (%u) not big enough to hold one fisdir (%lu)\n", sizeof(workspace_end), fisdir_size);
    exit(-1);
  }

  flash_start = (vcoreii) ? (void *)VCOREII_FLASH_START : (void *)VCOREIII_FLASH_START;

  cfg_base = flash_start + redboot_config_offset;
  cfg_size = sizeof(struct _config);

  len = fis_init(output_file,
    redboot_offset,        size_redboot_section,   size_redboot_file, redboot_crc,
    conf_offset,           size_conf_section,
    stackconf_offset,      size_stackconf_section,
    syslog_offset,         size_syslog_section,
    appl1_offset,          size_appl1_section,    size_appl1_file, appl1_crc,
    appl2_offset,          size_appl2_section,    size_appl2_file, appl2_crc,
    redboot_config_offset,
    fis_directory_offset,
    redundant_fis_offset);

  if(verbose) {
    printf("0x%08lx - 0x%08lx: Writing %s\n", ftell(output_file), ftell(output_file) + len, primary ? FIS_NAME_FIS_DIRECTORY : FIS_NAME_REDUNDANT_FIS);
  }

  if(my_fwrite(workspace_end, 1, len, output_file) != len) {
    printf("Error: Couldn't write to output file\n");
    return false;
  }

  return pad(output_file, pad_to);
}

/******************************************************************************/
/******************************************************************************/
static bool write_local_cfg(FILE *output_file, u32 pad_to)
{
  u32 len = strlen(conf_section) + 1;
  if(verbose) {
    printf("0x%08lx - 0x%08lx: Writing '%s' section\n", ftell(output_file), ftell(output_file) + len, FIS_NAME_CONF);
  }

  if(my_fwrite(conf_section, 1, len, output_file) != len) {
    printf("Error: Couldn't write to output file\n");
    return false;
  }

  return pad(output_file, pad_to);
}

/******************************************************************************/
/******************************************************************************/
static void init_conf_section(void)
{
  memset(conf_section, 0, sizeof(conf_section));
  conf_section_ptr = &conf_section[0];
  conf_section_ptr += sprintf(conf_section_ptr, "%s", FLASH_CONF_BOARD_SIG);
}

/******************************************************************************/
/******************************************************************************/
static void init_boot_script(void)
{
  memset(boot_script, 0, sizeof(boot_script));
  boot_script_ptr = &boot_script[0];
}

/******************************************************************************/
/******************************************************************************/
static void usage(char *err_str)
{
  char format_str[] = "  %-16s %s\n";
  if(err_str) {
    printf("Error:\n  ****\n  **** %s.\n  ****\n\n", err_str);
  } else {
    printf("This tool merges RedBoot and a WebStaX Application image into one\nsingle output file.\n\n");
  }
  printf("Usage:\n  %s [options] -r <redboot_bin> -w <webstax_bin> -o <output_file>\n\n", prog_name);

  printf("Required arguments:\n");
  printf(format_str, "-r <redboot_bin>", "Binary RedBoot input file v. 1.07 or later.");
  printf(format_str, "-w <webstax_bin>", "Binary WebStaX input file (extension is either .gz or .bin).");
  printf(format_str, "-o <output_file>", "Output File.");

  printf("\nOptional arguments:\n");
  printf(format_str, "-a <attr=val>",    "Add this attribute and value to the 'conf' FIS section.");
  printf(format_str, "",                 "This option may be specified several times on the command line.");
  printf(format_str, "",                 "Besides allowing for adding a MAC Address and Board ID, it allows");
  printf(format_str, "",                 "for customer specific extensions.");
  printf(format_str, "",                 "The 'attr' part may contain underscores and any combination of digits and letters.");
  printf(format_str, "",                 "The 'val' part may be any printable character.");
  printf(format_str, "",                 "Typical usage is '-a MAC=xx:xx:xx:xx:xx:xx' and '-a BOARDID=xxxxx'.");
  printf(format_str, "-h",               "Print this help text and exit.");
  printf(format_str, "-n",               "Only single-image support. If not specified,");
  printf(format_str, "",                 "you'll get dual image support.");
  printf(format_str, "-sa <KBytes>",     "Size - in KBytes - of 'managed' and 'managed.bk' FIS entries.");
  printf(format_str, "",                 "Valid values: > size of application image. Default is 3072.");
  printf(format_str, "-sb <KBytes>",     "Size - in KBytes - of the largest flash block (sector).");
  printf(format_str, "",                 "This is only used when placing the 'RedBoot config',");
  printf(format_str, "",                 "'FIS Directory', and 'Redundant FIS' sections.");
  printf(format_str, "",                 "Note that if the largest sector size in the flash is 128 KBytes, but");
  printf(format_str, "",                 "the flash contains smaller sectors (e.g. 8 KBytes) at the end, you");
  printf(format_str, "",                 "should still specify 128 KBytes on the command line, and not 8!");
  printf(format_str, "",                 "Valid values: >= 8, but normally only 64 and 128. Default is 128.");
  printf(format_str, "-sc <KBytes>",     "Size - in KBytes - of 'conf' FIS entry.");
  printf(format_str, "",                 "Valid values: > 0. Default is 128.");
  printf(format_str, "-sf <MBytes>",     "Size - in MBytes - of final image (i.e. the flash size).");
  printf(format_str, "",                 "Valid values: > 0 && < 128. Default is 16.");
  printf(format_str, "-sl <KBytes>",     "Size - in KBytes - of 'syslog' FIS entry.");
  printf(format_str, "",                 "Valid values: >= 0 (0 disables syslog). Default is 256.");
  printf(format_str, "-sr <KBytes>",     "Size - in KBytes - of 'RedBoot' FIS entry.");
  printf(format_str, "",                 "Valid values: > size of RedBoot image. Default is 256.");
  printf(format_str, "-ss <KBytes>",     "Size - in KBytes - of 'stackconf' FIS entry.");
  printf(format_str, "",                 "Valid values: > 0. Default is 1024.");
  printf(format_str, "-t <script_line>", "Add this line to the boot script.");
  printf(format_str, "",                 "This option may be specified several times on the command line.");
  printf(format_str, "",                 "If no -t options are given, then this default boot script will be used:");
  printf(format_str, "",                 "  diag -a");
  printf(format_str, "",                 "  fis load -d managed");
  printf(format_str, "",                 "  go");
  printf(format_str, "",                 "  fis load -d managed.bk");
  printf(format_str, "",                 "  go");
  printf(format_str, "",                 "The latter two lines are only added if the -n option is not used.");
  printf(format_str, "",                 "If at least one -t option is given, you will have to specify");
  printf(format_str, "",                 "the full script yourself. The lines will be added in command-line order.");
  printf(format_str, "",                 "Enclose each <script_line> in \"\".");
  printf(format_str, "-v",               "Verbose output.");
  printf(format_str, "-V",               "Print version number of this tool and exit.");
  printf(format_str, "-ii",              "For vcore-ii.");
  printf("\n");
  printf("Example1: Create a 16MByte flash image with default layout:\n");
  printf("  %s -r redboot.bin -w smb_switch.gz -o flash16.bin -a \"MAC=00:01:c1:00:3a:e0\" -a \"BOARDID=55\"\n\n", prog_name);
  printf("Example2: Create an 8MByte flash image with default layout:\n");
  printf("  %s -r redboot.bin -w smb_switch.gz -o flash8.bin -a \"MAC=00:01:c1:00:3a:e0\" -a \"BOARDID=55\" -sf 8\n", prog_name);
}

/******************************************************************************/
/******************************************************************************/
int main(int argc, char *argv[])
{
  bool attr_val_pair_set          = false; // -a
  bool single_img_set             = false; // -n
  bool size_appl_section_set      = false; // -sa
  bool size_largest_sector_set    = false; // -sb
  bool size_conf_section_set      = false; // -sc
  bool size_flash_set             = false; // -sf
  bool size_syslog_section_set    = false; // -sl
  bool size_redboot_section_set   = false; // -sr
  bool size_stackconf_section_set = false; // -ss
  bool boot_script_set            = false; // -t
  bool redboot_filename_set       = false;
  bool appl_filename_set          = false;
  bool output_filename_set        = false;

  // Command-line argument defaults
  u32  size_flash                 = DEFAULT_SIZE_FLASH;
  u32  size_largest_sector        = DEFAULT_SIZE_SECTOR;
  u32  size_redboot_section       = DEFAULT_SIZE_REDBOOT;
  u32  size_conf_section          = DEFAULT_SIZE_CONF;
  u32  size_stackconf_section     = DEFAULT_SIZE_STACKCONF;
  u32  size_syslog_section        = DEFAULT_SIZE_SYSLOG;
  u32  size_appl_section          = DEFAULT_SIZE_APPL;
  bool dual_image_support         = true;

  // Auto-vars
  char *redboot_filename          = NULL;
  char *appl_filename             = NULL;
  char *output_filename           = NULL;
  int  i, result                  = -1; // Assume error
  FILE *redboot_file              = NULL;
  FILE *appl_file                 = NULL;
  FILE *output_file               = NULL;
  u32  size_redboot_file, size_appl_file, pad_to = 0, total_size;
  u32  redboot_offset, conf_offset, stackconf_offset, syslog_offset, appl1_offset, appl2_offset, redboot_config_offset, fis_directory_offset, redundant_fis_offset, size_appl2_section;
  u32  redboot_crc, appl_crc;

  // Initialize conf section
  init_conf_section();

  // Initialize boot script section.
  init_boot_script();

  prog_name = argv[0] + strlen(argv[0]) - 1;
  while(prog_name > argv[0] && *(prog_name - 1) != '\\' && *(prog_name - 1) != '/')
    prog_name--;

  for(i = 1; i < argc; i++) {

    if(!strcmp(argv[i], "-a")) {
      // -a <attr=val>. Add attr=val to conf section.
      char *attr_ptr, *val_ptr;
      i++;  // Eat an argument

      if(conf_section_ptr - &conf_section[0] + strlen(argv[i]) >= sizeof(conf_section)) {
        usage("-a option: conf section size exceeded");
        return -1;
      }

      attr_ptr = argv[i];
      val_ptr  = strstr(argv[i], "=");
      if(val_ptr == NULL) {
        usage("-a arguments are on the form <attr=val>");
        return -1;
      }

      // Validate attr
      while(*attr_ptr != '=') {
        if(!isalpha(*attr_ptr) && !isdigit(*attr_ptr) && *attr_ptr != '_') {
          usage("-a option's attr-part only takes letters or digits.");
          return -1;
        }
        attr_ptr++;
      }

      while(*val_ptr != '\0') {
        if(!isprint(*val_ptr)) {
          usage("-a option's val-part only takes printable characters.");
          return -1;
        }
        val_ptr++;
      }

      conf_section_ptr += sprintf(conf_section_ptr, "%s\n", argv[i]);
      attr_val_pair_set = true;
    } else if(!strcmp(argv[i], "-h")) {
      // -h. Help.
      usage(NULL);
      return 0;

    } else if(!strcmp(argv[i], "-n")) {
      // -n. No dual image support.
      if(single_img_set) {
        usage("-n can only be specified once on the command line");
        return -1;
      }
      single_img_set = true;
      dual_image_support = false;

    } else if(!strcmp(argv[i], "-sa")) {
      // -sa. Size of 'managed' and 'managed.bk' sections in KBytes.
      if(size_appl_section_set) {
        usage("-sa can only be specified once on the command line");
        return -1;
      }
      size_appl_section_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_appl_section) != 1) {
        usage("Invalid format of <KBytes> for -sa option");
        return -1;
      }
      if(size_appl_section == 0) {
        usage("'managed'/'managed.bk' sizes must be > 0");
        return -1;
      }
      size_appl_section *= 1024;
      if((size_appl_section % SECTOR_SIZE_BYTES) != 0) {
        usage("'managed'/'managed.bk' sizes must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-sb")) {
      // -sb. Size of the largest flash sector.
      if(size_largest_sector_set) {
        usage("-sb can only be specified once on the command line");
        return -1;
      }
      size_largest_sector_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_largest_sector) != 1) {
        usage("Invalid format of <KBytes> for -sb option");
        return -1;
      }
      if(size_largest_sector < 8) {
        usage("-sb size must be >= 8");
        return -1;
      }
      if((ALLOWED_MAXIMUM_SECTOR_SIZE == 128) && (size_largest_sector != 64 && size_largest_sector != 128)) {
        printf("*** Warning (-sb option): It is *VERY* unusual that the size ***\n*** of the largest sector is different from 64 and 128 KBytes.***\n");
      }
      size_largest_sector *= 1024;

    } else if(!strcmp(argv[i], "-sc")) {
      // -sc. Size of 'conf' section in KBytes.
      if(size_conf_section_set) {
        usage("-sc can only be specified once on the command line");
        return -1;
      }
      size_conf_section_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_conf_section) != 1) {
        usage("Invalid format of <KBytes> for -sc option");
        return -1;
      }
      if(size_conf_section == 0) {
        usage("'conf' size must be > 0");
        return -1;
      }
      size_conf_section *= 1024;
      if((size_conf_section % SECTOR_SIZE_BYTES) != 0) {
        usage("'conf' size must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-sf")) {
      // -sf. Size of flash in MBytes.
      if(size_flash_set) {
        usage("-sf can only be specified once on the command line");
        return -1;
      }
      size_flash_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_flash) != 1) {
        usage("Invalid format of <MBytes> in -sf option");
        return -1;
      }
      if(size_flash == 0 || size_flash > 128) { // Limit the output file size.
        usage("Flash size must be > 0 and <= 128 MBytes");
        return -1;
      }
      size_flash *= 1024 * 1024;
      if((size_flash % SECTOR_SIZE_BYTES) != 0) {
        usage("Flash size must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-sl")) {
      // -sl. Size of 'syslog' section in KBytes.
      if(size_syslog_section_set) {
        usage("-sl can only be specified once on the command line");
        return -1;
      }
      size_syslog_section_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_syslog_section) != 1) {
        usage("Invalid format of <KBytes> for -sl option");
        return -1;
      }
      // No limit on syslog size. User doesn't want a syslog if 0 is specified.
      size_syslog_section *= 1024;
      if((size_syslog_section % SECTOR_SIZE_BYTES) != 0) {
        usage("'syslog' size must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-sr")) {
      // -sr. Size of 'RedBoot' section in KBytes.
      if(size_redboot_section_set) {
        usage("-sr can only be specified once on the command line");
        return -1;
      }
      size_redboot_section_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_redboot_section) != 1) {
        usage("Invalid format of <KBytes> for -sr option");
        return -1;
      }
      if(size_redboot_section == 0) {
        usage("'RedBoot' size must be > 0");
        return -1;
      }
      size_redboot_section *= 1024;
      if((size_redboot_section % SECTOR_SIZE_BYTES) != 0) {
        usage("'RedBoot' size must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-ss")) {
      // -ss. Size of 'stackconf' section in KBytes.
      if(size_stackconf_section_set) {
        usage("-ss can only be specified once on the command line");
        return -1;
      }
      size_stackconf_section_set = true;
      i++; // Eat an argument
      if(sscanf(argv[i], "%lu", &size_stackconf_section) != 1) {
        usage("Invalid format of <KBytes> for -ss option");
        return -1;
      }
      if(size_stackconf_section == 0) {
        usage("'stackconf' size must be > 0");
        return -1;
      }
      size_stackconf_section *= 1024;
      if((size_stackconf_section % SECTOR_SIZE_BYTES) != 0) {
        usage("'stackconf' size must be a multiplum of the sector size");
        return -1;
      }

    } else if(!strcmp(argv[i], "-t")) {
      // -t <script_line>. Add a line to the boot script.
      i++; // Eat an arg.
      boot_script_ptr += sprintf(boot_script_ptr, "%s\n", argv[i]);
      boot_script_set = true;
    } else if(!strcmp(argv[i], "-v")) {
      verbose = true;
    } else if(!strcmp(argv[i], "-V")) {
      printf("%s v.%s, built %s %s\n\n",argv[0], BINMERGE_VERSION, __DATE__, __TIME__);
      return 0;
    } else if(!strcmp(argv[i], "-ii")) {
      vcoreii = true;
    } else if(!strcmp(argv[i], "-r")) {
      // -r <RedBoot image>
      if(redboot_filename_set) {
        usage("-r can only be specified once on the command line");
        return -1;
      }
      redboot_filename_set = true;
      i++; // Eat an argument
      if(argv[i] == NULL) {
        usage("-r takes an additional parameter");
        return -1;
      }
      redboot_filename = argv[i];

    } else if(!strcmp(argv[i], "-w")) {
      // -w <application image>
      if(appl_filename_set) {
        usage("-w can only be specified once on the command line");
        return -1;
      }
      appl_filename_set = true;
      i++; // Eat an argument
      if(argv[i] == NULL) {
        usage("-w takes an additional parameter");
        return -1;
      }
      appl_filename = argv[i];

    } else if(!strcmp(argv[i], "-o")) {
      // -o <output file>
      if(output_filename_set) {
        usage("-o can only be specified once on the command line");
        return -1;
      }
      output_filename_set = true;
      i++; // Eat an argument
      if(argv[i] == NULL) {
        usage("-o takes an additional parameter");
        return -1;
      }
      output_filename = argv[i];

    } else {
      // Unknown option.
      printf("Error: Unknown option: %s\n", argv[i]);
      return -1;
    }
  }

  if(redboot_filename_set == false || appl_filename_set == false || output_filename_set == false) {
    usage("Insufficient number of arguments");
    return -1;
  }

  if((redboot_file = fopen(redboot_filename, "rb")) == NULL) {
    printf("Error: Unable to open redboot image (%s) for reading.\n", redboot_filename);
    goto do_exit;
  }
  if((appl_file = fopen(appl_filename, "rb")) == NULL) {
    printf("Error: Unable to open application image (%s) for reading.\n", appl_filename);
    goto do_exit;
  }

  if((output_file = fopen(output_filename, "wb")) == NULL) {
    printf("Error: Unable to open output file (%s) for writing.\n", output_filename);
    goto do_exit;
  }

  // Various sanity checks
  size_redboot_file = fsize(redboot_file);
  if(size_redboot_file > size_redboot_section) {
    printf("Error: RedBoot's size (%lu bytes) is greater than what is reserved in the output image (%lu bytes)\n", size_redboot_file, size_redboot_section);
    goto do_exit;
  }
  size_appl_file = fsize(appl_file);
  if(size_appl_file > size_appl_section) {
    printf("Error: The application's size (%lu bytes) is greater than what is reserved in the output image (%lu bytes)\n", size_appl_file, size_appl_section);
    goto do_exit;
  }

  // Is there room in the flash?
  total_size = size_redboot_section + size_conf_section + size_stackconf_section + size_syslog_section + size_appl_section + 3 * size_largest_sector;
  if(dual_image_support) {
    total_size += size_appl_section;
  }

  if(total_size > size_flash) {
    printf("Error: The various sections add up to more than the flash size (%lu bytes):\n", size_flash);
    printf("  %15s: %10lu bytes\n", FIS_NAME_REDBOOT,   size_redboot_section);
    printf("  %15s: %10lu bytes\n", FIS_NAME_CONF,      size_conf_section);
    printf("  %15s: %10lu bytes\n", FIS_NAME_STACKCONF, size_stackconf_section);
    printf("  %15s: %10lu bytes\n", FIS_NAME_SYSLOG,    size_syslog_section);
    printf("  %15s: %10lu bytes\n", FIS_NAME_APPL1,     size_appl_section);
    if(dual_image_support) {
      printf("  %15s: %10lu bytes\n", FIS_NAME_APPL2, size_appl_section);
    }
    printf("  %15s: %10lu bytes\n", FIS_NAME_REDBOOT_CONFIG, size_largest_sector);
    printf("  %15s: %10lu bytes\n", FIS_NAME_FIS_DIRECTORY,  size_largest_sector);
    printf("  %15s: %10lu bytes\n", FIS_NAME_REDUNDANT_FIS,  size_largest_sector);
    printf("--------------------------------------------------\n");
    printf("  %15s: %10lu bytes\n", "Total", total_size);
    return -1;
  }

  // Provide a default boot script in case no -t options were specified on the command line
  if(!boot_script_set) {
    boot_script_ptr += sprintf(boot_script_ptr,
      "diag -a\n"
      "fis load -d %s\n"
      "go"
      "%s%s\n",
      FIS_NAME_APPL1,
      dual_image_support ? "\nfis load -d "        : "",
      dual_image_support ? FIS_NAME_APPL2 "\ngo" : "");
  }

  if(!attr_val_pair_set) {
    printf("Warning: No 'conf' section <attr, val> pairs were added. This means that the switch gets a default MAC address, which may not be intentional!\n");
  }

  // RedBoot: Default: [0x00000000; 0x00040000[
  redboot_offset = pad_to;
  pad_to        += size_redboot_section;
  if(!copy_file(output_file, redboot_file, pad_to, &redboot_crc, FIS_NAME_REDBOOT)) {
    goto do_exit;
  }

  // 'conf' section. Default: [0x00040000; 0x00060000[
  conf_offset = pad_to;
  pad_to     += size_conf_section;
  if(!write_local_cfg(output_file, pad_to)) {
    goto do_exit;
  }

  // 'stackconf' section. Default [0x00060000; 0x00160000[
  // This section is self-configuring when the application starts.
  stackconf_offset = pad_to;
  pad_to          += size_stackconf_section;
  if(!pad(output_file, pad_to)) {
    goto do_exit;
  }

  // 'syslog' section (optional). Default [0x00160000; 0x001A0000[
  // This section is self-configuring.
  syslog_offset = pad_to;
  pad_to       += size_syslog_section;
  if(!pad(output_file, pad_to)) {
    goto do_exit;
  }

  // Application, img1. Default [0x001A0000; 0x004A0000[
  appl1_offset = pad_to;
  pad_to      += size_appl_section;
  if(!copy_file(output_file, appl_file, pad_to, &appl_crc, FIS_NAME_APPL1)) {
    goto do_exit;
  }

  appl2_offset = pad_to;
  if(dual_image_support) {
    // Application, img2. Default [0x004A0000; 0x007A0000[
    size_appl2_section = size_appl_section;
    pad_to            += size_appl_section;
    if(!copy_file(output_file, appl_file, pad_to, &appl_crc, FIS_NAME_APPL2)) {
      goto do_exit;
    }
  } else {
    size_appl2_section = 0;
  }

  // Jump to the location of the RedBoot configuration, which is 3 user-specified sectors before the end of the flash.
  pad_to = size_flash - 3 * size_largest_sector;
  if(!pad(output_file, pad_to)) {
    goto do_exit;
  }

  redundant_fis_offset = pad_to;
  fis_directory_offset = pad_to + 2*(size_largest_sector);
  redboot_config_offset = pad_to + (size_largest_sector);

// Write the second FIS index (similar to the first)
  pad_to += (size_largest_sector);
  if(!write_fis_dir(output_file, false,
    redboot_offset,        size_redboot_section,   size_redboot_file, redboot_crc,
    conf_offset,           size_conf_section,
    stackconf_offset,      size_stackconf_section,
    syslog_offset,         size_syslog_section,
    appl1_offset,          size_appl_section,      size_appl_file,    appl_crc,
    appl2_offset,          size_appl2_section,     size_appl_file,    appl_crc,
    redboot_config_offset,
    fis_directory_offset,
    redundant_fis_offset,
    size_largest_sector, pad_to)) {
    goto do_exit;
  }

  pad_to += (size_largest_sector);
  if(!write_redboot_cfg(output_file, pad_to)) {
    goto do_exit;
  }

  pad_to += (size_largest_sector);
  if(!write_fis_dir(output_file, true,
    redboot_offset,        size_redboot_section,   size_redboot_file, redboot_crc,
    conf_offset,           size_conf_section,
    stackconf_offset,      size_stackconf_section,
    syslog_offset,         size_syslog_section,
    appl1_offset,          size_appl_section,      size_appl_file,    appl_crc,
    appl2_offset,          size_appl2_section,     size_appl_file,    appl_crc,
    redboot_config_offset,
    fis_directory_offset,
    redundant_fis_offset,
    size_largest_sector, pad_to)) {
    goto do_exit;
  }

  if(pad_to != size_flash) {
    printf("Error: Internal error. The pad_to (%lu) didn't wind up being the same as the flash size (%lu)\n", pad_to, size_flash);
    goto do_exit;
  }

  // Everything is fine.
  result = 0;

do_exit:
  fclose(redboot_file);
  fclose(appl_file);
  fclose(output_file);
  return result;
}

