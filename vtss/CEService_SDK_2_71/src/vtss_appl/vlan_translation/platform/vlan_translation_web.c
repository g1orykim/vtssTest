/*

   Vitesse VCL software.

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
#include "web_api.h"
#include "cli.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#include "vlan_translation_api.h"

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_conf_grp2port_map_conf(CYG_HTTPD_STATE *p)
{
    //vtss_isid_t sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int iport, iport2, ct, row = 0, temp, errors = 0, grp_idx = 0, port_idx = 0;
    vlan_trans_mgmt_port2grp_conf_t     conf, existing_conf;
    BOOL next, add = FALSE;
    i8                                search_str[32];
    const char *value_str;
    size_t len;
    int port_count = port_isid_port_count(VTSS_ISID_LOCAL);

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VLAN_TRANSLATION)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {/* POST Method */

        /* Max number of entries allowed is equal to Max number of port in switch */
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++, row++) {/* loop for MAX group ids time */
            add = FALSE;
            memset(&conf, 0x0, sizeof(conf));
            memset(&existing_conf, 0x0, sizeof(existing_conf));

            /* Update: Hidden group id field for already existing entry */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenGrp_%u", row)) {
                conf.group_id = temp;
                /* existing entry may be updated, so check port list */
                /* get the new list of member ports for current entry from web page */
                for (iport2 = VTSS_PORT_NO_START; iport2 < port_count; iport2++) {/* loop to max port members time */
                    conf.ports[iport2] = FALSE;
                    sprintf(search_str, "grp_port_%u", iport2uport(iport2));
                    value_str = cyg_httpd_form_varable_string(p, search_str, &len);
                    if (sscanf(value_str, "mask_%d_%d", &grp_idx, &port_idx) == 2) {
                        if (grp_idx == row) {
                            add = TRUE;
                            conf.ports[uport2iport(port_idx)] = TRUE;
                        }
                    }
                }
                if (add && (vlan_trans_mgmt_port2grp_entry_add(conf.group_id, conf.ports)) != VTSS_OK) {
                    errors++;
                }
            }
        }

        /* Add new: check for newly added entries in web page */
        row = 1;
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++, row++) {/* loop for MAX group ids time */
            add = FALSE;
            memset(&conf, 0x0, sizeof(conf));
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "grp_new_%u", row)) {
                conf.group_id = temp;
                /* Look for port members */
                for (iport2 = VTSS_PORT_NO_START; iport2 < port_count; iport2++) {/* loop to max port members time */
                    conf.ports[iport2] = FALSE;
                    sprintf(search_str, "grp_port_%u", iport2uport(iport2));
                    value_str = cyg_httpd_form_varable_string(p, search_str, &len);
                    if (sscanf(value_str, "mask_new_%d_%d", &grp_idx, &port_idx) == 2) {
                        if (grp_idx == row) {
                            add = TRUE;
                            conf.ports[uport2iport(port_idx)] = TRUE;
                        }
                    }
                }
                /* add the new group entry now */
                if (add && (vlan_trans_mgmt_port2grp_entry_add(conf.group_id, conf.ports)) != VTSS_OK) {
                    errors++;
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/VT_grp_2_port_mapping.htm");
    } else {/* GET Method */
        /*Format:
            <Group ID 1>/<Port member 1>#<Port member 2>#...#<Port member max>|
            <Group ID 2>/<Port member 2>#<Port member 2>#...#<Port member max>|
            ...
            <Group ID max>/Port member 2#<Port member 2>#...#<Port member max>|
        */
        cyg_httpd_start_chunked("html");
        /* Show port to Group mapping */
        next = FALSE;
        conf.group_id = VT_NULL_GROUP_ID;
        while (vlan_trans_mgmt_port2grp_entry_get(&conf, next) == VTSS_RC_OK) {
            next = TRUE;
            if (conf.group_id > port_count) {
                continue;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/", conf.group_id);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                if (!PORT_NO_IS_STACK(iport)) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), iport == VTSS_PORT_NO_START ? "%u" : "#%u",
                                  conf.ports[iport]);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
            cyg_httpd_write_chunked("|", 1);
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}

static cyg_int32 handler_conf_vt_table_conf(CYG_HTTPD_STATE *p)
{
    vlan_trans_mgmt_grp2vlan_conf_t     vt_entry;
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VLAN_TRANSLATION)) {
        return -1;
    }
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {/* POST Method */
        i8                                search_str[32];
        int errors = 0, cur_vt = 0, temp;
        for (cur_vt = 0; cur_vt < VT_MAX_TRANSLATION_CNT; cur_vt++) {
            memset(&vt_entry, 0x0, sizeof(vt_entry));

            /* look for hiddenGrp id of already existing vt entries */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenGrp_%u", cur_vt)) {
                vt_entry.group_id = temp;

                /* get the vid mapped to current entry from web page data buffer */
                if (cyg_httpd_form_variable_int_fmt(p, &temp, "hiddenVID_%u", cur_vt)) {
                    vt_entry.vid = temp;

                    /* take a decision here after checking delete_$vt_entry checkbox where to update or delete this entry */
                    sprintf(search_str, "delete_%u", cur_vt);
                    if (cyg_httpd_form_varable_find(p, search_str)) {//find and delete complete entry
                        if ((vlan_trans_mgmt_grp2vlan_entry_delete(vt_entry.group_id, vt_entry.vid)) != VTSS_OK) {
                            errors++;
                        }
                    } else {/* update existing entry */
                        /* get the trans vid from web page data buffer */
                        if (cyg_httpd_form_variable_int_fmt(p, &temp, "vt_vid_%u", cur_vt)) {
                            if ((vlan_trans_mgmt_grp2vlan_entry_get(&vt_entry, 0)) == VTSS_RC_OK) {
                                if (vt_entry.trans_vid != temp) { /* Update the entry */
                                    vt_entry.trans_vid = temp;
                                    /* call add function to update current entry in switch */
                                    if ((vlan_trans_mgmt_grp2vlan_entry_add(vt_entry.group_id, vt_entry.vid,
                                                                            vt_entry.trans_vid)) != VTSS_OK) {
                                        errors++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            /* add new entries if any in switch after getting from web page data buffer */
            /* look for new group id for new VT entry */
            if (cyg_httpd_form_variable_int_fmt(p, &temp, "grp_id_new_%u", (cur_vt + 1))) {
                vt_entry.group_id = temp;
                /* look for new vid for new VT entry */
                if (cyg_httpd_form_variable_int_fmt(p, &temp, "vid_new_%u", (cur_vt + 1))) {
                    vt_entry.vid = temp;
                    /* look for new trans vid value from web page data buffer */
                    if (cyg_httpd_form_variable_int_fmt(p, &temp, "vt_vid_new_%u", (cur_vt + 1))) {
                        vt_entry.trans_vid  = temp;
                        /* now add new entry */
                        if ((vlan_trans_mgmt_grp2vlan_entry_add(vt_entry.group_id, vt_entry.vid,
                                                                vt_entry.trans_vid)) != VTSS_OK) {
                            errors++;
                        }
                    }
                }
            }
        }
        redirect(p, errors ? STACK_ERR_URL : "/VT_mapping.htm");
    } else {/* GET Method */
        /* Format: <Group ID i>/<VID i>/<Translated to VID i> */
        BOOL next;
        int ct;
        cyg_httpd_start_chunked("html");

        /* Show Group to VLAN Translations */
        next = FALSE;
        vt_entry.group_id = VT_NULL_GROUP_ID;
        while (vlan_trans_mgmt_grp2vlan_entry_get(&vt_entry, next) == VTSS_RC_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%u/%u|",
                          vt_entry.group_id,
                          vt_entry.vid,
                          vt_entry.trans_vid);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            next = TRUE;
        }
        cyg_httpd_end_chunked();
    }
    return -1;
}
/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t vlan_translation_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[512];
    (void) snprintf(buff, 512, "var configVlanTranslationMax = %d;\n", VT_MAX_TRANSLATION_CNT);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}


/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(vlan_translation_lib_config_js);
/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_conf_grp2port_handler, "/config/grp2port_map_config", handler_conf_grp2port_map_conf);

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_confg_vt_table_handler, "/config/vt_table", handler_conf_vt_table_conf);

/*****************************************************************************/
/*                                   End of File                             */
/*****************************************************************************/
