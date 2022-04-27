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

#include <time.h>
#include "web_api.h"
#include "msg_api.h"
#include "sysutil_api.h"
#include "port_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */
#include "mgmt_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_shared.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#include "misc_api.h"
#endif

// Set WEB_CACHE_STATIC_FILES to 1 if you want to cache the static files in memory
// This is really not neccessary as browsers will cache the static files locally.
// Static files are currently config.js and filter.css.
#define WEB_CACHE_STATIC_FILES 0
#define WEB_BUF_LEN 1024

#define ARG_NAME_MAXLEN	64

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_WEB

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CNT          1


#include <vtss_trace_api.h>
/* ============== */

#ifndef MIN
#define MIN(_x_, _y_) ((_x_) < (_y_) ? (_x_) : (_y_))
#endif

/* START :Config Js lib table boundaries definition */
/*lint -e{19} ...it's need to declare web_js_lib_table_start, so skip lint error */
CYG_HAL_TABLE_BEGIN(web_js_lib_table_start, web_lib_config_js_table);
/*lint -e{19} ...it's need to declare web_js_lib_table_end, so skip lint error */
CYG_HAL_TABLE_END(web_js_lib_table_end, web_lib_config_js_table);

extern struct lib_config_js_t web_js_lib_table_start[], web_js_lib_table_end[];
/* END :Config Js lib table boundaries definition */

/* START :Filter CSS lib table boundaries definition */
/*lint -e{19} ...it's need to declare web_js_lib_table_start, so skip lint error */
CYG_HAL_TABLE_BEGIN(web_css_lib_table_start, web_lib_filter_css_table);
/*lint -e{19} ...it's need to declare web_js_lib_table_end, so skip lint error */
CYG_HAL_TABLE_END(web_css_lib_table_end, web_lib_filter_css_table);

extern struct lib_filter_css_t web_css_lib_table_start[], web_css_lib_table_end[];
/* END :Filter CSS lib table boundaries definition */

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "web",
    .descr     = "Web server"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
char http_ires_etag[32];
#endif

#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
CYG_HTTPD_AUTH_TABLE_ENTRY(webstax_entry, \
                           "/", VTSS_PRODUCT_NAME, \
                           "admin", "", /* Password set at initialization time */ \
                           CYG_HTTPD_AUTH_BASIC);
#endif /* CYGOPT_NET_ATHTTPD_USE_AUTH */

char var_SelectQceId[HTML_VAR_LEN];
char var_SelectQclId[HTML_VAR_LEN];
char var_bool[6]; /* "true" or "false" */
char var_clear[HTML_VAR_LEN];
char var_clock_inst[HTML_VAR_LEN];  // Clock Instance
char var_Flush[2];
char var_dyn_get_next_entry[2];
char var_dyn_num_of_ent[4];
char var_dyn_start_mac_addr[20];
char var_dyn_start_vid[5];
char var_eps[4];
char var_evc[4];
char var_mep[4];
char var_qceConfigFlag[HTML_VAR_LEN];
char var_server[HTML_VAR_LEN];
char var_syslogEntryNum[HTML_VAR_LEN];
char var_syslogFlag[HTML_VAR_LEN];
char var_syslogLevel[HTML_VAR_LEN];
char var_syslogStartId[HTML_VAR_LEN];
char var_uni[4];
char var_uport[HTML_VAR_LEN];
char var_vid[5];  // VLAN ID


#define XSTR(s) STR(s)
#define STR(s) #s
#define ENTRY(x, v) CYG_HTTPD_FVAR_TABLE_ENTRY(hal_form_entry_##x, XSTR(x), v, sizeof(v))

ENTRY(DynGetNextAddr,            var_dyn_start_mac_addr);
ENTRY(DynNumberOfEntries,        var_dyn_num_of_ent);
ENTRY(DynStartVid,               var_dyn_start_vid);
ENTRY(Flush,                     var_Flush);
ENTRY(GetNextEntry,              var_dyn_get_next_entry);
ENTRY(SelectQceId,               var_SelectQceId);
ENTRY(SelectQclId,               var_SelectQclId);
ENTRY(bool,                      var_bool);
ENTRY(clear,                     var_clear);
ENTRY(clock_inst,                var_clock_inst);
ENTRY(eps,                       var_eps);
ENTRY(evc,                       var_evc);
ENTRY(mep,                       var_mep);
ENTRY(port,                      var_uport);
ENTRY(qceConfigFlag,             var_qceConfigFlag);
ENTRY(server,                    var_server);
ENTRY(syslogEntryNum,            var_syslogEntryNum);
ENTRY(syslogFlag,                var_syslogFlag);
ENTRY(syslogLevel,               var_syslogLevel);
ENTRY(syslogStartId,             var_syslogStartId);
ENTRY(uni,                       var_uni);
ENTRY(vid,                       var_vid);



/****************************************************************************/
/*  HTTPD Variable Table Entries                                            */
/****************************************************************************/


#undef ENTRY

/*
 * Pseudo CLI IO interface layer
 */
/*lint -esym(459,cli_io_mem)*/
struct cli_io_mem {
    cli_iolayer_t base;
    int in_use;
    int process_done;
    time_t timeout;
    int buflen;
    int bufmax;
    int bufleft;
    char mem[2048]; // 2048 is needed for ping6 multicast
};

static
void cli_io_mem_init(cli_iolayer_t *pIO)
{
    struct cli_io_mem *pMIO = (struct cli_io_mem *) pIO;
    pMIO->bufleft = pMIO->bufmax = sizeof(pMIO->mem);
    pMIO->buflen = 0;
}

static
int cli_io_mem_getch(struct cli_iolayer *pIO, int timeout, char *ch)
{
    return CLI_ERROR_IO_TIMEOUT;
}

static
int cli_io_mem_vprintf(cli_iolayer_t *pIO, const char *fmt, va_list ap)
{
    struct cli_io_mem *pMIO = (struct cli_io_mem *) pIO;
    int ct = vsnprintf(pMIO->mem + pMIO->buflen, pMIO->bufleft, fmt, ap);
    pMIO->buflen += ct;
    pMIO->bufleft -= ct;
    return ct;
}

static
void cli_io_mem_putchar(cli_iolayer_t *pIO, char ch)
{
    struct cli_io_mem *pMIO = (struct cli_io_mem *) pIO;
    if(pMIO->bufleft) {
      pMIO->mem[pMIO->buflen++] = ch;
      pMIO->bufleft--;
    }
}

static
void cli_io_mem_puts(cli_iolayer_t *pIO, const char *str)
{
    struct cli_io_mem *pMIO = (struct cli_io_mem *) pIO;
    while(pMIO->bufleft && *str) {
      pMIO->mem[pMIO->buflen++] = *str++;
      pMIO->bufleft--;
    }
}

static
void cli_io_mem_dummy(cli_iolayer_t *pIO)
{
}

static
void cli_io_mem_close(cli_iolayer_t *pIO)
{
    struct cli_io_mem *pMIO = (struct cli_io_mem *) pIO;
    pMIO->process_done = 1;
}

#define WEB_CLI_IO_TIMEOUT      36000   // 6 minutes

static struct cli_io_mem cli_io_mem[WEB_CLI_IO_MAX];
static struct cli_io_mem cli_io_mem_default =
{
    {
        .cli_init = cli_io_mem_init,
        .cli_getch = cli_io_mem_getch,
        .cli_vprintf = cli_io_mem_vprintf,
        .cli_putchar = cli_io_mem_putchar,
        .cli_puts = cli_io_mem_puts,
        .cli_flush = cli_io_mem_dummy,
        .cli_close = cli_io_mem_close,
        .bEcho = FALSE,
        .cDEL = CLI_DEL_KEY_WINDOWS, /* Not used */
        .cBS  = CLI_BS_KEY_WINDOWS,  /* Not used */
    }
};

cli_iolayer_t *
web_get_iolayer(int web_io_type)
{
    int i = web_io_type;

    if (web_io_type == WEB_CLI_IO_TYPE_PING) {
        for (; i < WEB_CLI_IO_MAX; i++) {
            if (!cli_io_mem[i].in_use || (cli_io_mem[i].in_use && time(NULL) - cli_io_mem[i].timeout > WEB_CLI_IO_TIMEOUT)) {
                break;
            }
        }
    }

    if (i < WEB_CLI_IO_MAX) {
        // Found available entry
        cli_io_mem[i].base.cli_init(&cli_io_mem[i].base);
        cli_io_mem[i].in_use = 1;
        cli_io_mem[i].timeout = time(NULL);
        return &cli_io_mem[i].base;
    }

    return NULL;
}

/* When web_io_type is equal to WEB_CLI_IO_TYPE_BOOT_LOADER or WEB_CLI_IO_TYPE_FIRMWARE,
   the parameter of "io" should be NULL.
   When web_io_type is equal to WEB_CLI_IO_TYPE_PING,
   the parameter of "io" should be specific memory address.
  */
void web_send_iolayer_output(int web_io_type, cli_iolayer_t *io, const char *mimetype)
{
    int i = web_io_type;

    if (web_io_type == WEB_CLI_IO_TYPE_PING) {
        for (; i < WEB_CLI_IO_MAX; i++) {
            if (cli_io_mem[i].in_use && io == &cli_io_mem[i].base) {
                break;
            }
        }
    }

    if (i < WEB_CLI_IO_MAX) {
        // Found specific entry
        cyg_httpd_start_chunked(mimetype);
        if (cli_io_mem[i].buflen > 0) {
            cyg_httpd_write_chunked(cli_io_mem[i].mem, cli_io_mem[i].buflen);
        }
        cyg_httpd_end_chunked();
        cli_io_mem[i].bufleft = cli_io_mem[i].bufmax;
        cli_io_mem[i].buflen = 0;

        if (cli_io_mem[i].process_done) {
            cli_io_mem[i].in_use = 0;
            cli_io_mem[i].process_done = 0;
        }
    }
}

/*****************************************************************************/
// cgi_escape()
// cgi_escape() encodes the string that is contained in @from to make it
// portable. A string is considered portable if it can be transmitted across
// any network to any computer that supports ASCII characters.
// To make a string portable, characters other than the following 62 ASCII
// characters must be encoded:
// 0-9, A-Z, a-z
// All other characters are converted to their two digit (%xx) hexadecimal
// equivalent (referred as the character's "hexadecimal escape sequence").
//
// This function is useful when the user can write any text in a field on
// a web page. If he uses the same character as you use as a delimiter when
// sending data from here to the user's browser, the user would experience
// problems.
// Therefore: In web.c you use cgi_escape() on free text fields, whereas
// in <your_web_page>.htm's processUpdate() function you use the JS function
// called unescape(), which performs the reverse operation. As field separator
// you must then use one of those characters that will get converted, e.g. '#'.
//
// The length of @to must be 3 times the length of @from, since in theory
// all characters in @from may have to be converted (the terminating NULL
// character is not encoded).
//
// The function returns the number of bytes excluding the terminating
// NULL byte written to @to.
/*****************************************************************************/
static int _cgi_escape(const char *from, char *to, size_t maxlen)
{
  char ch, *to_ptr = to;
  const char *from_ptr = from;
  int len = 0;

  /* peter, 2007/11,
     The unreserved characters are alphabet, digit, hyphen(-),
     dot(.), underscore(_) and tilde(~). [RFC 3986].
     Note:
     In the real enviorment(IE6 and FF2.0), Asterisk(*) is unreserved character, tilde(~) is not.
     escape !"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~
     =>
     <%20%21%22%23%24%25%26%27%28%29*%2B%2C-.%2F%3A%3B%3C%3D%3E%3F%40%5B%5C%5D%5E_%60%7B%7C%7D%7E> */
  while(maxlen-- && (ch = *from_ptr) != '\0') {
    if((ch >= '0' && ch <= '9') ||
       (ch >= 'A' && ch <= 'Z') ||
       (ch >= 'a' && ch <= 'z') ||
       (ch == '*') ||
       (ch == '-') ||
       (ch == '.') ||
       (ch == '_')) {
      *to_ptr++ = ch;
      len++;
    } else {
      sprintf(to_ptr, "%%%02X", ch);
      to_ptr += 3;
      len += 3;
    }
    from_ptr++;
  }
  *to_ptr = '\0';
  return len;
}

int cgi_escape(const char *from, char *to)
{
    return _cgi_escape(from, to, strlen(from));
}

int cgi_escape_n(const char *from, char *to, size_t maxlen)
{
    return _cgi_escape(from, to, maxlen);
}

/*****************************************************************************/
// a16toi()
// Converts a hex digit to an int.
/*****************************************************************************/
static int a16toi(char ch) {
  if(ch >= '0' && ch <= '9') {
    return ch - '0';
  }

  return toupper(ch) - 'A' + 10;
}

/*****************************************************************************/
// cgi_unescape()
// This function performs the opposite operation of cgi_escape(). Please
// see cgi_escape() for details.
// The only difference is that the browser converts a space to '+' and a '+'
// to "%2B" before it POSTs the data. Therefore, this function must
// convert back '+' to ' '. All other "%xx" sequences are converted normally.
//
// @from     is the string possibly containing escape sequences.
// @to       is the string where the escape sequences are converted into real chars.
// @from_len is the number of chars to take from @from excluding a possible
//           terminating NULL character (@from need not be NULL-terminated).
// @to_len   is the number of characters that there's room for in @to including
//           the terminating NULL character.
// The function returns FALSE if the @from string is invalid (e.g. there aren't
// two hex chars following a percent sign) or if there's not room in the
// @to string for all unescaped chars. TRUE otherwise.
/*****************************************************************************/
BOOL cgi_unescape(const char *from, char *to, uint from_len, uint to_len)
{
  uint from_i = 0, to_i = 0;
  char ch;

  if(to_len == 0) {
      return FALSE;
  }

  while(from_i < from_len) {
    if(from[from_i] == '%') {
      // Check if there are chars enough left in @from to complete the conversion
      if(from_i + 2 >= from_len) {
        // There aren't.
        return FALSE;
      }

      // Check if the two next chars are hexadecimal.
      if(!isxdigit(from[from_i + 1])  || !isxdigit(from[from_i + 2])) {
        return FALSE;
      }

      ch = 16 * a16toi(from[from_i + 1])  + a16toi(from[from_i + 2]);

      from_i += 3;
    } else if(from[from_i] == '+') {
      ch = ' '; // Special handling of '+'. The browser converts spaces to '+' and '+' to "%2B" (tested with FF2.0, IE6.0 and IE7.0).
      from_i++;
    } else {
      ch = from[from_i++];
    }

    to[to_i++] = ch;
    if(to_i == to_len) {
      // Not room for trailing '\0'
      return FALSE;
    }
  }

  to[to_i] = '\0';
  return TRUE;
}

/*****************************************************************************/
// cgi_text_str_to_ascii_str()
//
// Example:  from = "A2#b" -> to = "0x410x320x230x62"
// @from     is the textual string.
// @to       is the string in ascii hex code format.
// @from_len is the number of chars to take from @from including a possible
//           terminating NULL character (@from must be NULL-terminated).
// @to_len   is the number of characters that there's enough room for @to
//           including the terminating NULL character.
// The function returns the number of bytes excluding the terminating
// NULL byte written to @to.
/*****************************************************************************/
int cgi_text_str_to_ascii_str(const char *from, char *to, uint from_len, uint to_len)
{
    uint    idx;
    int     length;

    if (!from || !to ||
        (to_len == 0) ||
        (to_len < ((from_len * 4) + 1))) {
        return -1;
    }

    idx = 0;
    length = 0;
    memset(to, 0x0, to_len);
    /* check from for every chars */
    while (idx < from_len) {
        if (!from[idx]) {
            break;
        }

        if ((((int)to_len - length) > 0) &&
            (((int)to_len - length) / 4)) {
            sprintf(to + length, "0x%02X", from[idx]);
        } else {
            length = 0;
            break;
        }

        idx++;
        length += 4;
    }

    to[length] = '\0';
    return length;
}

/*****************************************************************************/
// cgi_ascii_str_to_text_str()
//
// Example:  from = "0x410x320x230x62" -> to = "A2#b"
// @from     is the string in ascii hex code format.
// @to       is the string where the ascii hex codes converted into real chars.
// @from_len is the number of chars to take from @from excluding a possible
//           terminating NULL character (@from need not be NULL-terminated).
// @to_len   is the number of characters that there's room for in @to including
//           the terminating NULL character.
// The function returns FALSE if the @from or @to string is invalid (e.g. null
// pointer, ...) or if there's not room in the @to string for all chars.
// Otherwise, the function returns TRUE.
/*****************************************************************************/
BOOL cgi_ascii_str_to_text_str(const char *from, char *to, uint from_len, uint to_len)
{
    int     asc;
    uint    from_i, to_i;

    if (!from || !to ||
        (to_len == 0) ||
        (from_len % 4)) {
        return FALSE;
    }

    from_i = to_i = 0;
    memset(to, 0x0, to_len);
    /* check from every four chars */
    while (from_i < from_len) {
        /* Check if the first two chars are reserved keyword "0x" */
        if ((from[from_i] != '0') ||
            (from[from_i + 1] != 'x')) {
            to[0] = '\0';
            return FALSE;
        }

        /* Check if the next two chars are hexadecimal */
        if (!isxdigit(from[from_i + 2])  || !isxdigit(from[from_i + 3])) {
            to[0] = '\0';
            return FALSE;
        }

        asc = a16toi(from[from_i + 2]) * 16 + a16toi(from[from_i + 3]);
        if ((asc < 32) || (asc > 127)) {
            to[0] = '\0';
            return FALSE;
        }

        from_i += 4;
        to[to_i++] = asc;
        if (to_i == to_len) {
            /* Not enough room for trailing '\0' */
            to[0] = '\0';
            return FALSE;
        }
    }

    to[to_i] = '\0';
    return TRUE;
}

/*
 *****************************************************************************
 */


/*
 * String helpers
 */

static inline bool str_noteol(const char *start, const char *end)
{
    return start && start < end && *start && *start != '\r' && *start != '\n';
}

static const char *str_nextline(const char *p, const char *end)
{
    //diag_printf("%s: start\n", __FUNCTION__);
    while(p && p < end && (p = memchr(p, '\r', end - p))) {
        if((end-p) > 1 && p[1] == '\n')
            return p+2;
        p++;                    /* Advance past \r */
    }
    return NULL;
}

/* NB: This implementation has *fixed* search for preceeding \r\n */
static const char *bin_findstr(const char *haystack, const char *haystack_end,
                               const char *needle, size_t nlen)
{
    //diag_printf("%s: start\n", __FUNCTION__);
    while(haystack &&
          haystack < haystack_end &&
          (haystack = str_nextline(haystack, haystack_end))) {
        if(strncmp(haystack, needle, nlen) == 0)
            return haystack;
    }
    return NULL;
}

/*
 * POST formdata extraction
 */

char *
get_formdata_boundary(CYG_HTTPD_STATE* p, int *len)
{
    const char *req = p->inbuffer, *end = p->header_end;
    const char *bheader = "Content-Type: multipart/form-data";
    const char *bkey = "boundary=";

    //diag_printf("%s: start\n", __FUNCTION__);
    while((req = str_nextline(req, end))) {
        if(strncasecmp(req, bheader, strlen(bheader)) == 0) {
            const char *bound, *bend;
            while(str_noteol(req, end)) { /* In this line, find "boundary=" */
                if(strncmp(req, bkey, strlen(bkey)) == 0) {
                    char *bound_copy;
                    int blen;
                    bend = bound = req + strlen(bkey);
                    while(str_noteol(bend, end))
                        bend++;
                    blen = 2 + bend - bound; /* Leading '--' */
                    bound_copy = VTSS_MALLOC(blen+1);
                    if(bound_copy) {
                        strncpy(bound_copy+2, bound, blen-2); /* Actual boundary */
                        bound_copy[0] = bound_copy[1] = '-'; /* Leading '--' */
                        bound_copy[blen] = '\0';
                        *len = blen;
                    }
                    return bound_copy;
                } else
                    req++;
            }
        }
    }

    return NULL;
}

int
cyg_httpd_form_parse_formdata(CYG_HTTPD_STATE* p, form_data_t *formdata, int maxdata)
{
    char *boundz;
    const char *start, *content_end;
    int i, bound_len;

    //diag_printf("%s: start\n", __FUNCTION__);
    if((boundz = get_formdata_boundary(p, &bound_len)) == NULL)
        return 0;
    //diag_printf("%s: boundary %s (%p)\n", __FUNCTION__, boundz, boundz);

    content_end = p->post_data + p->content_len;
    start = strstr(p->post_data, boundz);
    for(i = 0; i < maxdata && start && start < content_end; ) {
        const char *end, *data, *name;

        /* Point at headers */
        start += bound_len;

        name = strstr(start, "name=\"");
        if (!name) {
            // Malformed.
            break;
        }
        name += 6;
        strncpy(formdata[i].name, name, MAX_FORM_NAME_LEN - 1);

        /* Zap off endquote in copied data */
        char *endquote = strchr(formdata[i].name, '"');
        if(endquote)
            *endquote = '\0';

        if((data = strstr(start, CYG_HTTPD_HEADER_END_MARKER))) {
            formdata[i].value = data + strlen(CYG_HTTPD_HEADER_END_MARKER);
            /* This may search through *binary* data */
            if((end = bin_findstr(start, content_end, boundz, bound_len))) {
                formdata[i].value_len = (end - formdata[i].value)  - 2 /* \r\n */;
                i++;                /* Got one */
                //diag_printf("%s: have %d parts\n", __FUNCTION__, i);
            } else
                break;
            start = end; /* Advance past this */
        } else
            break; /* Malformed */
    }

    //diag_printf("%s: end %s\n", __FUNCTION__, boundz);
    VTSS_FREE(boundz);               /* VTSS_MALLOC'ed asciiz */

    return i;
}

static const char *
search_arg(const char *arglist, const char *name)
{
    int idlen = strlen(name);
    const char *start, *ptr;

    start = arglist;
    while(*arglist && (ptr = strstr(arglist, name))) {
        if(ptr[idlen] == '=' &&
           (ptr == start ||     /* ^name= match OR*/
            ptr[-1] == '&'))    /* &name= match */
            return ptr + idlen + 1; /* Match, skip past "name=" */
        /* False match, advance past match */
        arglist = ptr + idlen;
    }
    return NULL;
}

/*
 * POST variable extraction
 */

const char *
cyg_httpd_form_varable_find(CYG_HTTPD_STATE* p, const char *name)
{
    const char *value;

    /* Search POST formdata args */
    if(httpstate.content_type == CYG_HTTPD_CONTENT_TYPE_URLENCODED &&
       p->post_data &&
       (value = search_arg(p->post_data, name)))
        return value;

    /* Search URL args (GET *and* POST) - xyz?bla=1&z=45 */
    return search_arg(p->args, name);
}

const char *
cyg_httpd_form_varable_string(CYG_HTTPD_STATE* p, const char *name, size_t *pLen)
{
    int datalen = 0;
    const char *value = cyg_httpd_form_varable_find(p, name);
    if(value) {                 /* Match */
        while(value[datalen] && value[datalen] != '&') {
            datalen++;
        }
    }
    if(pLen)
        *pLen = datalen;
    return value;

}

cyg_bool
cyg_httpd_form_varable_int(CYG_HTTPD_STATE* p, const char *name, int *pVal)
{
    size_t len;
    const char *value;
    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        // If the user has added a '+' before the number in the input edit field,
        // it will get converted to "%2B" before it reaches this file.
        // In that case we skip the initial "%2B" before calling atoi()
        if(value[0] == '%' && value[1] == '2' && toupper(value[2]) == 'B') {
            if(len > 3) {
                *pVal = atoi(&value[3]);
            } else {
                return FALSE;
            }
        } else{
            *pVal = atoi(value);
        }
        return TRUE;
    }
    return FALSE;
}

#if defined(VTSS_FEATURE_ACL_V2)
/* The parameter of 'idx' start from 1, it means the first matched.
   When the value is 2, it means the seconds matched and so on. */
const char *
cyg_httpd_form_multi_varable_find(CYG_HTTPD_STATE* p, const char *name, int idx)
{
    const char  *value;
    int         i, offset;

    /* Search POST formdata args */
    if (httpstate.content_type == CYG_HTTPD_CONTENT_TYPE_URLENCODED &&
        p->post_data &&
        (value = search_arg(p->post_data, name))) {
        if (idx == 1) {
            return value;
        } else {
            for (i = 2, offset = strlen(p->post_data) - strlen(value); i <= idx; i++, offset = strlen(p->post_data) - strlen(value)) {
                value = search_arg(p->post_data + offset, name);
                if (i == idx) {
                    return value;
                }
            }
        }
    }

    return NULL;
}

const char *
cyg_httpd_form_multi_varable_string(CYG_HTTPD_STATE* p, const char *name, size_t *pLen, int idx)
{
    int datalen = 0;
    const char *value = cyg_httpd_form_multi_varable_find(p, name, idx);
    if(value) {                 /* Match */
        while(value[datalen] && value[datalen] != '&') {
            datalen++;
        }
    }
    if(pLen)
        *pLen = datalen;
    return value;
}

cyg_bool
cyg_httpd_form_multi_varable_int(CYG_HTTPD_STATE* p, const char *name, int *pVal, int idx)
{
    size_t len;
    const char *value;

    if((value = cyg_httpd_form_multi_varable_string(p, name, &len, idx)) && len > 0) {
        // If the user has added a '+' before the number in the input edit field,
        // it will get converted to "%2B" before it reaches this file.
        // In that case we skip the initial "%2B" before calling atoi()
        if(value[0] == '%' && value[1] == '2' && toupper(value[2]) == 'B') {
            if(len > 3) {
                *pVal = atoi(&value[3]);
            } else {
                return FALSE;
            }
        } else{
            *pVal = atoi(value);
        }
        return TRUE;
    }
    return FALSE;
}
#endif /* VTSS_FEATURE_ACL_V2 */

cyg_bool
cyg_httpd_form_varable_uint64(CYG_HTTPD_STATE* p, const char *name, u64 *pVal)
{
    size_t len;
    const char *value;
    char *endptr;
    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        // If the user has added a '+' before the number in the input edit field,
        // it will get converted to "%2B" before it reaches this file.
        // In that case we skip the initial "%2B" before calling atoi()
        if(value[0] == '%' && value[1] == '2' && toupper(value[2]) == 'B') {
            if(len > 3) {
                *pVal = strtoull(&value[3],&endptr,10);
            } else {
                return FALSE;
            }
        } else{
            *pVal =strtoull(value,&endptr,10);
        }
        return TRUE;
    }
    return FALSE;
}


// Function that returns the value from a web form containing a integer. It checks
// if the value is within an allowed range given by min_value and max_value (both
// values included) . If the value isn't within the allowed ranged an error message
// is thrown, and the minimum value is returned.
int httpd_form_get_value_int(CYG_HTTPD_STATE* p, const char form_name[255],int min_value,int max_value)
{
    int form_value;
    if(cyg_httpd_form_varable_int(p, form_name, &form_value)) {
      if (form_value < min_value || form_value > max_value) {
          T_E("Invalid value. Form name = %s, form value = %u, min_value = %d, max_value = %d ", form_name, form_value, min_value, max_value);
          form_value =  min_value;
      }
    } else {
        T_E("Unknown form. Form name = %s, form_value = %d", form_name, form_value);
        form_value =  min_value;
    }

    return form_value;
}



cyg_bool
_cyg_httpd_form_varable_long_int(CYG_HTTPD_STATE* p, const char *name, long *pVal)
{
    size_t len;
    const char *value;

    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        // If the user has added a '+' before the number in the input edit field,
        // it will get converted to "%2B" before it reaches this file.
        // In that case we skip the initial "%2B" before calling atol()
        if(value[0] == '%' && value[1] == '2' && toupper(value[2]) == 'B') {
            if(len > 3) {
                *pVal = atol(&value[3]);
            } else {
                return FALSE;
            }
        } else {
            *pVal = atol(value);
        }
        return TRUE;
    }
    return FALSE;
}


cyg_bool cyg_httpd_is_hex(char c)
{
   return (((c >= '0') && (c <= '9')) ||
            ((c >='A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}


cyg_bool
cyg_httpd_form_variable_mac(CYG_HTTPD_STATE* p, const char *name, vtss_mac_t *mac)
{
    size_t      len;
    const char  *value;
    uint        mac_addr[6];
    uint        i;
    char        str[20];

    value = cyg_httpd_form_varable_string(p, name, &len);
    if ( value && len > 0 ) {
        if ( cgi_unescape(value, str, len, 20) == FALSE ) {
            return FALSE;
        }

        if ( (sscanf(str, "%2x-%2x-%2x-%2x-%2x-%2x",  &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) ||
             (sscanf(str, "%2x.%2x.%2x.%2x.%2x.%2x",  &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) ||
             (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x",  &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) ||
             (sscanf(str, "%02x%02x%02x%02x%02x%02x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6) ) {
            for (i= 0 ; i < sizeof(mac->addr); i++) {
                mac->addr[i] = (uchar) mac_addr[i];
            }
            return TRUE;
        }
    }
    return FALSE;
}

cyg_bool
cyg_httpd_form_variable_int_fmt(CYG_HTTPD_STATE* p, int *pVal, const char *fmt, ...)
{
    char instance_name[ARG_NAME_MAXLEN];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return cyg_httpd_form_varable_int(p, instance_name, pVal);
}

cyg_bool
cyg_httpd_form_variable_long_int_fmt(CYG_HTTPD_STATE* p, ulong *pVal, const char *fmt, ...)
{
    char instance_name[ARG_NAME_MAXLEN];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return cyg_httpd_form_varable_long_int(p, instance_name, pVal);
}

cyg_bool
cyg_httpd_form_variable_check_fmt(CYG_HTTPD_STATE* p, const char *fmt, ...)
{
    char instance_name[ARG_NAME_MAXLEN];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return cyg_httpd_form_varable_find(p, instance_name) ?
        TRUE : FALSE;           /* "on" if checked - else not found */
}

const char *
cyg_httpd_form_variable_str_fmt(CYG_HTTPD_STATE* p, size_t *pLen, const char *fmt, ...)
{
    char instance_name[ARG_NAME_MAXLEN];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return cyg_httpd_form_varable_string(p, instance_name, pLen);
}

cyg_bool
cyg_httpd_str_to_hex(const char *str_p, ulong *hex_value_p)
{
    char token[20];
    int i=0, j=0;
    ulong k=0, temp=0;

    while (str_p[i]!='\0') {
        token[j++]=str_p[i++];
    }
    token[j]='\0';

    if (strlen(token)>8)
        return FALSE;

    i=0;
    while (token[i]!='\0') {
        if (!((token[i]>= '0' && token[i]<= '9') || (token[i]>= 'A' && token[i]<= 'F') || (token[i]>= 'a' && token[i]<= 'f'))) {
            return FALSE;
        }
        i++;
    }

    temp=0;
    for (i=0; i<j; i++) {
        if (token[i]>='0' && token[i]<='9') {
            k=token[i]-'0';
        }   else if (token[i]>='A' && token[i]<='F') {
            k=token[i]-'A'+10;
        } else if (token[i]>='a' && token[i]<='f') {
            k=token[i]-'a'+10;
        }
        temp=16*temp+ k;
    }

    *hex_value_p=temp;

    return TRUE;
}

cyg_bool
cyg_httpd_form_varable_hex(CYG_HTTPD_STATE* p, const char *name, ulong *pVal)
{
    size_t len;
    const char *value;
    char buf[12];
    unsigned int i;
    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        for (i=0; i<len; i++) {
            buf[i] = value[i];
        }
        buf[i] = '\0';

        return (cyg_httpd_str_to_hex(buf, pVal));
    }
    return FALSE;
}

/* Input format is "xx-xx-xx-xx-xx-xx" or "xx.xx.xx.xx.xx.xx" or "xxxxxxxxxxxx" (x is a hexadecimal digit). */
cyg_bool
cyg_httpd_form_varable_mac(CYG_HTTPD_STATE* p, const char *name, uchar pVal[6])
{
    size_t len;
    const char *value;
    ulong i, j, k=0, m;
    char buf[4], var_value[20], sign = '-';

    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        for (i=0; i<len; i++) {
            var_value[i] = value[i];
            if (var_value[i] == '.') {
                sign = '.';
            }
        }
        var_value[i] = '\0';

        for (i=0, j=0, k=0; i<len; i++) {
            if (var_value[i] == sign || j==2) {
                buf[j]='\0';
                j = 0;
                if (!cyg_httpd_str_to_hex(buf, &m)) {
                    return FALSE;
                }
                pVal[k++]=(uchar)m;
                if (var_value[i] != sign || j==2) {
                    buf[j++]=var_value[i];
                }
            } else {
                buf[j++]=var_value[i];
            }
        }
        buf[j]='\0';
        if (!cyg_httpd_str_to_hex(buf, &m)) {
            return FALSE;
        }
        pVal[k]=(uchar)m;
        return TRUE;
    }
    return FALSE;
}

cyg_bool
cyg_httpd_form_varable_ipv4(CYG_HTTPD_STATE* p, const char *name, vtss_ipv4_t *pVal)
{
    size_t len;
    const char *value;
    uint  ip1, ip2, ip3, ip4, num;

    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
            num = sscanf(value, "%u.%u.%u.%u", &ip1, &ip2, &ip3, &ip4);
            if (num >= 4 && ip1 != 0 && ip1 < 256 && ip2 < 256 && ip3 < 256 && ip4 < 256) {
                *pVal = ((ip1<<24) + (ip2<<16) + (ip3<<8) + ip4);
                return TRUE;
            } else if (ip1 == 0 && ip2 == 0 && ip3 == 0 && ip4 == 0) {
                *pVal = 0;
                return TRUE;
            }
    }
    return FALSE;
}

cyg_bool
cyg_httpd_form_varable_ipv6(CYG_HTTPD_STATE* p, const char *name, vtss_ipv6_t *pVal)
{
    const char  *buf;
    size_t      len;
    char        ip_buf[IPV6_ADDR_IBUF_MAX_LEN];

    if (pVal && ((buf = cyg_httpd_form_varable_string(p, name, &len)) != NULL)) {
        memset(&ip_buf[0], 0x0, sizeof(ip_buf));
        if (len > 0 && cgi_unescape(buf, ip_buf, len, sizeof(ip_buf))) {
            return (mgmt_txt2ipv6(ip_buf, pVal) == VTSS_OK);
        }

    }

    return FALSE;
}

char *
cyg_httpd_form_varable_strdup(CYG_HTTPD_STATE* p, const char *fmt, ...)
{
    size_t len;
    const char *value;
    char instance_name[ARG_NAME_MAXLEN];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    if((value = cyg_httpd_form_varable_string(p, instance_name, &len)) && len > 0) {
        char *dup = VTSS_MALLOC(len+1);
        if(dup) {
            (void) cgi_unescape(value, dup, len, len+1);
            dup[len] = '\0';
            return dup;
        }
    }
    return NULL;
}

/* Input format is "xx-xx-xx" or "xxxxxx" (x is a hexadecimal digit). */
cyg_bool
cyg_httpd_form_varable_oui(CYG_HTTPD_STATE* p, const char *name, uchar pVal[3])
{
    size_t len;
    const char *value;
    ulong i, j, k=0, m;
    char buf[4], var_value[20];

    if((value = cyg_httpd_form_varable_string(p, name, &len)) && len > 0) {
        for (i=0; i<len; i++) {
            var_value[i] = value[i];
        }
        var_value[i] = '\0';

        for (i=0, j=0, k=0; i<len; i++) {
            if (var_value[i] == '-' || j==2) {
                buf[j]='\0';
                j = 0;
                if (!cyg_httpd_str_to_hex(buf, &m)) {
                    return FALSE;
                }
                pVal[k++]=(uchar)m;
                if (var_value[i] != '-' || j==2) {
                    buf[j++]=var_value[i];
                }
            } else {
                buf[j++]=var_value[i];
            }
        }
        buf[j]='\0';
        if (!cyg_httpd_str_to_hex(buf, &m)) {
            return FALSE;
        }
        pVal[k]=(uchar)m;
        return TRUE;
    }
    return FALSE;
}

vtss_isid_t web_retrieve_request_sid(CYG_HTTPD_STATE* p)
{
#if VTSS_SWITCH_STACKABLE
    int sid;
    if(vtss_stacking_enabled()) {
        /* Pull USID from request */
        if(cyg_httpd_form_varable_int(p, "sid", &sid) && VTSS_USID_LEGAL(sid))
            sid = topo_usid2isid(sid); /* Map USID => ISID */
        else
            sid = VTSS_ISID_LOCAL;
    }  else {
        /* Yes, cumbersome - but what we have... */
        for(sid = VTSS_ISID_START; sid < VTSS_ISID_END; sid++)
            if(msg_switch_is_local(sid))
                break;
    }
    T_N("Got isid = %d", sid);
    return (vtss_isid_t) sid;
#else
    return 1;
#endif /* VTSS_SWITCH_STACKABLE */
}

/*
 * ************************** Redirection ***************************
 */

void
redirect(CYG_HTTPD_STATE* p, const char *to)
{
    strncpy(p->url, to, sizeof(p->url)-1);
    cyg_httpd_send_error(CYG_HTTPD_STATUS_MOVED_TEMPORARILY);
}

static void
redirect_get_or_post(CYG_HTTPD_STATE* p, const char *where)
{
    if(p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, where);
    } else {
        int ct;
        (void)cyg_httpd_start_chunked("html");
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "Error: %s", where);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        cyg_httpd_end_chunked();
    }
}

static BOOL
redirectUnmanaged(CYG_HTTPD_STATE* p, vtss_isid_t isid)
{
    return FALSE;
}

static BOOL
redirectInvalid(CYG_HTTPD_STATE* p, vtss_isid_t isid)
{
    if(vtss_switch_stackable()) {
        const char *where = STACK_ERR_URL;
        if(!VTSS_ISID_LEGAL(isid)) {
            T_D("Invalid, Redirect %d to %s!", isid, where);
            redirect_get_or_post(p, where);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL
redirectNonexisting(CYG_HTTPD_STATE* p, vtss_isid_t isid)
{
    if(vtss_switch_stackable()) {
        const char *where = STACK_ERR_URL;
        if(!msg_switch_exists(isid)) {
            T_D("Nonexistent, Redirect %d to %s!", isid, where);
            redirect_get_or_post(p, where);
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
redirectUnmanagedOrInvalid(CYG_HTTPD_STATE* p, vtss_isid_t isid)
{
    if(vtss_switch_stackable()) {
        if(redirectInvalid(p, isid) ||
           redirectUnmanaged(p, isid) ||
           redirectNonexisting(p, isid))
            return TRUE;
    }
    return FALSE;
}

void
send_custom_error(CYG_HTTPD_STATE *p,
                  const char      *title,
                  const char      *errtxt,
                  size_t           errtxt_len)
{
    int ct;
    (void)cyg_httpd_start_chunked("html");
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                  "<html>"
                  "<head><title>%s</title>"
                  "<link href=\"lib/normal.css\" rel=\"stylesheet\" type=\"text/css\">"
                  "</head>"
                  "<body><h1>%s</h1><pre class=\"alert\">", title, title);
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    (void)cyg_httpd_write_chunked(errtxt, errtxt_len);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                  "</pre></body></html>");
    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
    cyg_httpd_end_chunked();
}


// Function for getting the method.
//
// Return: Returns the method or in case that something is wrong -1
int web_get_method(CYG_HTTPD_STATE *p, int module_id) {
    vtss_isid_t    sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    T_D ("SID =  %d", sid );

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        T_E("Invalid isid - redirecting - ISID =  %d", sid );
        return -1;
    }


#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, module_id)) {
        return -1;
    }
#endif

    if (!VTSS_ISID_LEGAL(sid)) {
        return -1;
    }

    return p->method;
}



/*
 * ************************* Semi-static Handlers Start *************************
 */

cyg_int32 handler_navbar_update(CYG_HTTPD_STATE* p)
{
    int ct;

    cyg_httpd_start_chunked("html");
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                  "%s",
                  "function NavBarUpdate()\n"
                  "{\n");
    cyg_httpd_write_chunked(p->outbuffer, ct);


#if defined VTSS_SW_OPTION_POE
    if(!(vtss_board_features() & VTSS_BOARD_FEATURE_POE)) {
        /* Hide POE pages if we are not POE-capable */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "%s",
                      " $('poe_config.htm').getParent('li').setStyle('display', 'none');\n"
#if defined VTSS_SW_OPTION_LLDP
                      " $('lldp_poe_neighbors.htm').getParent('li').setStyle('display', 'none');\n"
#endif  /* VTSS_SW_OPTION_LLDP */
                      " $('poe_status.htm').getParent('li').setStyle('display', 'none');\n");
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }
#endif

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                  "%s",
                  "}\n");
    cyg_httpd_write_chunked(p->outbuffer, ct);

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_navbar_update, "/lib/navbarupdate.js", handler_navbar_update);

/*
 * This particular handler can be OVERRIDDEN in web_custom.c
 */
cyg_int32 handler_lib_config_js(CYG_HTTPD_STATE* p) __attribute__ ((weak, alias("__handler_lib_config_js")));

#define JSCONFIG_SIZE (2*1024)
cyg_int32 __handler_lib_config_js(CYG_HTTPD_STATE* p)
{
    T_R("handler_lib_config_js enter");
    static char   *file_buffer = NULL;
    static size_t  file_alloc_size = JSCONFIG_SIZE;
    static size_t  file_size = 0;

    if (file_buffer == NULL) { // First time or cache disabled
        char buff[WEB_BUF_LEN];
        char *file_offset;
        file_buffer = VTSS_MALLOC(file_alloc_size);
        VTSS_ASSERT(file_buffer != NULL);
        T_D("Alocated %zu bytes for file", file_alloc_size);
        file_offset = file_buffer;
        file_size = 0;

        (void) snprintf(buff, WEB_BUF_LEN,
#ifdef VTSS_ARCH_LUTON28
                        "var configArchLuton28  = 1;\n"
#else
                        "var configArchLuton28  = 0;\n"
#endif
#ifdef VTSS_ARCH_JAGUAR_1
                        "var configArchJaguar_1 = 1;\n"
#else
                        "var configArchJaguar_1 = 0;\n"
#endif
#ifdef VTSS_ARCH_LUTON26
                        "var configArchLuton26  = 1;\n"
#else
                        "var configArchLuton26  = 0;\n"
#endif
#ifdef VTSS_ARCH_SERVAL
                        "var configArchServal   = 1;\n"
#else
                        "var configArchServal   = 0;\n"
#endif
#ifdef VTSS_SW_OPTION_BUILD_SMB
                        "var configBuildSMB     = 1;\n"
#else
                        "var configBuildSMB     = 0;\n"
#endif
#ifdef VTSS_SW_OPTION_BUILD_CE
                        "var configBuildCE      = 1;\n"
#else
                        "var configBuildCE      = 0;\n"
#endif
#ifdef VTSS_SW_OPTION_LLDP_MED
                        "var configLldpmedPoliciesMin = %u;\n"
                        "var configLldpmedPoliciesMax = %u;\n"
#endif
                        "var configPortMin = %u;\n"
                        "var configNormalPortMax = %u;\n"
                        "var configRgmiiWifi = 0;\n"
                        "var configPortType = 0;\n"
                        "var configStackable = %d;\n"
                        "var configSidMin = %d;\n"
                        "var configSidMax = %d;\n"
                        "var configAuthServerCnt = %d;\n"
                        "var configSwitchName = \"%s\";\n"
                        "var configSwitchDescription = \"%s\";\n\n"
                        "var configSoftwareId = %d;\n\n"
                        "var configHostNameLengthMax = %d;\n\n"
                        "function configPortName(portno, long) {\n"
                        " var portname = String(portno);\n"
                        " if(long) portname = \"Port \" + portname;\n"
                        " return portname;\n"
                        "}\n\n"
                        ,
#ifdef VTSS_SW_OPTION_LLDP_MED
                        LLDPMED_POLICY_MIN,
                        LLDPMED_POLICY_MAX,
#endif
                        iport2uport(VTSS_PORT_NO_START),
                        vtss_stacking_enabled() ? VTSS_PORTS : port_count_max(),
#if VTSS_SWITCH_STACKABLE
                        TRUE,
#else
                        FALSE,
#endif
                        VTSS_USID_START,
                        VTSS_USID_END - 1,
                        0,
                        VTSS_PRODUCT_NAME,
                        VTSS_PRODUCT_DESC,
                        VTSS_SW_ID,
                        VTSS_SYS_HOSTNAME_LEN - 1);
        file_size += webCommonBufferHandler(&file_buffer, &file_offset, &file_alloc_size, buff);

        /* JS Collector: Collects the module variables for the modules that are included in the build */
        {
            int i;
            for (i = 0; i < (web_js_lib_table_end - web_js_lib_table_start); i++) {
                file_size += web_js_lib_table_start[i].module_fun(&file_buffer, &file_offset, &file_alloc_size);
            }
        }
        T_D("Using generated config.js file (size = %zu, alloc = %zu)", file_size, file_alloc_size);
        file_alloc_size = file_size + 1; // Optimize allocation size if cache is disabled
    }
    else {
        T_D("Using cached config.js file (size = %zu, alloc = %zu)", file_size, file_alloc_size);
    }

    /* Make this look like a cache-able ires asset */
    {
        cyg_httpd_ires_table_entry entry;
        entry.f_pname = "/lib/config.js";
        entry.f_ptr = (unsigned char *)file_buffer;
        entry.f_size = file_size;
        cyg_httpd_send_ires(&entry);
    }

#if WEB_CACHE_STATIC_FILES == 0
    VTSS_FREE(file_buffer);
    file_buffer = NULL;
#endif
    T_R("handler_lib_config_js exit");
    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_lib_config_js, "/lib/config.js", handler_lib_config_js);

cyg_int32 handler_config_filter_css(CYG_HTTPD_STATE* p) __attribute__ ((weak, alias("__handler_config_filter_css")));

#define CSSFILTER_SIZE (3*1024)
cyg_int32 __handler_config_filter_css(CYG_HTTPD_STATE* p)
{
    T_R("handler_config_filter_css enter");

    static char   *file_buffer = NULL;
    static size_t  file_alloc_size = CSSFILTER_SIZE;
    static size_t  file_size = 0;


    if (file_buffer == NULL) { // First time or cache disabled
        char buff[WEB_BUF_LEN];
        char *file_offset;
        file_buffer = VTSS_MALLOC(file_alloc_size);
        VTSS_ASSERT(file_buffer != NULL);
        T_D("Alocated %zu bytes for file", file_alloc_size);
        file_offset = file_buffer;
        file_size = 0;

        (void) snprintf(buff, WEB_BUF_LEN, vtss_stacking_enabled() ? ".Standalone_only {display: none;}\r\n" : ".SPOM_only {display: none;}\r\n");
        file_size += webCommonBufferHandler(&file_buffer, &file_offset, &file_alloc_size, buff);


        (void) snprintf(buff, WEB_BUF_LEN, ".hasWarmStart { display: none; }\r\n");
        file_size += webCommonBufferHandler(&file_buffer, &file_offset, &file_alloc_size, buff);


        /* CSS Generation: Generates for all of the modules those are not included in the build.
           This will be in the form of  ".has_xxx { display: none; }" here xxx is the module name.
           All of them may not be used at this point of time, this is mainly to achieve uniform notation.
           NOTE :If a new module is added to the webstax, this will automatically generates you the class string and
           the same name should be used in the respective html files */
        {
            char disabled_modules[] = XSTR(DISABLED_MODULES); // This will allocate a modifiable copy on the stack, which is needed by strtok_r()
            char *saveptr; // Local strtok_r() context
            char *ch = strtok_r(disabled_modules, " ", &saveptr);
            while (ch) {
                (void) snprintf(buff, WEB_BUF_LEN, ".has_%s { display: none; }\r\n", ch);
                file_size += webCommonBufferHandler(&file_buffer, &file_offset, &file_alloc_size, buff);
                ch = strtok_r(NULL, " ", &saveptr);
            }
        }

        /* CSS Collector: Collects the CSS entries for the modules that are included in the build */
        {
            int i;
            for (i = 0; i < (web_css_lib_table_end - web_css_lib_table_start); i++) {
                file_size += web_css_lib_table_start[i].module_fun(&file_buffer, &file_offset, &file_alloc_size);
            }
        }
        T_D("Using generated filter.css file (size = %zu, alloc = %zu)", file_size, file_alloc_size);
        file_alloc_size = file_size + 1; // Optimize allocation size if cache is disabled
    }
    else {
        T_D("Using cached filter.css file (size = %zu, alloc = %zu)", file_size, file_alloc_size);
    }

    /* Make this look like a cache-able ires asset */
    {
        cyg_httpd_ires_table_entry entry;
        entry.f_pname = "/config/filter.css";
        entry.f_ptr = (unsigned char *)file_buffer;
        entry.f_size = file_size;
        cyg_httpd_send_ires(&entry);
    }

#if WEB_CACHE_STATIC_FILES == 0
    VTSS_FREE(file_buffer);
    file_buffer = NULL;
#endif
    T_R("handler_config_filter_css exit");
    return -1; // Do not further search the file system.
}

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_filter_css, "/config/filter.css", handler_config_filter_css);

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
static ssize_t web_warn_silent_upgrade(char *buffer, int maxlen)
{
#define SILENT_UPGRADE_NOTICE \
        "System config has just converted to a new format.\n" \
        "Please backup the configuration if needed."

    ssize_t cnt;

    /*lint -esym(459, port_web_notice_callback) */
    port_web_set_notice_callback(NULL);

    cnt = snprintf(buffer, maxlen, "%s", SILENT_UPGRADE_NOTICE);
    buffer[maxlen - 1] = '\0';
    
    return cnt;
}
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

/* LIB JS GENERIC FUNCTION */
size_t webCommonBufferHandler(char **base_ptr, char **cur_ptr, size_t *length, const char *buff)
{
    size_t used  = 0; // used buffer length.
    size_t avail = 0; // available buffer length.
    size_t ct    = 0;

    if ((*base_ptr == NULL) || (*cur_ptr == NULL) || (length == NULL) || (buff == NULL)) {
        return 0;               /* Argument error - nothing copied */
    }

    used  = *cur_ptr - *base_ptr; // calculating used buffer length
    avail = *length - used;       // calculating available buffer length
    ct = strlen(buff);

    if (ct >= avail) {
        size_t new_len = used + ct + 1; // new buffer length.
        char *new_ptr = VTSS_REALLOC(*base_ptr, new_len);
        VTSS_ASSERT(new_ptr != NULL); // VTSS_REALLOC() failed.
        T_D("VTSS_REALLOC(): (avail %zu, need %zu, bp %p, np %p, oz %zu, nz %zu", avail, ct, *base_ptr, new_ptr, *length, new_len);
        *base_ptr = new_ptr;          // update base_ptr to the (possible) new memory location
        *cur_ptr  = *base_ptr + used; // update cur_ptr to the (possibly) new memory location.
        *length   = new_len;
    }

    strcpy(*cur_ptr, buff);
    *cur_ptr += ct;
    return ct;
}

/* Initialize module */
vtss_rc web_init(vtss_init_data_t *data)
{
    int i;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
#ifdef CYGOPT_NET_ATHTTPD_USE_IRES_ETAG
    {
        cyg_uint32 crc;
        const char *v;
        v = misc_software_version_txt();
        crc = cyg_crc32((void*)v, strlen(v));
        v = misc_software_date_txt();
        crc = cyg_crc32_accumulate(crc, (void*)v, strlen(v));
        v = vtss_board_name();
        crc = cyg_crc32_accumulate(crc, (void*)v, strlen(v));
        (void)snprintf(http_ires_etag, sizeof(http_ires_etag), "\"%x\"", crc);
    }
#endif
        memset(cli_io_mem, 0, sizeof(cli_io_mem));
        for (i = 0; i < WEB_CLI_IO_MAX; i++) {
            memcpy(&cli_io_mem[i], &cli_io_mem_default, sizeof(struct cli_io_mem));
            cli_io_mem[i].base.cli_init(&cli_io_mem[i].base);
        }
        break;
    case INIT_CMD_START:
    {
#ifdef CYGOPT_NET_ATHTTPD_USE_AUTH
#ifdef VTSS_SW_OPTION_USERS
        static char sys_passwd[VTSS_SYS_PASSWD_LEN];
        memset(sys_passwd, 0, sizeof(sys_passwd));
        webstax_entry.auth_password = sys_passwd;
#else
        webstax_entry.auth_password = system_get_passwd();
#endif /* VTSS_SW_OPTION_USERS */
#endif /* CYGOPT_NET_ATHTTPD_USE_AUTH */

#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
        if (vtss_icfg_silent_upgrade_invoked())
            port_web_set_notice_callback(web_warn_silent_upgrade);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        cyg_httpd_start();
        break;
    }
    case INIT_CMD_CONF_DEF:
        break;
    default:
        break;
    }

    T_D("exit");
    return 0;
}

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
