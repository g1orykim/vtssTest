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
******************************************************************************

    Revision history
    > CP.Wang, 2012/09/14 10:14
        - create

******************************************************************************
*/

/*
******************************************************************************

    Include File

******************************************************************************
*/
#include "conf_api.h"
#include "icli_api.h"
#include "icli_porting_trace.h"
#include "misc_api.h"
#include "port_api.h"
#include "msg_api.h"

/*
******************************************************************************

    Constant and Macro

******************************************************************************
*/
#define ICLI_CONF_VERSION       0x0d01385e

/*
******************************************************************************

    Type Definition

******************************************************************************
*/
typedef struct {
    u32                 version;
    icli_conf_data_t    conf;
} icli_conf_file_data_t;

/*
******************************************************************************

    Static Variable

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/

/*
******************************************************************************

    Public Function

******************************************************************************
*/
/*
    save ICLI config data into file

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL icli_conf_file_write(
    void
)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    icli_conf_file_data_t   *conf_file;
    ulong                   size;

    // get conf
    conf_file = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ICLI, &size);
    if ( conf_file == NULL || size != sizeof(icli_conf_file_data_t)) {
        conf_file = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_ICLI, sizeof(icli_conf_file_data_t));
        if ( conf_file == NULL ) {
            T_E("fail to creat conf\n");
            return FALSE;
        }
    }

    // get ICLI config data
    if ( icli_conf_get( &(conf_file->conf) ) != ICLI_RC_OK ) {
        T_E("fail to get ICLI config\n");
        return FALSE;
    }

    // get version
    conf_file->version = ICLI_CONF_VERSION;

    // write to file
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ICLI);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    // successful
    return TRUE;
}

/*
    load and apply ICLI config data from file

    INPUT
        b_default : create default config or not

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL icli_conf_file_load(
    IN BOOL     b_default
)
{
    icli_conf_file_data_t   *conf_file;
    ulong                   size;

    if ( misc_conf_read_use() ) {
        // get conf
        conf_file = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ICLI, &size);
        if ( conf_file == NULL || size != sizeof(icli_conf_file_data_t)) {
            conf_file = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_ICLI, sizeof(icli_conf_file_data_t));
            if ( conf_file == NULL ) {
                T_E("fail to creat conf\n");
                return FALSE;
            }
        }

        // version check
        if ( conf_file->version != ICLI_CONF_VERSION ) {
            b_default = TRUE;
        }

        // reset to default
        if ( b_default ) {
            if ( icli_conf_default() != ICLI_RC_OK ) {
                T_E("fail to reset conf to default\n");
                return FALSE;
            }

            // get ICLI config data
            if ( icli_conf_get( &(conf_file->conf) ) != ICLI_RC_OK ) {
                T_E("fail to get ICLI config\n");
                return FALSE;
            }

            // get version
            conf_file->version = ICLI_CONF_VERSION;
        } else {
            // apply conf
            if ( icli_conf_set( &(conf_file->conf) ) != ICLI_RC_OK ) {
                T_E("fail to apply conf\n");
                return FALSE;
            }
        }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        if ( conf_file != NULL ) {
            // get version
            conf_file->version = ICLI_CONF_VERSION;

            // close conf
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ICLI);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } else {
        // Not using conf; load defaults
        if ( icli_conf_default() != ICLI_RC_OK ) {
            T_E("fail to reset conf to default\n");
            return FALSE;
        }
    }

    // successful
    return TRUE;
}

/*
    configure current port ranges including stacking into ICLI

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL icli_conf_port_range(
    void
)
{
    icli_stack_port_range_t *rng;
    switch_iter_t           s_iter;
    port_iter_t             p_iter;
    icli_port_type_t        type;
    vtss_rc                 rc;
    BOOL                    b_rc;
    u32                     r_idx = 0;

    T_D("entry");

    if ( ! msg_switch_is_master() ) {
        // Not master => don't do anything
        return TRUE;
    }

    icli_port_range_reset();

    rng = (icli_stack_port_range_t *)icli_malloc(sizeof(icli_stack_port_range_t));
    if (!rng) {
        T_E("Cannot allocate memory for building port range");
        return FALSE;
    }
    memset(rng, 0, sizeof(icli_stack_port_range_t));

    if ((rc = switch_iter_init(&s_iter, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG)) != VTSS_RC_OK) {
        T_E("Cannot init switch iteration; rc=%d", rc);
        icli_free(rng);
        return FALSE;
    }

    while (switch_iter_getnext(&s_iter)) {
        u32               fe_cnt = 0, g_cnt = 0, g_2_5_cnt = 0, g_5_cnt = 0, g_10_cnt = 0, begin = 0;
        vtss_board_type_t board_type = port_isid_info_board_type_get(s_iter.isid);

        if ((rc = port_iter_init(&p_iter, NULL, s_iter.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_STACK)) != VTSS_RC_OK) {
            T_E("Cannot init port iteration for isid=%u; rc=%d. Ignoring switch.", s_iter.isid, rc);
            continue;
        }

        while (port_iter_getnext(&p_iter)) {
            if (! p_iter.exists) {
                continue;
            }
            port_cap_t cap = vtss_board_port_cap(board_type, p_iter.iport);
            if (cap & (PORT_CAP_10G_FDX)) {
                type = ICLI_PORT_TYPE_TEN_GIGABIT_ETHERNET;
                begin = ++g_10_cnt;
            } else if (cap & (PORT_CAP_5G_FDX)) {
                type = ICLI_PORT_TYPE_FIVE_GIGABIT_ETHERNET;
                begin = ++g_5_cnt;
            } else if (cap & PORT_CAP_2_5G_FDX) {
                type = ICLI_PORT_TYPE_2_5_GIGABIT_ETHERNET;
                begin = ++g_2_5_cnt;
            } else if (cap & PORT_CAP_1G_FDX) {
                type = ICLI_PORT_TYPE_GIGABIT_ETHERNET;
                begin = ++g_cnt;
            } else if (cap & PORT_CAP_100M_FDX) {
                type = ICLI_PORT_TYPE_FAST_ETHERNET;
                begin = ++fe_cnt;
            } else {
                T_D("Unexpected port speed. Capabilities = 0x%08x", (unsigned int)cap);
                continue;
            }

            if (rng->switch_range[r_idx].port_cnt == 0) {  // First port, init range entry
                rng->switch_range[r_idx].port_type   = type;
                rng->switch_range[r_idx].switch_id   = s_iter.usid;
                rng->switch_range[r_idx].begin_port  = begin;
                rng->switch_range[r_idx].usid        = s_iter.usid;
                rng->switch_range[r_idx].begin_uport = p_iter.uport;
                rng->switch_range[r_idx].isid        = s_iter.isid;
                rng->switch_range[r_idx].begin_iport = p_iter.iport;
            }

            if (rng->switch_range[r_idx].port_type != type) {  // Type change, we need a new range entry
                if (r_idx == ICLI_RANGE_LIST_CNT - 1) {
                    T_E("ICLI port range enumeration full; remaining ports will be ignored");
                    goto done;
                }
                ++r_idx;
                rng->switch_range[r_idx].port_cnt    = 0;
                rng->switch_range[r_idx].port_type   = type;
                rng->switch_range[r_idx].switch_id   = s_iter.usid;
                rng->switch_range[r_idx].begin_port  = begin;
                rng->switch_range[r_idx].usid        = s_iter.usid;
                rng->switch_range[r_idx].begin_uport = p_iter.uport;
                rng->switch_range[r_idx].isid        = s_iter.isid;
                rng->switch_range[r_idx].begin_iport = p_iter.iport;
            }

            rng->switch_range[r_idx].port_cnt++;

        }  /* while (port_iter_getnext(...)) */

        r_idx += (rng->switch_range[0].port_cnt > 0) ? 1 : 0;
    }  /* while (switch_iter_getnext(...) */

done:
    rng->cnt = r_idx;
    b_rc = icli_port_range_set(rng);
    icli_free(rng);

    T_D("exit, cnt=%u; return=%d", r_idx, (int)b_rc);

    return b_rc;
}
