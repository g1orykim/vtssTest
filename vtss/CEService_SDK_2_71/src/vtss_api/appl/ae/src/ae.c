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

#include "ae.h"
#include "ae_cfg.h"
#include "ae_reg_access.h"



#include <stdio.h>



void ae_init(AE_LINK* link,int id){
  link->status = AE_RESET;
  link->i = 0;
  link->id = id;

  link->last_gain = 0;
  link->gain_tog_cnt = 0;

  link->c0_acc = 0;
  link->c0_cnv = 0;
  link->c0_tog = 0;
	
  link->c2_acc = 0;
  link->c2_cnv = 0;
  link->c2_tog = 0;	
		
  link->chan_est = 0;
  link->c0_prev  = 0;
  link->c0_max   = 0;
  link->c2_prev  = 0;
  link->c2_max   = 0;
  link->abort    = 0;
  link->zfe_cnt    = 0;

  link->requires_update = 0;
  link->requires_wait = 0;
	
 
	link->c0 = 0;
	link->c2 = 0;	

	link->tx_upd = AE_TX_UPD_NONE;
	link->tx_upd_cnt = 0;

	link->trace = 0;
}

#define AE_TX_UPD(label, ae_tx_upd, check) \
	label: if(check){ link->tx_upd = ae_tx_upd; return 0;}

char ae_update_tx(AE_LINK* link){
	switch(link->tx_upd){
	case AE_TX_UPD_NONE:     break;
	case AE_TX_UPD_HOLD:     goto TX_UPD_HOLD;
	case AE_TX_UPD_PRESET_0: goto TX_UPD_PRESET_0;
	case AE_TX_UPD_PRESET_1: goto TX_UPD_PRESET_1;
	case AE_TX_UPD_CX_INC_0: goto TX_UPD_CX_INC_0;
	case AE_TX_UPD_CX_INC_1: goto TX_UPD_CX_INC_1;		
	case AE_TX_UPD_CX_DEC_0: goto TX_UPD_CX_DEC_0;
	case AE_TX_UPD_CX_DEC_1: goto TX_UPD_CX_DEC_1;
	case AE_TX_UPD_CX_DEC_2: goto TX_UPD_CX_DEC_2;
	case AE_TX_UPD_CX_DEC_3: goto TX_UPD_CX_DEC_3;
	case AE_TX_UPD_CX_DEC_4: goto TX_UPD_CX_DEC_4;
	case AE_TX_UPD_CX_DEC_5: goto TX_UPD_CX_DEC_5;		
	case AE_TX_UPD_AI_C1_0:  goto TX_UPD_AI_C1_0;
	case AE_TX_UPD_AI_C1_1:  goto TX_UPD_AI_C1_1;		
	}

	link->tx_init_req = link->tx_req;
  ae_set_req(link, link->tx_init_req);
  switch(link->tx_init_req){
  case AE_HOLD:
		AE_TX_UPD(TX_UPD_HOLD,
							AE_TX_UPD_HOLD,
							!AE_IS_NUPD(ae_get_status(link)));
		
    link->tx_stat = ae_get_status(link);
		break;
		
  case AE_PRESET:
  case AE_INIT:
		AE_TX_UPD(TX_UPD_PRESET_0,
							AE_TX_UPD_PRESET_0,
							AE_IS_NUPD(ae_get_status(link)));

    link->tx_stat = ae_get_status(link);
		
    ae_set_req(link, AE_HOLD);
	  AE_TX_UPD(TX_UPD_PRESET_1,
							AE_TX_UPD_PRESET_1,
							!AE_IS_NUPD(ae_get_status(link)));
		break;
		
  case AE_C0_INC:
  case AE_C2_INC:
		if(link->tx_init_req == AE_C0_INC)
			link->c0++;
		else
			link->c2++;
		
		AE_TX_UPD(TX_UPD_CX_INC_0,
							AE_TX_UPD_CX_INC_0,
							AE_IS_NUPD(ae_get_status(link)));			

    link->tx_stat = ae_get_status(link);

    if(AE_IS_BND(link->tx_stat)){
      link->tx_stat = 0x1;
      if(link->tx_init_req == AE_C0_INC){
				link->c0_max = 1;
				link->c0 = 0;
#if(AE_DEBUG > 3)
				printf("C0 AT MAX\n");
#endif				
      }else{
				link->c2_max = 1;
				link->c2 = 0;
#if(AE_DEBUG > 3)
				printf("C2 AT MAX\n");
#endif				
      }
    }
		
    ae_set_req(link, AE_HOLD);
		AE_TX_UPD(TX_UPD_CX_INC_1,
							AE_TX_UPD_CX_INC_1,
							!AE_IS_NUPD(ae_get_status(link)));			
		break;
		
  case AE_C0_DEC:
  case AE_C2_DEC:
		if(link->tx_init_req == AE_C0_DEC)
			link->c0--;
		else
			link->c2--;
		
		AE_TX_UPD(TX_UPD_CX_DEC_0,
							AE_TX_UPD_CX_DEC_0,
							AE_IS_NUPD(ae_get_status(link)));			

    link->tx_stat = ae_get_status(link);
		
    ae_set_req(link, AE_HOLD);
		AE_TX_UPD(TX_UPD_CX_DEC_1,
							AE_TX_UPD_CX_DEC_1,
							!AE_IS_NUPD(ae_get_status(link)));			
			
    if(AE_IS_BND(link->tx_stat)){
      ae_set_req(link, AE_C1_DEC);
			AE_TX_UPD(TX_UPD_CX_DEC_2,
								AE_TX_UPD_CX_DEC_2,
								AE_IS_NUPD(ae_get_status(link)));			
			
      link->tx_stat = ae_get_status(link);
			 
      ae_set_req(link, AE_HOLD);
			AE_TX_UPD(TX_UPD_CX_DEC_3,
								AE_TX_UPD_CX_DEC_3,
								!AE_IS_NUPD(ae_get_status(link)));			

      ae_set_req(link, link->tx_init_req);
			AE_TX_UPD(TX_UPD_CX_DEC_4,
								AE_TX_UPD_CX_DEC_4,
								AE_IS_NUPD(ae_get_status(link)));			
      
      ae_set_req(link, AE_HOLD);
			AE_TX_UPD(TX_UPD_CX_DEC_5,
								AE_TX_UPD_CX_DEC_5,
								!AE_IS_NUPD(ae_get_status(link)));			
    }
		break;
  }
  
  if(AE_EN_AI_C1 && (link->tx_upd_cnt++%4) == 0){
			ae_set_req(link, AE_C1_INC);
			AE_TX_UPD(TX_UPD_AI_C1_0,
								AE_TX_UPD_AI_C1_0,
								AE_IS_NUPD(ae_get_status(link)));

      ae_set_req(link, AE_HOLD);
			AE_TX_UPD(TX_UPD_AI_C1_1,
								AE_TX_UPD_AI_C1_1,
								!AE_IS_NUPD(ae_get_status(link)));			
			
	}
	link->tx_upd = AE_TX_UPD_NONE;
	return 1;
}

AE_STATUS ae_run_link(AE_LINK* link){
  int h1t;
  if(link->abort) {
    link->status = AE_ABORT;
    goto RETURN;
  }

  switch(link->status){
  case AE_INIT_COEFS:
    goto INIT_COEFS;
  case AE_START_BLIND_DEC:
    goto START_BLIND_DEC;
  case AE_BLIND_DEC:
    goto BLIND_DEC;
  case AE_CHAN_EST:
    goto CHAN_EST;
  case AE_EST_PRE_LOOP:
    goto EST_PRE_LOOP;
  case AE_EST_POST_LOOP:
    goto EST_POST_LOOP;
  case AE_ZFE_UPDATE:
    goto ZFE;
  case AE_H1TEQ_UPDATE:
    goto H1TEQ;
	case AE_ZFE_STATUS:
		goto ZFE_SAMPLE;
  }

  ae_init_rx(link);

  link->tx_req = AE_HOLD;
  return (link->status = AE_INIT_COEFS);

 INIT_COEFS:
	if(AE_INIT_METH == 0 || AE_INIT_METH == 3)
		link->tx_req = AE_PRESET;
	else
		link->tx_req = AE_INIT;
  return (link->status = AE_START_BLIND_DEC);
	
 START_BLIND_DEC:
  link->i = AE_CE_DECS;

 BLIND_DEC:
  if(link->i != 0){ 
    link->i--;
		if(AE_INIT_METH == 2)
			link->tx_req = AE_C2_INC;
		else
			link->tx_req = AE_C2_DEC;
    return (link->status = AE_BLIND_DEC);
  }

	if(AE_INIT_METH != 3)
		goto ZFE;
	
  link->i = AE_CE_DECS;
 CHAN_EST:
  if(link->i != 0){
    link->i--;
	
    h1t = ae_get_h1t(link);
    if(h1t > link->chan_est) link->chan_est = h1t;
		
    link->tx_req = AE_C2_INC;
    return (link->status = AE_CHAN_EST);
  }

  link->i = AE_EST_PRE;
 EST_PRE_LOOP:
  if(link->i != 0){
    link->i--;
	
    link->tx_req = AE_C0_DEC;
    return (link->status = AE_EST_PRE_LOOP);
  }

  switch(link->chan_est){
  case 7: link->i = AE_EST_POST7;  break;
  case 6: link->i = AE_EST_POST6;  break;
  case 5:
  case 4: link->i = AE_EST_POST45; break;	
  default:link->i = AE_EST_POST123;break;	
  }

 EST_POST_LOOP:
  if(link->i != 0){
    link->i--;
	
    link->tx_req = AE_C2_DEC;
    return (link->status = AE_EST_POST_LOOP);
  }
	
  link->c0_max = 0;
  link->c2_max = 0;
	
 ZFE:
  if(AE_EN_ZFE) while(1){
			char req_complete;
			int datas, edges, i;
			signed char amin, gain;
			int sample_cnt;
			if(link->status == AE_ZFE_UPDATE){
				link->status = AE_RESET;
			
				if(AE_IS_BND(link->tx_stat)){
					link->status = AE_FAIL_BND;
					goto RETURN;
				}
			
				amin = ae_get_amin(link);
				gain = ae_get_gain(link);
				if(amin < AE_AMIN_TH
					 && gain >= AE_GAIN_TH){
					link->status = AE_FAIL_SIG_AMIN;
					goto RETURN;
				}

				if(gain > link->last_gain){
					if(!link->last_gain_sgn)
						link->gain_tog_cnt++;
					else
						link->gain_tog_cnt = 0;
					link->last_gain_sgn = 1;
				}else if(gain < link->last_gain){
					if(link->last_gain_sgn)
						link->gain_tog_cnt++;
					else
						link->gain_tog_cnt = 0;
					link->last_gain_sgn = 0;
				}else link->gain_tog_cnt = 0;

				if(link->gain_tog_cnt == 7) {
					link->status = AE_FAIL_GAIN_TOG;
					goto RETURN;
				}
				link->last_gain = gain;
			}
		
			if(((link->c0_cnv >= AE_CNV_LIM)
					||(link->c0_tog >= AE_TOG_LIM)
					||(link->c0_max))
				 && ((link->c2_cnv >= AE_CNV_LIM)
						 ||(link->c2_tog >= AE_TOG_LIM)
						 ||(link->c2_max))) break;

		ZFE_SAMPLE:			
			// ZFE GET SAMPLE
			req_complete = 0;
			sample_cnt = 0;			
			while(!req_complete){
				if(link->abort) {
					link->status = AE_ABORT;
					goto RETURN;
				}
				
				if(sample_cnt++ > AE_SAMP_REQ_LIM) {
					link->status = AE_SAMP_ERROR;
					goto RETURN;
				}
				req_complete = ae_req_dfe_sample(link);
			}
			
			datas = ae_get_datas_edges(link);
			edges = AE_GET_BITS(datas, 15, 8);
			datas = datas & 0xFF;

			// ZFE PROCESS SAMPLE
			for(i = 0; i < 6; i++){
				if(AE_GET_BIT(datas,0) != AE_GET_BIT(datas,1)){
					if(AE_GET_BIT(edges,1) != AE_GET_BIT(datas,2))
						link->c0_acc++;
					else
						link->c0_acc--;
				}
      
				if(AE_GET_BIT(datas,1) != AE_GET_BIT(datas,2)){
					if(AE_GET_BIT(edges,2) != AE_GET_BIT(datas,0))
						link->c2_acc++;
					else
						link->c2_acc--;
				}
      
				datas = datas >> 1;
				edges = edges >> 1;
			}

			// ZFE CHECK C0
			if(link->c0_tog == AE_TOG_LIM && !link->c0_max){

			}else if(link->c0_acc <= -AE_ACC_LIM){
				link->c0_max = 0;
				link->tx_req = AE_C0_DEC;
#if(AE_DEBUG > 3)
				printf("ZFE: %7s, c0= %6d %6d, c2= %6d %6d\n",
							 ae_tx_req_str(link->tx_req),
							 link->c0_acc,
							 link->c0_cnv,
							 link->c2_acc,
							 link->c2_cnv);
#endif				
				link->c0_cnv = 0;
				link->c0_acc = 0;

				if(link->c0_prev)
					link->c0_tog++;
				else
					link->c0_tog = 0;
      
#if(AE_DEBUG > 3)
				if(link->c0_tog == AE_TOG_LIM)
					printf("C0_TOG\n");
#endif

				link->c0_prev = 0;

				return (link->status = AE_ZFE_UPDATE);
			
			}else if(link->c0_acc >= AE_ACC_LIM && !link->c0_max){
				link->tx_req = AE_C0_INC;
#if(AE_DEBUG > 3)
				printf("ZFE: %7s, c0= %6d %6d, c2= %6d %6d\n",
							 ae_tx_req_str(link->tx_req),
							 link->c0_acc,
							 link->c0_cnv,
							 link->c2_acc,
							 link->c2_cnv);
#endif
				link->c0_cnv = 0;
				link->c0_acc = 0;

				if(!link->c0_prev)
					link->c0_tog++;
				else
					link->c0_tog = 0;


#if(AE_DEBUG > 3)
				if(link->c0_tog == AE_TOG_LIM)
					printf("C0_TOG\n");
#endif
				link->c0_prev = 1;

				return (link->status = AE_ZFE_UPDATE);
			}else{
				link->c0_cnv += 6;
			}

			// ZFE CHECK C2
			if(link->c2_tog == AE_TOG_LIM && !link->c2_max){
            
			}else if(link->c2_acc <= -AE_ACC_LIM){
				link->c2_max = 0;
				link->tx_req = AE_C2_DEC;
#if(AE_DEBUG > 3)
				printf("ZFE: %7s, c0= %6d %6d, c2= %6d %6d\n",
							 ae_tx_req_str(link->tx_req),
							 link->c0_acc,
							 link->c0_cnv,
							 link->c2_acc,
							 link->c2_cnv);
#endif
				link->c2_cnv = 0;
				link->c2_acc = 0;

				if(link->c2_prev)
					link->c2_tog++;
				else
					link->c2_tog = 0;


#if(AE_DEBUG > 3)
				if(link->c2_tog == AE_TOG_LIM)
					printf("C2_TOG\n");
#endif
				link->c2_prev = 0;

				return (link->status = AE_ZFE_UPDATE);
			
			}else if(link->c2_acc >= AE_ACC_LIM && !link->c2_max){
				link->tx_req = AE_C2_INC;
#if(AE_DEBUG > 3)
				printf("ZFE: %7s, c0= %6d %6d, c2= %6d %6d\n",
							 ae_tx_req_str(link->tx_req),
							 link->c0_acc,
							 link->c0_cnv,
							 link->c2_acc,
							 link->c2_cnv);
#endif
				link->c2_cnv = 0;
				link->c2_acc = 0;

				if(!link->c2_prev)
					link->c2_tog++;
				else
					link->c2_tog = 0;


#if(AE_DEBUG > 3)
				if(link->c2_tog == AE_TOG_LIM)
					printf("C2_TOG\n");
#endif
				link->c2_prev = 1;

				return (link->status = AE_ZFE_UPDATE);
			}else{
				link->c2_cnv += 6;
			}

			if(AE_DEBUG > 2){
				if(((link->zfe_cnt)++ % AE_ZFE_PER) == (AE_ZFE_PER-1)){
					return (link->status = AE_ZFE_STATUS);
				}				
			}
		}//while(1)

 H1TEQ: 
  if(AE_EN_H1T_EQ){
    char amin, gain;
    h1t = ae_get_h1t(link);
    if(h1t > AE_H1TEQ_GOAL){
		
      if(link->abort) {
				link->status = AE_ABORT;
				goto RETURN;
      }
			
      if(AE_IS_BND(link->tx_stat)){
				link->status = AE_FAIL_BND;
				goto RETURN;
      }
			
      amin = ae_get_amin(link);
      gain = ae_get_gain(link);
      if(amin <= AE_AMIN_TH 
				 && gain >= AE_GAIN_TH){
				link->status = AE_FAIL_SIG_AMIN;
				goto RETURN;
      }
			
      if(link->i & 1) {
				link->tx_req = AE_C0_DEC;
      }else{
				link->tx_req = AE_C2_DEC;
      }
			
      link->i++;
      return (link->status = AE_H1TEQ_UPDATE);
    }
  }	

  link->status = AE_CONVERGED;
	
 RETURN:
  ae_return_rx(link);
  return link->status;
}
