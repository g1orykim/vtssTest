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

#ifndef _VTSS_CLI_H_
#define _VTSS_CLI_H_

#include <cyg/io/io.h>
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_tables.h>

#include "main.h"
#include "critd_api.h"
#include "cli_api.h"
#include "vtss_module_id.h"
#include "misc_api.h" /* For vtss_uport_no_t */
#if defined(VTSS_SW_OPTION_SYSUTIL)
#include "sysutil_api.h"
#endif /* VTSS_SW_OPTION_SYSUTIL */

#ifdef VTSS_SW_OPTION_PORT
#include "port_api.h"           /* port_iter_t and friends */
#endif

#if defined(VTSS_SW_OPTION_L2PROTO)
#include "l2proto_api.h"
#endif /* VTSS_SW_OPTION_L2PROTO */

#if defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI)
/* More stack is needed in order to run vCLI commands inside iCLI */
#define CLI_STACK_SIZE (THREAD_DEFAULT_STACK_SIZE * 8)
#else
#define CLI_STACK_SIZE (THREAD_DEFAULT_STACK_SIZE * 5)
#endif /* defined(VTSS_SW_OPTION_ICLI) && defined(VTSS_SW_OPTION_VCLI) */

/*
   Instructions to implement vCLI support for a control module:
   ===========================================================
   1. Command table: Add command table entries by cli_cmd_tab_entry macro.
      Central CLI keeps a single command table, using this macro we can
      add one entry into the table. Each entry in command table is of type
      "struct cli_cmd_t" i.e you need to pass all these parameters, see the
      description of each field in the structure.
      Table entries must mention the command handler function.
      It must pass parameter table address to lookup parameter for the
      command.
      CLI_CMD_FLAG_SYS_CONF should be set if the command action routine
      should be executed for "system config all" comand.
   2. Module specific parameter table: Central CLI keeps a parameter table
      (cli_parm_table) to define generic parameters like "port_list", "vid"
      etc. All the parameters in module comamnd must have a definition either
      in central CLI param table or create a param table inside the module to
      define the parameters. Don't modify the cli_parm_table to add any
      module specific parameter. If you find any parameter which can be a
      candidate for generic parm, then only add into cli_parm_table and the
      corresponding parm parse function.
      Parameter table entry is of type cli_parm_t.
      Following is the guideline to create an entry into param table:
      a. Parameter for a command is identified in module parm table by command
         handler function (cli_cmd_handl_fun).
      b. Parameter table entry should contain the parser function and command
         handler function for the entry. Param table must have last entry filled
         with NULL, this is used by the param lookup function to indicate end
         of the table
      c. if two commands have a common parameter (in terms of help text,
         parser function), you can keep one entry into param table where
         cli_cmd_handl_fun should be NULL.
      d. If you want to have different help text for two identical parameter
         you need to keep separate entries where parm parser func may be same,
         but cmd handler func whould be different to identify parm belongs to
         which command
      e. If a parm has an entry into central CLI parm table, and with different
         help text, but you want to use the same parser function, you can do that
         by creating an entry into module parm table with parse func pointing to
         CLI parm parse function
   3. Command handler function: Each command should have it's action routine.
   4. Parameter parser function: Each parameter in parameter table must have
      a parsing function to parse the parameter. Central CLI defines some generic
      parm parse function to parse generic parm, it can use the same parse func
      and store the parm value in central cli_req_t or it can define it's own parse
      func and save the value either in central cli_req_t or module specific
      req_t structure. Once all the parameters are parsed, cli_req_t struct is
      passed to command handler function
   5. Module specific cli_req_t structure: Module can use central cli_req_t
      structure field to keep the parm while parsing a command param, but
      it may not sufficient to store all the module parms in cli_req_t. If so,
      define a struct called xxx_cli_req_t inside module which will keep the
      module specific parsed parameter, but don't add any new field into cli_req_t
      which is intended to keep only generic parm, not module specific parm.
      Central CLI has to know the size of xxx_cli_req_t struct so that it can
      allocate the memory and pass the address to the module pointing to
      xxx_cli_req_t where module can parse and save the parm value.
      To register the size of this structure, CLI provides one API called
      cli_req_size_register. You should register this size from module init
      function.

    Note: As a reference module to start with you may see the port module cli.
          For detailed information please look into the CLI Design Spec:DS0198

*/

/* CLI command type */
typedef enum {
    CLI_CMD_TYPE_CONF,      /* Configuration command */
    CLI_CMD_TYPE_STATUS,    /* Status command */
    CLI_CMD_TYPE_NONE
} cli_cmd_type_t;

#define CLI_CMD_FLAG_NONE     0x00000000
#define CLI_CMD_FLAG_PHY      0x00000001
#define CLI_CMD_FLAG_SYS_CONF 0x00000002

typedef struct cli_req_t  cli_req_t;
typedef struct cli_parm_t cli_parm_t;

/* Command handler function prototype */
typedef void (*cli_cmd_handl_fun)(cli_req_t *);

/* Request structure default value setting function prototype */
typedef void (*cli_req_def_set_fun)(cli_req_t *);

struct cli_cmd_t {
    char                 *ro_syntax; /* Read-only syntax string */
    /* Read-Write syntax string.
       If the Read-Write syntax is the same as Read-only syntax, only
       fill Read-only syntax and use NULL in this parameter */
    char                 *rw_syntax;
    char                 *descr;     /* Description string */
#define CLI_CMD_SORT_KEY_DEFAULT 100
    ulong                sort_key;   /* cmd table sorting key */
    cli_cmd_type_t       type;       /* Command type */
    /* Privilege level group module ID.
       In most cases, a privilege level group consists of a single module
       (e.g. LACP, RSTP or QoS), but a few of them contains more than one.
       If this module has an independent privilege level group, the value
       is the same as its module ID. If not, the value should be equal the
       reference module ID. For example, HTTPS and SSH belong the security
       privilege level that the value should be equal VTSS_MODULE_ID_SECURITY */
    vtss_module_id_t     module_id;
    cli_cmd_handl_fun    cmd_fun;    /* command handler function */
    cli_req_def_set_fun  def_fun;    /* setting default value in req. struct */
    cli_parm_t           *parm_table; /* parameter table reference */
    ulong                flags;      /* Miscelleneous flags, presently */
    /* used only for conf display */
} CYG_HAL_TABLE_TYPE;

#define CLI_PARM_FLAG_NONE   0x00000000 /* No flags */
#define CLI_PARM_FLAG_NO_TXT 0x00000001 /* Suppress identification text */
#define CLI_PARM_FLAG_SET    0x00000002 /* Set operation parameter */
#define CLI_PARM_FLAG_DUAL   0x00000004 /* Dual parameter */

typedef int (*cli_parm_parse_fun)(char *, char *, char *, char *, cli_req_t *);

/* CLI parameter entry */
struct cli_parm_t {
    char               *txt;  /* Identification text */
    char               *help; /* Help text */
    ulong              flags; /* Miscellaneous flags */
    cli_parm_parse_fun parm_parse_fun; /* Parameter parse function */
    cli_cmd_handl_fun  cmd_fun; /* command handle function used to identify the command */
};

#define CLI_PARM_MAX 256

/* CLI data specification type */
typedef enum {
    CLI_SPEC_NONE = 0, /* Data not specified */
    CLI_SPEC_ANY,      /* Data specified with keyword 'any' */
    CLI_SPEC_VAL       /* Data specified with value */
} cli_spec_t;

#define CLI_INT_VALUES_MAX 10

/* CLI stack state */
typedef struct {
    BOOL        master;                 /* Stack state */
    BOOL        isid[VTSS_USID_END];    /* ISID values for each USID:
                                           VTSS_ISID_END  : USID not active
                                           VTSS_ISID_LOCAL: Slave mode, Local switch
                                           Other          : Master mode, ISID active */
    vtss_isid_t isid_debug;              /* Debug read/write */
    int         count;                   /* Number of active switches */
} cli_stack_state_t;

/* CLI request block */
struct cli_req_t {
    int               parm_parsed; /* Internal use */
    BOOL              set; /* True if SET request */

    cli_stack_state_t stack; /* Global stack state */
    vtss_usid_t       usid_sel; /* "Stack Select" switch ID */
    vtss_usid_t       usid[2];  /* Other USIDs */

    vtss_uport_no_t   uport;     /* 1 - VTSS_PORTS */
    BOOL              uport_list[VTSS_PORT_ARRAY_SIZE + 1]; /* Index 0 may be used for type CLI_PARM_TYPE_PORT_LIST_0. This is the reason why we can't make a corresponding iport_list[] */

    uchar             mac_addr[6];    /* MAC address parameter */
    cli_spec_t        mac_addr_spec;  //***mac or conf

    cli_spec_t        vid_spec;
    vtss_vid_t        vid;

    /* IP setup */
    cli_spec_t        ipv4_addr_spec;
    vtss_ipv4_t       ipv4_addr;
    cli_spec_t        ipv4_mask_spec;
    vtss_ipv4_t       ipv4_mask;
    cli_spec_t        ipv6_addr_spec;
    vtss_ipv6_t       ipv6_addr;
    // Generic IPv4/IPv6 CLI parsing:
    vtss_ip_addr_t    ip_addr;
    cli_spec_t        ip_addr_spec;
#ifdef VTSS_SW_OPTION_QOS
    cli_spec_t        class_spec;
    vtss_prio_t       class_;
#endif /* VTSS_SW_OPTION_QOS */
    cli_spec_t        host_name_spec;
    char              host_name[CLI_PARM_MAX];

    /* Keywords */
    BOOL              clear;
    BOOL              all;
    BOOL              enable;
    BOOL              disable;
    BOOL              port;
    BOOL              smac;
    BOOL              dmac;
    BOOL              aggr_ip;
    BOOL              cpu_port;
#ifdef VTSS_SW_OPTION_ICLI
    BOOL              toggle;
#endif
    ulong             value;
    long              signed_value;
    /* Integer parameters */
    uint              int_value_cnt;
    int               int_values[CLI_INT_VALUES_MAX];

    /* Raw parameter string */
    char              parm[CLI_PARM_MAX];

    /* Memory dump/fill */
    ulong             addr;
    ulong             fill_val;
    ulong             item_cnt;
    ulong             item_size;
    ulong             module_id;
    void              *module_req; /* Module specific req space */
};

#define TRACE_MODULE_ID_UNSPECIFIED -2
#define TRACE_GRP_IDX_UNSPECIFIED   -2
#define THREAD_ID_UNSPECIFIED       -2
#define VTSS_TRACE_LVL_UNSPECIFIED  -2
#define USID_UNSPECIFIED            0xff

/*
 * CLI cmd state layer
 */

#define MAX_CMD_LEN         (1024 + 1)
#define MAX_CMD_HISTORY_LEN 20
#define MAX_WORD_LEN        512
#define MAX_WORD_CNT        128

typedef struct cli_cmdstate {
    char cmd_buf [MAX_CMD_LEN];
    uint cmd_len;
    uint cmd_cursor;
    struct {
        uint idx;
        uint len;
        uint scroll;
        struct {
            uint  cmd_len;
            char cmd [MAX_CMD_LEN];
        } buf [MAX_CMD_HISTORY_LEN];
    } cmd_history;
    char cmd_group[MAX_CMD_LEN];
    vtss_usid_t       usid;  /* Selected USID */
    cli_stack_state_t stack; /* Stack state */
} cli_cmdstate_t;

/*
 * Combined CLI IO & cmd state layer
 */

typedef struct cli_io {
    cli_iolayer_t  io;
    cli_cmdstate_t cmd;
} cli_io_t;

/* Entry definition rule for dynamically created command table */

// This macro allows modules to use #ifdef/#endifs within their commands.
#define CLI_CMD_TAB_ENTRY_DECL(_cf_) \
  struct cli_cmd_t _cli_cmd_tab_##_cf_ CYG_HAL_TABLE_QUALIFIED_ENTRY(cli_cmd_table, _cf_)

// And this macro doesn't allow #ifdef/#endifs within its arguments.
#define cli_cmd_tab_entry(_ro_,_rw_,_dc_,_sk_,_ty_,_mi_,_cf_,_df_,_lf_,_fl_) \
  CLI_CMD_TAB_ENTRY_DECL(_cf_) = {_ro_,_rw_,_dc_,(ulong)_sk_,_ty_,_mi_,_cf_,_df_,_lf_,_fl_}

BOOL cli_cmd_switch_none(cli_req_t *req);
BOOL cli_cmd_slave(cli_req_t *req);
BOOL cli_cmd_conf_slave(cli_req_t *req);
BOOL cli_cmd_slave_do_not_set(cli_req_t *req);
void cli_cmd_usid_print(vtss_usid_t usid, cli_req_t *req, BOOL nl);

#if defined(VTSS_SW_OPTION_L2PROTO)
const char *cli_l2port2uport_str(l2_port_no_t l2port);
#endif /* VTSS_SW_OPTION_L2PROTO */
void cli_cmd_port_numbers(void);
char *cli_iport_list_txt(BOOL iport_list[VTSS_PORT_ARRAY_SIZE], char *buf);
void cli_cmd_stati(char *col1, char *col2, ulong c1, ulong c2);

/* vCLI parser functions */
int cli_parm_parse_list(char *cmd, BOOL *list, ulong min, ulong max, BOOL def);
int cli_parse_wc(char *cmd, cli_spec_t *spec);
int cli_parse_word(char *cmd, char *stx);
int cli_parse_all(char *cmd);
int cli_parse_cpu(char *cmd);
int cli_parse_none(char *cmd);
int cli_parse_disable(char *cmd);
int cli_parse_mac(char *cmd, uchar *mac_addr, cli_spec_t *spec, BOOL wildcard);
#if defined(VTSS_SW_OPTION_IP2)
int cli_parse_ipv4(char *cmd, vtss_ipv4_t *ipv4, vtss_ipv4_t *mask, cli_spec_t *spec, BOOL is_mask);
int cli_parse_ipmcv4_addr(char *cmd, vtss_ipv4_t *ipv4, cli_spec_t *spec);
int cli_parse_ipv6(char *cmd, vtss_ipv6_t *ipv6, cli_spec_t *spec);
int cli_parse_ipmcv6_addr(char *cmd, vtss_ipv6_t *ipv6, cli_spec_t *spec);
/* IPv4/IPv6 parser. If IPv6 is not included in build, it cannot return an IPv6 address. */
int cli_parse_ip(char *cmd, vtss_ip_addr_t *ip, cli_spec_t *spec);
int cli_parm_parse_ipaddr_str(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_ipaddr(char *, char *, char *, char *, cli_req_t *);
#endif /* defined(VTSS_SW_OPTION_IP2) */
char *cli_parse_find(char *cmd, char *stx);
int cli_parse_raw(char *cmd, cli_req_t *req);
int cli_parse_ulong_wc(char *cmd, ulong *req, ulong min, ulong max, cli_spec_t *spec);
int cli_parse_ulong(char *cmd, ulong *req, ulong min, ulong max);
int cli_parse_long(char *cmd, long *req, long min, long max);
longlong cli_parse_longlong(char *cmd, longlong *req, longlong min, longlong max);
ulonglong cli_parse_ulonglong(char *cmd, ulonglong *req, ulonglong min, ulonglong max);
int cli_parse_range(char *cmd, ulong *req_min, ulong *req_max, ulong min, ulong max);
int cli_parse_integer(char *cmd, cli_req_t *req, char *stx);
int cli_parse_quoted_string(const char *cmd, char *result, int len);
int cli_parse_string(const char *cmd, char *admin_string, size_t min, size_t max);
int cli_parse_text(const char *cmd, char *parm, int max);
int cli_parse_text_numbers_only(const char *cmd, char *parm, int max);
int cli_parm_parse_keyword (char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_port_list(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_mac_addr(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_sid(char *, char *, char *, char *, cli_req_t *);
#ifdef VTSS_SW_OPTION_QOS
int cli_parm_parse_class(char *, char *, char *, char *, cli_req_t *);
#endif /* VTSS_SW_OPTION_QOS */
int cli_parm_parse_vid(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_port(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_mem_addr(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_memsize(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_integer(char *, char *, char *, char *, cli_req_t *);
int cli_parm_parse_prompt(char *, char *, char *, char *, cli_req_t *);

/* Register size of module specific vCLI request block */
void cli_req_size_register(cyg_uint32 size);

/* Display complete configuration */
void cli_system_conf_disp(cli_req_t *req);

/* Prepend vCLI prompt with a name */
void cli_name_set(const char *name);


#ifdef VTSS_SW_OPTION_PORT

/*
 * Initialize CLI switch iterator to iterate over all switches in USID order.
 * Use cli_switch_iter_getnext() to filter out non-selected, non-existing switches.
 */
static inline vtss_rc cli_switch_iter_init(switch_iter_t *sit)
{
    return switch_iter_init(sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
}

/*
 * CLI switch iterator. Returns selected and existing switches.
 * Updates sit->first to match first selected switch.
 *
 * sit->last is not updated and therefore unreliable.
 */
static inline BOOL cli_switch_iter_getnext(switch_iter_t *sit, cli_req_t *req)
{
    BOOL first = FALSE;
    while (switch_iter_getnext(sit)) {
        if (sit->first) {
            first = TRUE;
        }
        if (req->stack.isid[sit->usid] != VTSS_ISID_END) {
            sit->first = first;
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Initialize CLI port iterator to iterate over all ports in uport order.
 * Use cli_port_iter_getnext() to filter out non-selected ports.
 */
static inline vtss_rc cli_port_iter_init(port_iter_t *pit, vtss_isid_t isid, port_iter_flags_t flags)
{
    return port_iter_init(pit, NULL, isid, PORT_ITER_SORT_ORDER_UPORT, flags);
}

/*
 * CLI port iterator. Returns selected ports.
 * Updated pit->first to match first selected port.
 *
 * pit->last is not updated and therefore unreliable.
 */
static inline BOOL cli_port_iter_getnext(port_iter_t *pit, cli_req_t *req)
{
    BOOL first = FALSE;
    while (port_iter_getnext(pit)) {
        if (pit->first) {
            first = TRUE;
        }
        if (req->uport_list[pit->uport]) {
            pit->first = first;
            return TRUE;
        }
    }
    return FALSE;
}


#endif /* VTSS_SW_OPTION_PORT */

/****************************************************************************/
/*  vCLI public interface                                                   */
/****************************************************************************/
/* Initialize vCLI module */
void vcli_init(void);

/* Initialize vCLI parser */
void vcli_cmd_parse_init(cli_iolayer_t *pIO);

/* Parse and execute a vCLI command */
vtss_rc vcli_cmd_parse_and_exec(cli_iolayer_t *pIO);

/* Execute a vCLI command in the current thread context */
vtss_rc vcli_cmd_exec(char *cmd);

/* Print vCLI exec banner */
void vcli_banner_exec(cli_iolayer_t *pIO);

#endif /* _VTSS_CLI_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
