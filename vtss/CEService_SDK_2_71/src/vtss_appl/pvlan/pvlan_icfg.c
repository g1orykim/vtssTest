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

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "pvlan_api.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "pvlan_icfg.h"
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PVLAN

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define PVLAN_IDS_BUF_LEN       256

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
#if defined(PVLAN_SRC_MASK_ENA)
static i8 *pvlan_ids_to_txt(u32 pvlan_ids[PVLAN_ID_END], i8 *buf)
{
    u32             pvlan_id, first_pvlan = 1;
    i8              *pvlan_buf, p[10];
    BOOL            first_pvlan_found = FALSE, more_vlans = FALSE;;

    if ((pvlan_buf = (i8 *)VTSS_MALLOC(PVLAN_IDS_BUF_LEN)) == NULL) {
        memset(buf, 0, PVLAN_IDS_BUF_LEN);
        return buf;
    }
    memset(pvlan_buf, 0, PVLAN_IDS_BUF_LEN);
    for (pvlan_id = PVLAN_ID_START; pvlan_id <= PVLAN_ID_END; pvlan_id++) {
        if (pvlan_ids[pvlan_id - 1] == 1) {
            if (more_vlans == TRUE) {
                sprintf(p, "%c", ',');
                strcat(pvlan_buf, p);
                more_vlans = FALSE;
            }
            if (first_pvlan_found == FALSE) {
                first_pvlan = pvlan_id;
                first_pvlan_found = TRUE;
            }
        }
        if ((pvlan_id == PVLAN_ID_END) || (pvlan_ids[pvlan_id - 1] == 0)) {
            if (first_pvlan_found == TRUE) {
                first_pvlan_found = FALSE;
                more_vlans = TRUE;
                sprintf(p, "%u", first_pvlan);
                strcat(pvlan_buf, p);
                if (pvlan_id != (first_pvlan + 1)) {
                    sprintf(p, "%c", '-');
                    strcat(pvlan_buf, p);
                    sprintf(p, "%u", (pvlan_id - 1));
                    strcat(pvlan_buf, p);
                }
            }
        }
    }
    memcpy(buf, pvlan_buf, PVLAN_IDS_BUF_LEN);
    VTSS_FREE(pvlan_buf);

    return buf;
}
#endif

static vtss_rc PVLAN_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    vtss_rc                     rc = VTSS_RC_OK;
    vtss_isid_t                 isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t              iport = uport2iport(req->instance_id.port.begin_uport);
    int                         conf_changed = 0;
    BOOL                        member[VTSS_PORT_ARRAY_SIZE];
#if defined(PVLAN_SRC_MASK_ENA)
    u32                         pvlan_id, pvlan_ids[PVLAN_ID_END], def_pvlan_ids[PVLAN_ID_END];
    pvlan_mgmt_entry_t          conf;
    char                        buf[PVLAN_IDS_BUF_LEN];
    BOOL                        member_of_at_least_one_pvlan_id = FALSE;
#endif

    memset(member, 0, sizeof(member));
#if defined(PVLAN_SRC_MASK_ENA)
    memset(buf, 0, sizeof(buf));
    memset(pvlan_ids, 0, sizeof(pvlan_ids));
    memset(def_pvlan_ids, 0, sizeof(def_pvlan_ids));
    def_pvlan_ids[0] = 1;
#endif
    if ((rc = pvlan_mgmt_isolate_conf_get(isid, member)) != VTSS_RC_OK) {
        return rc;
    }
    conf_changed = (member[iport] == 0) ? FALSE : TRUE;
    if (req->all_defaults || conf_changed) {
        rc = vtss_icfg_printf(result, " %s%s\n", ((member[iport] != 0) ? "" : PVLAN_NO_FORM_TEXT), PVLAN_PORT_ISOLATE_TEXT);
    }
#if defined(PVLAN_SRC_MASK_ENA)
    for (pvlan_id = PVLAN_ID_START; pvlan_id < PVLAN_ID_END; pvlan_id++) {
        memset(&conf, 0, sizeof(conf));
        if (pvlan_mgmt_pvlan_get(isid, pvlan_id - 1, &conf, 0) != VTSS_RC_OK) {
            continue;
        }
        if (conf.ports[iport]) {
            member_of_at_least_one_pvlan_id = TRUE;
            pvlan_ids[pvlan_id - 1] = 1;
        }
    }
    conf_changed = memcmp(def_pvlan_ids, pvlan_ids, sizeof(pvlan_ids));
    if (req->all_defaults || conf_changed) {
        if (pvlan_ids[0] == 0) {
            rc = vtss_icfg_printf(result, " no pvlan 1\n");
        }
        if (member_of_at_least_one_pvlan_id) {
            rc = vtss_icfg_printf(result, " %s %s\n", PVLAN_PORT_MEMBER_TEXT, pvlan_ids_to_txt(pvlan_ids, buf));
        }
    }
#endif

    return (rc == VTSS_RC_OK) ? rc : VTSS_RC_ERROR;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc PVLAN_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       1. Port configuration
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_PVLAN_PORT_CONF, "pvlan", PVLAN_ICFG_port_conf);

    return rc;
}
