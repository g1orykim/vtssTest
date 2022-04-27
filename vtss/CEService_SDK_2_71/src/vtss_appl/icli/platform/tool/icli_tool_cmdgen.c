/*
 Vitesse Switch Application software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse".
 All Rights Reserved.

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

    1. generate C and H file from ICLI script file(*.icli).
    2. auto command register
    3. generate HTML
    Run on linux and Windows

    Revision history
    > CP Wang, 2012/04/05 20:54
        - not do 2. and 3.
        - but generate IREG and IHTL files
    > CP.Wang, 2011/03/09 20:18
        - create

==============================================================================
*/

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "icli_tool_platform.h"
#include "vtss_icli.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#define __UL_E(_line_, ...) \
     printf("%s.icli:%d: error: ", g_script_file, _line_); \
     printf(__VA_ARGS__);

#define __U_E(...) \
     printf("%s.icli:%d: error: ", g_script_file, g_current_cmd ? g_current_cmd->command->line : 0); \
     printf(__VA_ARGS__);

/*
    if 0, then the simple html is generated,
    that does not generate Description, Default, Usage Guideline and Example
*/
#define _COMPLETE_HTML              0

#define _TAG_MAX_LEN                32
#define _VALUE_MAX_LEN              ICLI_STR_MAX_LEN
#define _CMD_MAX_CNT                512   // for each ICLI script, not for total
#define _WORD_MAX_CNT               ICLI_CMD_WORD_CNT   // for each command
#define _SCRIPT_SUFFIX              ".icli"
#define _SYNTAX_STACK_SIZE          64
#define _REPEAT_SYNTAX              "..."
#define _CONST_STR_PREFIX           "##"

#if 1 /* CP, 2012/09/05 17:42, <port_type_id> */
#define _PORT_TYPE                  "<port_type>"
#define _HELP_PORT_TYPE             "Port type in Fast, Giga or Tengiga ethernet"
#define _PORT_TYPE_ID               "<port_type_id>"
#define _HELP_PORT_ID               "Port ID in the format of switch-no/port-no"
#define _BYWORD_PORT_ID              "PORT_ID"
#define _XORT_TYPE_ID               "<xort_type_id>"
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
#define _PORT_TYPE_LIST             "<port_type_list>"
#define _HELP_PORT_LIST             "List of Port ID, ex, 1/1,3-5;2/2-4,6"
#define _BYWORD_PORT_LIST           "PORT_LIST"
#define _XORT_TYPE_LIST             "<xort_type_list>"
#endif

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
#define _DSCP_VAR_MAX_CNT           16
#define _DSCP_VAR                   "<dscp>"
#define _DSCP_EXPAND                "{ be | af11 | af12 | af13 | af21 | af22 | af23 | af31 | af32 | af33 | af41 | af42 | af43 | cs1 | cs2 | cs3 | cs4 | cs5 | cs6 | cs7 | ef | va }"
#endif

#define _NO_FORM_HELP               "Negate a command or set its defaults"
#define _NO_FORM_PREFIX             "no "
#define _NO_FORM_POSTFIX            "_no"

#define _DEFAULT_FORM_HELP          "Set a command to its defaults"
#define _DEFAULT_FORM_PREFIX        "default "
#define _DEFAULT_FORM_POSTFIX       "_default"

typedef enum {
    _SEGMENT_INIT,
    _SEGMENT_BEGIN,
    _SEGMENT_END
} seg_state_t;

#define ___VALID_FILE_CHAR(c) \
    ( ICLI_IS_ALPHABET(c)   || \
      ICLI_IS_DIGIT(c)      || \
      ICLI_IS_(UNDERLINE,c) )

#define ___GREP_CMD_STR         " { [ <grep> { <grep_begin> | <grep_include> | <grep_exclude> } <grep_string> ] } "
#define ___GREP_WORD_CNT        5
#define ___GREP_HELP            "Output modifiers"
#define ___GREP_BEGIN_HELP      "Begin with the line that matches"
#define ___GREP_INCLUDE_HELP    "Include lines that match"
#define ___GREP_EXCLUDE_HELP    "Exclude lines that match"
#define ___GREP_STRING_HELP     "String to match output lines"

/*
    p: pointer of structure
    s: struct type
    b: check if existed already
*/
#define ___STRUCT_MALLOC(p,s,b) \
    if ( b ) {\
        if ( p != NULL ) {\
            T_E("duplicate "#p"\n");\
            return ICLI_RC_ERROR;\
        }\
    }\
    if ((p) == NULL) {\
        (p) = (s *)icli_malloc(sizeof(s));\
        if ( (p) == NULL ) {\
            T_E("icli_malloc() for "#p"\n");\
            return ICLI_RC_ERROR;\
        }\
        memset((p), 0, sizeof(s));\
    }

#define ___VALUE_STR_GET(p) \
    p = (char *)icli_malloc(vtss_icli_str_len(g_value_str)+1);\
    if ( p == NULL ) {\
        T_E("icli_malloc() for "#p" by length %u\n", vtss_icli_str_len(g_value_str)+1);\
        return ICLI_RC_ERROR;\
    }\
    (void)vtss_icli_str_cpy(p, g_value_str);

#define ___TAG_GET(p) \
    p = (char *)icli_malloc(vtss_icli_str_len(g_tag_str)+1);\
    if ( p == NULL ) {\
        T_E("icli_malloc() for "#p" by length %u\n", vtss_icli_str_len(g_tag_str)+1);\
        return ICLI_RC_ERROR;\
    }\
    (void)vtss_icli_str_cpy(p, g_tag_str);

#define ___VALUE_STR_GET_WIHT_EXTRA_LEN(p, len) \
    p = (char *)icli_malloc(vtss_icli_str_len(g_value_str)+len+1);\
    if ( p == NULL ) {\
        T_E("icli_malloc() for "#p" by length %u\n", vtss_icli_str_len(g_value_str)+len+1);\
        return ICLI_RC_ERROR;\
    }\
    (void)vtss_icli_str_cpy(p, g_value_str);

#define ___LINE_BUF_GET(p) \
    p = (char *)icli_malloc(vtss_icli_str_len(g_line_buf)+1);\
    if ( p == NULL ) {\
        T_E("icli_malloc() for "#p" by length %u\n", vtss_icli_str_len(g_line_buf)+1);\
        return ICLI_RC_ERROR;\
    }\
    (void)vtss_icli_str_cpy(p, g_line_buf);

#define ___MORE_GET()\
{\
    i32 r;\
    for ( r=_more_value_str_get(); r==ICLI_RC_ERR_MORE; r=_more_value_str_get() );\
    if ( r != ICLI_RC_OK ) {\
        T_E("fail to _more_value_str_get()\n");\
        return ICLI_RC_ERROR;\
    }\
}

#define ___ADD_TO_TAIL(s,p,t)\
{\
    s    *n;\
    for ( n=(p); n->next; ___NEXT(n) );\
    n->next = t;\
}

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef enum {

    //------- Global Segment
    _TAG_MODULE_IF_FLAG,

    //------- Include Segment
    _TAG_INCLUDE_BEGIN,
    _TAG_INCLUDE_END,

    //------- Function Segment
    _TAG_FUNCTION_BEGIN,
    _TAG_FUNCTION_END,

    //------- Export Segment
    _TAG_EXPORT_BEGIN,
    _TAG_EXPORT_END,

    //------- Command Segment
    _TAG_CMD_BEGIN,
    _TAG_COMMAND,
    _TAG_DOC_CMD_DESC,
    _TAG_DOC_CMD_DEFAULT,
    _TAG_DOC_CMD_USAGE,
    _TAG_DOC_CMD_EXAMPLE,
    _TAG_FUNC_NAME,
    _TAG_FUNC_REUSE,
    _TAG_PRIVILEGE,
    _TAG_CMD_MODE,
    _TAG_GOTO_MODE,
    _TAG_DESTROY_MODE,
    _TAG_SUB_MODE,
    _TAG_USR_OPT,
    _TAG_IF_FLAG,
    _TAG_MODE_VAR,
    _TAG_CMD_VAR,
    _TAG_RUNTIME,
    _TAG_BYWORD,
    _TAG_HELP,
    _TAG_PROPERTY,
    _TAG_VARIABLE_BEGIN,
    _TAG_VARIABLE_END,
    _TAG_CODE_BEGIN,
    _TAG_CODE_END,

    _TAG_NO_FORM_DOC_CMD_DESC,
    _TAG_NO_FORM_DOC_CMD_DEFAULT,
    _TAG_NO_FORM_DOC_CMD_USAGE,
    _TAG_NO_FORM_DOC_CMD_EXAMPLE,
    _TAG_NO_FORM_VARIABLE_BEGIN,
    _TAG_NO_FORM_VARIABLE_END,
    _TAG_NO_FORM_CODE_BEGIN,
    _TAG_NO_FORM_CODE_END,

    _TAG_DEFAULT_FORM_DOC_CMD_DESC,
    _TAG_DEFAULT_FORM_DOC_CMD_DEFAULT,
    _TAG_DEFAULT_FORM_DOC_CMD_USAGE,
    _TAG_DEFAULT_FORM_DOC_CMD_EXAMPLE,
    _TAG_DEFAULT_FORM_VARIABLE_BEGIN,
    _TAG_DEFAULT_FORM_VARIABLE_END,
    _TAG_DEFAULT_FORM_CODE_BEGIN,
    _TAG_DEFAULT_FORM_CODE_END,

    _TAG_CMD_END,

    // max tag
    _TAG_MAX,
    _TAG_STR_DEF = _TAG_MAX
} _tag_t;

typedef struct __tag_element {
    i32                     line;
    char                    *value_str;
    // next element
    struct __tag_element    *next;
} _tag_element_t;

typedef struct __tag_value_element {
    i32                         line;
    char                        *tag;
    char                        *value_str;
    // next element
    struct __tag_value_element  *next;
} _tag_value_element_t;

typedef struct __module {
    _tag_element_t          *module_if_flag;
} _module_t;

typedef struct __segment {
    i32                     line;
    _tag_element_t          *element;
    // next segment
    struct __segment        *next;
} _segment_t;

typedef struct __command {
    char                    *original_cmd;
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    char                    *var_cmd;
#endif
    i32                     command_word_cnt;
    i32                     grep;
    _tag_element_t          *command;
    _tag_element_t          *doc_cmd_desc;
    _tag_element_t          *doc_cmd_default;
    _tag_element_t          *doc_cmd_usage;
    _tag_element_t          *doc_cmd_example;
    _tag_element_t          *func_name;
    _tag_element_t          *func_reuse;
    _tag_element_t          *privilege;
    _tag_element_t          *cmd_mode;
    _tag_element_t          *mode_goto;
    _tag_element_t          *mode_destroy;
    _tag_element_t          *mode_sub;
    _tag_element_t          *usr_opt;
    _tag_element_t          *if_flag;
    _tag_element_t          *mode_var;
    _tag_element_t          *cmd_var;
    _tag_element_t          *runtime;
    _tag_element_t          *byword;
    _tag_element_t          *help;
    _tag_element_t          *property;
    _segment_t              *variable;
    _segment_t              *code;

    _tag_element_t          *no_form_doc_cmd_desc;
    _tag_element_t          *no_form_doc_cmd_default;
    _tag_element_t          *no_form_doc_cmd_usage;
    _tag_element_t          *no_form_doc_cmd_example;
    _segment_t              *no_form_variable;
    _segment_t              *no_form_code;

    _tag_element_t          *default_form_doc_cmd_desc;
    _tag_element_t          *default_form_doc_cmd_default;
    _tag_element_t          *default_form_doc_cmd_usage;
    _tag_element_t          *default_form_doc_cmd_example;
    _segment_t              *default_form_variable;
    _segment_t              *default_form_code;

    /* command mode information */
    i32                         mode_word_cnt;
    const icli_cmd_mode_info_t  *mode_info;

#if 1 /* CP, 2012/09/17 15:47, add ICLI_CMD_PROP_LOOSELY to no command */
    i32     b_no_command;
#endif

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
    i32     dscp_word_cnt;
    i32     dscp_word_id[_DSCP_VAR_MAX_CNT];
#endif

    // next command
    struct __command    *next;
} _command_t;

typedef struct {
    u32     cnt;
    struct {
        char    *min;
        char    *max;
    } range[ICLI_RANGE_LIST_CNT];
} _range_constant_t;

typedef struct {
    char        *word;
    i32         para_cnt;
    char        *para[ICLI_BYWORD_PARA_MAX_CNT];
} _byword_constant_t;

/*
==============================================================================

    Static Variable

==============================================================================
*/
static char *g_tag_name[] = {
    "MODULE_IF_FLAG",                   // _TAG_MODULE_IF_FLAG

    "INCLUDE_BEGIN",                    // _TAG_INCLUDE_BEGIN
    "INCLUDE_END",                      // _TAG_INCLUDE_END

    "FUNCTION_BEGIN",                   // _TAG_FUNCTION_BEGIN
    "FUNCTION_END",                     // _TAG_FUNCTION_END

    "EXPORT_BEGIN",                     // _TAG_EXPORT_BEGIN
    "EXPORT_END",                       // _TAG_EXPORT_END

    "CMD_BEGIN",                        // _TAG_CMD_BEGIN
    "COMMAND",                          // _TAG_COMMAND
    "DOC_CMD_DESC",                     // _TAG_DOC_CMD_DESC
    "DOC_CMD_DEFAULT",                  // _TAG_DOC_CMD_DEFAULT
    "DOC_CMD_USAGE",                    // _TAG_DOC_CMD_USAGE
    "DOC_CMD_EXAMPLE",                  // _TAG_DOC_CMD_EXAMPLE
    "FUNC_NAME",                        // _TAG_FUNC_NAME
    "FUNC_REUSE",                       // _TAG_FUNC_REUSE
    "PRIVILEGE",                        // _TAG_PRIVILEGE
    "CMD_MODE",                         // _TAG_CMD_MODE
    "GOTO_MODE",                        // _TAG_GOTO_MODE
    "DESTROY_MODE",                     // _TAG_DESTROY_MODE
    "SUB_MODE",                         // _TAG_SUB_MODE
    "USR_OPT",                          // _TAG_USR_OPT
    "IF_FLAG",                          // _TAG_IF_FLAG
    "MODE_VAR",                         // _TAG_MODE_VAR
    "CMD_VAR",                          // _TAG_CMD_VAR
    "RUNTIME",                          // _TAG_RUNTIME
    "BYWORD",                           // _TAG_BYWORD
    "HELP",                             // _TAG_HELP
    "PROPERTY",                         // _TAG_PROPERTY
    "VARIABLE_BEGIN",                   // _TAG_VARIABLE_BEGIN
    "VARIABLE_END",                     // _TAG_VARIABLE_END
    "CODE_BEGIN",                       // _TAG_CODE_BEGIN
    "CODE_END",                         // _TAG_CODE_END

    "NO_FORM_DOC_CMD_DESC",             // _TAG_NO_FORM_DOC_CMD_DESC
    "NO_FORM_DOC_CMD_DEFAULT",          // _TAG_NO_FORM_DOC_CMD_DEFAULT
    "NO_FORM_DOC_CMD_USAGE",            // _TAG_NO_FORM_DOC_CMD_USAGE
    "NO_FORM_DOC_CMD_EXAMPLE",          // _TAG_NO_FORM_DOC_CMD_EXAMPLE
    "NO_FORM_VARIABLE_BEGIN",           // _TAG_NO_FORM_VARIABLE_BEGIN
    "NO_FORM_VARIABLE_END",             // _TAG_NO_FORM_VARIABLE_END
    "NO_FORM_CODE_BEGIN",               // _TAG_NO_FORM_CODE_BEGIN
    "NO_FORM_CODE_END",                 // _TAG_NO_FORM_CODE_END

    "DEFAULT_FORM_DOC_CMD_DESC",        // _TAG_DEFAULT_FORM_DOC_CMD_DESC
    "DEFAULT_FORM_DOC_CMD_DEFAULT",     // _TAG_DEFAULT_FORM_DOC_CMD_DEFAULT
    "DEFAULT_FORM_DOC_CMD_USAGE",       // _TAG_DEFAULT_FORM_DOC_CMD_USAGE
    "DEFAULT_FORM_DOC_CMD_EXAMPLE",     // _TAG_DEFAULT_FORM_DOC_CMD_EXAMPLE
    "DEFAULT_FORM_VARIABLE_BEGIN",      // _TAG_DEFAULT_FORM_VARIABLE_BEGIN
    "DEFAULT_FORM_VARIABLE_END",        // _TAG_DEFAULT_FORM_VARIABLE_END
    "DEFAULT_FORM_CODE_BEGIN",          // _TAG_DEFAULT_FORM_CODE_BEGIN
    "DEFAULT_FORM_CODE_END",            // _TAG_DEFAULT_FORM_CODE_END

    "CMD_END"                           // _TAG_CMD_END
};

static _module_t            *g_module       = NULL;
static _segment_t           *g_include      = NULL;
static _segment_t           *g_function     = NULL;
static _segment_t           *g_export       = NULL;
static _command_t           *g_command      = NULL;
static _command_t           *g_current_cmd  = NULL;
static _tag_value_element_t *g_str_def      = NULL;

static FILE                 *g_fd;
static i32                  g_line_number;
static i32                  g_command_number;
static char                 g_line_buf[ICLI_STR_MAX_LEN * 2];
static char                 g_tag_str[_TAG_MAX_LEN + 1];
static char                 g_value_str[_VALUE_MAX_LEN + 1];

//the stack is used for check command syntax,
static char                 g_syntax_stack[_SYNTAX_STACK_SIZE];
static i32                  g_stack_index;

static char                 *g_mode_var_name[_CMD_MAX_CNT][_WORD_MAX_CNT];
static icli_variable_type_t g_mode_var_type[_CMD_MAX_CNT][_WORD_MAX_CNT];
static icli_range_t         *g_mode_var_range[_CMD_MAX_CNT][_WORD_MAX_CNT];

static char                 *g_variable_name[_CMD_MAX_CNT][_WORD_MAX_CNT];
static icli_variable_type_t g_variable_type[_CMD_MAX_CNT][_WORD_MAX_CNT];
static icli_range_t         *g_variable_range[_CMD_MAX_CNT][_WORD_MAX_CNT];

static _byword_constant_t   g_variable_byword[_CMD_MAX_CNT][_WORD_MAX_CNT];

static BOOL                 g_line_directive = FALSE;

static char                 *g_script_file   = NULL;
static char                 *g_script_path   = NULL;
static char                 *g_generate_path = NULL;
static char                 *g_icli_c_file   = NULL;
static char                 *g_icli_h_file   = NULL;
static char                 *g_html_file     = NULL;
static char                 *g_txt_file      = NULL;

static u32                  g_func_name_number = 0;

/*
==============================================================================

    Static Function

==============================================================================
*/
static void  _stack_init( void )
{
    g_stack_index = -1;
}

//return icli_rc_t
static i32  _stack_push(
    IN  char    c
)
{
    if ( (g_stack_index + 1) >= _SYNTAX_STACK_SIZE ) {
        T_E("overflow\n");
        return ICLI_RC_ERR_OVERFLOW;
    }
    ++g_stack_index;
    g_syntax_stack[g_stack_index] = c;
    return ICLI_RC_OK;
}

//return icli_rc_t
static i32  _stack_pop(
    IN  char    c
)
{
    if ( g_stack_index < 0 ) {
        T_E("empty\n");
        return ICLI_RC_ERR_EMPTY;
    }
    if ( g_stack_index >= _SYNTAX_STACK_SIZE ) {
        T_E("overflow\n");
        return ICLI_RC_ERR_OVERFLOW;
    }
    if ( g_syntax_stack[g_stack_index] != c ) {
        switch (g_syntax_stack[g_stack_index]) {
        case ICLI_MANDATORY_BEGIN:
            __U_E("expect '%c'\n", ICLI_MANDATORY_END);
            break;
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        case ICLI_LOOP_BEGIN:
            __U_E("expect '%c'\n", ICLI_LOOP_END);
            break;
#endif
        case ICLI_OPTIONAL_BEGIN:
            __U_E("expect '%c'\n", ICLI_OPTIONAL_END);
            break;
        default :
            __U_E("'%c' != '%c'\n", g_syntax_stack[g_stack_index], c);
            break;
        }
        return ICLI_RC_ERROR;
    }
    --( g_stack_index );
    return ICLI_RC_OK;
}

//TRUE: empty, FALSE: not empty
static BOOL  _stack_is_empty( void )
{
    if ( g_stack_index < 0 ) {
        return TRUE;
    }
    return FALSE;
}

/*
    return -
        tag value if successful
        _TAG_MAX if failed
*/
static _tag_t _tag_get_by_str(char *tag_str)
{
    i32 i;

    for ( i = 0; i < _TAG_MAX; ++i ) {
        if (vtss_icli_str_cmp(tag_str, g_tag_name[i]) == 0) {
            return i;
        }
    }
    return i;
}

static _tag_t _tag_get( void )
{
    return _tag_get_by_str(g_tag_str);
}

/*
    input -
        fd : file descriptor
    output -
        str_buf : buffer to store the line read
    return -
        icli_rc_t
*/
static i32 _line_read( void )
{
    i32     i;

    if ( fgets(g_line_buf, ICLI_STR_MAX_LEN, g_fd) == NULL ) {
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

/*
    return -
        icli_rc_t
*/
static i32 _str_tag_and_value_get( void )
{
    i32     i, j;

    /* get g_tag_str */

    /* CP, 2012/08/24 16:03, skip space or TAB */
    for ( i = 0; ICLI_IS_(SPACE, g_line_buf[i]) || ICLI_IS_(TAB, g_line_buf[i]); ++i ) {
        ;
    }

    //skip comment
    if ( ICLI_IS_(SLASH, g_line_buf[i]) && ICLI_IS_(SLASH, g_line_buf[i + 1]) ) {
        return ICLI_RC_ERR_EMPTY;
    }

    // skip comment
    if ( ICLI_IS_COMMENT(g_line_buf[i]) ) {
        return ICLI_RC_ERR_EMPTY;
    }

    // skip empty line
    if ( ICLI_IS_(EOS, g_line_buf[i]) ) {
        return ICLI_RC_ERR_EMPTY;
    }

    // get g_tag_str
    /* CP, 2012/08/24 16:03, skip space or TAB */
    for (   j = 0;
            ICLI_NOT_(SPACE, g_line_buf[i]) && ICLI_NOT_(TAB, g_line_buf[i]) && ICLI_NOT_(EQUAL, g_line_buf[i]) && ICLI_NOT_(EOS, g_line_buf[i]);
            ++i, ++j ) {
        if ( j >= _TAG_MAX_LEN ) {
            T_E("fail to get tag str, too long, %d > MAX_LEN(%d)\n", j, _TAG_MAX_LEN);
            return ICLI_RC_ERROR;
        }
        g_tag_str[j] = g_line_buf[i];
    }

    // end of string
    g_tag_str[j] = 0;

    // get '='
    /* CP, 2012/08/24 16:03, skip space or TAB */
    for ( ; ICLI_IS_(SPACE, g_line_buf[i]) || ICLI_IS_(TAB, g_line_buf[i]); ++i ) {
        ;
    }

    switch ( g_line_buf[i] ) {
    case ICLI_EOS :
        // only tag, without value
        g_value_str[0] = 0;
        return ICLI_RC_OK;

    case ICLI_EQUAL :
        // go on to get value
        break;

    default :
        // wrong
        T_E("missing '=' or 'End of string' after tag %s at line %d\n", g_tag_str, g_line_number);
        return ICLI_RC_ERROR;
    }

    /* get g_value_str */

    /* CP, 2012/08/24 16:03, skip space or TAB */
    for ( ++i; ICLI_IS_(SPACE, g_line_buf[i]) || ICLI_IS_(TAB, g_line_buf[i]); ++i ) {
        ;
    }

    // get g_value_str
    for ( j = 0; g_line_buf[i] != 0; ++i, ++j ) {
        if ( j >= _VALUE_MAX_LEN ) {
            T_E("fail to get value str, too long, %d > MAX_LEN(%d)\n", j, _VALUE_MAX_LEN);
            return ICLI_RC_ERROR;
        }
        g_value_str[j] = g_line_buf[i];
    }

    // end of string
    g_value_str[j] = 0;

    // skip space and TAB
    for ( --j; ICLI_IS_(SPACE, g_value_str[j]) || ICLI_IS_(TAB, g_value_str[j]); --j ) {
        ;
    }

    // find '\' if more
    if (g_value_str[j] == '\\') {
        g_value_str[j] = 0;
        return ICLI_RC_ERR_MORE;
    }

    // skip space
    g_value_str[j + 1] = 0;
    return ICLI_RC_OK;
}

/*
    return -
        icli_rc_t
*/
static i32 _more_value_str_get( void )
{
    i32     i, j;

    if ( _line_read() != ICLI_RC_OK ) {
        T_E("fail to _line_read()\n");
        return ICLI_RC_ERROR;
    }

    /* CP, 2012/08/24 16:03, skip space or TAB */
    for ( i = 0; ICLI_IS_(SPACE, g_line_buf[i]) || ICLI_IS_(TAB, g_line_buf[i]); ++i ) {
        ;
    }

    // get length of g_value_str, that is, where to append
    j = vtss_icli_str_len( g_value_str );

    // get g_value_str
    for ( ; g_line_buf[i] != 0; ++i, ++j ) {
        if ( j >= _VALUE_MAX_LEN ) {
            T_E("fail to get more value str, too long, %d > MAX_LEN(%d)\n", j, _VALUE_MAX_LEN);
            return ICLI_RC_ERROR;
        }
        g_value_str[j] = g_line_buf[i];
    }

    // end of string
    g_value_str[j] = 0;

    /* CP, 2012/08/24 16:03, skip space or TAB */
    for ( --j; ICLI_IS_(SPACE, g_value_str[j]) || ICLI_IS_(TAB, g_value_str[j]); --j ) {
        ;
    }

    // find '\' if more
    if (g_value_str[j] == '\\') {
        g_value_str[j] = 0;
        return ICLI_RC_ERR_MORE;
    }

    // skip space
    g_value_str[j + 1] = 0;
    return ICLI_RC_OK;
}

#define ___VALUE_STR_MALLOC_AND_GET(p, b) \
        ___STRUCT_MALLOC(p, _tag_element_t, b);\
        ___VALUE_STR_GET(p->value_str);\
        p->line = g_line_number;

#define ___VALUE_STR_MALLOC_AND_GET_WITH_EXTRA_LEN(p, b, len) \
        ___STRUCT_MALLOC(p, _tag_element_t, b);\
        ___VALUE_STR_GET_WIHT_EXTRA_LEN(p->value_str, len);\
        p->line = g_line_number;

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

static i32 _add_str_def( void )
{
    _tag_value_element_t    *tv;

    if ( vtss_icli_str_len(g_value_str) == 0 ) {
        __UL_E(g_line_number, "no string defined for tag '%s'\n", g_tag_str);
        return ICLI_RC_ERROR;
    }

    tv = (_tag_value_element_t *)icli_malloc(sizeof(_tag_value_element_t));
    if ( tv == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    memset(tv, 0, sizeof(_tag_value_element_t));

    tv->line = g_line_number;
    ___TAG_GET( tv->tag );
    ___VALUE_STR_GET( tv->value_str );

    if ( g_str_def == NULL ) {
        g_str_def = tv;
    } else {
        tv->next  = g_str_def;
        g_str_def = tv;
    }
    return ICLI_RC_OK;
}

static i32 _value_str_get( void )
{
    i32     r = ICLI_RC_OK;

    switch (_tag_get()) {

        /*
            Global segment
        */
    case _TAG_MODULE_IF_FLAG:
        ___VALUE_STR_MALLOC_AND_GET(g_module->module_if_flag, TRUE);
        break;

        /*
            Command segment
        */
    case _TAG_COMMAND:
        ___VALUE_STR_MALLOC_AND_GET_WITH_EXTRA_LEN(g_current_cmd->command, TRUE, vtss_icli_str_len(___GREP_CMD_STR));
        break;

    case _TAG_DOC_CMD_DESC:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->doc_cmd_desc );
        break;

    case _TAG_DOC_CMD_DEFAULT:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->doc_cmd_default );
        break;

    case _TAG_DOC_CMD_USAGE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->doc_cmd_usage );
        break;

    case _TAG_DOC_CMD_EXAMPLE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->doc_cmd_example );
        break;

    case _TAG_FUNC_NAME:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->func_name, TRUE);
        break;

    case _TAG_FUNC_REUSE:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->func_reuse, TRUE);
        break;

    case _TAG_PRIVILEGE:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->privilege, TRUE);
        break;

    case _TAG_CMD_MODE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->cmd_mode );
        break;

    case _TAG_GOTO_MODE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->mode_goto );
        break;

    case _TAG_DESTROY_MODE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->mode_destroy );
        break;

    case _TAG_SUB_MODE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->mode_sub );
        break;

    case _TAG_USR_OPT:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->usr_opt, TRUE);
        break;

    case _TAG_IF_FLAG:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->if_flag, TRUE);
        break;

    case _TAG_MODE_VAR:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->mode_var );
        break;

    case _TAG_CMD_VAR:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->cmd_var );
        break;

    case _TAG_RUNTIME:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->runtime );
        break;

    case _TAG_BYWORD:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->byword );
        break;

    case _TAG_HELP:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->help );
        break;

    case _TAG_PROPERTY:
        ___VALUE_STR_MALLOC_AND_GET(g_current_cmd->property, TRUE);
        break;

    case _TAG_STR_DEF:
        r = _add_str_def();
        break;

    case _TAG_NO_FORM_DOC_CMD_DESC:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->no_form_doc_cmd_desc );
        break;

    case _TAG_NO_FORM_DOC_CMD_DEFAULT:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->no_form_doc_cmd_default );
        break;

    case _TAG_NO_FORM_DOC_CMD_USAGE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->no_form_doc_cmd_usage );
        break;

    case _TAG_NO_FORM_DOC_CMD_EXAMPLE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->no_form_doc_cmd_example );
        break;

    case _TAG_DEFAULT_FORM_DOC_CMD_DESC:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->default_form_doc_cmd_desc );
        break;

    case _TAG_DEFAULT_FORM_DOC_CMD_DEFAULT:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->default_form_doc_cmd_default );
        break;

    case _TAG_DEFAULT_FORM_DOC_CMD_USAGE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->default_form_doc_cmd_usage );
        break;

    case _TAG_DEFAULT_FORM_DOC_CMD_EXAMPLE:
        ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL( g_current_cmd->default_form_doc_cmd_example );
        break;

    default:
        r = ICLI_RC_ERROR;
        // Fall through
    }
    return r;
}

static i32 _section_parsing(
    _tag_t              begin_tag,
    _tag_t              end_tag,
    _segment_t          **segment,
    i32                 init_state)
{
    i32               s = init_state;
    _segment_t        *g = NULL;
    _tag_element_t    *t = NULL;

    // parsing line by line
    while (_line_read() == ICLI_RC_OK) {
        switch (s) {
        case _SEGMENT_INIT:

            switch ( _str_tag_and_value_get() ) {
            case ICLI_RC_OK:
                if ( _tag_get() == begin_tag ) {
                    s = _SEGMENT_BEGIN;
                } else {
                    T_E("invalid tag - %s at line %d\n", g_tag_str, g_line_number);
                    return ICLI_RC_ERROR;
                }
                break;

            case ICLI_RC_ERR_EMPTY :
                // next line
                break;

            default:
                T_E("fail to _str_tag_and_value_get()\n");
                return ICLI_RC_ERROR;
            }
            break;

        case _SEGMENT_BEGIN:

            if ( _tag_get_by_str(g_line_buf) == end_tag ) {
                /* nothing inside the segment */
                return ICLI_RC_OK;
            } else {
                ___STRUCT_MALLOC(t, _tag_element_t, FALSE);
                ___LINE_BUF_GET(t->value_str);

                if ( g == NULL ) {
                    ___STRUCT_MALLOC(g, _segment_t, FALSE);
                    g->element = t;
                    g->line = g_line_number;
                    if (*segment == NULL) {
                        *segment = g;
                    } else {
                        ___ADD_TO_TAIL(_segment_t, (*segment), g);
                    }
                } else {
                    ___ADD_TO_TAIL(_tag_element_t, g->element, t);
                }

                // reset to get next line
                t = NULL;
            }
            break;

        case _SEGMENT_END:
        default:
            T_E("invalid state s = %d\n", s);
            return ICLI_RC_ERROR;
        }
    }
    T_E("should not be here\n");
    return ICLI_RC_ERROR;
}

#if 0 /* not used */
static void _segment_printf(
    char                *t,
    _segment_t    *s)
{
    _tag_element_t    *e;

    T_E("/* %s debug begin */\n", t);
    for ( ; s != NULL; ___NEXT(s) ) {
        for ( e = s->element; e != NULL;  ___NEXT(e) ) {
            T_E("%s\n", e->value_str);
        }
    }
    T_E("/* %s debug end */\n", t);
}
#endif

#define ___VALUE_STR(p)\
    ( g_current_cmd->p->value_str )

#define ___MODULE_WITH_VALUE_STR(p)\
    ( g_module->p && vtss_icli_str_len(g_module->p->value_str) )

#define ___CMD_NO_VALUE_STR(p)\
    ( g_current_cmd->p == NULL || vtss_icli_str_len(g_current_cmd->p->value_str) == 0 )

#define ___CMD_WITH_VALUE_STR(p)\
    ( g_current_cmd->p && vtss_icli_str_len(g_current_cmd->p->value_str) )

#define ___CMD_EXIST_CHECK_RETURN(p)\
    if ___CMD_NO_VALUE_STR(p)\
    {\
        T_E(#p" does not exist or length = 0\n");\
        return ICLI_RC_ERROR;\
    }

#define ___REPEAT_PARSING(begin, end, inbegin, inend)\
    for ( c = l-1; (c >= v) && (out_b == NULL); --c ) {\
        switch( *c ) {\
            case begin:\
                if ( in_b ) {\
                    out_b = c;\
                } else {\
                    if ( in_t ) {\
                        if ( *in_t != end ) {\
                            __U_E("begin and end not mapping in repeat\n");\
                            return ICLI_RC_ERROR;\
                        }\
                        in_b = c;\
                    } else {\
                        __U_E("no any selection in repeat\n");\
                        return ICLI_RC_ERROR;\
                    }\
                }\
                break;\
            case inbegin:\
                if ( in_t ) {\
                    if ( *in_t != inend ) {\
                        __U_E("begin and end not mapping in repeat\n");\
                        return ICLI_RC_ERROR;\
                    }\
                    in_b = c;\
                } else {\
                    __U_E("orphan begin in repeat\n");\
                    return ICLI_RC_ERROR;\
                }\
                break;\
            case end:\
            case inend:\
                if ( in_t ) {\
                    __U_E("multiple mandatory or optional, wrong format in repeat\n");\
                    return ICLI_RC_ERROR;\
                } else {\
                    in_t = c;\
                }\
                break;\
            case ICLI_OR:\
                ++n_or;\
                break;\
            default:\
                break;\
        }\
    }

#define ___tag_ALLOC(t, str_len) \
    t = (_tag_element_t *)icli_malloc(sizeof(_tag_element_t));\
    if ( t == NULL ) {\
        T_E("icli_malloc\n");\
        return ICLI_RC_ERROR;\
    }\
    memset(t, 0, sizeof(_tag_element_t));\
    t->value_str = (char *)icli_malloc(str_len);\
    if ( t->value_str == NULL ) {\
        T_E("icli_malloc\n");\
        return ICLI_RC_ERROR;\
    }\
    memset(t->value_str, 0, str_len);

#define ___tag_FILL_FULL(tag_list) \
    if ( g_current_cmd->tag_list ) {\
        for ( i=1, t=g_current_cmd->tag_list; t->next != NULL; ++i, ___NEXT(t) );\
        if ( i < w_cnt ) {\
            for ( ; i < w_cnt; ++i ) {\
                ___tag_ALLOC(t_new, 1);\
                t->next = t_new;\
                t       = t_new;\
            }\
        }\
    } else {\
        ___tag_ALLOC(t_new, 1);\
        g_current_cmd->tag_list = t_new;\
        for ( i = 1, t = t_new; i < w_cnt; ++i ) {\
            ___tag_ALLOC(t_new, 1);\
            t->next = t_new;\
            t       = t_new;\
        }\
    }

#define ___tag_CMD_EXPAND(tag_list) \
    for ( i=0, t_repeat=g_current_cmd->tag_list; i<pre_cnt; ++i, ___NEXT(t_repeat) );\
    for ( i=0, t_repeat_end=t_repeat; i<(repeat_cnt-1); ++i, ___NEXT(t_repeat_end) );\
    t_tail = t_repeat_end->next;\
    t_start = t_repeat_end;\
    for ( i = 0; i < n_or; ++i ) {\
        for ( j=0, t = t_repeat; j < repeat_cnt; ++j, ___NEXT(t) ) {\
            len = vtss_icli_str_len(t->value_str);\
            str_len = len + 1 + 2;\
            ___tag_ALLOC(t_new, str_len);\
            if ( len ) {\
                (void)vtss_icli_str_cpy(t_new->value_str, t->value_str);\
                t_new->value_str[len] = '_';\
                t_new->value_str[len+1] = (char)('1' + i);\
            }\
            t_start->next = t_new;\
            t_start       = t_new;\
        }\
    }\
    t_start->next = t_tail;

#define ___tag_EXPAND(tag_list) \
    for ( i=0, t_repeat=g_current_cmd->tag_list; i<pre_cnt; ++i, ___NEXT(t_repeat) );\
    for ( i=0, t_repeat_end=t_repeat; i<(repeat_cnt-1); ++i, ___NEXT(t_repeat_end) );\
    t_tail = t_repeat_end->next;\
    t_start = t_repeat_end;\
    for ( i = 0; i < n_or; ++i ) {\
        for ( j=0, t = t_repeat; j < repeat_cnt; ++j, ___NEXT(t) ) {\
            len = vtss_icli_str_len(t->value_str);\
            str_len = len + 1;\
            ___tag_ALLOC(t_new, str_len);\
            if ( len ) {\
                (void)vtss_icli_str_cpy(t_new->value_str, t->value_str);\
            }\
            t_start->next = t_new;\
            t_start       = t_new;\
        }\
    }\
    t_start->next = t_tail;

static i32 _repeat_process( void )
{
    i32     i,
            j,
            len,
            str_len;

    char    *v,
            *c,
            *cmd;

    i32     pre_cnt,
            repeat_cnt,
            w_cnt;

    // number of | bar
    i32     n_or;

    // outer begin and tail
    // 1   2       3 4   5
    // { a [ b | c ] ... }
    // 1:out_b, 2:in_b, 3:in_t, 4:l, 5:out_t
    char    *l,
            *out_b,
            *out_t,
            *in_b,
            *in_t;

    _tag_element_t  *t,
                    *t_start,
                    *t_new,
                    *t_repeat,
                    *t_repeat_end,
                    *t_tail;

    BOOL    b_optional;

    // process repeat "..."
    l = vtss_icli_str_str(_REPEAT_SYNTAX, ___VALUE_STR(command));

    if ( l == NULL ) {
        return ICLI_RC_OK;
    }

    for ( c = l + 3; ICLI_IS_(SPACE, *c); ++c ) {
        ;
    }

    if ( ICLI_NOT_(MANDATORY_END, *c) && ICLI_NOT_(OPTIONAL_END, *c) ) {
        __U_E("repeat(...) should be followed by } or ]\n");
        return ICLI_RC_ERROR;
    }

    // get outer tail
    out_t = c;

    v     = ___VALUE_STR(command);
    out_b = NULL;
    in_t  = NULL;
    in_b  = NULL;
    n_or  = 0;

    switch ( *c ) {
    case ICLI_MANDATORY_END:
        ___REPEAT_PARSING(ICLI_MANDATORY_BEGIN, ICLI_MANDATORY_END, ICLI_OPTIONAL_BEGIN, ICLI_OPTIONAL_END);
        break;

    case ICLI_OPTIONAL_END:
        ___REPEAT_PARSING(ICLI_OPTIONAL_BEGIN, ICLI_OPTIONAL_END, ICLI_MANDATORY_BEGIN, ICLI_MANDATORY_END);
        break;

    default :
        break;
    }

    if ( n_or == 0 ) {
        __U_E("no select items in repeat(...)\n");
        return ICLI_RC_ERROR;
    }

    if ( out_b == NULL ) {
        __U_E("no corresponding begin in repeat(...)\n");
        return ICLI_RC_ERROR;
    }

    // check if only optional inside repeat
    b_optional = FALSE;
    if ( *in_b == ICLI_OPTIONAL_BEGIN ) {
        // check begin
        for ( c = out_b + 1; ICLI_IS_(SPACE, *c); ++c ) {
            ;
        }

        // check tail
        if ( c == in_b ) {
            for ( c = l - 1; ICLI_IS_(SPACE, *c); --c ) {
                ;
            }
            if ( c == in_t ) {
                b_optional = TRUE;
            }
        }
    }

    // revise command
    len = vtss_icli_str_len(v) * (n_or + 1) + 1;
    cmd = icli_malloc(len);
    if ( cmd == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    memset(cmd, 0, len);

    if ( b_optional ) {
        (void)vtss_icli_str_ncpy(cmd, v, (u32)(out_b - v + 1));
        c = cmd + vtss_icli_str_len(cmd);
        *c++ = ICLI_SPACE;

        for ( i = 0; i <= n_or; ++i ) {
            if ( i ) {
                *c++ = ICLI_OPTIONAL_BEGIN;
                *c++ = ICLI_SPACE;
            }
            *c++ = ICLI_MANDATORY_BEGIN;
            *c++ = ICLI_SPACE;

            (void)vtss_icli_str_ncpy(c, in_b + 1, (u32)(in_t - in_b - 1));
            c += vtss_icli_str_len(c);

            *c++ = ICLI_MANDATORY_END;
            *c++ = ICLI_SPACE;
        }

        for ( i = 0; i <= n_or - 1; ++i ) {
            *c++ = ICLI_OPTIONAL_END;
            *c++ = ICLI_SPACE;
        }

        *c++ = ICLI_SPACE;
        (void)vtss_icli_str_cpy(c, out_t);
    } else {
        (void)vtss_icli_str_ncpy(cmd, v, (u32)(l - v));
        c = cmd + vtss_icli_str_len(cmd);
        *c++ = ICLI_SPACE;

        for ( i = 0; i < n_or; ++i ) {
            (void)vtss_icli_str_ncpy(c, out_b + 1, (u32)(l - out_b - 1));
            c += vtss_icli_str_len(c);
            *c++ = ICLI_SPACE;
        }

        *c++ = ICLI_SPACE;
        (void)vtss_icli_str_cpy(c, out_t);
    }

    // change value_str
    ___VALUE_STR(command) = cmd;

    // get w_cnt
    // -1 because of repeat "..."
    w_cnt = vtss_icli_word_count( v ) - 1;
    if ( w_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }

    // get word count for prefix and repeat
    *out_b = ICLI_EOS;
    *l     = ICLI_EOS;

    pre_cnt  = vtss_icli_word_count( v );
    if ( pre_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }
    repeat_cnt = vtss_icli_word_count( out_b + 1 );
    if ( repeat_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }

    /*
        expand CMD_VAR
    */

    // fill for full
    ___tag_FILL_FULL( cmd_var );

    // expand
    ___tag_CMD_EXPAND( cmd_var );

    /*
        expand RUNTIME
    */

    // fill for full
    ___tag_FILL_FULL( runtime );

    // expand
    ___tag_EXPAND( runtime );

    /*
        expand BYWORD
    */
    // fill for full
    ___tag_FILL_FULL( byword );

    // expand
    ___tag_EXPAND( byword );

    /*
        expand HELP
    */
    // fill for full
    ___tag_FILL_FULL( help );

    // expand
    ___tag_EXPAND( help );

    // free original command string
    icli_free(v);
    return ICLI_RC_OK;
}

#if 1 /* CP, 2012/09/05 17:42, <port_type_id> */
static i32 _port_type_id_process( void )
{
    char                *p;
    char                *v;
    char                *cmd;
    char                *c;
    u32                 len;
    i32                 pre_cnt;
    i32                 i;
    _tag_element_t      *t;
    _tag_element_t      *new_tag;
    i32                 rc;

    /* check if exist or not */
    p = vtss_icli_str_str(_PORT_TYPE_ID, ___VALUE_STR(command));
    if ( p == NULL ) {
        return ICLI_RC_OK;
    }

    /* get count before <port_type_id> to insert all tags */
    *p = 0;
    v = ___VALUE_STR(command);
    pre_cnt = vtss_icli_word_count( v );
    if ( pre_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }
    *p = '<';
    // change to _XORT_TYPE_ID for recursive later
    *(p + 1) = 'x';

    /*
        Expand command string
    */

    /* get new command buffer */
    // 2 for two spaces
    len = vtss_icli_str_len(v) + vtss_icli_str_len(_PORT_TYPE) + 16 + 1;
    cmd = icli_malloc(len);
    if ( cmd == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    memset(cmd, 0, len);

    // copy before
    *p = 0;
    (void)vtss_icli_str_cpy(cmd, v);
    *p = '<';

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c = ' ';

    // add _PORT_TYPE
    (void)vtss_icli_str_cpy(c + 1, _PORT_TYPE);

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c = ' ';

    // copy after
    (void)vtss_icli_str_cpy(c + 1, p);

    /* change value_str */
    ___VALUE_STR(command) = cmd;

    /* free original command string */
    icli_free(v);
    v = NULL; // to prevent use later

    /*
        Expand cmd_var
    */
    for ( i = 1, t = g_current_cmd->cmd_var; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one
        ___tag_ALLOC(new_tag, 1);
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand runtime
    */
    for ( i = 1, t = g_current_cmd->runtime; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one
        ___tag_ALLOC(new_tag, vtss_icli_str_len(t->next->value_str) + 1);
        // copy value
        new_tag->line = t->next->line;
        (void)vtss_icli_str_cpy(new_tag->value_str, t->next->value_str);
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand byword
    */
    for ( i = 1, t = g_current_cmd->byword; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one
        ___tag_ALLOC(new_tag, 1);
        // change byword for port id
        t->next->value_str = _BYWORD_PORT_ID;
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand help
    */
    for ( i = 1, t = g_current_cmd->help; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one
        ___tag_ALLOC(new_tag, vtss_icli_str_len(_HELP_PORT_TYPE) + 1);
        // help for port type
        new_tag->line = t->next->line;
        (void)vtss_icli_str_cpy(new_tag->value_str, _HELP_PORT_TYPE);
        // change help for port id
        t->next->value_str = _HELP_PORT_ID;
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /* check if exist or not */
    rc = ICLI_RC_OK;
    p = vtss_icli_str_str(_PORT_TYPE_ID, ___VALUE_STR(command));
    if ( p ) {
        rc = _port_type_id_process();
    } else {
        // no more, then change _XORT_TYPE_ID back to _PORT_TYPE_ID
        p = ___VALUE_STR(command);
        for ( p = vtss_icli_str_str(_XORT_TYPE_ID, p); p; p = vtss_icli_str_str(_XORT_TYPE_ID, p + 1) ) {
            *(p + 1) = 'p';
        }
        // get word count
        g_current_cmd->command_word_cnt = vtss_icli_word_count( ___VALUE_STR(command) );
        if ( g_current_cmd->command_word_cnt < 0 ) {
            T_E("fail to get command word count\n");
            return ICLI_RC_ERROR;
        }
        if ( g_current_cmd->command_word_cnt > _WORD_MAX_CNT ) {
            __U_E("too many words\n");
            return ICLI_RC_ERROR;
        }
    }
    return rc;
}
#endif /* CP, 2012/09/05 17:42, <port_type_id> */

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
static i32 _port_type_list_process( void )
{
    char            *p;
    char            *v;
    char            *cmd;
    u32             len;
    i32             pre_cnt;
    i32             i;
    _tag_element_t  *t;
    _tag_element_t  *new_tag;
    i32             rc;
    char            *c;

    /* check if exist or not */
    p = vtss_icli_str_str(_PORT_TYPE_LIST, ___VALUE_STR(command));
    if ( p == NULL ) {
        return ICLI_RC_OK;
    }

    /* get count before <port_type_list> to insert all tags */
    *p = 0;
    v = ___VALUE_STR(command);
    pre_cnt = vtss_icli_word_count( v );
    if ( pre_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }
    *p = '<';

    /*
        Expand command string
    */

    /* get new command buffer */
    len = vtss_icli_str_len(v) + vtss_icli_str_len(_PORT_TYPE) + 16 + 1;
    cmd = icli_malloc( len );
    if ( cmd == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    memset(cmd, 0, len);

    // copy before
    *p = 0;
    (void)vtss_icli_str_cpy(cmd, v);
    *p = '<';

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c++ = ' ';
    *c++ = '(';
    *c = ' ';

    // add _PORT_TYPE
    (void)vtss_icli_str_cpy(c + 1, _PORT_TYPE);

    // add space
    c = cmd + vtss_icli_str_len(cmd);

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    *c++ = ' ';
    *c++ = '[';
#endif

    *c = ' ';

    // add _XORT_TYPE_LIST
    (void)vtss_icli_str_cpy(c + 1, _XORT_TYPE_LIST);

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c++ = ' ';

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    *c++ = ']';
    *c++ = ' ';
#endif

    *c++ = ')';
    *c = ' ';

    // copy after
    p += vtss_icli_str_len(_PORT_TYPE_LIST);
    (void)vtss_icli_str_cpy(c + 1, p);

    /* change value_str */
    ___VALUE_STR(command) = cmd;

    /* free original command string */
    icli_free(v);
    v = NULL; // to prevent use later

    /*
        Expand cmd_var
    */
    for ( i = 1, t = g_current_cmd->cmd_var; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one for _XORT_TYPE_LIST
        ___tag_ALLOC(new_tag, 1);
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand runtime
    */
    for ( i = 1, t = g_current_cmd->runtime; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one for _XORT_TYPE_LIST
        ___tag_ALLOC(new_tag, vtss_icli_str_len(t->next->value_str) + 1);
        // copy value
        new_tag->line = t->next->line;
        (void)vtss_icli_str_cpy(new_tag->value_str, t->next->value_str);
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand byword
    */
    for ( i = 1, t = g_current_cmd->byword; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }

    if ( t && t->next ) {
        // allocate new one for _XORT_TYPE_LIST
        ___tag_ALLOC(new_tag, 1);
        // change byword for port list
        t->next->value_str = _BYWORD_PORT_LIST;
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /*
        Expand help
    */
    for ( i = 1, t = g_current_cmd->help; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t && t->next ) {
        // allocate new one for _XORT_TYPE_LIST
        ___tag_ALLOC(new_tag, vtss_icli_str_len(_HELP_PORT_TYPE) + 1);
        // help for port type
        new_tag->line = t->next->line;
        (void)vtss_icli_str_cpy(new_tag->value_str, _HELP_PORT_TYPE);
        // change help for port id
        t->next->value_str = _HELP_PORT_LIST;
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
    }

    /* check if exist or not */
    rc = ICLI_RC_OK;
    p = vtss_icli_str_str(_PORT_TYPE_LIST, ___VALUE_STR(command));
    if ( p ) {
        rc = _port_type_list_process();
    } else {
        // no more, then change _XORT_TYPE_LIST back to _PORT_TYPE_LIST
        p = ___VALUE_STR(command);
        for ( p = vtss_icli_str_str(_XORT_TYPE_LIST, p); p; p = vtss_icli_str_str(_XORT_TYPE_LIST, p + 1) ) {
            *(p + 1) = 'p';
        }
    }
    return rc;
}
#endif /* CP, 2012/09/07 11:52, <port_type_list> */

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
static i32 _dscp_process( void )
{
    char            *p;
    char            *v;
    char            *cmd;
    char            *c;
    u32             len;
    i32             pre_cnt;
    i32             i;
    _tag_element_t  *t;
    _tag_element_t  *pre_tag;
    _tag_element_t  *new_tag;
    i32             rc;
    i32             j;

    icli_dscp_wvh_t *dscp_wvh = vtss_icli_variable_dscp_wvh_get();

    /* check if exist or not */
    p = vtss_icli_str_str(_DSCP_VAR, ___VALUE_STR(command));
    if ( p == NULL ) {
        return ICLI_RC_OK;
    }

    /* get count before <dscp> to insert all tags */
    *p = 0;
    v = ___VALUE_STR(command);
    pre_cnt = vtss_icli_word_count( v );
    if ( pre_cnt < 0 ) {
        T_E("fail to get command word count\n");
        return ICLI_RC_ERROR;
    }
    *p = '<';

    /* store word id and cnt */
    g_current_cmd->dscp_word_id[g_current_cmd->dscp_word_cnt] = pre_cnt;
    ++( g_current_cmd->dscp_word_cnt );

    /*
        Expand command string
    */

    /* get new command buffer */
    len = vtss_icli_str_len(v) + vtss_icli_str_len(_DSCP_EXPAND) + 8;
    cmd = icli_malloc(len);
    if ( cmd == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    memset(cmd, 0, len);

    // copy before
    *p = 0;
    (void)vtss_icli_str_cpy(cmd, v);
    *p = '<';

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c = ' ';

    // add _DSCP_EXPAND
    (void)vtss_icli_str_cpy(c + 1, _DSCP_EXPAND);

    // add space
    c = cmd + vtss_icli_str_len(cmd);
    *c = ' ';

    // copy after
    p += vtss_icli_str_len(_DSCP_VAR);
    (void)vtss_icli_str_cpy(c + 1, p);

    /* change value_str */
    ___VALUE_STR(command) = cmd;

    /* free original command string */
    icli_free(v);
    v = NULL; // to prevent use later

    /*
        Expand cmd_var
    */
    for ( i = 0, t = g_current_cmd->cmd_var; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t ) {
        len = vtss_icli_str_len(t->value_str) + 1;
        for ( j = 1; j < ICLI_DSCP_MAX_CNT; ++j ) {
            // allocate new one
            ___tag_ALLOC(new_tag, len);
            // copy
            new_tag->line = t->line;
            (void)vtss_icli_str_cpy(new_tag->value_str, t->value_str);
            // chain
            new_tag->next = t->next;
            t->next = new_tag;
            t = new_tag;
        }
    }

    /*
        Expand runtime
    */
    for ( i = 0, t = g_current_cmd->runtime; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t ) {
        len = vtss_icli_str_len(t->value_str) + 1;
        for ( j = 1; j < ICLI_DSCP_MAX_CNT; ++j ) {
            // allocate new one
            ___tag_ALLOC(new_tag, len);
            // copy
            new_tag->line = t->line;
            (void)vtss_icli_str_cpy(new_tag->value_str, t->value_str);
            // chain
            new_tag->next = t->next;
            t->next = new_tag;
            t = new_tag;
        }
    }

    /*
        Expand byword
    */
    for ( i = 0, t = g_current_cmd->byword; i < pre_cnt && t != NULL; ++i, ___NEXT(t) ) {
        ;
    }
    if ( t ) {
        t->value_str[0] = 0;
        for ( j = 1; j < ICLI_DSCP_MAX_CNT; ++j ) {
            // allocate new one
            ___tag_ALLOC(new_tag, 1);
            // copy
            new_tag->line = t->line;
            // chain
            new_tag->next = t->next;
            t->next = new_tag;
            t = new_tag;
        }
    }

    /*
        Expand help
    */
    if ( g_current_cmd->help ) {
        for ( i = 0, pre_tag = NULL, t = g_current_cmd->help;
              i < pre_cnt && t != NULL;
              ++i, pre_tag = t, ___NEXT(t) ) {
            ;
        }
    } else {
        // allocate new one
        ___tag_ALLOC(new_tag, 1);
        // chain
        g_current_cmd->help = new_tag;
        pre_tag = new_tag;
        t = NULL;
        // increase i
        i = 1;
    }

    // the number of help tags is not enough
    if ( t == NULL ) {
        for ( ; i <= pre_cnt; ++i ) {
            // allocate new one
            ___tag_ALLOC(new_tag, 1);
            // chain
            if ( pre_tag ) {
                pre_tag->next = new_tag;
            }
            pre_tag = new_tag;
        }
        t = pre_tag;
    }

    j = 0;
    c = (char *)icli_malloc(vtss_icli_str_len(dscp_wvh[j].help) + 1);
    if ( c == NULL ) {
        T_E("icli_malloc()\n");
        return ICLI_RC_ERROR;
    }
    (void)vtss_icli_str_cpy(c, dscp_wvh[j].help);
    icli_free(t->value_str);
    t->value_str = c;

    for ( j = 1; j < ICLI_DSCP_MAX_CNT; ++j ) {
        // allocate new one
        ___tag_ALLOC(new_tag, vtss_icli_str_len(dscp_wvh[j].help) + 1);
        // copy
        new_tag->line = t->line;
        (void)vtss_icli_str_cpy(new_tag->value_str, dscp_wvh[j].help);
        // chain
        new_tag->next = t->next;
        t->next = new_tag;
        t = new_tag;
    }

    /* check if exist or not */
    rc = ICLI_RC_OK;
    p = vtss_icli_str_str(_DSCP_VAR, ___VALUE_STR(command));
    if ( p ) {
        rc = _dscp_process();
    }
    return rc;
}
#endif

/*
    format command to be one-space seperated,
    and also check the syntax
*/
static i32  _cmd_formated(
    IN  char                    *cmd,
    OUT char                    *formatted_cmd
)
{
    char                *c;
    icli_word_state_t   w_state;
    BOOL                not_over;

    if ( ! ICLI_IS_KEYWORD(*cmd) ) {
        __U_E("The first char should be a keyword\n");
        return ICLI_RC_ERROR;
    }

    //
    //  check syntax and format the string with a single space
    //  j: word count
    //
    _stack_init();
    w_state = ICLI_WORD_STATE_WORD;
    for ( c = cmd, not_over = TRUE; not_over; ++c ) {
        switch ( w_state ) {
        case ICLI_WORD_STATE_WORD:
            if ( ICLI_IS_KEYWORD(*c) ) {
                w_state = ICLI_WORD_STATE_WORD;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                not_over = FALSE;
                continue;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                w_state = ICLI_WORD_STATE_VARIABLE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                if ( _stack_push(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                if ( _stack_pop(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_RANDOM_MUST(c) ) {
                    *formatted_cmd++ = *(++c);
                    *formatted_cmd++ = *(++c);
                }
#endif
                continue;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                if ( _stack_push(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                if ( _stack_pop(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                if ( _stack_push(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                if ( _stack_pop(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(OR, *c) ) {
                if ( _stack_is_empty() ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                not_over = FALSE;
                break;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                break;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                break;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                break;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(OR, *c) ) {
                break;
            }
            break;

        case ICLI_WORD_STATE_SPACE:
            if ( ICLI_IS_KEYWORD(*c) ) {
                w_state = ICLI_WORD_STATE_WORD;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                not_over = FALSE;
                continue;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                w_state = ICLI_WORD_STATE_VARIABLE;
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                if ( _stack_push(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                if ( _stack_pop(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_RANDOM_MUST(c) ) {
                    *formatted_cmd++ = *(++c);
                    *formatted_cmd++ = *(++c);
                }
#endif
                continue;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                if ( _stack_push(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                if ( _stack_pop(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                if ( _stack_push(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                if ( _stack_pop(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            if ( ICLI_IS_(OR, *c) ) {
                if ( _stack_is_empty() ) {
                    break;
                }
                *formatted_cmd++ = ICLI_SPACE;
                *formatted_cmd++ = *c;
                continue;
            }
            break;

        default:
            T_E("invalid state %u for command %s\n",
                w_state, cmd);
            return ICLI_RC_ERROR;
        }
        __U_E("invalid syntax commad = %s\n", cmd);
        return ICLI_RC_ERROR;
    }
    if ( _stack_is_empty() == FALSE ) {
        __U_E("invalid syntax at end for command = %s\n", cmd);
        _stack_pop(0);
        return ICLI_RC_ERROR;
    }
    return ICLI_RC_OK;
}

/*
    reuse the original command syntax by linking pointer
*/
static i32 _no_and_default_form_add( void )
{
    _command_t      *new_command;
    i32             len;
    i32             i;

    /* no form */
    if ( g_current_cmd->no_form_code ) {
        // reset new_command
        new_command = NULL;
        // allocate memory
        ___STRUCT_MALLOC(new_command, _command_t, FALSE);
        // get current command
        *new_command = *g_current_cmd;
        // original_cmd
        len = vtss_icli_str_len(g_current_cmd->original_cmd) + vtss_icli_str_len(_NO_FORM_PREFIX) + 1;
        new_command->original_cmd = (char *)icli_malloc(len);
        memset(new_command->original_cmd, 0, len);
        (void)vtss_icli_str_cpy(new_command->original_cmd, _NO_FORM_PREFIX);
        (void)vtss_icli_str_cpy(new_command->original_cmd + vtss_icli_str_len(_NO_FORM_PREFIX), g_current_cmd->original_cmd);
        // command_word_cnt
        ++( new_command->command_word_cnt );
        if ( new_command->command_word_cnt > _WORD_MAX_CNT ) {
            __U_E("too many words\n");
            return ICLI_RC_ERROR;
        }
        // command
        len = vtss_icli_str_len(___VALUE_STR(command)) + vtss_icli_str_len(_NO_FORM_PREFIX) + 1;
        ___tag_ALLOC(new_command->command, len);
        (void)vtss_icli_str_cpy(new_command->command->value_str, _NO_FORM_PREFIX);
        (void)vtss_icli_str_cpy(new_command->command->value_str + vtss_icli_str_len(_NO_FORM_PREFIX), ___VALUE_STR(command));
        // doc_cmd_desc
        new_command->doc_cmd_desc = g_current_cmd->no_form_doc_cmd_desc;
        // doc_cmd_default
        new_command->doc_cmd_default = g_current_cmd->no_form_doc_cmd_default;
        // doc_cmd_usage
        new_command->doc_cmd_usage = g_current_cmd->no_form_doc_cmd_usage;
        // doc_cmd_example
        new_command->doc_cmd_example = g_current_cmd->no_form_doc_cmd_example;
        // func_name
        if ___CMD_WITH_VALUE_STR( func_name ) {
            len = vtss_icli_str_len(___VALUE_STR(func_name)) + vtss_icli_str_len(_NO_FORM_POSTFIX) + 1;
            ___tag_ALLOC(new_command->func_name, len);
            (void)vtss_icli_str_cpy(new_command->func_name->value_str, ___VALUE_STR(func_name));
            (void)vtss_icli_str_cpy(new_command->func_name->value_str + vtss_icli_str_len(___VALUE_STR(func_name)), _NO_FORM_POSTFIX);
        }
        // func_reuse
        if ___CMD_WITH_VALUE_STR( func_reuse ) {
            len = vtss_icli_str_len(___VALUE_STR(func_reuse)) + vtss_icli_str_len(_NO_FORM_POSTFIX) + 1;
            ___tag_ALLOC(new_command->func_reuse, len);
            (void)vtss_icli_str_cpy(new_command->func_reuse->value_str, ___VALUE_STR(func_reuse));
            (void)vtss_icli_str_cpy(new_command->func_reuse->value_str + vtss_icli_str_len(___VALUE_STR(func_reuse)), _NO_FORM_POSTFIX);
        }
        // cmd_var
        ___tag_ALLOC(new_command->cmd_var, 1);
        new_command->cmd_var->next = g_current_cmd->cmd_var;
        // runtime
        ___tag_ALLOC(new_command->runtime, 1);
        new_command->runtime->next = g_current_cmd->runtime;
        // byword
        ___tag_ALLOC(new_command->byword, 1);
        new_command->byword->next = g_current_cmd->byword;
        // help
        len = vtss_icli_str_len(_NO_FORM_HELP) + 1;
        ___tag_ALLOC(new_command->help, len);
        (void)vtss_icli_str_cpy(new_command->help->value_str, _NO_FORM_HELP);
        new_command->help->next = g_current_cmd->help;
        // property
        new_command->property = g_current_cmd->property;
        // variable
        new_command->variable = g_current_cmd->no_form_variable;
        // code
        new_command->code = g_current_cmd->no_form_code;
        // next
        new_command->next = NULL;

        // link to g_current_cmd
        g_current_cmd->next = new_command;

#if 1 /* CP, 2012/09/17 15:47, add ICLI_CMD_PROP_LOOSELY to no command */
        new_command->b_no_command = TRUE;
#endif

        /* update dscp cnt */
        for ( i = 0; i < new_command->dscp_word_cnt; ++i ) {
            ++( new_command->dscp_word_id[i] );
        }

        // increase the number of commands
        ++( g_command_number );
    }

    /* default form */
    if ( g_current_cmd->default_form_code ) {
        // reset
        new_command = NULL;
        // allocate memory
        ___STRUCT_MALLOC(new_command, _command_t, FALSE);
        // get current command
        *new_command = *g_current_cmd;
        // original_cmd
        len = vtss_icli_str_len(g_current_cmd->original_cmd) + vtss_icli_str_len(_DEFAULT_FORM_PREFIX) + 1;
        new_command->original_cmd = (char *)icli_malloc(len);
        memset(new_command->original_cmd, 0, len);
        (void)vtss_icli_str_cpy(new_command->original_cmd, _DEFAULT_FORM_PREFIX);
        (void)vtss_icli_str_cpy(new_command->original_cmd + vtss_icli_str_len(_DEFAULT_FORM_PREFIX), g_current_cmd->original_cmd);
        // command_word_cnt
        ++( new_command->command_word_cnt );
        if ( new_command->command_word_cnt > _WORD_MAX_CNT ) {
            __U_E("too many words\n");
            return ICLI_RC_ERROR;
        }
        // command
        len = vtss_icli_str_len(___VALUE_STR(command)) + vtss_icli_str_len(_DEFAULT_FORM_PREFIX) + 1;
        ___tag_ALLOC(new_command->command, len);
        (void)vtss_icli_str_cpy(new_command->command->value_str, _DEFAULT_FORM_PREFIX);
        (void)vtss_icli_str_cpy(new_command->command->value_str + vtss_icli_str_len(_DEFAULT_FORM_PREFIX), ___VALUE_STR(command));
        // doc_cmd_desc
        new_command->doc_cmd_desc = g_current_cmd->default_form_doc_cmd_desc;
        // doc_cmd_default
        new_command->doc_cmd_default = g_current_cmd->default_form_doc_cmd_default;
        // doc_cmd_usage
        new_command->doc_cmd_usage = g_current_cmd->default_form_doc_cmd_usage;
        // doc_cmd_example
        new_command->doc_cmd_example = g_current_cmd->default_form_doc_cmd_example;
        // func_name
        if ___CMD_WITH_VALUE_STR( func_name ) {
            len = vtss_icli_str_len(___VALUE_STR(func_name)) + vtss_icli_str_len(_DEFAULT_FORM_POSTFIX) + 1;
            ___tag_ALLOC(new_command->func_name, len);
            (void)vtss_icli_str_cpy(new_command->func_name->value_str, ___VALUE_STR(func_name));
            (void)vtss_icli_str_cpy(new_command->func_name->value_str + vtss_icli_str_len(___VALUE_STR(func_name)), _DEFAULT_FORM_POSTFIX);
        }
        // func_reuse
        if ___CMD_WITH_VALUE_STR( func_reuse ) {
            len = vtss_icli_str_len(___VALUE_STR(func_reuse)) + vtss_icli_str_len(_DEFAULT_FORM_POSTFIX) + 1;
            ___tag_ALLOC(new_command->func_reuse, len);
            (void)vtss_icli_str_cpy(new_command->func_reuse->value_str, ___VALUE_STR(func_reuse));
            (void)vtss_icli_str_cpy(new_command->func_reuse->value_str + vtss_icli_str_len(___VALUE_STR(func_reuse)), _DEFAULT_FORM_POSTFIX);
        }
        // cmd_var
        ___tag_ALLOC(new_command->cmd_var, 1);
        new_command->cmd_var->next = g_current_cmd->cmd_var;
        // runtime
        ___tag_ALLOC(new_command->runtime, 1);
        new_command->runtime->next = g_current_cmd->runtime;
        // byword
        ___tag_ALLOC(new_command->byword, 1);
        new_command->byword->next = g_current_cmd->byword;
        // help
        len = vtss_icli_str_len(_NO_FORM_HELP) + 1;
        ___tag_ALLOC(new_command->help, len);
        (void)vtss_icli_str_cpy(new_command->help->value_str, _DEFAULT_FORM_HELP);
        new_command->help->next = g_current_cmd->help;
        // property
        new_command->property = g_current_cmd->property;
        // variable
        new_command->variable = g_current_cmd->default_form_variable;
        // code
        new_command->code = g_current_cmd->default_form_code;
        // next
        new_command->next = NULL;

        // link to g_current_cmd
        if ( g_current_cmd->next ) {
            g_current_cmd->next->next = new_command;
        } else {
            g_current_cmd->next = new_command;
        }

        /* update dscp cnt */
        for ( i = 0; i < new_command->dscp_word_cnt; ++i ) {
            ++( new_command->dscp_word_id[i] );
        }

        // increase the number of commands
        ++( g_command_number );
    }

    return ICLI_RC_OK;
}

/*
    reuse the original command syntax by linking pointer
*/
static i32 _multiple_command_mode_add( void )
{
    _tag_element_t  *t;
    _command_t      *new_command;
    _command_t      *cmd;
    _command_t      *next_cmd;
    u32             n;
    i32             len;

    g_current_cmd = g_command;
    while ( g_current_cmd != NULL ) {
        // get next command
        next_cmd = g_current_cmd->next;

        cmd = g_current_cmd;
        n = 1;
        for ( t = g_current_cmd->cmd_mode->next; t != NULL; ___NEXT(t), ++n ) {
            // reset new_command
            new_command = NULL;

            // allocate memory
            ___STRUCT_MALLOC(new_command, _command_t, FALSE);

            // copy current command and reuse as possible
            *new_command = *cmd;

            // func_name, if exist then make it unique
            if ___CMD_WITH_VALUE_STR( func_name ) {
                len = vtss_icli_str_len(___VALUE_STR(func_name)) + 8;
                ___tag_ALLOC(new_command->func_name, len);
                (void)vtss_icli_str_cpy(new_command->func_name->value_str, ___VALUE_STR(func_name));
                icli_sprintf(new_command->func_name->value_str + vtss_icli_str_len(___VALUE_STR(func_name)), "_%u", n);
            }

            // cmd_mode
            new_command->cmd_mode = t;

            // get command mode information
            new_command->mode_info = icli_cmd_mode_info_get_by_str( new_command->cmd_mode->value_str );
            if ( new_command->mode_info == NULL ) {
                __U_E("invalid command mode: %s\n", new_command->cmd_mode->value_str);
                return ICLI_RC_ERROR;
            }

            // get mode word count
            if ( new_command->mode_info->b_mode_var ) {
                new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
                if ( new_command->mode_word_cnt < 0 ) {
                    T_E("fail to get command word count\n");
                    return ICLI_RC_ERROR;
                }
            } else {
                new_command->mode_word_cnt = 0;
            }

            // link to g_command
            // not need to link new_command->next as it is already reused
            cmd->next = new_command;
            cmd = new_command;

            // increase the number of commands
            ++( g_command_number );
        }

        // set current command
        g_current_cmd = next_cmd;
    }
    return ICLI_RC_OK;
}

/*
    automatically generate sub mode entry commands
*/
static i32 _sub_mode_entry_add( void )
{
    icli_cmd_mode_t             sub_mode;
    const icli_cmd_mode_info_t  *mode_info;
    u32                         n;
    BOOL                        b_new;
    _command_t                  *new_command;
    i32                         len;

    n = 0;
    g_line_number = 0;
    for ( sub_mode = ICLI_CMD_MODE_CONFIG_VLAN; sub_mode < ICLI_CMD_MODE_MAX; ++sub_mode ) {
        // reset new_command and allocate memory
        new_command = NULL;
        ___STRUCT_MALLOC(new_command, _command_t, FALSE);

        // copy current command and reuse as possible
        *new_command = *g_current_cmd;

        // null mode_sub
        new_command->mode_sub = NULL;

        // func_name, if exist then make it unique
        if ___CMD_WITH_VALUE_STR( func_name ) {
            len = vtss_icli_str_len(___VALUE_STR(func_name)) + 8;
            ___tag_ALLOC(new_command->func_name, len);
            icli_sprintf( new_command->func_name->value_str, "%s_%u", ___VALUE_STR(func_name), ++n );
        }

        // property
        b_new = FALSE;
        memset( g_value_str, 0, sizeof(g_value_str) );
        if ___CMD_WITH_VALUE_STR( property ) {
            if ( vtss_icli_str_str("ICLI_CMD_PROP_INVISIBLE", ___VALUE_STR(property)) == NULL ) {
                // property existing, concat invisible
                icli_sprintf( g_value_str, "%s | %s", ___VALUE_STR(property), "ICLI_CMD_PROP_INVISIBLE" );
                b_new = TRUE;
            }
        } else {
            // no property, create property with invisible
            vtss_icli_str_cpy( g_value_str, "ICLI_CMD_PROP_INVISIBLE" );
            b_new = TRUE;
        }
        if ( b_new ) {
            new_command->property = NULL;
            ___VALUE_STR_MALLOC_AND_GET( new_command->property, FALSE );
        }

        // get mode information
        mode_info = icli_cmd_mode_info_get_by_mode( sub_mode );
        if ( mode_info == NULL ) {
            __U_E("invalid command mode: %u\n", sub_mode);
            return ICLI_RC_ERROR;
        }

        // set sub mode
        new_command->cmd_mode = NULL;
        vtss_icli_str_cpy( g_value_str, mode_info->str );
        ___VALUE_STR_MALLOC_AND_GET( new_command->cmd_mode, FALSE );

        // get command mode information
        new_command->mode_info = mode_info;

        // get mode word count
        if ( new_command->mode_info->b_mode_var ) {
            new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
            if ( new_command->mode_word_cnt < 0 ) {
                T_E("fail to get command word count\n");
                return ICLI_RC_ERROR;
            }
        } else {
            new_command->mode_word_cnt = 0;
        }

        // link to g_current_cmd
        // not need to link new_command->next as it is already reused
        g_current_cmd->next = new_command;

        // increase the number of commands
        ++( g_command_number );
    }
    return ICLI_RC_OK;
}

/*
    automatically generate sub mode commands
*/
static i32 _sub_mode_def_cmd_add( void )
{
    _command_t                  *new_command;
    _command_t                  *command;
    i32                         len;

    static u32                  n = 0;

    /*
        do <line>
    */
    command = g_command;

    // reset new_command and allocate memory
    new_command = NULL;
    ___STRUCT_MALLOC( new_command, _command_t, FALSE );

    // copy current command and reuse as possible
    *new_command = *command;

    // next
    new_command->next = NULL;

    // func_reuse
    new_command->func_reuse = new_command->func_name;

    // func_name, make it unique
    len = vtss_icli_str_len( new_command->func_name->value_str ) + 8;
    ___tag_ALLOC( new_command->func_name, len );
    icli_sprintf( new_command->func_name->value_str, "%s_%u", new_command->func_reuse->value_str, ++n );

    // cmd_mode
    new_command->cmd_mode = g_current_cmd->mode_goto;

    // get command mode information
    new_command->mode_info = icli_cmd_mode_info_get_by_str( new_command->cmd_mode->value_str );
    if ( new_command->mode_info == NULL ) {
        __U_E("invalid command mode: %s\n", new_command->cmd_mode->value_str);
        return ICLI_RC_ERROR;
    }

    if ( new_command->mode_info->b_mode_var ) {
        new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
        if ( new_command->mode_word_cnt < 0 ) {
            T_E("fail to get command word count\n");
            return ICLI_RC_ERROR;
        }
    } else {
        new_command->mode_word_cnt = 0;
    }

    // add to tail
    ___ADD_TO_TAIL(_command_t, g_command, new_command);

    // increase the number of commands
    ++( g_command_number );

    /*
        end
    */
    ___NEXT( command );

    // reset new_command and allocate memory
    new_command = NULL;
    ___STRUCT_MALLOC( new_command, _command_t, FALSE );

    // copy current command and reuse as possible
    *new_command = *command;

    // next
    new_command->next = NULL;

    // func_reuse
    new_command->func_reuse = new_command->func_name;

    // func_name, make it unique
    len = vtss_icli_str_len( new_command->func_name->value_str ) + 8;
    ___tag_ALLOC( new_command->func_name, len );
    icli_sprintf( new_command->func_name->value_str, "%s_%u", new_command->func_reuse->value_str, ++n );

    // cmd_mode
    new_command->cmd_mode = g_current_cmd->mode_goto;

    // get command mode information
    new_command->mode_info = icli_cmd_mode_info_get_by_str( new_command->cmd_mode->value_str );
    if ( new_command->mode_info == NULL ) {
        __U_E("invalid command mode: %s\n", new_command->cmd_mode->value_str);
        return ICLI_RC_ERROR;
    }

    if ( new_command->mode_info->b_mode_var ) {
        new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
        if ( new_command->mode_word_cnt < 0 ) {
            T_E("fail to get command word count\n");
            return ICLI_RC_ERROR;
        }
    } else {
        new_command->mode_word_cnt = 0;
    }

    // add to tail
    ___ADD_TO_TAIL(_command_t, g_command, new_command);

    // increase the number of commands
    ++( g_command_number );

    /*
        exit
    */
    ___NEXT( command );

    // reset new_command and allocate memory
    new_command = NULL;
    ___STRUCT_MALLOC( new_command, _command_t, FALSE );

    // copy current command and reuse as possible
    *new_command = *command;

    // next
    new_command->next = NULL;

    // func_reuse
    new_command->func_reuse = new_command->func_name;

    // func_name, make it unique
    len = vtss_icli_str_len( new_command->func_name->value_str ) + 8;
    ___tag_ALLOC( new_command->func_name, len );
    icli_sprintf( new_command->func_name->value_str, "%s_%u", new_command->func_reuse->value_str, ++n );

    // cmd_mode
    new_command->mode_goto = new_command->cmd_mode;
    new_command->cmd_mode  = g_current_cmd->mode_goto;

    // get command mode information
    new_command->mode_info = icli_cmd_mode_info_get_by_str( new_command->cmd_mode->value_str );
    if ( new_command->mode_info == NULL ) {
        __U_E("invalid command mode: %s\n", new_command->cmd_mode->value_str);
        return ICLI_RC_ERROR;
    }

    if ( new_command->mode_info->b_mode_var ) {
        new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
        if ( new_command->mode_word_cnt < 0 ) {
            T_E("fail to get command word count\n");
            return ICLI_RC_ERROR;
        }
    } else {
        new_command->mode_word_cnt = 0;
    }

    // add to tail
    ___ADD_TO_TAIL(_command_t, g_command, new_command);

    // increase the number of commands
    ++( g_command_number );

    /*
        help
    */
    ___NEXT( command );

    // reset new_command and allocate memory
    new_command = NULL;
    ___STRUCT_MALLOC( new_command, _command_t, FALSE );

    // copy current command and reuse as possible
    *new_command = *command;

    // next
    new_command->next = NULL;

    // func_reuse
    new_command->func_reuse = new_command->func_name;

    // func_name, make it unique
    len = vtss_icli_str_len( new_command->func_name->value_str ) + 8;
    ___tag_ALLOC( new_command->func_name, len );
    icli_sprintf( new_command->func_name->value_str, "%s_%u", new_command->func_reuse->value_str, ++n );

    // cmd_mode
    new_command->mode_goto = new_command->cmd_mode;
    new_command->cmd_mode  = g_current_cmd->mode_goto;

    // get command mode information
    new_command->mode_info = icli_cmd_mode_info_get_by_str( new_command->cmd_mode->value_str );
    if ( new_command->mode_info == NULL ) {
        __U_E("invalid command mode: %s\n", new_command->cmd_mode->value_str);
        return ICLI_RC_ERROR;
    }

    if ( new_command->mode_info->b_mode_var ) {
        new_command->mode_word_cnt = vtss_icli_word_count( (char *)(new_command->mode_info->cmd) );
        if ( new_command->mode_word_cnt < 0 ) {
            T_E("fail to get command word count\n");
            return ICLI_RC_ERROR;
        }
    } else {
        new_command->mode_word_cnt = 0;
    }

    // add to tail
    ___ADD_TO_TAIL(_command_t, g_command, new_command);

    // increase the number of commands
    ++( g_command_number );

    return ICLI_RC_OK;
}

/*
    automatically generate sub mode commands
*/
static i32 _sub_mode_gen( void )
{
    _command_t      *next_cmd;
    i32             rc;

    g_current_cmd = g_command;
    while ( g_current_cmd ) {
        // get next command
        next_cmd = g_current_cmd->next;

        // check if sub mode
        if ( ___CMD_NO_VALUE_STR(mode_sub) || vtss_icli_str_cmp(___VALUE_STR(mode_sub), "1") ) {
            // next current command
            g_current_cmd = next_cmd;
            continue;
        }

        // generate sub mode entry commands
        rc = _sub_mode_entry_add();
        if ( rc != ICLI_RC_OK ) {
            return rc;
        }

        // generate default commands in sub mode
        rc = _sub_mode_def_cmd_add();
        if ( rc != ICLI_RC_OK ) {
            return rc;
        }

        // next current command
        g_current_cmd = next_cmd;
    }
    return ICLI_RC_OK;
}

#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
/*
    _first_word_get() is copied from vtss_icli_parsing.c
*/
#define _WORD_TYPE_GET(t) \
    if ( ICLI_IS_(t, (*c)) ) {\
        if ( ICLI_IS_(EOS,*(c+1)) ) {\
            *cmd_str = c + 1;\
        } else {\
            *(c+1) = ICLI_EOS;\
            *cmd_str = c + 2;\
        }\
        *w = c;\
        return ICLI_WORD_TYPE_##t;\
    }

/*
    get the first word in cmd_str

    input -
        cmd_str : command string
    output -
        cmd_str : point to the second word
        w       : the first word
    return -
        icli_word_type_t
    comment -
        n/a
*/
static i32  _first_word_get(
    INOUT   char                **cmd_str,
    OUT     char                **w
)
{
    char    *c;

    //skip space
    ICLI_SPACE_SKIP(c, (*cmd_str));

    //check type
    if ( ICLI_IS_KEYWORD(*c) ) {
        *w = c;
        for ( ; ICLI_IS_KEYWORD(*c); ++c ) {
            ;
        }
        if ( ICLI_IS_(SPACE, *(c)) ) {
            *c++ = ICLI_EOS;
        }
        *cmd_str = c;
        return ICLI_WORD_TYPE_KEYWORD;
    }

    if ( ICLI_IS_(VARIABLE_BEGIN, (*c)) ) {
        *w = c;
        for ( ++c; ICLI_NOT_(VARIABLE_END, *(c)) && ICLI_NOT_(EOS, *(c)); ++c ) {
            ;
        }
        if ( ICLI_NOT_(VARIABLE_END, *(c)) ) {
            return ICLI_WORD_TYPE_INVALID;
        }
        if ( ICLI_IS_(EOS, *(c + 1)) ) {
            *cmd_str = c + 1;
        } else {
            *(c + 1) = ICLI_EOS;
            *cmd_str = c + 2;
        }
        return ICLI_WORD_TYPE_VARIABLE;
    }

    if ( ICLI_IS_(MANDATORY_END, (*c)) ) {
        if ( ICLI_IS_(EOS,*(c+1)) ) {
            *cmd_str = c + 1;
        } else if ( ICLI_IS_RANDOM_MUST(c) ) {
            *(c+3) = ICLI_EOS;
            *cmd_str = c + 4;
        } else {
            *(c+1) = ICLI_EOS;
            *cmd_str = c + 2;
        }
        *w = c;
        return ICLI_WORD_TYPE_MANDATORY_END;
    }

    _WORD_TYPE_GET( MANDATORY_BEGIN );
    _WORD_TYPE_GET( LOOP_BEGIN );
    _WORD_TYPE_GET( LOOP_END );
    _WORD_TYPE_GET( OPTIONAL_BEGIN );
    _WORD_TYPE_GET( OPTIONAL_END );
    _WORD_TYPE_GET( OR );

    if ( ICLI_IS_(EOS, (*c)) ) {
        return ICLI_WORD_TYPE_EOS;
    }

    return ICLI_WORD_TYPE_INVALID;
}

/*
    replace variables in original command with CMD_VAR
*/
static i32 _var_cmd_replace(
    _command_t      *cmd
)
{
    char                *c;
    char                *w;
    char                *d;
    _tag_element_t      *cmd_var;
    i32                 t;

    memset( g_value_str, 0, sizeof(g_value_str) );

    // remove grep if exist
    c = cmd->command->value_str;
    if ( cmd->grep ) {
        (void)vtss_icli_str_ncpy( g_value_str, c, vtss_icli_str_len(c) - vtss_icli_str_len(___GREP_CMD_STR) );
    } else {
        (void)vtss_icli_str_cpy( g_value_str, c );
    }

    w = g_value_str;
    cmd_var = cmd->cmd_var;
    d = g_line_buf;

    for ( t = _first_word_get(&w, &c); t != ICLI_WORD_TYPE_EOS; t = _first_word_get(&w, &c) ) {
        switch ( t ) {
        case ICLI_WORD_TYPE_VARIABLE:
            if ( cmd_var && vtss_icli_str_len(cmd_var->value_str) ) {
                icli_sprintf( d, "<%s> ", cmd_var->value_str );
            } else {
                icli_sprintf( d, "%s ", c );
            }
            if ( cmd_var ) {
                ___NEXT( cmd_var );
            }
            break;

        case ICLI_WORD_TYPE_KEYWORD:
            if ( cmd_var ) {
                ___NEXT( cmd_var );
            }

            // pass through

        default:
            icli_sprintf( d, "%s ", c );
            break;

        case ICLI_WORD_TYPE_INVALID:
            T_E("ICLI_WORD_TYPE_INVALID - %s\n", cmd->original_cmd);
            return ICLI_RC_ERROR;
        }

        // next word
        d += vtss_icli_str_len( d );
    }
    return ICLI_RC_OK;
}

/*
    generate var_cmd
*/
static i32 _var_cmd_gen( void )
{
    _command_t      *cmd;
    u32             len;

    for ( cmd = g_command; cmd; ___NEXT(cmd) ) {
        // clean line buffer
        memset( g_line_buf, 0, sizeof(g_line_buf) );

        // replace variable with CMD_VAR and write into g_line_buf
        if ( _var_cmd_replace(cmd) != ICLI_RC_OK ) {
            return ICLI_RC_ERROR;
        }

        // create var_cmd and copy from g_line_buf
        len = vtss_icli_str_len( g_line_buf ) + 1;
        cmd->var_cmd = (char *)icli_malloc( len );
        memset( cmd->var_cmd, 0, len );
        (void)vtss_icli_str_cpy( cmd->var_cmd, g_line_buf );
    }
    return ICLI_RC_OK;
}
#endif

/*
    RETURN:
        icli_rc_t
*/
static i32 _command_parsing( void )
{
    i32     s = _SEGMENT_BEGIN;
    i32     r;
    u32     u;
    char    *c;

    ___STRUCT_MALLOC(g_current_cmd, _command_t, FALSE);

    // parsing line by line
    while (_line_read() == ICLI_RC_OK) {
        switch ( _str_tag_and_value_get() ) {
        case ICLI_RC_OK:

            switch (s) {
            case _SEGMENT_BEGIN:
                switch (_tag_get()) {
                case _TAG_COMMAND:
                case _TAG_DOC_CMD_DESC:
                case _TAG_DOC_CMD_DEFAULT:
                case _TAG_DOC_CMD_USAGE:
                case _TAG_DOC_CMD_EXAMPLE:
                case _TAG_FUNC_NAME:
                case _TAG_FUNC_REUSE:
                case _TAG_PRIVILEGE:
                case _TAG_CMD_MODE:
                case _TAG_GOTO_MODE:
                case _TAG_DESTROY_MODE:
                case _TAG_SUB_MODE:
                case _TAG_USR_OPT:
                case _TAG_IF_FLAG:
                case _TAG_MODE_VAR:
                case _TAG_CMD_VAR:
                case _TAG_RUNTIME:
                case _TAG_BYWORD:
                case _TAG_HELP:
                case _TAG_PROPERTY:
                case _TAG_STR_DEF:
                case _TAG_NO_FORM_DOC_CMD_DESC:
                case _TAG_NO_FORM_DOC_CMD_DEFAULT:
                case _TAG_NO_FORM_DOC_CMD_USAGE:
                case _TAG_NO_FORM_DOC_CMD_EXAMPLE:
                case _TAG_DEFAULT_FORM_DOC_CMD_DESC:
                case _TAG_DEFAULT_FORM_DOC_CMD_DEFAULT:
                case _TAG_DEFAULT_FORM_DOC_CMD_USAGE:
                case _TAG_DEFAULT_FORM_DOC_CMD_EXAMPLE:
                    if ( _value_str_get() != ICLI_RC_OK ) {
                        T_E("_value_str_get()\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_VARIABLE_BEGIN:
                    r = _section_parsing(_TAG_VARIABLE_BEGIN, _TAG_VARIABLE_END, &(g_current_cmd->variable), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( VARIABLE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_CODE_BEGIN:
                    r = _section_parsing(_TAG_CODE_BEGIN, _TAG_CODE_END, &(g_current_cmd->code), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( CODE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_NO_FORM_VARIABLE_BEGIN:
                    r = _section_parsing(_TAG_NO_FORM_VARIABLE_BEGIN, _TAG_NO_FORM_VARIABLE_END, &(g_current_cmd->no_form_variable), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( NO_FORM_VARIABLE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_NO_FORM_CODE_BEGIN:
                    r = _section_parsing(_TAG_NO_FORM_CODE_BEGIN, _TAG_NO_FORM_CODE_END, &(g_current_cmd->no_form_code), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( NO_FORM_CODE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_DEFAULT_FORM_VARIABLE_BEGIN:
                    r = _section_parsing(_TAG_DEFAULT_FORM_VARIABLE_BEGIN, _TAG_DEFAULT_FORM_VARIABLE_END, &(g_current_cmd->default_form_variable), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( DEFAULT_FORM_VARIABLE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_DEFAULT_FORM_CODE_BEGIN:
                    r = _section_parsing(_TAG_DEFAULT_FORM_CODE_BEGIN, _TAG_DEFAULT_FORM_CODE_END, &(g_current_cmd->default_form_code), s);
                    if (r != ICLI_RC_OK) {
                        T_E("_section_parsing( DEFAULT_FORM_CODE )\n");
                        return ICLI_RC_ERROR;
                    }
                    break;

                case _TAG_CMD_END:
                    // check necessary items
                    ___CMD_EXIST_CHECK_RETURN(command);
                    ___CMD_EXIST_CHECK_RETURN(privilege);
                    ___CMD_EXIST_CHECK_RETURN(cmd_mode);

                    // if function name not defined, create one
                    if ___CMD_NO_VALUE_STR( func_name ) {
                        ___tag_ALLOC(g_current_cmd->func_name, vtss_icli_str_len(g_script_file) + 32);
                        // make sure the function name can not include '-' when minus vlaue
                        u = (icli_current_time_get() + rand()) & 0x7FFFffff;
                        icli_sprintf(___VALUE_STR(func_name), "_%s_icli_%u_%08u", g_script_file, g_func_name_number++, u);
                    }

                    // store original command first
                    // but this need to be formatted
                    r = vtss_icli_str_len( ___VALUE_STR(command) ) * 2 + 1;
                    g_current_cmd->original_cmd = (char *)icli_malloc(r);
                    if ( g_current_cmd->original_cmd == NULL ) {
                        T_E("icli_malloc()\n");
                        return ICLI_RC_ERROR;
                    }
                    memset(g_current_cmd->original_cmd, 0, r);
                    if ( _cmd_formated(___VALUE_STR(command), g_current_cmd->original_cmd) != ICLI_RC_OK ) {
                        return ICLI_RC_ERROR;
                    }

                    // copy back for formatted
                    r = vtss_icli_str_len(g_current_cmd->original_cmd) + 1;
                    c = (char *)icli_malloc(r);
                    if ( c == NULL ) {
                        T_E("icli_malloc()\n");
                        return ICLI_RC_ERROR;
                    }
                    memset(c, 0, r);
                    (void)vtss_icli_str_cpy(c, g_current_cmd->original_cmd);
                    icli_free( ___VALUE_STR(command) );
                    ___VALUE_STR(command) = c;

                    // process repeat "..."
                    r = _repeat_process();
                    if ( r != ICLI_RC_OK ) {
                        T_E("_repeat_process()\n");
                        return ICLI_RC_ERROR;
                    }

#if 1 /* CP, 2012/09/05 17:42, <port_type_id> */
                    // process port type id
                    r = _port_type_id_process();
                    if ( r != ICLI_RC_OK ) {
                        T_E("_port_type_id_list_process()\n");
                        return ICLI_RC_ERROR;
                    }
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                    // process port type id
                    r = _port_type_list_process();
                    if ( r != ICLI_RC_OK ) {
                        T_E("_port_type_list_process()\n");
                        return ICLI_RC_ERROR;
                    }
#endif

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
                    // process dscp
                    r = _dscp_process();
                    if ( r != ICLI_RC_OK ) {
                        T_E("_dscp_process()\n");
                        return ICLI_RC_ERROR;
                    }
#endif

                    // check if GREP, then append the command
                    g_current_cmd->grep = 0;
                    if ___CMD_WITH_VALUE_STR( property ) {
                        if (vtss_icli_str_str("ICLI_CMD_PROP_GREP", g_current_cmd->property->value_str) != NULL) {
                            g_current_cmd->grep = 1;
                            // realloc memory for grep string
                            c = (char *)icli_malloc(vtss_icli_str_len(___VALUE_STR(command)) + vtss_icli_str_len(___GREP_CMD_STR) + 1);
                            if ( c == NULL ) {
                                T_E("memory insufficient\n");
                                return ICLI_RC_ERROR;
                            }
                            // copy original cmd string
                            (void)vtss_icli_str_cpy(c, ___VALUE_STR(command));
                            // free original string
                            icli_free( ___VALUE_STR(command) );
                            // use new string
                            ___VALUE_STR(command) = c;
                            // concat grep string
                            if ( vtss_icli_str_concat(___VALUE_STR(command), ___GREP_CMD_STR) == NULL ) {
                                T_E("Fail to concatenate GREP string into command, %s\n",
                                    ___VALUE_STR(command));
                                return ICLI_RC_ERROR;
                            }
                        }
                    }

                    /* get number of words in the command */
                    g_current_cmd->command_word_cnt = vtss_icli_word_count( ___VALUE_STR(command) );
                    if ( g_current_cmd->command_word_cnt < 0 ) {
                        T_E("fail to get command word count\n");
                        return ICLI_RC_ERROR;
                    }

                    if ( g_current_cmd->command_word_cnt > _WORD_MAX_CNT ) {
                        __U_E("too many words %d > MAX_CNT(%d)\n",
                              g_current_cmd->command_word_cnt, _WORD_MAX_CNT);
                        return ICLI_RC_ERROR;
                    }

                    /* get command mode information */
                    g_current_cmd->mode_info = icli_cmd_mode_info_get_by_str( g_current_cmd->cmd_mode->value_str );
                    if ( g_current_cmd->mode_info == NULL ) {
                        __U_E("invalid command mode: %s\n", g_current_cmd->cmd_mode->value_str);
                        return ICLI_RC_ERROR;
                    }

                    if ( g_current_cmd->mode_info->b_mode_var ) {
                        g_current_cmd->mode_word_cnt = vtss_icli_word_count( (char *)(g_current_cmd->mode_info->cmd) );
                        if ( g_current_cmd->mode_word_cnt < 0 ) {
                            T_E("fail to get command word count\n");
                            return ICLI_RC_ERROR;
                        }
                    } else {
                        g_current_cmd->mode_word_cnt = 0;
                    }

#if 1 /* CP, 2012/09/17 15:47, add ICLI_CMD_PROP_LOOSELY to no command */
                    g_current_cmd->b_no_command = FALSE;
#endif

                    /* add into command list */
                    if ( g_command == NULL ) {
                        g_command = g_current_cmd;
                    } else {
                        ___ADD_TO_TAIL(_command_t, g_command, g_current_cmd);
                    }

                    /* increase the number of commands */
                    ++( g_command_number );

                    /* add no and default form */
                    r = _no_and_default_form_add();
                    if ( r != ICLI_RC_OK ) {
                        T_E("_no_and_default_form_add()\n");
                        return ICLI_RC_ERROR;
                    }

                    /* reset to get next command */
                    g_current_cmd = NULL;

                    return ICLI_RC_OK;

                default:
                    T_E("invalid tag - %s at line %d\n", g_tag_str, g_line_number);
                    return ICLI_RC_ERROR;
                }
                break;

            default:
                T_E("invalid s = %d\n", s);
                return ICLI_RC_ERROR;
            }
            break;

        case ICLI_RC_ERR_MORE:

            switch (_tag_get()) {
            case _TAG_COMMAND:
            case _TAG_DOC_CMD_DESC:
            case _TAG_DOC_CMD_DEFAULT:
            case _TAG_DOC_CMD_USAGE:
            case _TAG_DOC_CMD_EXAMPLE:
            case _TAG_FUNC_NAME:
            case _TAG_FUNC_REUSE:
            case _TAG_PRIVILEGE:
            case _TAG_CMD_MODE:
            case _TAG_GOTO_MODE:
            case _TAG_DESTROY_MODE:
            case _TAG_SUB_MODE:
            case _TAG_USR_OPT:
            case _TAG_IF_FLAG:
            case _TAG_MODE_VAR:
            case _TAG_CMD_VAR:
            case _TAG_RUNTIME:
            case _TAG_BYWORD:
            case _TAG_HELP:
            case _TAG_PROPERTY:
            case _TAG_STR_DEF:
            case _TAG_NO_FORM_DOC_CMD_DESC:
            case _TAG_NO_FORM_DOC_CMD_DEFAULT:
            case _TAG_NO_FORM_DOC_CMD_USAGE:
            case _TAG_NO_FORM_DOC_CMD_EXAMPLE:
            case _TAG_DEFAULT_FORM_DOC_CMD_DESC:
            case _TAG_DEFAULT_FORM_DOC_CMD_DEFAULT:
            case _TAG_DEFAULT_FORM_DOC_CMD_USAGE:
            case _TAG_DEFAULT_FORM_DOC_CMD_EXAMPLE:
                ___MORE_GET();
                if ( _value_str_get() != ICLI_RC_OK ) {
                    return ICLI_RC_ERROR;
                }
                break;

            default:
                T_E("invalid tag - %s at line %d\n", g_tag_str, g_line_number);
                return ICLI_RC_ERROR;
            }
            break;

        case ICLI_RC_ERR_EMPTY:
            break;

        default:
            T_E("fail to _str_tag_and_value_get()\n");
            return ICLI_RC_ERROR;
        }
    }
    T_E("should not be here\n");
    return ICLI_RC_ERROR;
}

/*
    INPUT:
        value_str - value string
    RETURN:
        1 - valid defined constant string
        0 - general string
       -1 - invalid string not defined
*/
static i32  _const_str_check(
    IN char     *value_str
)
{
    i32                     r;
    char                    *s;
    _tag_value_element_t    *tv;

    r = vtss_icli_str_sub(_CONST_STR_PREFIX, value_str, 1, NULL);
    switch ( r ) {
    case 1:
        // skip space
        ICLI_SPACE_SKIP(s, &(value_str[vtss_icli_str_len(_CONST_STR_PREFIX)]));
        for ( tv = g_str_def; tv != NULL; ___NEXT(tv) ) {
            if ( vtss_icli_str_cmp(s, tv->tag) == 0 ) {
                return 1;
            }
        }
        return -1;

    case 0:
    case -1:
    default:
        return 0;
    }
}

static char     header_string[] =
    "/*\n"
    "\n"
    " Vitesse Switch Application software.\n"
    "\n"
    " Copyright (c) 2002-2013 Vitesse Semiconductor Corporation \"Vitesse\". All\n"
    " Rights Reserved.\n"
    "\n"
    " Unpublished rights reserved under the copyright laws of the United States of\n"
    " America, other countries and international treaties. Permission to use, copy,\n"
    " store and modify, the software and its source code is granted. Permission to\n"
    " integrate into other products, disclose, transmit and distribute the software\n"
    " in an absolute machine readable format (e.g. HEX file) is also granted.  The\n"
    " source code of the software may not be disclosed, transmitted or distributed\n"
    " without the written permission of Vitesse. The software and its source code\n"
    " may only be used in products utilizing the Vitesse switch products.\n"
    "\n"
    " This copyright notice must appear in any copy, modification, disclosure,\n"
    " transmission or distribution of the software. Vitesse retains all ownership,\n"
    " copyright, trade secret and proprietary rights in the software.\n"
    "\n"
    " THIS SOFTWARE HAS BEEN PROVIDED \"AS IS,\" WITHOUT EXPRESS OR IMPLIED WARRANTY\n"
    " INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS\n"
    " FOR A PARTICULAR USE AND NON-INFRINGEMENT.\n"
    "\n"
    "*/\n"
    "\n"
    "/*\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "\n"
    "        This file is automatically generated from script file.\n"
    "        Please modify the corresponding script file if needed.\n"
    "        Modifying this file will -NOT- take effect.\n"
    "\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "*/\n"
    "\n";

static char include_begin_string[] =
    "#include <stdio.h>\n"
    "#include <string.h>\n"
    "#include \"icli_api.h\"\n"
    "\n"
    "//===== INCLUDE_BEGIN ========================================================\n"
    "/*lint --e{766} */\n";

static char include_end_string[] =
    "//===== INCLUDE_END ==========================================================\n"
    "\n"
    "#include \"icli_porting_trace.h\"\n"
    "\n";

static char str_def_begin_string[] =
    "//===== CONSTANT_STRING_DEFINITION_BEGIN =====================================\n";

static char str_def_end_string[] =
    "//===== CONSTANT_STRING_DEFINITION_END =======================================\n"
    "\n";

static char command_alias_help_string[] =
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    < auto-generation >\n"
    "    strings of command, alias and help for each command\n"
    "\n"
    "******************************************************************************\n"
    "*/\n";

static char function_begin_string[] =
    "//===== FUNCTION_BEGIN =======================================================\n";

static char function_end_string[] =
    "//===== FUNCTION_END =========================================================\n"
    "\n";

static char cmd_begin_string[] =
    "\n"
    "//----- Command -----\n"
    "static char %s_cmd[] =\n"
    "    \"";

static char cmd_end_string[] =
    " \";\n";

static char byword_string[] =
    "static icli_byword_t %s_byword[] = {\n";

static char help_string[] =
    "static char *%s_help[] = {\n";

static char range_string[] =
    "static icli_range_t %s_range_%d = {\n";

static char runtime_string[] =
    "static icli_runtime_cb_t *%s_runtime[] = {\n";

static char property_begin_string[] =
    "static icli_cmd_property_t %s_property = {\n"
    "    0, // command ID, assigned when register\n";

static char property_end_string[] =
    "    %s, // privilege\n"
    "    %s, // orig_priv\n"
    "    %s, // mode goto\n"
    "    %s, // mode destroy\n"
    "    %s_byword,\n"
    "    %s_help,\n"
    "    %s_runtime\n"
    "};\n";

static char execution_begin_string[] =
    "static icli_cmd_execution_t %s_execution = {\n"
    "    0, // command ID, assigned when register\n"
    "    %s, // command callback\n";

static char execution_end_string[] =
    "};\n";

static char cmd_nodes_begin_string[] =
    "static icli_parsing_node_t %s_node[] = {\n";

static char cmd_node_string[] =
    "    {\n"
    "        %d, // word id\n"
    "        NULL, // word, re-use command string buffer\n"
    "        %s, // type\n"
    "        NULL, // range\n"
    "        { %d, &%s_property, 0 }, // node property\n"
    "        NULL, // execution, assigned when register\n"
    "        0, // optional_begin\n"
    "        0, // optional_end\n"
    "        0, // parent_cnt\n"
    "        NULL, // parent\n"
    "        NULL, // child\n"
    "        NULL, // sibling\n"
    "        NULL, // loop\n"
#if ICLI_RANDOM_OPTIONAL
    "        {NULL, NULL, NULL}, // random_optional\n"
    "        {0,    0,    0}, // random_optional_level\n"
#endif
#if ICLI_RANDOM_MUST_NUMBER
    "        NULL,  // must_head\n"
    "        0,     // must_number\n"
    "        0,     // must_level\n"
    "        FALSE, // must_begin\n"
    "        FALSE, // must_end\n"
#endif
    "    }%s";

static char cmd_node_range_string[] =
    "    {\n"
    "        %d, // word id\n"
    "        NULL, // word, re-use command string buffer\n"
    "        %s, // type\n"
    "        &%s_range_%d, // range\n"
    "        {%d, &%s_property, 0}, // node property\n"
    "        NULL, // execution, assigned when register\n"
    "        0, // optional_begin\n"
    "        0, // optional_end\n"
    "        0, // parent_cnt\n"
    "        NULL, // parent\n"
    "        NULL, // child\n"
    "        NULL, // sibling\n"
    "        NULL, // loop\n"
#if ICLI_RANDOM_OPTIONAL
    "        {NULL, NULL, NULL}, // random_optional\n"
    "        {0,    0,    0}, // random_optional_level\n"
#if ICLI_RANDOM_MUST_NUMBER
    "        NULL,  // must_head\n"
    "        0,     // must_number\n"
    "        0,     // must_level\n"
    "        FALSE, // must_begin\n"
    "        FALSE, // must_end\n"
#endif // ICLI_RANDOM_MUST_NUMBER
#endif // ICLI_RANDOM_OPTIONAL

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    "        0,     // or_head\n"
#endif
    "    }%s";

static char cmd_nodes_end_string[] =
    "};\n";

static char register_begin_string[] =
    "static icli_cmd_register_t  %s_register = {\n"
    "    %s_cmd,\n"
    "    \"%s\", // original_cmd\n"
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    "    \"%s\", // var_cmd\n"
#endif
    "    %s_node,\n"
    "    %d,\n"
    "    &%s_property,\n"
    "    &%s_execution,\n";

static char register_end_string[] =
    "    %s // command mode\n"
    "};\n";

static char cb_comment_string[] =
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    < auto-generation >\n"
    "    callbacks for each command\n"
    "\n"
    "******************************************************************************\n"
    "*/\n";

static char cb_header_string[] =
    "static i32 %s(\n"
    "    u32                 session_id,\n"
    "    icli_parameter_t    *mode_var,\n"
    "    icli_parameter_t    *cmd_var,\n"
    "    i32                 usr_opt\n"
    ")";

static char cb_end_string[] =
    "    // successful\n"
    "    return ICLI_RC_OK;\n"
    "}\n";

static char cb_warning_avoid_string[] =
    "    // avoid compile warning\n"
    "    if ( session_id ){}\n"
    "    if ( mode_var ){}\n"
    "    if ( cmd_var ){}\n"
    "    if ( usr_opt ){}\n";

static char cb_disable_lint_string[] =
    "    //Disable lint's warning about use of possibly-NULL pointers:\n"
    "    /*lint --e{613} */\n";

static char var_init_begin_string[] =
    "//===== VARIABLE_BEGIN ======\n";

static char var_init_end_string[] =
    "//===== VARIABLE_END ========\n"
    "\n";

static char mode_begin_string[] =
    "    // MODE command\n"
    "    // %s\n"
    "    while ( mode_var ) {\n"
    "        switch ( mode_var->word_id ) {\n";

static char mode_end_string[] =
    "        default: /* unknown variable */\n"
    "            return ICLI_RC_ERROR;\n"
    "        }\n"
    "        mode_var = mode_var->next;\n"
    "    }\n"
    "\n";

static char cmd_var_begin_string[] =
    "    // COMMAND\n"
    "    // %s\n"
    "    while ( cmd_var ) {\n"
    "        switch ( cmd_var->word_id ) {\n";

static char cmd_var_case_string[] =
    "        case  %d: /* %s */\n";

static char cmd_var_case2_string[] =
    "        case  %d: /* <%s> */\n";

static char cmd_var_break_string[] =
    "            break;\n";

static char cmd_var_end_string[] =
    "        default: /* unknown parameter */\n"
    "            return ICLI_RC_ERROR;\n"
    "        }\n"
    "        cmd_var = cmd_var->next;\n"
    "    }\n"
    "\n";

static char code_begin_string[] =
    "//===== CODE_BEGIN ==========\n";

static char code_end_string[] =
    "//===== CODE_END ============\n"
    "\n";

static char cmd_register_header_string[] =
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    < auto-generation >\n"
    "    global API for command registration\n"
    "    return value -\n"
    "        ICLI_RC_OK    : successful\n"
    "        ICLI_RC_ERROR : failed\n"
    "\n"
    "******************************************************************************\n"
    "*/\n"
    "i32 %s_icli_cmd_register(void)%s\n";

static char cmd_register_begin_string[] =
    "{\n"
    "    i32     cmd_id = 0, cmd_seq = 0;\n"
    "\n"
    "    // to avoid warning for ths case of un-used\n"
    "    if ( cmd_id | cmd_seq ){}\n"
    "\n"
    "    // init database of command id\n"
    "    memset(g_cmd_id, 0xff, sizeof(g_cmd_id));\n"
    "\n";

static char cmd_register_string[] =
    "    /*\n"
    "        %s\n"
    "    */\n"
    "    cmd_seq = %d;\n"
    "    cmd_id = icli_cmd_register( &%s_register );\n"
    "    if ( cmd_id >= 0 ) {\n"
    "        // update command id\n"
    "        g_cmd_id[cmd_seq] = cmd_id;\n"
    "    } else {\n"
    "        T_E(\"register command \\\"%s\\\"\\n\");\n"
    "    }\n";

static char cmd_register_end_string[] =
    "    return ICLI_RC_OK;\n"
    "}\n"
    "\n";

static char cmd_id_api_header_string[] =
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    < auto-generation >\n"
    "    global API for command ID retrieval\n"
    "    return value -\n"
    "        >= 0 : valid command ID\n"
    "        < 0  : failed, cmd_seq is out of boundary or cmd not registered\n"
    "\n"
    "******************************************************************************\n"
    "*/\n"
    "i32 %s_icli_cmd_id_get(\n"
    "    IN i32 cmd_seq\n"
    ")%s\n";

static char cmd_id_api_body_string[] =
    "{\n"
    "    return (cmd_seq < ___ICLI_CMD_CNT) ? g_cmd_id[cmd_seq] : ICLI_RC_ERR_PARAMETER;\n"
    "}\n"
    "\n";

#define ___STR_LEN(t)       vtss_icli_str_len(t->value_str)
#define ___STR_GET(t)       (t)?t->value_str:""

#define ___LINE_GEN(p) \
    if ( g_line_directive ) {\
        fprintf(g_fd, "#line %d \"%s\"\n", p->line, g_script_path);\
    }

#define ___SEGMENT_GEN(g, b, e) \
{\
    _segment_t    *s;\
    fputs(b, g_fd);             \
    for ( s=g; s!=NULL; ___NEXT(s) ) {\
        if ( s->line > 0 ) {\
            ___LINE_GEN( s );\
        }\
        for (t=s->element; t!=NULL; ___NEXT(t)) {\
            fprintf(g_fd, "%s\n", ___STR_GET(t));\
        }\
    }\
    fputs(e, g_fd);             \
}

#define ___HELP_GEN(s)\
    fprintf(g_fd, s, g_current_cmd->func_name->value_str);\
    for ( i=0, t=g_current_cmd->help; i<n; ++i ) {\
        if ( g_current_cmd->grep && i >= (n - ___GREP_WORD_CNT) ) {\
            switch (i - (n - ___GREP_WORD_CNT)) {\
            case 0:\
                fprintf(g_fd, "    \"%s\"%s\n", ___GREP_HELP, (i<n-1)?",":"");\
                break;\
            case 1:\
                fprintf(g_fd, "    \"%s\"%s\n", ___GREP_BEGIN_HELP, (i<n-1)?",":"");\
                break;\
            case 2:\
                fprintf(g_fd, "    \"%s\"%s\n", ___GREP_INCLUDE_HELP, (i<n-1)?",":"");\
                break;\
            case 3:\
                fprintf(g_fd, "    \"%s\"%s\n", ___GREP_EXCLUDE_HELP, (i<n-1)?",":"");\
                break;\
            case 4:\
                fprintf(g_fd, "    \"%s\"%s\n", ___GREP_STRING_HELP, (i<n-1)?",":"");\
                break;\
            default:\
                break;\
            }\
        } else {\
            if ( t == NULL ) {\
                fprintf(g_fd, "    \"\"%s\n", (i<n-1)?",":"");\
            } else {\
                r = _const_str_check( t->value_str );\
                switch( r ) {\
                    case 1:\
                        ICLI_SPACE_SKIP(cs, &(t->value_str[vtss_icli_str_len(_CONST_STR_PREFIX)]));\
                        fprintf(g_fd, "    %s%s\n", cs, (i<n-1)?",":"");\
                        break;\
                    case 0:\
                        fprintf(g_fd, "    \"%s\"%s\n", t->value_str, (i<n-1)?",":"");\
                        break;\
                    case -1:\
                    default:\
                        printf("at line %d, constant string not defined, %s\n", t->line, t->value_str);\
                        return ICLI_RC_ERROR;\
                }\
            }\
        }\
        if ( t != NULL ) {\
            ___NEXT(t);\
        }\
    }\
    fprintf(g_fd, "};\n");

static BOOL _byword_gen(
    IN  i32             k,
    IN  _tag_element_t  *byword,
    IN  i32             n
)
{
    _tag_element_t      *t;
    i32                 i;
    _byword_constant_t  *bw;

    fprintf(g_fd, byword_string, g_current_cmd->func_name->value_str);
    for ( i = 0, t = byword; i < n; ++i ) {
        if ( g_current_cmd->grep && i == (n - ___GREP_WORD_CNT) ) {
            fprintf(g_fd, "    {\"%s\", 0, {0, 0}},\n", ICLI_GREP_KEYWORD);
            fprintf(g_fd, "    {\"%s\", 0, {0, 0}},\n", ICLI_GREP_BEGIN_KEYWORD);
            fprintf(g_fd, "    {\"%s\", 0, {0, 0}},\n", ICLI_GREP_INCLUDE_KEYWORD);
            fprintf(g_fd, "    {\"%s\", 0, {0, 0}},\n", ICLI_GREP_EXCLUDE_KEYWORD);
            fprintf(g_fd, "    {\"LINE\", 0, {0, 0}}\n");
            break;
        } else {
            switch ( g_variable_type[k][i] ) {
            case ICLI_VARIABLE_RANGE_INT:
            case ICLI_VARIABLE_RANGE_INT_LIST:
                if ( g_variable_range[k][i] ) {
                    fprintf(g_fd, "    {\"%s\", 0, {0, 0}}%s\n", ___STR_GET(t), (i < n - 1) ? "," : "");
                } else {
                    bw = &(g_variable_byword[k][i]);
                    if ( bw->word == NULL ) {
                        __U_E("g_variable_byword[%d][%d].word == NULL\n", k, i);
                        return FALSE;
                    }
                    fprintf(g_fd, "    {\"%s\", %c, {%s, %s}}%s\n",
                            bw->word,
                            (char)('0' + bw->para_cnt),
                            (bw->para_cnt > 0) ? bw->para[0] : "0",
                            (bw->para_cnt > 1) ? bw->para[1] : "0",
                            (i < n - 1) ? "," : "");
                }
                break;

            default:
                fprintf(g_fd, "    {\"%s\", 0, {0, 0}}%s\n", ___STR_GET(t), (i < n - 1) ? "," : "");
                break;
            }
        }
        if ( t != NULL ) {
            ___NEXT(t);
        }
    }
    fprintf(g_fd, "};\n");
    return TRUE;
}

#define ___CMD_ID_DB_GEN()\
    fprintf(g_fd, "/* nummber of commands */\n");\
    fprintf(g_fd, "#define ___ICLI_CMD_CNT     %d\n", g_command_number);\
    fprintf(g_fd, "\n");\
    fprintf(g_fd, "/* database of command ID */\n");\
    fprintf(g_fd, "static i32  g_cmd_id[___ICLI_CMD_CNT];\n");\
    fprintf(g_fd, "\n");

#define ___COUNT_WORD()\
    ++j;\
    if ( j >= _WORD_MAX_CNT ) {\
        __U_E("number of words overflow\n");\
        return ICLI_RC_ERR_OVERFLOW;\
    }\
    i = 0;\
    cmd_word[j] = (char *)icli_malloc(len + 1);\
    if ( cmd_word[j] == NULL ) {\
        T_E("fail to allocate memory for cmd_word[%d]\n", j);\
        return ICLI_RC_ERR_MEMORY;\
    }

#define __ASSIGN_WORD_TYPE()\
    cmd_type[j] = ICLI_VARIABLE_KEYWORD;\
    cmd_word[j][i] = ICLI_EOS;

/*
    check syntax and format the string with a single space
    also get each word and its variable type or range correspondingly
*/
static i32  _word_type_get(
    IN  char                    *cmd,
    IN  BOOL                    b_format,
    OUT char                    **cmd_word,
    OUT icli_variable_type_t    *cmd_type,
    OUT icli_range_t            **cmd_range
)
{
    i32                     i;
    i32                     j;
    i32                     len;
    char                    *c;
    icli_word_state_t       w_state;
    icli_range_t            *range;
    BOOL                    not_over;
    i32                     rc;
    char                    *s;

    if ( ! ICLI_IS_KEYWORD(*cmd) ) {
        __U_E("The first char should be a keyword\n");
        return ICLI_RC_ERROR;
    }
    len = vtss_icli_str_len( cmd );

    /* get memory */
    range = (icli_range_t *)icli_malloc(sizeof(icli_range_t));
    if ( range == NULL ) {
        T_E("memory insufficient\n");
        return ICLI_RC_ERR_MEMORY;
    }
    memset(range, 0, sizeof(icli_range_t));

    //
    //  check syntax and format the string with a single space
    //  i: char count for current word
    //  j: word count
    //
    _stack_init();
    w_state = ICLI_WORD_STATE_SPACE;
    j = -1;
    for ( c = cmd, not_over = TRUE; not_over; ++c ) {
        switch ( w_state ) {
        case ICLI_WORD_STATE_WORD:
            if ( ICLI_IS_KEYWORD(*c) ) {
                w_state = ICLI_WORD_STATE_WORD;
                if ( b_format ) {
                    fprintf(g_fd, "%c", *c);
                }
                if ( i >= len ) {
                    __U_E("variable name is too long\n");
                    return ICLI_RC_ERR_OVERFLOW;
                }
                cmd_word[j][i++] = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                __ASSIGN_WORD_TYPE();
                not_over = FALSE;
                continue;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                __ASSIGN_WORD_TYPE();
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                w_state = ICLI_WORD_STATE_VARIABLE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                ___COUNT_WORD();
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                if ( _stack_push(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                if ( _stack_pop(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
#if ICLI_RANDOM_MUST_NUMBER
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        fprintf(g_fd, "%c", *(++c));
                        fprintf(g_fd, "%c", *(++c));
                    }
#endif
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                if ( _stack_push(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                if ( _stack_pop(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                if ( _stack_push(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                if ( _stack_pop(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
            if ( ICLI_IS_(OR, *c) ) {
                if ( _stack_is_empty() ) {
                    break;
                }
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                __ASSIGN_WORD_TYPE();
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                if ( b_format && ICLI_NOT_(SQUOTE, *c) ) {
                    fprintf(g_fd, "%c", *c);
                }
                if ( i >= len ) {
                    __U_E("variable name is too long\n");
                    return ICLI_RC_ERR_OVERFLOW;
                }
                cmd_word[j][i++] = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                not_over = FALSE;
                break;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                break;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                w_state = ICLI_WORD_STATE_SPACE;
                if ( b_format ) {
                    fprintf(g_fd, "%c", *c);
                }

                //get variable name
                cmd_word[j][i] = ICLI_EOS;

                //get variable type
                memset(range, 0, sizeof(icli_range_t));
                rc = vtss_icli_variable_type_get( cmd_word[j], cmd_type + j, range );
                switch ( rc ) {
                case ICLI_RC_OK:
                    if ( vtss_icli_variable_data_has_range_get(cmd_type[j]) ) {
                        cmd_range[j] = (icli_range_t *)icli_malloc(sizeof(icli_range_t));
                        if ( cmd_range[j] == NULL ) {
                            T_E("fail to allocate memory for cmd_range[%d]\n", j);
                            return ICLI_RC_ERR_MEMORY;
                        }
                        *(cmd_range[j]) = *range;
                    }
                    break;

                case ICLI_RC_ERR_RANGE:
                    __U_E("invalid range <%s>\n", cmd_word[j]);
                    return ICLI_RC_ERROR;

                default:
                    // predict ID or List
                    cmd_type[j] = ICLI_VARIABLE_RANGE_INT;
                    for ( s = cmd_word[j]; ICLI_NOT_(EOS, *s); s++ ) {
                        if ( ICLI_IS_(TILDE, *s) ) {
                            cmd_type[j] = ICLI_VARIABLE_RANGE_INT_LIST;
                        }
                    }
                    break;
                }
                continue;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                break;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                break;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                break;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(OR, *c) ) {
                break;
            }
            break;

        case ICLI_WORD_STATE_SPACE:
            if ( ICLI_IS_KEYWORD(*c) ) {
                w_state = ICLI_WORD_STATE_WORD;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                ___COUNT_WORD();
                cmd_word[j][i++] = *c;
                continue;
            }
            if ( ICLI_IS_(EOS, *c) ) {
                not_over = FALSE;
                continue;
            }
            if ( ICLI_IS_(SPACE, *c) ) {
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, *c) ) {
                w_state = ICLI_WORD_STATE_VARIABLE;
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                ___COUNT_WORD();
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, *c) ) {
                break;
            }
            if ( ICLI_IS_(MANDATORY_BEGIN, *c) ) {
                if ( _stack_push(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
            if ( ICLI_IS_(MANDATORY_END, *c) ) {
                if ( _stack_pop(ICLI_MANDATORY_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
#if ICLI_RANDOM_MUST_NUMBER
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        fprintf(g_fd, "%c", *(++c));
                        fprintf(g_fd, "%c", *(++c));
                    }
#endif
                }
                continue;
            }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            if ( ICLI_IS_(LOOP_BEGIN, *c) ) {
                if ( _stack_push(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
            if ( ICLI_IS_(LOOP_END, *c) ) {
                if ( _stack_pop(ICLI_LOOP_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
#endif
            if ( ICLI_IS_(OPTIONAL_BEGIN, *c) ) {
                if ( _stack_push(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
            if ( ICLI_IS_(OPTIONAL_END, *c) ) {
                if ( _stack_pop(ICLI_OPTIONAL_BEGIN) != ICLI_RC_OK ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
            if ( ICLI_IS_(OR, *c) ) {
                if ( _stack_is_empty() ) {
                    break;
                }
                if ( b_format ) {
                    fprintf(g_fd, " %c", *c);
                }
                continue;
            }
            break;

        default:
            T_E("invalid state %u for command %s\n",
                w_state, cmd);
            return ICLI_RC_ERROR;
        }
        __U_E("invalid syntax commad = %s\n", cmd);
        return ICLI_RC_ERROR;
    }
    if ( _stack_is_empty() == FALSE ) {
        __U_E("invalid syntax at end for command = %s\n", cmd);
        _stack_pop(0);
        return ICLI_RC_ERROR;
    }
    return ICLI_RC_OK;
}

#define ___GET_VARIABLE(x,y) \
    case x:\
        fprintf(g_fd, "%s = cmd_var->value.u."#y";\n", t->value_str);\
        break;

#define ___GET_VARIABLE_PTR(x,y) \
    case x:\
        fprintf(g_fd, "%s = &( cmd_var->value.u."#y" );\n", t->value_str);\
        break;

#define ___GET_PARAMETER(x,y) \
    case x:\
        fprintf(g_fd, "%s = mode_var->value.u."#y";\n", t->value_str);\
        break;

#define ___GET_PARAMETER_PTR(x,y) \
    case x:\
        fprintf(g_fd, "%s = &( mode_var->value.u."#y" );\n", t->value_str);\
        break;

#define ___GET_ALL(_what_) \
    ___GET_##_what_( ICLI_VARIABLE_MAC_ADDR,               u_mac_addr         ); \
    ___GET_##_what_( ICLI_VARIABLE_MAC_UCAST,              u_mac_ucast        ); \
    ___GET_##_what_( ICLI_VARIABLE_MAC_MCAST,              u_mac_mcast        ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_ADDR,              u_ipv4_addr        ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_NETMASK,           u_ipv4_netmask     ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_WILDCARD,          u_ipv4_wildcard    ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_UCAST,             u_ipv4_ucast       ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_MCAST,             u_ipv4_mcast       ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_NMCAST,            u_ipv4_nmcast      ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_ABC,               u_ipv4_abc         ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_SUBNET,            u_ipv4_subnet      ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV4_PREFIX,            u_ipv4_prefix      ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_ADDR,              u_ipv6_addr        ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_NETMASK,           u_ipv6_netmask     ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_UCAST,             u_ipv6_ucast       ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_MCAST,             u_ipv6_mcast       ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_LINKLOCAL,         u_ipv6_linklocal   ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_SUBNET,            u_ipv6_subnet      ); \
    ___GET_##_what_( ICLI_VARIABLE_IPV6_PREFIX,            u_ipv6_prefix      ); \
    ___GET_##_what_( ICLI_VARIABLE_INT8,                   u_int8             ); \
    ___GET_##_what_( ICLI_VARIABLE_INT16,                  u_int16            ); \
    ___GET_##_what_( ICLI_VARIABLE_INT,                    u_int              ); \
    ___GET_##_what_( ICLI_VARIABLE_UINT8,                  u_uint8            ); \
    ___GET_##_what_( ICLI_VARIABLE_UINT16,                 u_uint16           ); \
    ___GET_##_what_( ICLI_VARIABLE_UINT,                   u_uint             ); \
    ___GET_##_what_( ICLI_VARIABLE_PORT_ID,                u_port_id          ); \
    ___GET_##_what_( ICLI_VARIABLE_VLAN_ID,                u_vlan_id          ); \
    ___GET_##_what_( ICLI_VARIABLE_DATE,                   u_date             ); \
    ___GET_##_what_( ICLI_VARIABLE_TIME,                   u_time             ); \
    ___GET_##_what_( ICLI_VARIABLE_HHMM,                   u_hhmm             ); \
    ___GET_##_what_( ICLI_VARIABLE_WORD,                   u_word             ); \
    ___GET_##_what_( ICLI_VARIABLE_KWORD,                  u_kword            ); \
    ___GET_##_what_( ICLI_VARIABLE_CWORD,                  u_cword            ); \
    ___GET_##_what_( ICLI_VARIABLE_DWORD,                  u_dword            ); \
    ___GET_##_what_( ICLI_VARIABLE_FWORD,                  u_fword            ); \
    ___GET_##_what_( ICLI_VARIABLE_STRING,                 u_string           ); \
    ___GET_##_what_( ICLI_VARIABLE_LINE,                   u_line             ); \
    ___GET_##_what_( ICLI_VARIABLE_PORT_TYPE,              u_port_type        ); \
    ___GET_##_what_( ICLI_VARIABLE_PORT_TYPE_ID,           u_port_type_id     ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_INT,              u_range_int        ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_UINT,             u_range_uint       ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_WORD,             u_range_word       ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_KWORD,            u_range_kword      ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_DWORD,            u_range_dword      ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_FWORD,            u_range_fword      ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_STRING,           u_range_string     ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_LINE,             u_range_line       ); \
    ___GET_##_what_( ICLI_VARIABLE_OUI,                    u_oui              ); \
    ___GET_##_what_( ICLI_VARIABLE_DPL,                    u_dpl              ); \
    ___GET_##_what_( ICLI_VARIABLE_HOSTNAME,               u_hostname         ); \
    ___GET_##_what_( ICLI_VARIABLE_CLOCK_ID,               u_clock_id         ); \
    ___GET_##_what_( ICLI_VARIABLE_VCAP_VR,                u_vcap_vr          ); \
    ___GET_##_what_( ICLI_VARIABLE_HEXVAL,                 u_hexval           ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_HEXVAL,           u_range_hexval     ); \
    ___GET_##_what_( ICLI_VARIABLE_VWORD,                  u_vword            ); \
    ___GET_##_what_( ICLI_VARIABLE_RANGE_VWORD,            u_range_vword      ); \
    ___GET_##_what_( ICLI_VARIABLE_SWITCH_ID,              u_switch_id        ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_PORT_LIST,        u_port_list        ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_VLAN_LIST,        u_vlan_list        ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_RANGE_LIST,       u_range_list       ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_RANGE_INT_LIST,   u_range_int_list   ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_RANGE_UINT_LIST,  u_range_uint_list  ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_PORT_TYPE_LIST,   u_port_type_list   ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_PCP,              u_pcp              ); \
    ___GET_##_what_##_PTR( ICLI_VARIABLE_SWITCH_LIST,      u_switch_list      );

static char *_help_str_get(
    IN  _tag_element_t  *t
)
{
    i32                     r;
    char                    *s;
    _tag_value_element_t    *tv;

    r = vtss_icli_str_sub(_CONST_STR_PREFIX, t->value_str, 1, NULL);
    switch ( r ) {
    case 1:
        // skip space
        ICLI_SPACE_SKIP(s, &(t->value_str[vtss_icli_str_len(_CONST_STR_PREFIX)]));
        for ( tv = g_str_def; tv != NULL; ___NEXT(tv) ) {
            if ( vtss_icli_str_cmp(s, tv->tag) == 0 ) {
                return tv->value_str;
            }
        }
        T_E("constant str \"%s\" is not defined\n", s);
        return "";

    case 0:
    case -1:
    default:
        return t->value_str;
    }
}

static BOOL _range_constant_get(
    IN  char                *word,
    IN  char                delimiter,
    OUT _range_constant_t   *rconst,
    OUT _byword_constant_t  *byword
)
{
    char    *c = word;
    BOOL    b_min;
    char    *start;
    u32     cnt;
    i32     len;
    char    *s;
    i32     val;
    char    *w;
    i32     min;
    BOOL    b_deli;

    if ( ICLI_IS_(COMMA, *c) || ICLI_IS_(TILDE, *c) ) {
        __U_E("syntax error\n");
        return FALSE;
    }

    w = (char *)icli_malloc(ICLI_BYWORD_MAX_LEN + 1);
    if ( w == NULL ) {
        T_E("Fail to icli_malloc()\n");
        return FALSE;
    }
    memset(w, 0, ICLI_BYWORD_MAX_LEN + 1);
    byword->word = w;

    cnt    = 0;
    b_min  = TRUE;
    b_deli = FALSE;
    start  = c;
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(COMMA, *c) ) {
            len = c - start;
            if ( len == 0 ) {
                __U_E("Syntax error\n");
                return FALSE;
            }

            s = (char *)icli_malloc(len + 1);
            if ( s == NULL ) {
                T_E("Fail to icli_malloc()\n");
                return FALSE;
            }
            memset(s, 0, len + 1);
            (void)vtss_icli_str_ncpy(s, start, len);

            if ( vtss_icli_str_to_int(s, &val) == ICLI_RC_OK ) {
                if ( b_deli && val <= min ) {
                    __U_E("invalid range, %d-%d\n", min, val);
                    return FALSE;
                }
                (void)vtss_icli_str_ncpy(w, s, len);
                w += len;
            } else {
                // constant should be enclosed by ' '
                if ( ICLI_NOT_(SQUOTE, *s) || ICLI_NOT_(SQUOTE, *(s + len - 1))) {
                    __U_E("The constant is not enclosed by single quote\n");
                    return FALSE;
                }
                // remove ' '
                *(s + len - 1) = 0;
                s = s + 1;

                if ( byword->para_cnt >= ICLI_BYWORD_PARA_MAX_CNT ) {
                    T_E("Byword para cnt is full\n");
                    return FALSE;
                }
                *w++ = '%';
                *w++ = 'l';
                *w++ = 'd';

                byword->para[byword->para_cnt] = s;
                ++( byword->para_cnt );
            }
            *w++ = ICLI_COMMA;

            rconst->range[cnt].max = s;
            if ( b_min ) {
                rconst->range[cnt].min = s;
            }

            ++cnt;
            start  = c + 1;
            b_min  = TRUE;
            b_deli = FALSE;
        } else if ( (*c) == delimiter ) {
            len = c - start;
            if ( len == 0 ) {
                if ( ICLI_IS_(DASH, *c) ) {
                    // minus sign, not for delimiter
                    continue;
                } else {
                    __U_E("syntax error\n");
                    return FALSE;
                }
            }

            if ( len == 1 ) {
                if ( ICLI_IS_(DASH, *start) ) {
                    __U_E("syntax error\n");
                    return FALSE;
                }
            }

            if ( b_min == FALSE ) {
                __U_E("syntax error\n");
                return FALSE;
            }

            s = (char *)icli_malloc(len + 1);
            if ( s == NULL ) {
                T_E("fail to icli_malloc()\n");
                return FALSE;
            }
            memset(s, 0, len + 1);
            (void)vtss_icli_str_ncpy(s, start, len);

            if ( vtss_icli_str_to_int(s, &val) == ICLI_RC_OK ) {
                (void)vtss_icli_str_ncpy(w, s, len);
                w += len;
                min = val;
                b_deli = TRUE;
            } else {
                // constant should be enclosed by ' '
                if ( ICLI_NOT_(SQUOTE, *s) || ICLI_NOT_(SQUOTE, *(s + len - 1))) {
                    __U_E("The constant is not enclosed by single quote\n");
                    return FALSE;
                }
                // remove ' '
                *(s + len - 1) = 0;
                s = s + 1;

                if ( byword->para_cnt >= ICLI_BYWORD_PARA_MAX_CNT ) {
                    T_E("Byword para cnt is full\n");
                    return FALSE;
                }
                *w++ = '%';
                *w++ = 'l';
                *w++ = 'd';

                byword->para[byword->para_cnt] = s;
                ++( byword->para_cnt );
                b_deli = FALSE;
            }
            *w++ = delimiter;
            rconst->range[cnt].min = s;

            start = c + 1;
            b_min = FALSE;
        }
    }

    len = c - start;
    if ( len == 0 ) {
        __U_E("syntax error\n");
        return FALSE;
    }

    s = (char *)icli_malloc(len + 1);
    if ( s == NULL ) {
        T_E("fail to icli_malloc()\n");
        return FALSE;
    }
    memset(s, 0, len + 1);
    (void)vtss_icli_str_ncpy(s, start, len);

    if ( vtss_icli_str_to_int(s, &val) == ICLI_RC_OK ) {
        if ( b_deli && val <= min ) {
            __U_E("invalid range, %d-%d\n", min, val);
            return FALSE;
        }
        (void)vtss_icli_str_ncpy(w, s, len);
        w += len;
    } else {
        // constant should be enclosed by ' '
        if ( ICLI_NOT_(SQUOTE, *s) || ICLI_NOT_(SQUOTE, *(s + len - 1))) {
            __U_E("The constant is not enclosed by single quote\n");
            return FALSE;
        }
        // remove ' '
        *(s + len - 1) = 0;
        s = s + 1;

        if ( byword->para_cnt >= ICLI_BYWORD_PARA_MAX_CNT ) {
            T_E("Byword para cnt is full\n");
            return FALSE;
        }
        *w++ = '%';
        *w++ = 'l';
        *w++ = 'd';

        byword->para[ byword->para_cnt ] = s;
        ++( byword->para_cnt );
    }

    rconst->range[cnt].max = s;
    if ( b_min ) {
        rconst->range[cnt].min = s;
    }

    // get count
    rconst->cnt = cnt + 1;

    // successful
    return TRUE;
}

static i32 _generate_C_and_H( void )
{
    _tag_element_t          *t;
    i32                     n,
                            i,
                            k,
                            r,
                            len,
                            mode_word_cnt,
                            mode_var_cnt,
                            cmd_word_cnt,
                            cmd_var_cnt;
    u32                     j;
    char                    *func_name;
    icli_signed_range_t     *sr;
#if 0 /* CP, 2012/09/10 13:19, always write int to avoid warning because the first initializer is sr */
    icli_unsigned_range_t   *ur;
#endif
    icli_privilege_t        priv;
    _tag_value_element_t    *tv;
    char                    *cs;
    BOOL                    b_pointer_var = FALSE;
    _range_constant_t       rconst;

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
    BOOL                    b_dscp;
    u32                     dscp_id;
    icli_dscp_wvh_t         *dscp_wvh = vtss_icli_variable_dscp_wvh_get();
#endif

    // get C and H file name
    if ( g_generate_path ) {
        len = vtss_icli_str_len(g_generate_path) + vtss_icli_str_len(g_script_file) + 16;

        g_icli_c_file = icli_malloc(len);
        if ( g_icli_c_file == NULL ) {
            T_E("fail to allocate memory for g_icli_c_file\n");
            return ICLI_RC_ERROR;
        }

        g_icli_h_file = icli_malloc(len);
        if ( g_icli_h_file == NULL ) {
            T_E("fail to allocate memory for g_icli_h_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_icli_c_file, "%s/%s_icli.c", g_generate_path, g_script_file);
        icli_sprintf(g_icli_h_file, "%s/%s_icli.h", g_generate_path, g_script_file);
    } else {
        len = vtss_icli_str_len(g_script_file) + 16;

        g_icli_c_file = icli_malloc(len);
        if ( g_icli_c_file == NULL ) {
            T_E("fail to allocate memory for g_icli_c_file\n");
            return ICLI_RC_ERROR;
        }

        g_icli_h_file = icli_malloc(len);
        if ( g_icli_h_file == NULL ) {
            T_E("fail to allocate memory for g_icli_h_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_icli_c_file, "%s_icli.c", g_script_file);
        icli_sprintf(g_icli_h_file, "%s_icli.h", g_script_file);
    }

    //===========================================
    //
    //  C file
    //
    //===========================================

    // create
    g_fd = fopen( g_icli_c_file, "w" );
    if ( g_fd == NULL ) {
        T_E("create file of %s\n", g_icli_c_file);
        return ICLI_RC_ERROR;
    }

    /*
        generate header
    */
    fputs(header_string, g_fd);

    /*
        generate INCLUDE_BEGIN
    */
    ___SEGMENT_GEN(g_include, include_begin_string, include_end_string);

    /*
        generate CONSTANT STRING DEFINITION
    */
    fprintf(g_fd, "%s\n", str_def_begin_string);
    for ( tv = g_str_def; tv != NULL; ___NEXT(tv) ) {
        fprintf(g_fd, "#define %s    \"%s\"\n", tv->tag, tv->value_str);
    }
    fprintf(g_fd, "%s\n", str_def_end_string);

    /*
        module IF flag
    */
    if ( ___MODULE_WITH_VALUE_STR(module_if_flag) ) {
        fprintf(g_fd, "//===== MODULE_IF_FLAG =======================================================\n");
        fprintf(g_fd, "#if %s\n", g_module->module_if_flag->value_str);
        fprintf(g_fd, "\n");
    }

    /*
        generate FUNCTION_BEGIN
    */
    ___SEGMENT_GEN(g_function, function_begin_string, function_end_string);

    /*
        < auto-generation >
        static memory for command string, alias, help ...
    */
    fputs(command_alias_help_string, g_fd);
    for ( g_current_cmd = g_command, k = 0; g_current_cmd != NULL; ___NEXT(g_current_cmd), ++k ) {

        if ( k >= _CMD_MAX_CNT ) {
            T_E("too many commands\n");
            return ICLI_RC_ERROR;
        }

        // number of words
        n = g_current_cmd->command_word_cnt;

        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#if %s\n", ___VALUE_STR(if_flag));
        }

        // get API name
        func_name = ___VALUE_STR(func_name);

        /* cmd string */
        fprintf(g_fd, cmd_begin_string, func_name);

        //
        //  check syntax and format the string with a single space
        //
        r = _word_type_get( ___VALUE_STR(command),
                            TRUE,
                            g_variable_name[k],
                            g_variable_type[k],
                            g_variable_range[k] );

        if ( r != ICLI_RC_OK ) {
            __U_E("invalid cmd = %s\n", ___VALUE_STR(command));
            return r;
        }

        fprintf(g_fd, cmd_end_string, func_name);

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
        if ( g_current_cmd->dscp_word_cnt ) {
            for ( i = 0; i < g_current_cmd->dscp_word_cnt; ++i ) {
                for ( j = 0; j < ICLI_DSCP_MAX_CNT; ++j ) {
                    g_variable_type[k][g_current_cmd->dscp_word_id[i] + j] = ICLI_VARIABLE_DSCP;
                }
            }
        }
#endif

        // RUNTIME
        fprintf(g_fd, runtime_string, func_name);
        for ( i = 0, t = g_current_cmd->runtime; i < n; ++i ) {
            if ( t && vtss_icli_str_len(t->value_str) ) {
                ___LINE_GEN( t );
                fprintf(g_fd, "    %s", t->value_str);
            } else {
                switch ( g_variable_type[k][i] ) {
                case ICLI_VARIABLE_CWORD:
                    __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                    __U_E("<cword> must have a corresponding runtime callback\n");
                    return ICLI_RC_ERROR;
                case ICLI_VARIABLE_VCAP_VR:
                    __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                    __U_E("<vcap_vr> must have a corresponding runtime callback\n");
                    return ICLI_RC_ERROR;
                default:
                    break;
                }
                fprintf(g_fd, "    NULL");
            }
            fprintf(g_fd, "%s\n", (i < n - 1) ? "," : "");
            if ( t != NULL ) {
                ___NEXT(t);
            }
        }
        fprintf(g_fd, "};\n");

        // RANGE
        for ( i = 0; i < n; ++i ) {
            if ( g_variable_range[k][i] == NULL ) {
                // reset
                memset(&rconst, 0, sizeof(rconst));

                switch ( g_variable_type[k][i] ) {
                case ICLI_VARIABLE_RANGE_INT:
                    if ( _range_constant_get(g_variable_name[k][i], ICLI_DASH, &rconst, &(g_variable_byword[k][i])) == FALSE ) {
                        __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                        __U_E("Invalid variable type <%s>\n", g_variable_name[k][i]);
                        return ICLI_RC_ERROR;
                    }
                    break;

                case ICLI_VARIABLE_RANGE_INT_LIST:
                    if ( _range_constant_get(g_variable_name[k][i], ICLI_TILDE, &rconst, &(g_variable_byword[k][i])) == FALSE ) {
                        __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                        __U_E("Invalid variable type <%s>\n", g_variable_name[k][i]);
                        return ICLI_RC_ERROR;
                    }
                    break;

                default:
                    continue;
                }

                // range BEGIN
                fprintf(g_fd, range_string, func_name, i);

                // type
                fprintf(g_fd, "    ICLI_RANGE_TYPE_SIGNED,\n");

                // count
                fprintf(g_fd, "    {{   %u", rconst.cnt);

                if ( rconst.cnt ) {
                    fprintf(g_fd, ",\n");
                    fprintf(g_fd, "        {\n");
                    // print each range
                    for ( j = 0; j < rconst.cnt; ++j ) {
                        fprintf(g_fd, "            { %s, %s }",
                                rconst.range[j].min, rconst.range[j].max );
                        if ( j < rconst.cnt - 1 ) {
                            fprintf(g_fd, ",\n");
                        } else {
                            fprintf(g_fd, "\n");
                        }
                    }
                    fprintf(g_fd, "        }\n");
                    fprintf(g_fd, "    }}\n");
                } else {
                    fprintf(g_fd, " }}\n");
                }

                // range END
                fprintf(g_fd, "};\n");

                // next range
                continue;
            }

            // range BEGIN
            fprintf(g_fd, range_string, func_name, i);

            // type
            fprintf(g_fd, "    %s,\n",
                    (g_variable_range[k][i]->type == ICLI_RANGE_TYPE_UNSIGNED) ?
                    "ICLI_RANGE_TYPE_UNSIGNED" : "ICLI_RANGE_TYPE_SIGNED");

            sr = &(g_variable_range[k][i]->u.sr);

            // count
            fprintf(g_fd, "    {{   %u", sr->cnt);

            if ( sr->cnt ) {
                fprintf(g_fd, ",\n");
                fprintf(g_fd, "        {\n");

                // print each range
                for ( j = 0; j < sr->cnt; ++j ) {
                    if ( sr->range[j].min == ICLI_MIN_INT ) {
                        fprintf(g_fd, "            { (-1-2147483647), %d }",
                                sr->range[j].max );
                    } else {
                        fprintf(g_fd, "            { %d, %d }",
                                sr->range[j].min, sr->range[j].max );
                    }
                    if ( j < sr->cnt - 1 ) {
                        fprintf(g_fd, ",\n");
                    } else {
                        fprintf(g_fd, "\n");
                    }
                }
                fprintf(g_fd, "        }\n");
                fprintf(g_fd, "    }}\n");
            } else {
                fprintf(g_fd, " }}\n");
            }

            // range END
            fprintf(g_fd, "};\n");
        }

        // BYWORD
        if ( _byword_gen(k, g_current_cmd->byword, n) == FALSE ) {
            __U_E("COMMAND = %s\n", ___VALUE_STR(command));
            __U_E("Fail to call _byword_gen()\n");
            return ICLI_RC_ERROR;
        }

        // HELP
        ___HELP_GEN( help_string );

        // Command Property
        fprintf(g_fd, property_begin_string,
                func_name);

#if 1 /* CP, 2012/09/17 15:47, add ICLI_CMD_PROP_LOOSELY to no command */
        if ( g_current_cmd->b_no_command ) {
            if ___CMD_NO_VALUE_STR( property ) {
                fprintf(g_fd, "    ICLI_CMD_PROP_LOOSELY, // command property\n");
            } else {
                ___LINE_GEN( g_current_cmd->property );
                fprintf(g_fd, "    %s | ICLI_CMD_PROP_LOOSELY, // command property\n", ___VALUE_STR(property));
            }
        } else {
            if ___CMD_NO_VALUE_STR( property ) {
                fprintf(g_fd, "    ICLI_CMD_PROP_ENABLE|ICLI_CMD_PROP_VISIBLE, // command property\n");
            } else {
                ___LINE_GEN( g_current_cmd->property );
                fprintf(g_fd, "    %s, // command property\n", ___VALUE_STR(property));
            }
        }
#endif

        ___LINE_GEN( g_current_cmd->privilege );

        priv = icli_privilege_get_by_str( ___VALUE_STR(privilege));
        if ( priv == ICLI_PRIVILEGE_MAX ) {
            __U_E("COMMAND = %s\n", ___VALUE_STR(command));
            __U_E("invalid privilege : %s\n", ___VALUE_STR(privilege));
            return ICLI_RC_ERROR;
        }

        fprintf(g_fd, property_end_string,
                g_current_cmd->privilege->value_str,
                g_current_cmd->privilege->value_str,
                ___CMD_WITH_VALUE_STR(mode_goto)    ? ___VALUE_STR(mode_goto)    : "ICLI_CMD_MODE_MAX",
                ___CMD_WITH_VALUE_STR(mode_destroy) ? ___VALUE_STR(mode_destroy) : "ICLI_CMD_MODE_MAX",
                func_name,
                func_name,
                func_name );

        // function header for execution
        // if with function reuse, then no need to declare the header
        if ___CMD_NO_VALUE_STR( func_reuse ) {
            fprintf(g_fd, cb_header_string, func_name);
            fprintf(g_fd, ";\n");
        }

        // execution
        // if function reuse, then use the reuse one
        fprintf(g_fd, execution_begin_string,
                func_name,
                ___CMD_WITH_VALUE_STR(func_reuse) ? ___VALUE_STR(func_reuse) : func_name);

        if ___CMD_NO_VALUE_STR( usr_opt ) {
            fprintf(g_fd, "    0  // user option\n");
        } else {
            ___LINE_GEN( g_current_cmd->usr_opt );
            fprintf(g_fd, "    %s // user option\n", ___VALUE_STR(usr_opt));
        }

        fputs(execution_end_string, g_fd);

        // node
        fprintf(g_fd, cmd_nodes_begin_string, func_name);
        for ( i = 0; i < n; ++i ) {
            if ( vtss_icli_variable_data_has_range_get(g_variable_type[k][i]) ) {
                fprintf(g_fd, cmd_node_range_string,
                        i,
                        vtss_icli_variable_data_name_get(g_variable_type[k][i]),
                        func_name, i,
                        i,
                        func_name,
                        i == (n - 1) ? "\n" : ",\n");
            } else {
                fprintf(g_fd, cmd_node_string,
                        i,
                        vtss_icli_variable_data_name_get(g_variable_type[k][i]),
                        i,
                        func_name,
                        i == (n - 1) ? "\n" : ",\n");
            }
        }
        fputs(cmd_nodes_end_string, g_fd);

        fprintf(g_fd, register_begin_string,
                func_name,
                func_name,
                g_current_cmd->original_cmd,
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
                g_current_cmd->var_cmd,
#endif
                func_name,
                n,
                func_name,
                func_name);

        ___LINE_GEN( g_current_cmd->cmd_mode );

        fprintf(g_fd, register_end_string,
                ___VALUE_STR(cmd_mode));

        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#endif\n");
        }

        fprintf(g_fd, "\n");
    }

    /*
        command ID database
    */
    ___CMD_ID_DB_GEN();

    /*
        generate command callbacks
    */
    fputs(cb_comment_string, g_fd);

    /*
        generate callback for each command
    */
    for ( g_current_cmd = g_command, k = 0; g_current_cmd != NULL; ___NEXT(g_current_cmd), ++k ) {

        if ( k >= _CMD_MAX_CNT ) {
            T_E("too many commands\n");
            return ICLI_RC_ERROR;
        }

        // if with function reuse, then no need to generate the body
        if ___CMD_WITH_VALUE_STR( func_reuse ) {
            continue;
        }

        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#if %s\n", ___VALUE_STR(if_flag));
        }

        // header
        fprintf(g_fd, cb_header_string, ___VALUE_STR(func_name));
        fprintf(g_fd, "\n{\n");

        // MODE_VAR
        mode_word_cnt = g_current_cmd->mode_word_cnt;

        // get data for mode par first
        if ( mode_word_cnt ) {
            r = _word_type_get( (char *)(g_current_cmd->mode_info->cmd),
                                FALSE,
                                g_mode_var_name[k],
                                g_mode_var_type[k],
                                g_mode_var_range[k] );
            if ( r != ICLI_RC_OK ) {
                __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                __U_E("invalid mode cmd = %s\n", g_current_cmd->mode_info->cmd);
                return r;
            }
        }

        mode_var_cnt = 0;
        for ( i = 0, t = g_current_cmd->mode_var; i < mode_word_cnt && t != NULL; ++i ) {
            if ( g_mode_var_type[k][i] != ICLI_VARIABLE_KEYWORD  &&
                 g_mode_var_type[k][i] != ICLI_VARIABLE_PORT_TYPE ) {
                for ( ; t != NULL && vtss_icli_str_len(t->value_str) == 0; ___NEXT(t) ) {
                    ;
                }
                if ( t == NULL ) {
                    break;
                }
                if ( mode_var_cnt == 0 ) {
                    fprintf(g_fd, "    // MODE_VAR\n");
                }
                switch ( g_mode_var_type[k][i] ) {
                default :
                    fprintf(g_fd, "    %-21s %s = %s;\n",
                            vtss_icli_variable_data_decl_type_get(g_mode_var_type[k][i]),
                            t->value_str,
                            vtss_icli_variable_data_init_val_get(g_mode_var_type[k][i]));
                    if ( vtss_icli_variable_data_pointer_type_get(g_mode_var_type[k][i]) ) {
                        b_pointer_var = TRUE;
                    }
                    break;
                }
                ++mode_var_cnt;
                ___NEXT(t);
            }
        }
        if ( mode_var_cnt ) {
            fprintf(g_fd, "\n");
        }

        // CMD_VAR
        cmd_word_cnt = g_current_cmd->command_word_cnt;
        cmd_var_cnt = 0;
        b_dscp = FALSE;
        for ( i = 0, t = g_current_cmd->cmd_var; i < cmd_word_cnt && t != NULL; ++i, ___NEXT(t) ) {
            if ( vtss_icli_str_len(t->value_str) > 0 ) {
                if ( cmd_var_cnt == 0 ) {
                    fprintf(g_fd, "    // CMD_VAR\n");
                }
                switch ( g_variable_type[k][i] ) {
                case ICLI_VARIABLE_DSCP:
                    if ( b_dscp == FALSE ) {
                        b_dscp = TRUE;
                        fprintf(g_fd, "    %-21s %s = %s;\n",
                                vtss_icli_variable_data_decl_type_get(g_variable_type[k][i]),
                                t->value_str,
                                vtss_icli_variable_data_init_val_get(g_variable_type[k][i]));
                    }
                    break;

                default :
                    fprintf(g_fd, "    %-21s %s = %s;\n",
                            vtss_icli_variable_data_decl_type_get(g_variable_type[k][i]),
                            t->value_str,
                            vtss_icli_variable_data_init_val_get(g_variable_type[k][i]));
                    b_dscp = FALSE;
                    if ( vtss_icli_variable_data_pointer_type_get(g_variable_type[k][i]) ) {
                        b_pointer_var = TRUE;
                    }
                    break;
                }
                ++cmd_var_cnt;
            }
        }
        if ( cmd_var_cnt ) {
            fprintf(g_fd, "\n");
        }

        if ( b_pointer_var ) {
            fputs(cb_disable_lint_string, g_fd);
            fprintf(g_fd, "\n");
        }

        // VARIABLE_BEGIN
        ___SEGMENT_GEN(g_current_cmd->variable, var_init_begin_string, var_init_end_string);

        // avoid compile warning
        fputs(cb_warning_avoid_string, g_fd);
        fprintf(g_fd, "\n");

        // MODE_VAR, go through and get variables
        if ( mode_var_cnt > 0 ) {
            fprintf(g_fd, mode_begin_string, g_current_cmd->mode_info->cmd);
            for ( i = 0, t = g_current_cmd->mode_var; i < mode_word_cnt; ++i ) {
                if ( g_mode_var_type[k][i] == ICLI_VARIABLE_KEYWORD ) {
                    fprintf(g_fd, cmd_var_case_string, i, g_mode_var_name[k][i]);
                } else {
                    fprintf(g_fd, cmd_var_case2_string, i, g_mode_var_name[k][i]);
                }
                // find t
                if ( g_mode_var_type[k][i] != ICLI_VARIABLE_KEYWORD ) {
                    for ( ; t != NULL && vtss_icli_str_len(t->value_str) == 0; ___NEXT(t) ) {
                        ;
                    }
                    if ( t == NULL ) {
                        fprintf(g_fd, "            ");
                        fprintf(g_fd, "break;\n");
                        continue;
                    }
                }
                fprintf(g_fd, "            ");

                if ( g_mode_var_type[k][i] == ICLI_VARIABLE_PORT_TYPE ) {
                    fprintf(g_fd, "break;\n");
                    continue;
                }

                switch ( g_mode_var_type[k][i] ) {

                    ___GET_ALL( PARAMETER );

                case ICLI_VARIABLE_KEYWORD:
                    fprintf(g_fd, "break;\n");
                    continue;

                default :
                    __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                    __U_E("wrong type when auto-generate mode_var, %s, %d, %d\n",
                          g_mode_var_name[k][i], k, i);
                    return ICLI_RC_ERROR;
                }
                fputs(cmd_var_break_string, g_fd);

                ___NEXT(t);
            }
            fputs(mode_end_string, g_fd);
        }

        // CMD_VAR, go through and get variables
        if ( cmd_var_cnt > 0 ) {
            fprintf(g_fd, cmd_var_begin_string, ___VALUE_STR(command));

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
            b_dscp = FALSE;
#endif

            for ( i = 0, t = g_current_cmd->cmd_var; i < cmd_word_cnt; ++i ) {
                if ( g_variable_type[k][i] == ICLI_VARIABLE_KEYWORD ) {
                    fprintf(g_fd, cmd_var_case_string, i, g_variable_name[k][i]);
                } else {
                    fprintf(g_fd, cmd_var_case2_string, i, g_variable_name[k][i]);
                }
#if 1 /* CP, 2012/09/25 10:47, <dscp> */
                if ( g_variable_type[k][i] == ICLI_VARIABLE_DSCP ) {
                    if ( b_dscp ) {
                        ++dscp_id;
                    } else {
                        b_dscp = TRUE;
                        dscp_id = 0;
                    }
                } else {
                    b_dscp = FALSE;
                }
#endif
                if ( t != NULL ) {
                    if ( vtss_icli_str_len(t->value_str) > 0 ) {
                        fprintf(g_fd, "            ");
                        switch (g_variable_type[k][i]) {

                            ___GET_ALL( VARIABLE );

                        case ICLI_VARIABLE_KEYWORD:
                            fprintf(g_fd, "%s = TRUE;\n", t->value_str);
                            break;

                        case ICLI_VARIABLE_DSCP:
                            fprintf(g_fd, "%s = %u;\n", t->value_str, dscp_wvh[dscp_id].value);
                            break;

                        default :
                            __U_E("COMMAND = %s\n", ___VALUE_STR(command));
                            __U_E("wrong type when auto-generate cmd_var, %s, %d, %d\n",
                                  g_variable_name[k][i], k, i);
                            return ICLI_RC_ERROR;
                        }
                    }
                    ___NEXT(t);
                }
                fputs(cmd_var_break_string, g_fd);
            }
            fputs(cmd_var_end_string, g_fd);
        }

        // CODE_BEGIN
        ___SEGMENT_GEN(g_current_cmd->code, code_begin_string, code_end_string);

        // end
        fputs(cb_end_string, g_fd);

        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#endif\n");
        }

        fprintf(g_fd, "\n");

        if ( g_current_cmd->next != NULL ) {
            fprintf(g_fd, "/****************************************************************************/\n");
        }
    }

    /*
        generate register API
    */
    fprintf(g_fd, cmd_register_header_string, g_script_file, "");
    fputs(cmd_register_begin_string, g_fd);

    for ( g_current_cmd = g_command, i = 0; g_current_cmd != NULL; ___NEXT(g_current_cmd), ++i ) {
        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#if %s\n", ___VALUE_STR(if_flag));
        }

        fprintf(g_fd, cmd_register_string,
                ___VALUE_STR(command),
                i,
                ___VALUE_STR(func_name),
                ___VALUE_STR(command));

        // #if flag
        if ___CMD_WITH_VALUE_STR( if_flag ) {
            fprintf(g_fd, "#endif\n");
        }

        fprintf(g_fd, "\n");
    }

    fputs(cmd_register_end_string, g_fd);

    /*
        generate cmd-id API
    */
    fprintf(g_fd, cmd_id_api_header_string, g_script_file, "");
    fputs(cmd_id_api_body_string, g_fd);

    /*
        module IF flag
    */
    if ( ___MODULE_WITH_VALUE_STR(module_if_flag) ) {
        fprintf(g_fd, "//===== MODULE_IF_FLAG =======================================================\n");
        fprintf(g_fd, "#endif // %s\n", g_module->module_if_flag->value_str);
    }

    /*
        close srcipt file
    */
    fclose( g_fd );

    //printf("%s is generated successfully\n", g_icli_c_file);

    //===========================================
    //
    //  H file
    //
    //===========================================

    // create
    g_fd = fopen( g_icli_h_file, "w" );
    if ( g_fd == NULL ) {
        T_E("create file of %s\n", g_icli_h_file);
        return ICLI_RC_ERROR;
    }

    // generate header
    fputs(header_string, g_fd);

    // #ifndef
    vtss_icli_to_upper_case(g_script_file);
    fprintf(g_fd, "#ifndef ___%s_ICLI_H___\n", g_script_file);
    fprintf(g_fd, "#define ___%s_ICLI_H___\n\n", g_script_file);
    vtss_icli_to_lower_case(g_script_file);

#if 0 /* CP, 2012/09/27 11:43, disable EXPORT degment because of the parallel compile issue on dependency */
    // generate EXPORT_BEGIN
    ___SEGMENT_GEN(g_export, export_begin_string, export_end_string);
#endif

    /*
        module IF flag
    */
    if ( ___MODULE_WITH_VALUE_STR(module_if_flag) ) {
        fprintf(g_fd, "//===== MODULE_IF_FLAG =======================================================\n");
        fprintf(g_fd, "#if %s\n", g_module->module_if_flag->value_str);
        fprintf(g_fd, "\n");
    }

    // generate register API
    fprintf(g_fd, cmd_register_header_string, g_script_file, ";\n");

    // generate cmd-id API
    fprintf(g_fd, cmd_id_api_header_string, g_script_file, ";\n");

    /*
        module IF flag
    */
    if ( ___MODULE_WITH_VALUE_STR(module_if_flag) ) {
        fprintf(g_fd, "//===== MODULE_IF_FLAG =======================================================\n");
        fprintf(g_fd, "#endif // %s\n\n", g_module->module_if_flag->value_str);
    }

    // #endif
    vtss_icli_to_upper_case(g_script_file);
    fprintf(g_fd, "#endif //___%s_ICLI_H___\n", g_script_file);
    fprintf(g_fd, "\n");
    vtss_icli_to_lower_case(g_script_file);

    // close srcipt file
    fclose( g_fd );

    //printf("%s is generated successfully\n", g_icli_h_file);
    return ICLI_RC_OK;
}

/*----------------------------------------------------------------------------

    Command Register

----------------------------------------------------------------------------*/

/*
    check if it is a file or a directory

    INPUT
        f      : file with path or directory
        b_file : TRUE  - file check
                 FALSE - directory check

    OUTPUT
        n/a

    RETURN
        TRUE  - yes, it is
        FALSE - no, it is not
*/
#define ___FILE_CHECK      TRUE
#define ___DIR_CHECK       FALSE

#define ICLI_S_ISREG(m)          (((m) & _S_IFREG) == _S_IFREG)
#define ICLI_S_ISDIR(m)          (((m) & _S_IFDIR) == _S_IFDIR)

static BOOL _is_file_dir(
    IN  char    *f,
    IN  BOOL    b_file
)
#ifdef WIN32
{
    struct _stat    s;
    i32             result;

    /* Get data associated with "stat.c": */
    result = _stat( f, &s );

    /* Check if statistics are valid: */
    if ( result != 0 ) {
        return FALSE;
    }

    return b_file ? ICLI_S_ISREG(s.st_mode) : ICLI_S_ISDIR(s.st_mode);
}
#else
{
    struct stat     s;
    i32             result;

    /* Get data associated with "stat.c": */
    result = stat( f, &s );

    /* Check if statistics are valid: */
    if ( result != 0 ) {
        return FALSE;
    }

    return b_file ? S_ISREG(s.st_mode) : S_ISDIR(s.st_mode);
}
#endif

#define _CMD_REG_MAX_LINE   1024
#define _CMD_REG_LINE_LEN   128
#define _CMD_REG_LEN        64
#define _CMD_REG_BEGIN      "static _cmd_register_cb_t   *g_cmd_reg[ICLI_CMD_CNT] = {\n"
#define _CMD_REG_END        "};\n"
#define _CMD_REG_NULL_CB    "NULL"
#define _CMD_REG_INCLUDE    "#include"

/*
    icli_cmd_reg.c will be created according to the IREG files.
    Because Include file, x_icli.h, and register API, x_icli_cmd_register
    are auto-generated by ICLI engine, ICLI engine can create them by x.
    Therefore, what ICLI engine does not know is the module compile flag.
    Then, currently, IREG file, x.ireg, will write the module compile flag
    only.

    HA, But ....
    because icli_cmd_reg.c will not be created by this applicatiion,
    if the naming rule is changed, there will be trouble for sync.
    So, writing all information into IREG file will make things easy.

    if the item does not exist, then that line will be emty.

     IREG file format in lines
     -------------------------------------------------------------------------
line 1 include file
     2 register API
     3 Module compile flag
*/
static i32 _generate_IREG( void )
{
    FILE    *f;
    char    *ireg_file;
    int     len;

    // get IREG file name
    if ( g_generate_path ) {
        len = vtss_icli_str_len(g_generate_path) + vtss_icli_str_len(g_script_file) + 16;

        ireg_file = icli_malloc(len);
        if ( ireg_file == NULL ) {
            T_E("fail to allocate memory for ireg_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(ireg_file, "%s/%s.ireg", g_generate_path, g_script_file);
    } else {
        len = vtss_icli_str_len(g_script_file) + 16;

        ireg_file = icli_malloc(len);
        if ( ireg_file == NULL ) {
            T_E("fail to allocate memory for ireg_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(ireg_file, "%s.ireg", g_script_file);
    }

    f = fopen( ireg_file, "w" );
    if ( f == NULL ) {
        T_E("open %s\n", ireg_file);
        icli_free(ireg_file);
        return ICLI_RC_ERROR;
    }

    // line 1: include file
    fprintf( f, "%s_icli.h\n", g_script_file);

    // line 2: register API
    fprintf( f, "%s_icli_cmd_register\n", g_script_file);

    // line 3: Module compile flag
    if ( ___MODULE_WITH_VALUE_STR(module_if_flag) ) {
        fprintf( f, "%s\n", g_module->module_if_flag->value_str);
    } else {
        fprintf( f, "\n");
    }

    fclose( f );

    //printf("Create %s successfully\n", ireg_file);
    icli_free(ireg_file);
    return ICLI_RC_OK;
}

/*----------------------------------------------------------------------------

    HTML Generation

----------------------------------------------------------------------------*/
#define _HTML_TITLE             "CLI Command Reference Guide"
#define _TAG_TITLE_BEGIN        "<div class=Title>"
#define _TAG_REFLINK_BEGIN      "<div class=RefLink>"
#define _HTML_REFLINK_BEGIN     "<p class=RefLink>"
#define _HTML_REFLINK_END       "</a></p>"
#define _HTML_COMMAND_BEGIN     "<div class=Command>"
#define _HTML_COMMAND_END       "</div>"
#define _HTML_COMMAND_TITLE     "<p class=CmdTitle>"
#define _HTML_NEW_LINE          "<p class=NewLine>&nbsp;</p>"
#define _HTML_COMMAND_CNT       "<!-- Number of commands"
#define _HTML_NULL_DESC         "None."

typedef struct __html_element {
    _tag_element_t          *begin;
    _tag_element_t          *command;
    _command_t              *cmd_ptr;
    _tag_element_t          *end;
    struct __html_element   *prev;
    struct __html_element   *next;
} _html_element;

static _html_element    *g_html_reflink = NULL;
static _html_element    *g_html_command = NULL;
static int              g_command_cnt = 0;

static const char _html_head[] =
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

#if 0 /* not used */
#define ___HTML_ADD_TO_TAIL(p)\
    (void)vtss_icli_str_cpy(g_value_str, g_line_buf);\
    ___VALUE_STR_MALLOC_AND_GET_AND_ADD_TO_TAIL(p);

static i32 _html_read( void )
{
    _tag_element_t  *t;
    _html_element   *h,
                    *tail;

    /* open file as read/write */
    g_fd = fopen( g_html_file, "r+" );
    if ( g_fd == NULL ) {
        T_E("open %s\n", g_html_file);
        return ICLI_RC_ERROR;
    }

    while (_line_read() == ICLI_RC_OK) {
        /* get g_command_cnt */
        if ( vtss_icli_str_cmp(g_line_buf, _HTML_COMMAND_CNT) == 0 ) {
            if (_line_read() != ICLI_RC_OK) {
                T_E("get g_command_cnt\n");
                return ICLI_RC_ERROR;
            }
            g_command_cnt = atoi( g_line_buf );
            continue;
        }

        /* get g_html_reflink */
        if ( vtss_icli_str_cmp(g_line_buf, _HTML_REFLINK_BEGIN) == 0 ) {
            h = (_html_element *)icli_malloc(sizeof(_html_element));
            if ( h == NULL ) {
                T_E("icli_malloc\n");
                return ICLI_RC_ERROR;
            }
            memset(h, 0, sizeof(_html_element));

            ___HTML_ADD_TO_TAIL(h->begin);

            while (_line_read() == ICLI_RC_OK) {
                ___HTML_ADD_TO_TAIL(h->begin);
                if ( vtss_icli_str_cmp(g_line_buf, _HTML_REFLINK_END) == 0 ) {
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

            if ( g_html_reflink ) {
                for (tail = g_html_reflink; tail->next != NULL; ___NEXT(tail)) {
                    ;
                }
                tail->next = h;
                h->prev    = tail;
            } else {
                g_html_reflink = h;
            }
        }

        /* get g_html_command */
        if ( vtss_icli_str_cmp(g_line_buf, _HTML_COMMAND_BEGIN) == 0 ) {
            h = (_html_element *)icli_malloc(sizeof(_html_element));
            if ( h == NULL ) {
                T_E("icli_malloc\n");
                return ICLI_RC_ERROR;
            }
            memset(h, 0, sizeof(_html_element));

            ___HTML_ADD_TO_TAIL(h->begin);

            while (_line_read() == ICLI_RC_OK) {
                ___HTML_ADD_TO_TAIL(h->begin);
                if ( vtss_icli_str_cmp(g_line_buf, _HTML_COMMAND_END) == 0 ) {
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->end = t;
                    break;
                } else if ( vtss_icli_str_sub("<a name=", g_line_buf, 1, NULL) == 1 ) {
                    //read command
                    if ( _line_read() != ICLI_RC_OK ) {
                        T_E("read command\n");
                        return ICLI_RC_ERROR;
                    }
                    ___HTML_ADD_TO_TAIL(h->begin);
                    for (t = h->begin; t->next != NULL; ___NEXT(t)) {
                        ;
                    }
                    h->command = t;
                }
            }

            if ( g_html_command ) {
                for (tail = g_html_command; tail->next != NULL; ___NEXT(tail)) {
                    ;
                }
                tail->next = h;
                h->prev    = tail;
            } else {
                g_html_command = h;
            }
        }
    }

    /* close file */
    fclose( g_fd );

    return ICLI_RC_OK;
}
#endif

#define ___HTML_DEF_TAG_SIZE    32

#define ___t_ALLOC(t,str_len) \
    t = (_tag_element_t *)icli_malloc(sizeof(_tag_element_t));\
    if ( t == NULL ) {\
        T_E("icli_malloc(tag)\n");\
        return NULL;\
    }\
    memset(t, 0, sizeof(_tag_element_t));\
    t->value_str = (char *)icli_malloc(str_len+4);\
    if ( t->value_str == NULL ) {\
        T_E("icli_malloc(value_str)\n");\
        return NULL;\
    }\
    memset(t->value_str, 0, str_len);

#define ___HTML_NEW_LINE_ADD()\
    ___t_ALLOC(t, ___HTML_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, _HTML_NEW_LINE);\
    p->next = t;\
    p = t;

#define ___HTML_TAG_BEGIN_ADD(s)\
    ___t_ALLOC(t, ___HTML_DEF_TAG_SIZE);\
    (void)vtss_icli_str_cpy(t->value_str, s);\
    h->begin = t;\
    p        = t;

#define ___HTML_TAG_ADD(s)\
    ___t_ALLOC(t, ___HTML_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, s);\
    p->next = t;\
    p = t;

#define ___HTML_TAG_ADD_CHAR_STR(s)\
    ___t_ALLOC(t, ___HTML_DEF_TAG_SIZE);\
    icli_sprintf(t->value_str, "%s", s);\
    p->next = t;\
    p = t;

#define ___HTML_STRING_ADD(l, format, s)\
    ___t_ALLOC(t, (l));\
    icli_sprintf(t->value_str, format, s);\
    p->next = t;\
    p = t;

#define ___HTML_TAG_END_ADD(s)\
    ___t_ALLOC(t, ___HTML_DEF_TAG_SIZE);\
    (void)vtss_icli_str_cpy(t->value_str, s);\
    p->next = t;\
    h->end  = t;

#define ___HTML_SUBTITLE1_ADD(s)\
    ___HTML_TAG_ADD("<p class=CmdSubTitle1>");\
    ___HTML_TAG_ADD(s);\
    ___HTML_TAG_ADD("</p>");

/*
    convert cmd string by html syntax into g_line_buf
*/
static BOOL _html_cmd_convert(
    IN char *cmd
)
{
    char    *s,
            *d;

    memset(g_line_buf, 0, sizeof(g_line_buf));
    for ( s = cmd, d = g_line_buf; ICLI_NOT_(EOS, *s); ++s, ++d ) {
        /* customize commad syntax */
        switch ( *s ) {
        case ICLI_MANDATORY_BEGIN:
            *d = ICLI_HTM_MANDATORY_BEGIN;
            break;

        case ICLI_MANDATORY_END:
            *d = ICLI_HTM_MANDATORY_END;
#if ICLI_RANDOM_MUST_NUMBER
            if ( ICLI_IS_RANDOM_MUST(s) ) {
                s += 2;
            }
#endif
            break;

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        case ICLI_LOOP_BEGIN:
        case ICLI_LOOP_END:
            d--;
            break;
#endif

        case ICLI_OPTIONAL_BEGIN:
            *d = ICLI_HTM_OPTIONAL_BEGIN;
            break;

        case ICLI_OPTIONAL_END:
            *d = ICLI_HTM_OPTIONAL_END;
            break;

        case ICLI_VARIABLE_BEGIN:
            *d = ICLI_HTM_VARIABLE_BEGIN;
            break;

        case ICLI_VARIABLE_END:
            *d = ICLI_HTM_VARIABLE_END;
            break;

        default:
            if ( ICLI_IS_(SPACE, *(d - 1)) && ICLI_IS_(SPACE, *s) ) {
                d--;
                continue;
            } else {
                *d = *s;
            }
            break;
        }

        /* convert htm syntax */
        switch ( *d ) {
        case '<':
            *d++ = '&';
            *d++ = 'l';
            *d++ = 't';
            *d   = ';';
            break;

        case '>':
            *d++ = '&';
            *d++ = 'g';
            *d++ = 't';
            *d   = ';';
            break;

        case '&':
            *d++ = '&';
            *d++ = 'a';
            *d++ = 'm';
            *d++ = 'p';
            *d   = ';';
            break;
        }
    }
    return TRUE;
}

static BOOL _html_str_convert(
    IN char *cmd
)
{
    char    *s,
            *d;

    memset(g_line_buf, 0, sizeof(g_line_buf));
    for ( s = cmd, d = g_line_buf; ICLI_NOT_(EOS, *s); ++s ) {
        switch ( *s ) {
        case '<':
            *d++ = '&';
            *d++ = 'l';
            *d++ = 't';
            *d++ = ';';
            break;

        case '>':
            *d++ = '&';
            *d++ = 'g';
            *d++ = 't';
            *d++ = ';';
            break;

        case '&':
            *d++ = '&';
            *d++ = 'a';
            *d++ = 'm';
            *d++ = 'p';
            *d++ = ';';
            break;

        default:
            *d++ = *s;
            break;
        }
    }
    return TRUE;
}

static _html_element *_html_reflink_create(
    IN _command_t *cmd
)
{
    _tag_element_t  *t,
                    *p;
    _html_element   *h;
    char            *c;

    h = (_html_element *)icli_malloc(sizeof(_html_element));
    if ( h == NULL ) {
        T_E("icli_malloc\n");
        return NULL;
    }
    memset(h, 0, sizeof(_html_element));

    ___HTML_TAG_BEGIN_ADD( _HTML_REFLINK_BEGIN );

    /* reference link */
    ___HTML_STRING_ADD(___HTML_DEF_TAG_SIZE, "<a href=\"#cr%04d\">", g_command_cnt);

    /* command */
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    if ( _html_cmd_convert(cmd->var_cmd) == FALSE ) {
#else
    if ( _html_cmd_convert(cmd->command->value_str) == FALSE ) {
#endif
        T_E("convert command to html\n");
        return NULL;
    }

    c = g_line_buf;
    ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
    h->command = t;
    h->cmd_ptr = cmd;

    ___HTML_TAG_END_ADD( _HTML_REFLINK_END );

    return h;
}

static _html_element *_html_command_create(
    IN _command_t *cmd
)
{
    _tag_element_t      *t,
                        *p,
                        *x;
    _html_element       *h;
    int                 i,
                        len;
    char                *c,
                        *w;
    icli_privilege_t    priv;
#if _COMPLETE_HTML
    int                 has_value_str;
#endif

    h = (_html_element *)icli_malloc(sizeof(_html_element));
    if ( h == NULL ) {
        T_E("icli_malloc\n");
        return NULL;
    }
    memset(h, 0, sizeof(_html_element));

    /* Begin */
    ___HTML_TAG_BEGIN_ADD( _HTML_COMMAND_BEGIN );

    ___HTML_TAG_ADD("<hr>");

    ___HTML_NEW_LINE_ADD();

    /* Title */
    ___HTML_TAG_ADD(_HTML_COMMAND_TITLE);

    // reference link
    ___HTML_STRING_ADD(___HTML_DEF_TAG_SIZE, "<a name=\"cr%04d\">", g_command_cnt);

    // command
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    if ( _html_cmd_convert(cmd->var_cmd) == FALSE ) {
#else
    if ( _html_cmd_convert(cmd->command->value_str) == FALSE ) {
#endif
        T_E("convert command '%s' to html\n", cmd->command->value_str);
        return NULL;
    }
    c   = g_line_buf;
    len = vtss_icli_str_len(c) + 4;
    ___HTML_STRING_ADD(len, "%s", c);
    h->command = t;
    h->cmd_ptr = cmd;

    ___HTML_TAG_ADD("</a></p>");

    /* Command Description */
#if _COMPLETE_HTML
    has_value_str = FALSE;
    for ( x = cmd->doc_cmd_desc; x != NULL; ___NEXT(x)) {
        if ( vtss_icli_str_len(x->value_str) ) {
            if ( _html_str_convert(x->value_str) == FALSE ) {
                T_E("convert string '%s' to html\n", x->value_str);
                return NULL;
            }
            c = g_line_buf;

            has_value_str = TRUE;
            ___HTML_TAG_ADD("<p class=CmdDesc>");
            ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
            ___HTML_TAG_ADD("</p>");
        }
    }
    if ( has_value_str == FALSE) {
        ___HTML_TAG_ADD("<p class=CmdDesc>");
        ___HTML_TAG_ADD(_HTML_NULL_DESC);
        ___HTML_TAG_ADD("</p>");
    }
#endif

    ___HTML_NEW_LINE_ADD();

    /* Syntax Description */
    ___HTML_SUBTITLE1_ADD("Syntax Description");

    // copy a command
    w = (char *)icli_malloc(len);
    memset(w, 0, len);
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    (void)vtss_icli_str_cpy(w, cmd->var_cmd);
#else
    (void)vtss_icli_str_cpy(w, cmd->command->value_str);
#endif
    for ( i = 0, x = cmd->help; i < cmd->command_word_cnt; ++i ) {
        c = vtss_icli_word_get_next(&w);
        if ( c == NULL ) {
            T_E("COMMAND = %s\n", cmd->command->value_str);
            T_E("get word in %s\n", c);
            return NULL;
        }

        if ( _html_cmd_convert(c) == FALSE ) {
            T_E("COMMAND = %s\n", cmd->command->value_str);
            T_E("convert command to html\n");
            return NULL;
        }

        c = g_line_buf;
        ___HTML_TAG_ADD("<p class=CmdSubTitle2>");
        ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
        ___HTML_TAG_ADD("</p>");

        ___HTML_TAG_ADD("<p class=CmdSubDesc2>");
        if ( x && vtss_icli_str_len(_help_str_get(x)) ) {
            if ( _html_str_convert(_help_str_get(x)) == FALSE ) {
                T_E("convert help '%s' to html\n", _help_str_get(x));
                return NULL;
            }
            c = g_line_buf;

            ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
            ___NEXT(x);
        } else {
            ___HTML_TAG_ADD(_HTML_NULL_DESC);
        }
        ___HTML_TAG_ADD("</p>");
    }

    ___HTML_NEW_LINE_ADD();

    /* Default */
#if _COMPLETE_HTML
    ___HTML_SUBTITLE1_ADD("Default");

    has_value_str = FALSE;
    for ( x = cmd->doc_cmd_default; x != NULL; ___NEXT(x)) {
        if ( vtss_icli_str_len(x->value_str) ) {
            if ( _html_str_convert(x->value_str) == FALSE ) {
                T_E("convert string '%s' to html\n", x->value_str);
                return NULL;
            }
            c = g_line_buf;

            has_value_str = TRUE;
            ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
            ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
            ___HTML_TAG_ADD("</p>");
        }
    }
    if ( has_value_str == FALSE ) {
        ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
        ___HTML_TAG_ADD(_HTML_NULL_DESC);
        ___HTML_TAG_ADD("</p>");
    }
#endif

    /* Command Mode */
    ___HTML_SUBTITLE1_ADD("Command Mode");
    ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
    ___HTML_TAG_ADD_CHAR_STR( cmd->mode_info->desc );
    ___HTML_TAG_ADD("</p>");

    /* Privilege Level */
    ___HTML_SUBTITLE1_ADD("Privilege level");
    ___HTML_TAG_ADD("<p class=CmdSubDesc1>");

    priv = icli_privilege_get_by_str( cmd->privilege->value_str );
    switch ( priv ) {
    case ICLI_PRIVILEGE_MAX:
        ___HTML_STRING_ADD(vtss_icli_str_len(cmd->privilege->value_str) + 4, "%s", cmd->privilege->value_str);
        break;
    case ICLI_PRIVILEGE_DEBUG:
        ___HTML_TAG_ADD("debug");
        break;
    default:
        ___HTML_STRING_ADD(___HTML_DEF_TAG_SIZE, "%d", priv);
        break;
    }

    /* Usage Guideline */
#if _COMPLETE_HTML
    ___HTML_SUBTITLE1_ADD("Usage Guideline");

    has_value_str = FALSE;
    for ( x = cmd->doc_cmd_usage; x != NULL; ___NEXT(x)) {
        if ( vtss_icli_str_len(x->value_str) ) {
            if ( _html_str_convert(x->value_str) == FALSE ) {
                T_E("convert string '%s' to html\n", x->value_str);
                return NULL;
            }
            c = g_line_buf;

            has_value_str = TRUE;
            ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
            ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
            ___HTML_TAG_ADD("</p>");
        }
    }
    if ( has_value_str == FALSE ) {
        ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
        ___HTML_TAG_ADD(_HTML_NULL_DESC);
        ___HTML_TAG_ADD("</p>");
    }
#endif

    /* Example */
#if _COMPLETE_HTML
    ___HTML_SUBTITLE1_ADD("Example");

    has_value_str = FALSE;
    for ( x = cmd->doc_cmd_example; x != NULL; ___NEXT(x)) {
        if ( vtss_icli_str_len(x->value_str) ) {
            if ( _html_str_convert(x->value_str) == FALSE ) {
                T_E("convert string '%s' to html\n", x->value_str);
                return NULL;
            }
            c = g_line_buf;

            has_value_str = TRUE;
            ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
            ___HTML_STRING_ADD(vtss_icli_str_len(c) + 4, "%s", c);
            ___HTML_TAG_ADD("</p>");
        }
    }
    if ( has_value_str == FALSE ) {
        ___HTML_TAG_ADD("<p class=CmdSubDesc1>");
        ___HTML_TAG_ADD(_HTML_NULL_DESC);
        ___HTML_TAG_ADD("</p>");
    }
#endif

    ___HTML_NEW_LINE_ADD();

    /* End */
    ___HTML_TAG_END_ADD( _HTML_COMMAND_END );

    return h;
}

#define ___HTML_ELEMENT_INSERT(element)\
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

static BOOL _html_cmd_ref_add( void )
{
    _html_element       *h;
    _html_element       *e;
    _html_element       *p;
    icli_privilege_t    priv;
    char                *a;
    char                *c;

    /* add command */
    for (g_current_cmd = g_command; g_current_cmd != NULL; ___NEXT(g_current_cmd)) {

        //check command property

        //debug level
        if ( g_current_cmd->privilege ) {
            priv = icli_privilege_get_by_str( g_current_cmd->privilege->value_str );
            if ( priv == ICLI_PRIVILEGE_DEBUG ) {
                continue;
            }
        }

        //ICLI_CMD_PROP_DISABLE
        if ( g_current_cmd->property &&
             vtss_icli_str_str("ICLI_CMD_PROP_DISABLE", g_current_cmd->property->value_str) != NULL ) {
            continue;
        }

        //ICLI_CMD_PROP_INVISIBLE
        if ( g_current_cmd->property &&
             vtss_icli_str_str("ICLI_CMD_PROP_INVISIBLE", g_current_cmd->property->value_str) != NULL ) {
            continue;
        }

        //increase count
        ++g_command_cnt;

        //remove grep if exist
        c = ___VALUE_STR(command);
        if ( g_current_cmd->grep ) {
            c[vtss_icli_str_len(c) - vtss_icli_str_len(___GREP_CMD_STR)] = 0;
            g_current_cmd->command_word_cnt = vtss_icli_word_count( c );
            if ( g_current_cmd->command_word_cnt < 0 ) {
                T_E("COMMAND = %s\n", ___VALUE_STR(command));
                T_E("fail to get command word count\n");
                return FALSE;
            }
        }

        //remove single Quote
        for ( c = ___VALUE_STR(command); *c; ++c ) {
            if ( ICLI_IS_(SQUOTE, *c) ) {
                for ( a = c + 1; *a; ++a ) {
                    *(a - 1) = *a;
                }
                *(a - 1) = 0;
            }
        }

        //create reference link
        h = _html_reflink_create( g_current_cmd );
        if ( h == NULL ) {
            T_E("COMMAND = %s\n", ___VALUE_STR(command));
            T_E("fail to create reflink\n");
            return FALSE;
        }

        //insert reference link
        ___HTML_ELEMENT_INSERT( g_html_reflink );

        //create command
        h = _html_command_create( g_current_cmd );
        if ( h == NULL ) {
            T_E("COMMAND = %s\n", ___VALUE_STR(command));
            T_E("fail to create command\n");
            return FALSE;
        }

        //insert command
        ___HTML_ELEMENT_INSERT( g_html_command );
    }

    return TRUE;
}

#define ___HTML_ELEMENT_WRITE(element)\
    if ( element ) {\
        for (h = element; h != NULL; ___NEXT(h)) {\
            for (t = h->begin; t != NULL; ___NEXT(t)) {\
                fprintf(g_fd, "%s\n", t->value_str);\
            }\
            fprintf(g_fd, "\n");\
        }\
    }

static BOOL _html_write( void )
{
    _tag_element_t  *t;
    _html_element   *h;

    /* open file as read/write */
    g_fd = fopen( g_html_file, "w" );
    if ( g_fd == NULL ) {
        T_E("open %s\n", g_html_file);
        return FALSE;
    }

    fprintf(g_fd, _html_head, g_command_cnt, _HTML_TITLE);

    ___HTML_ELEMENT_WRITE( g_html_reflink );

    fprintf(g_fd, "%s\n", _HTML_NEW_LINE);
    fprintf(g_fd, "</div>\n");
    fprintf(g_fd, "\n");

    ___HTML_ELEMENT_WRITE( g_html_command );

    fprintf(g_fd, "</body></html>\n");
    fprintf(g_fd, "\n");

    /* close file */
    fclose( g_fd );
    return TRUE;
}

#if 0 /* not used */
static void _html_cmd_syntax_porting( void )
{
    char    *c;

    for (g_current_cmd = g_command; g_current_cmd != NULL; ___NEXT(g_current_cmd)) {
        for ( c = ___VALUE_STR(command); ICLI_NOT_(EOS, *c); ++c ) {
            switch ( *c ) {
            case ICLI_MANDATORY_BEGIN:
                *c = ICLI_HTM_MANDATORY_BEGIN;
                break;

            case ICLI_MANDATORY_END:
                *c = ICLI_HTM_MANDATORY_END;
                break;

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            case ICLI_LOOP_BEGIN:
                *c = ICLI_HTM_LOOP_BEGIN;
                break;

            case ICLI_LOOP_END:
                *c = ICLI_HTM_LOOP_END;
                break;
#endif

            case ICLI_OPTIONAL_BEGIN:
                *c = ICLI_HTM_OPTIONAL_BEGIN;
                break;

            case ICLI_OPTIONAL_END:
                *c = ICLI_HTM_OPTIONAL_END;
                break;

            case ICLI_VARIABLE_BEGIN:
                *c = ICLI_HTM_VARIABLE_BEGIN;
                break;

            case ICLI_VARIABLE_END:
                *c = ICLI_HTM_VARIABLE_END;
                break;

            default:
                break;
            }
        }
    }
}
#endif

static i32 _generate_HTM( void )
{
    int     len;

    g_html_reflink = NULL;
    g_html_command = NULL;
    g_command_cnt  = 0;

    // get HTML file name
    if ( vtss_icli_str_len(g_generate_path) ) {
        len = vtss_icli_str_len(g_generate_path) + vtss_icli_str_len(g_script_file) + 16;

        g_html_file = icli_malloc(len);
        if ( g_html_file == NULL ) {
            T_E("fail to allocate memory for g_html_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_html_file, "%s/%s.htm", g_generate_path, g_script_file);
    } else {
        len = vtss_icli_str_len(g_script_file) + 16;

        g_html_file = icli_malloc(len);
        if ( g_html_file == NULL ) {
            T_E("fail to allocate memory for g_html_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_html_file, "%s.htm", g_script_file);
    }

    /* add current commands into */
    if ( _html_cmd_ref_add() == FALSE ) {
        T_E("fail to add command reference!\n");
        return ICLI_RC_ERROR;
    }

    /* write back into html */
    if ( _html_write() == FALSE ) {
        T_E("fail to write %s\n", g_html_file);
        return ICLI_RC_ERROR;
    }

    //printf("Create %s successfully\n", g_html_file);
    return ICLI_RC_OK;
}

static i32 _generate_TXT( void )
{
    int             len;
    _html_element   *h;

    // get HTML file name
    if ( vtss_icli_str_len(g_generate_path) ) {
        len = vtss_icli_str_len(g_generate_path) + vtss_icli_str_len(g_script_file) + 16;

        g_txt_file = icli_malloc(len);
        if ( g_txt_file == NULL ) {
            T_E("fail to allocate memory for g_txt_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_txt_file, "%s/%s.txt", g_generate_path, g_script_file);
    } else {
        len = vtss_icli_str_len(g_script_file) + 16;

        g_txt_file = icli_malloc(len);
        if ( g_txt_file == NULL ) {
            T_E("fail to allocate memory for g_txt_file\n");
            return ICLI_RC_ERROR;
        }

        icli_sprintf(g_txt_file, "%s.txt", g_script_file);
    }

    /* open file as read/write */
    g_fd = fopen( g_txt_file, "w" );
    if ( g_fd == NULL ) {
        T_E("open %s\n", g_txt_file);
        return FALSE;
    }

    for (h = g_html_reflink; h != NULL; ___NEXT(h)) {
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
        fprintf(g_fd, "%s\n", h->cmd_ptr->var_cmd);
#else
        fprintf(g_fd, "%s\n", h->cmd_ptr->original_cmd);
#endif
    }

    /* close file */
    fclose( g_fd );

    return ICLI_RC_OK;
}

static i32 _script_file_check(
    IN char *script_path
)
{
    char    *c, *p, f;

    c = script_path + (vtss_icli_str_len(script_path) - vtss_icli_str_len(_SCRIPT_SUFFIX));
    if ( vtss_icli_str_cmp(c + (vtss_icli_str_len(c) - vtss_icli_str_len(_SCRIPT_SUFFIX)), _SCRIPT_SUFFIX) != 0 ) {
        T_E("bad script extension\n");
        return ICLI_RC_ERROR;
    }

    if ( ! ___VALID_FILE_CHAR(*(c - 1)) ) {
        T_E("bad script file name\n");
        return ICLI_RC_ERROR;
    }

    // get script file name
    for ( p = c--; ___VALID_FILE_CHAR(*c); --c ) {
        if ( c == script_path ) {
            break;
        }
    }

    //check if '/' or '\'
    if ( c != script_path ) {
        if ( ICLI_NOT_(SLASH, *c) && ICLI_NOT_(BACK_SLASH, *c) ) {
            T_E("bad script file name\n");
            return ICLI_RC_ERROR;
        } else {
            ++c;
        }
    }

    g_script_path = icli_malloc(vtss_icli_str_len(c) + 1);
    if ( g_script_path == NULL ) {
        T_E("fail to allocate memory for g_script_path\n");
        return ICLI_RC_ERROR;
    }
    (void)vtss_icli_str_cpy(g_script_path, c);

    f = *p;
    *p = 0;

    g_script_file = icli_malloc(vtss_icli_str_len(c) + 1);
    if ( g_script_file == NULL ) {
        T_E("fail to allocate memory for g_script_file\n");
        return ICLI_RC_ERROR;
    }
    (void)vtss_icli_str_cpy(g_script_file, c);

    vtss_icli_to_lower_case(g_script_file);
    *p = f;
    return ICLI_RC_OK;
}

#define ___CHECK_r(s) \
    if (r != ICLI_RC_OK) {\
        T_E("parse script file at line %d in %s segment\n", g_line_number, s);\
        return ICLI_RC_ERROR;\
    }

static i32 _include_file_parse(
    IN char     *include_file
)
{
    char    *c;
    char    *e;
    u32     len;

    // open script file for parsing
    g_fd = fopen(include_file, "r");
    if (g_fd == NULL) {
        T_E("open %s\n", include_file);
        return ICLI_RC_ERROR;
    }

    g_line_number = 0;
    while (_line_read() == ICLI_RC_OK) {
        /*
            get g_tag_str
        */
        // skip space
        ICLI_SPACE_SKIP(c, g_line_buf);
        // must #define
        if ( vtss_icli_str_sub("#define", c, 0, NULL) != 1 ) {
            continue;
        }
        // skip #define
        c += 7;
        // must SPACE
        if ( ICLI_NOT_(SPACE, *c) ) {
            continue;
        }
        // skip SPACE
        ICLI_SPACE_SKIP(c, c);
        // get constant end
        for ( e = c + 1; ICLI_NOT_(SPACE, *e) && ICLI_NOT_(EOS, *e); ++e ) {
            ;
        }
        if ( ICLI_IS_(EOS, *e) ) {
            continue;
        }
        // set to EOS
        *e = ICLI_EOS;
        // copy to g_tag_str
        memset(g_tag_str, 0, sizeof(g_tag_str));
        (void)vtss_icli_str_cpy(g_tag_str, c);

        /*
            get g_value_str
        */
        // skip SPACE
        ICLI_SPACE_SKIP(c, e + 1);
        // must ICLI_STRING_BEGIN
        if ( ICLI_NOT_(STRING_BEGIN, *c) ) {
            continue;
        }
        // skip ICLI_STRING_BEGIN
        ++c;
        // find end
        len = vtss_icli_str_len(c);
        if ( len == 0 ) {
            continue;
        }
        e = c + len - 1;
        // skip SPACE at end
        for ( ; ICLI_IS_(SPACE, *e); --e ) {
            ;
        }
        // must ICLI_STRING_END
        if ( ICLI_NOT_(STRING_END, *e) ) {
            continue;
        }
        // set to EOS
        *e = ICLI_EOS;
        // copy c to g_value_str
        memset(g_value_str, 0, sizeof(g_value_str));
        (void)vtss_icli_str_cpy(g_value_str, c);

        /*
            get g_value_str
        */
        if ( _add_str_def() != ICLI_RC_OK ) {
            T_E("include file %s line %d\n", include_file, g_line_number);
            return ICLI_RC_ERROR;
        }
    }

    //check if end of file
    if ( ! feof( g_fd ) ) {
        T_E("read line %d\n", (g_line_number + 1));
        return ICLI_RC_ERROR;
    }

    // close include file
    fclose( g_fd );
    return ICLI_RC_OK;
}

static i32 _script_file_parse(
    IN char *script_file
)
{
    i32     r;
    u32     u;

    // open script file for parsing
    g_fd = fopen(script_file, "r");
    if (g_fd == NULL) {
        T_E("open %s\n", script_file);
        return ICLI_RC_ERROR;
    }

    // init g_line_number
    g_line_number = 0;
    g_command_number = 0;

    u = (u32)((unsigned long)script_file & 0xFFffFFff);
    srand( icli_current_time_get() + u + vtss_icli_str_len(script_file) );

    //allocate module segment
    ___STRUCT_MALLOC(g_module, _module_t, FALSE);

    while (_line_read() == ICLI_RC_OK) {
        switch ( _str_tag_and_value_get() ) {
        case ICLI_RC_OK:
            switch (_tag_get()) {

                /* Global segment */
            case _TAG_MODULE_IF_FLAG:
                r = _value_str_get();
                ___CHECK_r("MODULE_IF_FLAG");
                break;

            case _TAG_STR_DEF:
                r = _value_str_get();
                ___CHECK_r("STR_DEF");
                break;

                /* Include segment */
            case _TAG_INCLUDE_BEGIN:
                // parsing include
                r = _section_parsing(_TAG_INCLUDE_BEGIN, _TAG_INCLUDE_END, &g_include, _SEGMENT_BEGIN);
                ___CHECK_r("INCLUDE");
                break;

                /* Function segment */
            case _TAG_FUNCTION_BEGIN:
                // parsing function
                r = _section_parsing(_TAG_FUNCTION_BEGIN, _TAG_FUNCTION_END, &g_function, _SEGMENT_BEGIN);
                ___CHECK_r("FUNCTION");
                break;

                /* Export segment */
            case _TAG_EXPORT_BEGIN:
                // parsing export
                r = _section_parsing(_TAG_EXPORT_BEGIN, _TAG_EXPORT_END, &g_export, _SEGMENT_BEGIN);
                ___CHECK_r("EXPORT");
                break;

                /* Command segment */
            case _TAG_CMD_BEGIN:
                // parsing command
                r = _command_parsing();
                ___CHECK_r("COMMAND");
                break;

            default:
                T_E("invalid tag %s on line %d\n", g_tag_str, g_line_number);
                return ICLI_RC_ERROR;
            }
            break;

        case ICLI_RC_ERR_EMPTY:
            break;

        case ICLI_RC_ERR_MORE:
            switch (_tag_get()) {

                /* Global segment */
            case _TAG_MODULE_IF_FLAG:
                ___MORE_GET();
                r = _value_str_get();
                ___CHECK_r("MODULE_IF_FLAG");
                break;

            case _TAG_STR_DEF:
                ___MORE_GET();
                r = _value_str_get();
                ___CHECK_r("STR_DEF");
                break;

            default:
                break;
            }
            break;

        default:
            T_E("fail to _str_tag_and_value_get()\n");
            return ICLI_RC_ERROR;
        }
    }

    //check if end of file
    if ( ! feof(g_fd) ) {
        T_E("read line %d\n", (g_line_number + 1));
        return ICLI_RC_ERROR;
    }

    // close srcipt file
    fclose( g_fd );

    // multiple command modes
    r = _multiple_command_mode_add();
    if ( r != ICLI_RC_OK ) {
        T_E("_multiple_command_mode_add()\n");
        return ICLI_RC_ERROR;
    }

    // automatically generate sub mode commands
    r = _sub_mode_gen();
    if ( r != ICLI_RC_OK ) {
        T_E("_sub_mode_add()\n");
        return ICLI_RC_ERROR;
    }

#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    // generate command string for html, that is to replace variable with CMD_VAR
    r = _var_cmd_gen();
    if ( r != ICLI_RC_OK ) {
        T_E("_sub_mode_add()\n");
        return ICLI_RC_ERROR;
    }
#endif

    return ICLI_RC_OK;
}

#define ___USAGE_PRINT() \
    printf("Usage : icli_cmd_gen [path]<*.icli> [-L] [-G path] [-I include_file]\n");\
    printf("  -L               genrate #line in C and H files.\n");\
    printf("                   If ignored, then #line is not generated.\n");\
    printf("  [path]<*.icli>   script file with optional path.\n");\
    printf("  [path]           the path to generate C and H files.\n");\
    printf("                   If it is ignored, then C and H files are generated\n");\
    printf("                   in the current directory.\n");

#define ___LINE_OPT(i) \
    (vtss_icli_str_cmp(argv[i], "-L") == 0 || vtss_icli_str_cmp(argv[i], "-l") == 0)

#define ___GENERATE_OPT(i) \
    (vtss_icli_str_cmp(argv[i], "-G") == 0 || vtss_icli_str_cmp(argv[i], "-g") == 0)

#define ___INCLUDE_OPT(i) \
    (vtss_icli_str_cmp(argv[i], "-I") == 0 || vtss_icli_str_cmp(argv[i], "-i") == 0)

/*
==============================================================================

    Public Function

==============================================================================
*/
int main(i32 argc, char *argv[])
{
    i32     r;
    char    *script_file   = NULL;
    char    *generate_path = NULL;
    char    *include_file  = NULL;

    switch ( argc ) {
    case 2:
        if (___LINE_OPT(1) || ___GENERATE_OPT(1) || ___INCLUDE_OPT(1)) {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        break;

    case 3:
        if ( ! ___LINE_OPT(2) ) {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        g_line_directive = TRUE;
        break;

    case 4:
        if ( ___GENERATE_OPT(2) ) {
            generate_path = argv[3];
        } else if ( ___INCLUDE_OPT(2) ) {
            include_file = argv[3];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        break;

    case 5:
        if ( ___LINE_OPT(2) ) {
            g_line_directive = TRUE;
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }

        if ( ___GENERATE_OPT(3) ) {
            generate_path = argv[4];
        } else if ( ___INCLUDE_OPT(3) ) {
            include_file = argv[4];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        break;

    case 6:
        if ( ___GENERATE_OPT(2) ) {
            generate_path = argv[3];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }

        if ( ___INCLUDE_OPT(4) ) {
            include_file = argv[5];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        break;

    case 7:
        if ( ___LINE_OPT(2) ) {
            g_line_directive = TRUE;
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }

        if ( ___GENERATE_OPT(3) ) {
            generate_path = argv[4];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }

        if ( ___INCLUDE_OPT(5) ) {
            include_file = argv[6];
        } else {
            ___USAGE_PRINT();
            return ICLI_RC_ERROR;
        }
        break;

    default:
        ___USAGE_PRINT();
        return ICLI_RC_ERROR;
    }

    /* get ICLI script file */
    script_file = argv[1];

    /* check script file name */
    if (_script_file_check(script_file) != 0) {
        T_E("invalid script file name\n");
        return ICLI_RC_ERROR;
    }

    /* get generation path */
    if ( generate_path ) {
        // get length
        r = vtss_icli_str_len( generate_path );

        //get path
        g_generate_path = icli_malloc(r + 1);
        if ( g_generate_path == NULL ) {
            T_E("fail to allocate memory for g_generate_path\n");
            return ICLI_RC_ERROR;
        }
        (void)vtss_icli_str_cpy(g_generate_path, generate_path);

        //remvoe last '/'
        if ( g_generate_path[r - 1] == ICLI_SLASH || g_generate_path[r - 1] == ICLI_BACK_SLASH ) {
            g_generate_path[r - 1] = 0;
        }

        //check if the path is valid or not
        if ( _is_file_dir(g_generate_path, ___DIR_CHECK) == FALSE ) {
            T_E("generation path, %s, is not a valid directory\n", g_generate_path);
            return ICLI_RC_ERROR;
        }
    }

    // parse include file for constant help string
    if ( include_file ) {
        if ( _include_file_parse(include_file) != ICLI_RC_OK ) {
            T_E("parse include file, %s\n", include_file);
            return ICLI_RC_ERROR;
        }
    }

    // open script file for parsing
    if ( _script_file_parse(script_file) != ICLI_RC_OK ) {
        T_E("parse script file, %s\n", script_file);
        return ICLI_RC_ERROR;
    }

    // command is necessary
    if ( g_command_number == 0 ) {
        T_E("there is no any command in the script file\n");
        return ICLI_RC_ERROR;
    }

    if ( g_command_number > _CMD_MAX_CNT ) {
        T_E("too many commands\n");
        return ICLI_RC_ERROR;
    }

    // init data
    memset( g_mode_var_name,    0, sizeof(g_mode_var_name) );
    memset( g_mode_var_type, 0xff, sizeof(g_mode_var_type) );
    memset( g_mode_var_range,   0, sizeof(g_mode_var_range) );

    memset( g_variable_name,    0, sizeof(g_variable_name) );
    memset( g_variable_type, 0xff, sizeof(g_variable_type) );
    memset( g_variable_range,   0, sizeof(g_variable_range) );

    memset( g_variable_byword,  0, sizeof(g_variable_byword) );

    // generate .c and .h file
    r = _generate_C_and_H();
    if (r != ICLI_RC_OK) {
        __U_E("generate %s_icli.c and %s_icli.h file\n", g_script_file, g_script_file);
        return ICLI_RC_ERROR;
    }

    // generate .ireg file
    r = _generate_IREG();
    if (r != ICLI_RC_OK) {
        T_E("generate %s.ireg file\n", g_script_file);
        return ICLI_RC_ERROR;
    }

    // generate .htm file
    r = _generate_HTM();
    if (r != ICLI_RC_OK) {
        T_E("generate %s.htm file\n", g_script_file);
        return ICLI_RC_ERROR;
    }

    // generate .txt file
    r = _generate_TXT();
    if (r != ICLI_RC_OK) {
        T_E("generate %s.txt file\n", g_script_file);
        return ICLI_RC_ERROR;
    }

    if ( g_script_file ) {
        icli_free(g_script_file);
    }
    if ( g_script_path ) {
        icli_free(g_script_path);
    }
    if ( g_generate_path ) {
        icli_free(g_generate_path);
    }
    if ( g_icli_c_file ) {
        icli_free(g_icli_c_file);
    }
    if ( g_icli_h_file ) {
        icli_free(g_icli_h_file);
    }
    if ( g_html_file ) {
        icli_free(g_html_file);
    }

    return ICLI_RC_OK;
}

