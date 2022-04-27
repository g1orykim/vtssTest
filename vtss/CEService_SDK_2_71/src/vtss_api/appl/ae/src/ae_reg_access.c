/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#include <stdlib.h>
#include <stdio.h>

#include "ae.h"
#include "ae_cfg.h"
#include "ae_reg_access.h"
#include "vtss_ae_api.h"



/* TODO - API for this? */
#define VTSS_API_INST 0


/* 
	 This function must get the status read from the FAR END tx. If the AEC is used,
	 this should read AEC reg 1.
*/
int  ae_get_status(AE_LINK* link){    
	u32 status=0;
    if(vtss_ae_get_coeff_update_stat(VTSS_API_INST,link->id,&status) == VTSS_RC_OK)
        return status;    
    else{
        printf("Error vtss API\n");
        return 0;
    }
}

/*
	This function must set the request sent to the FAR END tx. If the AEC is used,
	this should write AEC reg 0.
*/	
void ae_set_req(AE_LINK* link, u16 request){    
    if(vtss_ae_set_coeff_update_req(VTSS_API_INST,link->id,request) != VTSS_RC_OK){
        printf("Error vtss API\n");
    }    
}

long long  time_usec(void){
	/* sleep and return current time */	
    return 0;
}



char ae_get_amin(AE_LINK* link){
    u32 retval=0;
    if(vtss_ae_get_eye_data(VTSS_API_INST,link->id, AMIN,&retval) == VTSS_RC_OK){
        return retval;
    }else{
        printf("Error vtss API\n");
        return 0;
    }
}

char ae_get_gain(AE_LINK* link){
    u32 retval=0;
    if(vtss_ae_get_eye_data(VTSS_API_INST,link->id, GAIN,&retval) == VTSS_RC_OK){
        return retval;
    }else{
        printf("Error vtss API\n");
        return 0;
    }
}

short ae_get_datas_edges(AE_LINK* link){
    u32 retval=0;
    if(vtss_ae_get_eye_data(VTSS_API_INST,link->id, DATA_EDGE,&retval) == VTSS_RC_OK){
        return retval;
    }else{
        printf("Error vtss API\n");
        return 0;
    }
}

char ae_get_h1t(AE_LINK* link){
    u32 retval=0;
    if(vtss_ae_get_eye_data(VTSS_API_INST,link->id, H1T,&retval) == VTSS_RC_OK){
        return retval;
    }else{
        printf("Error vtss API\n");
        return 0;
    }
}

char ae_req_dfe_sample(AE_LINK* link){
    u32 retval=0;
    if(vtss_ae_get_eye_data(VTSS_API_INST,link->id, DFE_SAMPLE,&retval) == VTSS_RC_OK){
        return retval;
    }else{
        printf("Error vtss API\n");
        return 0;
    }
   
}

void ae_init_rx(AE_LINK* link){

    if(vtss_ae_init_rx(VTSS_API_INST,link->id) != VTSS_RC_OK){        
        printf("Error vtss API\n");        
    }
    
}

void ae_return_rx(AE_LINK* link){
    if(vtss_ae_return_rx_cfg(VTSS_API_INST,link->id) != VTSS_RC_OK){        
        printf("Error vtss API\n");        
    }
}

void ae_init_tx(AE_LINK* link){
     if(vtss_ae_init_rx(VTSS_API_INST,link->id) != VTSS_RC_OK){        
        printf("Error vtss API\n");        
    }

}

