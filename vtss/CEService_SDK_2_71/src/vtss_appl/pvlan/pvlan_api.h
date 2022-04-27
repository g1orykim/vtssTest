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

#ifndef _VTSS_PVLAN_API_H_
#define _VTSS_PVLAN_API_H_

#define PVLAN_PORT_SIZE VTSS_PORT_BF_SIZE(PORT_NUMBER)

#define PVLAN_ID_START 1           /* First VLAN ID */
#define PVLAN_ID_END   VTSS_PVLANS /* Last VLAN ID */
#define PVLAN_ID_IS_LEGAL(x) ((x) >= PVLAN_ID_START && (x) <= PVLAN_ID_END)

/* PVLAN error codes (vtss_rc) */
typedef enum {
    PVLAN_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_PVLAN), /* Generic error code         */
    PVLAN_ERROR_PARM,                                           /* Illegal parameter          */
    PVLAN_ERROR_CONFIG_NOT_OPEN,                                /* Configuration open error   */
    PVLAN_ERROR_ENTRY_NOT_FOUND,                                /* Private VLAN not found     */
    PVLAN_ERROR_PVLAN_TABLE_EMPTY,                              /* Private VLAN table empty   */
    PVLAN_ERROR_PVLAN_TABLE_FULL,                               /* Private VLAN table full    */
    PVLAN_ERROR_STACK_STATE,                                    /* Illegal MASTER/SLAVE state */
    PVLAN_ERROR_UNSUPPORTED,                                    /* Unsupported feature        */
    PVLAN_ERROR_DEL_INSTEAD_OF_ADD,                             /* The PVLAN was deleted instead of added (because no ports were selected for the PVLAN). This might not be an error */
} pvlan_error_t;

/* PVLAN error text */
char *pvlan_error_txt(vtss_rc rc);

#if defined(VTSS_FEATURE_PVLAN) && VTSS_SWITCH_STANDALONE
#define PVLAN_SRC_MASK_ENA 1
typedef struct {
    vtss_pvlan_no_t privatevid;       /* Private VLAN ID */
    BOOL ports[VTSS_PORT_ARRAY_SIZE]; /* Port mask */
} pvlan_mgmt_entry_t;

/* PVLAN management functions */
vtss_rc pvlan_mgmt_pvlan_add(vtss_isid_t isid, pvlan_mgmt_entry_t *pvlan_mgmt_entry);

vtss_rc pvlan_mgmt_pvlan_del(vtss_pvlan_no_t privatevid);

vtss_rc pvlan_mgmt_pvlan_get(vtss_isid_t isid, vtss_pvlan_no_t privatevid, pvlan_mgmt_entry_t *pvlan_mgmt_entry, BOOL next);
#endif /* VTSS_FEATURE_PVLAN */

vtss_rc pvlan_mgmt_isolate_conf_set(vtss_isid_t isid, const BOOL member[VTSS_PORT_ARRAY_SIZE]);

vtss_rc pvlan_mgmt_isolate_conf_get(vtss_isid_t isid, BOOL member[VTSS_PORT_ARRAY_SIZE]);

/* Initialize module */
vtss_rc pvlan_init(vtss_init_data_t *data);


#endif /* _VTSS_PVLAN_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
