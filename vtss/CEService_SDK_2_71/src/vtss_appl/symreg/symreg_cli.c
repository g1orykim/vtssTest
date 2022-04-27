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

#include "cli_api.h"         /* For cli_xxx()                                           */
#include "cli.h"             /* For cli_req_t (sadly enough)                            */
#include "mgmt_api.h"        /* For mgmt_txt2bf()                                       */
#include "symreg_api.h"      /* Check that public function decls. and defs. are in sync */
#include "vtss_api_if_api.h" /* For vtss_api_ip_chip_count()                            */

#if defined(VTSS_ARCH_JAGUAR_1)
  #include "jaguar_regs.c"
#elif defined(VTSS_ARCH_LUTON26)
  #include "luton26_regs.c"
#elif defined(VTSS_ARCH_SERVAL)
  #include "serval_regs.c"
#else
  #error "Registers not generated for this architecture"
#endif

#define SYMREG_CLI_PATH "Debug Sym "
#define SYMREG_REPL_LEN_MAX 32 /* Allocate 32 chars for the user to specify a replication list */

typedef enum {
  SYMREG_CLI_TGT,
  SYMREG_CLI_REGGRP,
  SYMREG_CLI_REG,
  SYMREG_CLI_LAST
} symreg_cli_components_t;

#define SYMREG_COMPONENT_NAME(_comp_) ((_comp_) == SYMREG_CLI_TGT ? "target" : (_comp_) == SYMREG_CLI_REGGRP ? "reggrp" : (_comp_) == SYMREG_CLI_REG ? "register" : "unknown")

/******************************************************************************/
// This defines the things that this module can parse.
// The fields are filled in by the dedicated parsers.
/******************************************************************************/
typedef struct {
  char           names[SYMREG_CLI_LAST][SYMREG_NAME_LEN_MAX + SYMREG_REPL_LEN_MAX + 1];
  u8             repls[SYMREG_CLI_LAST][VTSS_BF_SIZE(SYMREG_REPL_CNT_MAX)];
  BOOL           all_repls[SYMREG_CLI_LAST];
  BOOL           verify;
  BOOL           first;
  BOOL           write;
  u32            addr_cnt;
  u32            value;
  BOOL           found[SYMREG_CLI_LAST];
  vtss_chip_no_t chip_no;
  int            chip_count;
  int            width;
} symreg_cli_req_t;

/****************************************************************************/
// SYMREG_cli_parse_value()
/****************************************************************************/
static int32_t SYMREG_cli_parse_value(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
  symreg_cli_req_t *symreg_req = req->module_req;
  return cli_parse_ulong(cmd, &symreg_req->value, 0, 0xffffffff);
}

/****************************************************************************/
// SYMREG_cli_parse_reg_syntax()
/****************************************************************************/
static int32_t SYMREG_cli_parse_reg_syntax(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
  symreg_cli_req_t *symreg_req = req->module_req;
  int              i;
  char             *str1 = cmd, *str2;
  size_t           cnt;
  BOOL             at_least_one_component_found = FALSE;

  for (i = 0; i < SYMREG_CLI_LAST; i++) {
    if (str1 == NULL) {
      break;
    }
    str2 = strstr(str1, ":");
    if (str2) {
      // Colon found
      if (i == SYMREG_CLI_REG) {
        CPRINTF("Syntax error: Too many colons\n");
        return VTSS_RC_ERROR;
      }

      cnt = str2 - str1;
    } else {
      // Colon not found.
      cnt = strlen(str1);
    }
    if (cnt + 1 > sizeof(symreg_req->names[i])) {
      CPRINTF("Syntax error: %s too long (%d). Only room for %d chars\n", SYMREG_COMPONENT_NAME(i), cnt, sizeof(symreg_req->names[i]) - 1);
      return VTSS_RC_ERROR;
    }
    if (cnt > 0) {
      strncpy(symreg_req->names[i], str1, cnt + 1);
      // Terminate in case a colon was found in str1.
      symreg_req->names[i][cnt] = '\0';
      at_least_one_component_found = TRUE;
    }
    if (str2 && str2[0] == ':') {
      str1 = str2 + 1;
    } else {
      str1 = NULL;
    }
  }

  if (!at_least_one_component_found) {
    CPRINTF("Syntax error: At least one target, one register group, or one register must be specified. You don't want to see all registers in the chip\n");
    return VTSS_RC_ERROR;
  }

  // Loop once more and find replications.
  for (i = 0; i < SYMREG_CLI_LAST; i++) {
    char *bracket_start = NULL;
    char *bracket_end   = NULL;
    int  j;

    str1 = symreg_req->names[i];
    cnt = strlen(str1);
    for (j = 0; j < (int)cnt; j++) {
      *str1 = toupper(*str1);
      if (*str1 == '[') {
        if (bracket_start != NULL) {
          CPRINTF("Error: Two left-brackets seen in %s.\n", SYMREG_COMPONENT_NAME(i));
          return VTSS_RC_ERROR;
        }
        bracket_start = str1;
      } else if (bracket_end == NULL && *str1 == ']') {
        if (bracket_start == NULL) {
          CPRINTF("Error: Right-bracket seen before left-bracket in %s.\n", SYMREG_COMPONENT_NAME(i));
          return VTSS_RC_ERROR;
        }
        bracket_end = str1;
      } else if (bracket_end != NULL) {
        CPRINTF("Error: Extra characters after end-bracket in %s.\n", SYMREG_COMPONENT_NAME(i));
        return VTSS_RC_ERROR;
      }
      str1++;
    }

    // If no brackets are found, or an empty list is given (e.g. dev1g[<nothing_here>]:xxx:xxx), show all replications.
    if (bracket_start == NULL || bracket_end == NULL || bracket_end == bracket_start + 1) {
      symreg_req->all_repls[i] = TRUE;
    } else {
      symreg_req->all_repls[i] = FALSE;

      // Parse the list, but first terminate it in both ends.
      *bracket_start = *bracket_end = '\0';
      if (mgmt_txt2bf(bracket_start + 1, symreg_req->repls[i], 0, SYMREG_REPL_CNT_MAX - 1, 0) != VTSS_OK) {
        CPRINTF("Error: Invalid replication list within brackets of %s.\n", SYMREG_COMPONENT_NAME(i));
        return VTSS_RC_ERROR;
      }
    }
  }

  return VTSS_RC_OK;
}

/****************************************************************************/
// SYMREG_cli_error()
/****************************************************************************/
static void SYMREG_cli_error(symreg_cli_req_t *symreg_req, symreg_cli_components_t component)
{
  if (symreg_req->names[component][0] == '\0') {
    CPRINTF("Error: No such %s replication\n", SYMREG_COMPONENT_NAME(component));
  } else {
    CPRINTF("Error: No such %s: %s\n", SYMREG_COMPONENT_NAME(component), symreg_req->names[component]);
  }
}

/****************************************************************************/
// SYMREG_cli_print_component()
/****************************************************************************/
static char *SYMREG_cli_print_component(char *p, char *name, int repl, u32 repl_cnt, char *post_str)
{
  char buf[13];
  if (repl_cnt == 1) {
    buf[0] = '\0';
  } else {
    sprintf(buf, "[%d]", repl);
  }
  return (p + sprintf(p, "%s%s%s", name, buf, post_str));
}

/****************************************************************************/
// SYMREG_cli_reg_loop()
/****************************************************************************/
static void SYMREG_cli_reg_loop(symreg_cli_req_t *symreg_req, const SYMREG_target_t *tgt, const SYMREG_reggrp_t *reggrp, int reggrp_repl, int reg_repl)
{
  const SYMREG_reg_t *reg = reggrp->regs;
  int                reg_repl_min, reg_repl_max, reg_repl_iter, i; 
  u32                value = 0;
  char               buf[128], *p;

  while (reg->name != NULL) {
    if ((symreg_req->names[SYMREG_CLI_REG][0] == '\0' || strcmp(symreg_req->names[SYMREG_CLI_REG], reg->name) == 0) &&
        (reg_repl == -1 || reg_repl < (int)reg->repl_cnt)) {
      // Either the user wants all regs (names[][0] = '\0') or a specific reg
      // and either he wants all replications (reg_repl == -1) or a specific replication.
      // Either way, a match is found.
      symreg_req->found[SYMREG_CLI_REG] = TRUE;

      if (reg_repl == -1) {
        // User wants all replications of this register
        reg_repl_min = 0;
        reg_repl_max = reg->repl_cnt - 1;
      } else {
        // User wants specific replication of this register
        reg_repl_min = reg_repl;
        reg_repl_max = reg_repl;
      }

      // Now it's time to figure out what to do. We can get here in three ways:
      //   1) Read the register
      //   2) Write the register
      //   3) Verify that the register exists.
      for (reg_repl_iter = reg_repl_min; reg_repl_iter <= reg_repl_max; reg_repl_iter++) {
        symreg_req->addr_cnt++;
        p = buf;
        p = SYMREG_cli_print_component(p, tgt->name,    tgt->repl_number, tgt->repl_number == -1 ? 1 : 2, ":");
        p = SYMREG_cli_print_component(p, reggrp->name, reggrp_repl,      reggrp->repl_cnt,               ":");
        p = SYMREG_cli_print_component(p, reg->name,    reg_repl_iter,    reg->repl_cnt,                  "");

        if (symreg_req->verify) {
          // Calculate register string width
          i = strlen(buf);
          if (i > symreg_req->width)
            symreg_req->width = i;
        } else {
          // Compute the address. We need the 32-bit address offset relative to
          // the beginning of the switch core base address because that's what the
          // vtss_reg_read/write() functions operate with. These functions are luckily
          // able to access CPU-domain registers and are also dual-chip aware.
          u32 addr_val = tgt->base_addr + 4 * (reggrp->base_addr + (reggrp_repl * reggrp->repl_width) + (reg_repl_iter * reg->repl_width) + reg->addr);
          vtss_rc rc;

          // addr_val is now including the offset to the switch core registers, so we need to
          // subtract that base address.
          addr_val -= VTSS_IO_ORIGIN1_OFFSET;

          // addr_val is now a byte-address, which we must convert to a 32-bit address
          addr_val >>= 2;

          if (symreg_req->write) {
            rc = vtss_reg_write(NULL, symreg_req->chip_no, addr_val, symreg_req->value);
            value = symreg_req->value;
          } else {
            rc = vtss_reg_read(NULL, symreg_req->chip_no, addr_val, &value);
          }

          if (symreg_req->first) {
            if (symreg_req->chip_count > 1) {
              CPRINTF("\nChip #%u\n", symreg_req->chip_no);
              CPRINTF("-------\n\n");
            }
            CPRINTF("%-*s  %-10s  %-10s  31     24 23     16 15      8 7       0\n",
                    symreg_req->width, "Register", "Value", "Decimal");
            symreg_req->first = FALSE;
          }

          CPRINTF("%-*s  ", symreg_req->width, buf);
          if (rc == VTSS_RC_OK) {
              CPRINTF("0x%08x  %-10u  ", value, value);
              for (i = 31; i >= 0; i--) {
                CPRINTF("%d%s", value & (1 << i) ? 1 : 0, i == 0 ? "\n" : (i % 4) ? "" : ".");
              }
          } else {
            CPRINTF("vtss_reg_read/write() returned an error\n");
          }
        }
      }
    }
    reg++;
  }
}

/****************************************************************************/
// SYMREG_cli_reggrp_loop()
/****************************************************************************/
static void SYMREG_cli_reggrp_loop(symreg_cli_req_t *symreg_req, const SYMREG_target_t *tgt, int reggrp_repl)
{
  int                   reggrp_repl_min, reggrp_repl_max, reggrp_repl_iter, reg_repl;
  const SYMREG_reggrp_t *reggrp = tgt->reggrps;

  while (reggrp->name != NULL) {
    if ((symreg_req->names[SYMREG_CLI_REGGRP][0] == '\0' || strcmp(symreg_req->names[SYMREG_CLI_REGGRP], reggrp->name) == 0) &&
        (reggrp_repl == -1 || reggrp_repl < (int)reggrp->repl_cnt)) {
      // Either the user wants all reggrps (names[][0] = '\0') or a specific reggrp
      // and either he wants all replications (reggrp_repl == -1) or a specific replication.
      // Either way, a match is found.
      symreg_req->found[SYMREG_CLI_REGGRP] = TRUE;

      if (reggrp_repl == -1) {
        // User wants all replications of this register
        reggrp_repl_min = 0;
        reggrp_repl_max = reggrp->repl_cnt - 1;
      } else {
        // User wants specific replication of this register
        reggrp_repl_min = reggrp_repl;
        reggrp_repl_max = reggrp_repl;
      }

      for (reggrp_repl_iter = reggrp_repl_min; reggrp_repl_iter <= reggrp_repl_max; reggrp_repl_iter++) {
        // In order to detect invalid requested replications, we have to loop over user-requested replications
        // if not all are requested, and we have to loop over all registers if all replications are requested.
        if (symreg_req->all_repls[SYMREG_CLI_REG]) {
          // All replications are requested.
          SYMREG_cli_reg_loop(symreg_req, tgt, reggrp, reggrp_repl_iter, -1);
        } else {
          // Only selected replications are requested.
          // Turn the loop upside down and iterate over user's request.
          for (reg_repl = 0; reg_repl < SYMREG_REPL_CNT_MAX; reg_repl++) {
            if (VTSS_BF_GET(symreg_req->repls[SYMREG_CLI_REG], reg_repl) == 0) {
              continue;
            }
            SYMREG_cli_reg_loop(symreg_req, tgt, reggrp, reggrp_repl_iter, reg_repl);
          }
        }
      }
    }
    reggrp++;
  }
}

/****************************************************************************/
// SYMREG_cli_tgt_loop()
/****************************************************************************/
static void SYMREG_cli_tgt_loop(symreg_cli_req_t *symreg_req, int tgt_repl)
{
  int t, reggrp_repl;

  for (t = 0; t < (int)ARRSZ(SYMREG_targets); t++) {
    const SYMREG_target_t *tgt = &SYMREG_targets[t];
    if ((symreg_req->names[SYMREG_CLI_TGT][0] == '\0' || strcmp(symreg_req->names[SYMREG_CLI_TGT], tgt->name) == 0) &&
        (tgt_repl == -1 || (tgt->repl_number == -1 && tgt_repl == 0) || (tgt->repl_number == tgt_repl))) {
      // Either the user wants all targets (names[][0] = '\0') or a specific target
      // and either he wants all replications (tgt_repl == -1) or a specific replication.
      // Either way, a match is found.
      // tgt->repl_number is -1 if the target is not replicated. We do support specifying 0 as target replication for such targets.
      symreg_req->found[SYMREG_CLI_TGT] = TRUE;

      // In order to detect invalid requested replications, we have to loop over user-requested replications
      // if not all are requested, and we have to loop over all reggrps if all replications are requested.
      if (symreg_req->all_repls[SYMREG_CLI_REGGRP]) {
        // All replications are requested.
        SYMREG_cli_reggrp_loop(symreg_req, tgt, -1);
      } else {
        // Only selected replications are requested.
        // Turn the loop upside down and iterate over user's request.
        for (reggrp_repl = 0; reggrp_repl < SYMREG_REPL_CNT_MAX; reggrp_repl++) {
          if (VTSS_BF_GET(symreg_req->repls[SYMREG_CLI_REGGRP], reggrp_repl) == 0) {
            continue;
          }
          SYMREG_cli_reggrp_loop(symreg_req, tgt, reggrp_repl);
        }
      }
    }
  }
}

/****************************************************************************/
// SYMREG_cli_do_read_write()
/****************************************************************************/
static void SYMREG_cli_do_read_write(symreg_cli_req_t *symreg_req)
{
  int tgt_repl;

  // In order to detect invalid requested replications, we have to loop over user-requested replications
  // if not all are requested, and we have to loop over all targets if all replications are requested.
  if (symreg_req->all_repls[SYMREG_CLI_TGT]) {
    // All replications are requested.
    SYMREG_cli_tgt_loop(symreg_req, -1);
  } else {
    // Only selected replications are requested.
    // Turn the loop upside down and iterate over user's request.
    for (tgt_repl = 0; tgt_repl < SYMREG_REPL_CNT_MAX; tgt_repl++) {
      if (VTSS_BF_GET(symreg_req->repls[SYMREG_CLI_TGT], tgt_repl) == 0) {
        continue;
      }
      SYMREG_cli_tgt_loop(symreg_req, tgt_repl);
    }
  }
}

/****************************************************************************/
// SYMREG_cli_read_write()
/****************************************************************************/
static void SYMREG_cli_read_write(symreg_cli_req_t *symreg_req)
{
  symreg_cli_components_t component;
  vtss_chip_no_t          requested_chip_no, min_chip_no, max_chip_no;

  // Get the chip number that the user wants to read.
  (void)misc_chip_no_get(&requested_chip_no);

  // Get the number of chips making up this target.
  symreg_req->chip_count = vtss_api_if_chip_count();

  if (requested_chip_no == VTSS_CHIP_NO_ALL) {
    min_chip_no = 0;
    max_chip_no = symreg_req->chip_count - 1;
  } else if ((int)requested_chip_no >= symreg_req->chip_count) {
    CPRINTF("Error: Invalid chip number requested (%u). Only %d chips supported\n", requested_chip_no, symreg_req->chip_count);
    return;
  } else {
    min_chip_no = max_chip_no = requested_chip_no;
  }

  // Gotta loop twice. The first time we just verify that the user's request is valid, so that
  // we don't start reading and writing registers only to find out later that e.g. a replication
  // doesn't exist.
  symreg_req->verify = TRUE;
  SYMREG_cli_do_read_write(symreg_req);

  for (component = 0; component < SYMREG_CLI_LAST; component++) {
    if (!symreg_req->found[component]) {
      SYMREG_cli_error(symreg_req, component);
      return;
    }
  }

  symreg_req->verify = FALSE;
  for (requested_chip_no = min_chip_no; requested_chip_no <= max_chip_no; requested_chip_no++) {
    symreg_req->first  = TRUE;
    symreg_req->chip_no = requested_chip_no;
    SYMREG_cli_do_read_write(symreg_req);
  }
}

/****************************************************************************/
// SYMREG_cli_cmd_read()
/****************************************************************************/
static void SYMREG_cli_cmd_read(cli_req_t *req)
{
  symreg_cli_req_t *symreg_req = req->module_req;
  symreg_req->write = FALSE;
  SYMREG_cli_read_write(symreg_req);
}

/****************************************************************************/
// SYMREG_cli_cmd_write()
/****************************************************************************/
static void SYMREG_cli_cmd_write(cli_req_t *req)
{
  symreg_cli_req_t *symreg_req = req->module_req;
  symreg_req->write = TRUE;
  SYMREG_cli_read_write(symreg_req);
}

/****************************************************************************/
// Parameter table
/****************************************************************************/
static cli_parm_t SYMREG_cli_parm_table[] = {
  {
    .txt            = "<reg_syntax>",
    .help           = "Register Syntax: target[t]:reggrp[g]:reg[r]\n"
                      "where\n"
                      "  'target' is the name of the target (e.g. dev).\n"
                      "  'reggrp' is the name of the register group. May be omitted.\n"
                      "  'reg'    is the name of the register.\n"
                      "  t        is a list of target replications if applicable.\n"
                      "  g        is a list of register group replications if applicable.\n"
                      "  r        is a list of register replications if applicable.\n"
                      "If a given replication (t, g, r) is omitted, all applicable replications will be accessed.\n"
                      "If reggrp is omitted, add two consecutive colons.\n"
                      "Example 1. dev1g[0-7,12]::dev_ptp_tx_id[0-3]\n"
                      "Example 2. dev1g::dev_rst_ctrl\n"
                      "Example 3: ana_ac:aggr[10]:aggr_cfg\n"
                      "Example 4: ana_ac::aggr_cfg\n"
                      "Example 5: ::MAC_ENA_CFG\n",
    .flags          = CLI_PARM_FLAG_NONE,
    .parm_parse_fun = SYMREG_cli_parse_reg_syntax,
    .cmd_fun        = NULL
  },
  {
    .txt            = "<value>",
    .help           = "Register value (0-0xffffffff)",
    .flags          = CLI_PARM_FLAG_SET,
    .parm_parse_fun = SYMREG_cli_parse_value,
    .cmd_fun        = SYMREG_cli_cmd_write
  },
  {
    .txt            = NULL,
    .help           = NULL,
    .flags          = 0,
    .parm_parse_fun = 0,
    .cmd_fun        = NULL
  },
};

/****************************************************************************/
// Command table
/****************************************************************************/
enum {
  SYMREG_CLI_PRIO_READ  = CLI_CMD_SORT_KEY_DEFAULT,
  SYMREG_CLI_PRIO_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry(
  NULL,
  SYMREG_CLI_PATH "Read <reg_syntax>",
  "Read switch chip register(s)",
  SYMREG_CLI_PRIO_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  SYMREG_cli_cmd_read,
  NULL,
  SYMREG_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  SYMREG_CLI_PATH "Write <reg_syntax> <value>",
  "Write switch chip register",
  SYMREG_CLI_PRIO_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  SYMREG_cli_cmd_write,
  NULL,
  SYMREG_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// symreg_cli_init()
/******************************************************************************/
void symreg_cli_init(void)
{
  // Register the size required for this module's structure
  cli_req_size_register(sizeof(symreg_cli_req_t));
}
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
