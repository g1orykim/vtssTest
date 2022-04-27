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

    Revision history
    > CP.Wang, 11/18/2013 14:46
        - create

==============================================================================
*/
/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_icli.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
/* bit array macro's */
#define _VLAN_ENABLE_SIZE               VTSS_BF_SIZE( VTSS_VIDS )
#define _VLAN_ENABLE_GET(vid)           VTSS_BF_GET( g_vlan_enable_bit, vid )
#define _VLAN_ENABLE_SET(vid, val)      VTSS_BF_SET( g_vlan_enable_bit, vid, val )
#define _VLAN_ENABLE_CLR_ALL()          VTSS_BF_CLR( g_vlan_enable_bit, VTSS_VIDS )

#define _VLAN_ENTER_SIZE                VTSS_BF_SIZE( VTSS_VIDS )
#define _VLAN_ENTER_GET(vid)            VTSS_BF_GET( g_vlan_enter_bit, vid )
#define _VLAN_ENTER_SET(vid, val)       VTSS_BF_SET( g_vlan_enter_bit, vid, val )
#define _VLAN_ENTER_CLR_ALL()           VTSS_BF_CLR( g_vlan_enter_bit, VTSS_VIDS )

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
static u8   g_vlan_enable_bit[ _VLAN_ENABLE_SIZE ];
static u8   g_vlan_enter_bit[ _VLAN_ENTER_SIZE ];

/*
==============================================================================

    Static Function

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    initialization

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_vlan_init(
    void
)
{
    /* all enabled */
    memset(g_vlan_enable_bit, 0xFF, _VLAN_ENABLE_SIZE);

    /* all not enter */
    _VLAN_ENTER_CLR_ALL();

    return ICLI_RC_OK;
}

/*
    enable/disable a VLAN interface

    INPUT
        vid      : VLAN interface ID to add
        b_enable : TRUE - enable the VLAN interface, FALSE - disable it

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_vlan_enable_set(
    IN u32      vid,
    IN BOOL     b_enable
)
{
    if ( vid < 1 || vid > 4095 ) {
        T_W("invalid vid = %u\n", vid);
        return ICLI_RC_ERR_PARAMETER;
    }

    _VLAN_ENABLE_SET( vid, b_enable );
    return ICLI_RC_OK;
}

/*
    get if the VLAN interface is enabled or disabled

    INPUT
        vid : VLAN interface ID to get

    OUTPUT
        b_enable : TRUE - enabled, FALSE - disabled

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_vlan_enable_get(
    IN  u32     vid,
    OUT BOOL    *b_enable
)
{
    if ( vid < 1 || vid > 4095 ) {
        T_W("invalid vid = %u\n", vid);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( b_enable == NULL ) {
        T_W("b_enable == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    *b_enable = _VLAN_ENABLE_GET( vid );
    return ICLI_RC_OK;
}

/*
    enter/leave a VLAN interface

    INPUT
        vid     : VLAN interface ID to add
        b_enter : TRUE - enter the VLAN interface, FALSE - leave it

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_vlan_enter_set(
    IN u32      vid,
    IN BOOL     b_enter
)
{
    if ( vid < 1 || vid > 4095 ) {
        T_W("invalid vid = %u\n", vid);
        return ICLI_RC_ERR_PARAMETER;
    }

    _VLAN_ENTER_SET( vid, b_enter );
    return ICLI_RC_OK;
}

/*
    get if enter or leave the VLAN interface

    INPUT
        vid : VLAN interface ID to get

    OUTPUT
        b_enter : TRUE - enter, FALSE - leave

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_vlan_enter_get(
    IN  u32     vid,
    OUT BOOL    *b_enter
)
{
    if ( vid < 1 || vid > 4095 ) {
        T_W("invalid vid = %u\n", vid);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( b_enter == NULL ) {
        T_W("b_enter == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    *b_enter = _VLAN_ENTER_GET( vid );
    return ICLI_RC_OK;
}
