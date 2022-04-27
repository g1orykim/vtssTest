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

#ifndef AE_H
#define AE_H 1

#include "vtss_types.h"

#define AE_KNOW_TX  (HSS10G||FFE_LUT||TS)

typedef struct{
    u32     id;
	i32     addr;
	u16     status;
	u16     rx_08;
	u16     rx_07;
	i32         i;
	u8     last_gain;
	u8     gain_tog_cnt;

	i16     c0_acc;
	i16     c0_cnv;
	i8      c0_tog;
	i16     c2_acc;
	u16     c2_cnv;
	i8      c2_tog;
	
	u16    tx_req;
	u8     tx_stat;
	u8     tx_upd;
	
    u8     tx_upd_cnt;
	u16    tx_init_req;
	
	u8 chan_est;
	u8 c0_prev;
	u8 c0_max;
	u8 c2_prev;
	u8 c2_max;
	u8 abort;
	u8 last_gain_sgn;
	
	u8 requires_update;
	u8 requires_wait;

	u8 trace;
    i32 zfe_cnt;

#if AE_KNOW_TX
	int c0;
	int c1;
	int c2;
#else
	i8 c0;
	i8 c2;
#endif

	long long wait_until;
} AE_LINK;

#define AE_ZFE_PER 250


#define AE_REQUIRES_UPDATE_BIT 0x100
#define AE_REQUIRES_WAIT_BIT   0x200
#define AE_DONE_BIT            0x400
#define AE_FAILED_BIT          0x800

#define AE_REQUIRES_UPDATE(status) ((status&AE_REQUIRES_UPDATE_BIT)!=0)
#define AE_REQUIRES_WAIT(status)   ((status&AE_REQUIRES_UPDATE_BIT)!=0)
#define AE_DONE(status)            ((status&AE_DONE_BIT)!=0)
#define AE_FAILED(status)          ((status&AE_FAILED_BIT)!=0)

// These are the possible values for link->status
typedef enum {
	AE_RESET               = 0,
	AE_INIT_COEFS          =13 | AE_REQUIRES_UPDATE_BIT,
	AE_START_BLIND_DEC     = 1 | AE_REQUIRES_UPDATE_BIT,
	AE_BLIND_DEC           = 2 | AE_REQUIRES_UPDATE_BIT,
	AE_CHAN_EST            = 3 | AE_REQUIRES_UPDATE_BIT | AE_REQUIRES_WAIT_BIT,
	AE_EST_PRE_LOOP        = 4 | AE_REQUIRES_UPDATE_BIT,
	AE_EST_POST_LOOP       = 5 | AE_REQUIRES_UPDATE_BIT,
	AE_ZFE_UPDATE          = 6 | AE_REQUIRES_UPDATE_BIT,
	AE_H1TEQ_UPDATE        = 7 | AE_REQUIRES_UPDATE_BIT | AE_REQUIRES_WAIT_BIT,
	AE_CONVERGED           = 8 | AE_DONE_BIT,
	AE_ABORT               = 9 | AE_DONE_BIT | AE_FAILED_BIT,
	AE_SAMP_ERROR          =14 | AE_DONE_BIT | AE_FAILED_BIT,
	AE_FAIL_SIG_AMIN       =10 | AE_DONE_BIT | AE_FAILED_BIT,
	AE_FAIL_GAIN_TOG       =11 | AE_DONE_BIT | AE_FAILED_BIT,
	AE_FAIL_BND            =12 | AE_DONE_BIT | AE_FAILED_BIT,
	AE_ZFE_STATUS          =15
} AE_STATUS;

typedef enum {
	AE_TX_UPD_NONE,
	AE_TX_UPD_HOLD,
	AE_TX_UPD_PRESET_0,
	AE_TX_UPD_PRESET_1,
	AE_TX_UPD_CX_INC_0,
	AE_TX_UPD_CX_INC_1,
	AE_TX_UPD_CX_DEC_0,
	AE_TX_UPD_CX_DEC_1,
	AE_TX_UPD_CX_DEC_2,
	AE_TX_UPD_CX_DEC_3,
	AE_TX_UPD_CX_DEC_4,
	AE_TX_UPD_CX_DEC_5,
	AE_TX_UPD_AI_C1_0,
	AE_TX_UPD_AI_C1_1,
} AE_TX_UPD;

// These are the requests sent to the tx.
enum AE_TX_REQUEST	{
	AE_PRESET = 0x2000,   
	AE_INIT   = 0x1000,
	AE_C2_DEC = 0x20,
	AE_C2_INC = 0x10,
	AE_C1_DEC = 0x08,
	AE_C1_INC = 0x04,
	AE_C0_DEC = 0x02,
	AE_C0_INC = 0x01,
	AE_HOLD   = 0x00
};


#define AE_IS_NUPD(status) ((status) == 0)
#define AE_IS_UPD(status)  ((((status) & 0x2A) == 0) && (status) != 0)
#define AE_IS_BND(status)  (!AE_IS_NUPD(status) && !AE_IS_UPD(status))
	
/* Update the LOCAL tx. This function calls ae_get_status() and
   ae_set_req() to talk to the tx.
*/
char ae_update_tx(AE_LINK* link);

/* Initialize the LINK structure. addr must be used as an identifier for
   the rx and tx on either end of the channel.
*/
void ae_init(AE_LINK* link,int id);

/* This function advances the state of link. ae_init() must be
	 called to reset the link state, and hss_link_id() must be used
	 to create the rx and tx LINK_IDs before calling this function.

	 ae_run.c demonstrates how this function should be called.
*/	 
AE_STATUS ae_run_link(AE_LINK* link);

/* Runs all links in link[LINKS] */
AE_STATUS ae_run(AE_LINK* link, int links);
void send_all_updates(AE_LINK* link, int links);


/*
   When compiled for use with the AEC macro, these functions
   should read from and write to the proper local AEC registers.

   Otherwise, the link->tx_req values need to be transmitted from
   the rx where they are generated, to the asic containing their
   respective transmitters (remote). Then this function should be
   called on the local asic and return with link->tx_stat properly set
   according to the remote tx. See fw/ae/ae_main.c for details.
*/
int  ae_get_status(AE_LINK* link); 


/* rw */
void ae_set_req   (AE_LINK* link, u16 request);
int  ae_read_rx (AE_LINK* link, int addr);
void ae_write_rx(AE_LINK* link, int addr, int data);
int  ae_read_tx (AE_LINK* link, int addr);
void ae_write_tx(AE_LINK* link, int addr, int data);




#define AE_SET_BIT(val, bit) (val | (1<<(bit)))
#define AE_CLR_BIT(val, bit) (val &~(1<<(bit)))
#define AE_GET_BIT(val, bit) ((val >> bit)&1)

#define AE_MASK(hi, lo) ((1<<(hi-lo+1))-1)
#define AE_SET_BITS(val, hi, lo, bits)							\
	((val & ~((AE_MASK(hi,lo))<<lo)) | (bits << lo))
#define AE_GET_BITS(val, hi, lo)       (((val) >> (lo)) & (AE_MASK((hi),(lo))))

#define AE_RMW_RX(link, addr, hi, lo, bits)															\
	ae_write_rx(link, addr, AE_SET_BITS(ae_read_rx(link, addr), hi, lo, bits))

#define AE_RMW_TX(link, addr, hi, lo, bits)															\
	ae_write_tx(link, addr, AE_SET_BITS(ae_read_tx(link, addr), hi, lo, bits))

#endif
