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

#include "web_api.h"
#include "erps_api.h"
#include "vtss_erps_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

/*lint -esym(459,error)*/

#define ERPS_WEB_BUF_LEN 512

char *erps_error_txt(u32 error)
{
    switch (error)
    {
        case ERPS_RC_OK:                  
            return("ERPS_RC_OK");
        case ERPS_ERROR_PG_CREATION_FAILED:
            return ("PROTECTION GROUP CREATION FAILED");
        case ERPS_ERROR_GROUP_ALREADY_EXISTS:
            return ("PROTECTION GROUP ALREADY EXISTS");
        case ERPS_ERROR_INVALID_PGID:
            return ("PROTECTION GROUP ID INVALID");
        case ERPS_ERROR_CANNOT_SET_RPL_OWNER_FOR_ACTIVE_GROUP:
            return ("CAN NOT SET RPL OWNER FOR ACTIVE GROUP");
        case ERPS_ERROR_NODE_ALREADY_RPL_OWNER:
            return ("NODE IS RPL OWNER FOR GIVEN GROUP");
        case ERPS_ERROR_NODE_NON_RPL:
            return ("NODE IS NOT RPL OWNER");
        case ERPS_ERROR_SETTING_RPL_BLOCK:
            return ("ERROR IN SETTING RPL BLOCK");
        case ERPS_ERROR_VLANS_CANNOT_BE_ADDED:
            return ("VLANS CAN NOT BE ADDED");
        case ERPS_ERROR_VLAN_ALREADY_PARTOF_GROUP:
            return ("GIVEN VLAN IS ALREADY PART OF THE PROTECTION GROUP");
        case ERPS_ERROR_MAXIMUM_PVIDS_REACHED:
            return ("MAXIMUM PROTECTION VIDS REACHED");
        default:     
            return ("ERPS RC OK"); 
    }
}


cyg_int32 handler_config_erps_create(CYG_HTTPD_STATE *p)
{  
    vtss_erps_mgmt_conf_t    erps_conf;
    int                        ct, major_ring_id;
    int                        erps_id = 0, E_port,W_port, e_raps,w_raps,e_mep,w_mep, ring_type;
    char                       buf[32];
    
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_ERPS))
        return -1;
#endif

    if (p->method == CYG_HTTPD_METHOD_POST)
    {
        erps_conf.group_id = 0;
        while( erps_mgmt_getnext_protection_group_request(&erps_conf) == VTSS_RC_OK) {
            /* Created */
            erps_id = erps_conf.group_id;
            sprintf(buf, "del_%u", erps_conf.group_id);
            if (cyg_httpd_form_varable_find(p, buf)) {
                erps_conf.req_type              = ERPS_CMD_PROTECTION_GROUP_DELETE;
                erps_conf.group_id= erps_id;
                if(erps_mgmt_set_protection_group_request(&erps_conf) == VTSS_RC_OK) {//;
                }
            }
        }
        
        /* Add ERPS */
        if (cyg_httpd_form_varable_int(p, "new_erps", &erps_id)) {
            if (cyg_httpd_form_varable_int(p, "E_port", &E_port))
            if (cyg_httpd_form_varable_int(p, "W_port", &W_port)) 
            if (cyg_httpd_form_varable_int(p, "e_raps", &e_raps)) 
            if (cyg_httpd_form_varable_int(p, "w_raps", &w_raps)) 
            if (cyg_httpd_form_varable_int(p, "e_mep", &e_mep)) 
            if (cyg_httpd_form_varable_int(p, "w_mep", &w_mep)) 
            if (cyg_httpd_form_varable_int(p, "ring_type", &ring_type)) {
            
                erps_conf.req_type              = ERPS_CMD_PROTECTION_GROUP_ADD;
                erps_conf.group_id= erps_id;
                erps_conf.data.create.east_port = E_port;
                erps_conf.data.create.west_port = W_port;   
                erps_conf.data.create.ring_type = ring_type;
                if (cyg_httpd_form_varable_find(p, "inter_connected_node")) {
                    erps_conf.data.create.interconnected = TRUE;
                } else {
                    erps_conf.data.create.interconnected = FALSE;
                }
                if (cyg_httpd_form_varable_find(p, "virtual_channel")) {
                    erps_conf.data.create.virtual_channel = TRUE;
                } else {
                    erps_conf.data.create.virtual_channel = FALSE;
                }
                if (cyg_httpd_form_varable_int(p, "major_ring_id", &major_ring_id)) { 
                    erps_conf.data.create.major_ring_id = major_ring_id;
                } else if (ring_type == 0) {
                    erps_conf.data.create.major_ring_id = erps_id;
                } else {
                    erps_conf.data.create.major_ring_id = 0;
                }
                if ( erps_mgmt_set_protection_group_request(&erps_conf) == VTSS_RC_OK) {
                    erps_conf.group_id = erps_id;
                    erps_conf.req_type              =  ERPS_CMD_ADD_MEP_ASSOCIATION;              
                    erps_conf.data.mep.east_mep_id  = e_mep;
                    erps_conf.data.mep.west_mep_id  = w_mep;
                    
                    erps_conf.data.mep.raps_eastmep = e_raps;
                    erps_conf.data.mep.raps_westmep = w_raps;   
                    if(erps_mgmt_set_protection_group_request(&erps_conf)== VTSS_RC_OK) {
                    }
                }
            }
        }
        redirect(p, "/erps.htm");
    }
    else {
        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");            
        erps_conf.group_id = 0;
        while( erps_mgmt_getnext_protection_group_request(&erps_conf) == VTSS_RC_OK) {   /* Created */     
	     ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%d/%d/%d/%u/%s|",                              
            erps_conf.group_id,
            erps_conf.data.get.erpg.east_port,
            erps_conf.data.get.erpg.west_port,
            erps_conf.data.mep.raps_eastmep,
            erps_conf.data.mep.raps_westmep,
            erps_conf.data.mep.east_mep_id,            
            erps_conf.data.mep.west_mep_id,
            erps_conf.data.get.erpg.ring_type,
            erps_conf.data.get.erpg.inter_connected_node,
            erps_conf.data.get.erpg.virtual_channel,
            erps_conf.data.get.erpg.major_ring_id,
            ((!erps_conf.data.get.erpg.rpl_owner && !(erps_conf.data.get.stats.rx[0] || erps_conf.data.get.stats.rx[1])) ||
             (erps_conf.data.get.erpg.rpl_owner && !erps_conf.data.get.stats.rpl_blocked) ||
             (erps_conf.data.get.stats.east_port_state == ERPS_PORT_STATE_SF) ||
             (erps_conf.data.get.stats.west_port_state == ERPS_PORT_STATE_SF)) ? "Down":"Up");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

static u16 req_calc(u16 req)
{
    switch (req) {
        case 14: return(4); //Event
        case 13: return(3); //FS
        case 11: return(2); //SF
        case 07: return(1); //MS
        case 00: return(0); //NR
        default: return(0);
    }
}

cyg_int32 handler_config_erps(CYG_HTTPD_STATE* p)
{
    vtss_isid_t                 sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_erps_mgmt_conf_t       erps_conf;
    vtss_erps_mgmt_conf_t       erps_conf_set;
    int                         erps_id = 0, hold, guard, rpl_block=0;    
    int                         wtr = 0,ring_type = 0, topo_change=0, ct,erps_version = 0;
    int                         e_port= 0, w_port =0, comm, commport, rpl_role = 0, rpl_port = 0;
    char                        buf[100];
    u64                         wtr_time = 0;
    char search_str[32];
    bool                        clear_rpl = FALSE,revert =FALSE;
    static u32                  error = VTSS_RC_OK;
    
    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;   
    
    #ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_ERPS))
        return -1;
    #endif
    
    //
    // Setting new configuration
    //
    
    
    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (cyg_httpd_form_varable_int(p, "erps_id_hidden", &erps_id)) {  
            erps_conf.group_id = erps_id-1;	            
            if ((error =  erps_mgmt_getnext_protection_group_request( &erps_conf)) == VTSS_RC_OK) {   
                e_port = erps_conf.data.get.erpg.east_port;
                w_port = erps_conf.data.get.erpg.west_port;         	 
                
                if (cyg_httpd_form_varable_int(p, "ring_type_hidden", &ring_type))  {		  	
                    memset(&erps_conf_set,0,sizeof(erps_conf_set));                    
                    erps_conf_set.group_id = erps_conf.group_id;				 
                    
                    if (ring_type) { /* Sub Ring */
                        topo_change = FALSE;
                        sprintf(search_str, "topo_%d", ring_type);                        
                        if (cyg_httpd_form_varable_find(p, search_str))  { /* "on" if checked */
                            topo_change = TRUE;	                         
                        }
                        
                        if (topo_change) {
                            erps_conf_set.req_type = ERPS_CMD_TOPOLOGY_CHANGE_PROPAGATE;
                        } else {
                            erps_conf_set.req_type = ERPS_CMD_TOPOLOGY_CHANGE_NO_PROPAGATE;                                                                        
                        }
                        error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                    }
                }

                if (cyg_httpd_form_varable_int(p, "rpl_role", &rpl_role)) {
                if (cyg_httpd_form_varable_int(p, "rpl_block", &rpl_block)) {
                        clear_rpl = FALSE;
                    if (cyg_httpd_form_varable_find(p, "clear")) { /* "on" if checked */
                            clear_rpl = TRUE;
                    }
                    erps_conf_set.group_id = erps_conf.group_id;
                        if (rpl_block == 1) {
                            erps_conf_set.data.rpl_block.rpl_port =  erps_conf.data.get.erpg.east_port;
                        } else {
                            erps_conf_set.data.rpl_block.rpl_port =  erps_conf.data.get.erpg.west_port;  
                        }
                        if ((rpl_role == 1) && rpl_block) { /* RPL Owner */
                            if (clear_rpl) {
                                erps_conf_set.req_type = ERPS_CMD_UNSET_RPL_BLOCK;
                                error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                            } else {
                                erps_conf_set.req_type = ERPS_CMD_SET_RPL_BLOCK;
                                error = erps_mgmt_set_protection_group_request(& erps_conf_set);
                            }
                        } else if ((rpl_role == 2) && rpl_block)  { /* RPL Neighbor */
                            if (clear_rpl) {
                                erps_conf_set.req_type = ERPS_CMD_UNSET_RPL_NEIGHBOUR;
                                error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                            } else {
                                erps_conf_set.req_type = ERPS_CMD_SET_RPL_NEIGHBOUR;
                                error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                            } /* if (clear_rpl) */
                        } /* if ((rpl_role == 1) && (rpl_block)) */
                    } /* if (cyg_httpd_form_varable_int(p, "rpl_block", &rpl_block)) */
                } /* if (cyg_httpd_form_varable_int(p, "rpl_role", &rpl_role)) */
                
                if (cyg_httpd_form_varable_int(p, "wtr", &wtr)) {
                    memset(&erps_conf_set,0,sizeof(erps_conf_set));
                    erps_conf_set.group_id = erps_id;
                    erps_conf_set.req_type              = ERPS_CMD_WTR_TIMER;   
                    wtr_time = wtr;
					
                    erps_conf_set.data.timer.time       = MIN_TO_MS(wtr_time);
                    error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                }
                
                if (cyg_httpd_form_varable_int(p, "hold", &hold)) {
                    if((hold%100) == 0) {			
                        memset(&erps_conf_set,0,sizeof(erps_conf_set));
                        erps_conf_set.group_id = erps_id;
                        erps_conf_set.req_type              = ERPS_CMD_HOLD_OFF_TIMER;
                        erps_conf_set.data.timer.time       = hold;
                        error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                    }
                }
                
                if (cyg_httpd_form_varable_int(p, "guard", &guard)) {
                    if((guard%10) == 0) {			
                        memset(&erps_conf_set,0,sizeof(erps_conf_set));
                        erps_conf_set.group_id = erps_id;
                        erps_conf_set.req_type              = ERPS_CMD_GUARD_TIMER;
                        erps_conf_set.data.timer.time       = guard;
                        error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                    }
                }

                if (cyg_httpd_form_varable_int(p, "erps_version", &erps_version)) {
                    memset(&erps_conf_set,0,sizeof(erps_conf_set));
                    erps_conf_set.group_id = erps_id;
                    if(erps_version == 2)
                    erps_conf_set.req_type  = ERPS_CMD_DISABLE_VERSION_1_COMPATIBLE;   
                    else
                    erps_conf_set.req_type  = ERPS_CMD_ENABLE_VERSION_1_COMPATIBLE;					
                    
                    error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                }

                revert = FALSE;
                sprintf(search_str, "revert_%d", ring_type);                        
                if(cyg_httpd_form_varable_find(p, search_str)) { /* "on" if checked */
                    revert = TRUE;	                
                }
                
                if(revert) {				   	
                    erps_conf_set.req_type = ERPS_CMD_DISABLE_NON_REVERTIVE;
                }
                else {
                    erps_conf_set.req_type = ERPS_CMD_ENABLE_NON_REVERTIVE;
                }
                error = erps_mgmt_set_protection_group_request(&erps_conf_set);
            
                if (cyg_httpd_form_varable_int(p, "comm", &comm)) 
                if (comm) 
                if (cyg_httpd_form_varable_int(p, "commport", &commport))
                if ((comm == 3) || commport) {
                    memset(&erps_conf_set,0,sizeof(erps_conf_set));
                    erps_conf_set.group_id = erps_id;
                    erps_conf_set.data.create.east_port = (commport == 1) ? erps_conf.data.get.erpg.east_port : erps_conf.data.get.erpg.west_port; 
                    switch (comm) {
                        case 1:  erps_conf_set.req_type = ERPS_CMD_MANUAL_SWITCH; break;
                        case 2:  erps_conf_set.req_type = ERPS_CMD_FORCED_SWITCH; break;
                        case 3:  erps_conf_set.req_type = ERPS_CMD_CLEAR; break;
                        default: erps_conf_set.req_type = ERPS_CMD_CLEAR;
                    }
                    error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                }
            }
        }
        
        sprintf(buf, "/erps_config.htm?erps=%u&eport=%u&wport=%u", erps_id,e_port,w_port);
        redirect(p, buf);
    }
    else {
        cyg_httpd_start_chunked("html");        
        memset(&erps_conf, 0, sizeof(erps_conf));        
        
        if( cyg_httpd_form_varable_int(p, "erps", &erps_id)) {
            erps_conf.group_id = erps_id-1;
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",  erps_id);
        cyg_httpd_write_chunked(p->outbuffer, ct);      		
        
        if( erps_mgmt_getnext_protection_group_request(&erps_conf) == VTSS_RC_OK) {
            if (erps_conf.data.get.erpg.rpl_owner) {
                rpl_role = 1;
                rpl_port = ((erps_conf.data.get.erpg.rpl_owner_port == erps_conf.data.get.erpg.east_port) ? 1 : 2);
            } else if (erps_conf.data.get.erpg.rpl_neighbour) {
                rpl_role = 2;
                rpl_port = ((erps_conf.data.get.erpg.rpl_neighbour_port == erps_conf.data.get.erpg.east_port) ? 1 : 2);
            } else {
                rpl_role = 0;
                rpl_port = 0;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d/%d/%d/%d/%d/%d|%s/%llu/%llu/%llu/%d/%d|",
            erps_conf.data.get.erpg.east_port,
            erps_conf.data.get.erpg.west_port,
            erps_conf.data.mep.raps_eastmep,
            erps_conf.data.mep.raps_westmep,
            erps_conf.data.mep.east_mep_id,
            erps_conf.data.mep.west_mep_id,  
            erps_conf.data.get.erpg.ring_type,
            rpl_role,
            rpl_port,
            
            (erps_conf.data.get.stats.active)? "Up":"Down",
            erps_conf.data.get.erpg.guard_time,
            erps_conf.data.get.erpg.wtr_time,
            erps_conf.data.get.erpg.hold_off_time,
            
            erps_conf.data.get.erpg.version,
            erps_conf.data.get.erpg.revertive);
            cyg_httpd_write_chunked(p->outbuffer, ct);	
            
            if(erps_conf.data.get.erpg.ring_type) {	      
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
                erps_conf.data.get.erpg.topology_change);	
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }         
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%02X-%02X-%02X-%02X-%02X-%02X/%d/%d/%d/%d/%d/%02X-%02X-%02X-%02X-%02X-%02X/%d/%d/%d/%d/%llu/%s/%s|%d/%d",
            erps_conf.data.get.stats.state,
            erps_conf.data.get.stats.east_port_state,
            erps_conf.data.get.stats.west_port_state,

            (erps_conf.data.get.stats.tx) ? TRUE :FALSE,
            req_calc(erps_conf.data.get.stats.tx_req),
            erps_conf.data.get.stats.tx_rb,
            erps_conf.data.get.stats.tx_dnf,
            erps_conf.data.get.stats.tx_bpr,

            (erps_conf.data.get.stats.rx[0]) ? TRUE : FALSE,
            req_calc(erps_conf.data.get.stats.rx_req[0]),
            erps_conf.data.get.stats.rx_rb[0],
            erps_conf.data.get.stats.rx_dnf[0],
            erps_conf.data.get.stats.rx_bpr[0],
            erps_conf.data.get.stats.rx_node_id[0][0],
            erps_conf.data.get.stats.rx_node_id[0][1],
            erps_conf.data.get.stats.rx_node_id[0][2],
            erps_conf.data.get.stats.rx_node_id[0][3],
            erps_conf.data.get.stats.rx_node_id[0][4],
            erps_conf.data.get.stats.rx_node_id[0][5],

            (erps_conf.data.get.stats.rx[1]) ? TRUE : FALSE,
            req_calc(erps_conf.data.get.stats.rx_req[1]),
            erps_conf.data.get.stats.rx_rb[1],
            erps_conf.data.get.stats.rx_dnf[1],
            erps_conf.data.get.stats.rx_bpr[1],
            erps_conf.data.get.stats.rx_node_id[1][0],
            erps_conf.data.get.stats.rx_node_id[1][1],
            erps_conf.data.get.stats.rx_node_id[1][2],
            erps_conf.data.get.stats.rx_node_id[1][3],
            erps_conf.data.get.stats.rx_node_id[1][4],
            erps_conf.data.get.stats.rx_node_id[1][5],
            erps_conf.data.get.stats.east_blocked,
            erps_conf.data.get.stats.west_blocked,
            erps_conf.data.get.stats.admin_cmd,
            erps_conf.data.get.stats.fop_alarm,

            erps_conf.data.get.stats.wtr_remaining_time,
            
            (erps_conf.data.get.erpg.rpl_owner || erps_conf.data.get.erpg.rpl_neighbour) ? (erps_conf.data.get.stats.rpl_blocked ? "Up" : "Down") : "Up",
            (((erps_conf.data.get.stats.state == ERPS_STATE_PENDING) && erps_conf.data.get.stats.tx) || erps_conf.data.get.erpg.rpl_owner || erps_conf.data.get.stats.rx[0] || erps_conf.data.get.stats.rx[1]) ? "Up":"Down",
            erps_conf.data.get.erpg.rpl_neighbour ? ((erps_conf.data.get.erpg.rpl_neighbour_port == erps_conf.data.get.erpg.east_port) ? 1 : 2) : 0,
            erps_conf.data.get.erpg.rpl_neighbour);
            
            cyg_httpd_write_chunked(p->outbuffer, ct);         
        }
        
        if (error != VTSS_RC_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", erps_error_txt(error));
            cyg_httpd_write_chunked(p->outbuffer, ct);
            error = VTSS_RC_OK;
        }
        
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

cyg_int32 handler_config_vlan_erps(CYG_HTTPD_STATE* p)
{
    vtss_isid_t         sid  = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_erps_mgmt_conf_t    erps_conf;
    vtss_erps_mgmt_conf_t    erps_conf_set;
    int                 erps_id = 0, vid= 0;  
    int                 vlan_count = 0;
    int                 e_port= 0,w_port =0;
    char                buf[100];
    int                 ct;    
    static u32          error = VTSS_RC_OK;

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
    return -1;    

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_ERPS))
    return -1;
#endif
    
    if(p->method == CYG_HTTPD_METHOD_POST) {
        char search_str[32];
        int new_vid, new_entry;        
           
        //add the erps ID----TODO
        if (cyg_httpd_form_varable_int(p, "erps_id_hidden", &erps_id)) {  
            erps_conf.group_id = erps_id-1;	
            
            if( (error = erps_mgmt_getnext_protection_group_request( &erps_conf)) == VTSS_RC_OK) {      
            
                e_port = erps_conf.data.get.erpg.east_port;
                w_port = erps_conf.data.get.erpg.west_port;               
                
                for (vlan_count =0; vlan_count<PROTECTED_VLANS_MAX; vlan_count++) {
                    if(erps_conf.data.get.erpg.protected_vlans[vlan_count]) {
                        sprintf(search_str, "delete_%d", erps_conf.data.get.erpg.protected_vlans[vlan_count]);
                        if(cyg_httpd_form_varable_find(p, search_str)) // "delete" if checked
                        {
                           memset(&erps_conf_set,0,sizeof(erps_conf_set));                    
                           erps_conf_set.group_id = erps_id;
                           erps_conf_set.req_type              = ERPS_CMD_DEL_VLANS;
                           erps_conf_set.data.vid.num_vids     = 1;
            			   
                           erps_conf_set.data.vid.p_vid        = erps_conf.data.get.erpg.protected_vlans[vlan_count];        		
                           error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                         }                    
                    }
                }
            }                    
            /* Check for Add new VLANs */
            for (new_entry = 1; new_entry < PROTECTED_VLANS_MAX; new_entry++) {
                /* Add new VLAN */
                new_vid = new_entry;
                sprintf(search_str, "vid_new_%d", new_vid);
                if(cyg_httpd_form_varable_int(p, search_str, &vid)) {
                    memset(&erps_conf_set,0,sizeof(erps_conf_set));
                    
                    erps_conf_set.group_id = erps_id;
                    erps_conf_set.req_type              = ERPS_CMD_ADD_VLANS;
                    erps_conf_set.data.vid.num_vids     = 1;
                    erps_conf_set.data.vid.p_vid        = vid;
                    
                    error = erps_mgmt_set_protection_group_request(&erps_conf_set);
                }
            }
        }
        sprintf(buf, "/erps_vlan.htm?erps=%u&eport=%u&wport=%u", erps_id,e_port,w_port);
        redirect(p, buf);
    } else {                    /* CYG_HTTPD_METHODGET (+HEAD) */
        memset(&erps_conf, 0, sizeof(erps_conf)); 
        if( cyg_httpd_form_varable_int(p, "erps", &erps_id)) {
           erps_conf.group_id = erps_id-1;
        }
        cyg_httpd_start_chunked("html");             
             
        if( erps_mgmt_getnext_protection_group_request(&erps_conf) == VTSS_RC_OK) {
            for (vlan_count =0; vlan_count<PROTECTED_VLANS_MAX;vlan_count++) {
                if(erps_conf.data.get.erpg.protected_vlans[vlan_count]) {            
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",erps_conf.data.get.erpg.protected_vlans[vlan_count]);
                cyg_httpd_write_chunked(p->outbuffer, ct); 
                }
            }
        }        

        if (error != VTSS_RC_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", erps_error_txt(error));
            cyg_httpd_write_chunked(p->outbuffer, ct);
            error = VTSS_RC_OK;
        }

        cyg_httpd_end_chunked();   
    }
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t erps_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[ERPS_WEB_BUF_LEN];
    (void) snprintf(buff, ERPS_WEB_BUF_LEN,
                    "var configErpsProtectionGroupsMin = %d;\n"
                    "var configErpsProtectionGroupsMax = %d;\n",
                    ERPS_MIN_PROTECTION_GROUPS,
                    ERPS_MAX_PROTECTION_GROUPS/* Last ERPS ID */
        );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(erps_lib_config_js);



/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_erps_create, "/config/erpsCreate", handler_config_erps_create);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_erps, "/config/erpsConfig", handler_config_erps);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_vlan_config_erps, "/config/erpsVlanConfig", handler_config_vlan_erps);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
