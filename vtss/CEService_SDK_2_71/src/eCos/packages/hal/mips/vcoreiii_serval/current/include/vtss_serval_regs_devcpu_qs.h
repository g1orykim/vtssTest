#ifndef _VTSS_SERVAL_REGS_DEVCPU_QS_H_
#define _VTSS_SERVAL_REGS_DEVCPU_QS_H_

/*
 *
 * VCore-III Register Definitions
 *
 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####
 */

#include "vtss_serval_regs_common.h"

#define VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(ri)   VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x0 + (ri))
#define  VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS  VTSS_BIT(1)
#define  VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_BYTE_SWAP  VTSS_BIT(0)

#define VTSS_DEVCPU_QS_XTR_XTR_RD(ri)        VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x2 + (ri))

#define VTSS_DEVCPU_QS_XTR_XTR_FRM_PRUNING(ri)  VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x4 + (ri))
#define  VTSS_F_DEVCPU_QS_XTR_XTR_FRM_PRUNING_PRUNE_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_QS_XTR_XTR_FRM_PRUNING_PRUNE_SIZE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_QS_XTR_XTR_FRM_PRUNING_PRUNE_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_DEVCPU_QS_XTR_XTR_FLUSH         VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x6)
#define  VTSS_F_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT  VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x7)
#define  VTSS_F_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(ri)   VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x9 + (ri))
#define  VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP  VTSS_BIT(0)

#define VTSS_DEVCPU_QS_INJ_INJ_WR(ri)        VTSS_IOREG(VTSS_TO_DEVCPU_QS,0xb + (ri))

#define VTSS_DEVCPU_QS_INJ_INJ_CTRL(ri)      VTSS_IOREG(VTSS_TO_DEVCPU_QS,0xd + (ri))
#define  VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(x)  VTSS_ENCODE_BITFIELD(x,21,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE     VTSS_ENCODE_BITMASK(21,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,21,2)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_ABORT  VTSS_BIT(20)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_EOF    VTSS_BIT(19)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF    VTSS_BIT(18)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

#define VTSS_DEVCPU_QS_INJ_INJ_STATUS        VTSS_IOREG(VTSS_TO_DEVCPU_QS,0xf)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_STATUS_WMARK_REACHED(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_STATUS_WMARK_REACHED     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_STATUS_WMARK_REACHED(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_STATUS_FIFO_RDY(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_STATUS_FIFO_RDY     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_STATUS_FIFO_RDY(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_STATUS_INJ_IN_PROGRESS(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_QS_INJ_INJ_STATUS_INJ_IN_PROGRESS     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_QS_INJ_INJ_STATUS_INJ_IN_PROGRESS(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_DEVCPU_QS_INJ_INJ_ERR(ri)       VTSS_IOREG(VTSS_TO_DEVCPU_QS,0x10 + (ri))
#define  VTSS_F_DEVCPU_QS_INJ_INJ_ERR_ABORT_ERR_STICKY  VTSS_BIT(1)
#define  VTSS_F_DEVCPU_QS_INJ_INJ_ERR_WR_ERR_STICKY  VTSS_BIT(0)


#endif /* _VTSS_SERVAL_REGS_DEVCPU_QS_H_ */