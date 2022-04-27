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

    1. merge *.ireg into icli_cmd_gen.c
    Run on linux and Windows

    Revision history
    > CP.Wang, 2012/04/06 12:52
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
    printf("Usage : icli_cmd_ireg <path> <list of *.ireg>\n");\
    printf("    <path> : path of icli_cmd_reg.c and where <list of *.ireg> are\n");\

#define _ICLI_CMD_REG_ALWAYS_CREATE         1

/*
==============================================================================

    Type Definition

==============================================================================
*/

/*
==============================================================================

    Static Variable

==============================================================================
*/
static char     *g_generate_path = NULL;
static char     *g_reg_c_file    = NULL;

/*
==============================================================================

    Static Function

==============================================================================
*/

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

#define _CMD_REG_MAX_LINE   (ICLI_CMD_CNT * 2)
#define _CMD_REG_LINE_LEN   128
#define _CMD_REG_BEGIN      "static _cmd_register_cb_t   *g_cmd_reg[ICLI_CMD_CNT] = {\n"
#define _CMD_REG_END        "};\n"
#define _CMD_REG_NULL_CB    "NULL"
#define _CMD_REG_INCLUDE    "#include"

static char g_reg_include[ICLI_STR_MAX_LEN + 1]     = {0};
static char g_reg_api[ICLI_STR_MAX_LEN + 1]         = {0};
static char g_module_if_flag[ICLI_STR_MAX_LEN + 1]  = {0};

static BOOL _cmd_reg_add(void)
{
    FILE    *f;
    char    line_buf[_CMD_REG_MAX_LINE][_CMD_REG_LINE_LEN + 1];
    BOOL    b_start,
            b_add,
            b_include,
            b_if;
    i32     line,
            i, j,
            include_line;
    i32     reg_include_line,
            reg_api_line;

    /* open file for read */
    f = fopen( g_reg_c_file, "r" );
    if ( f == NULL ) {
        __T_E("open %s\n", g_reg_c_file);
        return FALSE;
    }

    /* read all lines into line_buf */
    memset(line_buf, 0, sizeof(line_buf));
    for ( line = 0; line < _CMD_REG_MAX_LINE && fgets(line_buf[line], _CMD_REG_LINE_LEN, f) != NULL; ++line ) {
        ;
    }

    if ( line >= _CMD_REG_MAX_LINE ) {
        __T_E("file is too large, > %d lines\n", _CMD_REG_MAX_LINE);
        fclose( f );
        return FALSE;
    }

    //check if end of file
    if ( ! feof(f) ) {
        __T_E("read line %d\n", line + 1);
        fclose( f );
        return FALSE;
    }

    /*
        find duplicate
        if duplicate then break
        if not then add command register
    */
    b_start   = FALSE;
    b_add     = FALSE;
    b_include = FALSE;
    include_line     = -1;
    reg_include_line = -1;
    reg_api_line     = -1;
    for ( i = 0; i < line; ++i ) {
        if ( include_line == -1 ) {
            if ( b_include ) {
                if ( line_buf[i][0] != '#' ) {
                    include_line = i;
                } else if ( vtss_icli_str_str(g_reg_include, line_buf[i]) != NULL ) {
                    reg_include_line = i;
                }
            } else {
                if ( line_buf[i][0] == '#' ) {
                    b_include = TRUE;
                    if ( vtss_icli_str_str(g_reg_include, line_buf[i]) != NULL ) {
                        reg_include_line = i;
                    }
                }
            }
        }

        if ( b_start ) {
            // check if compile flag
            if ( line_buf[i][0] == '#' ) {
                continue;
            }

            if ( vtss_icli_str_sub("NULL", line_buf[i], TRUE, NULL) >= 0 ) {
                continue;
            }

            /* check end */
            if ( vtss_icli_str_cmp(line_buf[i], _CMD_REG_END) == 0 ) {
                __T_E("register is full already\n");
                b_start = FALSE;
                break;
            }

            // skip space
            for ( j = 0; line_buf[i][j] == ' '; ++j ) {
                ;
            }

            /* check duplicate */
            if ( line_buf[i][j] == '&' ) {
                ++j;
                if ( vtss_icli_str_sub(g_reg_api, &(line_buf[i][j]), TRUE, NULL) >= 0 ) {
                    //T_E("Command already has been registered.\n");
                    reg_api_line = i;
                    break;
                }
            } else {
                /* add */
                memset( line_buf[i], 0, sizeof(line_buf[i]));
                icli_sprintf(line_buf[i], "    &%s,\n", g_reg_api);
                reg_api_line = i;
                b_add = TRUE;
                break;
            }
        } else {
            if ( vtss_icli_str_cmp(line_buf[i], _CMD_REG_BEGIN) == 0 ) {
                b_start = TRUE;
            }
        }
    }

    // close file
    fclose( f );

    if ( b_start == FALSE ) {
        __T_E("find reg cb array\n");
        return FALSE;
    }

    if ( include_line == -1 ) {
        __T_E("get include line\n");
        return FALSE;
    }

    /* open file for write */
    f = fopen( g_reg_c_file, "w" );
    if ( f == NULL ) {
        __T_E("open %s\n", g_reg_c_file);
        return FALSE;
    }

    /* add successful then write back to the file */
    if ( b_add ) {
        for ( i = 0; i < line; ++i ) {
            if ( i == include_line ) {
                if ( g_module_if_flag[0] ) {
                    fprintf( f, "#if %s\n", g_module_if_flag);
                    fprintf( f, "#include \"%s\"\n", g_reg_include);
                    fprintf( f, "#endif\n");
                } else {
                    fprintf( f, "#include \"%s\"\n", g_reg_include);
                }
            } else if ( i == reg_api_line ) {
                if ( g_module_if_flag[0] ) {
                    fprintf( f, "#if %s\n", g_module_if_flag);
                    fprintf( f, "%s", line_buf[i]);
                    fprintf( f, "#else\n");
                    fprintf( f, "NULL,\n");
                    fprintf( f, "#endif\n");
                    continue;
                }
            }
            fprintf(f, "%s", line_buf[i]);
        }
    } else {
        b_if = FALSE;
        if ( vtss_icli_str_sub("#if ", line_buf[reg_include_line - 1], TRUE, NULL) >= 0 ) {
            b_if = TRUE;
        }
        for ( i = 0; i < line; ++i ) {
            if ( b_if ) {
                if ( i == (reg_include_line - 1) ) {
                    if ( g_module_if_flag[0] ) {
                        fprintf( f, "#if %s\n", g_module_if_flag);
                    } else {
                        fprintf( f, "#if 1\n");
                    }
                    continue;
                } else if ( i == (reg_api_line - 1) ) {
                    if ( g_module_if_flag[0] ) {
                        fprintf( f, "#if %s\n", g_module_if_flag);
                    } else {
                        fprintf( f, "#if 1\n");
                    }
                    continue;
                }
            } else {
                if ( i == reg_include_line ) {
                    if ( g_module_if_flag[0] ) {
                        fprintf( f, "#if %s\n", g_module_if_flag);
                        fprintf(f, "%s", line_buf[i]);
                        fprintf( f, "#endif\n");
                        continue;
                    }
                } else if ( i == reg_api_line ) {
                    if ( g_module_if_flag[0] ) {
                        fprintf( f, "#if %s\n", g_module_if_flag);
                        fprintf( f, "%s", line_buf[i]);
                        fprintf( f, "#else\n");
                        fprintf( f, "NULL,\n");
                        fprintf( f, "#endif\n");
                        continue;
                    }
                }
            }
            fprintf(f, "%s", line_buf[i]);
        }
    }

    // close srcipt file
    fclose( f );

    return TRUE;
}

static const char _cmd_reg_begin[] =
    "/*\n"
    "\n"
    " Vitesse Switch Application software.\n"
    "\n"
    " Copyright (c) 2002-2011 Vitesse Semiconductor Corporation \"Vitesse\". All\n"
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
    "        This file is genreated by script.\n"
    "        Please do -NOT- modify this file.\n"
    "\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "*/\n"
    "#include \"icli_api.h\"\n";

static const char _cmd_reg_middle[] =
    "\n"
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    Type Definition\n"
    "\n"
    "******************************************************************************\n"
    "*/\n"
    "typedef i32 _cmd_register_cb_t(void);\n"
    "\n"
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    Static Variable\n"
    "\n"
    "******************************************************************************\n"
    "*/\n";

static const char _cmd_reg_end[] =
    "};\n"
    "\n"
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    Extern Function\n"
    "\n"
    "******************************************************************************\n"
    "*/\n"
    "BOOL icli_cmd_reg(void)\n"
    "{\n"
    "    i32     i;\n"
    "\n"
    "    for ( i = 0; i < ICLI_CMD_CNT; ++i ) {\n"
    "        if ( g_cmd_reg[i] != NULL ) {\n"
    "            (void)(g_cmd_reg[i])();\n"
    "        }\n"
    "    }\n"
    "    return TRUE;\n"
    "}\n";

static const char _cmd_reg_h[] =
    "/*\n"
    "\n"
    " Vitesse Switch Application software.\n"
    "\n"
    " Copyright (c) 2002-2011 Vitesse Semiconductor Corporation \"Vitesse\". All\n"
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
    "        This file is genreated by script.\n"
    "        Please do -NOT- modify this file.\n"
    "\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "*/\n"
    "\n"
    "/*\n"
    "******************************************************************************\n"
    "\n"
    "    Extern Function\n"
    "\n"
    "******************************************************************************\n"
    "*/\n"
    "BOOL icli_cmd_reg(void);\n"
    "\n";

static BOOL _cmd_reg_create(void)
{
    FILE    *f;
    i32     i;

    /* create C file */
    f = fopen( g_reg_c_file, "w" );
    if ( f == NULL ) {
        __T_E("open %s\n", g_reg_c_file);
        return FALSE;
    }

    /* begin */
    fprintf( f, _cmd_reg_begin );

    /* include */
    if ( g_module_if_flag[0] ) {
        fprintf( f, "#if %s\n", g_module_if_flag);
        fprintf( f, "#include \"%s\"\n", g_reg_include);
        fprintf( f, "#endif\n");
    } else {
        fprintf( f, "#include \"%s\"\n", g_reg_include);
    }

    /* middle */
    fprintf( f, _cmd_reg_middle );
    fprintf( f, _CMD_REG_BEGIN );

    /* g_reg_api */
    if ( g_module_if_flag[0] ) {
        fprintf( f, "#if %s\n", g_module_if_flag);
        fprintf( f, "    &%s,\n", g_reg_api);
        fprintf( f, "#else\n");
        fprintf( f, "NULL,\n");
        fprintf( f, "#endif\n");
    } else {
        fprintf( f, "    &%s,\n", g_reg_api);
    }

    /* other NULLs */
    for ( i = 0; i < (ICLI_CMD_CNT - 1); ++i ) {
        fprintf( f, "    NULL,\n");
    }

    /* end */
    fprintf( f, _cmd_reg_end );

    /* close file */
    fclose( f );

    return TRUE;
}

static void _remove_newline(
    IN char  *s
)
{
    for ( ; *s; ++s ) {
        if ( *s == '\n' ) {
            *s = 0;
            return;
        }
    }
}

#if 0 /* CP, 2012/09/13 15:22, try until ireg_file is generated */
#define __TRY_TIMES     10
#endif

static i32 _get_ireg(
    IN char  *ireg_file
)
{
    FILE    *f;

#if 0 /* CP, 2012/09/13 15:22, try until ireg_file is generated */
    i32     i;

    for ( i = 0; i < __TRY_TIMES; ++i ) {
        f = fopen( ireg_file, "r" );
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
    // open file for read
    f = fopen( ireg_file, "r" );
#endif
    if ( f == NULL ) {
        __T_E("open %s\n", ireg_file);
        return ICLI_RC_ERROR;
    }

    // get include file
    if ( fgets(g_reg_include, ICLI_STR_MAX_LEN, f) == NULL ) {
        __T_E("get register include file\n");
        return ICLI_RC_ERROR;
    }
    _remove_newline( g_reg_include );

    // get register API
    if ( fgets(g_reg_api, ICLI_STR_MAX_LEN, f) == NULL ) {
        __T_E("get register API\n");
        return ICLI_RC_ERROR;
    }
    _remove_newline( g_reg_api );

    // get Module IF flag
    if ( fgets(g_module_if_flag, ICLI_STR_MAX_LEN, f) == NULL ) {
        __T_E("get Module IF flag\n");
        return ICLI_RC_ERROR;
    }
    _remove_newline( g_module_if_flag );

    fclose( f );
    return ICLI_RC_OK;
}

#if _ICLI_CMD_REG_ALWAYS_CREATE
static i32 _command_register_create(
    IN char  *ireg_file
)
{
    // open file for read
    if ( _get_ireg(ireg_file) != ICLI_RC_OK ) {
        __T_E("fail to get ireg\n");
        return ICLI_RC_ERROR;
    }

    /* create icli_cmd_reg.c */
    if ( _cmd_reg_create() == FALSE ) {
        __T_E("fail to create command register, %s\n", g_reg_c_file);
        return ICLI_RC_ERROR;
    }
    return ICLI_RC_OK;
}

static i32 _command_register_add(
    IN char  *ireg_file
)
{
    // open file for read
    if ( _get_ireg(ireg_file) != ICLI_RC_OK ) {
        __T_E("fail to get ireg\n");
        return ICLI_RC_ERROR;
    }

    /* add icli_cmd_reg.c */
    if ( _cmd_reg_add() == FALSE ) {
        __T_E("fail to add command register, %s\n", g_reg_c_file);
        return ICLI_RC_ERROR;
    }
    return ICLI_RC_OK;
}
#else
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

static i32 _command_register(
    IN char  *ireg_file
)
{
    // open file for read
    if ( _get_ireg(ireg_file) != ICLI_RC_OK ) {
        __T_E("fail to get ireg\n");
        return ICLI_RC_ERROR;
    }

    /* make icli_cmd_reg.c */
    if ( _is_file_dir(g_reg_c_file, ___FILE_CHECK) ) {
        /* exist */
        if ( _cmd_reg_add() == FALSE ) {
            __T_E("fail to add command register, %s\n", g_reg_c_file);
            return ICLI_RC_ERROR;
        }
    } else {
        /* not exist */
        if ( _cmd_reg_create() == FALSE ) {
            __T_E("fail to create command register, %s\n", g_reg_c_file);
            return ICLI_RC_ERROR;
        }
    }
    return ICLI_RC_OK;
}
#endif

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

    // get icli_cmd_reg
    g_reg_c_file = icli_malloc(i + 32);
    if ( g_reg_c_file == NULL ) {
        __T_E("fail to allocate memory for g_reg_c_file\n");
        return ICLI_RC_ERROR;
    }
    icli_sprintf(g_reg_c_file, "%s/icli_cmd_reg.c", g_generate_path);

    // register for each ireg
#if _ICLI_CMD_REG_ALWAYS_CREATE
    i = 2;
    if ( _command_register_create(argv[i]) != ICLI_RC_OK ) {
        __T_E("fail to create command register, %s\n", argv[i]);
        return ICLI_RC_ERROR;
    }

    for ( ++i; i < argc; ++i ) {
        if ( _command_register_add(argv[i]) != ICLI_RC_OK ) {
            __T_E("fail to create command register, %s\n", argv[i]);
            return ICLI_RC_ERROR;
        }
    }
#else
    for ( i = 2; i < argc; ++i ) {
        if ( _command_register(argv[i]) != ICLI_RC_OK ) {
            __T_E("fail to command register, %s\n", argv[i]);
            return ICLI_RC_ERROR;
        }
    }
#endif

    icli_free( g_generate_path );
    icli_free( g_reg_c_file );

    /* successful */
    return ICLI_RC_OK;
}
