/*
 Vitesse Switch Application software.

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

/*
==============================================================================

    1. merge *.htm into cmd_ref.htm
    Run on linux and Windows

    Revision history
    > CP.Wang, 2012/04/07 17:06
        - create

==============================================================================
*/

/*
==============================================================================

    Include File

==============================================================================
*/
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "icli_tool_platform.h"
#include "vtss_icli.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#define __T_E(...) \
    printf("Error: "); \
    printf(__VA_ARGS__);

#define ___USAGE_PRINT() \
    printf("Usage : icli_cmd_htm <path> <list of *.htm>\n");\
    printf("    <path> : path of cmd_ref.htm and where <list of *.htm> are.\n");\

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct __tag_element {
    i32                     line;
    char                    *value_str;
    // next element
    struct __tag_element    *next;
} _tag_element_t;

typedef struct __htm_element {
    _tag_element_t          *begin;
    _tag_element_t          *command;
    _tag_element_t          *end;
    struct __htm_element    *prev;
    struct __htm_element    *next;
} _htm_element_t;

/*
==============================================================================

    Static Variable

==============================================================================
*/
static char             g_value_str[ICLI_STR_MAX_LEN + 1];

static i32              g_line_number;
static char             g_line_buf[ICLI_STR_MAX_LEN + 1];

static _htm_element_t   *g_htm_reflink = NULL;
static _htm_element_t   *g_htm_command = NULL;
static int              g_htm_cmd_cnt = 0;

static _htm_element_t   *g_read_reflink = NULL;
static _htm_element_t   *g_read_command = NULL;
static int              g_read_cmd_cnt = 0;

static char             *g_generate_path = NULL;
static char             *g_htm_file      = NULL;

/*
==============================================================================

    Static Function

==============================================================================
*/
static void _tag_element_free(
    IN _tag_element_t   *t
)
{
    _tag_element_t  *n;

    if ( t == NULL ) {
        return;
    }

    while ( t ) {
        n = t->next;

        if ( t->value_str ) {
            icli_free( t->value_str );
        }
        icli_free( t );

        t = n;
    }
}

static void _htm_element_free(
    IN _htm_element_t   *h
)
{
    _htm_element_t  *n;

    while ( h ) {
        n = h->next;

        _tag_element_free( h->begin );
        //_tag_element_free( h->command );
        //_tag_element_free( h->end );
        icli_free( h );

        h = n;
    }
}

/*
    input -
        f : file descriptor
    output -
        str_buf : buffer to store the line read
    return -
        icli_rc_t
*/
static i32 _line_read(FILE *f)
{
    i32     i;

    if ( fgets(g_line_buf, ICLI_STR_MAX_LEN, f) == NULL ) {
        // eof or failed
        return ICLI_RC_ERROR;
    }

    // remove \n, \r and space from tail
    i = vtss_icli_str_len(g_line_buf);
    for ( --i;
          g_line_buf[i] == '\n' || g_line_buf[i] == '\r' || g_line_buf[i] == ' ';
          g_line_buf[i--] = 0 ) {
        ;
    }

    // successful and increase line number
    ++g_line_number;
    return ICLI_RC_OK;
}

/*----------------------------------------------------------------------------

    HTM Generation

----------------------------------------------------------------------------*/
#define _HTM_TITLE             "CLI Command Reference Guide"
#define _TAG_TITLE_BEGIN        "<div class=Title>"
#define _TAG_REFLINK_BEGIN      "<div class=RefLink>"
#define _HTM_REFLINK_BEGIN     "<p class=RefLink>"
#define _HTM_REFLINK_END       "</a></p>"
#define _HTM_COMMAND_BEGIN     "<div class=Command>"
#define _HTM_COMMAND_END       "</div>"
#define _HTM_COMMAND_TITLE     "<p class=CmdTitle>"
#define _HTM_NEW_LINE          "<p class=NewLine>&nbsp;</p>"
#define _HTM_COMMAND_CNT       "<!-- Number of commands"
#define _HTM_NULL_DESC         "none."

static const char _htm_head[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
    "\n"
    "<!-- Number of commands\n"
    "%04d\n"
    "-->\n"
    "\n"
    "<html>\n"
    "\n"
    "<head>\n"
    "<title></title>\n"
    "<link href=\"vtss.css\" rel=\"stylesheet\" type=\"text/css\">\n"
    "</head>\n"
    "\n"
    "<body lang=EN-US>\n"
    "\n"
    "<!-- Title -->\n"
    "<div class=Title>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "<p class=Title>\n"
    "%s\n"
    "</p>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "</div>\n"
    "\n"
    "<!-- Reference Link -->\n"
    "<div class=RefLink>\n"
    "<hr>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "\n";

#define ___STRUCT_MALLOC(p,s,b) \
    if ( b ) {\
        if ( p != NULL ) {\
            __T_E("duplicate "#p"\n");\
            return ICLI_RC_ERROR;\
        }\
    }\
    if ((p) == NULL) {\
        (p) = (s *)icli_malloc(sizeof(s));\
        if ( (p) == NULL ) {\
            __T_E("icli_malloc() for "#p"\n");\
            return ICLI_RC_ERROR;\
        }\
        memset((p), 0, sizeof(s));\
    }

#define ___VALUE_STR_GET(p) \
    p = (char *)icli_malloc(vtss_icli_str_len(g_value_str)+1);\
    if ( p == NULL ) {\
        __T_E("icli_malloc() for "#p" by length %d\n", vtss_icli_str_len(g_value_str)+1);\
        return ICLI_RC_ERROR;\
    }\
    (void)vtss_icli_str_cpy(p, g_value_str);

#define ___VALUE_STR_MALLOC_AND_GET(p, b) \
        ___STRUCT_MALLOC(p, _tag_element_t, b);\
        ___VALUE_STR_GET(p->value_str);\
        p->line = g_line_number;

#define ___ADD_TO_TAIL(s,p,t)\
{\
    s    *n;\
    for (n=(p); n->next!=NULL; ___NEXT(n));\
    n->next = t;\
}

#define ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL(p) \
{\
        _tag_element_t    *t = NULL;\
\
        ___VALUE_STR_MALLOC_AND_GET(t, FALSE);\
\
        if ( p == NULL ) {\
            p = t;\
        } else {\
            ___ADD_TO_TAIL(_tag_element_t, p, t);\
        }\
}

#define ___HTM_ADD_TO_TAIL(p)\
    (void)vtss_icli_str_cpy(g_value_str, g_line_buf);\
    ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL(p);

#if 0 /* not used */
static void _read_element_free(
    void
)
{
    _htm_element_free( g_read_reflink );
    _htm_element_free( g_read_command );

    g_read_reflink = NULL;
    g_read_command = NULL;
    g_read_cmd_cnt = 0;
}
#endif

#if 0 /* CP, 2012/09/13 15:22, try until ireg_file is generated */
#define __TRY_TIMES     10
#endif

static i32 _htm_read(
    IN char     *htm_file
)
{
    FILE            *f;
    _tag_element_t  *t;
    _htm_element_t  *h,
                    *tail;

    /* open file as read/write */
#if 0 /* CP, 2012/09/13 15:22, try until ireg_file is generated */
    i32     i;

    for ( i = 0; i < __TRY_TIMES; ++i ) {
        f = fopen( htm_file, "r" );
        if ( f != NULL ) {
            break;
        } else {
#ifdef WIN32
            Sleep( 500 );
#else
            sleep( 1 );
#endif
        }
    }
#else
    f = fopen( htm_file, "r" );
#endif

    if ( f == NULL ) {
        __T_E("open %s\n", htm_file);
        return ICLI_RC_ERROR;
    }

    /* reset */
    g_read_reflink = NULL;
    g_read_command = NULL;
    g_read_cmd_cnt = 0;

    while (_line_read(f) == ICLI_RC_OK) {
        /* get g_read_cmd_cnt */
        if ( vtss_icli_str_cmp(g_line_buf, _HTM_COMMAND_CNT) == 0 ) {
            if (_line_read(f) != ICLI_RC_OK) {
                __T_E("get g_read_cmd_cnt\n");
                fclose( f );
                return ICLI_RC_ERROR;
            }
            g_read_cmd_cnt = atoi( g_line_buf );
            continue;
        }

        /* get g_read_reflink */
        if ( vtss_icli_str_cmp(g_line_buf, _HTM_REFLINK_BEGIN) == 0 ) {
            h = (_htm_element_t *)icli_malloc(sizeof(_htm_element_t));
            if ( h == NULL ) {
                __T_E("icli_malloc\n");
                fclose( f );
                return ICLI_RC_ERROR;
            }
            memset(h, 0, sizeof(_htm_element_t));

            ___HTM_ADD_TO_TAIL(h->begin);

            while (_line_read(f) == ICLI_RC_OK) {
                ___HTM_ADD_TO_TAIL(h->begin);
                if ( vtss_icli_str_cmp(g_line_buf, _HTM_REFLINK_END) == 0 ) {
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->end = t;
                    break;
                } else if ( ICLI_IS_KEYWORD(g_line_buf[0]) ) {
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->command = t;
                }
            }

            if ( g_read_reflink ) {
                for (tail = g_read_reflink; tail->next != NULL; ___NEXT(tail)) {
                    ;
                }
                tail->next = h;
                h->prev    = tail;
            } else {
                g_read_reflink = h;
            }
        }

        /* get g_read_command */
        if ( vtss_icli_str_cmp(g_line_buf, _HTM_COMMAND_BEGIN) == 0 ) {
            h = (_htm_element_t *)icli_malloc(sizeof(_htm_element_t));
            if ( h == NULL ) {
                __T_E("icli_malloc\n");
                fclose( f );
                return ICLI_RC_ERROR;
            }
            memset(h, 0, sizeof(_htm_element_t));

            ___HTM_ADD_TO_TAIL(h->begin);

            while (_line_read(f) == ICLI_RC_OK) {
                ___HTM_ADD_TO_TAIL(h->begin);
                if ( vtss_icli_str_cmp(g_line_buf, _HTM_COMMAND_END) == 0 ) {
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->end = t;
                    break;
                } else if ( vtss_icli_str_sub("<a name=", g_line_buf, 1, NULL) == 1 ) {
                    //read command
                    if ( _line_read(f) != ICLI_RC_OK ) {
                        __T_E("read command\n");
                        fclose( f );
                        return ICLI_RC_ERROR;
                    }
                    ___HTM_ADD_TO_TAIL(h->begin);
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->command = t;
                }
            }

            if ( g_read_command ) {
                for (tail = g_read_command; tail->next != NULL; ___NEXT(tail)) {
                    ;
                }
                tail->next = h;
                h->prev    = tail;
            } else {
                g_read_command = h;
            }
        }
    }

    /* close file */
    fclose( f );
    return ICLI_RC_OK;
}

#define ___HTM_DEF_TAG_SIZE    32

#define ___t_ALLOC(t,s) \
    t = (_tag_element_t *)icli_malloc(sizeof(_tag_element_t));\
    if ( t == NULL ) {\
        __T_E("icli_malloc\n");\
        return NULL;\
    }\
    memset(t, 0, sizeof(_tag_element_t));\
    t->value_str = (char *)icli_malloc(s);\
    if ( t->value_str == NULL ) {\
        __T_E("icli_malloc\n");\
        return NULL;\
    }\
    memset(t->value_str, 0, s);

#define ___HTM_NEW_LINE_ADD()\
    ___t_ALLOC(t, ___HTM_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, _HTM_NEW_LINE);\
    p->next = t;\
    p = t;

#define ___HTM_TAG_BEGIN_ADD(s)\
    ___t_ALLOC(t, ___HTM_DEF_TAG_SIZE);\
    (void)vtss_icli_str_cpy(t->value_str, s);\
    h->begin = t;\
    p        = t;

#define ___HTM_TAG_ADD(s)\
    ___t_ALLOC(t, ___HTM_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, s);\
    p->next = t;\
    p = t;\

#define ___HTM_TAG_ADD_CHAR_STR(s)\
    ___t_ALLOC(t, ___HTM_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, "%s", s);\
    p->next = t;\
    p = t;\

#define ___HTM_STRING_ADD(l, format, s)\
    ___t_ALLOC(t, (l));\
    icli_sprintf(t->value_str, format, s);\
    p->next = t;\
    p = t;\

#define ___HTM_TAG_END_ADD(s)\
    ___t_ALLOC(t, ___HTM_DEF_TAG_SIZE);\
    (void)vtss_icli_str_cpy(t->value_str, s);\
    p->next = t;\
    h->end  = t;

#define ___HTM_SUBTITLE1_ADD(s)\
    ___HTM_TAG_ADD("<p class=CmdSubTitle1>");\
    ___HTM_TAG_ADD(s);\
    ___HTM_TAG_ADD("</p>");

#define ___HTM_ELEMENT_WRITE(element)\
    if ( element ) {\
        for (h = element; h != NULL; ___NEXT(h)) {\
            for (t = h->begin; t != NULL; ___NEXT(t)) {\
                fprintf(f, "%s\n", t->value_str);\
            }\
            fprintf(f, "\n");\
        }\
    }

static i32 _htm_write(void)
{
    FILE            *f;
    _tag_element_t  *t;
    _htm_element_t  *h;

    /* open file as read/write */
    f = fopen( g_htm_file, "w" );
    if ( f == NULL ) {
        __T_E("open %s\n", g_htm_file);
        return ICLI_RC_ERROR;
    }

    fprintf(f, _htm_head, g_htm_cmd_cnt, _HTM_TITLE);

    ___HTM_ELEMENT_WRITE( g_htm_reflink );

    fprintf(f, "%s\n", _HTM_NEW_LINE);
    fprintf(f, "</div>\n");
    fprintf(f, "\n");

    ___HTM_ELEMENT_WRITE( g_htm_command );

    fprintf(f, "</body></html>\n");
    fprintf(f, "\n");

    /* close file */
    fclose( f );
    return ICLI_RC_OK;
}

#define ___HTML_ELEMENT_INSERT(element)\
    h->next = 0;\
    h->prev = 0;\
    e = element;\
    if ( e ) {\
        for ( p = NULL; e != NULL; p = e, ___NEXT(e)) {\
            if ( vtss_icli_str_cmp(e->command->value_str, h->command->value_str) == 1 ) {\
                break;\
            }\
        }\
        if ( e ) {\
            if ( e == element ) {\
                element = h;\
            } else {\
                h->prev = e->prev;\
                e->prev->next = h;\
            }\
            e->prev = h;\
            h->next = e;\
        } else {\
            p->next = h;\
            h->prev = p;\
        }\
    } else {\
        element = h;\
    }

static i32 _htm_add(
    void
)
{
    _htm_element_t      *h,
                        *e,
                        *p,
                        *n;
    _tag_element_t      *t;
    int                 i;

    /* first one */
    if ( g_htm_reflink == NULL ) {
        g_htm_reflink = g_read_reflink;
        g_htm_command = g_read_command;
        g_htm_cmd_cnt = g_read_cmd_cnt;
        return ICLI_RC_OK;
    }

    /* add into reflink */
    h = g_read_reflink;
    while ( h ) {
        n = h->next;

        // find <a href
        for ( t = h->begin; t; t = t->next ) {
            if ( vtss_icli_str_sub("<a href=", t->value_str, 1, NULL) == 1 ) {
                break;
            }
        }
        if ( t == NULL ) {
            __T_E("fail to find <a href=\n");
            return ICLI_RC_ERROR;
        }

        // get the value
        t->value_str[16] = 0;
        i = atoi( &(t->value_str[12]) );

        // update href
        icli_sprintf(t->value_str, "<a href=\"#cr%04d\">", i + g_htm_cmd_cnt);

        // insert
        ___HTML_ELEMENT_INSERT( g_htm_reflink );

        h = n;
    }

    /* add into command */
    h = g_read_command;
    while ( h ) {
        n = h->next;

        // find <a name
        for ( t = h->begin; t; t = t->next ) {
            if ( vtss_icli_str_sub("<a name=", t->value_str, 1, NULL) == 1 ) {
                break;
            }
        }
        if ( t == NULL ) {
            __T_E("fail to find <a name=\n");
            return ICLI_RC_ERROR;
        }

        // get the value
        t->value_str[15] = 0;
        i = atoi( &(t->value_str[11]) );

        // update name
        icli_sprintf(t->value_str, "<a name=\"cr%04d\">", i + g_htm_cmd_cnt);

        // insert
        ___HTML_ELEMENT_INSERT( g_htm_command );

        h = n;
    }

    /* update count */
    g_htm_cmd_cnt += g_read_cmd_cnt;

    return ICLI_RC_OK;
}

static const char _feature_list_htm_head[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
    "\n"
    "<html>\n"
    "\n"
    "<head>\n"
    "<title></title>\n"
    "<link href=\"vtss.css\" rel=\"stylesheet\" type=\"text/css\">\n"
    "</head>\n"
    "\n"
    "<body lang=EN-US>\n"
    "\n"
    "<!-- Title -->\n"
    "<div class=Title>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "<p class=Title>\n"
    "Feature List of CLI commands\n"
    "</p>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "</div>\n"
    "\n"
    "<!-- Reference Link -->\n"
    "<div class=RefLink>\n"
    "<hr>\n"
    "<p class=NewLine>&nbsp;</p>\n"
    "\n";

static i32 _feature_list_htm_gen(
    IN  i32     argc,
    IN  char    *argv[]
)
{
    i32             i;
    i32             j;
    u32             len;
    char            *c;
    _tag_element_t  *list = NULL;
    _tag_element_t  *t;
    _tag_element_t  *p, *e;
    FILE            *f;

    /* get feature list */
    for ( i = 2; i < argc; ++i ) {
        /* get g_value_str */
        len = vtss_icli_str_len( argv[i] );
        // skip path to find the begin
        for ( j = len - 1; (j >= 0) && (argv[i][j] != '/') && (argv[i][j] != '\\'); --j ) {
            ;
        }
        // skip the begin
        ++j;
        // copy to g_value_str
        (void)vtss_icli_str_cpy(g_value_str, &(argv[i][j]));

        // create t
        t = NULL;
        ___VALUE_STR_MALLOC_AND_GET(t, FALSE);

        // sorting insert
        if ( list ) {
            for ( p = NULL, e = list; e != NULL; p = e, ___NEXT(e)) {
                if ( vtss_icli_str_cmp(e->value_str, t->value_str) == 1 ) {
                    break;
                }
            }
            if ( e ) {
                if ( e == list ) {
                    list = t;
                } else {
                    p->next = t;
                }
                t->next = e;
            } else {
                p->next = t;
            }
        } else {
            list = t;
        }
    }

    /* create feature list html */
    g_htm_file = icli_malloc(vtss_icli_str_len(argv[1]) + 32);
    if ( g_htm_file == NULL ) {
        __T_E("fail to allocate memory for g_htm_file\n");
        return ICLI_RC_ERROR;
    }
    icli_sprintf(g_htm_file, "%s/feature_list.htm", g_generate_path);

    /* open file as read/write */
    f = fopen(g_htm_file, "w");
    if ( f == NULL ) {
        __T_E("open %s\n", g_htm_file);
        return ICLI_RC_ERROR;
    }

    fprintf(f, _feature_list_htm_head);

    for ( t = list; t; ___NEXT(t) ) {
        fprintf(f, "<p class=RefLink>\n");
        fprintf(f, "<a href=\"%s\">\n", t->value_str);
        for ( c = t->value_str; ICLI_NOT_(EOS, *c) && ICLI_NOT_(DOT, *c); ++c ) {
            fprintf(f, "%c", *c);
        }
        fprintf(f, "\n");
        fprintf(f, "</a></p>\n");
        fprintf(f, "</a></p>\n\n");
    }

    fprintf(f, "%s\n", _HTM_NEW_LINE);
    fprintf(f, "</div>\n");
    fprintf(f, "\n");

    fprintf(f, "</body></html>\n");
    fprintf(f, "\n");

    /* close file */
    fclose( f );
    return ICLI_RC_OK;
}

/*
==============================================================================

    Public Function

==============================================================================
*/
int main(i32 argc, char *argv[])
{
    int     i;

    if ( argc < 3 ) {
        ___USAGE_PRINT();
        return ICLI_RC_ERROR;
    }

    // get g_generate_path
    i = vtss_icli_str_len( argv[1] );
    g_generate_path = icli_malloc(i + 1);
    if ( g_generate_path == NULL ) {
        __T_E("fail to allocate memory for g_generate_path\n");
        return ICLI_RC_ERROR;
    }
    (void)vtss_icli_str_cpy(g_generate_path, argv[1]);

    // get cmd_ref.htm
    g_htm_file = icli_malloc(i + 32);
    if ( g_htm_file == NULL ) {
        __T_E("fail to allocate memory for g_htm_file\n");
        return ICLI_RC_ERROR;
    }
    icli_sprintf(g_htm_file, "%s/cmd_ref.htm", g_generate_path);

    for ( i = 2; i < argc; ++i ) {
        /* read htm */
        if ( _htm_read(argv[i]) != ICLI_RC_OK ) {
            __T_E("fail to read %s\n", argv[i]);
            return ICLI_RC_ERROR;
        }

        /* add into cmd ref */
        if ( _htm_add() != ICLI_RC_OK ) {
            __T_E("fail to add %s\n", argv[i]);
            return ICLI_RC_ERROR;
        }
    }

    if ( _htm_write() != ICLI_RC_OK ) {
        __T_E("fail to write %s\n", g_htm_file);
        return ICLI_RC_ERROR;
    }

    _htm_element_free( g_htm_reflink );
    _htm_element_free( g_htm_command );

    icli_free( g_htm_file );

#if 1 /* CP, 2012/11/07 11:28, Bugzilla#10191 - table list to each component html */
    if ( _feature_list_htm_gen(argc, argv) == ICLI_RC_ERROR ) {
        __T_E("fail to generate component list\n");
        return ICLI_RC_ERROR;
    }
#endif

    icli_free( g_generate_path );
    return ICLI_RC_OK;
}

